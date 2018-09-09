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
 * cvmx-bgxx-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon bgxx.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_BGXX_DEFS_H__
#define __CVMX_BGXX_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_CONFIG(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_CONFIG(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000000ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_CONFIG(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000000ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_INT(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_INT(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000020ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_INT(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000020ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_PRT_CBFC_CTL(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_PRT_CBFC_CTL(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000408ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_PRT_CBFC_CTL(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000408ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_RX_ADR_CTL(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_RX_ADR_CTL(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E00000A0ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_RX_ADR_CTL(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E00000A0ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_RX_BP_DROP(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_RX_BP_DROP(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000080ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_RX_BP_DROP(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000080ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_RX_BP_OFF(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_RX_BP_OFF(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000090ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_RX_BP_OFF(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000090ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_RX_BP_ON(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_RX_BP_ON(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000088ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_RX_BP_ON(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000088ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_RX_BP_STATUS(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_RX_BP_STATUS(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E00000A8ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_RX_BP_STATUS(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E00000A8ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_RX_FIFO_LEN(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_RX_FIFO_LEN(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E00000C0ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_RX_FIFO_LEN(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E00000C0ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_RX_ID_MAP(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_RX_ID_MAP(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000028ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_RX_ID_MAP(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000028ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_RX_LOGL_XOFF(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_RX_LOGL_XOFF(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E00000B0ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_RX_LOGL_XOFF(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E00000B0ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_RX_LOGL_XON(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_RX_LOGL_XON(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E00000B8ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_RX_LOGL_XON(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E00000B8ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_RX_PAUSE_DROP_TIME(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_RX_PAUSE_DROP_TIME(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000030ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_RX_PAUSE_DROP_TIME(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000030ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_RX_STAT0(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_RX_STAT0(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000038ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_RX_STAT0(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000038ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_RX_STAT1(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_RX_STAT1(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000040ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_RX_STAT1(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000040ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_RX_STAT2(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_RX_STAT2(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000048ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_RX_STAT2(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000048ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_RX_STAT3(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_RX_STAT3(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000050ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_RX_STAT3(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000050ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_RX_STAT4(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_RX_STAT4(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000058ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_RX_STAT4(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000058ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_RX_STAT5(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_RX_STAT5(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000060ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_RX_STAT5(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000060ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_RX_STAT6(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_RX_STAT6(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000068ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_RX_STAT6(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000068ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_RX_STAT7(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_RX_STAT7(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000070ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_RX_STAT7(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000070ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_RX_STAT8(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_RX_STAT8(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000078ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_RX_STAT8(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000078ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_RX_WEIGHT(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_RX_WEIGHT(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000098ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_RX_WEIGHT(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000098ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_TX_CHANNEL(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_TX_CHANNEL(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000400ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_TX_CHANNEL(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000400ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_TX_FIFO_LEN(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_TX_FIFO_LEN(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000418ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_TX_FIFO_LEN(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000418ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_TX_HG2_STATUS(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_TX_HG2_STATUS(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000410ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_TX_HG2_STATUS(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000410ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_TX_OVR_BP(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_TX_OVR_BP(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000420ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_TX_OVR_BP(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000420ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_TX_STAT0(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_TX_STAT0(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000508ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_TX_STAT0(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000508ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_TX_STAT1(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_TX_STAT1(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000510ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_TX_STAT1(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000510ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_TX_STAT10(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_TX_STAT10(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000558ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_TX_STAT10(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000558ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_TX_STAT11(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_TX_STAT11(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000560ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_TX_STAT11(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000560ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_TX_STAT12(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_TX_STAT12(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000568ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_TX_STAT12(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000568ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_TX_STAT13(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_TX_STAT13(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000570ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_TX_STAT13(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000570ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_TX_STAT14(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_TX_STAT14(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000578ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_TX_STAT14(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000578ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_TX_STAT15(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_TX_STAT15(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000580ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_TX_STAT15(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000580ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_TX_STAT16(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_TX_STAT16(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000588ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_TX_STAT16(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000588ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_TX_STAT17(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_TX_STAT17(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000590ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_TX_STAT17(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000590ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_TX_STAT2(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_TX_STAT2(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000518ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_TX_STAT2(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000518ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_TX_STAT3(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_TX_STAT3(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000520ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_TX_STAT3(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000520ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_TX_STAT4(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_TX_STAT4(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000528ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_TX_STAT4(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000528ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_TX_STAT5(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_TX_STAT5(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000530ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_TX_STAT5(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000530ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_TX_STAT6(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_TX_STAT6(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000538ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_TX_STAT6(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000538ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_TX_STAT7(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_TX_STAT7(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000540ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_TX_STAT7(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000540ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_TX_STAT8(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_TX_STAT8(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000548ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_TX_STAT8(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000548ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMRX_TX_STAT9(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMRX_TX_STAT9(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000550ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_CMRX_TX_STAT9(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000550ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMR_BAD(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 5)))))
		cvmx_warn("CVMX_BGXX_CMR_BAD(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0001020ull) + ((block_id) & 7) * 0x1000000ull;
}
#else
#define CVMX_BGXX_CMR_BAD(block_id) (CVMX_ADD_IO_SEG(0x00011800E0001020ull) + ((block_id) & 7) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMR_BIST_STATUS(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 5)))))
		cvmx_warn("CVMX_BGXX_CMR_BIST_STATUS(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000300ull) + ((block_id) & 7) * 0x1000000ull;
}
#else
#define CVMX_BGXX_CMR_BIST_STATUS(block_id) (CVMX_ADD_IO_SEG(0x00011800E0000300ull) + ((block_id) & 7) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMR_CHAN_MSK_AND(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 5)))))
		cvmx_warn("CVMX_BGXX_CMR_CHAN_MSK_AND(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000200ull) + ((block_id) & 7) * 0x1000000ull;
}
#else
#define CVMX_BGXX_CMR_CHAN_MSK_AND(block_id) (CVMX_ADD_IO_SEG(0x00011800E0000200ull) + ((block_id) & 7) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMR_CHAN_MSK_OR(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 5)))))
		cvmx_warn("CVMX_BGXX_CMR_CHAN_MSK_OR(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000208ull) + ((block_id) & 7) * 0x1000000ull;
}
#else
#define CVMX_BGXX_CMR_CHAN_MSK_OR(block_id) (CVMX_ADD_IO_SEG(0x00011800E0000208ull) + ((block_id) & 7) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMR_GLOBAL_CONFIG(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 5)))))
		cvmx_warn("CVMX_BGXX_CMR_GLOBAL_CONFIG(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000008ull) + ((block_id) & 7) * 0x1000000ull;
}
#else
#define CVMX_BGXX_CMR_GLOBAL_CONFIG(block_id) (CVMX_ADD_IO_SEG(0x00011800E0000008ull) + ((block_id) & 7) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMR_MEM_CTRL(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 5)))))
		cvmx_warn("CVMX_BGXX_CMR_MEM_CTRL(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000018ull) + ((block_id) & 7) * 0x1000000ull;
}
#else
#define CVMX_BGXX_CMR_MEM_CTRL(block_id) (CVMX_ADD_IO_SEG(0x00011800E0000018ull) + ((block_id) & 7) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMR_MEM_INT(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 5)))))
		cvmx_warn("CVMX_BGXX_CMR_MEM_INT(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000010ull) + ((block_id) & 7) * 0x1000000ull;
}
#else
#define CVMX_BGXX_CMR_MEM_INT(block_id) (CVMX_ADD_IO_SEG(0x00011800E0000010ull) + ((block_id) & 7) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMR_NXC_ADR(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 5)))))
		cvmx_warn("CVMX_BGXX_CMR_NXC_ADR(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0001018ull) + ((block_id) & 7) * 0x1000000ull;
}
#else
#define CVMX_BGXX_CMR_NXC_ADR(block_id) (CVMX_ADD_IO_SEG(0x00011800E0001018ull) + ((block_id) & 7) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMR_RX_ADRX_CAM(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 31)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_CMR_RX_ADRX_CAM(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000100ull) + (((offset) & 31) + ((block_id) & 7) * 0x200000ull) * 8;
}
#else
#define CVMX_BGXX_CMR_RX_ADRX_CAM(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0000100ull) + (((offset) & 31) + ((block_id) & 7) * 0x200000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMR_RX_LMACS(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 5)))))
		cvmx_warn("CVMX_BGXX_CMR_RX_LMACS(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000308ull) + ((block_id) & 7) * 0x1000000ull;
}
#else
#define CVMX_BGXX_CMR_RX_LMACS(block_id) (CVMX_ADD_IO_SEG(0x00011800E0000308ull) + ((block_id) & 7) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMR_RX_OVR_BP(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 5)))))
		cvmx_warn("CVMX_BGXX_CMR_RX_OVR_BP(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0000318ull) + ((block_id) & 7) * 0x1000000ull;
}
#else
#define CVMX_BGXX_CMR_RX_OVR_BP(block_id) (CVMX_ADD_IO_SEG(0x00011800E0000318ull) + ((block_id) & 7) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_CMR_TX_LMACS(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 5)))))
		cvmx_warn("CVMX_BGXX_CMR_TX_LMACS(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0001000ull) + ((block_id) & 7) * 0x1000000ull;
}
#else
#define CVMX_BGXX_CMR_TX_LMACS(block_id) (CVMX_ADD_IO_SEG(0x00011800E0001000ull) + ((block_id) & 7) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_PRTX_CFG(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_PRTX_CFG(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0038010ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_GMI_PRTX_CFG(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0038010ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_RXX_DECISION(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_RXX_DECISION(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0038040ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_GMI_RXX_DECISION(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0038040ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_RXX_FRM_CHK(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_RXX_FRM_CHK(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0038020ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_GMI_RXX_FRM_CHK(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0038020ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_RXX_FRM_CTL(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_RXX_FRM_CTL(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0038018ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_GMI_RXX_FRM_CTL(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0038018ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_RXX_IFG(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_RXX_IFG(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0038058ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_GMI_RXX_IFG(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0038058ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_RXX_INT(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_RXX_INT(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0038000ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_GMI_RXX_INT(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0038000ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_RXX_JABBER(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_RXX_JABBER(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0038038ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_GMI_RXX_JABBER(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0038038ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_RXX_UDD_SKP(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_RXX_UDD_SKP(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0038048ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_GMI_RXX_UDD_SKP(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0038048ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_SMACX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_SMACX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0038230ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_GMI_SMACX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0038230ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_TXX_APPEND(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_TXX_APPEND(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0038218ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_GMI_TXX_APPEND(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0038218ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_TXX_BURST(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_TXX_BURST(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0038228ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_GMI_TXX_BURST(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0038228ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_TXX_CTL(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_TXX_CTL(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0038270ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_GMI_TXX_CTL(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0038270ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_TXX_INT(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_TXX_INT(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0038500ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_GMI_TXX_INT(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0038500ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_TXX_MIN_PKT(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_TXX_MIN_PKT(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0038240ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_GMI_TXX_MIN_PKT(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0038240ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_TXX_PAUSE_PKT_INTERVAL(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_TXX_PAUSE_PKT_INTERVAL(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0038248ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_GMI_TXX_PAUSE_PKT_INTERVAL(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0038248ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_TXX_PAUSE_PKT_TIME(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_TXX_PAUSE_PKT_TIME(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0038238ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_GMI_TXX_PAUSE_PKT_TIME(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0038238ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_TXX_PAUSE_TOGO(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_TXX_PAUSE_TOGO(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0038258ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_GMI_TXX_PAUSE_TOGO(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0038258ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_TXX_PAUSE_ZERO(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_TXX_PAUSE_ZERO(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0038260ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_GMI_TXX_PAUSE_ZERO(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0038260ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_TXX_SGMII_CTL(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_TXX_SGMII_CTL(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0038300ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_GMI_TXX_SGMII_CTL(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0038300ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_TXX_SLOT(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_TXX_SLOT(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0038220ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_GMI_TXX_SLOT(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0038220ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_TXX_SOFT_PAUSE(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_TXX_SOFT_PAUSE(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0038250ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_GMI_TXX_SOFT_PAUSE(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0038250ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_TXX_THRESH(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_TXX_THRESH(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0038210ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_GMI_TXX_THRESH(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0038210ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_TX_COL_ATTEMPT(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 5)))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_TX_COL_ATTEMPT(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0039010ull) + ((block_id) & 7) * 0x1000000ull;
}
#else
#define CVMX_BGXX_GMP_GMI_TX_COL_ATTEMPT(block_id) (CVMX_ADD_IO_SEG(0x00011800E0039010ull) + ((block_id) & 7) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_TX_IFG(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 5)))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_TX_IFG(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0039000ull) + ((block_id) & 7) * 0x1000000ull;
}
#else
#define CVMX_BGXX_GMP_GMI_TX_IFG(block_id) (CVMX_ADD_IO_SEG(0x00011800E0039000ull) + ((block_id) & 7) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_TX_JAM(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 5)))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_TX_JAM(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0039008ull) + ((block_id) & 7) * 0x1000000ull;
}
#else
#define CVMX_BGXX_GMP_GMI_TX_JAM(block_id) (CVMX_ADD_IO_SEG(0x00011800E0039008ull) + ((block_id) & 7) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_TX_LFSR(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 5)))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_TX_LFSR(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0039028ull) + ((block_id) & 7) * 0x1000000ull;
}
#else
#define CVMX_BGXX_GMP_GMI_TX_LFSR(block_id) (CVMX_ADD_IO_SEG(0x00011800E0039028ull) + ((block_id) & 7) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_TX_PAUSE_PKT_DMAC(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 5)))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_TX_PAUSE_PKT_DMAC(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0039018ull) + ((block_id) & 7) * 0x1000000ull;
}
#else
#define CVMX_BGXX_GMP_GMI_TX_PAUSE_PKT_DMAC(block_id) (CVMX_ADD_IO_SEG(0x00011800E0039018ull) + ((block_id) & 7) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_GMI_TX_PAUSE_PKT_TYPE(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 5)))))
		cvmx_warn("CVMX_BGXX_GMP_GMI_TX_PAUSE_PKT_TYPE(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0039020ull) + ((block_id) & 7) * 0x1000000ull;
}
#else
#define CVMX_BGXX_GMP_GMI_TX_PAUSE_PKT_TYPE(block_id) (CVMX_ADD_IO_SEG(0x00011800E0039020ull) + ((block_id) & 7) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_PCS_ANX_ADV(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_PCS_ANX_ADV(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0030010ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_PCS_ANX_ADV(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0030010ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_PCS_ANX_EXT_ST(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_PCS_ANX_EXT_ST(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0030028ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_PCS_ANX_EXT_ST(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0030028ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_PCS_ANX_LP_ABIL(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_PCS_ANX_LP_ABIL(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0030018ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_PCS_ANX_LP_ABIL(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0030018ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_PCS_ANX_RESULTS(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_PCS_ANX_RESULTS(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0030020ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_PCS_ANX_RESULTS(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0030020ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_PCS_INTX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_PCS_INTX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0030080ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_PCS_INTX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0030080ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_PCS_LINKX_TIMER(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_PCS_LINKX_TIMER(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0030040ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_PCS_LINKX_TIMER(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0030040ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_PCS_MISCX_CTL(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_PCS_MISCX_CTL(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0030078ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_PCS_MISCX_CTL(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0030078ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_PCS_MRX_CONTROL(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_PCS_MRX_CONTROL(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0030000ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_PCS_MRX_CONTROL(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0030000ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_PCS_MRX_STATUS(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_PCS_MRX_STATUS(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0030008ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_PCS_MRX_STATUS(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0030008ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_PCS_RXX_STATES(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_PCS_RXX_STATES(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0030058ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_PCS_RXX_STATES(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0030058ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_PCS_RXX_SYNC(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_PCS_RXX_SYNC(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0030050ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_PCS_RXX_SYNC(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0030050ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_PCS_SGMX_AN_ADV(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_PCS_SGMX_AN_ADV(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0030068ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_PCS_SGMX_AN_ADV(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0030068ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_PCS_SGMX_LP_ADV(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_PCS_SGMX_LP_ADV(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0030070ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_PCS_SGMX_LP_ADV(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0030070ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_PCS_TXX_STATES(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_PCS_TXX_STATES(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0030060ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_PCS_TXX_STATES(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0030060ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_GMP_PCS_TX_RXX_POLARITY(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_GMP_PCS_TX_RXX_POLARITY(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0030048ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_GMP_PCS_TX_RXX_POLARITY(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0030048ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_CBFC_CTL(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_CBFC_CTL(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020218ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_CBFC_CTL(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020218ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_CTRL(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_CTRL(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020200ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_CTRL(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020200ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_EXT_LOOPBACK(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_EXT_LOOPBACK(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020208ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_EXT_LOOPBACK(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020208ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_HG2_CONTROL(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_HG2_CONTROL(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020210ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_HG2_CONTROL(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020210ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_RX_BAD_COL_HI(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_RX_BAD_COL_HI(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020040ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_RX_BAD_COL_HI(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020040ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_RX_BAD_COL_LO(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_RX_BAD_COL_LO(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020038ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_RX_BAD_COL_LO(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020038ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_RX_CTL(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_RX_CTL(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020030ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_RX_CTL(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020030ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_RX_DECISION(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_RX_DECISION(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020020ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_RX_DECISION(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020020ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_RX_FRM_CHK(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_RX_FRM_CHK(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020010ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_RX_FRM_CHK(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020010ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_RX_FRM_CTL(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_RX_FRM_CTL(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020008ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_RX_FRM_CTL(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020008ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_RX_INT(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_RX_INT(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020000ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_RX_INT(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020000ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_RX_JABBER(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_RX_JABBER(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020018ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_RX_JABBER(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020018ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_RX_UDD_SKP(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_RX_UDD_SKP(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020028ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_RX_UDD_SKP(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020028ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_SMAC(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_SMAC(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020108ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_SMAC(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020108ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_TX_APPEND(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_TX_APPEND(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020100ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_TX_APPEND(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020100ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_TX_CTL(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_TX_CTL(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020160ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_TX_CTL(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020160ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_TX_IFG(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_TX_IFG(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020148ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_TX_IFG(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020148ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_TX_INT(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_TX_INT(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020140ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_TX_INT(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020140ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_TX_MIN_PKT(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_TX_MIN_PKT(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020118ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_TX_MIN_PKT(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020118ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_TX_PAUSE_PKT_DMAC(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_TX_PAUSE_PKT_DMAC(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020150ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_TX_PAUSE_PKT_DMAC(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020150ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_TX_PAUSE_PKT_INTERVAL(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_TX_PAUSE_PKT_INTERVAL(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020120ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_TX_PAUSE_PKT_INTERVAL(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020120ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_TX_PAUSE_PKT_TIME(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_TX_PAUSE_PKT_TIME(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020110ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_TX_PAUSE_PKT_TIME(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020110ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_TX_PAUSE_PKT_TYPE(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_TX_PAUSE_PKT_TYPE(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020158ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_TX_PAUSE_PKT_TYPE(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020158ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_TX_PAUSE_TOGO(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_TX_PAUSE_TOGO(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020130ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_TX_PAUSE_TOGO(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020130ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_TX_PAUSE_ZERO(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_TX_PAUSE_ZERO(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020138ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_TX_PAUSE_ZERO(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020138ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_TX_SOFT_PAUSE(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_TX_SOFT_PAUSE(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020128ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_TX_SOFT_PAUSE(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020128ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SMUX_TX_THRESH(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SMUX_TX_THRESH(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0020168ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SMUX_TX_THRESH(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0020168ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_AN_ADV(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_AN_ADV(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E00100D8ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_AN_ADV(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E00100D8ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_AN_BP_STATUS(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_AN_BP_STATUS(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E00100F8ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_AN_BP_STATUS(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E00100F8ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_AN_CONTROL(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_AN_CONTROL(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E00100C8ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_AN_CONTROL(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E00100C8ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_AN_LP_BASE(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_AN_LP_BASE(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E00100E0ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_AN_LP_BASE(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E00100E0ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_AN_LP_XNP(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_AN_LP_XNP(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E00100F0ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_AN_LP_XNP(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E00100F0ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_AN_STATUS(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_AN_STATUS(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E00100D0ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_AN_STATUS(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E00100D0ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_AN_XNP_TX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_AN_XNP_TX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E00100E8ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_AN_XNP_TX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E00100E8ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_BR_ALGN_STATUS(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_BR_ALGN_STATUS(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010050ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_BR_ALGN_STATUS(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010050ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_BR_BIP_ERR_CNT(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_BR_BIP_ERR_CNT(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010058ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_BR_BIP_ERR_CNT(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010058ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_BR_LANE_MAP(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_BR_LANE_MAP(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010060ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_BR_LANE_MAP(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010060ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_BR_PMD_CONTROL(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_BR_PMD_CONTROL(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010068ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_BR_PMD_CONTROL(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010068ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_BR_PMD_LD_CUP(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_BR_PMD_LD_CUP(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010088ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_BR_PMD_LD_CUP(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010088ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_BR_PMD_LD_REP(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_BR_PMD_LD_REP(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010090ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_BR_PMD_LD_REP(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010090ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_BR_PMD_LP_CUP(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_BR_PMD_LP_CUP(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010078ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_BR_PMD_LP_CUP(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010078ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_BR_PMD_LP_REP(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_BR_PMD_LP_REP(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010080ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_BR_PMD_LP_REP(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010080ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_BR_PMD_STATUS(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_BR_PMD_STATUS(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010070ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_BR_PMD_STATUS(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010070ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_BR_STATUS1(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_BR_STATUS1(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010030ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_BR_STATUS1(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010030ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_BR_STATUS2(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_BR_STATUS2(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010038ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_BR_STATUS2(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010038ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_BR_TP_CONTROL(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_BR_TP_CONTROL(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010040ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_BR_TP_CONTROL(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010040ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_BR_TP_ERR_CNT(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_BR_TP_ERR_CNT(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010048ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_BR_TP_ERR_CNT(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010048ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_BX_STATUS(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_BX_STATUS(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010028ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_BX_STATUS(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010028ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_CONTROL1(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_CONTROL1(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010000ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_CONTROL1(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010000ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_CONTROL2(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_CONTROL2(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010018ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_CONTROL2(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010018ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_FEC_ABIL(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_FEC_ABIL(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010098ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_FEC_ABIL(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010098ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_FEC_CONTROL(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_FEC_CONTROL(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E00100A0ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_FEC_CONTROL(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E00100A0ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_FEC_CORR_BLKS01(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_FEC_CORR_BLKS01(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E00100A8ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_FEC_CORR_BLKS01(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E00100A8ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_FEC_CORR_BLKS23(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_FEC_CORR_BLKS23(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E00100B0ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_FEC_CORR_BLKS23(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E00100B0ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_FEC_UNCORR_BLKS01(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_FEC_UNCORR_BLKS01(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E00100B8ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_FEC_UNCORR_BLKS01(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E00100B8ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_FEC_UNCORR_BLKS23(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_FEC_UNCORR_BLKS23(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E00100C0ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_FEC_UNCORR_BLKS23(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E00100C0ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_INT(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_INT(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010220ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_INT(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010220ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_LPCS_STATES(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_LPCS_STATES(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010208ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_LPCS_STATES(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010208ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_MISC_CONTROL(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_MISC_CONTROL(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010218ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_MISC_CONTROL(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010218ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_SPD_ABIL(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_SPD_ABIL(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010010ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_SPD_ABIL(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010010ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_STATUS1(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_STATUS1(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010008ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_STATUS1(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010008ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPUX_STATUS2(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPUX_STATUS2(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010020ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576;
}
#else
#define CVMX_BGXX_SPUX_STATUS2(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010020ull) + (((offset) & 3) + ((block_id) & 7) * 0x10ull) * 1048576)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPU_BIST_STATUS(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 5)))))
		cvmx_warn("CVMX_BGXX_SPU_BIST_STATUS(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010318ull) + ((block_id) & 7) * 0x1000000ull;
}
#else
#define CVMX_BGXX_SPU_BIST_STATUS(block_id) (CVMX_ADD_IO_SEG(0x00011800E0010318ull) + ((block_id) & 7) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPU_DBG_CONTROL(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 5)))))
		cvmx_warn("CVMX_BGXX_SPU_DBG_CONTROL(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010300ull) + ((block_id) & 7) * 0x1000000ull;
}
#else
#define CVMX_BGXX_SPU_DBG_CONTROL(block_id) (CVMX_ADD_IO_SEG(0x00011800E0010300ull) + ((block_id) & 7) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPU_MEM_INT(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 5)))))
		cvmx_warn("CVMX_BGXX_SPU_MEM_INT(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010310ull) + ((block_id) & 7) * 0x1000000ull;
}
#else
#define CVMX_BGXX_SPU_MEM_INT(block_id) (CVMX_ADD_IO_SEG(0x00011800E0010310ull) + ((block_id) & 7) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPU_MEM_STATUS(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 5)))))
		cvmx_warn("CVMX_BGXX_SPU_MEM_STATUS(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010308ull) + ((block_id) & 7) * 0x1000000ull;
}
#else
#define CVMX_BGXX_SPU_MEM_STATUS(block_id) (CVMX_ADD_IO_SEG(0x00011800E0010308ull) + ((block_id) & 7) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPU_SDSX_SKEW_STATUS(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPU_SDSX_SKEW_STATUS(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010320ull) + (((offset) & 3) + ((block_id) & 7) * 0x200000ull) * 8;
}
#else
#define CVMX_BGXX_SPU_SDSX_SKEW_STATUS(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010320ull) + (((offset) & 3) + ((block_id) & 7) * 0x200000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_BGXX_SPU_SDSX_STATES(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 5))))))
		cvmx_warn("CVMX_BGXX_SPU_SDSX_STATES(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800E0010340ull) + (((offset) & 3) + ((block_id) & 7) * 0x200000ull) * 8;
}
#else
#define CVMX_BGXX_SPU_SDSX_STATES(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800E0010340ull) + (((offset) & 3) + ((block_id) & 7) * 0x200000ull) * 8)
#endif

/**
 * cvmx_bgx#_cmr#_config
 *
 * Logical MAC/PCS configuration registers; one per LMAC. The maximum number of LMACs (and
 * maximum LMAC ID) that can be enabled by these registers is limited by
 * BGX(0..5)_CMR_RX_LMACS[LMACS] and BGX(0..5)_CMR_TX_LMACS[LMACS]. When multiple LMACs are
 * enabled, they must be configured with the same [LMAC_TYPE] value.
 * Typical configurations:
 *   ---------------------------------------------------------------------------
 *   Configuration           LMACS  Register             [ENABLE]    [LMAC_TYPE]
 *   ---------------------------------------------------------------------------
 *   1x40GBASE-R4            1      BGXn_CMR0_CONFIG     1           4
 *                                  BGXn_CMR1_CONFIG     0           --
 *                                  BGXn_CMR2_CONFIG     0           --
 *                                  BGXn_CMR3_CONFIG     0           --
 *   ---------------------------------------------------------------------------
 *   4x10GBASE-R             4      BGXn_CMR0_CONFIG     1           3
 *                                  BGXn_CMR1_CONFIG     1           3
 *                                  BGXn_CMR2_CONFIG     1           3
 *                                  BGXn_CMR3_CONFIG     1           3
 *   ---------------------------------------------------------------------------
 *   2xRXAUI                 2      BGXn_CMR0_CONFIG     1           2
 *                                  BGXn_CMR1_CONFIG     1           2
 *                                  BGXn_CMR2_CONFIG     0           --
 *                                  BGXn_CMR3_CONFIG     0           --
 *   ---------------------------------------------------------------------------
 *   1x10GBASE-X/XAUI/DXAUI  1      BGXn_CMR0_CONFIG     1           1
 *                                  BGXn_CMR1_CONFIG     0           --
 *                                  BGXn_CMR2_CONFIG     0           --
 *                                  BGXn_CMR3_CONFIG     0           --
 *   ---------------------------------------------------------------------------
 *   4xSGMII/1000BASE-X      4      BGXn_CMR0_CONFIG     1           0
 *                                  BGXn_CMR1_CONFIG     1           0
 *                                  BGXn_CMR2_CONFIG     1           0
 *                                  BGXn_CMR3_CONFIG     1           0
 *   ---------------------------------------------------------------------------
 */
union cvmx_bgxx_cmrx_config {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_config_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t enable                       : 1;  /**< Logical MAC/PCS Enable. This is the master enable for the LMAC. When clear, all the
                                                         dedicated BGX context state for the LMAC (state machines, FIFOs, counters, etc.) is reset,
                                                         and LMAC access to shared BGX resources (SMU/SPU data path, serdes lanes) is disabled.
                                                         When set, LMAC operation is enabled, including link bring-up, synchronization, and
                                                         transmit/receive of of idles and fault sequences. Note that configuration registers for an
                                                         LMAC are not reset when this bit is clear, allowing software to program them before
                                                         setting this bit to enable the LMAC.  This bit together with the LMAC_TYPE is also used
                                                         to enable the clocking to the GMP and/or blocks of the Super path(SMU and SPU).
                                                         CMR clocking will be enable when any of the paths are enabled. */
	uint64_t data_pkt_rx_en               : 1;  /**< Data Packet Receive Enable When ENABLE=1 and DATA_PKT_RX_EN=1, the reception of data
                                                         packets is enabled in the MAC layer. When ENABLE=1 and DATA_PKT_RX_EN=0, the MAC layer
                                                         drops received data and flow control packets. */
	uint64_t data_pkt_tx_en               : 1;  /**< Data Packet Transmit Enable When ENABLE=1 and DATA_PKT_TX_EN=1, the transmission of data
                                                         packets is enabled in the MAC layer. When ENABLE=1 and DATA_PKT_TX_EN=0, the MAC layer
                                                         suppresses the transmission of new data and packets for the LMAC */
	uint64_t int_beat_gen                 : 1;  /**< Internal Beat Generation This bit is used for debug/test purposes and should be clear
                                                         during normal operation. When set, the LMAC's PCS layer ignores RXVALID and
                                                         TXREADY/TXCREDIT from the associated serdes lane(s), internally generates fake (idle)
                                                         RXVALID and TXCREDIT pulses, and suppresses transmission to the serdes */
	uint64_t mix_en                       : 1;  /**< Managemenet enable. This bit is used by LMACs 0 and 1 only, and should be kept clear for
                                                         LMACs 2 and 3. Setting it will pipe the LMAC to and from the MIX inteface (LMAC0 to/from
                                                         MIX0, LMAC1 to/from MIX1). LMAC_TYPE must be 0 (SGMII) then this bit is set. Note that at
                                                         most one BGX can be attached to each of MIX0 and MIX1, i.e. at most one
                                                         BGX(0..5)_CMR0_CONFIG[MIX_EN] bit and one BGX(0..5)_CMR1_CONFIG[MIX_EN] bit can be set. */
	uint64_t lmac_type                    : 3;  /**< Logical MAC/PCS Type:
                                                           ----------+----------------------------------------------------------
                                                           LMAC_TYPE | Name         Description                NUM_PCS_LANES
                                                           ----------+----------------------------------------------------------
                                                           0         | SGMII      SGMII/1000BASE-X             1
                                                           1         | XAUI       10GBASE-X/XAUI or DXAUI      4
                                                           2         | RXAUI      Reduced XAUI                 2
                                                           3         | 10G_R      10GBASE-R                    1
                                                           4         | 40G_R      40GBASE-R                    4
                                                           Other     | -          Reserved                     -
                                                           ----------+----------------------------------------------------------
                                                         NUM_PCS_LANES specifies the number of of PCS lanes that are valid for
                                                         each type. Each valid PCS lane is mapped to a physical serdes lane
                                                         based on the programming of [LANE_TO_SDS]. */
	uint64_t lane_to_sds                  : 8;  /**< PCS Lane to Serdes Mapping.
                                                         This is an array of 2-bit values that map each logical PCS Lane to a
                                                         physical serdes lane, as follows:
                                                           ----------+----------------------------------------------------------
                                                           Bits     | Description                     Reset value
                                                           ----------+----------------------------------------------------------
                                                           <7:6>    | PCS Lane 3 Serdes ID            0x3
                                                           <5:4>    | PCS Lane 2 Serdes ID            0x2
                                                           <3:2>    | PCS Lane 1 Serdes ID            0x1
                                                           <1:0>    | PCS Lane 0 Serdes ID            0x0
                                                           ----------+----------------------------------------------------------
                                                         PCS lanes 0 through NUM_PCS_LANES-1 are valid, where NUM_PCS_LANES is
                                                         a function of the logical MAC/PCS type (see definition of
                                                         LMAC_TYPE field in this register). For example, when LMAC_TYPE =
                                                         RXAUI, then NUM_PCS_LANES = 2, PCS lanes 0 and 1 valid and the
                                                         associated physical serdes lanes are selected by bits <1:0> and
                                                         <3:2>, respectively.
                                                         For 40GBASE-R (LMAC_TYPE = 40G_R), all four PCS lanes are valid, and
                                                         the PCS lane IDs determine the block distribution order and
                                                         associated alignment markers on the *transmit* side. This is not
                                                         necessarily the order in which PCS lanes *receive* data because 802.3
                                                         allows multi-lane BASE-R receive lanes to be re-ordered. When a
                                                         lane (called 'service interface' in 802.3ba-2010) has achieved
                                                         alignment marker lock on the receive side (i.e. the associated
                                                         MARKER_LOCK bit is set in BR_ALGN_STATUS), then the actual detected
                                                         RX PCS lane number is recorded in the corresponding LNx_MAPPING
                                                         field in BR_LANE_MAP. */
#else
	uint64_t lane_to_sds                  : 8;
	uint64_t lmac_type                    : 3;
	uint64_t mix_en                       : 1;
	uint64_t int_beat_gen                 : 1;
	uint64_t data_pkt_tx_en               : 1;
	uint64_t data_pkt_rx_en               : 1;
	uint64_t enable                       : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_cmrx_config_s        cn78xx;
};
typedef union cvmx_bgxx_cmrx_config cvmx_bgxx_cmrx_config_t;

/**
 * cvmx_bgx#_cmr#_int
 */
union cvmx_bgxx_cmrx_int {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t pko_nxc                      : 1;  /**< TX channel out-of-range from PKO interface. Assigned to the LMAC ID based on the lower 2
                                                         bits of the offending channel */
	uint64_t overflw                      : 1;  /**< RX overflow. */
	uint64_t pause_drp                    : 1;  /**< RX PAUSE packet was dropped due to full RXB FIFO or during partner reset. */
#else
	uint64_t pause_drp                    : 1;
	uint64_t overflw                      : 1;
	uint64_t pko_nxc                      : 1;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_bgxx_cmrx_int_s           cn78xx;
};
typedef union cvmx_bgxx_cmrx_int cvmx_bgxx_cmrx_int_t;

/**
 * cvmx_bgx#_cmr#_prt_cbfc_ctl
 *
 * See XOFF definition listed under BGX(0..5)_SMU(0..3)_CBFC_CTL.
 *
 */
union cvmx_bgxx_cmrx_prt_cbfc_ctl {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_prt_cbfc_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t phys_bp                      : 16; /**< When BGXn_SMUm_CBFC_CTL[RX_EN] is set and the hardware is backpressuring any LMACs (from
                                                         either PFC/CBFC PAUSE packets or BGXn_CMR_TX_OVR_BP[TX_CHAN_BP]) and all LMACs indicated
                                                         by PHYS_BP are backpressured, simulate physical backpressure by deferring all packets on
                                                         the transmitter. */
	uint64_t reserved_0_15                : 16;
#else
	uint64_t reserved_0_15                : 16;
	uint64_t phys_bp                      : 16;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_bgxx_cmrx_prt_cbfc_ctl_s  cn78xx;
};
typedef union cvmx_bgxx_cmrx_prt_cbfc_ctl cvmx_bgxx_cmrx_prt_cbfc_ctl_t;

/**
 * cvmx_bgx#_cmr#_rx_adr_ctl
 */
union cvmx_bgxx_cmrx_rx_adr_ctl {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_rx_adr_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t cam_accept                   : 1;  /**< Allow or deny DMAC address filter.
                                                         0 = Reject the packet on DMAC CAM address match
                                                         1 = Accept the packet on DMAC CAM address match */
	uint64_t mcst_mode                    : 2;  /**< Multicast mode.
                                                         0x0 = Force reject all multicast packets
                                                         0x1 = Force accept all multicast packets
                                                         0x2 = Use the address filter CAM
                                                         0x3 = Reserved */
	uint64_t bcst_accept                  : 1;  /**< Allow or deny broadcast packets.
                                                         0 = Reject all broadcast packets
                                                         1 = Accept all broadcast Packets */
#else
	uint64_t bcst_accept                  : 1;
	uint64_t mcst_mode                    : 2;
	uint64_t cam_accept                   : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_bgxx_cmrx_rx_adr_ctl_s    cn78xx;
};
typedef union cvmx_bgxx_cmrx_rx_adr_ctl cvmx_bgxx_cmrx_rx_adr_ctl_t;

/**
 * cvmx_bgx#_cmr#_rx_bp_drop
 */
union cvmx_bgxx_cmrx_rx_bp_drop {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_rx_bp_drop_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t mark                         : 7;  /**< Number of eight-byte cycles to reserve in the RX FIFO. When the number of free
                                                         entries in the RX FIFO is less than or equal to MARK, incoming packet data is
                                                         dropped. Mark additionally indicates the number of entries to reserve in the RX FIFO for
                                                         closing partially received packets. MARK should typically be programmed to its reset
                                                         value; failure to program correctly can lead to system instability. */
#else
	uint64_t mark                         : 7;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_bgxx_cmrx_rx_bp_drop_s    cn78xx;
};
typedef union cvmx_bgxx_cmrx_rx_bp_drop cvmx_bgxx_cmrx_rx_bp_drop_t;

/**
 * cvmx_bgx#_cmr#_rx_bp_off
 */
union cvmx_bgxx_cmrx_rx_bp_off {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_rx_bp_off_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t mark                         : 7;  /**< Low watermark (number of eight-byte cycles to deassert backpressure). Level is also used
                                                         to exit the overflow dropping state. */
#else
	uint64_t mark                         : 7;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_bgxx_cmrx_rx_bp_off_s     cn78xx;
};
typedef union cvmx_bgxx_cmrx_rx_bp_off cvmx_bgxx_cmrx_rx_bp_off_t;

/**
 * cvmx_bgx#_cmr#_rx_bp_on
 */
union cvmx_bgxx_cmrx_rx_bp_on {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_rx_bp_on_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t mark                         : 12; /**< High watermark (number of eight-byte cycles to assert backpressure). Each register is for
                                                         an individual LMAC. MARK must satisfy:
                                                         BGX(0..5)_CMR(0..3)_RX_BP_OFF[MARK] <= MARK <
                                                         (FIFO_SIZE - BGX(0..5)_CMR(0..3)_RX_BP_DROP[MARK]).
                                                         A value of 0x0 immediately asserts backpressure.
                                                         The recommended value is 1/4th the size of the per-LMAC RX FIFO_SIZE as determined by
                                                         GX_CMR_RX_LMACS[LMACS]. For example in SGMII mode with four LMACs of type SGMII where
                                                         BGX*_CMR*_RX_LMACS[LMACS]=0x4, MARK = 0x100 (the reset value. */
#else
	uint64_t mark                         : 12;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_bgxx_cmrx_rx_bp_on_s      cn78xx;
};
typedef union cvmx_bgxx_cmrx_rx_bp_on cvmx_bgxx_cmrx_rx_bp_on_t;

/**
 * cvmx_bgx#_cmr#_rx_bp_status
 */
union cvmx_bgxx_cmrx_rx_bp_status {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_rx_bp_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t bp                           : 1;  /**< Per-LMAC backpressure status.
                                                         0 = LMAC is not backpressured
                                                         1 = LMAC is backpressured */
#else
	uint64_t bp                           : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_bgxx_cmrx_rx_bp_status_s  cn78xx;
};
typedef union cvmx_bgxx_cmrx_rx_bp_status cvmx_bgxx_cmrx_rx_bp_status_t;

/**
 * cvmx_bgx#_cmr#_rx_fifo_len
 */
union cvmx_bgxx_cmrx_rx_fifo_len {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_rx_fifo_len_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_13_63               : 51;
	uint64_t fifo_len                     : 13; /**< Per-LMAC FIFO length. Useful for determining if FIFO is empty when bringing an LMAC down. */
#else
	uint64_t fifo_len                     : 13;
	uint64_t reserved_13_63               : 51;
#endif
	} s;
	struct cvmx_bgxx_cmrx_rx_fifo_len_s   cn78xx;
};
typedef union cvmx_bgxx_cmrx_rx_fifo_len cvmx_bgxx_cmrx_rx_fifo_len_t;

/**
 * cvmx_bgx#_cmr#_rx_id_map
 *
 * These registers set the RX LMAC ID mapping for X2P/PKI.
 *
 */
union cvmx_bgxx_cmrx_rx_id_map {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_rx_id_map_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t rid                          : 7;  /**< Reassembly ID map for this LMAC. A shared pool of 96 reassembly IDs (RIDs) exists for all
                                                         MACs. See description of RIDs in .
                                                         The RID for this LMAC must be constrained such that it does not overlap with any other MAC
                                                         in the system. Its reset value has been chosen such that this condition is satisfied:
                                                         RID reset value = 4*(BGX_ID + 1) + LMAC_ID
                                                         Changes to RID must only occur when the LMAC is quiescent (i.e. the LMAC receive interface
                                                         is down and the RX FIFO is empty). */
	uint64_t pknd                         : 8;  /**< Port kind for this LMAC */
#else
	uint64_t pknd                         : 8;
	uint64_t rid                          : 7;
	uint64_t reserved_15_63               : 49;
#endif
	} s;
	struct cvmx_bgxx_cmrx_rx_id_map_s     cn78xx;
};
typedef union cvmx_bgxx_cmrx_rx_id_map cvmx_bgxx_cmrx_rx_id_map_t;

/**
 * cvmx_bgx#_cmr#_rx_logl_xoff
 */
union cvmx_bgxx_cmrx_rx_logl_xoff {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_rx_logl_xoff_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t xoff                         : 16; /**< Together with BGX(0..5)_CMR(0..3)_RX_LOGL_XON, defines type of channel backpressure to
                                                         apply to the SMU. Do not write when HiGig2 is enabled. Writing 1 sets the same physical
                                                         register as that which is cleared by XON. An XOFF value of 1 will cause a backpressure on
                                                         SMU. */
#else
	uint64_t xoff                         : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_cmrx_rx_logl_xoff_s  cn78xx;
};
typedef union cvmx_bgxx_cmrx_rx_logl_xoff cvmx_bgxx_cmrx_rx_logl_xoff_t;

/**
 * cvmx_bgx#_cmr#_rx_logl_xon
 */
union cvmx_bgxx_cmrx_rx_logl_xon {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_rx_logl_xon_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t xon                          : 16; /**< Together with BGX(0..5)_CMR(0..3)_RX_LOGL_XOFF defines type of channel backpressure to
                                                         apply. Do not write when HiGig2 is enabled. Writing 1 clears the same physical register as
                                                         that which is set by XOFF. An XON value of 1 means only PKI channel BP can cause a
                                                         backpressure on SMU. */
#else
	uint64_t xon                          : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_cmrx_rx_logl_xon_s   cn78xx;
};
typedef union cvmx_bgxx_cmrx_rx_logl_xon cvmx_bgxx_cmrx_rx_logl_xon_t;

/**
 * cvmx_bgx#_cmr#_rx_pause_drop_time
 */
union cvmx_bgxx_cmrx_rx_pause_drop_time {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_rx_pause_drop_time_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t pause_time                   : 16; /**< Time extracted from the dropped PAUSE packet dropped due to RXB FIFO full or during partner reset */
#else
	uint64_t pause_time                   : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_cmrx_rx_pause_drop_time_s cn78xx;
};
typedef union cvmx_bgxx_cmrx_rx_pause_drop_time cvmx_bgxx_cmrx_rx_pause_drop_time_t;

/**
 * cvmx_bgx#_cmr#_rx_stat0
 *
 * These registers provide a count of received packets that meet the following conditions:
 * are not recognized as PAUSE packets
 * are not dropped due DMAC filtering
 * are not dropped due FIFO full status
 * do not have any other OPCODE (FCS, Length, etc).
 */
union cvmx_bgxx_cmrx_rx_stat0 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_rx_stat0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t cnt                          : 48; /**< Count of received packets. CNT will wrap and is cleared if LMAC is disabled with
                                                         BGX*_CMR*_CONFIG[ENABLE]=0. */
#else
	uint64_t cnt                          : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_rx_stat0_s      cn78xx;
};
typedef union cvmx_bgxx_cmrx_rx_stat0 cvmx_bgxx_cmrx_rx_stat0_t;

/**
 * cvmx_bgx#_cmr#_rx_stat1
 *
 * These registers provide a count of octets of received packets.
 *
 */
union cvmx_bgxx_cmrx_rx_stat1 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_rx_stat1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t cnt                          : 48; /**< Octet count of received packets. CNT will wrap and is cleared if LMAC is disabled with
                                                         BGX*_CMR*_CONFIG[ENABLE]=0. */
#else
	uint64_t cnt                          : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_rx_stat1_s      cn78xx;
};
typedef union cvmx_bgxx_cmrx_rx_stat1 cvmx_bgxx_cmrx_rx_stat1_t;

/**
 * cvmx_bgx#_cmr#_rx_stat2
 *
 * These registers provide a count of all packets received that were recognized as flow-control
 * or PAUSE packets. PAUSE packets with any kind of error are counted in BGX*_CMR*_RX_STAT8
 * (error stats register) and does not include those reported in BGX(0..5)_CMR(0..3)_RX_STAT6
 * nor BGX(0..5)_CMR(0..3)_RX_STAT4.
 * Pause packets can be optionally dropped or forwarded based on
 * BGX_SMU_RX_FRM_CTL[CTL_DRP]. This count increments regardless of whether the packet is
 * dropped. PAUSE packets are never counted in BGX*_CMR*_RX_STAT0.
 */
union cvmx_bgxx_cmrx_rx_stat2 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_rx_stat2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t cnt                          : 48; /**< Count of received PAUSE packets. CNT will wrap and is cleared if LMAC is disabled with
                                                         BGX*_CMR*_CONFIG[ENABLE]=0. */
#else
	uint64_t cnt                          : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_rx_stat2_s      cn78xx;
};
typedef union cvmx_bgxx_cmrx_rx_stat2 cvmx_bgxx_cmrx_rx_stat2_t;

/**
 * cvmx_bgx#_cmr#_rx_stat3
 *
 * These registers provide a count of octets of received PAUSE and control packets.
 *
 */
union cvmx_bgxx_cmrx_rx_stat3 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_rx_stat3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t cnt                          : 48; /**< Octet count of received PAUSE packets. CNT will wrap and is cleared if LMAC is disabled
                                                         with BGX*_CMR*_CONFIG[ENABLE]=0. */
#else
	uint64_t cnt                          : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_rx_stat3_s      cn78xx;
};
typedef union cvmx_bgxx_cmrx_rx_stat3 cvmx_bgxx_cmrx_rx_stat3_t;

/**
 * cvmx_bgx#_cmr#_rx_stat4
 *
 * These registers provide a count of all packets received that were dropped by the DMAC filter.
 * Packets that match the DMAC are dropped and counted here regardless of whether they were ERR
 * packets, but does not include those reported in BGX(0..5)_CMR(0..3)_RX_STAT6.
 * These packets are never counted in BGX*_CMR*_RX_STAT0. Eight-byte packets as the
 * result of truncation or other means are not be dropped by CN78XX and will never appear in this
 * count.
 */
union cvmx_bgxx_cmrx_rx_stat4 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_rx_stat4_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t cnt                          : 48; /**< Count of filtered DMAC packets. CNT will wrap and is cleared if LMAC is disabled with
                                                         BGX*_CMR*_CONFIG[ENABLE]=0. */
#else
	uint64_t cnt                          : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_rx_stat4_s      cn78xx;
};
typedef union cvmx_bgxx_cmrx_rx_stat4 cvmx_bgxx_cmrx_rx_stat4_t;

/**
 * cvmx_bgx#_cmr#_rx_stat5
 *
 * These registers provide a count of octets of filtered DMAC packets.
 *
 */
union cvmx_bgxx_cmrx_rx_stat5 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_rx_stat5_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t cnt                          : 48; /**< Octet count of filtered DMAC packets. CNT will wrap and is cleared if LMAC is disabled
                                                         with BGX*_CMR*_CONFIG[ENABLE]=0. */
#else
	uint64_t cnt                          : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_rx_stat5_s      cn78xx;
};
typedef union cvmx_bgxx_cmrx_rx_stat5 cvmx_bgxx_cmrx_rx_stat5_t;

/**
 * cvmx_bgx#_cmr#_rx_stat6
 *
 * These registers provide a count of all packets received that were dropped due to a full
 * receive FIFO. They do not count any packet that is truncated at the point at the point of
 * overflow and sent on to the PKI. These registers count all entire packets dropped by the FIFO
 * for a given LMAC regardless of DMAC or PAUSE type.
 */
union cvmx_bgxx_cmrx_rx_stat6 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_rx_stat6_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t cnt                          : 48; /**< Count of dropped packets. CNT will wrap and is cleared if LMAC is disabled with
                                                         BGX*_CMR*_CONFIG[ENABLE]=0. */
#else
	uint64_t cnt                          : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_rx_stat6_s      cn78xx;
};
typedef union cvmx_bgxx_cmrx_rx_stat6 cvmx_bgxx_cmrx_rx_stat6_t;

/**
 * cvmx_bgx#_cmr#_rx_stat7
 *
 * These registers provide a count of octets of received packets that were dropped due to a full
 * receive FIFO.
 */
union cvmx_bgxx_cmrx_rx_stat7 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_rx_stat7_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t cnt                          : 48; /**< Octet count of dropped packets. CNT will wrap and is cleared if LMAC is disabled with
                                                         BGX*_CMR*_CONFIG[ENABLE]=0. */
#else
	uint64_t cnt                          : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_rx_stat7_s      cn78xx;
};
typedef union cvmx_bgxx_cmrx_rx_stat7 cvmx_bgxx_cmrx_rx_stat7_t;

/**
 * cvmx_bgx#_cmr#_rx_stat8
 *
 * These registers provide a count of all packets received with some error that were not dropped
 * either due to the DMAC filter or lack of room in the receive FIFO.
 * This does not include packets which were counted in
 * BGX(0..5)_CMR(0..3)_RX_STAT2, BGX(0..5)_CMR(0..3)_RX_STAT4 nor
 * BGX(0..5)_CMR(0..3)_RX_STAT6 nor BGX(0..5)_CMR(0..3)_RX_STAT8.
 * Which statistics are updated on errors and drops are shown below:
 * if dropped [
 *   if !errored STAT8
 *   if overflow STAT6
 *   else if dmac drop STAT4
 *   else if filter drop STAT2
 * ] else [
 *   if errored STAT2
 *   else STAT8
 * ]
 */
union cvmx_bgxx_cmrx_rx_stat8 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_rx_stat8_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t cnt                          : 48; /**< Count of error packets. CNT will wrap and is cleared if LMAC is disabled with
                                                         BGX*_CMR*_CONFIG[ENABLE]=0. */
#else
	uint64_t cnt                          : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_rx_stat8_s      cn78xx;
};
typedef union cvmx_bgxx_cmrx_rx_stat8 cvmx_bgxx_cmrx_rx_stat8_t;

/**
 * cvmx_bgx#_cmr#_rx_weight
 */
union cvmx_bgxx_cmrx_rx_weight {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_rx_weight_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t weight                       : 4;  /**< For the weighted round robin algorithm in CMR RXB, weight to assign for this LMAC relative
                                                         to other LMAC weights. Defaults to round-robin (non-weighted minimum setting of 0x1). A
                                                         setting of 0x0 effectively takes the LMAC out of eligibility. */
#else
	uint64_t weight                       : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_bgxx_cmrx_rx_weight_s     cn78xx;
};
typedef union cvmx_bgxx_cmrx_rx_weight cvmx_bgxx_cmrx_rx_weight_t;

/**
 * cvmx_bgx#_cmr#_tx_channel
 */
union cvmx_bgxx_cmrx_tx_channel {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_tx_channel_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t msk                          : 16; /**< Backpressure channel mask. BGX can completely ignore the channel backpressure for channel
                                                         specified by this field. Any channel in which MSK == 1 never sends backpressure
                                                         information to PKO. */
	uint64_t dis                          : 16; /**< Credit return backpressure disable. BGX stops returning channel credits for any channel
                                                         that is backpressured. These bits can be used to override that. DIS == 1 allows channel
                                                         credits to flow back regardless of the backpressure for that channel. */
#else
	uint64_t dis                          : 16;
	uint64_t msk                          : 16;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_bgxx_cmrx_tx_channel_s    cn78xx;
};
typedef union cvmx_bgxx_cmrx_tx_channel cvmx_bgxx_cmrx_tx_channel_t;

/**
 * cvmx_bgx#_cmr#_tx_fifo_len
 */
union cvmx_bgxx_cmrx_tx_fifo_len {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_tx_fifo_len_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t lmac_idle                    : 1;  /**< Idle signal to identify when all credits and other pipeline buffers are also cleared out
                                                         and LMAC can be considered IDLE in the BGX CMR TX. */
	uint64_t fifo_len                     : 13; /**< Per-LMAC TXB main FIFO length. Useful for determining if main FIFO is empty when bringing
                                                         an LMAC down. */
#else
	uint64_t fifo_len                     : 13;
	uint64_t lmac_idle                    : 1;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_bgxx_cmrx_tx_fifo_len_s   cn78xx;
};
typedef union cvmx_bgxx_cmrx_tx_fifo_len cvmx_bgxx_cmrx_tx_fifo_len_t;

/**
 * cvmx_bgx#_cmr#_tx_hg2_status
 */
union cvmx_bgxx_cmrx_tx_hg2_status {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_tx_hg2_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t xof                          : 16; /**< 16-bit XOF back pressure vector from HiGig2 message packet or from PFC/CBFC packets. Non-
                                                         zero only when logical back pressure is active. All bits are 0 when LGTIM2GO=0x0. */
	uint64_t lgtim2go                     : 16; /**< Logical packet flow back pressure time remaining. Initial value set from XOF time field of
                                                         HiGig2 message packet received or a function of the enabled and current timers for
                                                         PFC/CBFC packets. Non-zero only when logical back pressure is active. */
#else
	uint64_t lgtim2go                     : 16;
	uint64_t xof                          : 16;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_bgxx_cmrx_tx_hg2_status_s cn78xx;
};
typedef union cvmx_bgxx_cmrx_tx_hg2_status cvmx_bgxx_cmrx_tx_hg2_status_t;

/**
 * cvmx_bgx#_cmr#_tx_ovr_bp
 */
union cvmx_bgxx_cmrx_tx_ovr_bp {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_tx_ovr_bp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t tx_chan_bp                   : 16; /**< Per-channel backpressure status sent to PKO.
                                                         1 = channel should be backpressured, 0 = channel is available */
#else
	uint64_t tx_chan_bp                   : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_cmrx_tx_ovr_bp_s     cn78xx;
};
typedef union cvmx_bgxx_cmrx_tx_ovr_bp cvmx_bgxx_cmrx_tx_ovr_bp_t;

/**
 * cvmx_bgx#_cmr#_tx_stat0
 */
union cvmx_bgxx_cmrx_tx_stat0 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_tx_stat0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t xscol                        : 48; /**< Number of packets dropped (never successfully sent) due to excessive collision. Defined by
                                                         BGX_GMP_GMI_TX_COL_ATTEMPT[LIMIT]. SGMII/1000Base-X half-duplex only.
                                                         Not cleared on read; cleared on a write with 0x0. Counters will wrap. Cleared if LMAC is
                                                         disabled with BGX_CMR_CONFIG[ENABLE]=0. */
#else
	uint64_t xscol                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_tx_stat0_s      cn78xx;
};
typedef union cvmx_bgxx_cmrx_tx_stat0 cvmx_bgxx_cmrx_tx_stat0_t;

/**
 * cvmx_bgx#_cmr#_tx_stat1
 */
union cvmx_bgxx_cmrx_tx_stat1 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_tx_stat1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t xsdef                        : 48; /**< Number of packets dropped (never successfully sent) due to excessive deferral.
                                                         SGMII/1000BASE-X half-duplex only.
                                                         Not cleared on read; cleared on a write with 0x0. Counters will wrap. Cleared if LMAC is
                                                         disabled with BGX_CMR_CONFIG[ENABLE]=0. */
#else
	uint64_t xsdef                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_tx_stat1_s      cn78xx;
};
typedef union cvmx_bgxx_cmrx_tx_stat1 cvmx_bgxx_cmrx_tx_stat1_t;

/**
 * cvmx_bgx#_cmr#_tx_stat10
 */
union cvmx_bgxx_cmrx_tx_stat10 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_tx_stat10_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t hist4                        : 48; /**< Number of packets sent with an octet count between 256-511. Packet length is the sum of
                                                         all data transmitted on the wire for the given packet including packet data, pad bytes,
                                                         FCS bytes, PAUSE bytes, and JAM bytes. The octet counts do not include PREAMBLE byte or
                                                         EXTEND cycles.
                                                         Not cleared on read; cleared on a write with 0x0.Counters will wrap. Cleared if LMAC is
                                                         disabled with BGX_CMR_CONFIG[ENABLE]=0. */
#else
	uint64_t hist4                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_tx_stat10_s     cn78xx;
};
typedef union cvmx_bgxx_cmrx_tx_stat10 cvmx_bgxx_cmrx_tx_stat10_t;

/**
 * cvmx_bgx#_cmr#_tx_stat11
 */
union cvmx_bgxx_cmrx_tx_stat11 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_tx_stat11_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t hist5                        : 48; /**< Number of packets sent with an octet count between 512-1023. Packet length is the sum of
                                                         all data transmitted on the wire for the given packet including packet data, pad bytes,
                                                         FCS bytes, PAUSE bytes, and JAM bytes. The octet counts do not include PREAMBLE byte or
                                                         EXTEND cycles.
                                                         Not cleared on read; cleared on a write with 0x0.Counters will wrap. Cleared if LMAC is
                                                         disabled with BGX_CMR_CONFIG[ENABLE]=0. */
#else
	uint64_t hist5                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_tx_stat11_s     cn78xx;
};
typedef union cvmx_bgxx_cmrx_tx_stat11 cvmx_bgxx_cmrx_tx_stat11_t;

/**
 * cvmx_bgx#_cmr#_tx_stat12
 */
union cvmx_bgxx_cmrx_tx_stat12 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_tx_stat12_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t hist6                        : 48; /**< Number of packets sent with an octet count between 1024-1518. Packet length is the sum of
                                                         all data transmitted on the wire for the given packet including packet data, pad bytes,
                                                         FCS bytes, PAUSE bytes, and JAM bytes. The octet counts do not include PREAMBLE byte or
                                                         EXTEND cycles.
                                                         Not cleared on read; cleared on a write with 0x0.Counters will wrap. Cleared if LMAC is
                                                         disabled with BGX_CMR_CONFIG[ENABLE]=0. */
#else
	uint64_t hist6                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_tx_stat12_s     cn78xx;
};
typedef union cvmx_bgxx_cmrx_tx_stat12 cvmx_bgxx_cmrx_tx_stat12_t;

/**
 * cvmx_bgx#_cmr#_tx_stat13
 */
union cvmx_bgxx_cmrx_tx_stat13 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_tx_stat13_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t hist7                        : 48; /**< Number of packets sent with an octet count > 1518. Packet length is the sum of all data
                                                         transmitted on the wire for the given packet including packet data, pad bytes, FCS bytes,
                                                         PAUSE bytes, and JAM bytes. The octet counts do not include PREAMBLE byte or EXTEND
                                                         cycles.
                                                         Not cleared on read; cleared on a write with 0x0.Counters will wrap. Cleared if LMAC is
                                                         disabled with BGX_CMR_CONFIG[ENABLE]=0. */
#else
	uint64_t hist7                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_tx_stat13_s     cn78xx;
};
typedef union cvmx_bgxx_cmrx_tx_stat13 cvmx_bgxx_cmrx_tx_stat13_t;

/**
 * cvmx_bgx#_cmr#_tx_stat14
 */
union cvmx_bgxx_cmrx_tx_stat14 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_tx_stat14_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t bcst                         : 48; /**< Number of packets sent to multicast DMAC. Does not include MCST packets.
                                                         Not cleared on read; cleared on a write with 0x0. Counters will wrap.
                                                         Note that BGX determines if the packet is MCST or BCST from the DMAC of the packet. BGX
                                                         assumes that the DMAC lies in the first six bytes of the packet as per the 802.3 frame
                                                         definition. If the system requires additional data before the L2 header, the MCST and BCST
                                                         counters may not reflect reality and should be ignored by software. Cleared if LMAC is
                                                         disabled with BGX_CMR_CONFIG[ENABLE]=0. */
#else
	uint64_t bcst                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_tx_stat14_s     cn78xx;
};
typedef union cvmx_bgxx_cmrx_tx_stat14 cvmx_bgxx_cmrx_tx_stat14_t;

/**
 * cvmx_bgx#_cmr#_tx_stat15
 */
union cvmx_bgxx_cmrx_tx_stat15 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_tx_stat15_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t mcst                         : 48; /**< Number of packets sent to multicast DMAC. Does not include BCST packets.
                                                         Not cleared on read; cleared on a write with 0x0. Counters will wrap.
                                                         Note that BGX determines if the packet is MCST or BCST from the DMAC of the packet. BGX
                                                         assumes that the DMAC lies in the first six bytes of the packet as per the 802.3 frame
                                                         definition. If the system requires additional data before the L2 header, then the MCST and
                                                         BCST counters may not reflect reality and should be ignored by software. Cleared if LMAC
                                                         is disabled with BGX_CMR_CONFIG[ENABLE]=0. */
#else
	uint64_t mcst                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_tx_stat15_s     cn78xx;
};
typedef union cvmx_bgxx_cmrx_tx_stat15 cvmx_bgxx_cmrx_tx_stat15_t;

/**
 * cvmx_bgx#_cmr#_tx_stat16
 */
union cvmx_bgxx_cmrx_tx_stat16 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_tx_stat16_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t undflw                       : 48; /**< Number of underflow packets.
                                                         Not cleared on read; cleared on a write with 0x0. Counters will wrap. Cleared if LMAC is
                                                         disabled with BGX_CMR_CONFIG[ENABLE]=0. */
#else
	uint64_t undflw                       : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_tx_stat16_s     cn78xx;
};
typedef union cvmx_bgxx_cmrx_tx_stat16 cvmx_bgxx_cmrx_tx_stat16_t;

/**
 * cvmx_bgx#_cmr#_tx_stat17
 */
union cvmx_bgxx_cmrx_tx_stat17 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_tx_stat17_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t ctl                          : 48; /**< Number of control packets (PAUSE flow control) generated by BGX. It does not include
                                                         control packets forwarded or generated by the cores.
                                                         CTL counts the number of generated PFC frames and does not track the number of generated
                                                         HG2 messages.
                                                         Not cleared on read; cleared on a write with 0x0. Counters will wrap. Cleared if LMAC is
                                                         disabled with BGX_CMR_CONFIG[ENABLE]=0. */
#else
	uint64_t ctl                          : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_tx_stat17_s     cn78xx;
};
typedef union cvmx_bgxx_cmrx_tx_stat17 cvmx_bgxx_cmrx_tx_stat17_t;

/**
 * cvmx_bgx#_cmr#_tx_stat2
 */
union cvmx_bgxx_cmrx_tx_stat2 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_tx_stat2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t mcol                         : 48; /**< Number of packets sent with multiple collisions. Must be less than
                                                         BGX_GMP_GMI_TX_COL_ATTEMPT[LIMIT]. SGMII/1000BASE-X half-duplex only.
                                                         Not cleared on read; cleared on a write with 0x0. Counters will wrap. Cleared if LMAC is
                                                         disabled with BGX_CMR_CONFIG[ENABLE]=0. */
#else
	uint64_t mcol                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_tx_stat2_s      cn78xx;
};
typedef union cvmx_bgxx_cmrx_tx_stat2 cvmx_bgxx_cmrx_tx_stat2_t;

/**
 * cvmx_bgx#_cmr#_tx_stat3
 */
union cvmx_bgxx_cmrx_tx_stat3 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_tx_stat3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t scol                         : 48; /**< Number of packets sent with a single collision. SGMII/1000BASE-X half-duplex only.
                                                         Not cleared on read; cleared on a write with 0x0. Counters will wrap. Cleared if LMAC is
                                                         disabled with BGX_CMR_CONFIG[ENABLE]=0. */
#else
	uint64_t scol                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_tx_stat3_s      cn78xx;
};
typedef union cvmx_bgxx_cmrx_tx_stat3 cvmx_bgxx_cmrx_tx_stat3_t;

/**
 * cvmx_bgx#_cmr#_tx_stat4
 */
union cvmx_bgxx_cmrx_tx_stat4 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_tx_stat4_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t octs                         : 48; /**< Number of total octets sent on the interface. Does not count octets from frames that were
                                                         truncated due to collisions in half-duplex mode.
                                                         Octet counts are the sum of all data transmitted on the wire including packet data, pad
                                                         bytes, FCS bytes, PAUSE bytes, and JAM bytes. The octet counts do not include PREAMBLE
                                                         byte or EXTEND cycles.
                                                         Not cleared on read; cleared on a write with 0x0. Counters will wrap. Cleared if LMAC is
                                                         disabled with BGX_CMR_CONFIG[ENABLE]=0. */
#else
	uint64_t octs                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_tx_stat4_s      cn78xx;
};
typedef union cvmx_bgxx_cmrx_tx_stat4 cvmx_bgxx_cmrx_tx_stat4_t;

/**
 * cvmx_bgx#_cmr#_tx_stat5
 */
union cvmx_bgxx_cmrx_tx_stat5 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_tx_stat5_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t pkts                         : 48; /**< Number of total frames sent on the interface. Does not count octets from frames that were
                                                         truncated due to collisions in half-duplex mode.
                                                         Not cleared on read; cleared on a write with 0x0. Counters will wrap. Cleared if LMAC is
                                                         disabled with BGX_CMR_CONFIG[ENABLE]=0. */
#else
	uint64_t pkts                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_tx_stat5_s      cn78xx;
};
typedef union cvmx_bgxx_cmrx_tx_stat5 cvmx_bgxx_cmrx_tx_stat5_t;

/**
 * cvmx_bgx#_cmr#_tx_stat6
 */
union cvmx_bgxx_cmrx_tx_stat6 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_tx_stat6_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t hist0                        : 48; /**< Number of packets sent with an octet count < 64. Packet length is the sum of all data
                                                         transmitted on the wire for the given packet including packet data, pad bytes, FCS bytes,
                                                         PAUSE bytes, and JAM bytes. The octet counts do not include PREAMBLE byte or EXTEND
                                                         cycles.
                                                         Not cleared on read; cleared on a write with 0x0.Counters will wrap. Cleared if LMAC is
                                                         disabled with BGX_CMR_CONFIG[ENABLE]=0. */
#else
	uint64_t hist0                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_tx_stat6_s      cn78xx;
};
typedef union cvmx_bgxx_cmrx_tx_stat6 cvmx_bgxx_cmrx_tx_stat6_t;

/**
 * cvmx_bgx#_cmr#_tx_stat7
 */
union cvmx_bgxx_cmrx_tx_stat7 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_tx_stat7_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t hist1                        : 48; /**< Number of packets sent with an octet count of 64. Packet length is the sum of all data
                                                         transmitted on the wire for the given packet including packet data, pad bytes, FCS bytes,
                                                         PAUSE bytes, and JAM bytes. The octet counts do not include PREAMBLE byte or EXTEND
                                                         cycles.
                                                         Not cleared on read; cleared on a write with 0x0.Counters will wrap. Cleared if LMAC is
                                                         disabled with BGX_CMR_CONFIG[ENABLE]=0. */
#else
	uint64_t hist1                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_tx_stat7_s      cn78xx;
};
typedef union cvmx_bgxx_cmrx_tx_stat7 cvmx_bgxx_cmrx_tx_stat7_t;

/**
 * cvmx_bgx#_cmr#_tx_stat8
 */
union cvmx_bgxx_cmrx_tx_stat8 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_tx_stat8_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t hist2                        : 48; /**< Number of packets sent with an octet count between 65-127. Packet length is the sum of all
                                                         data transmitted on the wire for the given packet including packet data, pad bytes, FCS
                                                         bytes, PAUSE bytes, and JAM bytes. The octet counts do not include PREAMBLE byte or EXTEND
                                                         cycles.
                                                         Not cleared on read; cleared on a write with 0x0.Counters will wrap. Cleared if LMAC is
                                                         disabled with BGX_CMR_CONFIG[ENABLE]=0. */
#else
	uint64_t hist2                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_tx_stat8_s      cn78xx;
};
typedef union cvmx_bgxx_cmrx_tx_stat8 cvmx_bgxx_cmrx_tx_stat8_t;

/**
 * cvmx_bgx#_cmr#_tx_stat9
 */
union cvmx_bgxx_cmrx_tx_stat9 {
	uint64_t u64;
	struct cvmx_bgxx_cmrx_tx_stat9_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t hist3                        : 48; /**< Number of packets sent with an octet count between 128-255. Packet length is the sum of
                                                         all data transmitted on the wire for the given packet including packet data, pad bytes,
                                                         FCS bytes, PAUSE bytes, and JAM bytes. The octet counts do not include PREAMBLE byte or
                                                         EXTEND cycles.
                                                         Not cleared on read; cleared on a write with 0x0.Counters will wrap. Cleared if LMAC is
                                                         disabled with BGX_CMR_CONFIG[ENABLE]=0. */
#else
	uint64_t hist3                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_cmrx_tx_stat9_s      cn78xx;
};
typedef union cvmx_bgxx_cmrx_tx_stat9 cvmx_bgxx_cmrx_tx_stat9_t;

/**
 * cvmx_bgx#_cmr_bad
 */
union cvmx_bgxx_cmr_bad {
	uint64_t u64;
	struct cvmx_bgxx_cmr_bad_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t rxb_nxl                      : 1;  /**< Receive side lmac_id > BGX_CMR_RX_LMACS */
#else
	uint64_t rxb_nxl                      : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_bgxx_cmr_bad_s            cn78xx;
};
typedef union cvmx_bgxx_cmr_bad cvmx_bgxx_cmr_bad_t;

/**
 * cvmx_bgx#_cmr_bist_status
 */
union cvmx_bgxx_cmr_bist_status {
	uint64_t u64;
	struct cvmx_bgxx_cmr_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t status                       : 25; /**< "BIST results. Hardware sets a bit to 1 for memory that fails; 0 indicates pass or never
                                                         run. INTERNAL:
                                                         <0> = bgx#.rxb.infif_gmp
                                                         <1> = bgx#.rxb.infif_smu
                                                         <2> = bgx#.rxb.fif_bnk00
                                                         <3> = bgx#.rxb.fif_bnk01
                                                         <4> = bgx#.rxb.fif_bnk10
                                                         <5> = bgx#.rxb.fif_bnk11
                                                         <6> = bgx#.rxb.skd_fif
                                                         <7> = bgx#.rxb_mix0_fif
                                                         <8> = bgx#.rxb_mix1_fif
                                                         <9> = RAZ
                                                         <10> = bgx#.txb_fif_bnk0
                                                         <11> = bgx#.txb_fif_bnk1
                                                         <12> = bgx#.txb_skd_fif
                                                         <13> = bgx#.txb_mix0_fif
                                                         <14> = bgx#.txb_mix1_fif
                                                         <24:15> = RAZ" */
#else
	uint64_t status                       : 25;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_bgxx_cmr_bist_status_s    cn78xx;
};
typedef union cvmx_bgxx_cmr_bist_status cvmx_bgxx_cmr_bist_status_t;

/**
 * cvmx_bgx#_cmr_chan_msk_and
 */
union cvmx_bgxx_cmr_chan_msk_and {
	uint64_t u64;
	struct cvmx_bgxx_cmr_chan_msk_and_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t msk_and                      : 64; /**< Assert physical backpressure when the backpressure channel vector combined with MSK_AND
                                                         indicates backpressure as follows:
                                                         phys_bp_msk_and = (CHAN_VECTOR<x:y> & MSK_AND<x:y>) == MSK_AND<x:y>
                                                         phys_bp = phys_bp_msk_or || phys_bp_msk_and
                                                         In single LMAC configurations, x = 63, y = 0
                                                         In multi-LMAC configurations, x/y are set as follows:
                                                         LMAC interface 0, x = 15, y = 0
                                                         LMAC interface 1, x = 31, y = 16
                                                         LMAC interface 2, x = 47, y = 32
                                                         LMAC interface 3, x = 63, y = 48 */
#else
	uint64_t msk_and                      : 64;
#endif
	} s;
	struct cvmx_bgxx_cmr_chan_msk_and_s   cn78xx;
};
typedef union cvmx_bgxx_cmr_chan_msk_and cvmx_bgxx_cmr_chan_msk_and_t;

/**
 * cvmx_bgx#_cmr_chan_msk_or
 */
union cvmx_bgxx_cmr_chan_msk_or {
	uint64_t u64;
	struct cvmx_bgxx_cmr_chan_msk_or_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t msk_or                       : 64; /**< Assert physical backpressure when the backpressure channel vector combined with MSK_OR
                                                         indicates backpressure as follows:
                                                         phys_bp_msk_or = (CHAN_VECTOR<x:y> & MSK_OR<x:y>) & MSK_OR<x:y>
                                                         phys_bp = phys_bp_msk_or || phys_bp_msk_and
                                                         In single LMAC configurations, x = 63, y = 0
                                                         In multi-LMAC configurations, x/y are set as follows:
                                                         LMAC interface 0, x = 15, y = 0
                                                         LMAC interface 1, x = 31, y = 16
                                                         LMAC interface 2, x = 47, y = 32
                                                         LMAC interface 3, x = 63, y = 48 */
#else
	uint64_t msk_or                       : 64;
#endif
	} s;
	struct cvmx_bgxx_cmr_chan_msk_or_s    cn78xx;
};
typedef union cvmx_bgxx_cmr_chan_msk_or cvmx_bgxx_cmr_chan_msk_or_t;

/**
 * cvmx_bgx#_cmr_global_config
 *
 * These registers configures the global CMR, PCS, and MAC.
 *
 */
union cvmx_bgxx_cmr_global_config {
	uint64_t u64;
	struct cvmx_bgxx_cmr_global_config_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t cmr_mix1_reset               : 1;  /**< If the MIX1 block is reset, software also needs to reset the MIX interface in the BGX by
                                                         setting this bit to 1. It resets the MIX interface state in the BGX (mix FIFO and pending
                                                         requests to MIX) and prevents the RXB FIFOs for all LMACs from pushing data to the
                                                         interface. Setting this bit to 0 will not reset the MIX interface. After MIX comes out of
                                                         reset, software should clear CMR_MIX_RESET. */
	uint64_t cmr_mix0_reset               : 1;  /**< If the MIX0 block is reset, software also needs to reset the MIX interface in the BGX by
                                                         setting this bit to 1. It resets the MIX interface state in the BGX (mix FIFO and pending
                                                         requests to MIX) and prevents the RXB FIFOs for all LMACs from pushing data to the
                                                         interface. Setting this bit to 0 will not reset the MIX interface. After MIX comes out of
                                                         reset, software should clear CMR_MIX_RESET. */
	uint64_t cmr_x2p_reset                : 1;  /**< If the PKI block is reset, software also needs to reset the X2P interface in the BGX by
                                                         setting this bit to 1. It resets the X2P interface state in the BGX (skid FIFO and pending
                                                         requests to PKI) and prevents the RXB FIFOs for all LMACs from pushing data to the
                                                         interface. Setting this bit to 0 does not reset the X2P interface. After PKI comes out of
                                                         reset, software should clear CMR_X2P_RESET. */
	uint64_t bgx_clk_enable               : 1;  /**< The global clock enable for BGX. Setting this bit overrides clock enables set by
                                                         BGX_CMR_CONFIG[ENABLE] and BGX_CMR_CONFIG[LMAC_TYPE], essentially turning on clocks for
                                                         the entire BGX. Setting this bit to 0 results in not overriding clock enables set by
                                                         BGX_CMR_CONFIG[ENABLE] and BGX_CMR_CONFIG[LMAC_TYPE]. */
	uint64_t pmux_sds_sel                 : 1;  /**< Serdes/QLM output select. Specifies which QLM output is selected as the BGX input, as
                                                         follows:
                                                           ------+----------------+----------------
                                                           Block | PMUX_SDS_SEL=0 | PMUX_SDS_SEL=1
                                                           ------+----------------+----------------
                                                           BGX0  | QLM0           | QLM2
                                                           BGX1  | QLM1           | QLM3
                                                           BGX2  | QLM4           | N/A
                                                           BGX3  | QLM5           | N/A
                                                           BGX4  | QLM6           | N/A
                                                           BGX5  | QLM7           | N/A
                                                           ------+----------------+---------------- */
#else
	uint64_t pmux_sds_sel                 : 1;
	uint64_t bgx_clk_enable               : 1;
	uint64_t cmr_x2p_reset                : 1;
	uint64_t cmr_mix0_reset               : 1;
	uint64_t cmr_mix1_reset               : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_bgxx_cmr_global_config_s  cn78xx;
};
typedef union cvmx_bgxx_cmr_global_config cvmx_bgxx_cmr_global_config_t;

/**
 * cvmx_bgx#_cmr_mem_ctrl
 */
union cvmx_bgxx_cmr_mem_ctrl {
	uint64_t u64;
	struct cvmx_bgxx_cmr_mem_ctrl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t txb_skid_synd                : 2;  /**< Syndrome to flip and generate single-bit/double-bit for TXB SKID FIFO */
	uint64_t txb_skid_cor_dis             : 1;  /**< ECC-correction disable for the TXB SKID FIFO */
	uint64_t txb_fif_bk1_syn              : 2;  /**< Syndrome to flip and generate single-bit/double-bit error for TXB main bank1 */
	uint64_t txb_fif_bk1_cdis             : 1;  /**< ECC-correction disable for the TXB main bank1 */
	uint64_t txb_fif_bk0_syn              : 2;  /**< Syndrome to flip and generate single-bit/double-bit error for TXB main bank0 */
	uint64_t txb_fif_bk0_cdis             : 1;  /**< ECC-correction disable for the TXB main bank0 */
	uint64_t rxb_skid_synd                : 2;  /**< Syndrome to flip and generate single-bit/double-bit error for RXB SKID FIFO */
	uint64_t rxb_skid_cor_dis             : 1;  /**< ECC-correction disable for the RXB SKID FIFO */
	uint64_t rxb_fif_bk1_syn1             : 2;  /**< Syndrome to flip and generate single-bit/double-bit error for RXB main bank1 srf1 */
	uint64_t rxb_fif_bk1_cdis1            : 1;  /**< ECC-correction disable for the RXB main bank1 srf1 */
	uint64_t rxb_fif_bk1_syn0             : 2;  /**< Syndrome to flip and generate single-bit/double-bit error for RXB main bank1 srf0 */
	uint64_t rxb_fif_bk1_cdis0            : 1;  /**< ECC-correction disable for the RXB main bank1 srf0. */
	uint64_t rxb_fif_bk0_syn1             : 2;  /**< Syndrome to flip and generate single-bit/double-bit error for RXB main bank0 srf1 */
	uint64_t rxb_fif_bk0_cdis1            : 1;  /**< ECC-correction disable for the RXB main bank0 srf1 */
	uint64_t rxb_fif_bk0_syn0             : 2;  /**< Syndrome to flip and generate single-bit/double-bit error for RXB main bank0 srf0 */
	uint64_t rxb_fif_bk0_cdis0            : 1;  /**< ECC-correction disable for the RXB main bank0 srf0 */
#else
	uint64_t rxb_fif_bk0_cdis0            : 1;
	uint64_t rxb_fif_bk0_syn0             : 2;
	uint64_t rxb_fif_bk0_cdis1            : 1;
	uint64_t rxb_fif_bk0_syn1             : 2;
	uint64_t rxb_fif_bk1_cdis0            : 1;
	uint64_t rxb_fif_bk1_syn0             : 2;
	uint64_t rxb_fif_bk1_cdis1            : 1;
	uint64_t rxb_fif_bk1_syn1             : 2;
	uint64_t rxb_skid_cor_dis             : 1;
	uint64_t rxb_skid_synd                : 2;
	uint64_t txb_fif_bk0_cdis             : 1;
	uint64_t txb_fif_bk0_syn              : 2;
	uint64_t txb_fif_bk1_cdis             : 1;
	uint64_t txb_fif_bk1_syn              : 2;
	uint64_t txb_skid_cor_dis             : 1;
	uint64_t txb_skid_synd                : 2;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_bgxx_cmr_mem_ctrl_s       cn78xx;
};
typedef union cvmx_bgxx_cmr_mem_ctrl cvmx_bgxx_cmr_mem_ctrl_t;

/**
 * cvmx_bgx#_cmr_mem_int
 */
union cvmx_bgxx_cmr_mem_int {
	uint64_t u64;
	struct cvmx_bgxx_cmr_mem_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t smu_in_overfl                : 1;  /**< RX SMU INFIFO overflow */
	uint64_t gmp_in_overfl                : 1;  /**< RX GMP INFIFO overflow */
	uint64_t txb_skid_sbe                 : 1;  /**< TXB SKID FIFO single-bit error */
	uint64_t txb_skid_dbe                 : 1;  /**< TXB SKID FIFO double-bit error */
	uint64_t txb_fif_bk1_sbe              : 1;  /**< TXB Main FIFO Bank1 single-bit error */
	uint64_t txb_fif_bk1_dbe              : 1;  /**< TXB Main FIFO Bank1 double-bit error */
	uint64_t txb_fif_bk0_sbe              : 1;  /**< TXB Main FIFO Bank0 single-bit error */
	uint64_t txb_fif_bk0_dbe              : 1;  /**< TXB Main FIFO Bank0 double-bit error */
	uint64_t rxb_skid_sbe                 : 1;  /**< RXB SKID FIFO single-bit error */
	uint64_t rxb_skid_dbe                 : 1;  /**< RXB SKID FIFO double-bit error */
	uint64_t rxb_fif_bk1_sbe1             : 1;  /**< RXB main FIFO bank1 srf1 single-bit error */
	uint64_t rxb_fif_bk1_dbe1             : 1;  /**< RXB main FIFO bank1 srf1 double-bit error */
	uint64_t rxb_fif_bk1_sbe0             : 1;  /**< RXB main FIFO bank1 srf0 single-bit error */
	uint64_t rxb_fif_bk1_dbe0             : 1;  /**< RXB main FIFO bank1 srf0 double-bit error */
	uint64_t rxb_fif_bk0_sbe1             : 1;  /**< RXB main FIFO bank0 srf1 single-bit error */
	uint64_t rxb_fif_bk0_dbe1             : 1;  /**< RXB main FIFO bank0 srf1 double-bit error */
	uint64_t rxb_fif_bk0_sbe0             : 1;  /**< RXB main FIFO bank0 srf0 single-bit error */
	uint64_t rxb_fif_bk0_dbe0             : 1;  /**< RXB main FIFO bank0 srf0 double-bit error */
#else
	uint64_t rxb_fif_bk0_dbe0             : 1;
	uint64_t rxb_fif_bk0_sbe0             : 1;
	uint64_t rxb_fif_bk0_dbe1             : 1;
	uint64_t rxb_fif_bk0_sbe1             : 1;
	uint64_t rxb_fif_bk1_dbe0             : 1;
	uint64_t rxb_fif_bk1_sbe0             : 1;
	uint64_t rxb_fif_bk1_dbe1             : 1;
	uint64_t rxb_fif_bk1_sbe1             : 1;
	uint64_t rxb_skid_dbe                 : 1;
	uint64_t rxb_skid_sbe                 : 1;
	uint64_t txb_fif_bk0_dbe              : 1;
	uint64_t txb_fif_bk0_sbe              : 1;
	uint64_t txb_fif_bk1_dbe              : 1;
	uint64_t txb_fif_bk1_sbe              : 1;
	uint64_t txb_skid_dbe                 : 1;
	uint64_t txb_skid_sbe                 : 1;
	uint64_t gmp_in_overfl                : 1;
	uint64_t smu_in_overfl                : 1;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_bgxx_cmr_mem_int_s        cn78xx;
};
typedef union cvmx_bgxx_cmr_mem_int cvmx_bgxx_cmr_mem_int_t;

/**
 * cvmx_bgx#_cmr_nxc_adr
 */
union cvmx_bgxx_cmr_nxc_adr {
	uint64_t u64;
	struct cvmx_bgxx_cmr_nxc_adr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t lmac_id                      : 4;  /**< Logged LMAC ID associated with NXC exceptions */
	uint64_t channel                      : 12; /**< Logged channel for NXC exceptions */
#else
	uint64_t channel                      : 12;
	uint64_t lmac_id                      : 4;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_cmr_nxc_adr_s        cn78xx;
};
typedef union cvmx_bgxx_cmr_nxc_adr cvmx_bgxx_cmr_nxc_adr_t;

/**
 * cvmx_bgx#_cmr_rx_adr#_cam
 *
 * These registers provide access to the 32 DMAC CAM entries in BGX.
 *
 */
union cvmx_bgxx_cmr_rx_adrx_cam {
	uint64_t u64;
	struct cvmx_bgxx_cmr_rx_adrx_cam_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_54_63               : 10;
	uint64_t id                           : 2;  /**< Logical MAC ID that this DMAC CAM address applies to. BGX has 32 DMAC CAM entries that can
                                                         be accessed with the BGX*_CMR_RX_ADR_CAM(0..31) CSRs. These 32 DMAC entries can be used by
                                                         any of the four SGMII MACs or the 10G/40G MACs using these register bits.
                                                         A typical configuration is to provide eight CAM entries per LMAC ID, which is configured
                                                         using the following settings:
                                                         LMAC interface 0: BGX(0..5)_CMR_RX_ADR(0..7)_CAM[ID] = 0x0
                                                         LMAC interface 1: BGX(0..5)_CMR_RX_ADR(8..15)_CAM[ID] = 0x1
                                                         LMAC interface 2: BGX(0..5)_CMR_RX_ADR(16..23)_CAM[ID] = 0x2
                                                         LMAC interface 3: BGX(0..5)_CMR_RX_ADR(24..31)_CAM[ID] = 0x3 */
	uint64_t reserved_49_51               : 3;
	uint64_t en                           : 1;  /**< CAM entry enable for this DMAC address.
                                                         1 = Include this address in the matching algorithm.
                                                         0 = Don't include this address in the matching algorithm. */
	uint64_t adr                          : 48; /**< DMAC address in the CAM used for matching. The CAM matches against unicast or multicast
                                                         DMAC addresses. All BGX*_CMR_RX_ADR_CAM(0..31) CSRs can be used in any of the LMAC_TYPE
                                                         combinations such that any BGX MAC can use any of the 32 common DMAC entries. */
#else
	uint64_t adr                          : 48;
	uint64_t en                           : 1;
	uint64_t reserved_49_51               : 3;
	uint64_t id                           : 2;
	uint64_t reserved_54_63               : 10;
#endif
	} s;
	struct cvmx_bgxx_cmr_rx_adrx_cam_s    cn78xx;
};
typedef union cvmx_bgxx_cmr_rx_adrx_cam cvmx_bgxx_cmr_rx_adrx_cam_t;

/**
 * cvmx_bgx#_cmr_rx_lmacs
 */
union cvmx_bgxx_cmr_rx_lmacs {
	uint64_t u64;
	struct cvmx_bgxx_cmr_rx_lmacs_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t lmacs                        : 3;  /**< "Number of LMACS: Specifies the number of LMACs that can be enabled.
                                                         This determines the logical RX buffer size per LMAC and the maximum
                                                         LMAC ID that can be used:
                                                           ----------+---------------------------------------------------
                                                           LMACS     |   RX buffer           Maximum
                                                                         size per LMAC       LMAC ID
                                                           ----------+---------------------------------------------------
                                                           0         |   reserved
                                                           1         |   64KB                0
                                                           2         |   32KB                1
                                                           3         |   16KB                2
                                                           4         |   16KB                3
                                                           5-7       |   reserved
                                                           ----------+---------------------------------------------------
                                                         Note: The maximum LMAC ID is determined by the smaller of
                                                         BGX_CMR_RX_LMACS[LMACS] and BGX_CMR_TX_LMACS[LMACS]. The two fields
                                                         should be set to the same value for normal operation." */
#else
	uint64_t lmacs                        : 3;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_bgxx_cmr_rx_lmacs_s       cn78xx;
};
typedef union cvmx_bgxx_cmr_rx_lmacs cvmx_bgxx_cmr_rx_lmacs_t;

/**
 * cvmx_bgx#_cmr_rx_ovr_bp
 *
 * BGX_CMR_RX_OVR_BP[EN<0>] must be set to one and BGX_CMR_RX_OVR_BP[BP<0>] must be cleared to
 * zero (to forcibly disable hardware-automatic 802.3 PAUSE packet generation) with the HiGig2
 * Protocol when BGX_SMU_HG2_CONTROL[HG2TX_EN]=0. (The HiGig2 protocol is indicated by
 * BGX_SMU_TX_CTL[HG_EN]=1 and BGX_SMU_RX_UDD_SKP[LEN]=16). Hardware can only auto-generate
 * backpressure through HiGig2 messages (optionally, when BGX_SMU_HG2_CONTROL[HG2TX_EN]=1) with
 * the HiGig2 protocol.
 */
union cvmx_bgxx_cmr_rx_ovr_bp {
	uint64_t u64;
	struct cvmx_bgxx_cmr_rx_ovr_bp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t en                           : 4;  /**< Per-LMAC enable backpressure override.
                                                         1 = Enable override, 0 = Don't enable
                                                         Bit<8> represents LMAC 0, ..., bit<11> represents LMAC 3. */
	uint64_t bp                           : 4;  /**< Per-LMAC backpressure status to use:
                                                         1 = LMAC should be backpressured, 0 = LMAC is available
                                                         Bit<4> represents LMAC 0, ..., bit<7> represents LMAC 3. */
	uint64_t ign_fifo_bp                  : 4;  /**< Ignore the RX FIFO BP_ON signal when computing backpressure. CMR does not backpressure the
                                                         MAC due to the FIFO length passing BP_ON mark. */
#else
	uint64_t ign_fifo_bp                  : 4;
	uint64_t bp                           : 4;
	uint64_t en                           : 4;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_bgxx_cmr_rx_ovr_bp_s      cn78xx;
};
typedef union cvmx_bgxx_cmr_rx_ovr_bp cvmx_bgxx_cmr_rx_ovr_bp_t;

/**
 * cvmx_bgx#_cmr_tx_lmacs
 *
 * Number of transmit LMACs.
 *
 */
union cvmx_bgxx_cmr_tx_lmacs {
	uint64_t u64;
	struct cvmx_bgxx_cmr_tx_lmacs_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t lmacs                        : 3;  /**< "Number of LMACS: Specifies the number of LMACs that can be enabled.
                                                         This determines the logical TX buffer size per LMAC and the maximum
                                                         LMAC ID that can be used:
                                                           ----------+---------------------------------------------------
                                                           LMACS     |   TX buffer           Maximum
                                                                         size per LMAC       LMAC ID
                                                           ----------+---------------------------------------------------
                                                           0         |   reserved
                                                           1         |   32KB                0
                                                           2         |   16KB                1
                                                           3         |   8KB                 2
                                                           4         |   8KB                 3
                                                           5-7       |   reserved
                                                           ----------+---------------------------------------------------
                                                         Note: The maximum LMAC ID is determined by the smaller of
                                                         BGX_CMR_RX_LMACS[LMACS] and BGX_CMR_TX_LMACS[LMACS]. The two fields
                                                         should be set to the same value for normal operation." */
#else
	uint64_t lmacs                        : 3;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_bgxx_cmr_tx_lmacs_s       cn78xx;
};
typedef union cvmx_bgxx_cmr_tx_lmacs cvmx_bgxx_cmr_tx_lmacs_t;

/**
 * cvmx_bgx#_gmp_gmi_prt#_cfg
 *
 * This register controls the configuration of the LMAC.
 *
 */
union cvmx_bgxx_gmp_gmi_prtx_cfg {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_prtx_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t tx_idle                      : 1;  /**< TX machine is idle. */
	uint64_t rx_idle                      : 1;  /**< RX machine is idle. */
	uint64_t reserved_9_11                : 3;
	uint64_t speed_msb                    : 1;  /**< Link speed MSB [SPEED_MSB:SPEED]
                                                         10 = 10 Mb/s operation
                                                         00 = 100 Mb/s operation
                                                         01 = 1000 Mb/s operation
                                                         11 = Reserved
                                                         (SGMII/1000Base-X only) */
	uint64_t reserved_4_7                 : 4;
	uint64_t slottime                     : 1;  /**< Slot time for half-duplex operation
                                                         0 = 512 bit times (10/100 Mb/s operation)
                                                         1 = 4096 bit times (1000 Mb/s operation)
                                                         (SGMII/1000Base-X only) */
	uint64_t duplex                       : 1;  /**< Duplex mode:
                                                         0 = half-duplex (collisions/extensions/bursts), 1 = full-duplex.
                                                         (SGMII/1000Base-X only) */
	uint64_t speed                        : 1;  /**< Link Speed LSB [SPEED_MSB:SPEED]
                                                         10 = 10 Mb/s operation
                                                         00 = 100 Mb/s operation
                                                         01 = 1000 Mb/s operation
                                                         11 = Reserved
                                                         (SGMII/1000Base-X only) */
	uint64_t reserved_0_0                 : 1;
#else
	uint64_t reserved_0_0                 : 1;
	uint64_t speed                        : 1;
	uint64_t duplex                       : 1;
	uint64_t slottime                     : 1;
	uint64_t reserved_4_7                 : 4;
	uint64_t speed_msb                    : 1;
	uint64_t reserved_9_11                : 3;
	uint64_t rx_idle                      : 1;
	uint64_t tx_idle                      : 1;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_prtx_cfg_s   cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_prtx_cfg cvmx_bgxx_gmp_gmi_prtx_cfg_t;

/**
 * cvmx_bgx#_gmp_gmi_rx#_decision
 *
 * Notes:
 * As each byte in a packet is received by GMI, the L2 byte count is compared
 * against the BGX_GMP_GMI_RX_DECISION[CNT].  The L2 byte count is the number of bytes
 * from the beginning of the L2 header (DMAC).  In normal operation, the L2
 * header begins after the PREAMBLE+SFD (BGX_GMP_GMI_RX_FRM_CTL[PRE_CHK]=1) and any
 * optional UDD skip data (BGX_GMP_GMI_RX_UDD_SKP[LEN]).
 * When BGX_GMP_GMI_RX_FRM_CTL[PRE_CHK] is clear, PREAMBLE+SFD are prepended to the
 * packet and would require UDD skip length to account for them.
 * Port Mode
 * - Full Duplex
 *     L2 Size <  BGX_RX_DECISION - Accept packet. No filtering is applied
 *     L2 Size >= BGX_RX_DECISION - Apply filter. Accept packet based on PAUSE packet filter
 * - Half Duplex
 *     L2 Size <  BGX_RX_DECISION - Drop packet. Packet is unconditionally dropped.
 *     L2 Size >= BGX_RX_DECISION - Accept packet.
 * where L2_size = MAX(0, total_packet_size - BGX_GMP_GMI_RX_UDD_SKP[LEN] -
 *                        ((BGX_GMP_GMI_RX_FRM_CTL[PRE_CHK]==1)*8))
 * BGX_GMP_GMI_RX_DECISION = The byte count to decide when to accept or filter a packet
 */
union cvmx_bgxx_gmp_gmi_rxx_decision {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_rxx_decision_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t cnt                          : 5;  /**< The byte count used to decide when to accept or filter a packet. Refer to GMI Decisions. */
#else
	uint64_t cnt                          : 5;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_rxx_decision_s cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_rxx_decision cvmx_bgxx_gmp_gmi_rxx_decision_t;

/**
 * cvmx_bgx#_gmp_gmi_rx#_frm_chk
 */
union cvmx_bgxx_gmp_gmi_rxx_frm_chk {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_rxx_frm_chk_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t skperr                       : 1;  /**< Skipper error. */
	uint64_t rcverr                       : 1;  /**< Frame was received with data-reception error. */
	uint64_t reserved_5_6                 : 2;
	uint64_t fcserr                       : 1;  /**< Frame was received with FCS/CRC error. */
	uint64_t jabber                       : 1;  /**< Frame was received with length > sys_length. */
	uint64_t reserved_2_2                 : 1;
	uint64_t carext                       : 1;  /**< Carrier extend error. SGMII/1000Base-X only. */
	uint64_t minerr                       : 1;  /**< PAUSE frame was received with length < minFrameSize. */
#else
	uint64_t minerr                       : 1;
	uint64_t carext                       : 1;
	uint64_t reserved_2_2                 : 1;
	uint64_t jabber                       : 1;
	uint64_t fcserr                       : 1;
	uint64_t reserved_5_6                 : 2;
	uint64_t rcverr                       : 1;
	uint64_t skperr                       : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_rxx_frm_chk_s cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_rxx_frm_chk cvmx_bgxx_gmp_gmi_rxx_frm_chk_t;

/**
 * cvmx_bgx#_gmp_gmi_rx#_frm_ctl
 *
 * Notes:
 * PRE_STRP
 * When PRE_CHK is set (indicating that the PREAMBLE will be sent), PRE_STRP
 * determines if the PREAMBLE+SFD bytes are thrown away or sent to the Octane
 * core as part of the packet.
 * In either mode, the PREAMBLE+SFD bytes are not counted toward the packet
 * size when checking against the MIN and MAX bounds.  Furthermore, the bytes
 * are skipped when locating the start of the L2 header for DMAC and Control
 * frame recognition.
 * CTL_BCK/CTL_DRP
 * These bits control how the HW handles incoming PAUSE packets.  Here are
 * the most common modes of operation:
 * CTL_BCK=1,CTL_DRP=1   - HW does it all
 * CTL_BCK=0,CTL_DRP=0   - SW sees all pause frames
 * CTL_BCK=0,CTL_DRP=1   - all pause frames are completely ignored
 * These control bits should be set to CTL_BCK=0,CTL_DRP=0 in halfdup mode.
 * Since PAUSE packets only apply to fulldup operation, any PAUSE packet
 * would constitute an exception which should be handled by the processing
 * cores.  PAUSE packets should not be forwarded.
 */
union cvmx_bgxx_gmp_gmi_rxx_frm_ctl {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_rxx_frm_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_13_63               : 51;
	uint64_t ptp_mode                     : 1;  /**< Timestamp mode. When PTP_MODE is set, a 64-bit timestamp is prepended to every incoming
                                                         packet.
                                                         The timestamp bytes are added to the packet in such a way as to not modify the packet's
                                                         receive byte count. This implies that the BGX(0..5)_GMP_GMI_RX(0..3)_RX_JABBER,
                                                         BGX(0..5)_GMP_GMI_RX(0..3)_RX_DECISION, BGX(0..5)_GMP_GMI_RX(0..3)_UDD_SKP, and
                                                         BGX(0..5)_CMR(0..3)_RX_STAT* do not require any adjustment as they operate on the received
                                                         packet size. When the packet reaches PKI, its size reflects the additional bytes and is
                                                         subject to the following restrictions:
                                                         If PTP_MODE = 1 and PRE_CHK = 1, PRE_STRP must be 1.
                                                         If PTP_MODE = 1
                                                         PKI_CL(0..3)_PKIND(0..63)_SKIP[FCS_SKIP,INST_SKIP] should be increased by 8.
                                                         PKI_CL(0..3)_PKIND(0..63)_CFG[HG_EN] should be 0.
                                                         PKI_FRM_LEN_CHK(0..1)[MAXLEN] should be increased by 8.
                                                         PKI_FRM_LEN_CHK(0..1)[MINLEN] should be increased by 8.
                                                         PKI_TAG_INC(0..63)_MASK should be adjusted.
                                                         This supported in uCode in O78 >>> PIP_PRT_CFGB(0..63)[ALT_SKP_EN] should be 0. */
	uint64_t reserved_11_11               : 1;
	uint64_t null_dis                     : 1;  /**< When set, do not modify the MOD bits on NULL ticks due to partial packets. */
	uint64_t pre_align                    : 1;  /**< When set, PREAMBLE parser aligns the SFD byte regardless of the number of previous
                                                         PREAMBLE nibbles. In this mode, PRE_STRP should be set to account for the variable nature
                                                         of the PREAMBLE. PRE_CHK must be set to enable this and all PREAMBLE features.
                                                         SGMII at 10/100Mbs only. */
	uint64_t reserved_7_8                 : 2;
	uint64_t pre_free                     : 1;  /**< When set, PREAMBLE checking is less strict. GMI will begin the frame at the first SFD.
                                                         PRE_CHK must be set to enable this and all PREAMBLE features. SGMII/1000Base-X only. */
	uint64_t ctl_smac                     : 1;  /**< Control PAUSE frames can match station SMAC. */
	uint64_t ctl_mcst                     : 1;  /**< Control PAUSE frames can match globally assign multicast address. */
	uint64_t ctl_bck                      : 1;  /**< Forward PAUSE information to TX block. */
	uint64_t ctl_drp                      : 1;  /**< Drop control-PAUSE frames. */
	uint64_t pre_strp                     : 1;  /**< Strip off the preamble (when present).
                                                         0 = PREAMBLE + SFD is sent to core as part of frame.
                                                         1 = PREAMBLE + SFD is dropped.
                                                         [PRE_CHK] must be set to enable this and all PREAMBLE features.
                                                         If PTP_MODE=1 and PRE_CHK=1, PRE_STRP must be 1.
                                                         When PRE_CHK is set (indicating that the PREAMBLE will be sent), PRE_STRP determines if
                                                         the PREAMBLE+SFD bytes are thrown away or sent to the core as part of the packet. In
                                                         either mode, the PREAMBLE+SFD bytes are not counted toward the packet size when checking
                                                         against the MIN and MAX bounds. Furthermore, the bytes are skipped when locating the start
                                                         of the L2 header for DMAC and Control frame recognition. */
	uint64_t pre_chk                      : 1;  /**< Check the preamble for correctness. This port is configured to send a valid 802.3 PREAMBLE
                                                         to begin every frame. GMI checks that a valid PREAMBLE is received (based on PRE_FREE).
                                                         When a problem does occur within the PREAMBLE sequence, the frame is marked as bad and not
                                                         sent into the core. The BGX(0..5)_SMU(0..3)_RX_INT[PCTERR] interrupt is also raised.
                                                         When BGX(0..5)_SMU(0..3)_TX_CTL[HG_EN] is set, PRE_CHK must be 0. If PTP_MODE = 1 and
                                                         PRE_CHK = 1, PRE_STRP must be 1. */
#else
	uint64_t pre_chk                      : 1;
	uint64_t pre_strp                     : 1;
	uint64_t ctl_drp                      : 1;
	uint64_t ctl_bck                      : 1;
	uint64_t ctl_mcst                     : 1;
	uint64_t ctl_smac                     : 1;
	uint64_t pre_free                     : 1;
	uint64_t reserved_7_8                 : 2;
	uint64_t pre_align                    : 1;
	uint64_t null_dis                     : 1;
	uint64_t reserved_11_11               : 1;
	uint64_t ptp_mode                     : 1;
	uint64_t reserved_13_63               : 51;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_rxx_frm_ctl_s cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_rxx_frm_ctl cvmx_bgxx_gmp_gmi_rxx_frm_ctl_t;

/**
 * cvmx_bgx#_gmp_gmi_rx#_ifg
 *
 * This register specifies the minimum number of interframe-gap (IFG) cycles between packets.
 *
 */
union cvmx_bgxx_gmp_gmi_rxx_ifg {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_rxx_ifg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t ifg                          : 4;  /**< Min IFG (in IFG * 8 bits) between packets used to determine IFGERR. Normally IFG is 96
                                                         bits.
                                                         Note that in some operating modes, IFG cycles can be inserted or removed in order to
                                                         achieve clock rate adaptation. For these reasons, the default value is slightly
                                                         conservative and does not check up to the full 96 bits of IFG.
                                                         (SGMII/1000Base-X only) */
#else
	uint64_t ifg                          : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_rxx_ifg_s    cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_rxx_ifg cvmx_bgxx_gmp_gmi_rxx_ifg_t;

/**
 * cvmx_bgx#_gmp_gmi_rx#_int
 *
 * "Notes:
 * (2) exception conditions 10:0 can also set the rcv/opcode in the received
 * packet's workQ entry.  The BGX_GMP_GMI_RX_FRM_CHK register provides a bit mask
 * for configuring which conditions set the error.
 * (3) in half duplex operation, the expectation is that collisions will appear
 * as either MINERR o r CAREXT errors.
 * (4) JABBER An RX Jabber error indicates that a packet was received which
 * is longer than the maximum allowed packet as defined by the
 * system.  GMI will truncate the packet at the JABBER count.
 * Failure to do so could lead to system instabilty.
 * (5) NIBERR This error is illegal at 1000Mbs speeds
 * (BGX_GMP_GMI_RX_PRT_CFG[SPEED]==0) and will never assert.
 * (6) MAXERR for untagged frames, the total frame DA+SA+TL+DATA+PAD+FCS >
 * BGX_GMP_GMI_RX_FRM_MAX.  For tagged frames, DA+SA+VLAN+TL+DATA+PAD+FCS
 * > BGX_GMP_GMI_RX_FRM_MAX + 4*VLAN_VAL + 4*VLAN_STACKED.
 * (7) MINERR total frame DA+SA+TL+DATA+PAD+FCS < 64
 * (8) ALNERR Indicates that the packet received was not an integer number of
 * bytes.  If FCS checking is enabled, ALNERR will only assert if
 * the FCS is bad.  If FCS checking is disabled, ALNERR will
 * assert in all non-integer frame cases.
 * (9) Collisions Collisions can only occur in half-duplex mode.  A collision
 * is assumed by the receiver when the slottime
 * (BGX_GMP_GMI_PRT_CFG[SLOTTIME]) is not satisfied.  In 10/100 mode,
 * this will result in a frame < SLOTTIME.  In 1000 mode, it
 * could result either in frame < SLOTTIME or a carrier extend
 * error with the SLOTTIME.  These conditions are visible by...
 * . transfer ended before slottime COLDET
 * . carrier extend error           CAREXT
 * (A) LENERR Length errors occur when the received packet does not match the
 * length field.  LENERR is only checked for packets between 64
 * and 1500 bytes.  For untagged frames, the length must exact
 * match.  For tagged frames the length or length+4 must match.
 * (B) PCTERR checks that the frame begins with a valid PREAMBLE sequence.
 * Does not check the number of PREAMBLE cycles.
 * (C) OVRERR
 * OVRERR is an architectural assertion check internal to GMI to
 * make sure no assumption was violated.  In a correctly operating
 * system, this interrupt can never fire.
 * GMI has an internal arbiter which selects which of 4 ports to
 * buffer in the main RX FIFO.  If we normally buffer 8 bytes,
 * then each port will typically push a tick every 8 cycles if
 * the packet interface is going as fast as possible.  If there
 * are four ports, they push every two cycles.  So that's the
 * assumption.  That the inbound module will always be able to
 * consume the tick before another is produced.  If that doesn't
 * happen that's when OVRERR will assert."
 */
union cvmx_bgxx_gmp_gmi_rxx_int {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_rxx_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t ifgerr                       : 1;  /**< Interframe gap violation. Does not necessarily indicate a failure. SGMII/1000Base-X only. */
	uint64_t coldet                       : 1;  /**< Collision detection. Collisions can only occur in half-duplex mode. A collision is assumed
                                                         by the receiver when the slottime (BGX(0..5)_GMP_GMI_PRT(0..3)_CFG[SLOTTIME]) is not
                                                         satisfied. In 10/100 mode, this will result in a frame < SLOTTIME. In 1000 mode, it could
                                                         result either in frame < SLOTTIME or a carrier extend error with the SLOTTIME. These
                                                         conditions are visible by 1) transfer ended before slottime - COLDET or 2) carrier extend
                                                         error - CAREXT. */
	uint64_t falerr                       : 1;  /**< False-carrier error, or carrier-extend error after slottime is satisfied. SGMII/1000Base-X only. */
	uint64_t rsverr                       : 1;  /**< Reserved opcode. */
	uint64_t pcterr                       : 1;  /**< Bad preamble/protocol error. Checks that the frame begins with a valid PREAMBLE sequence.
                                                         Does not check the number of PREAMBLE cycles. */
	uint64_t ovrerr                       : 1;  /**< Internal data aggregation overflow. This interrupt should never assert. SGMII/1000Base-X only. */
	uint64_t skperr                       : 1;  /**< Skipper error. */
	uint64_t rcverr                       : 1;  /**< Data-reception error. Frame was received with data-reception error */
	uint64_t fcserr                       : 1;  /**< FCS/CRC error. Frame was received with FCS/CRC error */
	uint64_t jabber                       : 1;  /**< System-length error: frame was received with length > sys_length.
                                                         An RX Jabber error indicates that a packet was received which is longer than the maximum
                                                         allowed packet as defined by the system. GMI truncates the packet at the JABBER count.
                                                         Failure to do so could lead to system instability. */
	uint64_t carext                       : 1;  /**< Carrier extend error
                                                         (SGMII/1000Base-X only) */
	uint64_t minerr                       : 1;  /**< PAUSE frame was received with length < minFrameSize. Frame length checks are typically
                                                         handled in PKI, but PAUSE frames are normally discarded before being inspected by PKI.
                                                         Total frame DA+SA+TL+DATA+PAD+FCS < 64. */
#else
	uint64_t minerr                       : 1;
	uint64_t carext                       : 1;
	uint64_t jabber                       : 1;
	uint64_t fcserr                       : 1;
	uint64_t rcverr                       : 1;
	uint64_t skperr                       : 1;
	uint64_t ovrerr                       : 1;
	uint64_t pcterr                       : 1;
	uint64_t rsverr                       : 1;
	uint64_t falerr                       : 1;
	uint64_t coldet                       : 1;
	uint64_t ifgerr                       : 1;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_rxx_int_s    cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_rxx_int cvmx_bgxx_gmp_gmi_rxx_int_t;

/**
 * cvmx_bgx#_gmp_gmi_rx#_jabber
 *
 * Notes:
 * CNT must be 8-byte aligned such that CNT[2:0] == 0
 * The packet that will be sent to the packet input logic will have an
 * additionl 8 bytes if BGX_GMP_GMI_RX_FRM_CTL[PRE_CHK] is set and
 * BGX_GMP_GMI_RX_FRM_CTL[PRE_STRP] is clear.  The max packet that will be sent is
 * defined as...
 * max_sized_packet = BGX_GMP_GMI_RX_JABBER[CNT]+((BGX_GMP_GMI_RX_FRM_CTL[PRE_CHK] &
 * !BGX_GMP_GMI_RX_FRM_CTL[PRE_STRP])*8)
 * BGX_GMP_GMI_RX_JABBER = The max size packet after which GMI will truncate
 */
union cvmx_bgxx_gmp_gmi_rxx_jabber {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_rxx_jabber_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t cnt                          : 16; /**< Byte count for jabber check. Failing packets set the JABBER interrupt and are optionally
                                                         sent with opcode = JABBER. GMI truncates the packet to CNT bytes.
                                                         CNT must be 8-byte aligned such that CNT<2:0> = 000. */
#else
	uint64_t cnt                          : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_rxx_jabber_s cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_rxx_jabber cvmx_bgxx_gmp_gmi_rxx_jabber_t;

/**
 * cvmx_bgx#_gmp_gmi_rx#_udd_skp
 *
 * Notes:
 * (1) The skip bytes are part of the packet and will be sent down the NCB
 * packet interface and will be handled by PKI.
 * (2) The system can determine if the UDD bytes are included in the FCS check
 * by using the FCSSEL field - if the FCS check is enabled.
 * (3) Assume that the preamble/sfd is always at the start of the frame - even
 * before UDD bytes.  In most cases, there will be no preamble in these
 * cases since it will be packet interface in direct communication to
 * another packet interface (MAC to MAC) without a PHY involved.
 * (4) We can still do address filtering and control packet filtering is the
 * user desires.
 * (5) UDD_SKP must be 0 in half-duplex operation unless
 * BGX_GMP_GMI_RX_FRM_CTL[PRE_CHK] is clear.  If BGX_GMP_GMI_RX_FRM_CTL[PRE_CHK] is clear,
 * then UDD_SKP will normally be 8.
 * (6) In all cases, the UDD bytes will be sent down the packet interface as
 * part of the packet.  The UDD bytes are never stripped from the actual
 * packet.
 * (7) If LEN != 0, then BGX_GMP_GMI_RX_FRM_CHK[LENERR] will be disabled and
 * BGX_GMP_GMI_RX_INT[LENERR] will be zero
 * BGX_GMP_GMI_RX_UDD_SKP = Amount of User-defined data before the start of the L2 data
 */
union cvmx_bgxx_gmp_gmi_rxx_udd_skp {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_rxx_udd_skp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t fcssel                       : 1;  /**< Include the skip bytes in the FCS calculation.
                                                         0 = all skip bytes are included in FCS
                                                         1 = the skip bytes are not included in FCS
                                                         The skip bytes are part of the packet and are sent through the IOI packet interface and
                                                         are handled by PKI. The system can determine if the UDD bytes are included in the FCS
                                                         check by using the FCSSEL field, if the FCS check is enabled. */
	uint64_t reserved_7_7                 : 1;
	uint64_t len                          : 7;  /**< Amount of user-defined data before the start of the L2C data, in bytes.
                                                         Setting to 0 means L2C comes first; maximum value is 64.
                                                         LEN must be 0x0 in half-duplex operation. */
#else
	uint64_t len                          : 7;
	uint64_t reserved_7_7                 : 1;
	uint64_t fcssel                       : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_rxx_udd_skp_s cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_rxx_udd_skp cvmx_bgxx_gmp_gmi_rxx_udd_skp_t;

/**
 * cvmx_bgx#_gmp_gmi_smac#
 */
union cvmx_bgxx_gmp_gmi_smacx {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_smacx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t smac                         : 48; /**< The SMAC field is used for generating and accepting control PAUSE packets. */
#else
	uint64_t smac                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_smacx_s      cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_smacx cvmx_bgxx_gmp_gmi_smacx_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_append
 */
union cvmx_bgxx_gmp_gmi_txx_append {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_txx_append_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t force_fcs                    : 1;  /**< Append the Ethernet FCS on each PAUSE packet. PAUSE packets are normally padded to 60
                                                         bytes. If BGX(0..5)_GMP_GMI_TX(0..3)_MIN_PKT[MIN_SIZE] exceeds 59, then FCS_C is not used. */
	uint64_t fcs                          : 1;  /**< Append the Ethernet FCS on each packet. */
	uint64_t pad                          : 1;  /**< Append PAD bytes such that minimum-sized packet is transmitted. */
	uint64_t preamble                     : 1;  /**< Prepend the Ethernet preamble on each transfer. */
#else
	uint64_t preamble                     : 1;
	uint64_t pad                          : 1;
	uint64_t fcs                          : 1;
	uint64_t force_fcs                    : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_append_s cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_txx_append cvmx_bgxx_gmp_gmi_txx_append_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_burst
 */
union cvmx_bgxx_gmp_gmi_txx_burst {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_txx_burst_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t burst                        : 16; /**< Burst (refer to 802.3 to set correctly). Only valid for 1000Mb/s half-duplex operation as
                                                         follows:
                                                         half duplex/1000Mb/s: 0x2000
                                                         all other modes: 0x0
                                                         SGMII/1000Base-X only. */
#else
	uint64_t burst                        : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_burst_s  cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_txx_burst cvmx_bgxx_gmp_gmi_txx_burst_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_ctl
 */
union cvmx_bgxx_gmp_gmi_txx_ctl {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_txx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t xsdef_en                     : 1;  /**< Enables the excessive-deferral check for statistics and interrupts. SGMII/1000Base-X half-
                                                         duplex only. */
	uint64_t xscol_en                     : 1;  /**< Enables the excessive-collision check for statistics and interrupts. SGMII/1000Base-X
                                                         half-duplex only. */
#else
	uint64_t xscol_en                     : 1;
	uint64_t xsdef_en                     : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_ctl_s    cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_txx_ctl cvmx_bgxx_gmp_gmi_txx_ctl_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_int
 */
union cvmx_bgxx_gmp_gmi_txx_int {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_txx_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t ptp_lost                     : 1;  /**< A packet with a PTP request was not able to be sent due to XSCOL. */
	uint64_t late_col                     : 1;  /**< TX late collision. (SGMII/1000BASE-X half-duplex only) */
	uint64_t xsdef                        : 1;  /**< TX excessive deferral. (SGMII/1000BASE-X half-duplex only) */
	uint64_t xscol                        : 1;  /**< TX excessive collisions. (SGMII/1000BASE-X half-duplex only) */
	uint64_t undflw                       : 1;  /**< TX underflow. */
#else
	uint64_t undflw                       : 1;
	uint64_t xscol                        : 1;
	uint64_t xsdef                        : 1;
	uint64_t late_col                     : 1;
	uint64_t ptp_lost                     : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_int_s    cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_txx_int cvmx_bgxx_gmp_gmi_txx_int_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_min_pkt
 */
union cvmx_bgxx_gmp_gmi_txx_min_pkt {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_txx_min_pkt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t min_size                     : 8;  /**< Minimum frame size in bytes before the FCS is applied.
                                                         Padding is only appended when BGX(0..5)_GMP_GMI_TX(0..3)_APPEND[PAD] for the corresponding
                                                         LMAC is set.
                                                         In SGMII mode, packets are padded to MIN_SIZE+1. The reset value pads to 60 bytes. */
#else
	uint64_t min_size                     : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_min_pkt_s cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_txx_min_pkt cvmx_bgxx_gmp_gmi_txx_min_pkt_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_pause_pkt_interval
 *
 * Notes:
 * Choosing proper values of BGX_GMP_GMI_TX_PAUSE_PKT_TIME[TIME] and
 * BGX_GMP_GMI_TX_PAUSE_PKT_INTERVAL[INTERVAL] can be challenging to the system
 * designer.  It is suggested that TIME be much greater than INTERVAL and
 * BGX_GMP_GMI_TX_PAUSE_ZERO[SEND] be set.  This allows a periodic refresh of the PAUSE
 * count and then when the backpressure condition is lifted, a PAUSE packet
 * with TIME==0 will be sent indicating that Octane is ready for additional
 * data.
 * If the system chooses to not set BGX_GMP_GMI_TX_PAUSE_ZERO[SEND], then it is
 * suggested that TIME and INTERVAL are programmed such that they satisify the
 * following rule...
 * INTERVAL <= TIME - (largest_pkt_size + IFG + pause_pkt_size)
 * where largest_pkt_size is that largest packet that the system can send
 * (normally 1518B), IFG is the interframe gap and pause_pkt_size is the size
 * of the PAUSE packet (normally 64B).
 */
union cvmx_bgxx_gmp_gmi_txx_pause_pkt_interval {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_txx_pause_pkt_interval_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t interval                     : 16; /**< Arbitrate for a 802.3 PAUSE packet or CBFC PAUSE packet every (INTERVAL * 512) bit-times.
                                                         Normally, 0 < INTERVAL < BGX(0..5)_GMP_GMI_TX(0..3)_PAUSE_PKT_TIME[TIME].
                                                         INTERVAL = 0 only sends a single PAUSE packet for each backpressure event. */
#else
	uint64_t interval                     : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_pause_pkt_interval_s cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_txx_pause_pkt_interval cvmx_bgxx_gmp_gmi_txx_pause_pkt_interval_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_pause_pkt_time
 */
union cvmx_bgxx_gmp_gmi_txx_pause_pkt_time {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_txx_pause_pkt_time_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t ptime                        : 16; /**< Provides the pause_time field placed in outbound 802.3 PAUSE packets or CBFC PAUSE packets
                                                         in 512 bit-times. Normally, P_TIME >
                                                         BGX(0..5)_GMP_GMI_TX(0..3)_PAUSE_PKT_INTERVAL[INTERVAL]. For programming information see
                                                         BGX(0..5)_GMP_GMI_TX(0..3)_PAUSE_PKT_INTERVAL. */
#else
	uint64_t ptime                        : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_pause_pkt_time_s cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_txx_pause_pkt_time cvmx_bgxx_gmp_gmi_txx_pause_pkt_time_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_pause_togo
 */
union cvmx_bgxx_gmp_gmi_txx_pause_togo {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_txx_pause_togo_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t ptime                        : 16; /**< Amount of time remaining to backpressure, from the standard 802.3 PAUSE timer. */
#else
	uint64_t ptime                        : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_pause_togo_s cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_txx_pause_togo cvmx_bgxx_gmp_gmi_txx_pause_togo_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_pause_zero
 */
union cvmx_bgxx_gmp_gmi_txx_pause_zero {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_txx_pause_zero_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t send                         : 1;  /**< Send PAUSE-zero enable.When this bit is set, and the backpressure condition is clear, it
                                                         allows sending a PAUSE packet with pause_time of 0 to enable the channel. */
#else
	uint64_t send                         : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_pause_zero_s cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_txx_pause_zero cvmx_bgxx_gmp_gmi_txx_pause_zero_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_sgmii_ctl
 */
union cvmx_bgxx_gmp_gmi_txx_sgmii_ctl {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_txx_sgmii_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t align                        : 1;  /**< Align the transmission to even cycles: (SGMII/1000BASE-X half-duplex only)
                                                         Recommended value is: ALIGN = !BGXn_GMP_GMI_TXm_APPEND[PREAMBLE].
                                                         (See Transmit Conversion to Code groups, Transmit Conversion to Code Groups for a complete
                                                         discussion.)
                                                         0 = Data can be sent on any cycle. In this mode, the interface functions at maximum
                                                         bandwidth. It is possible for the TX PCS machine to drop the first byte of the TX frame.
                                                         When BGXn_GMP_GMI_TXm_APPEND[PREAMBLE] is set, the first byte is a preamble byte, which
                                                         can be dropped to compensate for an extended IPG.
                                                         1 = Data is only sent on even cycles. In this mode, there can be bandwidth implications
                                                         when sending odd-byte packets as the IPG can extend an extra cycle. There will be no loss
                                                         of data. */
#else
	uint64_t align                        : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_sgmii_ctl_s cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_txx_sgmii_ctl cvmx_bgxx_gmp_gmi_txx_sgmii_ctl_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_slot
 */
union cvmx_bgxx_gmp_gmi_txx_slot {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_txx_slot_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t slot                         : 10; /**< Slottime (refer to Std 802.3 to set correctly):
                                                         10/100Mbs: set SLOT to 0x40
                                                         1000Mbs: set SLOT to 0x200
                                                         SGMII/1000Base-X only. */
#else
	uint64_t slot                         : 10;
	uint64_t reserved_10_63               : 54;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_slot_s   cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_txx_slot cvmx_bgxx_gmp_gmi_txx_slot_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_soft_pause
 */
union cvmx_bgxx_gmp_gmi_txx_soft_pause {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_txx_soft_pause_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t ptime                        : 16; /**< Back off the TX bus for (PTIME * 512) bit-times. */
#else
	uint64_t ptime                        : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_soft_pause_s cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_txx_soft_pause cvmx_bgxx_gmp_gmi_txx_soft_pause_t;

/**
 * cvmx_bgx#_gmp_gmi_tx#_thresh
 */
union cvmx_bgxx_gmp_gmi_txx_thresh {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_txx_thresh_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t cnt                          : 11; /**< Number of 128-bit words to accumulate in the TX FIFO before sending on the packet
                                                         interface. This field should be large enough to prevent underflow on the packet interface
                                                         and must never be set to 0x0.
                                                         10G/40G Mode, CNT = 0x100. In all modes, this register cannot exceed the TX FIFO depth as
                                                         follows.
                                                         BGX*_CMR*_TX_LMACS = 0,1:  CNT maximum = 0x7FF
                                                         BGX*_CMR*_TX_LMACS = 2:     CNT maximum = 0x3FF
                                                         BGX*_CMR*_TX_LMACS = 3,4:  CNT maximum = 0x1FF */
#else
	uint64_t cnt                          : 11;
	uint64_t reserved_11_63               : 53;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_txx_thresh_s cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_txx_thresh cvmx_bgxx_gmp_gmi_txx_thresh_t;

/**
 * cvmx_bgx#_gmp_gmi_tx_col_attempt
 */
union cvmx_bgxx_gmp_gmi_tx_col_attempt {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_tx_col_attempt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t limit                        : 5;  /**< Number of collision attempts allowed. (SGMII/1000BASE-X half-duplex only.) */
#else
	uint64_t limit                        : 5;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_tx_col_attempt_s cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_tx_col_attempt cvmx_bgxx_gmp_gmi_tx_col_attempt_t;

/**
 * cvmx_bgx#_gmp_gmi_tx_ifg
 *
 * Consider the following when programming IFG1 and IFG2:
 * For 10/100/1000 Mb/s half-duplex systems that require IEEE 802.3 compatibility, IFG1 must be
 * in the range of 1-8, IFG2 must be in the range of 4-12, and the IFG1 + IFG2 sum must be 12.
 * For 10/100/1000 Mb/s full-duplex systems that require IEEE 802.3 compatibility, IFG1 must be
 * in the range of 1-11, IFG2 must be in the range of 1-11, and the IFG1 + IFG2 sum must be 12.
 * For all other systems, IFG1 and IFG2 can be any value in the range of 1-15, allowing for a
 * total possible IFG sum of 2-30.
 */
union cvmx_bgxx_gmp_gmi_tx_ifg {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_tx_ifg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t ifg2                         : 4;  /**< Remainder of interFrameGap timing, equal to interFrameGap - IFG1 (in IFG2 * 8 bits). If
                                                         CRS is detected during IFG2, the interFrameSpacing timer is not reset and a frame is
                                                         transmitted once the timer expires. */
	uint64_t ifg1                         : 4;  /**< First portion of interFrameGap timing, in the range of 0 to 2/3 (in IFG2 * 8 bits). If CRS
                                                         is detected during IFG1, the interFrameSpacing timer is reset and a frame is not
                                                         transmitted. */
#else
	uint64_t ifg1                         : 4;
	uint64_t ifg2                         : 4;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_tx_ifg_s     cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_tx_ifg cvmx_bgxx_gmp_gmi_tx_ifg_t;

/**
 * cvmx_bgx#_gmp_gmi_tx_jam
 *
 * This register provides the pattern used in JAM bytes.
 *
 */
union cvmx_bgxx_gmp_gmi_tx_jam {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_tx_jam_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t jam                          : 8;  /**< JAM pattern. (SGMII/1000BASE-X half-duplex only.) */
#else
	uint64_t jam                          : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_tx_jam_s     cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_tx_jam cvmx_bgxx_gmp_gmi_tx_jam_t;

/**
 * cvmx_bgx#_gmp_gmi_tx_lfsr
 *
 * This register shows the contents of the linear feedback shift register (LFSR), which is used
 * to implement truncated binary exponential backoff.
 */
union cvmx_bgxx_gmp_gmi_tx_lfsr {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_tx_lfsr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t lfsr                         : 16; /**< Contains the current state of the LFSR, which is used to feed random numbers to compute
                                                         truncated binary exponential backoff. (SGMII/1000Base-X half-duplex only.) */
#else
	uint64_t lfsr                         : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_tx_lfsr_s    cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_tx_lfsr cvmx_bgxx_gmp_gmi_tx_lfsr_t;

/**
 * cvmx_bgx#_gmp_gmi_tx_pause_pkt_dmac
 */
union cvmx_bgxx_gmp_gmi_tx_pause_pkt_dmac {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_tx_pause_pkt_dmac_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t dmac                         : 48; /**< The DMAC field, which is placed is outbound PAUSE packets. */
#else
	uint64_t dmac                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_tx_pause_pkt_dmac_s cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_tx_pause_pkt_dmac cvmx_bgxx_gmp_gmi_tx_pause_pkt_dmac_t;

/**
 * cvmx_bgx#_gmp_gmi_tx_pause_pkt_type
 *
 * This register provides the PTYPE field that is placed in outbound PAUSE packets.
 *
 */
union cvmx_bgxx_gmp_gmi_tx_pause_pkt_type {
	uint64_t u64;
	struct cvmx_bgxx_gmp_gmi_tx_pause_pkt_type_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t ptype                        : 16; /**< The PTYPE field placed in outbound PAUSE packets. */
#else
	uint64_t ptype                        : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_gmp_gmi_tx_pause_pkt_type_s cn78xx;
};
typedef union cvmx_bgxx_gmp_gmi_tx_pause_pkt_type cvmx_bgxx_gmp_gmi_tx_pause_pkt_type_t;

/**
 * cvmx_bgx#_gmp_pcs_an#_adv
 */
union cvmx_bgxx_gmp_pcs_anx_adv {
	uint64_t u64;
	struct cvmx_bgxx_gmp_pcs_anx_adv_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t np                           : 1;  /**< Next page capable. This feature is not supported; this field is always 0. */
	uint64_t reserved_14_14               : 1;
	uint64_t rem_flt                      : 2;  /**< Remote fault.
                                                         00 = Link OK, XMIT = DATA
                                                         01 = Link failure (loss of sync, XMIT !=DATA)
                                                         10 = Local device offline
                                                         11 = Auto-Negotiation error; failure to complete Auto-Negotiation. AN error is set if
                                                         resolution function precludes operation with link partner. */
	uint64_t reserved_9_11                : 3;
	uint64_t pause                        : 2;  /**< PAUSE frame flow capability across link, exchanged during Auto-Negotiation as follows:
                                                         00 = No PAUSE.
                                                         01 = Symmetric PAUSE.
                                                         10 = Asymmetric PAUSE.
                                                         11 = Both symmetric and asymmetric PAUSE to local device. */
	uint64_t hfd                          : 1;  /**< Half-duplex. When set, local device is half-duplex capable. */
	uint64_t fd                           : 1;  /**< Full-duplex. When set, local device is full-duplex capable. */
	uint64_t reserved_0_4                 : 5;
#else
	uint64_t reserved_0_4                 : 5;
	uint64_t fd                           : 1;
	uint64_t hfd                          : 1;
	uint64_t pause                        : 2;
	uint64_t reserved_9_11                : 3;
	uint64_t rem_flt                      : 2;
	uint64_t reserved_14_14               : 1;
	uint64_t np                           : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_gmp_pcs_anx_adv_s    cn78xx;
};
typedef union cvmx_bgxx_gmp_pcs_anx_adv cvmx_bgxx_gmp_pcs_anx_adv_t;

/**
 * cvmx_bgx#_gmp_pcs_an#_ext_st
 */
union cvmx_bgxx_gmp_pcs_anx_ext_st {
	uint64_t u64;
	struct cvmx_bgxx_gmp_pcs_anx_ext_st_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t thou_xfd                     : 1;  /**< When set, PHY is 1000 BASE-X full duplex capable. */
	uint64_t thou_xhd                     : 1;  /**< When set, PHY is 1000 BASE-X half duplex capable. */
	uint64_t thou_tfd                     : 1;  /**< When set, PHY is 1000 BASE-T full duplex capable. */
	uint64_t thou_thd                     : 1;  /**< When set, PHY is 1000 BASE-T half duplex capable. */
	uint64_t reserved_0_11                : 12;
#else
	uint64_t reserved_0_11                : 12;
	uint64_t thou_thd                     : 1;
	uint64_t thou_tfd                     : 1;
	uint64_t thou_xhd                     : 1;
	uint64_t thou_xfd                     : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_gmp_pcs_anx_ext_st_s cn78xx;
};
typedef union cvmx_bgxx_gmp_pcs_anx_ext_st cvmx_bgxx_gmp_pcs_anx_ext_st_t;

/**
 * cvmx_bgx#_gmp_pcs_an#_lp_abil
 *
 * This is the Auto-Negotiation Link partner ability register 5 as per IEEE 802.3, Clause 37.
 *
 */
union cvmx_bgxx_gmp_pcs_anx_lp_abil {
	uint64_t u64;
	struct cvmx_bgxx_gmp_pcs_anx_lp_abil_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t np                           : 1;  /**< 0 = Link partner not next page capable.
                                                         1 = Link partner next page capable. */
	uint64_t ack                          : 1;  /**< When set, indicates acknowledgement received. */
	uint64_t rem_flt                      : 2;  /**< Link partner's link status as follows:
                                                         00 = Link OK.
                                                         01 = Offline.
                                                         10 = Link failure.
                                                         11 = Auto-Negotiation error. */
	uint64_t reserved_9_11                : 3;
	uint64_t pause                        : 2;  /**< Link partner PAUSE setting as follows:
                                                         00 = No PAUSE.
                                                         01 = Symmetric PAUSE.
                                                         10 = Asymmetric PAUSE.
                                                         11 = Both symmetric and asymmetric PAUSE to local device. */
	uint64_t hfd                          : 1;  /**< Half-duplex. When set, link partner is half-duplex capable. */
	uint64_t fd                           : 1;  /**< Full-duplex. When set, link partner is full-duplex capable. */
	uint64_t reserved_0_4                 : 5;
#else
	uint64_t reserved_0_4                 : 5;
	uint64_t fd                           : 1;
	uint64_t hfd                          : 1;
	uint64_t pause                        : 2;
	uint64_t reserved_9_11                : 3;
	uint64_t rem_flt                      : 2;
	uint64_t ack                          : 1;
	uint64_t np                           : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_gmp_pcs_anx_lp_abil_s cn78xx;
};
typedef union cvmx_bgxx_gmp_pcs_anx_lp_abil cvmx_bgxx_gmp_pcs_anx_lp_abil_t;

/**
 * cvmx_bgx#_gmp_pcs_an#_results
 *
 * This register is not valid when BGX(0..5)_GMP_PCS_MR(0..3)_CONTROL[AN_OVRD] is set to 1. If
 * BGX(0..5)_GMP_PCS_MR(0..3)_CONTROL[AN_OVRD] is set to 0 and
 * BGX(0..5)_GMP_PCS_AN(0..3)_RESULTS[AN_CPT] is set to 1, this register is valid.
 */
union cvmx_bgxx_gmp_pcs_anx_results {
	uint64_t u64;
	struct cvmx_bgxx_gmp_pcs_anx_results_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t pause                        : 2;  /**< PAUSE selection ('don't care' for SGMII) as follows:
                                                         00 = Disable PAUSE, TX and RX.
                                                         01 = Enable PAUSE frames, RX only.
                                                         10 = Enable PAUSE frames, TX only.
                                                         11 = Enable PAUSE frames, TX and RX. */
	uint64_t spd                          : 2;  /**< Link speed selection as follows:
                                                         00 = 10 Mb/s.
                                                         01 = 100 Mb/s.
                                                         10 = 1000 Mb/s.
                                                         11 = Reserved. */
	uint64_t an_cpt                       : 1;  /**< Auto-Negotiation completed.
                                                         1 = Auto-Negotiation completed.
                                                         0 = Auto-Negotiation not completed or failed. */
	uint64_t dup                          : 1;  /**< Duplex mode. 1 = full duplex, 0 = half duplex. */
	uint64_t link_ok                      : 1;  /**< Link status: 1 = link up (OK), 1 = link down. */
#else
	uint64_t link_ok                      : 1;
	uint64_t dup                          : 1;
	uint64_t an_cpt                       : 1;
	uint64_t spd                          : 2;
	uint64_t pause                        : 2;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_bgxx_gmp_pcs_anx_results_s cn78xx;
};
typedef union cvmx_bgxx_gmp_pcs_anx_results cvmx_bgxx_gmp_pcs_anx_results_t;

/**
 * cvmx_bgx#_gmp_pcs_int#
 */
union cvmx_bgxx_gmp_pcs_intx {
	uint64_t u64;
	struct cvmx_bgxx_gmp_pcs_intx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_13_63               : 51;
	uint64_t dbg_sync                     : 1;  /**< Code group sync failure debug help. DBG_SYNC interrupt fires when code group
                                                         synchronization state machine makes a transition from SYNC_ACQUIRED_1 state to
                                                         SYNC_ACQUIRED_2 state. (See IEEE 802.3-2005, figure 37-9). It indicates that a bad code
                                                         group was received after code group synchronization was achieved. This interrupt should be
                                                         disabled during normal link operation. Use it as a debug help feature only. */
	uint64_t dup                          : 1;  /**< Set whenever duplex mode changes on the link. */
	uint64_t sync_bad                     : 1;  /**< Set by hardware whenever RX sync state machine reaches a bad state. Should never be set
                                                         during normal operation. */
	uint64_t an_bad                       : 1;  /**< Set by hardware whenever Auto-Negotiation state machine reaches a bad state. Should never
                                                         be set during normal operation. */
	uint64_t rxlock                       : 1;  /**< Set by hardware whenever code group sync or bit lock failure occurs. Cannot fire in loopback1 mode. */
	uint64_t rxbad                        : 1;  /**< Set by hardware whenever RX state machine reaches a bad state. Should never be set during
                                                         normal operation. */
	uint64_t rxerr                        : 1;  /**< Set whenever RX receives a code group error in 10-bit to 8-bit decode logic. Cannot fire
                                                         in loopback1 mode. */
	uint64_t txbad                        : 1;  /**< Set by hardware whenever TX state machine reaches a bad state. Should never be set during
                                                         normal operation. */
	uint64_t txfifo                       : 1;  /**< Set whenever hardware detects a TX FIFO overflow condition. */
	uint64_t txfifu                       : 1;  /**< Set whenever hardware detects a TX FIFO underflow condition. */
	uint64_t an_err                       : 1;  /**< Auto-Negotiation error; AN resolution function failed. */
	uint64_t xmit                         : 1;  /**< Set whenever hardware detects a change in the XMIT variable. XMIT variable states are
                                                         IDLE, CONFIG and DATA. */
	uint64_t lnkspd                       : 1;  /**< Set by hardware whenever link speed has changed. */
#else
	uint64_t lnkspd                       : 1;
	uint64_t xmit                         : 1;
	uint64_t an_err                       : 1;
	uint64_t txfifu                       : 1;
	uint64_t txfifo                       : 1;
	uint64_t txbad                        : 1;
	uint64_t rxerr                        : 1;
	uint64_t rxbad                        : 1;
	uint64_t rxlock                       : 1;
	uint64_t an_bad                       : 1;
	uint64_t sync_bad                     : 1;
	uint64_t dup                          : 1;
	uint64_t dbg_sync                     : 1;
	uint64_t reserved_13_63               : 51;
#endif
	} s;
	struct cvmx_bgxx_gmp_pcs_intx_s       cn78xx;
};
typedef union cvmx_bgxx_gmp_pcs_intx cvmx_bgxx_gmp_pcs_intx_t;

/**
 * cvmx_bgx#_gmp_pcs_link#_timer
 *
 * This is the 1.6 ms nominal Link timer register.
 *
 */
union cvmx_bgxx_gmp_pcs_linkx_timer {
	uint64_t u64;
	struct cvmx_bgxx_gmp_pcs_linkx_timer_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t count                        : 16; /**< (Coprocessor clock period * 1024) * COUNT should be 1.6 ms for SGMII and 10 ms otherwise,
                                                         which is the link timer used in Auto-Negotiation. Reset assumes a 700 MHz coprocessor
                                                         clock for 1.6 ms link timer. */
#else
	uint64_t count                        : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_gmp_pcs_linkx_timer_s cn78xx;
};
typedef union cvmx_bgxx_gmp_pcs_linkx_timer cvmx_bgxx_gmp_pcs_linkx_timer_t;

/**
 * cvmx_bgx#_gmp_pcs_misc#_ctl
 *
 * SGMII bit [12] is really a misnomer, it is a decode  of pi_qlm_cfg pins to indicate SGMII or
 * 1000Base-X modes.
 * Note: MODE bit
 * When MODE=1,  1000Base-X mode is selected. Auto negotiation will follow IEEE 802.3 clause 37.
 * When MODE=0,  SGMII mode is selected and the following note will apply.
 * Repeat note from SGM_AN_ADV register
 * NOTE: The SGMII AN Advertisement Register above will be sent during Auto Negotiation if the
 * MAC_PHY mode bit in misc_ctl_reg
 * is set (1=PHY mode). If the bit is not set (0=MAC mode), the tx_config_reg[14] becomes ACK bit
 * and [0] is always 1.
 * All other bits in tx_config_reg sent will be 0. The PHY dictates the Auto Negotiation results.
 * SGMII Misc Control Register
 */
union cvmx_bgxx_gmp_pcs_miscx_ctl {
	uint64_t u64;
	struct cvmx_bgxx_gmp_pcs_miscx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_13_63               : 51;
	uint64_t sgmii                        : 1;  /**< SGMII mode. 1 = SGMII or 1000BASE-X mode selected, 0 = other mode selected. See
                                                         GSERx_LANE_MODE[LMODE]. */
	uint64_t gmxeno                       : 1;  /**< GMI enable override. When set, forces GMI to appear disabled. The enable/disable status of
                                                         GMI is checked only at SOP of every packet. */
	uint64_t loopbck2                     : 1;  /**< Sets external loopback mode to return RX data back out via the TX data path. 0 = No
                                                         loopback, 1 = Loopback.
                                                         LOOPBCK1 and LOOPBCK2 modes may not be supported simultaneously. */
	uint64_t mac_phy                      : 1;  /**< MAC/PHY.
                                                         0 = MAC, 1 = PHY decides the TX_CONFIG_REG value to be sent during Auto-Negotiation. */
	uint64_t mode                         : 1;  /**< Mode bit. 0 = SGMII, 1 = 1000Base X.
                                                         1 = 1000Base-X mode is selected. Auto-Negotiation follows IEEE 802.3 clause 37.
                                                         0 = SGMII mode is selected and the following note applies.
                                                         The SGMII AN advertisement register (BGX(0..5)_GMP_PCS_SGM(0..3)_AN_ADV) is sent during
                                                         Auto-Negotiation if BGX(0..5)_GMP_PCS_MISC(0..3)_CTL[MAC_PHY] = 1 (PHY mode). If [MAC_PHY]
                                                         = 0 (MAC mode), the TX_CONFIG_REG<14> becomes ACK bit and <0> is always 1. All other bits
                                                         in TX_CONFIG_REG sent are 0. The PHY dictates the Auto-Negotiation results. */
	uint64_t an_ovrd                      : 1;  /**< Auto-Negotiation results override: 1 = enable override, 0 = disable.
                                                         Auto-Negotiation is allowed to happen but the results are ignored when this bit is set.
                                                         Duplex and Link speed values are set from BGX(0..5)_GMP_PCS_MISC(0..3)_CTL. */
	uint64_t samp_pt                      : 7;  /**< Byte number in elongated frames for 10/100Mb/s operation for data sampling on RX side in
                                                         PCS. Recommended values are 0x5 for
                                                         100Mb/s operation and 0x32 for 10Mb/s operation.
                                                         For 10Mb/s operation, this field should be set to a value less than 99 and greater than 0.
                                                         If set out of this range, a value of 50 is used for actual sampling internally without
                                                         affecting the CSR field.
                                                         For 100Mb/s operation this field should be set to a value less than 9 and greater than 0.
                                                         If set out of this range, a value of 5 is used for actual sampling internally without
                                                         affecting the CSR field. */
#else
	uint64_t samp_pt                      : 7;
	uint64_t an_ovrd                      : 1;
	uint64_t mode                         : 1;
	uint64_t mac_phy                      : 1;
	uint64_t loopbck2                     : 1;
	uint64_t gmxeno                       : 1;
	uint64_t sgmii                        : 1;
	uint64_t reserved_13_63               : 51;
#endif
	} s;
	struct cvmx_bgxx_gmp_pcs_miscx_ctl_s  cn78xx;
};
typedef union cvmx_bgxx_gmp_pcs_miscx_ctl cvmx_bgxx_gmp_pcs_miscx_ctl_t;

/**
 * cvmx_bgx#_gmp_pcs_mr#_control
 */
union cvmx_bgxx_gmp_pcs_mrx_control {
	uint64_t u64;
	struct cvmx_bgxx_gmp_pcs_mrx_control_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t reset                        : 1;  /**< Set to reset. 1 = software PCS reset, 0 = normal operation.
                                                         The bit returns to 0 after PCS has been reset. Takes 32 coprocessor-clock cycles to reset
                                                         PCS. */
	uint64_t loopbck1                     : 1;  /**< Enable loopback: 1 = internal loopback mode, 0 = normal operation
                                                         The loopback mode returns loopback TX data from GMII TX back to GMII RX interface. The
                                                         loopback happens in the PCS module. Auto-Negotiation is disabled even if AN_EN is set
                                                         during loopback. */
	uint64_t spdlsb                       : 1;  /**< Least-significant bit of the link-speed field, i.e. SPD<0>. Refer to SPDMSB. */
	uint64_t an_en                        : 1;  /**< Auto-Negotiation enable: 1 = enable, 0 = disable. */
	uint64_t pwr_dn                       : 1;  /**< Power down: 1 = power down (hardware reset), 0 = normal operation. */
	uint64_t reserved_10_10               : 1;
	uint64_t rst_an                       : 1;  /**< Reset Auto-Negotiation. When set, if AN_EN = 1 and
                                                         BGX(0..5)_GMP_PCS_MR(0..3)_STATUS[AN_ABIL] = 1, Auto-Negotiation begins. Otherwise,
                                                         software write requests are ignored and this bit remains at 0. This bit clears itself to
                                                         0, when Auto-Negotiation starts. */
	uint64_t dup                          : 1;  /**< Duplex mode: 1 = full duplex, 0 = half duplex; effective only if Auto-Negotiation is
                                                         disabled. If BGX(0..5)_GMP_PCS_MR(0..3)_STATUS <15:9> and
                                                         BGX(0..5)_GMP_PCS_AN(0..3)_ADV<15:12> allow only one duplex mode, this bit corresponds to
                                                         that value and any attempts to write are ignored. */
	uint64_t coltst                       : 1;  /**< COL test: 1 = enable COL signal test, 0 = disable test.
                                                         During COL test, the COL signal reflects the GMII TX_EN signal with less than 16BT delay. */
	uint64_t spdmsb                       : 1;  /**< Link speed most-significant bit, i.e SPD<1>; effective only if Auto-Negotiation is
                                                         disabled.
                                                         SPDMSB SPDLSB Link Speed
                                                         0 0 10 Mb/s
                                                         0 1 100 Mb/s
                                                         1 0 1000 Mb/s
                                                         1 1 reserved */
	uint64_t uni                          : 1;  /**< Unidirectional (Std 802.3-2005, Clause 66.2). When set to 1, this bit overrides AN_EN and
                                                         disables the Auto-Negotiation variable mr_an_enable. Used in both 1000BASE-X and SGMII
                                                         modes. */
	uint64_t reserved_0_4                 : 5;
#else
	uint64_t reserved_0_4                 : 5;
	uint64_t uni                          : 1;
	uint64_t spdmsb                       : 1;
	uint64_t coltst                       : 1;
	uint64_t dup                          : 1;
	uint64_t rst_an                       : 1;
	uint64_t reserved_10_10               : 1;
	uint64_t pwr_dn                       : 1;
	uint64_t an_en                        : 1;
	uint64_t spdlsb                       : 1;
	uint64_t loopbck1                     : 1;
	uint64_t reset                        : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_gmp_pcs_mrx_control_s cn78xx;
};
typedef union cvmx_bgxx_gmp_pcs_mrx_control cvmx_bgxx_gmp_pcs_mrx_control_t;

/**
 * cvmx_bgx#_gmp_pcs_mr#_status
 *
 * Bits <15:9> in this register indicate the ability to operate when
 * BGX(0..5)_GMP_PCS_MISC(0..3)_CTL[MAC_PHY] is set to MAC mode. Bits <15:9> are always read as
 * 0, indicating that the chip cannot operate in the corresponding modes. The field [RM_FLT] is a
 * 'don't care' when the selected mode is SGMII.
 */
union cvmx_bgxx_gmp_pcs_mrx_status {
	uint64_t u64;
	struct cvmx_bgxx_gmp_pcs_mrx_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t hun_t4                       : 1;  /**< Indicates 100BASE-T4 capable. */
	uint64_t hun_xfd                      : 1;  /**< Indicates 100BASE-X full duplex. */
	uint64_t hun_xhd                      : 1;  /**< Indicates 100BASE-X half duplex. */
	uint64_t ten_fd                       : 1;  /**< Indicates 10Mb/s full duplex. */
	uint64_t ten_hd                       : 1;  /**< Indicates 10Mb/s half duplex. */
	uint64_t hun_t2fd                     : 1;  /**< Indicates 100BASE-T2 full duplex. */
	uint64_t hun_t2hd                     : 1;  /**< Indicates 100BASE-T2 half duplex. */
	uint64_t ext_st                       : 1;  /**< Extended status information. When set to 1, indicates that additional status data is
                                                         available in BGX(0..5)_GMP_PCS_AN(0..3)_EXT_ST. */
	uint64_t reserved_7_7                 : 1;
	uint64_t prb_sup                      : 1;  /**< Preamble not needed.
                                                         1 = Can work without preamble bytes at the beginning of frames.
                                                         0 = Cannot accept frames without preamble bytes. */
	uint64_t an_cpt                       : 1;  /**< Indicates Auto-Negotiation is complete; the contents of the
                                                         BGX(0..5)_GMP_PCS_AN(0..3)_RESULTS are valid. */
	uint64_t rm_flt                       : 1;  /**< Indicates remote fault condition occurred. This bit implements a latching-high behavior.
                                                         It is cleared when software reads this register or when
                                                         BGX(0..5)_GMP_PCS_MR(0..3)_CONTROL[RESET] is asserted.
                                                         See BGX(0..5)_GMP_PCS_AN(0..3)_ADV[REM_FLT] for fault conditions. */
	uint64_t an_abil                      : 1;  /**< Indicates Auto-Negotiation capable. */
	uint64_t lnk_st                       : 1;  /**< Link state: 0 = link down, 1 = link up.
                                                         Set during Auto-Negotiation process. Set whenever XMIT = DATA. Latching-low behavior when
                                                         link goes down. Link down value of the bit stays low until software reads the register. */
	uint64_t reserved_1_1                 : 1;
	uint64_t extnd                        : 1;  /**< This field is always 0, extended capability registers not present. */
#else
	uint64_t extnd                        : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t lnk_st                       : 1;
	uint64_t an_abil                      : 1;
	uint64_t rm_flt                       : 1;
	uint64_t an_cpt                       : 1;
	uint64_t prb_sup                      : 1;
	uint64_t reserved_7_7                 : 1;
	uint64_t ext_st                       : 1;
	uint64_t hun_t2hd                     : 1;
	uint64_t hun_t2fd                     : 1;
	uint64_t ten_hd                       : 1;
	uint64_t ten_fd                       : 1;
	uint64_t hun_xhd                      : 1;
	uint64_t hun_xfd                      : 1;
	uint64_t hun_t4                       : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_gmp_pcs_mrx_status_s cn78xx;
};
typedef union cvmx_bgxx_gmp_pcs_mrx_status cvmx_bgxx_gmp_pcs_mrx_status_t;

/**
 * cvmx_bgx#_gmp_pcs_rx#_states
 */
union cvmx_bgxx_gmp_pcs_rxx_states {
	uint64_t u64;
	struct cvmx_bgxx_gmp_pcs_rxx_states_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t rx_bad                       : 1;  /**< Receive state machine is in an illegal state. */
	uint64_t rx_st                        : 5;  /**< Receive state-machine state. */
	uint64_t sync_bad                     : 1;  /**< Receive synchronization state machine is in an illegal state. */
	uint64_t sync                         : 4;  /**< Receive synchronization state-machine state. */
	uint64_t an_bad                       : 1;  /**< Auto-Negotiation state machine is in an illegal state. */
	uint64_t an_st                        : 4;  /**< Auto-Negotiation state-machine state. */
#else
	uint64_t an_st                        : 4;
	uint64_t an_bad                       : 1;
	uint64_t sync                         : 4;
	uint64_t sync_bad                     : 1;
	uint64_t rx_st                        : 5;
	uint64_t rx_bad                       : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_gmp_pcs_rxx_states_s cn78xx;
};
typedef union cvmx_bgxx_gmp_pcs_rxx_states cvmx_bgxx_gmp_pcs_rxx_states_t;

/**
 * cvmx_bgx#_gmp_pcs_rx#_sync
 */
union cvmx_bgxx_gmp_pcs_rxx_sync {
	uint64_t u64;
	struct cvmx_bgxx_gmp_pcs_rxx_sync_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t sync                         : 1;  /**< When set, code group synchronization achieved. */
	uint64_t bit_lock                     : 1;  /**< When set, bit lock achieved. */
#else
	uint64_t bit_lock                     : 1;
	uint64_t sync                         : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_bgxx_gmp_pcs_rxx_sync_s   cn78xx;
};
typedef union cvmx_bgxx_gmp_pcs_rxx_sync cvmx_bgxx_gmp_pcs_rxx_sync_t;

/**
 * cvmx_bgx#_gmp_pcs_sgm#_an_adv
 *
 * This is the SGMII Auto-Negotiation advertisement register (sent out as TX_CONFIG_REG). This
 * register is sent during Auto-Negotiation if
 * BGX(0..5)_GMP_PCS_MISC(0..3)_CTL[MAC_PHY] is set (1 = PHY mode). If the bit is not set (0 =
 * MAC mode), the TX_CONFIG_REG<14> becomes ACK bit and <0> is always 1. All other bits in
 * TX_CONFIG_REG sent will be 0. The PHY dictates the Auto-Negotiation results.
 */
union cvmx_bgxx_gmp_pcs_sgmx_an_adv {
	uint64_t u64;
	struct cvmx_bgxx_gmp_pcs_sgmx_an_adv_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t link                         : 1;  /**< Link status: 1 = Link up, 0 = Link down. */
	uint64_t ack                          : 1;  /**< Auto-Negotiation acknowledgement. */
	uint64_t reserved_13_13               : 1;
	uint64_t dup                          : 1;  /**< Duplex mode: 1 = full duplex, 0 = half duplex */
	uint64_t speed                        : 2;  /**< Link speed:
                                                         00 = 10 Mb/s.
                                                         01 = 100 Mb/s.
                                                         10 = 1000 Mb/s.
                                                         11 = Reserved. */
	uint64_t reserved_1_9                 : 9;
	uint64_t one                          : 1;  /**< Always set to match TX_CONFIG_REG<0>. */
#else
	uint64_t one                          : 1;
	uint64_t reserved_1_9                 : 9;
	uint64_t speed                        : 2;
	uint64_t dup                          : 1;
	uint64_t reserved_13_13               : 1;
	uint64_t ack                          : 1;
	uint64_t link                         : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_gmp_pcs_sgmx_an_adv_s cn78xx;
};
typedef union cvmx_bgxx_gmp_pcs_sgmx_an_adv cvmx_bgxx_gmp_pcs_sgmx_an_adv_t;

/**
 * cvmx_bgx#_gmp_pcs_sgm#_lp_adv
 *
 * This is the SGMII Link partner advertisement register (received as RX_CONFIG_REG).
 *
 */
union cvmx_bgxx_gmp_pcs_sgmx_lp_adv {
	uint64_t u64;
	struct cvmx_bgxx_gmp_pcs_sgmx_lp_adv_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t link                         : 1;  /**< Link status: 1 = Link up, 0 = Link down. */
	uint64_t reserved_13_14               : 2;
	uint64_t dup                          : 1;  /**< Duplex mode: 1 = Full duplex, 0 = Half duplex */
	uint64_t speed                        : 2;  /**< Link speed:
                                                         00 = 10 Mb/s.
                                                         01 = 100 Mb/s.
                                                         10 = 1000 Mb/s.
                                                         11 = Reserved. */
	uint64_t reserved_1_9                 : 9;
	uint64_t one                          : 1;  /**< Always set to match TX_CONFIG_REG<0> */
#else
	uint64_t one                          : 1;
	uint64_t reserved_1_9                 : 9;
	uint64_t speed                        : 2;
	uint64_t dup                          : 1;
	uint64_t reserved_13_14               : 2;
	uint64_t link                         : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_gmp_pcs_sgmx_lp_adv_s cn78xx;
};
typedef union cvmx_bgxx_gmp_pcs_sgmx_lp_adv cvmx_bgxx_gmp_pcs_sgmx_lp_adv_t;

/**
 * cvmx_bgx#_gmp_pcs_tx#_states
 */
union cvmx_bgxx_gmp_pcs_txx_states {
	uint64_t u64;
	struct cvmx_bgxx_gmp_pcs_txx_states_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t xmit                         : 2;  /**< 0x0 = Undefined.
                                                         0x1 = Config.
                                                         0x2 = Idle.
                                                         0x3 = Data */
	uint64_t tx_bad                       : 1;  /**< Transmit state machine in an illegal state. */
	uint64_t ord_st                       : 4;  /**< Transmit ordered set state-machine state. */
#else
	uint64_t ord_st                       : 4;
	uint64_t tx_bad                       : 1;
	uint64_t xmit                         : 2;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_bgxx_gmp_pcs_txx_states_s cn78xx;
};
typedef union cvmx_bgxx_gmp_pcs_txx_states cvmx_bgxx_gmp_pcs_txx_states_t;

/**
 * cvmx_bgx#_gmp_pcs_tx_rx#_polarity
 *
 * BGX(0..5)_GMP_PCS_TX_RX(0..3)_POLARITY[AUTORXPL] shows correct polarity needed on the link
 * receive path after code group synchronization is achieved.
 */
union cvmx_bgxx_gmp_pcs_tx_rxx_polarity {
	uint64_t u64;
	struct cvmx_bgxx_gmp_pcs_tx_rxx_polarity_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t rxovrd                       : 1;  /**< RX polarity override.
                                                         0 = AUTORXPL determines polarity
                                                         1 = RXPLRT determines polarity */
	uint64_t autorxpl                     : 1;  /**< Auto RX polarity detected:
                                                         0 = Normal polarity
                                                         1 = Inverted polarity
                                                         This bit always represents the correct RX polarity setting needed for successful RX path
                                                         operation, once a successful code group sync is obtained. */
	uint64_t rxplrt                       : 1;  /**< RX polarity: 0 = Normal polarity, 1 = Inverted polarity. */
	uint64_t txplrt                       : 1;  /**< TX polarity: 0 = Normal polarity, 1 = Inverted polarity. */
#else
	uint64_t txplrt                       : 1;
	uint64_t rxplrt                       : 1;
	uint64_t autorxpl                     : 1;
	uint64_t rxovrd                       : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_bgxx_gmp_pcs_tx_rxx_polarity_s cn78xx;
};
typedef union cvmx_bgxx_gmp_pcs_tx_rxx_polarity cvmx_bgxx_gmp_pcs_tx_rxx_polarity_t;

/**
 * cvmx_bgx#_smu#_cbfc_ctl
 */
union cvmx_bgxx_smux_cbfc_ctl {
	uint64_t u64;
	struct cvmx_bgxx_smux_cbfc_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t phys_en                      : 16; /**< Physical backpressure enable. Determines which LMACs will have physical backpressure PAUSE
                                                         packets. The value placed in the Class Enable Vector field of the PFC/CBFC PAUSE packet is
                                                         PHYS_EN | LOGL_EN. */
	uint64_t logl_en                      : 16; /**< Logical backpressure enable. Determines which LMACs will have logical backpressure PAUSE
                                                         packets. The value placed in the Class Enable Vector field of the PFC/CBFC PAUSE packet is
                                                         PHYS_EN | LOGL_EN. */
	uint64_t reserved_4_31                : 28;
	uint64_t bck_en                       : 1;  /**< Forward PFC/CBFC PAUSE information to the backpressure block. */
	uint64_t drp_en                       : 1;  /**< Drop-control enable. When set, drop PFC/CBFC PAUSE frames. */
	uint64_t tx_en                        : 1;  /**< Transmit enable. When set, allow for PFC/CBFC PAUSE packets. Must be clear in HiGig2 mode
                                                         i.e. when BGX(0..5)_SMU(0..3)_TX_CTL[HG_EN] = 1 and BGX(0..5)_SMU(0..3)_RX_UDD_SKP[LEN] =
                                                         16. */
	uint64_t rx_en                        : 1;  /**< Receive enable. When set, allow for PFC/CBFC PAUSE packets. Must be clear in HiGig2 mode
                                                         i.e. when BGX(0..5)_SMU(0..3)_TX_CTL[HG_EN] = 1 and BGX(0..5)_SMU(0..3)_RX_UDD_SKP[LEN] =
                                                         16. */
#else
	uint64_t rx_en                        : 1;
	uint64_t tx_en                        : 1;
	uint64_t drp_en                       : 1;
	uint64_t bck_en                       : 1;
	uint64_t reserved_4_31                : 28;
	uint64_t logl_en                      : 16;
	uint64_t phys_en                      : 16;
#endif
	} s;
	struct cvmx_bgxx_smux_cbfc_ctl_s      cn78xx;
};
typedef union cvmx_bgxx_smux_cbfc_ctl cvmx_bgxx_smux_cbfc_ctl_t;

/**
 * cvmx_bgx#_smu#_ctrl
 */
union cvmx_bgxx_smux_ctrl {
	uint64_t u64;
	struct cvmx_bgxx_smux_ctrl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t tx_idle                      : 1;  /**< TX machine is idle This indication pertains to the framer FSM and ignores the effects on
                                                         the data-path controls or values which occur when BGX_SMU_TX_CTL[LS_BYP] is set */
	uint64_t rx_idle                      : 1;  /**< RX machine is idle. */
#else
	uint64_t rx_idle                      : 1;
	uint64_t tx_idle                      : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_bgxx_smux_ctrl_s          cn78xx;
};
typedef union cvmx_bgxx_smux_ctrl cvmx_bgxx_smux_ctrl_t;

/**
 * cvmx_bgx#_smu#_ext_loopback
 *
 * In loopback mode, the IFG1+IFG2 of local and remote parties must match exactly; otherwise one
 * of the two sides' loopback FIFO will overrun: BGX(0..5)_SMU(0..3)_TX_INT[LB_OVRFLW].
 */
union cvmx_bgxx_smux_ext_loopback {
	uint64_t u64;
	struct cvmx_bgxx_smux_ext_loopback_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t en                           : 1;  /**< Loopback enable. Puts the packet interface in external loopback mode where the RX lines
                                                         are reflected on the TX lines. */
	uint64_t thresh                       : 4;  /**< Threshold on the TX FIFO. Software must only write the typical value. Any other value
                                                         causes loopback mode not to function correctly. */
#else
	uint64_t thresh                       : 4;
	uint64_t en                           : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_bgxx_smux_ext_loopback_s  cn78xx;
};
typedef union cvmx_bgxx_smux_ext_loopback cvmx_bgxx_smux_ext_loopback_t;

/**
 * cvmx_bgx#_smu#_hg2_control
 *
 * HiGig2 TX- and RX-enable are normally set together for HiGig2 messaging. Setting just the TX
 * or RX bit results in only the HG2 message transmit or receive capability.
 * Setting [PHYS_EN] and [LOGL_EN] to 1 allows link PAUSE or backpressure to PKO as per the
 * received HiGig2 message. Setting these fields to 0 disables link PAUSE and backpressure to PKO
 * in response to received messages.
 * BGX(0..5)_SMU(0..3)_TX_CTL[HG_EN] must be set (to enable HiGig) whenever either [HG2TX_EN] or
 * [HG2RX_EN] are set. BGX(0..5)_SMU(0..3)_RX_UDD_SKP[LEN] must be set to 16 (to select HiGig2)
 * whenever either [HG2TX_EN] or [HG2RX_EN] are set.
 * BGX(0..5)_CMR_RX_OVR_BP[EN<0>] must be set and BGX(0..5)_CMR_RX_OVR_BP[BP<0>] must be cleared
 * to 0 (to forcibly disable hardware-automatic 802.3 PAUSE packet generation) with the HiGig2
 * Protocol when BGX(0..5)_SMU(0..3)_HG2_CONTROL[HG2TX_EN] = 0. (The HiGig2 protocol is indicated
 * by BGX(0..5)_SMU(0..3)_TX_CTL[HG_EN] = 1 and BGX(0..5)_SMU(0..3)_RX_UDD_SKP[LEN]=16.) Hardware
 * can only autogenerate backpressure via HiGig2 messages (optionally, when HG2TX_EN = 1) with
 * the HiGig2 protocol.
 */
union cvmx_bgxx_smux_hg2_control {
	uint64_t u64;
	struct cvmx_bgxx_smux_hg2_control_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_19_63               : 45;
	uint64_t hg2tx_en                     : 1;  /**< Enable transmission of HG2 physical and logical messages. When set, also disables hardware
                                                         autogenerated (802.3 and PFC/CBFC) PAUSE frames. (CN78XX cannot generate proper 802.3 or
                                                         PFC/CBFC PAUSE frames in HiGig2 mode.) */
	uint64_t hg2rx_en                     : 1;  /**< Enable extraction and processing of HG2 message packet from RX flow. Physical and logical
                                                         PAUSE information is used to PAUSE physical-link, backpressure PKO. This field must be set
                                                         when HiGig2 messages are present in the receive stream. This bit is also forwarded to CMR
                                                         so it can generate the required deferring signals to SMU TX and backpressure signals to
                                                         PKO. */
	uint64_t phys_en                      : 1;  /**< 1 bit physical link pause enable for recevied
                                                         HiGig2 physical pause message. This bit enables the SMU TX
                                                         to CMR HG2 deferring counter to be set every time SMU RX
                                                         receives and filters out a valid physical HG2 message. */
	uint64_t logl_en                      : 16; /**< 16 bit xof enables for recevied HiGig2 messages
                                                         or PFC/CBFC packets. This field is NOT used by SMU at all.
                                                         It is forwarded to CMR without alteration. It appears here
                                                         for backward compatibility with O68. */
#else
	uint64_t logl_en                      : 16;
	uint64_t phys_en                      : 1;
	uint64_t hg2rx_en                     : 1;
	uint64_t hg2tx_en                     : 1;
	uint64_t reserved_19_63               : 45;
#endif
	} s;
	struct cvmx_bgxx_smux_hg2_control_s   cn78xx;
};
typedef union cvmx_bgxx_smux_hg2_control cvmx_bgxx_smux_hg2_control_t;

/**
 * cvmx_bgx#_smu#_rx_bad_col_hi
 */
union cvmx_bgxx_smux_rx_bad_col_hi {
	uint64_t u64;
	struct cvmx_bgxx_smux_rx_bad_col_hi_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_17_63               : 47;
	uint64_t val                          : 1;  /**< Set when BGX(0..5)_SMU(0..3)_RX_INT[PCTERR] is set. */
	uint64_t state                        : 8;  /**< When BGX(0..5)_SMU(0..3)_RX_INT[PCTERR] is set, contains the receive state at the time of
                                                         the error. */
	uint64_t lane_rxc                     : 8;  /**< When BGX(0..5)_SMU(0..3)_RX_INT[PCTERR] is set, contains the column at the time of the error. */
#else
	uint64_t lane_rxc                     : 8;
	uint64_t state                        : 8;
	uint64_t val                          : 1;
	uint64_t reserved_17_63               : 47;
#endif
	} s;
	struct cvmx_bgxx_smux_rx_bad_col_hi_s cn78xx;
};
typedef union cvmx_bgxx_smux_rx_bad_col_hi cvmx_bgxx_smux_rx_bad_col_hi_t;

/**
 * cvmx_bgx#_smu#_rx_bad_col_lo
 */
union cvmx_bgxx_smux_rx_bad_col_lo {
	uint64_t u64;
	struct cvmx_bgxx_smux_rx_bad_col_lo_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t lane_rxd                     : 64; /**< When BGX(0..5)_SMU(0..3)_RX_INT[PCTERR] is set, LANE_RXD contains the XAUI/RXAUI column at
                                                         the time of the error. */
#else
	uint64_t lane_rxd                     : 64;
#endif
	} s;
	struct cvmx_bgxx_smux_rx_bad_col_lo_s cn78xx;
};
typedef union cvmx_bgxx_smux_rx_bad_col_lo cvmx_bgxx_smux_rx_bad_col_lo_t;

/**
 * cvmx_bgx#_smu#_rx_ctl
 */
union cvmx_bgxx_smux_rx_ctl {
	uint64_t u64;
	struct cvmx_bgxx_smux_rx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t status                       : 2;  /**< Link status.
                                                         0x0 = Link OK
                                                         0x1 = Local fault
                                                         0x2 = Remote fault
                                                         0x3 = Reserved */
#else
	uint64_t status                       : 2;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_bgxx_smux_rx_ctl_s        cn78xx;
};
typedef union cvmx_bgxx_smux_rx_ctl cvmx_bgxx_smux_rx_ctl_t;

/**
 * cvmx_bgx#_smu#_rx_decision
 *
 * This register specifies the byte count used to determine when to accept or to filter a packet.
 * As each byte in a packet is received by BGX, the L2 byte count (i.e. the number of bytes from
 * the beginning of the L2 header (DMAC)) is compared against CNT. In normal operation, the L2
 * header begins after the PREAMBLE + SFD (BGX(0..5)_SMU(0..3)_RX_FRM_CTL[PRE_CHK] = 1) and any
 * optional UDD skip data (BGX(0..5)_SMU(0..3)_RX_UDD_SKP[LEN]).
 */
union cvmx_bgxx_smux_rx_decision {
	uint64_t u64;
	struct cvmx_bgxx_smux_rx_decision_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t cnt                          : 5;  /**< The byte count to decide when to accept or filter a packet. Refer to SMU Decisions. */
#else
	uint64_t cnt                          : 5;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_bgxx_smux_rx_decision_s   cn78xx;
};
typedef union cvmx_bgxx_smux_rx_decision cvmx_bgxx_smux_rx_decision_t;

/**
 * cvmx_bgx#_smu#_rx_frm_chk
 *
 * The CSRs provide the enable bits for a subset of errors passed to CMR encoded.
 *
 */
union cvmx_bgxx_smux_rx_frm_chk {
	uint64_t u64;
	struct cvmx_bgxx_smux_rx_frm_chk_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t skperr                       : 1;  /**< Skipper error. */
	uint64_t rcverr                       : 1;  /**< Frame was received with data-reception error. */
	uint64_t reserved_6_6                 : 1;
	uint64_t fcserr_c                     : 1;  /**< Control frame was received with FCS/CRC error. */
	uint64_t fcserr_d                     : 1;  /**< Data frame was received with FCS/CRC error. */
	uint64_t jabber                       : 1;  /**< Frame was received with length > sys_length. */
	uint64_t reserved_0_2                 : 3;
#else
	uint64_t reserved_0_2                 : 3;
	uint64_t jabber                       : 1;
	uint64_t fcserr_d                     : 1;
	uint64_t fcserr_c                     : 1;
	uint64_t reserved_6_6                 : 1;
	uint64_t rcverr                       : 1;
	uint64_t skperr                       : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_bgxx_smux_rx_frm_chk_s    cn78xx;
};
typedef union cvmx_bgxx_smux_rx_frm_chk cvmx_bgxx_smux_rx_frm_chk_t;

/**
 * cvmx_bgx#_smu#_rx_frm_ctl
 *
 * This register controls the handling of the frames.
 * The CTL_BCK/CTL_DRP bits control how the hardware handles incoming PAUSE packets. The most
 * common modes of operation:
 * CTL_BCK = 1, CTL_DRP = 1: hardware handles everything
 * CTL_BCK = 0, CTL_DRP = 0: software sees all PAUSE frames
 * CTL_BCK = 0, CTL_DRP = 1: all PAUSE frames are completely ignored
 * These control bits should be set to CTL_BCK = 0,CTL_DRP = 0 in half-duplex mode. Since PAUSE
 * packets only apply to full duplex operation, any PAUSE packet would constitute an exception
 * which should be handled by the processing cores. PAUSE packets should not be forwarded.
 */
union cvmx_bgxx_smux_rx_frm_ctl {
	uint64_t u64;
	struct cvmx_bgxx_smux_rx_frm_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_13_63               : 51;
	uint64_t ptp_mode                     : 1;  /**< Timestamp mode. When PTP_MODE is set, a 64-bit timestamp is prepended to every incoming
                                                         packet.
                                                         The timestamp bytes are added to the packet in such a way as to not modify the packet's
                                                         receive byte count. This implies that the BGX(0..5)_SMU(0..3)_RX_JABBER,
                                                         BGX(0..5)_SMU(0..3)_RX_DECISION, and BGX(0..5)_SMU(0..3)_RX_UDD_SKP do not require any
                                                         adjustment as they operate on the received packet size. When the packet reaches PKI, its
                                                         size reflects the additional bytes and is subject to the following restrictions:
                                                         If PTP_MODE = 1 and PRE_CHK = 1, PRE_STRP must be 1.
                                                         If PTP_MODE = 1
                                                         PKI_CL(0..3)_PKIND(0..63)_SKIP[FCS_SKIP,INST_SKIP] should be increased by 8
                                                         PKI_CL(0..3)_PKIND(0..63)_CFG[HG_EN] should be 0
                                                         PKI_FRM_LEN_CHK(0..1)[MAXLEN] should be increased by 8
                                                         PKI_FRM_LEN_CHK(0..1)[MINLEN] should be increased by 8
                                                         PKI_TAG_INC(0..63)_MASK should be adjusted
                                                         This supported in uCode in O78 >>> PIP_PRT_CFGB(0..63)[ALT_SKP_EN] should be 0. */
	uint64_t reserved_6_11                : 6;
	uint64_t ctl_smac                     : 1;  /**< Control PAUSE frames can match station SMAC. */
	uint64_t ctl_mcst                     : 1;  /**< Control PAUSE frames can match globally assign multicast address. */
	uint64_t ctl_bck                      : 1;  /**< Forward PAUSE information to TX block. */
	uint64_t ctl_drp                      : 1;  /**< Drop control PAUSE frames. */
	uint64_t pre_strp                     : 1;  /**< Strip off the preamble (when present).
                                                         0 = PREAMBLE + SFD is sent to core as part of frame
                                                         1 = PREAMBLE + SFD is dropped
                                                         [PRE_CHK] must be set to enable this and all PREAMBLE features.
                                                         If PTP_MODE = 1 and PRE_CHK = 1, PRE_STRP must be 1.
                                                         When PRE_CHK is set (indicating that the PREAMBLE will be sent), PRE_STRP determines if
                                                         the PREAMBLE+SFD bytes are thrown away or sent to the core as part of the packet. In
                                                         either mode, the PREAMBLE+SFD bytes are not counted toward the packet size when checking
                                                         against the MIN and MAX bounds. Furthermore, the bytes are skipped when locating the start
                                                         of the L2 header for DMAC and control frame recognition. */
	uint64_t pre_chk                      : 1;  /**< Check the preamble for correctness.
                                                         This port is configured to send a valid 802.3 PREAMBLE to begin every frame. BGX checks
                                                         that a valid PREAMBLE is received (based on PRE_FREE). When a problem does occur within
                                                         the PREAMBLE sequence, the frame is marked as bad and not sent into the core. The
                                                         BGX(0..5)_SMU(0..3)_RX_INT[PCTERR] interrupt is also raised.
                                                         When BGX(0..5)_SMU(0..3)_TX_CTL[HG_EN] is set, PRE_CHK must be 0.
                                                         If PTP_MODE = 1 and PRE_CHK = 1, PRE_STRP must be 1. */
#else
	uint64_t pre_chk                      : 1;
	uint64_t pre_strp                     : 1;
	uint64_t ctl_drp                      : 1;
	uint64_t ctl_bck                      : 1;
	uint64_t ctl_mcst                     : 1;
	uint64_t ctl_smac                     : 1;
	uint64_t reserved_6_11                : 6;
	uint64_t ptp_mode                     : 1;
	uint64_t reserved_13_63               : 51;
#endif
	} s;
	struct cvmx_bgxx_smux_rx_frm_ctl_s    cn78xx;
};
typedef union cvmx_bgxx_smux_rx_frm_ctl cvmx_bgxx_smux_rx_frm_ctl_t;

/**
 * cvmx_bgx#_smu#_rx_int
 *
 * SMU Interrupt Register.
 *
 */
union cvmx_bgxx_smux_rx_int {
	uint64_t u64;
	struct cvmx_bgxx_smux_rx_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t hg2cc                        : 1;  /**< HiGig2 received message CRC or control-character error. Set when either a CRC8 error is
                                                         detected, or when a control character is found in the message bytes after the K.SOM.
                                                         HG2CC has higher priority than HG2FLD, which means that a HiGig2 message that results in
                                                         HG2CC getting set never sets HG2FLD. */
	uint64_t hg2fld                       : 1;  /**< HiGig2 received message field error:
                                                         MSG_TYPE field not 0x0, i.e. it is not a flow-control message, which is the only defined
                                                         type for HiGig2
                                                         FWD_TYPE field not 0x0, i.e. it is not a link-level message, which is the only defined
                                                         type for HiGig2
                                                         FC_OBJECT field is neither 0x0 for physical link, nor 0x2 for logical link. Those are the
                                                         only two defined types in HiGig2 */
	uint64_t bad_term                     : 1;  /**< Frame is terminated by control character other than /T/. (XAUI/RXAUI mode only) The error
                                                         propagation control character /E/ will be included as part of the frame and does not cause
                                                         a frame termination. */
	uint64_t bad_seq                      : 1;  /**< Reserved sequence detected. (XAUI/RXAUI mode only) */
	uint64_t rem_fault                    : 1;  /**< Remote-fault sequence detected. (XAUI/RXAUI mode only) */
	uint64_t loc_fault                    : 1;  /**< Local-fault sequence detected. (XAUI/RXAUI mode only) */
	uint64_t rsverr                       : 1;  /**< Reserved opcodes. */
	uint64_t pcterr                       : 1;  /**< Bad preamble/protocol. In XAUI/RXAUI mode, the column of data that was bad is logged in
                                                         BGX(0..5)_SMU(0..3)_RX_BAD_COL_*. PCTERR checks that the frame begins with a valid
                                                         PREAMBLE sequence. Does not check the number of PREAMBLE cycles. */
	uint64_t skperr                       : 1;  /**< Skipper error. */
	uint64_t rcverr                       : 1;  /**< Frame was received with data-reception error. */
	uint64_t fcserr                       : 1;  /**< Frame was received with FCS/CRC error */
	uint64_t jabber                       : 1;  /**< Frame was received with length > sys_length. An RX Jabber error indicates that a packet
                                                         was received which is longer than the maximum allowed packet as defined by the system. BGX
                                                         terminates the packet with an EOP on the beat on which JABBER was exceeded. The beat on
                                                         which JABBER was exceeded is left unchanged and all subsequent data beats are dropped.
                                                         Failure to truncate could lead to system instability. */
#else
	uint64_t jabber                       : 1;
	uint64_t fcserr                       : 1;
	uint64_t rcverr                       : 1;
	uint64_t skperr                       : 1;
	uint64_t pcterr                       : 1;
	uint64_t rsverr                       : 1;
	uint64_t loc_fault                    : 1;
	uint64_t rem_fault                    : 1;
	uint64_t bad_seq                      : 1;
	uint64_t bad_term                     : 1;
	uint64_t hg2fld                       : 1;
	uint64_t hg2cc                        : 1;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_bgxx_smux_rx_int_s        cn78xx;
};
typedef union cvmx_bgxx_smux_rx_int cvmx_bgxx_smux_rx_int_t;

/**
 * cvmx_bgx#_smu#_rx_jabber
 */
union cvmx_bgxx_smux_rx_jabber {
	uint64_t u64;
	struct cvmx_bgxx_smux_rx_jabber_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t cnt                          : 16; /**< Byte count for jabber check. Failing packets set the JABBER interrupt and are optionally
                                                         sent with opcode = JABBER. BGX truncates the packet to CNT+1 to CNT+8 bytes.
                                                         CNT must be 8-byte aligned such that CNT[2:0] = 000. */
#else
	uint64_t cnt                          : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_smux_rx_jabber_s     cn78xx;
};
typedef union cvmx_bgxx_smux_rx_jabber cvmx_bgxx_smux_rx_jabber_t;

/**
 * cvmx_bgx#_smu#_rx_udd_skp
 */
union cvmx_bgxx_smux_rx_udd_skp {
	uint64_t u64;
	struct cvmx_bgxx_smux_rx_udd_skp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t fcssel                       : 1;  /**< Include the skip bytes in the FCS calculation.
                                                         0 = All skip bytes are included in FCS
                                                         1 = The skip bytes are not included in FCS
                                                         When BGX(0..5)_SMU(0..3)_TX_CTL[HG_EN] is set, this field must be 0.
                                                         The skip bytes are part of the packet and are sent through the IOI packet interface and
                                                         are handled by PKI. The system can determine if the UDD bytes are included in the FCS
                                                         check by using the FCSSEL field, if the FCS check is enabled. */
	uint64_t reserved_7_7                 : 1;
	uint64_t len                          : 7;  /**< Amount of user-defined data before the start of the L2C data, in bytes.
                                                         Setting to 0 means L2C comes first; maximum value is 64.
                                                         LEN must be 0x0 in half-duplex operation.
                                                         When BGX(0..5)_SMU(0..3)_TX_CTL[HG_EN] is set, this field must be set to 12 or 16
                                                         (depending on HiGig header size) to account for the HiGig header.
                                                         LEN = 12 selects HiGig/HiGig+; LEN = 16 selects HiGig2. */
#else
	uint64_t len                          : 7;
	uint64_t reserved_7_7                 : 1;
	uint64_t fcssel                       : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_bgxx_smux_rx_udd_skp_s    cn78xx;
};
typedef union cvmx_bgxx_smux_rx_udd_skp cvmx_bgxx_smux_rx_udd_skp_t;

/**
 * cvmx_bgx#_smu#_smac
 */
union cvmx_bgxx_smux_smac {
	uint64_t u64;
	struct cvmx_bgxx_smux_smac_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t smac                         : 48; /**< The SMAC field is used for generating and accepting control PAUSE packets. */
#else
	uint64_t smac                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_smux_smac_s          cn78xx;
};
typedef union cvmx_bgxx_smux_smac cvmx_bgxx_smux_smac_t;

/**
 * cvmx_bgx#_smu#_tx_append
 *
 * For more details on the interactions between FCS and PAD, see also the description of
 * BGX(0..5)_SMU(0..3)_TX_MIN_PKT[MIN_SIZE].
 */
union cvmx_bgxx_smux_tx_append {
	uint64_t u64;
	struct cvmx_bgxx_smux_tx_append_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t fcs_c                        : 1;  /**< Append the Ethernet FCS on each PAUSE packet. PAUSE packets are normally padded to 60
                                                         bytes. If BGX(0..5)_SMU(0..3)_TX_MIN_PKT[MIN_SIZE] exceeds 59, then FCS_C is not used. */
	uint64_t fcs_d                        : 1;  /**< Append the Ethernet FCS on each data packet. */
	uint64_t pad                          : 1;  /**< Append PAD bytes such that minimum-sized packet is transmitted. */
	uint64_t preamble                     : 1;  /**< Prepend the Ethernet preamble on each transfer. When BGX(0..5)_SMU(0..3)_TX_CTL[HG_EN] is
                                                         set, PREAMBLE must be 0. */
#else
	uint64_t preamble                     : 1;
	uint64_t pad                          : 1;
	uint64_t fcs_d                        : 1;
	uint64_t fcs_c                        : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_bgxx_smux_tx_append_s     cn78xx;
};
typedef union cvmx_bgxx_smux_tx_append cvmx_bgxx_smux_tx_append_t;

/**
 * cvmx_bgx#_smu#_tx_ctl
 */
union cvmx_bgxx_smux_tx_ctl {
	uint64_t u64;
	struct cvmx_bgxx_smux_tx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_31_63               : 33;
	uint64_t spu_mrk_cnt                  : 20; /**< 40GBASE-R Transmit Marker Interval Count: Specifies the interval
                                                         (number of 66-bit BASE-R blocks) at which the LMAC transmit logic
                                                         inserts 40GBASE-R alignment markers. An internal counter in SMU is
                                                         initialized to this value, counts down for each BASE-R block
                                                         transmitted by the LMAC, and wraps back to the initial value from 0.
                                                         The LMAC transmit logic inserts alignment markers for lanes 0, 1, 2
                                                         and 3, respectively, in the last four BASE-R blocks before the
                                                         counter wraps (3, 2, 1, 0). The default value corresponds to an
                                                         alignment marker period of 16363 blocks (exclusive) per lane, as
                                                         specified in 802.3ba-2010. The default value should always be used
                                                         for normal operation. */
	uint64_t hg_pause_hgi                 : 2;  /**< HGI field for hardware-generated HiGig PAUSE packets. */
	uint64_t hg_en                        : 1;  /**< Enable HiGig mode.
                                                         When this field is set and BGX(0..5)_SMU(0..3)_RX_UDD_SKP[LEN] = 12, the interface is in
                                                         HiGig/HiGig+ mode and the following must be set:
                                                         BGX(0..5)_SMU(0..3)_RX_FRM_CTL[PRE_CHK] = 0
                                                         BGX(0..5)_SMU(0..3)_RX_UDD_SKP[FCSSEL] = 0
                                                         BGX(0..5)_SMU(0..3)_RX_UDD_SKP[LEN] = 12
                                                         BGX(0..5)_SMU(0..3)_TX_APPEND[PREAMBLE] = 0
                                                         When this field is set and BGX(0..5)_SMU(0..3)_RX_UDD_SKP[LEN] = 16, the interface is in
                                                         HiGig2 mode and the following must be set:
                                                         BGX(0..5)_SMU(0..3)_RX_FRM_CTL[PRE_CHK] = 0
                                                         BGX(0..5)_SMU(0..3)_RX_UDD_SKP[FCSSEL] = 0
                                                         BGX(0..5)_SMU(0..3)_RX_UDD_SKP[LEN] = 16
                                                         BGX(0..5)_SMU(0..3)_TX_APPEND[PREAMBLE] = 0
                                                         BGX(0..5)_SMU(0..3)_SMUX_CBFC_CTL[RX_EN] = 0
                                                         BGX(0..5)_SMU(0..3)_CBFC_CTL[TX_EN] = 0 */
	uint64_t l2p_bp_conv                  : 1;  /**< If set will cause TX to generate 802.3 pause packets when CMR applies logical backpressure
                                                         (XOFF), if and only if BGX(0..5)_SMU(0..3)_CBFC_CTL[TX_EN] is clear and
                                                         BGX(0..5)_SMU(0..3)_HG2_CONTROL[HG2TX_EN] is clear. */
	uint64_t ls_byp                       : 1;  /**< Bypass the link status, as determined by the XGMII receiver, and set the link status of
                                                         the transmitter to LS. */
	uint64_t ls                           : 2;  /**< Link status.
                                                         0 = Link OK; link runs normally. RS passes MAC data to PCS.
                                                         1 = Local fault. RS layer sends continuous remote fault sequences.
                                                         2 = Remote fault. RS layer sends continuous idle sequences.
                                                         3 = Link drain. RS layer drops full packets to allow BGX and PKO to drain their FIFOs. */
	uint64_t reserved_2_3                 : 2;
	uint64_t uni_en                       : 1;  /**< Enable unidirectional mode (IEEE Clause 66). */
	uint64_t dic_en                       : 1;  /**< Enable the deficit idle counter for IFG averaging. */
#else
	uint64_t dic_en                       : 1;
	uint64_t uni_en                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t ls                           : 2;
	uint64_t ls_byp                       : 1;
	uint64_t l2p_bp_conv                  : 1;
	uint64_t hg_en                        : 1;
	uint64_t hg_pause_hgi                 : 2;
	uint64_t spu_mrk_cnt                  : 20;
	uint64_t reserved_31_63               : 33;
#endif
	} s;
	struct cvmx_bgxx_smux_tx_ctl_s        cn78xx;
};
typedef union cvmx_bgxx_smux_tx_ctl cvmx_bgxx_smux_tx_ctl_t;

/**
 * cvmx_bgx#_smu#_tx_ifg
 *
 * Programming IFG1 and IFG2:
 * For XAUI/RXAUI/10Gbs/40Gbs systems that require IEEE 802.3 compatibility, the IFG1+IFG2 sum
 * must be 12.
 * In loopback mode, the IFG1+IFG2 of local and remote parties must match exactly; otherwise one
 * of the two sides' loopback FIFO will overrun: BGX(0..5)_SMU(0..3)_TX_INT[LB_OVRFLW].
 */
union cvmx_bgxx_smux_tx_ifg {
	uint64_t u64;
	struct cvmx_bgxx_smux_tx_ifg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t ifg2                         : 4;  /**< 1/2 of the interframe gap timing (in IFG2*8 bits) */
	uint64_t ifg1                         : 4;  /**< 1/2 of the interframe gap timing (in IFG1*8 bits) */
#else
	uint64_t ifg1                         : 4;
	uint64_t ifg2                         : 4;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_bgxx_smux_tx_ifg_s        cn78xx;
};
typedef union cvmx_bgxx_smux_tx_ifg cvmx_bgxx_smux_tx_ifg_t;

/**
 * cvmx_bgx#_smu#_tx_int
 */
union cvmx_bgxx_smux_tx_int {
	uint64_t u64;
	struct cvmx_bgxx_smux_tx_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t lb_ovrflw                    : 1;  /**< TX loopback overflow. */
	uint64_t lb_undflw                    : 1;  /**< TX loopback underflow. */
	uint64_t fake_commit                  : 1;  /**< TX SMU started a packet with PTP on SOP and has not seen a commit for it from TX SPU after
                                                         2^SMU_TX_PTP_TIMEOUT_WIDTH (2^8) cycles so it faked a commit to CMR. */
	uint64_t xchange                      : 1;  /**< Link status changed. This denotes a change to BGX(0..5)_SMU(0..3)_RX_CTL[STATUS]. */
	uint64_t undflw                       : 1;  /**< TX underflow. */
#else
	uint64_t undflw                       : 1;
	uint64_t xchange                      : 1;
	uint64_t fake_commit                  : 1;
	uint64_t lb_undflw                    : 1;
	uint64_t lb_ovrflw                    : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_bgxx_smux_tx_int_s        cn78xx;
};
typedef union cvmx_bgxx_smux_tx_int cvmx_bgxx_smux_tx_int_t;

/**
 * cvmx_bgx#_smu#_tx_min_pkt
 */
union cvmx_bgxx_smux_tx_min_pkt {
	uint64_t u64;
	struct cvmx_bgxx_smux_tx_min_pkt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t min_size                     : 8;  /**< Min frame in bytes inclusive of FCS, if applied. Padding is only appended when
                                                         BGX_TX_APPEND[PAD] for the corresponding port is set. When FCS is added to a packet which
                                                         was padded, the FCS always appears in the 4 octets preceding /T/ or /E/. */
#else
	uint64_t min_size                     : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_bgxx_smux_tx_min_pkt_s    cn78xx;
};
typedef union cvmx_bgxx_smux_tx_min_pkt cvmx_bgxx_smux_tx_min_pkt_t;

/**
 * cvmx_bgx#_smu#_tx_pause_pkt_dmac
 *
 * This register provides the DMAC value that is placed in outbound PAUSE packets.
 *
 */
union cvmx_bgxx_smux_tx_pause_pkt_dmac {
	uint64_t u64;
	struct cvmx_bgxx_smux_tx_pause_pkt_dmac_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t dmac                         : 48; /**< The DMAC field, which is placed is outbound PAUSE packets. */
#else
	uint64_t dmac                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_smux_tx_pause_pkt_dmac_s cn78xx;
};
typedef union cvmx_bgxx_smux_tx_pause_pkt_dmac cvmx_bgxx_smux_tx_pause_pkt_dmac_t;

/**
 * cvmx_bgx#_smu#_tx_pause_pkt_interval
 *
 * This register specifies how often PAUSE packets are sent.
 *
 */
union cvmx_bgxx_smux_tx_pause_pkt_interval {
	uint64_t u64;
	struct cvmx_bgxx_smux_tx_pause_pkt_interval_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_33_63               : 31;
	uint64_t hg2_intra_en                 : 1;  /**< Allow intrapacket HiGig2 message generation. Relevant only if HiGig2 message generation is enabled. */
	uint64_t hg2_intra_interval           : 16; /**< Arbitrate for a HiGig2 message, every (INTERVAL*512) bit-times whilst sending regular
                                                         packet data. Relevant only if HiGig2 message generation and HG2_INTRA_EN are both set.
                                                         Normally, 0 < INTERVAL < BGX_TX_PAUSE_PKT_TIME.
                                                         INTERVAL = 0 only sends a single PAUSE packet for each backpressure event. */
	uint64_t interval                     : 16; /**< Arbitrate for a 802.3 PAUSE packet, HiGig2 message, or PFC/CBFC PAUSE packet every
                                                         (INTERVAL * 512) bit-times.
                                                         Normally, 0 < INTERVAL < BGX(0..5)_SMU(0..3)_TX_PAUSE_PKT_TIME[TIME].
                                                         INTERVAL = 0 only sends a single PAUSE packet for each backpressure event. */
#else
	uint64_t interval                     : 16;
	uint64_t hg2_intra_interval           : 16;
	uint64_t hg2_intra_en                 : 1;
	uint64_t reserved_33_63               : 31;
#endif
	} s;
	struct cvmx_bgxx_smux_tx_pause_pkt_interval_s cn78xx;
};
typedef union cvmx_bgxx_smux_tx_pause_pkt_interval cvmx_bgxx_smux_tx_pause_pkt_interval_t;

/**
 * cvmx_bgx#_smu#_tx_pause_pkt_time
 */
union cvmx_bgxx_smux_tx_pause_pkt_time {
	uint64_t u64;
	struct cvmx_bgxx_smux_tx_pause_pkt_time_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t p_time                       : 16; /**< Provides the pause_time field placed in outbound 802.3 PAUSE packets, HiGig2 messages, or
                                                         PFC/CBFC PAUSE packets in 512 bit-times. Normally, P_TIME >
                                                         BGX(0..5)_SMU(0..3)_TX_PAUSE_PKT_INTERVAL[INTERVAL]. See programming notes in
                                                         BGX(0..5)_SMU(0..3)_TX_PAUSE_PKT_INTERVAL. */
#else
	uint64_t p_time                       : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_smux_tx_pause_pkt_time_s cn78xx;
};
typedef union cvmx_bgxx_smux_tx_pause_pkt_time cvmx_bgxx_smux_tx_pause_pkt_time_t;

/**
 * cvmx_bgx#_smu#_tx_pause_pkt_type
 *
 * This register provides the P_TYPE field that is placed in outbound PAUSE packets.
 *
 */
union cvmx_bgxx_smux_tx_pause_pkt_type {
	uint64_t u64;
	struct cvmx_bgxx_smux_tx_pause_pkt_type_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t p_type                       : 16; /**< The P_TYPE field that is placed in outbound PAUSE packets. */
#else
	uint64_t p_type                       : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_smux_tx_pause_pkt_type_s cn78xx;
};
typedef union cvmx_bgxx_smux_tx_pause_pkt_type cvmx_bgxx_smux_tx_pause_pkt_type_t;

/**
 * cvmx_bgx#_smu#_tx_pause_togo
 */
union cvmx_bgxx_smux_tx_pause_togo {
	uint64_t u64;
	struct cvmx_bgxx_smux_tx_pause_togo_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t msg_time                     : 16; /**< Amount of time remaining to backpressure, from the HiGig2 physical message PAUSE timer
                                                         (only valid on port0). */
	uint64_t p_time                       : 16; /**< Amount of time remaining to backpressure, from the standard 802.3 PAUSE timer. */
#else
	uint64_t p_time                       : 16;
	uint64_t msg_time                     : 16;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_bgxx_smux_tx_pause_togo_s cn78xx;
};
typedef union cvmx_bgxx_smux_tx_pause_togo cvmx_bgxx_smux_tx_pause_togo_t;

/**
 * cvmx_bgx#_smu#_tx_pause_zero
 */
union cvmx_bgxx_smux_tx_pause_zero {
	uint64_t u64;
	struct cvmx_bgxx_smux_tx_pause_zero_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t send                         : 1;  /**< Send PAUSE-zero enable. When this bit is set, and the backpressure condition is clear, it
                                                         allows sending a PAUSE packet with pause_time of 0 to enable the channel. */
#else
	uint64_t send                         : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_bgxx_smux_tx_pause_zero_s cn78xx;
};
typedef union cvmx_bgxx_smux_tx_pause_zero cvmx_bgxx_smux_tx_pause_zero_t;

/**
 * cvmx_bgx#_smu#_tx_soft_pause
 */
union cvmx_bgxx_smux_tx_soft_pause {
	uint64_t u64;
	struct cvmx_bgxx_smux_tx_soft_pause_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t p_time                       : 16; /**< Back off the TX bus for (P_TIME * 512) bit-times */
#else
	uint64_t p_time                       : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_smux_tx_soft_pause_s cn78xx;
};
typedef union cvmx_bgxx_smux_tx_soft_pause cvmx_bgxx_smux_tx_soft_pause_t;

/**
 * cvmx_bgx#_smu#_tx_thresh
 */
union cvmx_bgxx_smux_tx_thresh {
	uint64_t u64;
	struct cvmx_bgxx_smux_tx_thresh_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t cnt                          : 11; /**< Number of 128-bit words to accumulate in the TX FIFO before sending on the packet
                                                         interface. This field should be large enough to prevent underflow on the packet interface
                                                         and must never be set to 0x0.
                                                         In 10G/40G mode, CNT = 0x100.
                                                         In all modes, this register cannot exceed the TX FIFO depth as follows.
                                                         BGX(0..5)_CMR_TX_PRTS = 0,1:  CNT maximum = 0x7FF
                                                         BGX(0..5)_CMR_TX_PRTS = 2:     CNT maximum = 0x3FF
                                                         BGX(0..5)_CMR_TX_PRTS = 3,4:  CNT maximum = 0x1FF */
#else
	uint64_t cnt                          : 11;
	uint64_t reserved_11_63               : 53;
#endif
	} s;
	struct cvmx_bgxx_smux_tx_thresh_s     cn78xx;
};
typedef union cvmx_bgxx_smux_tx_thresh cvmx_bgxx_smux_tx_thresh_t;

/**
 * cvmx_bgx#_spu#_an_adv
 *
 * Software programs this register with the contents of the AN-link code word base page to be
 * transmitted during Auto-Negotiation. (See Std 802.3 section 73.6 for details.) Any write
 * operations to this register prior to completion of Auto-Negotiation, as indicated by
 * BGX(0..5)_SPU(0..3)_AN_STATUS[AN_COMPLETE], should be followed by a renegotiation in order for
 * the new values to take effect. Renegotiation is initiated by setting
 * BGX(0..5)_SPU(0..3)_AN_CONTROL[AN_RESTART]. Once Auto-Negotiation has completed, software can
 * examine this register along with BGX(0..5)_SPU(0..3)_AN_LP_BASE to determine the highest
 * common denominator technology.
 */
union cvmx_bgxx_spux_an_adv {
	uint64_t u64;
	struct cvmx_bgxx_spux_an_adv_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t fec_req                      : 1;  /**< FEC requested. */
	uint64_t fec_able                     : 1;  /**< FEC ability. */
	uint64_t arsv                         : 19; /**< Technology ability. Reserved bits, should always be 0. */
	uint64_t a100g_cr10                   : 1;  /**< 100GBASE-CR10 ability. Should always be 0; 100GBASE-R is not supported. */
	uint64_t a40g_cr4                     : 1;  /**< 40GBASE-CR4 ability. */
	uint64_t a40g_kr4                     : 1;  /**< 40GBASE-KR4 ability. */
	uint64_t a10g_kr                      : 1;  /**< 10GBASE-KR ability. */
	uint64_t a10g_kx4                     : 1;  /**< 10GBASE-KX4 ability. */
	uint64_t a1g_kx                       : 1;  /**< 1000BASE-KX ability. Should always be 0; Auto-Negotiation is not supported for 1000Base-KX. */
	uint64_t t                            : 5;  /**< Transmitted nonce. This field is automatically updated with a pseudo-random value on entry
                                                         to the AN ability detect state. */
	uint64_t np                           : 1;  /**< Next page. Always 0; extended next pages are not used for 10G+ Auto-Negotiation. */
	uint64_t ack                          : 1;  /**< Acknowledge. Always 0 in this register. */
	uint64_t rf                           : 1;  /**< Remote fault. */
	uint64_t xnp_able                     : 1;  /**< Extended next page ability. */
	uint64_t asm_dir                      : 1;  /**< Asymmetric PAUSE. */
	uint64_t pause                        : 1;  /**< PAUSE ability. */
	uint64_t e                            : 5;  /**< Echoed nonce. Provides the echoed-nonce value to use when ACK = 0 in transmitted DME page.
                                                         Should always be 0x0. */
	uint64_t s                            : 5;  /**< Selector. Should be 0x1 (encoding for IEEE Std 802.3). */
#else
	uint64_t s                            : 5;
	uint64_t e                            : 5;
	uint64_t pause                        : 1;
	uint64_t asm_dir                      : 1;
	uint64_t xnp_able                     : 1;
	uint64_t rf                           : 1;
	uint64_t ack                          : 1;
	uint64_t np                           : 1;
	uint64_t t                            : 5;
	uint64_t a1g_kx                       : 1;
	uint64_t a10g_kx4                     : 1;
	uint64_t a10g_kr                      : 1;
	uint64_t a40g_kr4                     : 1;
	uint64_t a40g_cr4                     : 1;
	uint64_t a100g_cr10                   : 1;
	uint64_t arsv                         : 19;
	uint64_t fec_able                     : 1;
	uint64_t fec_req                      : 1;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_spux_an_adv_s        cn78xx;
};
typedef union cvmx_bgxx_spux_an_adv cvmx_bgxx_spux_an_adv_t;

/**
 * cvmx_bgx#_spu#_an_bp_status
 *
 * The contents of this register (with the exception of the static BP_AN_ABLE bit) are updated
 * during Auto-Negotiation and are valid when BGX(0..5)_SPU(0..3)_AN_STATUS[AN_COMPLETE] is set.
 * At that time, one of the port type bits (A100G_CR10, A40G_CR4, A40G_KR4, A10G_KR, A10G_KX4,
 * A1G_KX) will be set depending on the AN priority resolution. If a BASE-R type is negotiated,
 * then the FEC bit will be set to indicate that FEC operation has been negotiated, and will be
 * clear otherwise.
 */
union cvmx_bgxx_spux_an_bp_status {
	uint64_t u64;
	struct cvmx_bgxx_spux_an_bp_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t n100g_cr10                   : 1;  /**< 100GBASE-CR10 negotiated; expected to always be 0; 100GBASE-R is not supported. */
	uint64_t reserved_7_7                 : 1;
	uint64_t n40g_cr4                     : 1;  /**< 40GBASE-CR4 negotiated */
	uint64_t n40g_kr4                     : 1;  /**< 40GBASE-KR4 negotiated */
	uint64_t fec                          : 1;  /**< BASE-R FEC negotiated */
	uint64_t n10g_kr                      : 1;  /**< 10GBASE-KR negotiated */
	uint64_t n10g_kx4                     : 1;  /**< 10GBASE-KX4 or CX4 negotiated (XAUI) */
	uint64_t n1g_kx                       : 1;  /**< 1000BASE-KX negotiated */
	uint64_t bp_an_able                   : 1;  /**< Backplane or BASE-R copper AN Ability; always 1. */
#else
	uint64_t bp_an_able                   : 1;
	uint64_t n1g_kx                       : 1;
	uint64_t n10g_kx4                     : 1;
	uint64_t n10g_kr                      : 1;
	uint64_t fec                          : 1;
	uint64_t n40g_kr4                     : 1;
	uint64_t n40g_cr4                     : 1;
	uint64_t reserved_7_7                 : 1;
	uint64_t n100g_cr10                   : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_bgxx_spux_an_bp_status_s  cn78xx;
};
typedef union cvmx_bgxx_spux_an_bp_status cvmx_bgxx_spux_an_bp_status_t;

/**
 * cvmx_bgx#_spu#_an_control
 */
union cvmx_bgxx_spux_an_control {
	uint64_t u64;
	struct cvmx_bgxx_spux_an_control_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t an_reset                     : 1;  /**< Auto-Negotiation reset. Setting this bit or BGXn_SPUm_CONTROL1[RESET] to 1 causes the
                                                         following to happen:
                                                         Resets the logical PCS (LPCS)
                                                         Sets the Std 802.3 PCS, FEC and AN registers for the LPCS to their default states
                                                         Resets the associated SerDes lanes.
                                                         It takes up to 32 coprocessor-clock cycles to reset the LPCS, after which RESET is
                                                         automatically cleared. */
	uint64_t reserved_14_14               : 1;
	uint64_t xnp_en                       : 1;  /**< Extended next-page enable. */
	uint64_t an_en                        : 1;  /**< Auto-Negotiation enable. This bit should not be set when BGX_CMR_CONFIG[LMAC_TYPE] is set
                                                         to RXAUI; auto-negotiation is not supported in RXAUI mode. */
	uint64_t reserved_10_11               : 2;
	uint64_t an_restart                   : 1;  /**< Auto-Negotiation restart. Writing a 1 to this bit restarts the Auto-Negotiation process if
                                                         AN_EN is also set. This is a self-clearing bit. */
	uint64_t reserved_0_8                 : 9;
#else
	uint64_t reserved_0_8                 : 9;
	uint64_t an_restart                   : 1;
	uint64_t reserved_10_11               : 2;
	uint64_t an_en                        : 1;
	uint64_t xnp_en                       : 1;
	uint64_t reserved_14_14               : 1;
	uint64_t an_reset                     : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_spux_an_control_s    cn78xx;
};
typedef union cvmx_bgxx_spux_an_control cvmx_bgxx_spux_an_control_t;

/**
 * cvmx_bgx#_spu#_an_lp_base
 *
 * This register captures the contents of the latest AN link code word base page received from
 * the link partner during Auto-Negotiation. (See Std 802.3 section 73.6 for details.)
 * BGX(0..5)_SPU(0..3)_AN_STATUS[PAGE_RX] is set when this register is updated by hardware.
 */
union cvmx_bgxx_spux_an_lp_base {
	uint64_t u64;
	struct cvmx_bgxx_spux_an_lp_base_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t fec_req                      : 1;  /**< FEC requested. */
	uint64_t fec_able                     : 1;  /**< FEC ability. */
	uint64_t arsv                         : 19; /**< Technology ability. Reserved bits, should always be 0. */
	uint64_t a100g_cr10                   : 1;  /**< 100GBASE-CR10 ability. */
	uint64_t a40g_cr4                     : 1;  /**< 40GBASE-CR4 ability. */
	uint64_t a40g_kr4                     : 1;  /**< 40GBASE-KR4 ability. */
	uint64_t a10g_kr                      : 1;  /**< 10GBASE-KR ability. */
	uint64_t a10g_kx4                     : 1;  /**< 10GBASE-KX4 ability. */
	uint64_t a1g_kx                       : 1;  /**< 1000BASE-KX ability. */
	uint64_t t                            : 5;  /**< Transmitted nonce. */
	uint64_t np                           : 1;  /**< Next page. */
	uint64_t ack                          : 1;  /**< Acknowledge. */
	uint64_t rf                           : 1;  /**< Remote fault. */
	uint64_t xnp_able                     : 1;  /**< Extended next page ability. */
	uint64_t asm_dir                      : 1;  /**< Asymmetric PAUSE. */
	uint64_t pause                        : 1;  /**< PAUSE ability. */
	uint64_t e                            : 5;  /**< Echoed nonce. */
	uint64_t s                            : 5;  /**< Selector. */
#else
	uint64_t s                            : 5;
	uint64_t e                            : 5;
	uint64_t pause                        : 1;
	uint64_t asm_dir                      : 1;
	uint64_t xnp_able                     : 1;
	uint64_t rf                           : 1;
	uint64_t ack                          : 1;
	uint64_t np                           : 1;
	uint64_t t                            : 5;
	uint64_t a1g_kx                       : 1;
	uint64_t a10g_kx4                     : 1;
	uint64_t a10g_kr                      : 1;
	uint64_t a40g_kr4                     : 1;
	uint64_t a40g_cr4                     : 1;
	uint64_t a100g_cr10                   : 1;
	uint64_t arsv                         : 19;
	uint64_t fec_able                     : 1;
	uint64_t fec_req                      : 1;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_spux_an_lp_base_s    cn78xx;
};
typedef union cvmx_bgxx_spux_an_lp_base cvmx_bgxx_spux_an_lp_base_t;

/**
 * cvmx_bgx#_spu#_an_lp_xnp
 *
 * This register captures the contents of the latest next page code word received from the link
 * partner during Auto-Negotiation, if any. See section 802.3 section 73.7.7 for details.
 */
union cvmx_bgxx_spux_an_lp_xnp {
	uint64_t u64;
	struct cvmx_bgxx_spux_an_lp_xnp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t u                            : 32; /**< Unformatted code field. */
	uint64_t np                           : 1;  /**< Next page. */
	uint64_t ack                          : 1;  /**< Acknowledge. */
	uint64_t mp                           : 1;  /**< Message page. */
	uint64_t ack2                         : 1;  /**< Acknowledge 2. */
	uint64_t toggle                       : 1;  /**< Toggle. */
	uint64_t m_u                          : 11; /**< Message/unformatted code field. */
#else
	uint64_t m_u                          : 11;
	uint64_t toggle                       : 1;
	uint64_t ack2                         : 1;
	uint64_t mp                           : 1;
	uint64_t ack                          : 1;
	uint64_t np                           : 1;
	uint64_t u                            : 32;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_spux_an_lp_xnp_s     cn78xx;
};
typedef union cvmx_bgxx_spux_an_lp_xnp cvmx_bgxx_spux_an_lp_xnp_t;

/**
 * cvmx_bgx#_spu#_an_status
 */
union cvmx_bgxx_spux_an_status {
	uint64_t u64;
	struct cvmx_bgxx_spux_an_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t prl_flt                      : 1;  /**< Parallel detection fault. Always 0; SPU does not support parallel detection as part of the
                                                         auto-negotiation protocol. */
	uint64_t reserved_8_8                 : 1;
	uint64_t xnp_stat                     : 1;  /**< Extended next-page status. */
	uint64_t page_rx                      : 1;  /**< Page received. This latching-high bit is set when a new page has been received and stored
                                                         in BGXn_SPUm_AN_LP_BASE or BGXn_SPUm_AN_LP_XNP; stays set until a 1 is written by
                                                         software, Auto-Negotiation is disabled or restarted, or next page exchange is initiated.
                                                         Note that in order to avoid read side effects, this is implemented as a write-1-to-clear
                                                         bit, rather than latching high read-only as specified in 802.3. */
	uint64_t an_complete                  : 1;  /**< Auto-Negotiation complete. Set when the Auto-Negotiation process has been completed and
                                                         the link is up and running using the negotiated highest common denominator (HCD)
                                                         technology. If AN is enabled (BGXn_SPUm_AN_CONTROL[AN_EN] = 1) and this bit is read as a
                                                         zero, it indicates that the AN process has not been completed, and the contents of
                                                         BGXn_SPUm_AN_LP_BASE, BGXn_SPUm_AN_XNP_TX, and BGXn_SPUm_AN_LP_XNP are as defined by the
                                                         current state of the Auto-Negotiation protocol, or as written for manual configuration.
                                                         This bit is always zero when AN is disabled (BGXn_SPUm_AN_CONTROL[AN_EN] = 0). */
	uint64_t rmt_flt                      : 1;  /**< Remote fault: Always 0. */
	uint64_t an_able                      : 1;  /**< Auto-Negotiation ability: Always 1. */
	uint64_t link_status                  : 1;  /**< Link status. This bit captures the state of the link_status variable as defined in 802.3
                                                         section 73.9.1. When set, indicates that a valid link has been established. When clear,
                                                         indicates that the link has been invalid after this bit was last set by software. Latching
                                                         low bit; stays clear until a 1 is written by software. Note that in order to avoid read
                                                         side effects, this is implemented as a write-1-to-set bit, rather than latching low read-
                                                         only as specified in 802.3. */
	uint64_t reserved_1_1                 : 1;
	uint64_t lp_an_able                   : 1;  /**< Link partner Auto-Negotiation ability. Set to indicate that the link partner is able to
                                                         participate in the Auto-Negotiation function, and cleared otherwise. */
#else
	uint64_t lp_an_able                   : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t link_status                  : 1;
	uint64_t an_able                      : 1;
	uint64_t rmt_flt                      : 1;
	uint64_t an_complete                  : 1;
	uint64_t page_rx                      : 1;
	uint64_t xnp_stat                     : 1;
	uint64_t reserved_8_8                 : 1;
	uint64_t prl_flt                      : 1;
	uint64_t reserved_10_63               : 54;
#endif
	} s;
	struct cvmx_bgxx_spux_an_status_s     cn78xx;
};
typedef union cvmx_bgxx_spux_an_status cvmx_bgxx_spux_an_status_t;

/**
 * cvmx_bgx#_spu#_an_xnp_tx
 *
 * Software programs this register with the contents of the AN message next page or unformatted
 * next page link code word to be transmitted during auto-negotiation. Next page exchange occurs
 * after the base link code words have been exchanged if either end of the link segment sets the
 * NP bit to 1, indicating that it has at least one next page to send. Once initiated, next page
 * exchange continues until both end of the link segment set their NP bits to 0. See section
 * 802.3 section 73.7.7 for details.
 */
union cvmx_bgxx_spux_an_xnp_tx {
	uint64_t u64;
	struct cvmx_bgxx_spux_an_xnp_tx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t u                            : 32; /**< Unformatted code field. When the MP bit is set, this field contains the 32-bit unformatted
                                                         code field of the message next page. When MP is clear, this field contains the upper 32
                                                         bits of the 43-bit unformatted code field of the unformatted next page. */
	uint64_t np                           : 1;  /**< Next page. */
	uint64_t ack                          : 1;  /**< Ack: Always 0 in this register. */
	uint64_t mp                           : 1;  /**< Message page. Set to indicate that this register contains a message next page. Clear to
                                                         indicate that the register contains anunformatted next page. */
	uint64_t ack2                         : 1;  /**< Acknowledge 2. Indicates that the receiver is able to act on the information (or perform
                                                         the task) defined in the message. */
	uint64_t toggle                       : 1;  /**< This bit is ignored by hardware. The value of the TOGGLE bit in
                                                         transmitted next pages is automatically generated by hardware. */
	uint64_t m_u                          : 11; /**< Message/Unformatted code field: When the MP bit is set, this field contains the message
                                                         code field (M) of the message next page. When MP is clear, this field contains the lower
                                                         11 bits of the 43-bit unformatted code field of the unformatted next page. */
#else
	uint64_t m_u                          : 11;
	uint64_t toggle                       : 1;
	uint64_t ack2                         : 1;
	uint64_t mp                           : 1;
	uint64_t ack                          : 1;
	uint64_t np                           : 1;
	uint64_t u                            : 32;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_bgxx_spux_an_xnp_tx_s     cn78xx;
};
typedef union cvmx_bgxx_spux_an_xnp_tx cvmx_bgxx_spux_an_xnp_tx_t;

/**
 * cvmx_bgx#_spu#_br_algn_status
 *
 * This register implements the Std 802.3 multilane BASE-R PCS alignment status 1-4 registers
 * (3.50-3.53). It is valid only when the LPCS type is 40GBASE-R (BGXn_CMRm_CONFIG[LMAC_TYPE] =
 * 0x4), and always returns 0x0 for all other LPCS types. Std 802.3 bits that are not applicable
 * to 40GBASE-R (e.g. status bits for PCS lanes 19-4) are not implemented and marked as reserved.
 * PCS lanes 3-0 are valid and are mapped to physical SerDes lanes based on the programming of
 * BGXn_CMRm_CONFIG[[LANE_TO_SDS].
 */
union cvmx_bgxx_spux_br_algn_status {
	uint64_t u64;
	struct cvmx_bgxx_spux_br_algn_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t marker_lock                  : 4;  /**< Marker-locked status for PCS lanes 3-0.
                                                         1 = locked, 0 = not locked */
	uint64_t reserved_13_31               : 19;
	uint64_t alignd                       : 1;  /**< All lanes are locked and aligned. This bit returns 1 when the logical PCS has locked and
                                                         aligned all associated receive lanes; returns 0 otherwise. For all other PCS types, this
                                                         bit always returns 0. */
	uint64_t reserved_4_11                : 8;
	uint64_t block_lock                   : 4;  /**< Block-lock status for PCS lanes 3-0: 1 = locked, 0 = not locked */
#else
	uint64_t block_lock                   : 4;
	uint64_t reserved_4_11                : 8;
	uint64_t alignd                       : 1;
	uint64_t reserved_13_31               : 19;
	uint64_t marker_lock                  : 4;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_bgxx_spux_br_algn_status_s cn78xx;
};
typedef union cvmx_bgxx_spux_br_algn_status cvmx_bgxx_spux_br_algn_status_t;

/**
 * cvmx_bgx#_spu#_br_bip_err_cnt
 *
 * This register implements the Std 802.3 BIP error-counter registers for PCS lanes 0-3
 * (3.200-3.203). It is valid only when the LPCS type is 40GBASE-R (BGXn_CMRm_CONFIG[LMAC_TYPE] =
 * 0x4), and always returns 0x0 for all other LPCS types. The counters are indexed by the RX PCS
 * lane number based on the Alignment Marker detected on each lane and captured in
 * BGX(0..5)_SPU(0..3)_BR_LANE_MAP. Each counter counts the BIP errors for its PCS lane, and is
 * held at all ones in case of overflow. The counters are reset to all 0s when this register is
 * read by software.
 * The reset operation takes precedence over the increment operation; if the register is read on
 * the same clock cycle as an increment operation, the counter is reset to all 0s and the
 * increment operation is lost. The counters are writable for test purposes, rather than read-
 * only as specified in Std 802.3.
 */
union cvmx_bgxx_spux_br_bip_err_cnt {
	uint64_t u64;
	struct cvmx_bgxx_spux_br_bip_err_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bip_err_cnt_ln3              : 16; /**< BIP error counter for lane on which PCS lane 3 markers are received. */
	uint64_t bip_err_cnt_ln2              : 16; /**< BIP error counter for lane on which PCS lane 2 markers are received. */
	uint64_t bip_err_cnt_ln1              : 16; /**< BIP error counter for lane on which PCS lane 1 markers are received. */
	uint64_t bip_err_cnt_ln0              : 16; /**< BIP error counter for lane on which PCS lane 0 markers are received. */
#else
	uint64_t bip_err_cnt_ln0              : 16;
	uint64_t bip_err_cnt_ln1              : 16;
	uint64_t bip_err_cnt_ln2              : 16;
	uint64_t bip_err_cnt_ln3              : 16;
#endif
	} s;
	struct cvmx_bgxx_spux_br_bip_err_cnt_s cn78xx;
};
typedef union cvmx_bgxx_spux_br_bip_err_cnt cvmx_bgxx_spux_br_bip_err_cnt_t;

/**
 * cvmx_bgx#_spu#_br_lane_map
 *
 * This register implements the Std 802.3 lane 0-3 mapping registers (3.400-3.403). It is valid
 * only when the LPCS type is 40GBASE-R (BGXn_CMRm_CONFIG[LMAC_TYPE] = 0x4), and always returns
 * 0x0 for all other LPCS types. The LNx_MAPPING field for each programmed PCS lane (called
 * service interface in 802.3ba-2010) is valid when that lane has achieved alignment marker lock
 * on the receive side (i.e. the associated BGXn_SPUm_BR_ALGN_STATUS[MARKER_LOCK] = 1), and is
 * invalid otherwise. When valid, it returns the actual detected receive PCS lane number based on
 * the received alignment marker contents received on that service interface.
 * The mapping is flexible because Std 802.3 allows multilane BASE-R receive lanes to be re-
 * ordered. Note that for the transmit side, each PCS lane is mapped to a physical SerDes lane
 * based on the programming of BGXn_CMRm_CONFIG[LANE_TO_SDS]. For the receive side,
 * BGXn_CMRm_CONFIG[LANE_TO_SDS] specifies the service interface to physical SerDes lane mapping,
 * and this register specifies the PCS lane to service interface mapping.
 */
union cvmx_bgxx_spux_br_lane_map {
	uint64_t u64;
	struct cvmx_bgxx_spux_br_lane_map_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_54_63               : 10;
	uint64_t ln3_mapping                  : 6;  /**< PCS lane number received on service interface 3 */
	uint64_t reserved_38_47               : 10;
	uint64_t ln2_mapping                  : 6;  /**< PCS lane number received on service interface 2 */
	uint64_t reserved_22_31               : 10;
	uint64_t ln1_mapping                  : 6;  /**< PCS lane number received on service interface 1 */
	uint64_t reserved_6_15                : 10;
	uint64_t ln0_mapping                  : 6;  /**< PCS lane number received on service interface 0 */
#else
	uint64_t ln0_mapping                  : 6;
	uint64_t reserved_6_15                : 10;
	uint64_t ln1_mapping                  : 6;
	uint64_t reserved_22_31               : 10;
	uint64_t ln2_mapping                  : 6;
	uint64_t reserved_38_47               : 10;
	uint64_t ln3_mapping                  : 6;
	uint64_t reserved_54_63               : 10;
#endif
	} s;
	struct cvmx_bgxx_spux_br_lane_map_s   cn78xx;
};
typedef union cvmx_bgxx_spux_br_lane_map cvmx_bgxx_spux_br_lane_map_t;

/**
 * cvmx_bgx#_spu#_br_pmd_control
 */
union cvmx_bgxx_spux_br_pmd_control {
	uint64_t u64;
	struct cvmx_bgxx_spux_br_pmd_control_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t train_en                     : 1;  /**< BASE-R training enable */
	uint64_t train_restart                : 1;  /**< BASE-R training restart. Writing a 1 to this bit restarts the training process if the
                                                         TRAIN_EN bit in this register is also set. This is a self-clearing bit. Software should
                                                         wait a minimum of 1.7ms after BGX(0..5)_SPU(0..3)_INT[TRAINING_FAILURE] is set before
                                                         restarting the training process. */
#else
	uint64_t train_restart                : 1;
	uint64_t train_en                     : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_bgxx_spux_br_pmd_control_s cn78xx;
};
typedef union cvmx_bgxx_spux_br_pmd_control cvmx_bgxx_spux_br_pmd_control_t;

/**
 * cvmx_bgx#_spu#_br_pmd_ld_cup
 *
 * This register implements 802.3 MDIO register 1.153 for 10GBASE-R (when LMAC_TYPE = 10G_R in
 * the associated BGX_CMR_CONFIG register) and MDIO registers 1.1300-1.1303 for 40GBASE-R (when
 * LMAC_TYPE = 40G_R). It is automatically cleared at the start of training. When link training
 * is in progress, each field reflects the contents of the coefficient update field in the
 * associated lane's outgoing training frame. The fields in this register are read/write even
 * though they are specified as read-only in 802.3. If DBG_CONTROL[BR_PMD_TRAIN_SOFT_EN] is set,
 * then this register must be updated by software during link training and hardware updates are
 * disabled. If DBG_CONTROL[BR_PMD_TRAIN_SOFT_EN] is clear, this register is automatically
 * updated by hardware, and it should not be written by software. The lane fields in this
 * register are indexed by logical PCS lane ID. The lane 0 field (LN0_*) is valid for both
 * 10GBASE-R and 40GBASE-R. The remaining fields (LN1_*, LN2_*, LN3_*) are only valid for
 * 40GBASE-R.
 */
union cvmx_bgxx_spux_br_pmd_ld_cup {
	uint64_t u64;
	struct cvmx_bgxx_spux_br_pmd_ld_cup_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ln3_cup                      : 16; /**< PCS lane 3 coefficient update: format defined by BGX_SPU_BR_TRAIN_CUP_S. Not valid for
                                                         10GBASE-R. */
	uint64_t ln2_cup                      : 16; /**< PCS lane 2 coefficient update: format defined by BGX_SPU_BR_TRAIN_CUP_S. Not valid for
                                                         10GBASE-R. */
	uint64_t ln1_cup                      : 16; /**< PCS lane 1 coefficient update: format defined by BGX_SPU_BR_TRAIN_CUP_S. Not valid for
                                                         10GBASE-R. */
	uint64_t ln0_cup                      : 16; /**< PCS lane 0 coefficient update: format defined by BGX_SPU_BR_TRAIN_CUP_S. */
#else
	uint64_t ln0_cup                      : 16;
	uint64_t ln1_cup                      : 16;
	uint64_t ln2_cup                      : 16;
	uint64_t ln3_cup                      : 16;
#endif
	} s;
	struct cvmx_bgxx_spux_br_pmd_ld_cup_s cn78xx;
};
typedef union cvmx_bgxx_spux_br_pmd_ld_cup cvmx_bgxx_spux_br_pmd_ld_cup_t;

/**
 * cvmx_bgx#_spu#_br_pmd_ld_rep
 *
 * This register implements 802.3 MDIO register 1.154 for 10GBASE-R (when LMAC_TYPE = 10G_R in
 * the associated BGX_CMR_CONFIG register) and MDIO registers 1.1400-1.1403 for 40GBASE-R (when
 * LMAC_TYPE = 40G_R). It is automatically cleared at the start of training. Each field reflects
 * the contents of the status report field in the associated lane's outgoing training frame. The
 * fields in this register are read/write even though they are specified as read-only in 802.3.
 * If DBG_CONTROL[BR_PMD_TRAIN_SOFT_EN] is set, then this register must be updated by software
 * during link training and hardware updates are disabled. If DBG_CONTROL[BR_PMD_TRAIN_SOFT_EN]
 * is clear, this register is automatically updated by hardware, and it should not be written by
 * software. The lane fields in this register are indexed by logical PCS lane ID. The lane 0
 * field (LN0_*) is valid for both 10GBASE-R and 40GBASE-R. The remaining fields (LN1_*, LN2_*,
 * LN3_*) are only valid for 40GBASE-R.
 */
union cvmx_bgxx_spux_br_pmd_ld_rep {
	uint64_t u64;
	struct cvmx_bgxx_spux_br_pmd_ld_rep_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ln3_rep                      : 16; /**< PCS lane 3 status report: format defined by BGX_SPU_BR_TRAIN_REP_S. Not valid for
                                                         10GBASE-R. */
	uint64_t ln2_rep                      : 16; /**< PCS lane 2 status report: format defined by BGX_SPU_BR_TRAIN_REP_S. Not valid for
                                                         10GBASE-R. */
	uint64_t ln1_rep                      : 16; /**< PCS lane 1 status report: format defined by BGX_SPU_BR_TRAIN_REP_S. Not valid for
                                                         10GBASE-R. */
	uint64_t ln0_rep                      : 16; /**< PCS lane 0 status report: format defined by BGX_SPU_BR_TRAIN_REP_S. */
#else
	uint64_t ln0_rep                      : 16;
	uint64_t ln1_rep                      : 16;
	uint64_t ln2_rep                      : 16;
	uint64_t ln3_rep                      : 16;
#endif
	} s;
	struct cvmx_bgxx_spux_br_pmd_ld_rep_s cn78xx;
};
typedef union cvmx_bgxx_spux_br_pmd_ld_rep cvmx_bgxx_spux_br_pmd_ld_rep_t;

/**
 * cvmx_bgx#_spu#_br_pmd_lp_cup
 *
 * This register implements 802.3 MDIO register 1.152 for 10GBASE-R (when LMAC_TYPE = 10G_R in
 * the associated BGX_CMR_CONFIG register) and MDIO registers 1.1100-1.1103 for 40GBASE-R (when
 * LMAC_TYPE = 40G_R). It is automatically cleared at the start of training. Each field reflects
 * the contents of the coefficient update field in the lane's most recently received training
 * frame. This register should not be written when link training is enabled, i.e. when TRAIN_EN
 * is set BR_PMD_CONTROL. The lane fields in this register are indexed by logical PCS lane ID.
 * The lane 0 field (LN0_*) is valid for both 10GBASE-R and 40GBASE-R. The remaining fields
 * (LN1_*, LN2_*, LN3_*) are only valid for 40GBASE-R.
 */
union cvmx_bgxx_spux_br_pmd_lp_cup {
	uint64_t u64;
	struct cvmx_bgxx_spux_br_pmd_lp_cup_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ln3_cup                      : 16; /**< PCS lane 3 coefficient update: format defined by BGX_SPU_BR_TRAIN_CUP_S. Not valid for
                                                         10GBASE-R. */
	uint64_t ln2_cup                      : 16; /**< PCS lane 2 coefficient update: format defined by BGX_SPU_BR_TRAIN_CUP_S. Not valid for
                                                         10GBASE-R. */
	uint64_t ln1_cup                      : 16; /**< PCS lane 1 coefficient update: format defined by BGX_SPU_BR_TRAIN_CUP_S. Not valid for
                                                         10GBASE-R. */
	uint64_t ln0_cup                      : 16; /**< PCS lane 0 coefficient update: format defined by BGX_SPU_BR_TRAIN_CUP_S. */
#else
	uint64_t ln0_cup                      : 16;
	uint64_t ln1_cup                      : 16;
	uint64_t ln2_cup                      : 16;
	uint64_t ln3_cup                      : 16;
#endif
	} s;
	struct cvmx_bgxx_spux_br_pmd_lp_cup_s cn78xx;
};
typedef union cvmx_bgxx_spux_br_pmd_lp_cup cvmx_bgxx_spux_br_pmd_lp_cup_t;

/**
 * cvmx_bgx#_spu#_br_pmd_lp_rep
 *
 * This register implements 802.3 MDIO register 1.153 for 10GBASE-R (when LMAC_TYPE = 10G_R in
 * the associated BGX_CMR_CONFIG register) and MDIO registers 1.1200-1.1203 for 40GBASE-R (when
 * LMAC_TYPE = 40G_R). It is automatically cleared at the start of training. Each field reflects
 * the contents of the status report field in the associated lane's most recently received
 * training frame. The lane fields in this register are indexed by logical PCS lane ID. The lane
 * 0 field (LN0_*) is valid for both 10GBASE-R and 40GBASE-R. The remaining fields (LN1_*, LN2_*,
 * LN3_*) are only valid for 40GBASE-R.
 */
union cvmx_bgxx_spux_br_pmd_lp_rep {
	uint64_t u64;
	struct cvmx_bgxx_spux_br_pmd_lp_rep_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ln3_rep                      : 16; /**< PCS lane 3 status report: format defined by BGX_SPU_BR_TRAIN_REP_S. Not valid for
                                                         10GBASE-R. */
	uint64_t ln2_rep                      : 16; /**< PCS lane 2 status report: format defined by BGX_SPU_BR_TRAIN_REP_S. Not valid for
                                                         10GBASE-R. */
	uint64_t ln1_rep                      : 16; /**< PCS lane 1 status report: format defined by BGX_SPU_BR_TRAIN_REP_S. Not valid for
                                                         10GBASE-R. */
	uint64_t ln0_rep                      : 16; /**< PCS lane 0 status report: format defined by BGX_SPU_BR_TRAIN_REP_S. */
#else
	uint64_t ln0_rep                      : 16;
	uint64_t ln1_rep                      : 16;
	uint64_t ln2_rep                      : 16;
	uint64_t ln3_rep                      : 16;
#endif
	} s;
	struct cvmx_bgxx_spux_br_pmd_lp_rep_s cn78xx;
};
typedef union cvmx_bgxx_spux_br_pmd_lp_rep cvmx_bgxx_spux_br_pmd_lp_rep_t;

/**
 * cvmx_bgx#_spu#_br_pmd_status
 *
 * The lane fields in this register are indexed by logical PCS lane ID. The lane 0 field (LN0_*)
 * is valid for both 10GBASE-R and 40GBASE-R. The remaining fields (LN1_*, LN2_*, LN3_*) are only
 * valid for 40GBASE-R.
 */
union cvmx_bgxx_spux_br_pmd_status {
	uint64_t u64;
	struct cvmx_bgxx_spux_br_pmd_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t ln3_train_status             : 4;  /**< PCS lane 3 link training status. Format defined by BGX_SPU_BR_LANE_TRAIN_STATUS_S. Not
                                                         valid for 10GBASE-R. */
	uint64_t ln2_train_status             : 4;  /**< PCS lane 2 link training status. Format defined by BGX_SPU_BR_LANE_TRAIN_STATUS_S. Not
                                                         valid for 10GBASE-R. */
	uint64_t ln1_train_status             : 4;  /**< PCS lane 1 link training status. Format defined by BGX_SPU_BR_LANE_TRAIN_STATUS_S. Not
                                                         valid for 10GBASE-R. */
	uint64_t ln0_train_status             : 4;  /**< PCS lane 0 link training status. Format defined by BGX_SPU_BR_LANE_TRAIN_STATUS_S. */
#else
	uint64_t ln0_train_status             : 4;
	uint64_t ln1_train_status             : 4;
	uint64_t ln2_train_status             : 4;
	uint64_t ln3_train_status             : 4;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_spux_br_pmd_status_s cn78xx;
};
typedef union cvmx_bgxx_spux_br_pmd_status cvmx_bgxx_spux_br_pmd_status_t;

/**
 * cvmx_bgx#_spu#_br_status1
 */
union cvmx_bgxx_spux_br_status1 {
	uint64_t u64;
	struct cvmx_bgxx_spux_br_status1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_13_63               : 51;
	uint64_t rcv_lnk                      : 1;  /**< BASE-R receive link status.
                                                         1 = BASE-R PCS receive-link up
                                                         0 = BASE-R PCS receive-link down.
                                                         This bit is a reflection of the PCS_status variable defined in Std 802.3 sections
                                                         49.2.14.1 and 82.3.1. */
	uint64_t reserved_4_11                : 8;
	uint64_t prbs9                        : 1;  /**< 10GBASE-R PRBS9 pattern testing ability. Always 0; PRBS9 pattern testing is not supported. */
	uint64_t prbs31                       : 1;  /**< 10GBASE-R PRBS31 pattern testing ability. Always 0; PRBS31 pattern testing is not supported. */
	uint64_t hi_ber                       : 1;  /**< BASE-R PCS high bit-error rate.
                                                         1 = 64B/66B receiver is detecting a bit-error rate of >= 10.4
                                                         0 = 64B/66B receiver is detecting a bit-error rate of < 10.4
                                                         This bit is a direct reflection of the state of the HI_BER variable in the 64B/66B state
                                                         diagram and is defined in Std 802.3 sections 49.2.13.2.2 and 82.2.18.2.2. */
	uint64_t blk_lock                     : 1;  /**< BASE-R PCS block lock.
                                                         1 = 64B/66B receiver for BASE-R has block lock
                                                         0 = No block lock
                                                         This bit is a direct reflection of the state of the BLOCK_LOCK variable in the 64B/66B
                                                         state diagram and is defined in Std 802.3 sections 49.2.13.2.2 and 82.2.18.2.2.
                                                         For a multilane logical PCS (i.e. 40GBASE-R), this bit indicates that the receiver has
                                                         both block lock and alignment for all lanes and is identical to
                                                         BGXn_SPUm_BR_ALGN_STATUS[ALIGND]. */
#else
	uint64_t blk_lock                     : 1;
	uint64_t hi_ber                       : 1;
	uint64_t prbs31                       : 1;
	uint64_t prbs9                        : 1;
	uint64_t reserved_4_11                : 8;
	uint64_t rcv_lnk                      : 1;
	uint64_t reserved_13_63               : 51;
#endif
	} s;
	struct cvmx_bgxx_spux_br_status1_s    cn78xx;
};
typedef union cvmx_bgxx_spux_br_status1 cvmx_bgxx_spux_br_status1_t;

/**
 * cvmx_bgx#_spu#_br_status2
 *
 * This register implements a combination of the following Std 802.3 registers:
 * BASE-R PCS status 2 (MDIO address 3.33)
 * BASE-R BER high-order counter (MDIO address 3.44)
 * Errored-blocks high-order counter (MDIO address 3.45).
 * Note that the relative locations of some fields have been moved from Std 802.3 in order to
 * make the register layout more software friendly: the BER counter high-order and low-order bits
 * from sections 3.44 and 3.33 have been combined into the contiguous, 22-bit BER_CNT field;
 * likewise, the errored-blocks counter high-order and low-order bits from section 3.45 have been
 * combined into the contiguous, 22-bit ERR_BLKS field.
 */
union cvmx_bgxx_spux_br_status2 {
	uint64_t u64;
	struct cvmx_bgxx_spux_br_status2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t err_blks                     : 22; /**< Errored-blocks counter. This is the BASE-R errored-blocks counter as defined by the
                                                         errored_block_count variable specified in Std 802.3 sections 49.2.14.2 and 82.2.18.2.4. It
                                                         increments by 1 on each block for which the BASE-R receive state machine, specified in Std
                                                         802.3 diagrams 49-15 and 82-15, enters the RX_E state.
                                                         Back-to-back blocks in the RX_E state are counted as transitions from RX_E to RX_E and
                                                         keep incrementing the counter. The counter is reset to all 0s after this register is read
                                                         by software.
                                                         The reset operation takes precedence over the increment operation: if the register is read
                                                         on the same clock cycle as an increment operation, the counter is reset to all 0s and the
                                                         increment operation is lost.
                                                         This field is writable for test purposes, rather than read-only as specified in Std 802.3. */
	uint64_t reserved_38_39               : 2;
	uint64_t ber_cnt                      : 22; /**< Bit-error-rate counter. This is the BASE-R BER counter as defined by the ber_count
                                                         variable in Std 802.3 sections 49.2.14.2 and 82.2.18.2.4. The counter is reset to all 0s
                                                         after this register is read by software, and is held at all 1s in case of overflow.
                                                         The reset operation takes precedence over the increment operation: if the register is read
                                                         on the same clock cycle an increment operation, the counter is reset to all 0s and the
                                                         increment operation is lost.
                                                         This field is writable for test purposes, rather than read-only as specified in Std 802.3. */
	uint64_t latched_lock                 : 1;  /**< Latched-block lock.
                                                         1 = 64B/66B receiver for BASE-R has block lock
                                                         0 = No block
                                                         This is a latching-low version of BGXn_SPUm_BR_STATUS1[BLK_LOCK]; it stays clear until the
                                                         register is read by software.
                                                         Note that in order to avoid read side effects, this is implemented as a write-1-to-set
                                                         bit, rather than latching low read-only as specified in 802.3. */
	uint64_t latched_ber                  : 1;  /**< Latched-high bit-error rate.
                                                         1 = 64B/66B receiver is detecting a high BER
                                                         0 = Not a high BER
                                                         This is a latching-high version of BGXn_SPUm_BR_STATUS1[HI_BER]; it stays set until the
                                                         register is read by software.
                                                         Note that in order to avoid read side effects, this is implemented as a write-1-to-clear
                                                         bit, rather than latching high read-only as specified in 802.3. */
	uint64_t reserved_0_13                : 14;
#else
	uint64_t reserved_0_13                : 14;
	uint64_t latched_ber                  : 1;
	uint64_t latched_lock                 : 1;
	uint64_t ber_cnt                      : 22;
	uint64_t reserved_38_39               : 2;
	uint64_t err_blks                     : 22;
	uint64_t reserved_62_63               : 2;
#endif
	} s;
	struct cvmx_bgxx_spux_br_status2_s    cn78xx;
};
typedef union cvmx_bgxx_spux_br_status2 cvmx_bgxx_spux_br_status2_t;

/**
 * cvmx_bgx#_spu#_br_tp_control
 *
 * Refer to the test pattern methodology described in 802.3 sections 49.2.8 and 82.2.10.
 *
 */
union cvmx_bgxx_spux_br_tp_control {
	uint64_t u64;
	struct cvmx_bgxx_spux_br_tp_control_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t scramble_tp                  : 1;  /**< Select scrambled idle test pattern. This bit selects the transmit test pattern used when
                                                         TX_TP_EN is set:
                                                         1 = scrambled idle test pattern, 0 = square wave test pattern. */
	uint64_t prbs9_tx                     : 1;  /**< 10GBASE-R PRBS9 TP transmit enable. Always 0; PRBS9 pattern testing is not supported. */
	uint64_t prbs31_rx                    : 1;  /**< 10GBASE-R PRBS31 TP receive enable. Always 0; PRBS31 pattern testing is not supported. */
	uint64_t prbs31_tx                    : 1;  /**< 10GBASE-R PRBS31 TP transmit enable. Always 0; PRBS31 pattern is not supported. */
	uint64_t tx_tp_en                     : 1;  /**< Transmit-test-pattern enable. */
	uint64_t rx_tp_en                     : 1;  /**< Receive-test-pattern enable. The only supported receive test pattern is the scrambled idle
                                                         test pattern. Setting this bit enables checking of that receive pattern. */
	uint64_t tp_sel                       : 1;  /**< Square/PRBS test pattern select. Always 1 to select square wave test pattern; PRBS test
                                                         patterns are not supported. */
	uint64_t dp_sel                       : 1;  /**< Data pattern select. Always 0; PRBS test patterns are not supported. */
#else
	uint64_t dp_sel                       : 1;
	uint64_t tp_sel                       : 1;
	uint64_t rx_tp_en                     : 1;
	uint64_t tx_tp_en                     : 1;
	uint64_t prbs31_tx                    : 1;
	uint64_t prbs31_rx                    : 1;
	uint64_t prbs9_tx                     : 1;
	uint64_t scramble_tp                  : 1;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_bgxx_spux_br_tp_control_s cn78xx;
};
typedef union cvmx_bgxx_spux_br_tp_control cvmx_bgxx_spux_br_tp_control_t;

/**
 * cvmx_bgx#_spu#_br_tp_err_cnt
 *
 * This register provides the BASE-R PCS test-pattern error counter.
 *
 */
union cvmx_bgxx_spux_br_tp_err_cnt {
	uint64_t u64;
	struct cvmx_bgxx_spux_br_tp_err_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t err_cnt                      : 16; /**< Error counter. This 16-bit counter contains the number of errors received during a pattern
                                                         test. These bits are reset to all 0s when this register is read by software, and they are
                                                         held at all 1s in the case of overflow.
                                                         The test pattern methodology is described in Std 802.3, Sections 49.2.12 and 82.2.10. This
                                                         counter counts either block errors or bit errors dependent on the test mode (see Section
                                                         49.2.12). The reset operation takes precedence over the increment operation; if the
                                                         register is read on the same clock cycle as an increment operation, the counter is reset
                                                         to all 0s and the increment operation is lost. This field is writable for test purposes,
                                                         rather than read-only as specified in Std 802.3. */
#else
	uint64_t err_cnt                      : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_spux_br_tp_err_cnt_s cn78xx;
};
typedef union cvmx_bgxx_spux_br_tp_err_cnt cvmx_bgxx_spux_br_tp_err_cnt_t;

/**
 * cvmx_bgx#_spu#_bx_status
 */
union cvmx_bgxx_spux_bx_status {
	uint64_t u64;
	struct cvmx_bgxx_spux_bx_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_13_63               : 51;
	uint64_t alignd                       : 1;  /**< 10GBASE-X lane-alignment status.
                                                         1 = receive lanes aligned, 0 = receive lanes not aligned */
	uint64_t pattst                       : 1;  /**< Pattern-testing ability. Always 0; 10GBASE-X pattern is testing not supported. */
	uint64_t reserved_4_10                : 7;
	uint64_t lsync                        : 4;  /**< Lane synchronization. BASE-X lane synchronization status for PCS lanes 3-0. Each bit is
                                                         set when the associated lane is code-group synchronized, and clear otherwise. If the PCS
                                                         type is RXAUI (i.e. the associated BGXn_CMRm_CONFIG[LMAC_TYPE] = RXAUI), then only lanes
                                                         1-0 are valid. */
#else
	uint64_t lsync                        : 4;
	uint64_t reserved_4_10                : 7;
	uint64_t pattst                       : 1;
	uint64_t alignd                       : 1;
	uint64_t reserved_13_63               : 51;
#endif
	} s;
	struct cvmx_bgxx_spux_bx_status_s     cn78xx;
};
typedef union cvmx_bgxx_spux_bx_status cvmx_bgxx_spux_bx_status_t;

/**
 * cvmx_bgx#_spu#_control1
 */
union cvmx_bgxx_spux_control1 {
	uint64_t u64;
	struct cvmx_bgxx_spux_control1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t reset                        : 1;  /**< Reset. Setting this bit or BGXn_SPUm_AN_CONTROL[AN_RESET] to 1 causes the following to
                                                         happen:
                                                         Resets the logical PCS (LPCS)
                                                         Sets the Std 802.3 PCS, FEC and AN registers for the LPCS to their default states
                                                         Resets the associated SerDes lanes.
                                                         It takes up to 32 coprocessor-clock cycles to reset the LPCS, after which RESET is
                                                         automatically cleared. */
	uint64_t loopbck                      : 1;  /**< TX-to-RX loopback enable. When set, transmit data for each SerDes lane is looped back as
                                                         receive data. */
	uint64_t spdsel1                      : 1;  /**< Speed select 1: always 1. */
	uint64_t reserved_12_12               : 1;
	uint64_t lo_pwr                       : 1;  /**< Low power enable. When set, the LPCS is disabled (overriding the associated
                                                         BGXn_CMRm_CONFIG[ENABLE]), and the SerDes lanes associated with the LPCS are reset. */
	uint64_t reserved_7_10                : 4;
	uint64_t spdsel0                      : 1;  /**< Speed select 0: always 1. */
	uint64_t spd                          : 4;  /**< "Speed selection:
                                                         Note that this is a read-only field rather than read/write as
                                                         specified in 802.3. The Logical PCS speed is actually configured by
                                                         the LMAC_TYPE field in the associated BGX_CMR_CONFIG register in
                                                         the CMR sub-block. The Read values returned by this field are as
                                                         follows:
                                                           ----------+---------------------------------------------------
                                                           LMAC_TYPE |   Speed       SPD Read Value      Comment
                                                           ----------+---------------------------------------------------
                                                           XAUI      |   10G/20G     0x0                 20G if DXAUI
                                                           RXAUI     |   10G         0x0
                                                           10G_R     |   10G         0x0
                                                           40G_R     |   40G         0x3
                                                           Other     |   -           X
                                                           ----------+---------------------------------------------------" */
	uint64_t reserved_0_1                 : 2;
#else
	uint64_t reserved_0_1                 : 2;
	uint64_t spd                          : 4;
	uint64_t spdsel0                      : 1;
	uint64_t reserved_7_10                : 4;
	uint64_t lo_pwr                       : 1;
	uint64_t reserved_12_12               : 1;
	uint64_t spdsel1                      : 1;
	uint64_t loopbck                      : 1;
	uint64_t reset                        : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_spux_control1_s      cn78xx;
};
typedef union cvmx_bgxx_spux_control1 cvmx_bgxx_spux_control1_t;

/**
 * cvmx_bgx#_spu#_control2
 */
union cvmx_bgxx_spux_control2 {
	uint64_t u64;
	struct cvmx_bgxx_spux_control2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t pcs_type                     : 3;  /**< "PCS type selection:
                                                         Note that this is a read-only field rather than read/write as
                                                         specified in 802.3. The Logical PCS speed is actually configured by
                                                         the LMAC_TYPE field in the associated BGX_CMR_CONFIG register in
                                                         the CMR sub-block. The Read values returned by this field are as
                                                         follows:
                                                           ----------+------------------------------------------
                                                           LMAC_TYPE |   PCS_TYPE          Comment
                                                                         Read Value
                                                           ----------+------------------------------------------
                                                           XAUI      |   0x1               10GBASE-X PCS type
                                                           RXAUI     |   0x1               10GBASE-X PCS type
                                                           10G_R     |   0x0               10GBASE-R PCS type
                                                           40G_R     |   0x4               40GBASE-R PCS type
                                                           Other     |   Undefined         Reserved
                                                           ----------+------------------------------------------" */
#else
	uint64_t pcs_type                     : 3;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_bgxx_spux_control2_s      cn78xx;
};
typedef union cvmx_bgxx_spux_control2 cvmx_bgxx_spux_control2_t;

/**
 * cvmx_bgx#_spu#_fec_abil
 */
union cvmx_bgxx_spux_fec_abil {
	uint64_t u64;
	struct cvmx_bgxx_spux_fec_abil_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t err_abil                     : 1;  /**< BASE-R FEC error-indication ability. Always 1 when the LPCS type is BASE-R, i.e.
                                                         BGXn_CMRm_CONFIG[LMAC_TYPE] = 0x3 or 0x4. Always 0 otherwise. */
	uint64_t fec_abil                     : 1;  /**< BASE-R FEC ability. Always 1 when the LPCS type is BASE-R, i.e.
                                                         BGXn_CMRm_CONFIG[LMAC_TYPE] = 0x3 or 0x4. Always 0 otherwise. */
#else
	uint64_t fec_abil                     : 1;
	uint64_t err_abil                     : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_bgxx_spux_fec_abil_s      cn78xx;
};
typedef union cvmx_bgxx_spux_fec_abil cvmx_bgxx_spux_fec_abil_t;

/**
 * cvmx_bgx#_spu#_fec_control
 */
union cvmx_bgxx_spux_fec_control {
	uint64_t u64;
	struct cvmx_bgxx_spux_fec_control_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t err_en                       : 1;  /**< BASE-R FEC error-indication enable. This bit corresponds to FEC_Enable_Error_to_PCS
                                                         variable for BASE-R as defined in 802.3 Clause 74. When FEC is enabled (per FEC_EN bit in
                                                         this register) and this bit is set, the FEC decoder on the receive side signals an
                                                         uncorrectable FEC error to the BASE-R decoder by driving a value of 2'b11 on the sync bits
                                                         for some of the 32 64B/66B blocks belonging to the uncorrectable FEC block. See
                                                         802.3-2008/802.3ba-2010 section 74.7.4.5.1 for more details. */
	uint64_t fec_en                       : 1;  /**< BASE-R FEC enable. When this bit is set and the LPCS type is BASE-R
                                                         (BGXn_CMRm_CONFIG[LMAC_TYPE] = 0x4), forward error correction is enabled. FEC is disabled
                                                         otherwise. Forward error correction is defined in IEEE Std 802.3-2008/802.3ba-2010 Clause
                                                         74. */
#else
	uint64_t fec_en                       : 1;
	uint64_t err_en                       : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_bgxx_spux_fec_control_s   cn78xx;
};
typedef union cvmx_bgxx_spux_fec_control cvmx_bgxx_spux_fec_control_t;

/**
 * cvmx_bgx#_spu#_fec_corr_blks01
 *
 * This register is valid only when the LPCS type is BASE-R (BGXn_CMRm_CONFIG[LMAC_TYPE] = 0x3 or
 * 0x4). The FEC corrected-block counters are defined in Std 802.3 section 74.8.4.1. Each
 * corrected-blocks counter increments by 1 for a corrected FEC block, i.e. an FEC block that has
 * been received with invalid parity on the associated PCS lane and has been corrected by the FEC
 * decoder. The counter is reset to all 0s when the register is read, and held at all 1s in case
 * of overflow.
 * The reset operation takes precedence over the increment operation; if the register is read on
 * the same clock cycle as an increment operation, the counter is reset to all 0s and the
 * increment operation is lost. The counters are writable for test purposes, rather than read-
 * only as specified in Std 802.3.
 */
union cvmx_bgxx_spux_fec_corr_blks01 {
	uint64_t u64;
	struct cvmx_bgxx_spux_fec_corr_blks01_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ln1_corr_blks                : 32; /**< PCS Lane 1 FEC corrected blocks.
                                                         For 10GBASE-R, reserved.
                                                         For 40GBASE-R, correspond to the Std 802.3 FEC_corrected_blocks_counter_1 variable
                                                         (registers 1.302-1.303). */
	uint64_t ln0_corr_blks                : 32; /**< PCS Lane 0 FEC corrected blocks.
                                                         For 10GBASE-R, corresponds to the Std 802.3 FEC_corrected_blocks_counter variable
                                                         (registers 1.172-1.173).
                                                         For 40GBASE-R, correspond to the Std 802.3 FEC_corrected_blocks_counter_0 variable
                                                         (registers 1.300-1.301). */
#else
	uint64_t ln0_corr_blks                : 32;
	uint64_t ln1_corr_blks                : 32;
#endif
	} s;
	struct cvmx_bgxx_spux_fec_corr_blks01_s cn78xx;
};
typedef union cvmx_bgxx_spux_fec_corr_blks01 cvmx_bgxx_spux_fec_corr_blks01_t;

/**
 * cvmx_bgx#_spu#_fec_corr_blks23
 *
 * This register is valid only when the LPCS type is 40GBASE-R (BGXn_CMRm_CONFIG[LMAC_TYPE] =
 * 0x4). The FEC corrected-block counters are defined in Std 802.3 section 74.8.4.1. Each
 * corrected-blocks counter increments by 1 for a corrected FEC block, i.e. an FEC block that has
 * been received with invalid parity on the associated PCS lane and has been corrected by the FEC
 * decoder. The counter is reset to all 0s when the register is read, and held at all 1s in case
 * of overflow.
 * The reset operation takes precedence over the increment operation; if the register is read on
 * the same clock cycle as an increment operation, the counter is reset to all 0s and the
 * increment operation is lost. The counters are writable for test purposes, rather than read-
 * only as specified in Std 802.3.
 */
union cvmx_bgxx_spux_fec_corr_blks23 {
	uint64_t u64;
	struct cvmx_bgxx_spux_fec_corr_blks23_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ln3_corr_blks                : 32; /**< PCS Lane 3 FEC corrected blocks. Correspond to the Std 802.3
                                                         FEC_corrected_blocks_counter_3 variable (registers 1.306-1.307). */
	uint64_t ln2_corr_blks                : 32; /**< PCS Lane 2 FEC corrected blocks. Correspond to the Std 802.3
                                                         FEC_corrected_blocks_counter_3 variable (registers 1.304-1.305). */
#else
	uint64_t ln2_corr_blks                : 32;
	uint64_t ln3_corr_blks                : 32;
#endif
	} s;
	struct cvmx_bgxx_spux_fec_corr_blks23_s cn78xx;
};
typedef union cvmx_bgxx_spux_fec_corr_blks23 cvmx_bgxx_spux_fec_corr_blks23_t;

/**
 * cvmx_bgx#_spu#_fec_uncorr_blks01
 *
 * This register is valid only when the LPCS type is BASE-R (BGXn_CMRm_CONFIG[LMAC_TYPE] = 0x3 or
 * 0x4). The FEC corrected-block counters are defined in Std 802.3 section 74.8.4.2. Each
 * uncorrected-blocks counter increments by 1 for an uncorrected FEC block, i.e. an FEC block
 * that has been received with invalid parity on the associated PCS lane and has not been
 * corrected by the FEC decoder. The counter is reset to all 0s when the register is read, and
 * held at all 1s in case of overflow.
 * The reset operation takes precedence over the increment operation; if the register is read on
 * the same clock cycle as an increment operation, the counter is reset to all 0s and the
 * increment operation is lost. The counters are writable for test purposes, rather than read-
 * only as specified in Std 802.3.
 */
union cvmx_bgxx_spux_fec_uncorr_blks01 {
	uint64_t u64;
	struct cvmx_bgxx_spux_fec_uncorr_blks01_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ln1_uncorr_blks              : 32; /**< PCS Lane 1 FEC corrected blocks.
                                                         For 10GBASE-R, reserved.
                                                         For 40GBASE-R, corresponds to the Std 802.3 FEC_uncorrected_blocks_counter_1 variable
                                                         (registers 1.702-1.703). */
	uint64_t ln0_uncorr_blks              : 32; /**< PCS Lane 0 FEC uncorrected blocks.
                                                         For 10GBASE-R, corresponds to the Std 802.3 FEC_uncorrected_blocks_counter variable
                                                         (registers 1.174-1.175).
                                                         For 40GBASE-R, correspond to the Std 802.3 FEC_uncorrected_blocks_counter_0 variable
                                                         (registers 1.700-1.701). */
#else
	uint64_t ln0_uncorr_blks              : 32;
	uint64_t ln1_uncorr_blks              : 32;
#endif
	} s;
	struct cvmx_bgxx_spux_fec_uncorr_blks01_s cn78xx;
};
typedef union cvmx_bgxx_spux_fec_uncorr_blks01 cvmx_bgxx_spux_fec_uncorr_blks01_t;

/**
 * cvmx_bgx#_spu#_fec_uncorr_blks23
 *
 * This register is valid only when the LPCS type is 40GBASE-R (BGXn_CMRm_CONFIG[LMAC_TYPE] =
 * 0x4). The FEC uncorrected-block counters are defined in Std 802.3 section 74.8.4.2. Each
 * corrected-blocks counter increments by 1 for an uncorrected FEC block, i.e. an FEC block that
 * has been received with invalid parity on the associated PCS lane and has not been corrected by
 * the FEC decoder. The counter is reset to all 0s when the register is read, and held at all 1s
 * in case of overflow.
 * The reset operation takes precedence over the increment operation; if the register is read on
 * the same clock cycle as an increment operation, the counter is reset to all 0s and the
 * increment operation is lost. The counters are writable for test purposes, rather than read-
 * only as specified in Std 802.3.
 */
union cvmx_bgxx_spux_fec_uncorr_blks23 {
	uint64_t u64;
	struct cvmx_bgxx_spux_fec_uncorr_blks23_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ln3_uncorr_blks              : 32; /**< PCS Lane 3 FEC uncorrected blocks. Corresponds to the Std 802.3
                                                         FEC_uncorrected_blocks_counter_3 variable (registers 1.706-1.707). */
	uint64_t ln2_uncorr_blks              : 32; /**< PCS Lane 2 FEC uncorrected blocks. Corresponds to the Std 802.3
                                                         FEC_uncorrected_blocks_counter_3 variable (registers 1.704-1.705). */
#else
	uint64_t ln2_uncorr_blks              : 32;
	uint64_t ln3_uncorr_blks              : 32;
#endif
	} s;
	struct cvmx_bgxx_spux_fec_uncorr_blks23_s cn78xx;
};
typedef union cvmx_bgxx_spux_fec_uncorr_blks23 cvmx_bgxx_spux_fec_uncorr_blks23_t;

/**
 * cvmx_bgx#_spu#_int
 */
union cvmx_bgxx_spux_int {
	uint64_t u64;
	struct cvmx_bgxx_spux_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t training_failure             : 1;  /**< BASE-R PMD training failure. Set when BASE-R PMD link training has failed on the 10GBASE-R
                                                         lane or any 40GBASE-R lane. Valid if the LPCS type selected by
                                                         BGX(0..5)_CMR(0..3)_CONFIG[LMAC_TYPE] is 10GBASE-R or 40GBASE-R and
                                                         BGX(0..5)_SPU(0..3)_BR_PMD_CONTROL[TRAIN_EN] is 1, and never set otherwise. */
	uint64_t training_done                : 1;  /**< BASE-R PMD training done. Set when the 10GBASE-R lane or all 40GBASE-R lanes have
                                                         successfully completed BASE-R PMD link training. Valid if the LPCS type selected by
                                                         BGX(0..5)_CMR(0..3)_CONFIG[LMAC_TYPE] is 10GBASE-R or 40GBASE-R and
                                                         BGX(0..5)_SPU(0..3)_BR_PMD_CONTROL[TRAIN_EN] is 1, and never set otherwise. */
	uint64_t an_complete                  : 1;  /**< Auto-Negotiation complete. Set when BGX(0..5)_SPU(0..3)_AN_STATUS[AN_COMPLETE] is set,
                                                         indicating that the Auto-Negotiation process has been completed and the link is up and
                                                         running using the negotiated highest common denominator (HCD) technology. */
	uint64_t an_link_good                 : 1;  /**< Auto-Negotiation link good. Set when the an_link_good variable is set as defined in
                                                         802.3-2008 Figure 73-11, indicating that Auto-Negotiation has completed. */
	uint64_t an_page_rx                   : 1;  /**< Auto-Negotiation page received. This bit is set along with
                                                         BGX(0..5)_SPU(0..3)_AN_STATUS[PAGE_RX] when a new page has been received and stored in
                                                         BGX(0..5)_SPU(0..3)_AN_LP_BASE or BGX(0..5)_SPU(0..3)_AN_LP_XNP. */
	uint64_t fec_uncorr                   : 1;  /**< Uncorrectable FEC error. Set when an FEC block with an uncorrectable error is received on
                                                         the 10GBASE-R lane or any 40GBASE-R lane. Valid if the LPCS type selected by
                                                         BGX(0..5)_CMR(0..3)_CONFIG[LMAC_TYPE] is 10GBASE-R or 40GBASE-R, and never set otherwise. */
	uint64_t fec_corr                     : 1;  /**< Correctable FEC error. Set when an FEC block with a correctable error is received on the
                                                         10GBASE-R lane or any 40GBASE-R lane. Valid if the LPCS type selected by
                                                         BGX(0..5)_CMR(0..3)_CONFIG[LMAC_TYPE] is 10GBASE-R or 40GBASE-R, and never set otherwise. */
	uint64_t bip_err                      : 1;  /**< 40GBASE-R bit interleaved parity error. Set when a BIP error is detected on any lane.
                                                         Valid if the LPCS type selected by BGX(0..5)_CMR(0..3)_CONFIG[LMAC_TYPE] is 40GBASE-R, and
                                                         never set otherwise. */
	uint64_t dbg_sync                     : 1;  /**< Sync failure debug. This interrupt is provided for link problem debugging help. It is set
                                                         as follows based on the LPCS type selected by BGX(0..5)_CMR(0..3)_CONFIG[LMAC_TYPE], and
                                                         whether FEC is enabled or disabled by BGX(0..5)_SPU(0..3)_FEC_CONTROL[FEC_EN]:
                                                         XAUI or RXAUI: Set when any lane's PCS synchronization state transitions from
                                                         SYNC_ACQUIRED_1 to SYNC_ACQUIRED_2 (see 802.3-2008 Figure 48-7).
                                                         10GBASE-R or 40GBASE-R with FEC disabled: Set when sh_invalid_cnt increments to 1 while
                                                         block_lock is 1 (see 802.3-2008 Figure 49-12 and 802.3ba-2010 Figure 82-20).
                                                         10GBASE-R or 40GBASE-R with FEC enabled: Set when parity_invalid_cnt increments to 1 while
                                                         fec_block_lock is 1 (see 802.3-2008 Figure 74-8). */
	uint64_t algnlos                      : 1;  /**< Loss of lane alignment. Set when lane-to-lane alignment is lost. This is only valid if the
                                                         logical PCS is a multilane type (i.e. XAUI, RXAUI or 40GBASE-R is selected by
                                                         BGX(0..5)_CMR(0..3)_CONFIG[LMAC_TYPE]), and is never set otherwise. */
	uint64_t synlos                       : 1;  /**< Loss of lane sync. Lane code-group or block synchronization is lost on one or more lanes
                                                         associated with the LMAC/LPCS. Set as follows based on the LPCS type selected by
                                                         BGX(0..5)_CMR(0..3)_CONFIG[LMAC_TYPE], and whether FEC is enabled or disabled by
                                                         BGX_SPU_FEC_CONTROL[FEC_EN]:
                                                         XAUI or RXAUI: Set when any lane's PCS synchronization state transitions to LOSS_OF_SYNC
                                                         (see 802.3-2008 Figure 48-7)
                                                         10GBASE-R or 40GBASE-R with FEC disabled: set when the block_lock variable is cleared on
                                                         the 10G lane or any 40G lane (see 802.3-2008 Figure 49-12 and 802.3ba-2010 Figure 82-20).
                                                         10GBASE-R or 40GBASE-R with FEC enabled: set when the fec_block_lock variable is cleared
                                                         on the 10G lane or any 40G lane (see 802.3-2008 Figure 74-8). */
	uint64_t bitlckls                     : 1;  /**< Bit lock lost on one or more lanes associated with the LMAC/LPCS. */
	uint64_t err_blk                      : 1;  /**< Errored block received. Set when an errored BASE-R block is received as described for
                                                         BGX(0..5)_SPU(0..3)_BR_STATUS2[ERR_BLKS]. Valid if the LPCS type selected by
                                                         BGX(0..5)_CMR(0..3)_CONFIG[LMAC_TYPE] is 10GBASE-R or 40GBASE-R, and never set otherwise. */
	uint64_t rx_link_down                 : 1;  /**< Set when the receive link goes down, which is the same condition that sets
                                                         BGX(0..5)_SPU(0..3)_STATUS2[RCVFLT]. */
	uint64_t rx_link_up                   : 1;  /**< Set when the receive link comes up, which is the same condition that allows the setting of
                                                         BGX(0..5)_SPU(0..3)_STATUS1[RCV_LNK]. */
#else
	uint64_t rx_link_up                   : 1;
	uint64_t rx_link_down                 : 1;
	uint64_t err_blk                      : 1;
	uint64_t bitlckls                     : 1;
	uint64_t synlos                       : 1;
	uint64_t algnlos                      : 1;
	uint64_t dbg_sync                     : 1;
	uint64_t bip_err                      : 1;
	uint64_t fec_corr                     : 1;
	uint64_t fec_uncorr                   : 1;
	uint64_t an_page_rx                   : 1;
	uint64_t an_link_good                 : 1;
	uint64_t an_complete                  : 1;
	uint64_t training_done                : 1;
	uint64_t training_failure             : 1;
	uint64_t reserved_15_63               : 49;
#endif
	} s;
	struct cvmx_bgxx_spux_int_s           cn78xx;
};
typedef union cvmx_bgxx_spux_int cvmx_bgxx_spux_int_t;

/**
 * cvmx_bgx#_spu#_lpcs_states
 */
union cvmx_bgxx_spux_lpcs_states {
	uint64_t u64;
	struct cvmx_bgxx_spux_lpcs_states_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t br_rx_sm                     : 3;  /**< BASE-R receive state machine state */
	uint64_t reserved_10_11               : 2;
	uint64_t bx_rx_sm                     : 2;  /**< BASE-X receive state machine state */
	uint64_t deskew_am_found              : 4;  /**< 40GBASE-R deskew state machine alignment marker found flag per logical PCS lane ID. */
	uint64_t reserved_3_3                 : 1;
	uint64_t deskew_sm                    : 3;  /**< BASE-X and 40GBASE-R deskew state machine state */
#else
	uint64_t deskew_sm                    : 3;
	uint64_t reserved_3_3                 : 1;
	uint64_t deskew_am_found              : 4;
	uint64_t bx_rx_sm                     : 2;
	uint64_t reserved_10_11               : 2;
	uint64_t br_rx_sm                     : 3;
	uint64_t reserved_15_63               : 49;
#endif
	} s;
	struct cvmx_bgxx_spux_lpcs_states_s   cn78xx;
};
typedef union cvmx_bgxx_spux_lpcs_states cvmx_bgxx_spux_lpcs_states_t;

/**
 * cvmx_bgx#_spu#_misc_control
 *
 * RX logical PCS lane polarity vector [3:0] = XOR_RXPLRT[3:0] ^ [4[RXPLRT]].
 *  TX logical PCS lane polarity vector [3:0] = XOR_TXPLRT[3:0] ^ [4[TXPLRT]].
 *  In short, keep RXPLRT and TXPLRT cleared, and use XOR_RXPLRT and XOR_TXPLRT fields to define
 *  the polarity per logical PCS lane. Only bit 0 of vector is used for 10GBASE-R, and only bits
 * - 1:0 of vector are used for RXAUI.
 */
union cvmx_bgxx_spux_misc_control {
	uint64_t u64;
	struct cvmx_bgxx_spux_misc_control_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_13_63               : 51;
	uint64_t rx_packet_dis                : 1;  /**< Receive packet disable. Software can set or clear this bit at any time to gracefully
                                                         disable or re-enable packet reception by the LPCS. If this bit is set while a packet is
                                                         being received, the packet is completed and all subsequent received packets are discarded
                                                         by the LPCS. Similarly, if this bit is cleared while a received packet is being discarded,
                                                         packet reception resumes after the current packet is fully discarded. When set for a
                                                         40GBASE-R or 10GBASE-R LMAC/LPCS type (selected by BGX(0..5)_CMR(0..3)_CONFIG[LMAC_TYPE]),
                                                         received errors and faults will be ignored while receive packets are discarded; idles will
                                                         be sent to the MAC layer (SMU) and the errored blocks counter
                                                         (BGX(0..5)_SPU(0..3)_BR_STATUS2[ERR_BLKS]) will not increment. */
	uint64_t skip_after_term              : 1;  /**< Enable sending of Idle Skip after Terminate. This bit is meaningful when the logical PCS
                                                         type is XAUI or RXAUI (selected by BGX(0..5)_CMR(0..3)_CONFIG[LMAC_TYPE]), and has no
                                                         effect otherwise. When set, the LMAC/LPCS transmits more Idle Skip columns for clock
                                                         compensation. Typically set in HiGig/HiGig2 modes; clear otherwise. This field can be set
                                                         to ensure sufficient density of XAUI Idle Skip (||R||) columns with a small transmit
                                                         inter-frame gap (IFG) in order to allow the link partner's receiver to delete ||R
                                                         columns as needed for clock rate compensation. It is usually set when the LMAC's transmit
                                                         IFG is set to 8 bytes in HiGig/HiGig2 modes (i.e. BGX(0..5)_SMU(0..3)_TX_IFG[IFG1] +
                                                         BGX(0..5)_SMU(0..3)_TX_IFG[IFG2] = 8), and should be cleared when the transmit IFG is
                                                         greater than 8 bytes. When this bit is set, the SPU will send an ||R|| column after a
                                                         ||T0|| column (terminate in lane 0) if no ||R|| was sent in the previous IFG. This is a
                                                         minor deviation from the functionality specified in 802.3-2008 Figure 48-6 (PCS transmit
                                                         source state diagram), whereby the state will transition directly from SEND_DATA to
                                                         SEND_RANDOM_R after ||T0|| if no ||R|| was transmitted in the previous IFG. Sending ||R
                                                         after ||T0|| only (and not ||T1||, |T2|| or ||T3||) ensures that the check_end function at
                                                         the receiving end, as defined in 802.3-2008 sub-clause 48.2.6.1.4, does not detect an
                                                         error due to this functional change. When this bit is clear, the LMAC will fully conform
                                                         to the functionality specified in Figure 48-6. */
	uint64_t intlv_rdisp                  : 1;  /**< RXAUI interleaved running disparity. This bit is meaningful when the logical PCS type is
                                                         RXAUI (BGX(0..5)_CMR(0..3)_CONFIG[LMAC_TYPE] = RXAUI), and has no effect otherwise. It
                                                         selects which disparity calculation to use when combining or splitting the RXAUI lanes, as
                                                         follows:
                                                         0 = Common running disparity. Common running disparity is computed for even and odd code-
                                                         groups of an RXAUI lane, i.e. interleave lanes before PCS layer as described in the Dune
                                                         Networks/Broadcom RXAUI v2.1 specification. This obeys 6.25GHz serdes disparity.
                                                         1 = Interleaved running disparity: Running disparity is computed separately for even and
                                                         odd code-groups of an RXAUI lane, i.e. interleave lanes after PCS layer as described in
                                                         the Marvell RXAUI Interface specification. This does not obey 6.25GHz SerDes disparity. */
	uint64_t xor_rxplrt                   : 4;  /**< RX polarity control per logical PCS lane */
	uint64_t xor_txplrt                   : 4;  /**< TX polarity control per logical PCS lane */
	uint64_t rxplrt                       : 1;  /**< Receive polarity. 1 = inverted polarity, 0 = normal polarity. */
	uint64_t txplrt                       : 1;  /**< Transmit polarity. 1 = inverted polarity, 0 = normal polarity. */
#else
	uint64_t txplrt                       : 1;
	uint64_t rxplrt                       : 1;
	uint64_t xor_txplrt                   : 4;
	uint64_t xor_rxplrt                   : 4;
	uint64_t intlv_rdisp                  : 1;
	uint64_t skip_after_term              : 1;
	uint64_t rx_packet_dis                : 1;
	uint64_t reserved_13_63               : 51;
#endif
	} s;
	struct cvmx_bgxx_spux_misc_control_s  cn78xx;
};
typedef union cvmx_bgxx_spux_misc_control cvmx_bgxx_spux_misc_control_t;

/**
 * cvmx_bgx#_spu#_spd_abil
 */
union cvmx_bgxx_spux_spd_abil {
	uint64_t u64;
	struct cvmx_bgxx_spux_spd_abil_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t hundredgb                    : 1;  /**< 100G capable. Always 0. */
	uint64_t fortygb                      : 1;  /**< 40G capable. Always 1. */
	uint64_t tenpasst                     : 1;  /**< 10PASS-TS/2BASE-TL capable. Always 0. */
	uint64_t tengb                        : 1;  /**< 10G capable. Always 1. */
#else
	uint64_t tengb                        : 1;
	uint64_t tenpasst                     : 1;
	uint64_t fortygb                      : 1;
	uint64_t hundredgb                    : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_bgxx_spux_spd_abil_s      cn78xx;
};
typedef union cvmx_bgxx_spux_spd_abil cvmx_bgxx_spux_spd_abil_t;

/**
 * cvmx_bgx#_spu#_status1
 */
union cvmx_bgxx_spux_status1 {
	uint64_t u64;
	struct cvmx_bgxx_spux_status1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t flt                          : 1;  /**< Fault: 1 = fault condition detected, 0 = no fault condition detected.
                                                         This bit is a logical OR of
                                                         BGX(0..5)_SPU(0..3)_STATUS2[XMTFLT, RCVFLT]. */
	uint64_t reserved_3_6                 : 4;
	uint64_t rcv_lnk                      : 1;  /**< PCS receive link status: 1 = receive link up, 0 = receive link down.
                                                         This is a latching-low bit; it stays clear until the register is read by software.
                                                         For a BASE-X logical PCS type (in the associated BGXn_CMRm_CONFIG[LMAC_TYPE] = XAUI or
                                                         RXAUI), this is a latching-low version of BGXn_SPUm_BX_STATUS[ALIGND].
                                                         For a BASE-R logical PCS type (in the associated BGXn_CMRm_CONFIG[LMAC_TYPE] = 10G_R or
                                                         40G_R), this is a latching-low version of BGXn_SPUm_BR_STATUS1[RCV_LNK].
                                                         Note that in order to avoid read side effects, this is implemented as a write-1-to-set
                                                         bit, rather than latching low read-only as specified in 802.3. */
	uint64_t lpable                       : 1;  /**< Low-power ability. Always returns 1 to indicate that the LPCS supports low-power mode. */
	uint64_t reserved_0_0                 : 1;
#else
	uint64_t reserved_0_0                 : 1;
	uint64_t lpable                       : 1;
	uint64_t rcv_lnk                      : 1;
	uint64_t reserved_3_6                 : 4;
	uint64_t flt                          : 1;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_bgxx_spux_status1_s       cn78xx;
};
typedef union cvmx_bgxx_spux_status1 cvmx_bgxx_spux_status1_t;

/**
 * cvmx_bgx#_spu#_status2
 */
union cvmx_bgxx_spux_status2 {
	uint64_t u64;
	struct cvmx_bgxx_spux_status2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t dev                          : 2;  /**< Device present. Always returns 0x2 to indicate a device is present at this address. */
	uint64_t reserved_12_13               : 2;
	uint64_t xmtflt                       : 1;  /**< Transmit fault. Always returns 0. */
	uint64_t rcvflt                       : 1;  /**< Receive fault: 1 = receive fault, 0 = no receive fault. Latching high bit; stays set until
                                                         software writes a 1.
                                                         Note that in order to avoid read side effects, this is implemented as a write-1-to-clear
                                                         bit, rather than latching high read-only as specified in 802.3. */
	uint64_t reserved_6_9                 : 4;
	uint64_t hundredgb_r                  : 1;  /**< 100GBASE-R capable. Always 0. */
	uint64_t fortygb_r                    : 1;  /**< 40GBASE-R capable. Always 1. */
	uint64_t tengb_t                      : 1;  /**< 10GBASE-T capable. Always 0. */
	uint64_t tengb_w                      : 1;  /**< 10GBASE-W capable. Always 0. */
	uint64_t tengb_x                      : 1;  /**< 10GBASE-X capable. Always 1. */
	uint64_t tengb_r                      : 1;  /**< 10GBASE-R capable. Always 1. */
#else
	uint64_t tengb_r                      : 1;
	uint64_t tengb_x                      : 1;
	uint64_t tengb_w                      : 1;
	uint64_t tengb_t                      : 1;
	uint64_t fortygb_r                    : 1;
	uint64_t hundredgb_r                  : 1;
	uint64_t reserved_6_9                 : 4;
	uint64_t rcvflt                       : 1;
	uint64_t xmtflt                       : 1;
	uint64_t reserved_12_13               : 2;
	uint64_t dev                          : 2;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_bgxx_spux_status2_s       cn78xx;
};
typedef union cvmx_bgxx_spux_status2 cvmx_bgxx_spux_status2_t;

/**
 * cvmx_bgx#_spu_bist_status
 *
 * This register provides memory BIST status from the SPU RX_BUF lane FIFOs.
 *
 */
union cvmx_bgxx_spu_bist_status {
	uint64_t u64;
	struct cvmx_bgxx_spu_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t rx_buf_bist_status           : 4;  /**< SPU RX_BUF BIST status for lanes 3-0. One bit per SerDes lane, set to indicate BIST
                                                         failure for the associated RX_BUF lane FIFO. */
#else
	uint64_t rx_buf_bist_status           : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_bgxx_spu_bist_status_s    cn78xx;
};
typedef union cvmx_bgxx_spu_bist_status cvmx_bgxx_spu_bist_status_t;

/**
 * cvmx_bgx#_spu_dbg_control
 */
union cvmx_bgxx_spu_dbg_control {
	uint64_t u64;
	struct cvmx_bgxx_spu_dbg_control_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t ms_clk_period                : 12; /**< Millisecond clock period. Specifies the number of microsecond clock ticks per millisecond,
                                                         minus 1. The default value of 999 (0x3E7) should be used during normal operation; other
                                                         values may be used for test/debug purposes. */
	uint64_t us_clk_period                : 12; /**< Microsecond clock period. Specifies the number of SCLK cycles per microseconds, minus 1.
                                                         For example, if SCLK runs at 1.3 GHz, the number of SCLK cycles per microsecond is 1,300
                                                         so the value of this field should be 1,299 (0x513). This is used by the BASE-R BER monitor
                                                         timers. */
	uint64_t reserved_31_31               : 1;
	uint64_t br_ber_mon_dis               : 1;  /**< BASE-R bit error rate monitor disable. This bit should be clear for normal operation.
                                                         Setting it disables the BASE-R BER monitor state machine defined in 802.3-2008 Figure
                                                         49-13 for 10GBASE-R and 802.3ba-2010 Figure 82-13 for 40GBASE-R. */
	uint64_t an_nonce_match_dis           : 1;  /**< Auto-Negotiation nonce match disable. This bit should be clear for normal operation.
                                                         Setting it disables Nonce Match check by forcing nonce_match variable to 0 in the Auto-
                                                         Negotiation arbitration state diagram, as defined in 802.3-2008 Figure 73-11. This bit can
                                                         be set by software for test purposes, e.g. for running auto-negotiation in loopback mode. */
	uint64_t timestamp_norm_dis           : 1;  /**< 40GBASE-R RX timestamp normalization disable. This bit controls the generation of the
                                                         receive SOP timestamp passed to the SMU sub-block for a 40GBASE-R LMAC/LPCS. When this bit
                                                         is clear, SPU normalizes the receive SOP timestamp in order to compensate for lane-to-lane
                                                         skew on a 40GBASE-R link, as described below. When this bit is set, timestamp
                                                         normalization is disabled and SPU directly passes the captured SOP timestamp values to
                                                         SMU.
                                                         In 40GBASE-R mode, a packet's SOP block can be transferred on any of the LMAC's lanes. In
                                                         the presence of lane-to-lane skew, the SOP delay from transmit (by the link partner) to
                                                         receive by SPU varies depending on which lane is used by the SOP block. This variation
                                                         reduces the accuracy of the received SOP timestamp relative to when it was transmitted by
                                                         the link partner.
                                                         SPU captures the timestamp of the alignment marker received on each SerDes lane during
                                                         align/skew detection; the captured value can be read from the SerDes lane's
                                                         BGX(0..5)_SPU_SDS(0..3)_SKEW_STATUS[SKEW_STATUS] field (AM_TIMESTAMP sub-field). If
                                                         alignment markers are transmitted at about the same time on all lanes by the link partner,
                                                         then the difference between the AM_TIMESTAMP values for a pair of lanes represents the
                                                         approximate skew between those lanes.
                                                         SPU uses the 40GBASE-R LMAC's programmed PCS lane 0 as a reference and computes the
                                                         AM_TIMESTAMP delta of every other lane relative to PCS lane 0. When normalization is
                                                         enabled, SPU adjusts the timestamp of a received SOP by subtracting the receiving lane's
                                                         AM_TIMESTAMP delta from the captured timestamp value. The adjusted/normalized timestamp
                                                         value is then passed to SMU along with the SOP.
                                                         Software can determine the actual maximum skew of a 40GBASE-R link by examining the
                                                         AM_TIMESTAMP values in the BGX(0..5)_SPU_SDS(0..3)_SKEW_STATUS registers, and decide if
                                                         timestamp normalization should be enabled or disabled to improve PTP accuracy.
                                                         Normalization improves accuracy for larger skew values but reduces the accuracy (due to
                                                         timestamp measurement errors) for small skew values. */
	uint64_t rx_buf_flip_synd             : 8;  /**< Flip SPU RX_BUF FIFO ECC bits. Two bits per SerDes lane; used to inject single-bit and
                                                         double-bit errors into the ECC field on writes to the associated SPU RX_BUF lane FIFO, as
                                                         follows:
                                                         0x0 = Normal operation
                                                         0x1 = SBE on ECC bit 0
                                                         0x2 = SBE on ECC bit 1
                                                         0x3 = DBE on ECC bits 1:0 */
	uint64_t br_pmd_train_soft_en         : 1;  /**< Enable BASE-R PMD software controlled link training. This bit configures the operation
                                                         mode for BASE-R link training for all LMACs and lanes. When this bit is set along with
                                                         BR_PMD_CONTROL[TRAIN_EN] for a given LMAC, the BASE-R link training protocol for that LMAC
                                                         is executed under software control, whereby the contents the BR_PMD_LD_CUP and
                                                         BR_PMD_LD_REP registers are updated by software. When this bit is clear and
                                                         BR_PMD_CONTROL[TRAIN_EN] is set, the link training protocol is fully automated in
                                                         hardware, whereby the contents BR_PMD_LD_CUP and BR_PMD_LD_REP registers are automatically
                                                         updated by hardware. */
	uint64_t an_arb_link_chk_en           : 1;  /**< Enable link status checking by Auto-Negotiation arbitration state machine. When Auto-
                                                         Negotiation is enabled (BGX(0..5)_SPU(0..3)_AN_CONTROL[AN_EN] is set), this bit controls
                                                         the behavior of the Auto-Negotiation arbitration state machine when it reaches the AN GOOD
                                                         CHECK state after DME pages are successfully exchanged, as defined in Figure 73-11 in
                                                         802.3-2008.
                                                         When this bit is set and the negotiated highest common denominator (HCD) technology
                                                         matches BGX(0..5)_CMR(0..3)_CONFIG[LMAC_TYPE], the Auto-Negotiation arbitration SM
                                                         performs the actions defined for the AN GOOD CHECK state in Figure 73-11, i.e. run the
                                                         link_fail_inhibit timer and eventually transition to the AN GOOD or TRANSMIT DISABLE
                                                         state.
                                                         When this bit is clear or the HCD technology does not match LMAC_TYPE, the AN arbitration
                                                         SM stay in the AN GOOD CHECK state, with the expectation that software will perform the
                                                         appropriate actions to complete the Auto-Negotiation protocol, as follows:
                                                         If this bit is clear and the HCD technology matches LMAC_TYPE, clear AN_EN in AN_CONTROL.
                                                         Otherwise, disable the LPCS by clearing the BGX(0..5)_CMR(0..3)_CONFIG[ENABLE], clear
                                                         BGX(0..5)_SPU(0..3)_AN_CONTROL[AN_EN], reconfigure the LPCS with the correct LMAC_TYPE,
                                                         and re-enable the LPCS by setting BGX(0..5)_CMR(0..3)_CONFIG[ENABLE].
                                                         In both cases, software should implement the link_fail_inhibit timer and verify the link
                                                         status as specified for the AN GOOD CHECK state. */
	uint64_t rx_buf_cor_dis               : 1;  /**< When set, disables ECC correction on all SPU RX_BUF FIFOs. */
	uint64_t scramble_dis                 : 1;  /**< BASE-R Scrambler/descrambler disable. Setting this bit to 1 disables the BASE-R scrambler
                                                         & descrambler functions and FEC PN-2112 scrambler & descrambler functions for debug
                                                         purposes. */
	uint64_t reserved_15_15               : 1;
	uint64_t marker_rxp                   : 15; /**< BASE-R alignment marker receive period. For a multilane BASE-R logical PCS (i.e.
                                                         40GBASE-R), this field specifies the expected alignment marker receive period per lane,
                                                         i.e. the expected number of received 66b non-marker blocks between consecutive markers on
                                                         the same lane. The default value corresponds to a period of 16363 blocks (exclusive) as
                                                         specified in 802.3ba-2010. Must be greater than 64. */
#else
	uint64_t marker_rxp                   : 15;
	uint64_t reserved_15_15               : 1;
	uint64_t scramble_dis                 : 1;
	uint64_t rx_buf_cor_dis               : 1;
	uint64_t an_arb_link_chk_en           : 1;
	uint64_t br_pmd_train_soft_en         : 1;
	uint64_t rx_buf_flip_synd             : 8;
	uint64_t timestamp_norm_dis           : 1;
	uint64_t an_nonce_match_dis           : 1;
	uint64_t br_ber_mon_dis               : 1;
	uint64_t reserved_31_31               : 1;
	uint64_t us_clk_period                : 12;
	uint64_t ms_clk_period                : 12;
	uint64_t reserved_56_63               : 8;
#endif
	} s;
	struct cvmx_bgxx_spu_dbg_control_s    cn78xx;
};
typedef union cvmx_bgxx_spu_dbg_control cvmx_bgxx_spu_dbg_control_t;

/**
 * cvmx_bgx#_spu_mem_int
 */
union cvmx_bgxx_spu_mem_int {
	uint64_t u64;
	struct cvmx_bgxx_spu_mem_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t rx_buf_sbe                   : 4;  /**< SPU RX_BUF single-bit error for lanes 3-0. One bit per physical SerDes lane. Each bit is
                                                         set when the associated RX_BUF lane FIFO detects a single-bit ECC error. */
	uint64_t rx_buf_dbe                   : 4;  /**< SPU RX_BUF double-bit error for lanes 3-0. One bit per physical SerDes lane. Each bit is
                                                         set when the associated RX_BUF lane FIFO detects a double-bit ECC error. */
#else
	uint64_t rx_buf_dbe                   : 4;
	uint64_t rx_buf_sbe                   : 4;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_bgxx_spu_mem_int_s        cn78xx;
};
typedef union cvmx_bgxx_spu_mem_int cvmx_bgxx_spu_mem_int_t;

/**
 * cvmx_bgx#_spu_mem_status
 *
 * This register provides memory ECC status from the SPU RX_BUF lane FIFOs.
 *
 */
union cvmx_bgxx_spu_mem_status {
	uint64_t u64;
	struct cvmx_bgxx_spu_mem_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t rx_buf_ecc_synd              : 32; /**< SPU RX_BUF ECC syndromes for lanes 3-0. 8-bit syndrome sub-field per SerDes lane. Each
                                                         8-bit sub-field contains the syndrome of the latest single-bit or double-bit ECC error
                                                         detected by the associated RX_BUF lane FIFO, i.e. it is loaded when the corresponding
                                                         RX_BUF_SBE or RX_BUF_DBE bit is set in the SPU MEM_INT register. */
#else
	uint64_t rx_buf_ecc_synd              : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_bgxx_spu_mem_status_s     cn78xx;
};
typedef union cvmx_bgxx_spu_mem_status cvmx_bgxx_spu_mem_status_t;

/**
 * cvmx_bgx#_spu_sds#_skew_status
 *
 * This register provides SerDes lane skew status. One register per physical SerDes lane.
 *
 */
union cvmx_bgxx_spu_sdsx_skew_status {
	uint64_t u64;
	struct cvmx_bgxx_spu_sdsx_skew_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t skew_status                  : 32; /**< Format defined by BGX_SPU_SDS_SKEW_STATUS_S. */
#else
	uint64_t skew_status                  : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_bgxx_spu_sdsx_skew_status_s cn78xx;
};
typedef union cvmx_bgxx_spu_sdsx_skew_status cvmx_bgxx_spu_sdsx_skew_status_t;

/**
 * cvmx_bgx#_spu_sds#_states
 *
 * This register provides SerDes lane states. One register per physical SerDes lane.
 *
 */
union cvmx_bgxx_spu_sdsx_states {
	uint64_t u64;
	struct cvmx_bgxx_spu_sdsx_states_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_52_63               : 12;
	uint64_t am_lock_invld_cnt            : 2;  /**< 40GBASE-R alignment marker lock state machine invalid AM counter */
	uint64_t am_lock_sm                   : 2;  /**< 40GBASE-R alignment marker lock state machine state */
	uint64_t reserved_45_47               : 3;
	uint64_t train_sm                     : 3;  /**< Link training state machine state */
	uint64_t train_code_viol              : 1;  /**< Link training code violation in received control channel */
	uint64_t train_frame_lock             : 1;  /**< Link training frame lock status */
	uint64_t train_lock_found_1st_marker  : 1;  /**< Link training lock state machine found first marker flag */
	uint64_t train_lock_bad_markers       : 3;  /**< Link training lock state machine bad markers counter */
	uint64_t reserved_35_35               : 1;
	uint64_t an_arb_sm                    : 3;  /**< Auto-Negotiation arbitration state machine state */
	uint64_t an_rx_sm                     : 2;  /**< Auto-Negotiation receive state machine state */
	uint64_t reserved_29_29               : 1;
	uint64_t fec_block_sync               : 1;  /**< FEC block sync status */
	uint64_t fec_sync_cnt                 : 4;  /**< FEC block sync state machine good/bad parity block counter */
	uint64_t reserved_23_23               : 1;
	uint64_t br_sh_invld_cnt              : 7;  /**< BASE-R lock state machine invalid sync header counter */
	uint64_t br_block_lock                : 1;  /**< BASE-R block lock status */
	uint64_t br_sh_cnt                    : 11; /**< BASE-R lock state machine sync header counter */
	uint64_t bx_sync_sm                   : 4;  /**< BASE-X PCS synchronization state machine state */
#else
	uint64_t bx_sync_sm                   : 4;
	uint64_t br_sh_cnt                    : 11;
	uint64_t br_block_lock                : 1;
	uint64_t br_sh_invld_cnt              : 7;
	uint64_t reserved_23_23               : 1;
	uint64_t fec_sync_cnt                 : 4;
	uint64_t fec_block_sync               : 1;
	uint64_t reserved_29_29               : 1;
	uint64_t an_rx_sm                     : 2;
	uint64_t an_arb_sm                    : 3;
	uint64_t reserved_35_35               : 1;
	uint64_t train_lock_bad_markers       : 3;
	uint64_t train_lock_found_1st_marker  : 1;
	uint64_t train_frame_lock             : 1;
	uint64_t train_code_viol              : 1;
	uint64_t train_sm                     : 3;
	uint64_t reserved_45_47               : 3;
	uint64_t am_lock_sm                   : 2;
	uint64_t am_lock_invld_cnt            : 2;
	uint64_t reserved_52_63               : 12;
#endif
	} s;
	struct cvmx_bgxx_spu_sdsx_states_s    cn78xx;
};
typedef union cvmx_bgxx_spu_sdsx_states cvmx_bgxx_spu_sdsx_states_t;

#endif
