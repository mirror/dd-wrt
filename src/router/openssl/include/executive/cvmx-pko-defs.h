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
 * cvmx-pko-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon pko.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_PKO_DEFS_H__
#define __CVMX_PKO_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_CHANNEL_LEVEL CVMX_PKO_CHANNEL_LEVEL_FUNC()
static inline uint64_t CVMX_PKO_CHANNEL_LEVEL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_CHANNEL_LEVEL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400000800F0ull);
}
#else
#define CVMX_PKO_CHANNEL_LEVEL (CVMX_ADD_IO_SEG(0x00015400000800F0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_DPFI_ENA CVMX_PKO_DPFI_ENA_FUNC()
static inline uint64_t CVMX_PKO_DPFI_ENA_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_DPFI_ENA not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000C00018ull);
}
#else
#define CVMX_PKO_DPFI_ENA (CVMX_ADD_IO_SEG(0x0001540000C00018ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_DPFI_FLUSH CVMX_PKO_DPFI_FLUSH_FUNC()
static inline uint64_t CVMX_PKO_DPFI_FLUSH_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_DPFI_FLUSH not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000C00008ull);
}
#else
#define CVMX_PKO_DPFI_FLUSH (CVMX_ADD_IO_SEG(0x0001540000C00008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_DPFI_FPA_AURA CVMX_PKO_DPFI_FPA_AURA_FUNC()
static inline uint64_t CVMX_PKO_DPFI_FPA_AURA_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_DPFI_FPA_AURA not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000C00010ull);
}
#else
#define CVMX_PKO_DPFI_FPA_AURA (CVMX_ADD_IO_SEG(0x0001540000C00010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_DPFI_STATUS CVMX_PKO_DPFI_STATUS_FUNC()
static inline uint64_t CVMX_PKO_DPFI_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_DPFI_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000C00000ull);
}
#else
#define CVMX_PKO_DPFI_STATUS (CVMX_ADD_IO_SEG(0x0001540000C00000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_DQX_BYTES(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_DQX_BYTES(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00015400000000C8ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_DQX_BYTES(offset) (CVMX_ADD_IO_SEG(0x00015400000000C8ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_DQX_CIR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_DQX_CIR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000280018ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_DQX_CIR(offset) (CVMX_ADD_IO_SEG(0x0001540000280018ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_DQX_DROPPED_BYTES(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_DQX_DROPPED_BYTES(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00015400000000D8ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_DQX_DROPPED_BYTES(offset) (CVMX_ADD_IO_SEG(0x00015400000000D8ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_DQX_DROPPED_PACKETS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_DQX_DROPPED_PACKETS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00015400000000D0ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_DQX_DROPPED_PACKETS(offset) (CVMX_ADD_IO_SEG(0x00015400000000D0ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_DQX_FIFO(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_DQX_FIFO(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000300078ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_DQX_FIFO(offset) (CVMX_ADD_IO_SEG(0x0001540000300078ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_DQX_PACKETS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_DQX_PACKETS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00015400000000C0ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_DQX_PACKETS(offset) (CVMX_ADD_IO_SEG(0x00015400000000C0ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_DQX_PICK(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_DQX_PICK(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000300070ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_DQX_PICK(offset) (CVMX_ADD_IO_SEG(0x0001540000300070ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_DQX_PIR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_DQX_PIR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000280020ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_DQX_PIR(offset) (CVMX_ADD_IO_SEG(0x0001540000280020ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_DQX_POINTERS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_DQX_POINTERS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000280078ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_DQX_POINTERS(offset) (CVMX_ADD_IO_SEG(0x0001540000280078ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_DQX_SCHEDULE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_DQX_SCHEDULE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000280008ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_DQX_SCHEDULE(offset) (CVMX_ADD_IO_SEG(0x0001540000280008ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_DQX_SCHED_STATE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_DQX_SCHED_STATE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000280028ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_DQX_SCHED_STATE(offset) (CVMX_ADD_IO_SEG(0x0001540000280028ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_DQX_SHAPE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_DQX_SHAPE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000280010ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_DQX_SHAPE(offset) (CVMX_ADD_IO_SEG(0x0001540000280010ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_DQX_SHAPE_STATE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_DQX_SHAPE_STATE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000280030ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_DQX_SHAPE_STATE(offset) (CVMX_ADD_IO_SEG(0x0001540000280030ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_DQX_SW_XOFF(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_DQX_SW_XOFF(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00015400002800E0ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_DQX_SW_XOFF(offset) (CVMX_ADD_IO_SEG(0x00015400002800E0ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_DQX_TOPOLOGY(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_DQX_TOPOLOGY(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000300000ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_DQX_TOPOLOGY(offset) (CVMX_ADD_IO_SEG(0x0001540000300000ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_DQX_WM_CNT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_DQX_WM_CNT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000000050ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_DQX_WM_CNT(offset) (CVMX_ADD_IO_SEG(0x0001540000000050ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_DQX_WM_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_DQX_WM_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000000040ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_DQX_WM_CTL(offset) (CVMX_ADD_IO_SEG(0x0001540000000040ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_DQX_WM_CTL_W1C(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_DQX_WM_CTL_W1C(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000000048ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_DQX_WM_CTL_W1C(offset) (CVMX_ADD_IO_SEG(0x0001540000000048ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_DQ_CSR_BUS_DEBUG CVMX_PKO_DQ_CSR_BUS_DEBUG_FUNC()
static inline uint64_t CVMX_PKO_DQ_CSR_BUS_DEBUG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_DQ_CSR_BUS_DEBUG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400003001F8ull);
}
#else
#define CVMX_PKO_DQ_CSR_BUS_DEBUG (CVMX_ADD_IO_SEG(0x00015400003001F8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_DQ_DEBUG CVMX_PKO_DQ_DEBUG_FUNC()
static inline uint64_t CVMX_PKO_DQ_DEBUG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_DQ_DEBUG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000300128ull);
}
#else
#define CVMX_PKO_DQ_DEBUG (CVMX_ADD_IO_SEG(0x0001540000300128ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_DRAIN_IRQ CVMX_PKO_DRAIN_IRQ_FUNC()
static inline uint64_t CVMX_PKO_DRAIN_IRQ_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_DRAIN_IRQ not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000000140ull);
}
#else
#define CVMX_PKO_DRAIN_IRQ (CVMX_ADD_IO_SEG(0x0001540000000140ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_ENABLE CVMX_PKO_ENABLE_FUNC()
static inline uint64_t CVMX_PKO_ENABLE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_ENABLE not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000D00008ull);
}
#else
#define CVMX_PKO_ENABLE (CVMX_ADD_IO_SEG(0x0001540000D00008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_FORMATX_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 127)))))
		cvmx_warn("CVMX_PKO_FORMATX_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000900800ull) + ((offset) & 127) * 8;
}
#else
#define CVMX_PKO_FORMATX_CTL(offset) (CVMX_ADD_IO_SEG(0x0001540000900800ull) + ((offset) & 127) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_L1_SQA_DEBUG CVMX_PKO_L1_SQA_DEBUG_FUNC()
static inline uint64_t CVMX_PKO_L1_SQA_DEBUG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_L1_SQA_DEBUG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000080128ull);
}
#else
#define CVMX_PKO_L1_SQA_DEBUG (CVMX_ADD_IO_SEG(0x0001540000080128ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_L1_SQB_DEBUG CVMX_PKO_L1_SQB_DEBUG_FUNC()
static inline uint64_t CVMX_PKO_L1_SQB_DEBUG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_L1_SQB_DEBUG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000080130ull);
}
#else
#define CVMX_PKO_L1_SQB_DEBUG (CVMX_ADD_IO_SEG(0x0001540000080130ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L1_SQX_CIR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_PKO_L1_SQX_CIR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000000018ull) + ((offset) & 31) * 512;
}
#else
#define CVMX_PKO_L1_SQX_CIR(offset) (CVMX_ADD_IO_SEG(0x0001540000000018ull) + ((offset) & 31) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L1_SQX_DROPPED_BYTES(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_PKO_L1_SQX_DROPPED_BYTES(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00015400000000B8ull) + ((offset) & 31) * 512;
}
#else
#define CVMX_PKO_L1_SQX_DROPPED_BYTES(offset) (CVMX_ADD_IO_SEG(0x00015400000000B8ull) + ((offset) & 31) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L1_SQX_DROPPED_PACKETS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_PKO_L1_SQX_DROPPED_PACKETS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00015400000000B0ull) + ((offset) & 31) * 512;
}
#else
#define CVMX_PKO_L1_SQX_DROPPED_PACKETS(offset) (CVMX_ADD_IO_SEG(0x00015400000000B0ull) + ((offset) & 31) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L1_SQX_GREEN(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_PKO_L1_SQX_GREEN(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000080058ull) + ((offset) & 31) * 512;
}
#else
#define CVMX_PKO_L1_SQX_GREEN(offset) (CVMX_ADD_IO_SEG(0x0001540000080058ull) + ((offset) & 31) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L1_SQX_GREEN_BYTES(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_PKO_L1_SQX_GREEN_BYTES(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000000088ull) + ((offset) & 31) * 512;
}
#else
#define CVMX_PKO_L1_SQX_GREEN_BYTES(offset) (CVMX_ADD_IO_SEG(0x0001540000000088ull) + ((offset) & 31) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L1_SQX_GREEN_PACKETS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_PKO_L1_SQX_GREEN_PACKETS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000000080ull) + ((offset) & 31) * 512;
}
#else
#define CVMX_PKO_L1_SQX_GREEN_PACKETS(offset) (CVMX_ADD_IO_SEG(0x0001540000000080ull) + ((offset) & 31) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L1_SQX_LINK(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_PKO_L1_SQX_LINK(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000000038ull) + ((offset) & 31) * 512;
}
#else
#define CVMX_PKO_L1_SQX_LINK(offset) (CVMX_ADD_IO_SEG(0x0001540000000038ull) + ((offset) & 31) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L1_SQX_PICK(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_PKO_L1_SQX_PICK(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000080070ull) + ((offset) & 31) * 512;
}
#else
#define CVMX_PKO_L1_SQX_PICK(offset) (CVMX_ADD_IO_SEG(0x0001540000080070ull) + ((offset) & 31) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L1_SQX_PIR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_PKO_L1_SQX_PIR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000000020ull) + ((offset) & 31) * 512;
}
#else
#define CVMX_PKO_L1_SQX_PIR(offset) (CVMX_ADD_IO_SEG(0x0001540000000020ull) + ((offset) & 31) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L1_SQX_RED(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_PKO_L1_SQX_RED(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000080068ull) + ((offset) & 31) * 512;
}
#else
#define CVMX_PKO_L1_SQX_RED(offset) (CVMX_ADD_IO_SEG(0x0001540000080068ull) + ((offset) & 31) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L1_SQX_RED_BYTES(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_PKO_L1_SQX_RED_BYTES(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00015400000000A8ull) + ((offset) & 31) * 512;
}
#else
#define CVMX_PKO_L1_SQX_RED_BYTES(offset) (CVMX_ADD_IO_SEG(0x00015400000000A8ull) + ((offset) & 31) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L1_SQX_RED_PACKETS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_PKO_L1_SQX_RED_PACKETS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00015400000000A0ull) + ((offset) & 31) * 512;
}
#else
#define CVMX_PKO_L1_SQX_RED_PACKETS(offset) (CVMX_ADD_IO_SEG(0x00015400000000A0ull) + ((offset) & 31) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L1_SQX_SHAPE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_PKO_L1_SQX_SHAPE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000000010ull) + ((offset) & 31) * 512;
}
#else
#define CVMX_PKO_L1_SQX_SHAPE(offset) (CVMX_ADD_IO_SEG(0x0001540000000010ull) + ((offset) & 31) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L1_SQX_SHAPE_STATE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_PKO_L1_SQX_SHAPE_STATE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000000030ull) + ((offset) & 31) * 512;
}
#else
#define CVMX_PKO_L1_SQX_SHAPE_STATE(offset) (CVMX_ADD_IO_SEG(0x0001540000000030ull) + ((offset) & 31) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L1_SQX_SW_XOFF(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_PKO_L1_SQX_SW_XOFF(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00015400000000E0ull) + ((offset) & 31) * 512;
}
#else
#define CVMX_PKO_L1_SQX_SW_XOFF(offset) (CVMX_ADD_IO_SEG(0x00015400000000E0ull) + ((offset) & 31) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L1_SQX_TOPOLOGY(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_PKO_L1_SQX_TOPOLOGY(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000080000ull) + ((offset) & 31) * 512;
}
#else
#define CVMX_PKO_L1_SQX_TOPOLOGY(offset) (CVMX_ADD_IO_SEG(0x0001540000080000ull) + ((offset) & 31) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L1_SQX_YELLOW(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_PKO_L1_SQX_YELLOW(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000080060ull) + ((offset) & 31) * 512;
}
#else
#define CVMX_PKO_L1_SQX_YELLOW(offset) (CVMX_ADD_IO_SEG(0x0001540000080060ull) + ((offset) & 31) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L1_SQX_YELLOW_BYTES(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_PKO_L1_SQX_YELLOW_BYTES(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000000098ull) + ((offset) & 31) * 512;
}
#else
#define CVMX_PKO_L1_SQX_YELLOW_BYTES(offset) (CVMX_ADD_IO_SEG(0x0001540000000098ull) + ((offset) & 31) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L1_SQX_YELLOW_PACKETS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_PKO_L1_SQX_YELLOW_PACKETS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000000090ull) + ((offset) & 31) * 512;
}
#else
#define CVMX_PKO_L1_SQX_YELLOW_PACKETS(offset) (CVMX_ADD_IO_SEG(0x0001540000000090ull) + ((offset) & 31) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_L1_SQ_CSR_BUS_DEBUG CVMX_PKO_L1_SQ_CSR_BUS_DEBUG_FUNC()
static inline uint64_t CVMX_PKO_L1_SQ_CSR_BUS_DEBUG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_L1_SQ_CSR_BUS_DEBUG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400000801F8ull);
}
#else
#define CVMX_PKO_L1_SQ_CSR_BUS_DEBUG (CVMX_ADD_IO_SEG(0x00015400000801F8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_L2_SQA_DEBUG CVMX_PKO_L2_SQA_DEBUG_FUNC()
static inline uint64_t CVMX_PKO_L2_SQA_DEBUG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_L2_SQA_DEBUG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000100128ull);
}
#else
#define CVMX_PKO_L2_SQA_DEBUG (CVMX_ADD_IO_SEG(0x0001540000100128ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_L2_SQB_DEBUG CVMX_PKO_L2_SQB_DEBUG_FUNC()
static inline uint64_t CVMX_PKO_L2_SQB_DEBUG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_L2_SQB_DEBUG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000100130ull);
}
#else
#define CVMX_PKO_L2_SQB_DEBUG (CVMX_ADD_IO_SEG(0x0001540000100130ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L2_SQX_CIR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L2_SQX_CIR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000080018ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L2_SQX_CIR(offset) (CVMX_ADD_IO_SEG(0x0001540000080018ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L2_SQX_GREEN(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L2_SQX_GREEN(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000100058ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L2_SQX_GREEN(offset) (CVMX_ADD_IO_SEG(0x0001540000100058ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L2_SQX_PICK(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L2_SQX_PICK(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000100070ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L2_SQX_PICK(offset) (CVMX_ADD_IO_SEG(0x0001540000100070ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L2_SQX_PIR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L2_SQX_PIR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000080020ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L2_SQX_PIR(offset) (CVMX_ADD_IO_SEG(0x0001540000080020ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L2_SQX_POINTERS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L2_SQX_POINTERS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000080078ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L2_SQX_POINTERS(offset) (CVMX_ADD_IO_SEG(0x0001540000080078ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L2_SQX_RED(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L2_SQX_RED(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000100068ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L2_SQX_RED(offset) (CVMX_ADD_IO_SEG(0x0001540000100068ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L2_SQX_SCHEDULE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L2_SQX_SCHEDULE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000080008ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L2_SQX_SCHEDULE(offset) (CVMX_ADD_IO_SEG(0x0001540000080008ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L2_SQX_SCHED_STATE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L2_SQX_SCHED_STATE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000080028ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L2_SQX_SCHED_STATE(offset) (CVMX_ADD_IO_SEG(0x0001540000080028ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L2_SQX_SHAPE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L2_SQX_SHAPE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000080010ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L2_SQX_SHAPE(offset) (CVMX_ADD_IO_SEG(0x0001540000080010ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L2_SQX_SHAPE_STATE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L2_SQX_SHAPE_STATE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000080030ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L2_SQX_SHAPE_STATE(offset) (CVMX_ADD_IO_SEG(0x0001540000080030ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L2_SQX_SW_XOFF(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L2_SQX_SW_XOFF(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00015400000800E0ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L2_SQX_SW_XOFF(offset) (CVMX_ADD_IO_SEG(0x00015400000800E0ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L2_SQX_TOPOLOGY(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L2_SQX_TOPOLOGY(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000100000ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L2_SQX_TOPOLOGY(offset) (CVMX_ADD_IO_SEG(0x0001540000100000ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L2_SQX_YELLOW(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L2_SQX_YELLOW(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000100060ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L2_SQX_YELLOW(offset) (CVMX_ADD_IO_SEG(0x0001540000100060ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_L2_SQ_CSR_BUS_DEBUG CVMX_PKO_L2_SQ_CSR_BUS_DEBUG_FUNC()
static inline uint64_t CVMX_PKO_L2_SQ_CSR_BUS_DEBUG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_L2_SQ_CSR_BUS_DEBUG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400001001F8ull);
}
#else
#define CVMX_PKO_L2_SQ_CSR_BUS_DEBUG (CVMX_ADD_IO_SEG(0x00015400001001F8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L3_L2_SQX_CHANNEL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L3_L2_SQX_CHANNEL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000080038ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L3_L2_SQX_CHANNEL(offset) (CVMX_ADD_IO_SEG(0x0001540000080038ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_L3_SQA_DEBUG CVMX_PKO_L3_SQA_DEBUG_FUNC()
static inline uint64_t CVMX_PKO_L3_SQA_DEBUG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_L3_SQA_DEBUG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000180128ull);
}
#else
#define CVMX_PKO_L3_SQA_DEBUG (CVMX_ADD_IO_SEG(0x0001540000180128ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_L3_SQB_DEBUG CVMX_PKO_L3_SQB_DEBUG_FUNC()
static inline uint64_t CVMX_PKO_L3_SQB_DEBUG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_L3_SQB_DEBUG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000180130ull);
}
#else
#define CVMX_PKO_L3_SQB_DEBUG (CVMX_ADD_IO_SEG(0x0001540000180130ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L3_SQX_CIR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L3_SQX_CIR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000100018ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L3_SQX_CIR(offset) (CVMX_ADD_IO_SEG(0x0001540000100018ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L3_SQX_GREEN(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L3_SQX_GREEN(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000180058ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L3_SQX_GREEN(offset) (CVMX_ADD_IO_SEG(0x0001540000180058ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L3_SQX_PICK(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L3_SQX_PICK(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000180070ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L3_SQX_PICK(offset) (CVMX_ADD_IO_SEG(0x0001540000180070ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L3_SQX_PIR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L3_SQX_PIR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000100020ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L3_SQX_PIR(offset) (CVMX_ADD_IO_SEG(0x0001540000100020ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L3_SQX_POINTERS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L3_SQX_POINTERS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000100078ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L3_SQX_POINTERS(offset) (CVMX_ADD_IO_SEG(0x0001540000100078ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L3_SQX_RED(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L3_SQX_RED(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000180068ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L3_SQX_RED(offset) (CVMX_ADD_IO_SEG(0x0001540000180068ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L3_SQX_SCHEDULE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L3_SQX_SCHEDULE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000100008ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L3_SQX_SCHEDULE(offset) (CVMX_ADD_IO_SEG(0x0001540000100008ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L3_SQX_SCHED_STATE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L3_SQX_SCHED_STATE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000100028ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L3_SQX_SCHED_STATE(offset) (CVMX_ADD_IO_SEG(0x0001540000100028ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L3_SQX_SHAPE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L3_SQX_SHAPE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000100010ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L3_SQX_SHAPE(offset) (CVMX_ADD_IO_SEG(0x0001540000100010ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L3_SQX_SHAPE_STATE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L3_SQX_SHAPE_STATE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000100030ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L3_SQX_SHAPE_STATE(offset) (CVMX_ADD_IO_SEG(0x0001540000100030ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L3_SQX_SW_XOFF(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L3_SQX_SW_XOFF(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00015400001000E0ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L3_SQX_SW_XOFF(offset) (CVMX_ADD_IO_SEG(0x00015400001000E0ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L3_SQX_TOPOLOGY(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L3_SQX_TOPOLOGY(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000180000ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L3_SQX_TOPOLOGY(offset) (CVMX_ADD_IO_SEG(0x0001540000180000ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L3_SQX_YELLOW(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L3_SQX_YELLOW(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000180060ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L3_SQX_YELLOW(offset) (CVMX_ADD_IO_SEG(0x0001540000180060ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_L3_SQ_CSR_BUS_DEBUG CVMX_PKO_L3_SQ_CSR_BUS_DEBUG_FUNC()
static inline uint64_t CVMX_PKO_L3_SQ_CSR_BUS_DEBUG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_L3_SQ_CSR_BUS_DEBUG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400001801F8ull);
}
#else
#define CVMX_PKO_L3_SQ_CSR_BUS_DEBUG (CVMX_ADD_IO_SEG(0x00015400001801F8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_L4_SQA_DEBUG CVMX_PKO_L4_SQA_DEBUG_FUNC()
static inline uint64_t CVMX_PKO_L4_SQA_DEBUG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_L4_SQA_DEBUG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000200128ull);
}
#else
#define CVMX_PKO_L4_SQA_DEBUG (CVMX_ADD_IO_SEG(0x0001540000200128ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_L4_SQB_DEBUG CVMX_PKO_L4_SQB_DEBUG_FUNC()
static inline uint64_t CVMX_PKO_L4_SQB_DEBUG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_L4_SQB_DEBUG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000200130ull);
}
#else
#define CVMX_PKO_L4_SQB_DEBUG (CVMX_ADD_IO_SEG(0x0001540000200130ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L4_SQX_CIR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_L4_SQX_CIR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000180018ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_L4_SQX_CIR(offset) (CVMX_ADD_IO_SEG(0x0001540000180018ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L4_SQX_GREEN(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_L4_SQX_GREEN(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000200058ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_L4_SQX_GREEN(offset) (CVMX_ADD_IO_SEG(0x0001540000200058ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L4_SQX_PICK(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_L4_SQX_PICK(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000200070ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_L4_SQX_PICK(offset) (CVMX_ADD_IO_SEG(0x0001540000200070ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L4_SQX_PIR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_L4_SQX_PIR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000180020ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_L4_SQX_PIR(offset) (CVMX_ADD_IO_SEG(0x0001540000180020ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L4_SQX_POINTERS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_L4_SQX_POINTERS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000180078ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_L4_SQX_POINTERS(offset) (CVMX_ADD_IO_SEG(0x0001540000180078ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L4_SQX_RED(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_L4_SQX_RED(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000200068ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_L4_SQX_RED(offset) (CVMX_ADD_IO_SEG(0x0001540000200068ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L4_SQX_SCHEDULE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_L4_SQX_SCHEDULE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000180008ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_L4_SQX_SCHEDULE(offset) (CVMX_ADD_IO_SEG(0x0001540000180008ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L4_SQX_SCHED_STATE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L4_SQX_SCHED_STATE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000180028ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L4_SQX_SCHED_STATE(offset) (CVMX_ADD_IO_SEG(0x0001540000180028ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L4_SQX_SHAPE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_L4_SQX_SHAPE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000180010ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_L4_SQX_SHAPE(offset) (CVMX_ADD_IO_SEG(0x0001540000180010ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L4_SQX_SHAPE_STATE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_PKO_L4_SQX_SHAPE_STATE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000180030ull) + ((offset) & 511) * 512;
}
#else
#define CVMX_PKO_L4_SQX_SHAPE_STATE(offset) (CVMX_ADD_IO_SEG(0x0001540000180030ull) + ((offset) & 511) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L4_SQX_SW_XOFF(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_L4_SQX_SW_XOFF(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00015400001800E0ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_L4_SQX_SW_XOFF(offset) (CVMX_ADD_IO_SEG(0x00015400001800E0ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L4_SQX_TOPOLOGY(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_L4_SQX_TOPOLOGY(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000200000ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_L4_SQX_TOPOLOGY(offset) (CVMX_ADD_IO_SEG(0x0001540000200000ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L4_SQX_YELLOW(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_L4_SQX_YELLOW(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000200060ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_L4_SQX_YELLOW(offset) (CVMX_ADD_IO_SEG(0x0001540000200060ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_L4_SQ_CSR_BUS_DEBUG CVMX_PKO_L4_SQ_CSR_BUS_DEBUG_FUNC()
static inline uint64_t CVMX_PKO_L4_SQ_CSR_BUS_DEBUG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_L4_SQ_CSR_BUS_DEBUG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400002001F8ull);
}
#else
#define CVMX_PKO_L4_SQ_CSR_BUS_DEBUG (CVMX_ADD_IO_SEG(0x00015400002001F8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_L5_SQA_DEBUG CVMX_PKO_L5_SQA_DEBUG_FUNC()
static inline uint64_t CVMX_PKO_L5_SQA_DEBUG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_L5_SQA_DEBUG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000280128ull);
}
#else
#define CVMX_PKO_L5_SQA_DEBUG (CVMX_ADD_IO_SEG(0x0001540000280128ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_L5_SQB_DEBUG CVMX_PKO_L5_SQB_DEBUG_FUNC()
static inline uint64_t CVMX_PKO_L5_SQB_DEBUG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_L5_SQB_DEBUG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000280130ull);
}
#else
#define CVMX_PKO_L5_SQB_DEBUG (CVMX_ADD_IO_SEG(0x0001540000280130ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L5_SQX_CIR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_L5_SQX_CIR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000200018ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_L5_SQX_CIR(offset) (CVMX_ADD_IO_SEG(0x0001540000200018ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L5_SQX_GREEN(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_L5_SQX_GREEN(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000280058ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_L5_SQX_GREEN(offset) (CVMX_ADD_IO_SEG(0x0001540000280058ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L5_SQX_PICK(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_L5_SQX_PICK(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000280070ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_L5_SQX_PICK(offset) (CVMX_ADD_IO_SEG(0x0001540000280070ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L5_SQX_PIR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_L5_SQX_PIR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000200020ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_L5_SQX_PIR(offset) (CVMX_ADD_IO_SEG(0x0001540000200020ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L5_SQX_POINTERS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_L5_SQX_POINTERS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000200078ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_L5_SQX_POINTERS(offset) (CVMX_ADD_IO_SEG(0x0001540000200078ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L5_SQX_RED(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_L5_SQX_RED(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000280068ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_L5_SQX_RED(offset) (CVMX_ADD_IO_SEG(0x0001540000280068ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L5_SQX_SCHEDULE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_L5_SQX_SCHEDULE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000200008ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_L5_SQX_SCHEDULE(offset) (CVMX_ADD_IO_SEG(0x0001540000200008ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L5_SQX_SCHED_STATE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_L5_SQX_SCHED_STATE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000200028ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_L5_SQX_SCHED_STATE(offset) (CVMX_ADD_IO_SEG(0x0001540000200028ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L5_SQX_SHAPE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_L5_SQX_SHAPE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000200010ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_L5_SQX_SHAPE(offset) (CVMX_ADD_IO_SEG(0x0001540000200010ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L5_SQX_SHAPE_STATE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_L5_SQX_SHAPE_STATE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000200030ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_L5_SQX_SHAPE_STATE(offset) (CVMX_ADD_IO_SEG(0x0001540000200030ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L5_SQX_SW_XOFF(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_L5_SQX_SW_XOFF(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00015400002000E0ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_L5_SQX_SW_XOFF(offset) (CVMX_ADD_IO_SEG(0x00015400002000E0ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L5_SQX_TOPOLOGY(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_L5_SQX_TOPOLOGY(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000280000ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_L5_SQX_TOPOLOGY(offset) (CVMX_ADD_IO_SEG(0x0001540000280000ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_L5_SQX_YELLOW(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_L5_SQX_YELLOW(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000280060ull) + ((offset) & 1023) * 512;
}
#else
#define CVMX_PKO_L5_SQX_YELLOW(offset) (CVMX_ADD_IO_SEG(0x0001540000280060ull) + ((offset) & 1023) * 512)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_L5_SQ_CSR_BUS_DEBUG CVMX_PKO_L5_SQ_CSR_BUS_DEBUG_FUNC()
static inline uint64_t CVMX_PKO_L5_SQ_CSR_BUS_DEBUG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_L5_SQ_CSR_BUS_DEBUG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400002801F8ull);
}
#else
#define CVMX_PKO_L5_SQ_CSR_BUS_DEBUG (CVMX_ADD_IO_SEG(0x00015400002801F8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_LUTX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_LUTX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000B00000ull) + ((offset) & 1023) * 8;
}
#else
#define CVMX_PKO_LUTX(offset) (CVMX_ADD_IO_SEG(0x0001540000B00000ull) + ((offset) & 1023) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_LUT_BIST_STATUS CVMX_PKO_LUT_BIST_STATUS_FUNC()
static inline uint64_t CVMX_PKO_LUT_BIST_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_LUT_BIST_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000B02018ull);
}
#else
#define CVMX_PKO_LUT_BIST_STATUS (CVMX_ADD_IO_SEG(0x0001540000B02018ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_LUT_ECC_CTL0 CVMX_PKO_LUT_ECC_CTL0_FUNC()
static inline uint64_t CVMX_PKO_LUT_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_LUT_ECC_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000BFFFD0ull);
}
#else
#define CVMX_PKO_LUT_ECC_CTL0 (CVMX_ADD_IO_SEG(0x0001540000BFFFD0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_LUT_ECC_DBE_STS0 CVMX_PKO_LUT_ECC_DBE_STS0_FUNC()
static inline uint64_t CVMX_PKO_LUT_ECC_DBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_LUT_ECC_DBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000BFFFF0ull);
}
#else
#define CVMX_PKO_LUT_ECC_DBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000BFFFF0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_LUT_ECC_DBE_STS_CMB0 CVMX_PKO_LUT_ECC_DBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PKO_LUT_ECC_DBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_LUT_ECC_DBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000BFFFD8ull);
}
#else
#define CVMX_PKO_LUT_ECC_DBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000BFFFD8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_LUT_ECC_SBE_STS0 CVMX_PKO_LUT_ECC_SBE_STS0_FUNC()
static inline uint64_t CVMX_PKO_LUT_ECC_SBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_LUT_ECC_SBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000BFFFF8ull);
}
#else
#define CVMX_PKO_LUT_ECC_SBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000BFFFF8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_LUT_ECC_SBE_STS_CMB0 CVMX_PKO_LUT_ECC_SBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PKO_LUT_ECC_SBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_LUT_ECC_SBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000BFFFE8ull);
}
#else
#define CVMX_PKO_LUT_ECC_SBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000BFFFE8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_MACX_CFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 27)))))
		cvmx_warn("CVMX_PKO_MACX_CFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000900000ull) + ((offset) & 31) * 8;
}
#else
#define CVMX_PKO_MACX_CFG(offset) (CVMX_ADD_IO_SEG(0x0001540000900000ull) + ((offset) & 31) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_MCI0_CRED_CNTX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 27)))))
		cvmx_warn("CVMX_PKO_MCI0_CRED_CNTX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000A40000ull) + ((offset) & 31) * 8;
}
#else
#define CVMX_PKO_MCI0_CRED_CNTX(offset) (CVMX_ADD_IO_SEG(0x0001540000A40000ull) + ((offset) & 31) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_MCI0_MAX_CREDX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 27)))))
		cvmx_warn("CVMX_PKO_MCI0_MAX_CREDX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000A00000ull) + ((offset) & 31) * 8;
}
#else
#define CVMX_PKO_MCI0_MAX_CREDX(offset) (CVMX_ADD_IO_SEG(0x0001540000A00000ull) + ((offset) & 31) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_MCI1_CRED_CNTX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 27)))))
		cvmx_warn("CVMX_PKO_MCI1_CRED_CNTX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000AB0000ull) + ((offset) & 31) * 8;
}
#else
#define CVMX_PKO_MCI1_CRED_CNTX(offset) (CVMX_ADD_IO_SEG(0x0001540000AB0000ull) + ((offset) & 31) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_MCI1_MAX_CREDX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 27)))))
		cvmx_warn("CVMX_PKO_MCI1_MAX_CREDX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000A80000ull) + ((offset) & 31) * 8;
}
#else
#define CVMX_PKO_MCI1_MAX_CREDX(offset) (CVMX_ADD_IO_SEG(0x0001540000A80000ull) + ((offset) & 31) * 8)
#endif
#define CVMX_PKO_MEM_COUNT0 (CVMX_ADD_IO_SEG(0x0001180050001080ull))
#define CVMX_PKO_MEM_COUNT1 (CVMX_ADD_IO_SEG(0x0001180050001088ull))
#define CVMX_PKO_MEM_DEBUG0 (CVMX_ADD_IO_SEG(0x0001180050001100ull))
#define CVMX_PKO_MEM_DEBUG1 (CVMX_ADD_IO_SEG(0x0001180050001108ull))
#define CVMX_PKO_MEM_DEBUG10 (CVMX_ADD_IO_SEG(0x0001180050001150ull))
#define CVMX_PKO_MEM_DEBUG11 (CVMX_ADD_IO_SEG(0x0001180050001158ull))
#define CVMX_PKO_MEM_DEBUG12 (CVMX_ADD_IO_SEG(0x0001180050001160ull))
#define CVMX_PKO_MEM_DEBUG13 (CVMX_ADD_IO_SEG(0x0001180050001168ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_MEM_DEBUG14 CVMX_PKO_MEM_DEBUG14_FUNC()
static inline uint64_t CVMX_PKO_MEM_DEBUG14_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_PKO_MEM_DEBUG14 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050001170ull);
}
#else
#define CVMX_PKO_MEM_DEBUG14 (CVMX_ADD_IO_SEG(0x0001180050001170ull))
#endif
#define CVMX_PKO_MEM_DEBUG2 (CVMX_ADD_IO_SEG(0x0001180050001110ull))
#define CVMX_PKO_MEM_DEBUG3 (CVMX_ADD_IO_SEG(0x0001180050001118ull))
#define CVMX_PKO_MEM_DEBUG4 (CVMX_ADD_IO_SEG(0x0001180050001120ull))
#define CVMX_PKO_MEM_DEBUG5 (CVMX_ADD_IO_SEG(0x0001180050001128ull))
#define CVMX_PKO_MEM_DEBUG6 (CVMX_ADD_IO_SEG(0x0001180050001130ull))
#define CVMX_PKO_MEM_DEBUG7 (CVMX_ADD_IO_SEG(0x0001180050001138ull))
#define CVMX_PKO_MEM_DEBUG8 (CVMX_ADD_IO_SEG(0x0001180050001140ull))
#define CVMX_PKO_MEM_DEBUG9 (CVMX_ADD_IO_SEG(0x0001180050001148ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_MEM_IPORT_PTRS CVMX_PKO_MEM_IPORT_PTRS_FUNC()
static inline uint64_t CVMX_PKO_MEM_IPORT_PTRS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_PKO_MEM_IPORT_PTRS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050001030ull);
}
#else
#define CVMX_PKO_MEM_IPORT_PTRS (CVMX_ADD_IO_SEG(0x0001180050001030ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_MEM_IPORT_QOS CVMX_PKO_MEM_IPORT_QOS_FUNC()
static inline uint64_t CVMX_PKO_MEM_IPORT_QOS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_PKO_MEM_IPORT_QOS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050001038ull);
}
#else
#define CVMX_PKO_MEM_IPORT_QOS (CVMX_ADD_IO_SEG(0x0001180050001038ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_MEM_IQUEUE_PTRS CVMX_PKO_MEM_IQUEUE_PTRS_FUNC()
static inline uint64_t CVMX_PKO_MEM_IQUEUE_PTRS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_PKO_MEM_IQUEUE_PTRS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050001040ull);
}
#else
#define CVMX_PKO_MEM_IQUEUE_PTRS (CVMX_ADD_IO_SEG(0x0001180050001040ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_MEM_IQUEUE_QOS CVMX_PKO_MEM_IQUEUE_QOS_FUNC()
static inline uint64_t CVMX_PKO_MEM_IQUEUE_QOS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_PKO_MEM_IQUEUE_QOS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050001048ull);
}
#else
#define CVMX_PKO_MEM_IQUEUE_QOS (CVMX_ADD_IO_SEG(0x0001180050001048ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_MEM_PORT_PTRS CVMX_PKO_MEM_PORT_PTRS_FUNC()
static inline uint64_t CVMX_PKO_MEM_PORT_PTRS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_PKO_MEM_PORT_PTRS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050001010ull);
}
#else
#define CVMX_PKO_MEM_PORT_PTRS (CVMX_ADD_IO_SEG(0x0001180050001010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_MEM_PORT_QOS CVMX_PKO_MEM_PORT_QOS_FUNC()
static inline uint64_t CVMX_PKO_MEM_PORT_QOS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_PKO_MEM_PORT_QOS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050001018ull);
}
#else
#define CVMX_PKO_MEM_PORT_QOS (CVMX_ADD_IO_SEG(0x0001180050001018ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_MEM_PORT_RATE0 CVMX_PKO_MEM_PORT_RATE0_FUNC()
static inline uint64_t CVMX_PKO_MEM_PORT_RATE0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_PKO_MEM_PORT_RATE0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050001020ull);
}
#else
#define CVMX_PKO_MEM_PORT_RATE0 (CVMX_ADD_IO_SEG(0x0001180050001020ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_MEM_PORT_RATE1 CVMX_PKO_MEM_PORT_RATE1_FUNC()
static inline uint64_t CVMX_PKO_MEM_PORT_RATE1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_PKO_MEM_PORT_RATE1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050001028ull);
}
#else
#define CVMX_PKO_MEM_PORT_RATE1 (CVMX_ADD_IO_SEG(0x0001180050001028ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_MEM_QUEUE_PTRS CVMX_PKO_MEM_QUEUE_PTRS_FUNC()
static inline uint64_t CVMX_PKO_MEM_QUEUE_PTRS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_PKO_MEM_QUEUE_PTRS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050001000ull);
}
#else
#define CVMX_PKO_MEM_QUEUE_PTRS (CVMX_ADD_IO_SEG(0x0001180050001000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_MEM_QUEUE_QOS CVMX_PKO_MEM_QUEUE_QOS_FUNC()
static inline uint64_t CVMX_PKO_MEM_QUEUE_QOS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_PKO_MEM_QUEUE_QOS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050001008ull);
}
#else
#define CVMX_PKO_MEM_QUEUE_QOS (CVMX_ADD_IO_SEG(0x0001180050001008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_MEM_THROTTLE_INT CVMX_PKO_MEM_THROTTLE_INT_FUNC()
static inline uint64_t CVMX_PKO_MEM_THROTTLE_INT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_PKO_MEM_THROTTLE_INT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050001058ull);
}
#else
#define CVMX_PKO_MEM_THROTTLE_INT (CVMX_ADD_IO_SEG(0x0001180050001058ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_MEM_THROTTLE_PIPE CVMX_PKO_MEM_THROTTLE_PIPE_FUNC()
static inline uint64_t CVMX_PKO_MEM_THROTTLE_PIPE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_PKO_MEM_THROTTLE_PIPE not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050001050ull);
}
#else
#define CVMX_PKO_MEM_THROTTLE_PIPE (CVMX_ADD_IO_SEG(0x0001180050001050ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_NCB_BIST_STATUS CVMX_PKO_NCB_BIST_STATUS_FUNC()
static inline uint64_t CVMX_PKO_NCB_BIST_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_NCB_BIST_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000EFFF00ull);
}
#else
#define CVMX_PKO_NCB_BIST_STATUS (CVMX_ADD_IO_SEG(0x0001540000EFFF00ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_NCB_ECC_CTL0 CVMX_PKO_NCB_ECC_CTL0_FUNC()
static inline uint64_t CVMX_PKO_NCB_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_NCB_ECC_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000EFFFD0ull);
}
#else
#define CVMX_PKO_NCB_ECC_CTL0 (CVMX_ADD_IO_SEG(0x0001540000EFFFD0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_NCB_ECC_DBE_STS0 CVMX_PKO_NCB_ECC_DBE_STS0_FUNC()
static inline uint64_t CVMX_PKO_NCB_ECC_DBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_NCB_ECC_DBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000EFFFF0ull);
}
#else
#define CVMX_PKO_NCB_ECC_DBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000EFFFF0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_NCB_ECC_DBE_STS_CMB0 CVMX_PKO_NCB_ECC_DBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PKO_NCB_ECC_DBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_NCB_ECC_DBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000EFFFD8ull);
}
#else
#define CVMX_PKO_NCB_ECC_DBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000EFFFD8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_NCB_ECC_SBE_STS0 CVMX_PKO_NCB_ECC_SBE_STS0_FUNC()
static inline uint64_t CVMX_PKO_NCB_ECC_SBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_NCB_ECC_SBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000EFFFF8ull);
}
#else
#define CVMX_PKO_NCB_ECC_SBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000EFFFF8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_NCB_ECC_SBE_STS_CMB0 CVMX_PKO_NCB_ECC_SBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PKO_NCB_ECC_SBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_NCB_ECC_SBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000EFFFE8ull);
}
#else
#define CVMX_PKO_NCB_ECC_SBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000EFFFE8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_NCB_INT CVMX_PKO_NCB_INT_FUNC()
static inline uint64_t CVMX_PKO_NCB_INT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_NCB_INT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000E00010ull);
}
#else
#define CVMX_PKO_NCB_INT (CVMX_ADD_IO_SEG(0x0001540000E00010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_NCB_TX_ERR_INFO CVMX_PKO_NCB_TX_ERR_INFO_FUNC()
static inline uint64_t CVMX_PKO_NCB_TX_ERR_INFO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_NCB_TX_ERR_INFO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000E00008ull);
}
#else
#define CVMX_PKO_NCB_TX_ERR_INFO (CVMX_ADD_IO_SEG(0x0001540000E00008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_NCB_TX_ERR_WORD CVMX_PKO_NCB_TX_ERR_WORD_FUNC()
static inline uint64_t CVMX_PKO_NCB_TX_ERR_WORD_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_NCB_TX_ERR_WORD not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000E00000ull);
}
#else
#define CVMX_PKO_NCB_TX_ERR_WORD (CVMX_ADD_IO_SEG(0x0001540000E00000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_BIST_STATUS CVMX_PKO_PDM_BIST_STATUS_FUNC()
static inline uint64_t CVMX_PKO_PDM_BIST_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_BIST_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400008FFF00ull);
}
#else
#define CVMX_PKO_PDM_BIST_STATUS (CVMX_ADD_IO_SEG(0x00015400008FFF00ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_CFG CVMX_PKO_PDM_CFG_FUNC()
static inline uint64_t CVMX_PKO_PDM_CFG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_CFG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000800000ull);
}
#else
#define CVMX_PKO_PDM_CFG (CVMX_ADD_IO_SEG(0x0001540000800000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_CFG_DBG CVMX_PKO_PDM_CFG_DBG_FUNC()
static inline uint64_t CVMX_PKO_PDM_CFG_DBG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_CFG_DBG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000800FF8ull);
}
#else
#define CVMX_PKO_PDM_CFG_DBG (CVMX_ADD_IO_SEG(0x0001540000800FF8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_CP_DBG CVMX_PKO_PDM_CP_DBG_FUNC()
static inline uint64_t CVMX_PKO_PDM_CP_DBG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_CP_DBG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000800190ull);
}
#else
#define CVMX_PKO_PDM_CP_DBG (CVMX_ADD_IO_SEG(0x0001540000800190ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_PDM_DQX_MINPAD(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_PKO_PDM_DQX_MINPAD(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00015400008F0000ull) + ((offset) & 1023) * 8;
}
#else
#define CVMX_PKO_PDM_DQX_MINPAD(offset) (CVMX_ADD_IO_SEG(0x00015400008F0000ull) + ((offset) & 1023) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_DRPBUF_DBG CVMX_PKO_PDM_DRPBUF_DBG_FUNC()
static inline uint64_t CVMX_PKO_PDM_DRPBUF_DBG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_DRPBUF_DBG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400008000B0ull);
}
#else
#define CVMX_PKO_PDM_DRPBUF_DBG (CVMX_ADD_IO_SEG(0x00015400008000B0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_DWPBUF_DBG CVMX_PKO_PDM_DWPBUF_DBG_FUNC()
static inline uint64_t CVMX_PKO_PDM_DWPBUF_DBG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_DWPBUF_DBG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400008000A8ull);
}
#else
#define CVMX_PKO_PDM_DWPBUF_DBG (CVMX_ADD_IO_SEG(0x00015400008000A8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_ECC_CTL0 CVMX_PKO_PDM_ECC_CTL0_FUNC()
static inline uint64_t CVMX_PKO_PDM_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_ECC_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400008FFFD0ull);
}
#else
#define CVMX_PKO_PDM_ECC_CTL0 (CVMX_ADD_IO_SEG(0x00015400008FFFD0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_ECC_CTL1 CVMX_PKO_PDM_ECC_CTL1_FUNC()
static inline uint64_t CVMX_PKO_PDM_ECC_CTL1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_ECC_CTL1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400008FFFD8ull);
}
#else
#define CVMX_PKO_PDM_ECC_CTL1 (CVMX_ADD_IO_SEG(0x00015400008FFFD8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_ECC_DBE_STS0 CVMX_PKO_PDM_ECC_DBE_STS0_FUNC()
static inline uint64_t CVMX_PKO_PDM_ECC_DBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_ECC_DBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400008FFFF0ull);
}
#else
#define CVMX_PKO_PDM_ECC_DBE_STS0 (CVMX_ADD_IO_SEG(0x00015400008FFFF0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_ECC_DBE_STS_CMB0 CVMX_PKO_PDM_ECC_DBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PKO_PDM_ECC_DBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_ECC_DBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400008FFFE0ull);
}
#else
#define CVMX_PKO_PDM_ECC_DBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x00015400008FFFE0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_ECC_SBE_STS0 CVMX_PKO_PDM_ECC_SBE_STS0_FUNC()
static inline uint64_t CVMX_PKO_PDM_ECC_SBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_ECC_SBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400008FFFF8ull);
}
#else
#define CVMX_PKO_PDM_ECC_SBE_STS0 (CVMX_ADD_IO_SEG(0x00015400008FFFF8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_ECC_SBE_STS_CMB0 CVMX_PKO_PDM_ECC_SBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PKO_PDM_ECC_SBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_ECC_SBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400008FFFE8ull);
}
#else
#define CVMX_PKO_PDM_ECC_SBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x00015400008FFFE8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_FILLB_DBG0 CVMX_PKO_PDM_FILLB_DBG0_FUNC()
static inline uint64_t CVMX_PKO_PDM_FILLB_DBG0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_FILLB_DBG0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400008002A0ull);
}
#else
#define CVMX_PKO_PDM_FILLB_DBG0 (CVMX_ADD_IO_SEG(0x00015400008002A0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_FILLB_DBG1 CVMX_PKO_PDM_FILLB_DBG1_FUNC()
static inline uint64_t CVMX_PKO_PDM_FILLB_DBG1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_FILLB_DBG1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400008002A8ull);
}
#else
#define CVMX_PKO_PDM_FILLB_DBG1 (CVMX_ADD_IO_SEG(0x00015400008002A8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_FILLB_DBG2 CVMX_PKO_PDM_FILLB_DBG2_FUNC()
static inline uint64_t CVMX_PKO_PDM_FILLB_DBG2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_FILLB_DBG2 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400008002B0ull);
}
#else
#define CVMX_PKO_PDM_FILLB_DBG2 (CVMX_ADD_IO_SEG(0x00015400008002B0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_FLSHB_DBG0 CVMX_PKO_PDM_FLSHB_DBG0_FUNC()
static inline uint64_t CVMX_PKO_PDM_FLSHB_DBG0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_FLSHB_DBG0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400008002B8ull);
}
#else
#define CVMX_PKO_PDM_FLSHB_DBG0 (CVMX_ADD_IO_SEG(0x00015400008002B8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_FLSHB_DBG1 CVMX_PKO_PDM_FLSHB_DBG1_FUNC()
static inline uint64_t CVMX_PKO_PDM_FLSHB_DBG1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_FLSHB_DBG1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400008002C0ull);
}
#else
#define CVMX_PKO_PDM_FLSHB_DBG1 (CVMX_ADD_IO_SEG(0x00015400008002C0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_ISRD_DBG CVMX_PKO_PDM_ISRD_DBG_FUNC()
static inline uint64_t CVMX_PKO_PDM_ISRD_DBG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_ISRD_DBG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000800090ull);
}
#else
#define CVMX_PKO_PDM_ISRD_DBG (CVMX_ADD_IO_SEG(0x0001540000800090ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_ISRD_DBG_DQ CVMX_PKO_PDM_ISRD_DBG_DQ_FUNC()
static inline uint64_t CVMX_PKO_PDM_ISRD_DBG_DQ_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_ISRD_DBG_DQ not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000800290ull);
}
#else
#define CVMX_PKO_PDM_ISRD_DBG_DQ (CVMX_ADD_IO_SEG(0x0001540000800290ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_ISRM_DBG CVMX_PKO_PDM_ISRM_DBG_FUNC()
static inline uint64_t CVMX_PKO_PDM_ISRM_DBG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_ISRM_DBG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000800098ull);
}
#else
#define CVMX_PKO_PDM_ISRM_DBG (CVMX_ADD_IO_SEG(0x0001540000800098ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_ISRM_DBG_DQ CVMX_PKO_PDM_ISRM_DBG_DQ_FUNC()
static inline uint64_t CVMX_PKO_PDM_ISRM_DBG_DQ_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_ISRM_DBG_DQ not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000800298ull);
}
#else
#define CVMX_PKO_PDM_ISRM_DBG_DQ (CVMX_ADD_IO_SEG(0x0001540000800298ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_MEM_ADDR CVMX_PKO_PDM_MEM_ADDR_FUNC()
static inline uint64_t CVMX_PKO_PDM_MEM_ADDR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_MEM_ADDR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000800018ull);
}
#else
#define CVMX_PKO_PDM_MEM_ADDR (CVMX_ADD_IO_SEG(0x0001540000800018ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_MEM_DATA CVMX_PKO_PDM_MEM_DATA_FUNC()
static inline uint64_t CVMX_PKO_PDM_MEM_DATA_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_MEM_DATA not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000800010ull);
}
#else
#define CVMX_PKO_PDM_MEM_DATA (CVMX_ADD_IO_SEG(0x0001540000800010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_MEM_RW_CTL CVMX_PKO_PDM_MEM_RW_CTL_FUNC()
static inline uint64_t CVMX_PKO_PDM_MEM_RW_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_MEM_RW_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000800020ull);
}
#else
#define CVMX_PKO_PDM_MEM_RW_CTL (CVMX_ADD_IO_SEG(0x0001540000800020ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_MEM_RW_STS CVMX_PKO_PDM_MEM_RW_STS_FUNC()
static inline uint64_t CVMX_PKO_PDM_MEM_RW_STS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_MEM_RW_STS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000800028ull);
}
#else
#define CVMX_PKO_PDM_MEM_RW_STS (CVMX_ADD_IO_SEG(0x0001540000800028ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_MWPBUF_DBG CVMX_PKO_PDM_MWPBUF_DBG_FUNC()
static inline uint64_t CVMX_PKO_PDM_MWPBUF_DBG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_MWPBUF_DBG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400008000A0ull);
}
#else
#define CVMX_PKO_PDM_MWPBUF_DBG (CVMX_ADD_IO_SEG(0x00015400008000A0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PDM_STS CVMX_PKO_PDM_STS_FUNC()
static inline uint64_t CVMX_PKO_PDM_STS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PDM_STS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000800008ull);
}
#else
#define CVMX_PKO_PDM_STS (CVMX_ADD_IO_SEG(0x0001540000800008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PEB_BIST_STATUS CVMX_PKO_PEB_BIST_STATUS_FUNC()
static inline uint64_t CVMX_PKO_PEB_BIST_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PEB_BIST_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000900D00ull);
}
#else
#define CVMX_PKO_PEB_BIST_STATUS (CVMX_ADD_IO_SEG(0x0001540000900D00ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PEB_ECC_CTL0 CVMX_PKO_PEB_ECC_CTL0_FUNC()
static inline uint64_t CVMX_PKO_PEB_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PEB_ECC_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400009FFFD0ull);
}
#else
#define CVMX_PKO_PEB_ECC_CTL0 (CVMX_ADD_IO_SEG(0x00015400009FFFD0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PEB_ECC_CTL1 CVMX_PKO_PEB_ECC_CTL1_FUNC()
static inline uint64_t CVMX_PKO_PEB_ECC_CTL1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PEB_ECC_CTL1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400009FFFA8ull);
}
#else
#define CVMX_PKO_PEB_ECC_CTL1 (CVMX_ADD_IO_SEG(0x00015400009FFFA8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PEB_ECC_DBE_STS0 CVMX_PKO_PEB_ECC_DBE_STS0_FUNC()
static inline uint64_t CVMX_PKO_PEB_ECC_DBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PEB_ECC_DBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400009FFFF0ull);
}
#else
#define CVMX_PKO_PEB_ECC_DBE_STS0 (CVMX_ADD_IO_SEG(0x00015400009FFFF0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PEB_ECC_DBE_STS_CMB0 CVMX_PKO_PEB_ECC_DBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PKO_PEB_ECC_DBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PEB_ECC_DBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400009FFFD8ull);
}
#else
#define CVMX_PKO_PEB_ECC_DBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x00015400009FFFD8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PEB_ECC_SBE_STS0 CVMX_PKO_PEB_ECC_SBE_STS0_FUNC()
static inline uint64_t CVMX_PKO_PEB_ECC_SBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PEB_ECC_SBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400009FFFF8ull);
}
#else
#define CVMX_PKO_PEB_ECC_SBE_STS0 (CVMX_ADD_IO_SEG(0x00015400009FFFF8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PEB_ECC_SBE_STS_CMB0 CVMX_PKO_PEB_ECC_SBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PKO_PEB_ECC_SBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PEB_ECC_SBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400009FFFE8ull);
}
#else
#define CVMX_PKO_PEB_ECC_SBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x00015400009FFFE8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PEB_ERR_INT CVMX_PKO_PEB_ERR_INT_FUNC()
static inline uint64_t CVMX_PKO_PEB_ERR_INT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PEB_ERR_INT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000900C00ull);
}
#else
#define CVMX_PKO_PEB_ERR_INT (CVMX_ADD_IO_SEG(0x0001540000900C00ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PEB_EXT_HDR_DEF_ERR_INFO CVMX_PKO_PEB_EXT_HDR_DEF_ERR_INFO_FUNC()
static inline uint64_t CVMX_PKO_PEB_EXT_HDR_DEF_ERR_INFO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PEB_EXT_HDR_DEF_ERR_INFO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000900C08ull);
}
#else
#define CVMX_PKO_PEB_EXT_HDR_DEF_ERR_INFO (CVMX_ADD_IO_SEG(0x0001540000900C08ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PEB_FCS_SOP_ERR_INFO CVMX_PKO_PEB_FCS_SOP_ERR_INFO_FUNC()
static inline uint64_t CVMX_PKO_PEB_FCS_SOP_ERR_INFO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PEB_FCS_SOP_ERR_INFO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000900C18ull);
}
#else
#define CVMX_PKO_PEB_FCS_SOP_ERR_INFO (CVMX_ADD_IO_SEG(0x0001540000900C18ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PEB_JUMP_DEF_ERR_INFO CVMX_PKO_PEB_JUMP_DEF_ERR_INFO_FUNC()
static inline uint64_t CVMX_PKO_PEB_JUMP_DEF_ERR_INFO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PEB_JUMP_DEF_ERR_INFO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000900C10ull);
}
#else
#define CVMX_PKO_PEB_JUMP_DEF_ERR_INFO (CVMX_ADD_IO_SEG(0x0001540000900C10ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PEB_MACX_CFG_WR_ERR_INFO CVMX_PKO_PEB_MACX_CFG_WR_ERR_INFO_FUNC()
static inline uint64_t CVMX_PKO_PEB_MACX_CFG_WR_ERR_INFO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PEB_MACX_CFG_WR_ERR_INFO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000900C50ull);
}
#else
#define CVMX_PKO_PEB_MACX_CFG_WR_ERR_INFO (CVMX_ADD_IO_SEG(0x0001540000900C50ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PEB_MAX_LINK_ERR_INFO CVMX_PKO_PEB_MAX_LINK_ERR_INFO_FUNC()
static inline uint64_t CVMX_PKO_PEB_MAX_LINK_ERR_INFO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PEB_MAX_LINK_ERR_INFO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000900C48ull);
}
#else
#define CVMX_PKO_PEB_MAX_LINK_ERR_INFO (CVMX_ADD_IO_SEG(0x0001540000900C48ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PEB_PAD_ERR_INFO CVMX_PKO_PEB_PAD_ERR_INFO_FUNC()
static inline uint64_t CVMX_PKO_PEB_PAD_ERR_INFO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PEB_PAD_ERR_INFO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000900C28ull);
}
#else
#define CVMX_PKO_PEB_PAD_ERR_INFO (CVMX_ADD_IO_SEG(0x0001540000900C28ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PEB_PSE_FIFO_ERR_INFO CVMX_PKO_PEB_PSE_FIFO_ERR_INFO_FUNC()
static inline uint64_t CVMX_PKO_PEB_PSE_FIFO_ERR_INFO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PEB_PSE_FIFO_ERR_INFO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000900C20ull);
}
#else
#define CVMX_PKO_PEB_PSE_FIFO_ERR_INFO (CVMX_ADD_IO_SEG(0x0001540000900C20ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PEB_SUBD_ADDR_ERR_INFO CVMX_PKO_PEB_SUBD_ADDR_ERR_INFO_FUNC()
static inline uint64_t CVMX_PKO_PEB_SUBD_ADDR_ERR_INFO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PEB_SUBD_ADDR_ERR_INFO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000900C38ull);
}
#else
#define CVMX_PKO_PEB_SUBD_ADDR_ERR_INFO (CVMX_ADD_IO_SEG(0x0001540000900C38ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PEB_SUBD_SIZE_ERR_INFO CVMX_PKO_PEB_SUBD_SIZE_ERR_INFO_FUNC()
static inline uint64_t CVMX_PKO_PEB_SUBD_SIZE_ERR_INFO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PEB_SUBD_SIZE_ERR_INFO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000900C40ull);
}
#else
#define CVMX_PKO_PEB_SUBD_SIZE_ERR_INFO (CVMX_ADD_IO_SEG(0x0001540000900C40ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PEB_TRUNC_ERR_INFO CVMX_PKO_PEB_TRUNC_ERR_INFO_FUNC()
static inline uint64_t CVMX_PKO_PEB_TRUNC_ERR_INFO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PEB_TRUNC_ERR_INFO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000900C30ull);
}
#else
#define CVMX_PKO_PEB_TRUNC_ERR_INFO (CVMX_ADD_IO_SEG(0x0001540000900C30ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PQA_DEBUG CVMX_PKO_PQA_DEBUG_FUNC()
static inline uint64_t CVMX_PKO_PQA_DEBUG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PQA_DEBUG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000000128ull);
}
#else
#define CVMX_PKO_PQA_DEBUG (CVMX_ADD_IO_SEG(0x0001540000000128ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PQB_DEBUG CVMX_PKO_PQB_DEBUG_FUNC()
static inline uint64_t CVMX_PKO_PQB_DEBUG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PQB_DEBUG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000000130ull);
}
#else
#define CVMX_PKO_PQB_DEBUG (CVMX_ADD_IO_SEG(0x0001540000000130ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PQ_CSR_BUS_DEBUG CVMX_PKO_PQ_CSR_BUS_DEBUG_FUNC()
static inline uint64_t CVMX_PKO_PQ_CSR_BUS_DEBUG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PQ_CSR_BUS_DEBUG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400000001F8ull);
}
#else
#define CVMX_PKO_PQ_CSR_BUS_DEBUG (CVMX_ADD_IO_SEG(0x00015400000001F8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PQ_DEBUG_GREEN CVMX_PKO_PQ_DEBUG_GREEN_FUNC()
static inline uint64_t CVMX_PKO_PQ_DEBUG_GREEN_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PQ_DEBUG_GREEN not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000000058ull);
}
#else
#define CVMX_PKO_PQ_DEBUG_GREEN (CVMX_ADD_IO_SEG(0x0001540000000058ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PQ_DEBUG_LINKS CVMX_PKO_PQ_DEBUG_LINKS_FUNC()
static inline uint64_t CVMX_PKO_PQ_DEBUG_LINKS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PQ_DEBUG_LINKS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000000068ull);
}
#else
#define CVMX_PKO_PQ_DEBUG_LINKS (CVMX_ADD_IO_SEG(0x0001540000000068ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PQ_DEBUG_YELLOW CVMX_PKO_PQ_DEBUG_YELLOW_FUNC()
static inline uint64_t CVMX_PKO_PQ_DEBUG_YELLOW_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PQ_DEBUG_YELLOW not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000000060ull);
}
#else
#define CVMX_PKO_PQ_DEBUG_YELLOW (CVMX_ADD_IO_SEG(0x0001540000000060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_DQ_BIST_STATUS CVMX_PKO_PSE_DQ_BIST_STATUS_FUNC()
static inline uint64_t CVMX_PKO_PSE_DQ_BIST_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_DQ_BIST_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000300138ull);
}
#else
#define CVMX_PKO_PSE_DQ_BIST_STATUS (CVMX_ADD_IO_SEG(0x0001540000300138ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_DQ_ECC_CTL0 CVMX_PKO_PSE_DQ_ECC_CTL0_FUNC()
static inline uint64_t CVMX_PKO_PSE_DQ_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_DQ_ECC_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000300100ull);
}
#else
#define CVMX_PKO_PSE_DQ_ECC_CTL0 (CVMX_ADD_IO_SEG(0x0001540000300100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_DQ_ECC_DBE_STS0 CVMX_PKO_PSE_DQ_ECC_DBE_STS0_FUNC()
static inline uint64_t CVMX_PKO_PSE_DQ_ECC_DBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_DQ_ECC_DBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000300118ull);
}
#else
#define CVMX_PKO_PSE_DQ_ECC_DBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000300118ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_DQ_ECC_DBE_STS_CMB0 CVMX_PKO_PSE_DQ_ECC_DBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PKO_PSE_DQ_ECC_DBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_DQ_ECC_DBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000300120ull);
}
#else
#define CVMX_PKO_PSE_DQ_ECC_DBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000300120ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_DQ_ECC_SBE_STS0 CVMX_PKO_PSE_DQ_ECC_SBE_STS0_FUNC()
static inline uint64_t CVMX_PKO_PSE_DQ_ECC_SBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_DQ_ECC_SBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000300108ull);
}
#else
#define CVMX_PKO_PSE_DQ_ECC_SBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000300108ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_DQ_ECC_SBE_STS_CMB0 CVMX_PKO_PSE_DQ_ECC_SBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PKO_PSE_DQ_ECC_SBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_DQ_ECC_SBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000300110ull);
}
#else
#define CVMX_PKO_PSE_DQ_ECC_SBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000300110ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_PQ_BIST_STATUS CVMX_PKO_PSE_PQ_BIST_STATUS_FUNC()
static inline uint64_t CVMX_PKO_PSE_PQ_BIST_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_PQ_BIST_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000000138ull);
}
#else
#define CVMX_PKO_PSE_PQ_BIST_STATUS (CVMX_ADD_IO_SEG(0x0001540000000138ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_PQ_ECC_CTL0 CVMX_PKO_PSE_PQ_ECC_CTL0_FUNC()
static inline uint64_t CVMX_PKO_PSE_PQ_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_PQ_ECC_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000000100ull);
}
#else
#define CVMX_PKO_PSE_PQ_ECC_CTL0 (CVMX_ADD_IO_SEG(0x0001540000000100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_PQ_ECC_DBE_STS0 CVMX_PKO_PSE_PQ_ECC_DBE_STS0_FUNC()
static inline uint64_t CVMX_PKO_PSE_PQ_ECC_DBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_PQ_ECC_DBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000000118ull);
}
#else
#define CVMX_PKO_PSE_PQ_ECC_DBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000000118ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_PQ_ECC_DBE_STS_CMB0 CVMX_PKO_PSE_PQ_ECC_DBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PKO_PSE_PQ_ECC_DBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_PQ_ECC_DBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000000120ull);
}
#else
#define CVMX_PKO_PSE_PQ_ECC_DBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000000120ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_PQ_ECC_SBE_STS0 CVMX_PKO_PSE_PQ_ECC_SBE_STS0_FUNC()
static inline uint64_t CVMX_PKO_PSE_PQ_ECC_SBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_PQ_ECC_SBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000000108ull);
}
#else
#define CVMX_PKO_PSE_PQ_ECC_SBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000000108ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_PQ_ECC_SBE_STS_CMB0 CVMX_PKO_PSE_PQ_ECC_SBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PKO_PSE_PQ_ECC_SBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_PQ_ECC_SBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000000110ull);
}
#else
#define CVMX_PKO_PSE_PQ_ECC_SBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000000110ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ1_BIST_STATUS CVMX_PKO_PSE_SQ1_BIST_STATUS_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ1_BIST_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ1_BIST_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000080138ull);
}
#else
#define CVMX_PKO_PSE_SQ1_BIST_STATUS (CVMX_ADD_IO_SEG(0x0001540000080138ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ1_ECC_CTL0 CVMX_PKO_PSE_SQ1_ECC_CTL0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ1_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ1_ECC_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000080100ull);
}
#else
#define CVMX_PKO_PSE_SQ1_ECC_CTL0 (CVMX_ADD_IO_SEG(0x0001540000080100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ1_ECC_DBE_STS0 CVMX_PKO_PSE_SQ1_ECC_DBE_STS0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ1_ECC_DBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ1_ECC_DBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000080118ull);
}
#else
#define CVMX_PKO_PSE_SQ1_ECC_DBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000080118ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ1_ECC_DBE_STS_CMB0 CVMX_PKO_PSE_SQ1_ECC_DBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ1_ECC_DBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ1_ECC_DBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000080120ull);
}
#else
#define CVMX_PKO_PSE_SQ1_ECC_DBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000080120ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ1_ECC_SBE_STS0 CVMX_PKO_PSE_SQ1_ECC_SBE_STS0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ1_ECC_SBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ1_ECC_SBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000080108ull);
}
#else
#define CVMX_PKO_PSE_SQ1_ECC_SBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000080108ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ1_ECC_SBE_STS_CMB0 CVMX_PKO_PSE_SQ1_ECC_SBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ1_ECC_SBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ1_ECC_SBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000080110ull);
}
#else
#define CVMX_PKO_PSE_SQ1_ECC_SBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000080110ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ2_BIST_STATUS CVMX_PKO_PSE_SQ2_BIST_STATUS_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ2_BIST_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ2_BIST_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000100138ull);
}
#else
#define CVMX_PKO_PSE_SQ2_BIST_STATUS (CVMX_ADD_IO_SEG(0x0001540000100138ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ2_ECC_CTL0 CVMX_PKO_PSE_SQ2_ECC_CTL0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ2_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ2_ECC_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000100100ull);
}
#else
#define CVMX_PKO_PSE_SQ2_ECC_CTL0 (CVMX_ADD_IO_SEG(0x0001540000100100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ2_ECC_DBE_STS0 CVMX_PKO_PSE_SQ2_ECC_DBE_STS0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ2_ECC_DBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ2_ECC_DBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000100118ull);
}
#else
#define CVMX_PKO_PSE_SQ2_ECC_DBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000100118ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ2_ECC_DBE_STS_CMB0 CVMX_PKO_PSE_SQ2_ECC_DBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ2_ECC_DBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ2_ECC_DBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000100120ull);
}
#else
#define CVMX_PKO_PSE_SQ2_ECC_DBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000100120ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ2_ECC_SBE_STS0 CVMX_PKO_PSE_SQ2_ECC_SBE_STS0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ2_ECC_SBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ2_ECC_SBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000100108ull);
}
#else
#define CVMX_PKO_PSE_SQ2_ECC_SBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000100108ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ2_ECC_SBE_STS_CMB0 CVMX_PKO_PSE_SQ2_ECC_SBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ2_ECC_SBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ2_ECC_SBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000100110ull);
}
#else
#define CVMX_PKO_PSE_SQ2_ECC_SBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000100110ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ3_BIST_STATUS CVMX_PKO_PSE_SQ3_BIST_STATUS_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ3_BIST_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ3_BIST_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000180138ull);
}
#else
#define CVMX_PKO_PSE_SQ3_BIST_STATUS (CVMX_ADD_IO_SEG(0x0001540000180138ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ3_ECC_CTL0 CVMX_PKO_PSE_SQ3_ECC_CTL0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ3_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ3_ECC_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000180100ull);
}
#else
#define CVMX_PKO_PSE_SQ3_ECC_CTL0 (CVMX_ADD_IO_SEG(0x0001540000180100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ3_ECC_DBE_STS0 CVMX_PKO_PSE_SQ3_ECC_DBE_STS0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ3_ECC_DBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ3_ECC_DBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000180118ull);
}
#else
#define CVMX_PKO_PSE_SQ3_ECC_DBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000180118ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ3_ECC_DBE_STS_CMB0 CVMX_PKO_PSE_SQ3_ECC_DBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ3_ECC_DBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ3_ECC_DBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000180120ull);
}
#else
#define CVMX_PKO_PSE_SQ3_ECC_DBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000180120ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ3_ECC_SBE_STS0 CVMX_PKO_PSE_SQ3_ECC_SBE_STS0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ3_ECC_SBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ3_ECC_SBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000180108ull);
}
#else
#define CVMX_PKO_PSE_SQ3_ECC_SBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000180108ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ3_ECC_SBE_STS_CMB0 CVMX_PKO_PSE_SQ3_ECC_SBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ3_ECC_SBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ3_ECC_SBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000180110ull);
}
#else
#define CVMX_PKO_PSE_SQ3_ECC_SBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000180110ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ4_BIST_STATUS CVMX_PKO_PSE_SQ4_BIST_STATUS_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ4_BIST_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ4_BIST_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000200138ull);
}
#else
#define CVMX_PKO_PSE_SQ4_BIST_STATUS (CVMX_ADD_IO_SEG(0x0001540000200138ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ4_ECC_CTL0 CVMX_PKO_PSE_SQ4_ECC_CTL0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ4_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ4_ECC_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000200100ull);
}
#else
#define CVMX_PKO_PSE_SQ4_ECC_CTL0 (CVMX_ADD_IO_SEG(0x0001540000200100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ4_ECC_DBE_STS0 CVMX_PKO_PSE_SQ4_ECC_DBE_STS0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ4_ECC_DBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ4_ECC_DBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000200118ull);
}
#else
#define CVMX_PKO_PSE_SQ4_ECC_DBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000200118ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ4_ECC_DBE_STS_CMB0 CVMX_PKO_PSE_SQ4_ECC_DBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ4_ECC_DBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ4_ECC_DBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000200120ull);
}
#else
#define CVMX_PKO_PSE_SQ4_ECC_DBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000200120ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ4_ECC_SBE_STS0 CVMX_PKO_PSE_SQ4_ECC_SBE_STS0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ4_ECC_SBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ4_ECC_SBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000200108ull);
}
#else
#define CVMX_PKO_PSE_SQ4_ECC_SBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000200108ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ4_ECC_SBE_STS_CMB0 CVMX_PKO_PSE_SQ4_ECC_SBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ4_ECC_SBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ4_ECC_SBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000200110ull);
}
#else
#define CVMX_PKO_PSE_SQ4_ECC_SBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000200110ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ5_BIST_STATUS CVMX_PKO_PSE_SQ5_BIST_STATUS_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ5_BIST_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ5_BIST_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000280138ull);
}
#else
#define CVMX_PKO_PSE_SQ5_BIST_STATUS (CVMX_ADD_IO_SEG(0x0001540000280138ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ5_ECC_CTL0 CVMX_PKO_PSE_SQ5_ECC_CTL0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ5_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ5_ECC_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000280100ull);
}
#else
#define CVMX_PKO_PSE_SQ5_ECC_CTL0 (CVMX_ADD_IO_SEG(0x0001540000280100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ5_ECC_DBE_STS0 CVMX_PKO_PSE_SQ5_ECC_DBE_STS0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ5_ECC_DBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ5_ECC_DBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000280118ull);
}
#else
#define CVMX_PKO_PSE_SQ5_ECC_DBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000280118ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ5_ECC_DBE_STS_CMB0 CVMX_PKO_PSE_SQ5_ECC_DBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ5_ECC_DBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ5_ECC_DBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000280120ull);
}
#else
#define CVMX_PKO_PSE_SQ5_ECC_DBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000280120ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ5_ECC_SBE_STS0 CVMX_PKO_PSE_SQ5_ECC_SBE_STS0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ5_ECC_SBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ5_ECC_SBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000280108ull);
}
#else
#define CVMX_PKO_PSE_SQ5_ECC_SBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000280108ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PSE_SQ5_ECC_SBE_STS_CMB0 CVMX_PKO_PSE_SQ5_ECC_SBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PKO_PSE_SQ5_ECC_SBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PSE_SQ5_ECC_SBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000280110ull);
}
#else
#define CVMX_PKO_PSE_SQ5_ECC_SBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000280110ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_PTFX_STATUS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 27)))))
		cvmx_warn("CVMX_PKO_PTFX_STATUS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000900100ull) + ((offset) & 31) * 8;
}
#else
#define CVMX_PKO_PTFX_STATUS(offset) (CVMX_ADD_IO_SEG(0x0001540000900100ull) + ((offset) & 31) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_PTF_IOBP_CFG CVMX_PKO_PTF_IOBP_CFG_FUNC()
static inline uint64_t CVMX_PKO_PTF_IOBP_CFG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_PTF_IOBP_CFG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000900300ull);
}
#else
#define CVMX_PKO_PTF_IOBP_CFG (CVMX_ADD_IO_SEG(0x0001540000900300ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_PTGFX_CFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7)))))
		cvmx_warn("CVMX_PKO_PTGFX_CFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001540000900200ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_PKO_PTGFX_CFG(offset) (CVMX_ADD_IO_SEG(0x0001540000900200ull) + ((offset) & 7) * 8)
#endif
#define CVMX_PKO_REG_BIST_RESULT (CVMX_ADD_IO_SEG(0x0001180050000080ull))
#define CVMX_PKO_REG_CMD_BUF (CVMX_ADD_IO_SEG(0x0001180050000010ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_REG_CRC_CTLX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN38XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN58XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PKO_REG_CRC_CTLX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180050000028ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_PKO_REG_CRC_CTLX(offset) (CVMX_ADD_IO_SEG(0x0001180050000028ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_REG_CRC_ENABLE CVMX_PKO_REG_CRC_ENABLE_FUNC()
static inline uint64_t CVMX_PKO_REG_CRC_ENABLE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN38XX) || OCTEON_IS_MODEL(OCTEON_CN58XX)))
		cvmx_warn("CVMX_PKO_REG_CRC_ENABLE not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050000020ull);
}
#else
#define CVMX_PKO_REG_CRC_ENABLE (CVMX_ADD_IO_SEG(0x0001180050000020ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_REG_CRC_IVX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN38XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN58XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PKO_REG_CRC_IVX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180050000038ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_PKO_REG_CRC_IVX(offset) (CVMX_ADD_IO_SEG(0x0001180050000038ull) + ((offset) & 1) * 8)
#endif
#define CVMX_PKO_REG_DEBUG0 (CVMX_ADD_IO_SEG(0x0001180050000098ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_REG_DEBUG1 CVMX_PKO_REG_DEBUG1_FUNC()
static inline uint64_t CVMX_PKO_REG_DEBUG1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN5XXX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_PKO_REG_DEBUG1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800500000A0ull);
}
#else
#define CVMX_PKO_REG_DEBUG1 (CVMX_ADD_IO_SEG(0x00011800500000A0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_REG_DEBUG2 CVMX_PKO_REG_DEBUG2_FUNC()
static inline uint64_t CVMX_PKO_REG_DEBUG2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN5XXX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_PKO_REG_DEBUG2 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800500000A8ull);
}
#else
#define CVMX_PKO_REG_DEBUG2 (CVMX_ADD_IO_SEG(0x00011800500000A8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_REG_DEBUG3 CVMX_PKO_REG_DEBUG3_FUNC()
static inline uint64_t CVMX_PKO_REG_DEBUG3_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN5XXX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_PKO_REG_DEBUG3 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800500000B0ull);
}
#else
#define CVMX_PKO_REG_DEBUG3 (CVMX_ADD_IO_SEG(0x00011800500000B0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_REG_DEBUG4 CVMX_PKO_REG_DEBUG4_FUNC()
static inline uint64_t CVMX_PKO_REG_DEBUG4_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_PKO_REG_DEBUG4 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800500000B8ull);
}
#else
#define CVMX_PKO_REG_DEBUG4 (CVMX_ADD_IO_SEG(0x00011800500000B8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_REG_ENGINE_INFLIGHT CVMX_PKO_REG_ENGINE_INFLIGHT_FUNC()
static inline uint64_t CVMX_PKO_REG_ENGINE_INFLIGHT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_PKO_REG_ENGINE_INFLIGHT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050000050ull);
}
#else
#define CVMX_PKO_REG_ENGINE_INFLIGHT (CVMX_ADD_IO_SEG(0x0001180050000050ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_REG_ENGINE_INFLIGHT1 CVMX_PKO_REG_ENGINE_INFLIGHT1_FUNC()
static inline uint64_t CVMX_PKO_REG_ENGINE_INFLIGHT1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_PKO_REG_ENGINE_INFLIGHT1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050000318ull);
}
#else
#define CVMX_PKO_REG_ENGINE_INFLIGHT1 (CVMX_ADD_IO_SEG(0x0001180050000318ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PKO_REG_ENGINE_STORAGEX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_PKO_REG_ENGINE_STORAGEX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180050000300ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_PKO_REG_ENGINE_STORAGEX(offset) (CVMX_ADD_IO_SEG(0x0001180050000300ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_REG_ENGINE_THRESH CVMX_PKO_REG_ENGINE_THRESH_FUNC()
static inline uint64_t CVMX_PKO_REG_ENGINE_THRESH_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_PKO_REG_ENGINE_THRESH not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050000058ull);
}
#else
#define CVMX_PKO_REG_ENGINE_THRESH (CVMX_ADD_IO_SEG(0x0001180050000058ull))
#endif
#define CVMX_PKO_REG_ERROR (CVMX_ADD_IO_SEG(0x0001180050000088ull))
#define CVMX_PKO_REG_FLAGS (CVMX_ADD_IO_SEG(0x0001180050000000ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_REG_GMX_PORT_MODE CVMX_PKO_REG_GMX_PORT_MODE_FUNC()
static inline uint64_t CVMX_PKO_REG_GMX_PORT_MODE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_PKO_REG_GMX_PORT_MODE not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050000018ull);
}
#else
#define CVMX_PKO_REG_GMX_PORT_MODE (CVMX_ADD_IO_SEG(0x0001180050000018ull))
#endif
#define CVMX_PKO_REG_INT_MASK (CVMX_ADD_IO_SEG(0x0001180050000090ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_REG_LOOPBACK_BPID CVMX_PKO_REG_LOOPBACK_BPID_FUNC()
static inline uint64_t CVMX_PKO_REG_LOOPBACK_BPID_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_PKO_REG_LOOPBACK_BPID not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050000118ull);
}
#else
#define CVMX_PKO_REG_LOOPBACK_BPID (CVMX_ADD_IO_SEG(0x0001180050000118ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_REG_LOOPBACK_PKIND CVMX_PKO_REG_LOOPBACK_PKIND_FUNC()
static inline uint64_t CVMX_PKO_REG_LOOPBACK_PKIND_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_PKO_REG_LOOPBACK_PKIND not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050000068ull);
}
#else
#define CVMX_PKO_REG_LOOPBACK_PKIND (CVMX_ADD_IO_SEG(0x0001180050000068ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_REG_MIN_PKT CVMX_PKO_REG_MIN_PKT_FUNC()
static inline uint64_t CVMX_PKO_REG_MIN_PKT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_PKO_REG_MIN_PKT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050000070ull);
}
#else
#define CVMX_PKO_REG_MIN_PKT (CVMX_ADD_IO_SEG(0x0001180050000070ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_REG_PREEMPT CVMX_PKO_REG_PREEMPT_FUNC()
static inline uint64_t CVMX_PKO_REG_PREEMPT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_PKO_REG_PREEMPT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050000110ull);
}
#else
#define CVMX_PKO_REG_PREEMPT (CVMX_ADD_IO_SEG(0x0001180050000110ull))
#endif
#define CVMX_PKO_REG_QUEUE_MODE (CVMX_ADD_IO_SEG(0x0001180050000048ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_REG_QUEUE_PREEMPT CVMX_PKO_REG_QUEUE_PREEMPT_FUNC()
static inline uint64_t CVMX_PKO_REG_QUEUE_PREEMPT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_PKO_REG_QUEUE_PREEMPT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050000108ull);
}
#else
#define CVMX_PKO_REG_QUEUE_PREEMPT (CVMX_ADD_IO_SEG(0x0001180050000108ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_REG_QUEUE_PTRS1 CVMX_PKO_REG_QUEUE_PTRS1_FUNC()
static inline uint64_t CVMX_PKO_REG_QUEUE_PTRS1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN5XXX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_PKO_REG_QUEUE_PTRS1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050000100ull);
}
#else
#define CVMX_PKO_REG_QUEUE_PTRS1 (CVMX_ADD_IO_SEG(0x0001180050000100ull))
#endif
#define CVMX_PKO_REG_READ_IDX (CVMX_ADD_IO_SEG(0x0001180050000008ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_REG_THROTTLE CVMX_PKO_REG_THROTTLE_FUNC()
static inline uint64_t CVMX_PKO_REG_THROTTLE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_PKO_REG_THROTTLE not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050000078ull);
}
#else
#define CVMX_PKO_REG_THROTTLE (CVMX_ADD_IO_SEG(0x0001180050000078ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_REG_TIMESTAMP CVMX_PKO_REG_TIMESTAMP_FUNC()
static inline uint64_t CVMX_PKO_REG_TIMESTAMP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_PKO_REG_TIMESTAMP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180050000060ull);
}
#else
#define CVMX_PKO_REG_TIMESTAMP (CVMX_ADD_IO_SEG(0x0001180050000060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PKO_STATUS CVMX_PKO_STATUS_FUNC()
static inline uint64_t CVMX_PKO_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PKO_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000D00000ull);
}
#else
#define CVMX_PKO_STATUS (CVMX_ADD_IO_SEG(0x0001540000D00000ull))
#endif

/**
 * cvmx_pko_channel_level
 */
union cvmx_pko_channel_level {
	uint64_t u64;
	struct cvmx_pko_channel_level_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t cc_level                     : 1;  /**< Channel credit level. Channels can be configured at levels 2 or 3 of the PSE hierarchy.
                                                         0 = Selects the level-2 as the channel level.
                                                         1 = Selects the level-3 as the channel level. */
#else
	uint64_t cc_level                     : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_pko_channel_level_s       cn78xx;
};
typedef union cvmx_pko_channel_level cvmx_pko_channel_level_t;

/**
 * cvmx_pko_dpfi_ena
 */
union cvmx_pko_dpfi_ena {
	uint64_t u64;
	struct cvmx_pko_dpfi_ena_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t enable                       : 1;  /**< Setting this bit enables the PKO to request/return pointers to FPA. This enable must be
                                                         set after reset so that the PKO can fill its private pointer cache prior to setting the
                                                         PKO_RDY flag in the PKO_STATUS register. */
#else
	uint64_t enable                       : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_pko_dpfi_ena_s            cn78xx;
};
typedef union cvmx_pko_dpfi_ena cvmx_pko_dpfi_ena_t;

/**
 * cvmx_pko_dpfi_flush
 */
union cvmx_pko_dpfi_flush {
	uint64_t u64;
	struct cvmx_pko_dpfi_flush_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t flush_en                     : 1;  /**< Pointer cache flush enable. When set, this flag commands the DPFI logic to flush all
                                                         valid pointers from the pointer cache and return them to the FPA. The flush operation is
                                                         complete when the CACHE_FLUSHED flag in PKO_DPFI_STATUS is set. Clearing the FLUSH_EN flag
                                                         results in the DPFI reloading its pointer cache. This flush mechanism should only be
                                                         enabled when the PKO is quiescent and all DQs have been closed. */
#else
	uint64_t flush_en                     : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_pko_dpfi_flush_s          cn78xx;
};
typedef union cvmx_pko_dpfi_flush cvmx_pko_dpfi_flush_t;

/**
 * cvmx_pko_dpfi_fpa_aura
 */
union cvmx_pko_dpfi_fpa_aura {
	uint64_t u64;
	struct cvmx_pko_dpfi_fpa_aura_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t node                         : 2;  /**< Node number of current chip, to ensure that the aura is on the local node. */
	uint64_t laura                        : 10; /**< Local aura to use for PKO command buffering. Must be on local OCI node. */
#else
	uint64_t laura                        : 10;
	uint64_t node                         : 2;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_pko_dpfi_fpa_aura_s       cn78xx;
};
typedef union cvmx_pko_dpfi_fpa_aura cvmx_pko_dpfi_fpa_aura_t;

/**
 * cvmx_pko_dpfi_status
 */
union cvmx_pko_dpfi_status {
	uint64_t u64;
	struct cvmx_pko_dpfi_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ptr_cnt                      : 32; /**< The number of pointers currently in use for storing descriptors
                                                         and meta-packets plus those available in the DPFI pointer cache. */
	uint64_t reserved_13_31               : 19;
	uint64_t isrd_ptr1_rtn_full           : 1;  /**< ISRD pointer return register 1 contains a valid pointer. */
	uint64_t isrd_ptr0_rtn_full           : 1;  /**< ISRD pointer return register 0 contains a valid pointer. */
	uint64_t isrm_ptr1_rtn_full           : 1;  /**< ISRM pointer return register 1 contains a valid pointer. */
	uint64_t isrm_ptr0_rtn_full           : 1;  /**< ISRM pointer return register 0 contains a valid pointer. */
	uint64_t isrd_ptr1_val                : 1;  /**< ISRD pointer register 1 contains a valid pointer. */
	uint64_t isrd_ptr0_val                : 1;  /**< ISRD pointer register 0 contains a valid pointer. */
	uint64_t isrm_ptr1_val                : 1;  /**< ISRM pointer register 1 contains a valid pointer. */
	uint64_t isrm_ptr0_val                : 1;  /**< ISRM pointer register 0 contains a valid pointer. */
	uint64_t ptr_req_pend                 : 1;  /**< DPFI has pointer requests to FPA pending. */
	uint64_t ptr_rtn_pend                 : 1;  /**< DPFI has pointer returns to FPA pending. */
	uint64_t fpa_empty                    : 1;  /**< 1 = FPA responded to pointer request with 'no pointers available.'
                                                         0 = FPA is providing pointers when requested. */
	uint64_t dpfi_empty                   : 1;  /**< DPFI pointer cache is empty. */
	uint64_t cache_flushed                : 1;  /**< 1 = Cache flush has completed. PKO_DPFI_STATUS[PTR_CNT] will read zero if all
                                                             outstanding pointers have been returned to the FPA.
                                                         0 = Cache flush not enabled or in-progress. */
#else
	uint64_t cache_flushed                : 1;
	uint64_t dpfi_empty                   : 1;
	uint64_t fpa_empty                    : 1;
	uint64_t ptr_rtn_pend                 : 1;
	uint64_t ptr_req_pend                 : 1;
	uint64_t isrm_ptr0_val                : 1;
	uint64_t isrm_ptr1_val                : 1;
	uint64_t isrd_ptr0_val                : 1;
	uint64_t isrd_ptr1_val                : 1;
	uint64_t isrm_ptr0_rtn_full           : 1;
	uint64_t isrm_ptr1_rtn_full           : 1;
	uint64_t isrd_ptr0_rtn_full           : 1;
	uint64_t isrd_ptr1_rtn_full           : 1;
	uint64_t reserved_13_31               : 19;
	uint64_t ptr_cnt                      : 32;
#endif
	} s;
	struct cvmx_pko_dpfi_status_s         cn78xx;
};
typedef union cvmx_pko_dpfi_status cvmx_pko_dpfi_status_t;

/**
 * cvmx_pko_dq#_bytes
 */
union cvmx_pko_dqx_bytes {
	uint64_t u64;
	struct cvmx_pko_dqx_bytes_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t count                        : 48; /**< Count. The running count of bytes. Note that this count wraps. */
#else
	uint64_t count                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pko_dqx_bytes_s           cn78xx;
};
typedef union cvmx_pko_dqx_bytes cvmx_pko_dqx_bytes_t;

/**
 * cvmx_pko_dq#_cir
 */
union cvmx_pko_dqx_cir {
	uint64_t u64;
	struct cvmx_pko_dqx_cir_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_41_63               : 23;
	uint64_t burst_exponent               : 4;  /**< Burst exponent. The burst limit is specified as 1.BURST_MANTISSA << BURST_EXPONENT. */
	uint64_t burst_mantissa               : 8;  /**< Burst mantissa. The burst limit is specified as 1.BURST_MANTISSA << BURST_EXPONENT. */
	uint64_t reserved_17_28               : 12;
	uint64_t rate_divider_exponent        : 4;  /**< Rate divider exponent. This 4-bit base-2 exponent is used to divide the credit rate by
                                                         specifying the number of time-wheel turns required before the accumulator is increased.
                                                         The rate count = (1 << RATE_COUNT_EXPONENT). The supported range for RATE_DIVIDER_EXPONENT
                                                         is 0 to 11. Programmed values greater than 11 are treated as 11. */
	uint64_t rate_exponent                : 4;  /**< Rate exponent. The rate is specified as 1.RATE_MANTISSA << RATE_EXPONENT. */
	uint64_t rate_mantissa                : 8;  /**< Rate mantissa. The rate is specified as 1.RATE_MANTISSA << RATE_EXPONENT. */
	uint64_t enable                       : 1;  /**< Enable. Enables CIR shaping. */
#else
	uint64_t enable                       : 1;
	uint64_t rate_mantissa                : 8;
	uint64_t rate_exponent                : 4;
	uint64_t rate_divider_exponent        : 4;
	uint64_t reserved_17_28               : 12;
	uint64_t burst_mantissa               : 8;
	uint64_t burst_exponent               : 4;
	uint64_t reserved_41_63               : 23;
#endif
	} s;
	struct cvmx_pko_dqx_cir_s             cn78xx;
};
typedef union cvmx_pko_dqx_cir cvmx_pko_dqx_cir_t;

/**
 * cvmx_pko_dq#_dropped_bytes
 */
union cvmx_pko_dqx_dropped_bytes {
	uint64_t u64;
	struct cvmx_pko_dqx_dropped_bytes_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t count                        : 48; /**< Count. The running count of bytes. Note that this count wraps. */
#else
	uint64_t count                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pko_dqx_dropped_bytes_s   cn78xx;
};
typedef union cvmx_pko_dqx_dropped_bytes cvmx_pko_dqx_dropped_bytes_t;

/**
 * cvmx_pko_dq#_dropped_packets
 */
union cvmx_pko_dqx_dropped_packets {
	uint64_t u64;
	struct cvmx_pko_dqx_dropped_packets_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t count                        : 40; /**< Count. The running count of packets. Note that this count wraps. */
#else
	uint64_t count                        : 40;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_pko_dqx_dropped_packets_s cn78xx;
};
typedef union cvmx_pko_dqx_dropped_packets cvmx_pko_dqx_dropped_packets_t;

/**
 * cvmx_pko_dq#_fifo
 */
union cvmx_pko_dqx_fifo {
	uint64_t u64;
	struct cvmx_pko_dqx_fifo_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t p_con                        : 1;  /**< Reserved. */
	uint64_t head                         : 7;  /**< See PKO_L2_SQ(0..511)_POINTERS[PREV]. */
	uint64_t tail                         : 7;  /**< See PKO_L2_SQ(0..511)_POINTERS[NEXT]. */
#else
	uint64_t tail                         : 7;
	uint64_t head                         : 7;
	uint64_t p_con                        : 1;
	uint64_t reserved_15_63               : 49;
#endif
	} s;
	struct cvmx_pko_dqx_fifo_s            cn78xx;
};
typedef union cvmx_pko_dqx_fifo cvmx_pko_dqx_fifo_t;

/**
 * cvmx_pko_dq#_packets
 */
union cvmx_pko_dqx_packets {
	uint64_t u64;
	struct cvmx_pko_dqx_packets_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t count                        : 40; /**< Count. The running count of packets. Note that this count wraps. */
#else
	uint64_t count                        : 40;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_pko_dqx_packets_s         cn78xx;
};
typedef union cvmx_pko_dqx_packets cvmx_pko_dqx_packets_t;

/**
 * cvmx_pko_dq#_pick
 */
union cvmx_pko_dqx_pick {
	uint64_t u64;
	struct cvmx_pko_dqx_pick_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dq                           : 10; /**< Descriptor queue. Index of originating descriptor queue. */
	uint64_t color                        : 2;  /**< See PKO_L2_SQ(0..511)_SHAPE[COLOR]. */
	uint64_t child                        : 10; /**< Child index. When the C_CON bit of this result is set, indicating that this
                                                         result is connected in a flow that extends through the child result, this is the
                                                         index of that child result. */
	uint64_t bubble                       : 1;  /**< This metapacket is a fake passed forward after a prune. */
	uint64_t p_con                        : 1;  /**< Parent connected flag. This pick has more picks in front of it. */
	uint64_t c_con                        : 1;  /**< Child connected flag. This pick has more picks behind it. */
	uint64_t uid                          : 7;  /**< Unique ID. 7-bit unique value assigned at the DQ level, increments for each packet. */
	uint64_t jump                         : 1;  /**< Jump. Set when metapacket originated from a jump descriptor. */
	uint64_t fpd                          : 1;  /**< First packet descriptor. Set when metapacket was the first in a cacheline. */
	uint64_t ds                           : 1;  /**< Don't send. Set when metapacket is not to be sent. */
	uint64_t adjust                       : 9;  /**< See PKO_L2_SQ(0..511)_SHAPE[ADJUST]. */
	uint64_t pir_dis                      : 1;  /**< PIR disable. Peak shaper disabled. */
	uint64_t cir_dis                      : 1;  /**< CIR disable. Committed shaper disabled. */
	uint64_t red_algo_override            : 2;  /**< See PKO_L2_SQ(0..511)_SHAPE[RED_ALGO]. */
	uint64_t length                       : 16; /**< Packet length. The packet length in bytes. */
#else
	uint64_t length                       : 16;
	uint64_t red_algo_override            : 2;
	uint64_t cir_dis                      : 1;
	uint64_t pir_dis                      : 1;
	uint64_t adjust                       : 9;
	uint64_t ds                           : 1;
	uint64_t fpd                          : 1;
	uint64_t jump                         : 1;
	uint64_t uid                          : 7;
	uint64_t c_con                        : 1;
	uint64_t p_con                        : 1;
	uint64_t bubble                       : 1;
	uint64_t child                        : 10;
	uint64_t color                        : 2;
	uint64_t dq                           : 10;
#endif
	} s;
	struct cvmx_pko_dqx_pick_s            cn78xx;
};
typedef union cvmx_pko_dqx_pick cvmx_pko_dqx_pick_t;

/**
 * cvmx_pko_dq#_pir
 */
union cvmx_pko_dqx_pir {
	uint64_t u64;
	struct cvmx_pko_dqx_pir_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_41_63               : 23;
	uint64_t burst_exponent               : 4;  /**< Burst exponent. The burst limit is specified as 1.BURST_MANTISSA << BURST_EXPONENT. */
	uint64_t burst_mantissa               : 8;  /**< Burst mantissa. The burst limit is specified as 1.BURST_MANTISSA << BURST_EXPONENT. */
	uint64_t reserved_17_28               : 12;
	uint64_t rate_divider_exponent        : 4;  /**< Rate divider exponent. This 4-bit base-2 exponent is used to divide the credit rate by
                                                         specifying the number of time-wheel turns required before the accumulator is increased.
                                                         The rate count = (1 << RATE_COUNT_EXPONENT). The supported range for RATE_DIVIDER_EXPONENT
                                                         is 0 to 11. Programmed values greater than 11 are treated as 11. */
	uint64_t rate_exponent                : 4;  /**< Rate exponent. The rate is specified as 1.RATE_MANTISSA << RATE_EXPONENT. */
	uint64_t rate_mantissa                : 8;  /**< Rate mantissa. The rate is specified as 1.RATE_MANTISSA << RATE_EXPONENT. */
	uint64_t enable                       : 1;  /**< Enable. Enables CIR shaping. */
#else
	uint64_t enable                       : 1;
	uint64_t rate_mantissa                : 8;
	uint64_t rate_exponent                : 4;
	uint64_t rate_divider_exponent        : 4;
	uint64_t reserved_17_28               : 12;
	uint64_t burst_mantissa               : 8;
	uint64_t burst_exponent               : 4;
	uint64_t reserved_41_63               : 23;
#endif
	} s;
	struct cvmx_pko_dqx_pir_s             cn78xx;
};
typedef union cvmx_pko_dqx_pir cvmx_pko_dqx_pir_t;

/**
 * cvmx_pko_dq#_pointers
 */
union cvmx_pko_dqx_pointers {
	uint64_t u64;
	struct cvmx_pko_dqx_pointers_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_26_63               : 38;
	uint64_t prev                         : 10; /**< See PKO_L2_SQ(0..511)_POINTERS[PREV]. */
	uint64_t reserved_10_15               : 6;
	uint64_t next                         : 10; /**< See PKO_L2_SQ(0..511)_POINTERS[NEXT]. */
#else
	uint64_t next                         : 10;
	uint64_t reserved_10_15               : 6;
	uint64_t prev                         : 10;
	uint64_t reserved_26_63               : 38;
#endif
	} s;
	struct cvmx_pko_dqx_pointers_s        cn78xx;
};
typedef union cvmx_pko_dqx_pointers cvmx_pko_dqx_pointers_t;

/**
 * cvmx_pko_dq#_sched_state
 */
union cvmx_pko_dqx_sched_state {
	uint64_t u64;
	struct cvmx_pko_dqx_sched_state_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t rr_count                     : 25; /**< Round-robin (DWRR) deficit counter. A 25-bit signed integer count. For diagnostic use. */
#else
	uint64_t rr_count                     : 25;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_pko_dqx_sched_state_s     cn78xx;
};
typedef union cvmx_pko_dqx_sched_state cvmx_pko_dqx_sched_state_t;

/**
 * cvmx_pko_dq#_schedule
 */
union cvmx_pko_dqx_schedule {
	uint64_t u64;
	struct cvmx_pko_dqx_schedule_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t prio                         : 4;  /**< Priority. The priority used for this SQ in the (lower-level) parent's scheduling
                                                         algorithm. When this SQ is not used, we recommend setting PRIO to zero. The legal PRIO
                                                         values are 0-9 when the SQ is used. In addition to priority, PRIO determines whether the
                                                         SQ is a static queue or not: If PRIO equals PKO_*_SQn_TOPOLOGY[RR_PRIO], where
                                                         PKO_*_TOPOLOGY[PARENT] for this SQ equals n, then this is a round-robin child queue into
                                                         the shaper at the next level. */
	uint64_t rr_quantum                   : 24; /**< Round-robin (DWRR) quantum. The deficit-weighted round-robin quantum (24-bit unsigned integer). */
#else
	uint64_t rr_quantum                   : 24;
	uint64_t prio                         : 4;
	uint64_t reserved_28_63               : 36;
#endif
	} s;
	struct cvmx_pko_dqx_schedule_s        cn78xx;
};
typedef union cvmx_pko_dqx_schedule cvmx_pko_dqx_schedule_t;

/**
 * cvmx_pko_dq#_shape
 */
union cvmx_pko_dqx_shape {
	uint64_t u64;
	struct cvmx_pko_dqx_shape_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t length_disable               : 1;  /**< Length disable. Disables the use of packet lengths in shaping calculations such that only
                                                         the value of PKO_L5_SQ(0..1023)_SHAPE[ADJUST]. */
	uint64_t reserved_13_23               : 11;
	uint64_t yellow_disable               : 1;  /**< Disable yellow transitions. Disables green-to-yellow packet color marking transitions when set. */
	uint64_t red_disable                  : 1;  /**< Disable red transitions. Disables green-to-red and yellow-to-red packet color marking
                                                         transitions when set. */
	uint64_t red_algo                     : 2;  /**< See PKO_L2_SQ(0..511)_SHAPE[RED_ALGO]. */
	uint64_t adjust                       : 9;  /**< See PKO_L2_SQ(0..511)_SHAPE[ADJUST]. */
#else
	uint64_t adjust                       : 9;
	uint64_t red_algo                     : 2;
	uint64_t red_disable                  : 1;
	uint64_t yellow_disable               : 1;
	uint64_t reserved_13_23               : 11;
	uint64_t length_disable               : 1;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_pko_dqx_shape_s           cn78xx;
};
typedef union cvmx_pko_dqx_shape cvmx_pko_dqx_shape_t;

/**
 * cvmx_pko_dq#_shape_state
 */
union cvmx_pko_dqx_shape_state {
	uint64_t u64;
	struct cvmx_pko_dqx_shape_state_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t tw_timestamp                 : 6;  /**< Time-wheel timestamp. Debug access to the live time-wheel timestamp. */
	uint64_t color                        : 2;  /**< Shaper color status. Debug access to the live shaper state.
                                                         0 = Green - operating in 'committed' range
                                                         1 = Yellow - operating in 'excess/peak' range
                                                         2 = Red - operating in 'oversubscribed' range
                                                         3 = Reserved */
	uint64_t pir_accum                    : 26; /**< Peak information rate accumulator. Debug access to the live PIR accumulator. */
	uint64_t cir_accum                    : 26; /**< Committed information rate accumulator. Debug access to the live CIR accumulator. */
#else
	uint64_t cir_accum                    : 26;
	uint64_t pir_accum                    : 26;
	uint64_t color                        : 2;
	uint64_t tw_timestamp                 : 6;
	uint64_t reserved_60_63               : 4;
#endif
	} s;
	struct cvmx_pko_dqx_shape_state_s     cn78xx;
};
typedef union cvmx_pko_dqx_shape_state cvmx_pko_dqx_shape_state_t;

/**
 * cvmx_pko_dq#_sw_xoff
 */
union cvmx_pko_dqx_sw_xoff {
	uint64_t u64;
	struct cvmx_pko_dqx_sw_xoff_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t drain_irq                    : 1;  /**< Drain IRQ. Enables an interrupt that fires when the drain operation has completed. */
	uint64_t drain_null_link              : 1;  /**< "Drain null link. Conditions the drain path to drain through the null link (i.e. link
                                                         \#28). As such, channel credits, HW_XOFF, and shaping are disabled on the draining path
                                                         until the path has drained." */
	uint64_t drain                        : 1;  /**< Drain. This control activates a drain path through the PSE that starts at this node and
                                                         ends at the SQ1 level. The drain path is prioritized over other paths through PSE and can
                                                         be used in combination with DRAIN_NULL_LINK and DRAIN_IRQ. */
	uint64_t xoff                         : 1;  /**< XOFF. The PQ is disabled when XOFF is asserted. PQ is enabled when XOFF is de-asserted.
                                                         NOTE: The associated PKO_L1_SQ(0..31)_TOPOLOGY[LINK] must be configured before using this
                                                         register field. Writing to this register field before the associated
                                                         PKO_L1_SQ(0..31)_TOPOLOGY[LINK] value is configure can result in modifying the software
                                                         XOFF state of the wrong SQ. */
#else
	uint64_t xoff                         : 1;
	uint64_t drain                        : 1;
	uint64_t drain_null_link              : 1;
	uint64_t drain_irq                    : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_pko_dqx_sw_xoff_s         cn78xx;
};
typedef union cvmx_pko_dqx_sw_xoff cvmx_pko_dqx_sw_xoff_t;

/**
 * cvmx_pko_dq#_topology
 */
union cvmx_pko_dqx_topology {
	uint64_t u64;
	struct cvmx_pko_dqx_topology_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_26_63               : 38;
	uint64_t parent                       : 10; /**< See PKO_L2_SQ(0..511)_TOPOLOGY[PARENT]. */
	uint64_t reserved_0_15                : 16;
#else
	uint64_t reserved_0_15                : 16;
	uint64_t parent                       : 10;
	uint64_t reserved_26_63               : 38;
#endif
	} s;
	struct cvmx_pko_dqx_topology_s        cn78xx;
};
typedef union cvmx_pko_dqx_topology cvmx_pko_dqx_topology_t;

/**
 * cvmx_pko_dq#_wm_cnt
 */
union cvmx_pko_dqx_wm_cnt {
	uint64_t u64;
	struct cvmx_pko_dqx_wm_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t count                        : 48; /**< Watermark count. The running value of the watermark counter. This value is a count of
                                                         bytes or packets as specified in PKO_DQ(0..1023)_WM_CONTROL[KIND]. */
#else
	uint64_t count                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pko_dqx_wm_cnt_s          cn78xx;
};
typedef union cvmx_pko_dqx_wm_cnt cvmx_pko_dqx_wm_cnt_t;

/**
 * cvmx_pko_dq#_wm_ctl
 */
union cvmx_pko_dqx_wm_ctl {
	uint64_t u64;
	struct cvmx_pko_dqx_wm_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_51_63               : 13;
	uint64_t enable                       : 1;  /**< Watermark enable. */
	uint64_t kind                         : 1;  /**< Watermark kind. The watermark logic can use a byte count or packet count.
                                                         0 = Byte count
                                                         1 = Packet count */
	uint64_t intr                         : 1;  /**< Watermark Interrupt. The interrupt bit is asserted and an interrupt message to the CIU is
                                                         generated when the specified threshold is reached or crossed. Subsequent interrupt
                                                         messages are only generated after this bit has been cleared. */
	uint64_t threshold                    : 48; /**< Watermark Threshold. This threshold is compared to the watermark count of
                                                         PKO_DQ(0..1023)_WM_COUNT[COUNT] and an interrupt is generated when the count reaches or
                                                         crosses the threshold. */
#else
	uint64_t threshold                    : 48;
	uint64_t intr                         : 1;
	uint64_t kind                         : 1;
	uint64_t enable                       : 1;
	uint64_t reserved_51_63               : 13;
#endif
	} s;
	struct cvmx_pko_dqx_wm_ctl_s          cn78xx;
};
typedef union cvmx_pko_dqx_wm_ctl cvmx_pko_dqx_wm_ctl_t;

/**
 * cvmx_pko_dq#_wm_ctl_w1c
 */
union cvmx_pko_dqx_wm_ctl_w1c {
	uint64_t u64;
	struct cvmx_pko_dqx_wm_ctl_w1c_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_49_63               : 15;
	uint64_t intr                         : 1;  /**< Interrupt. The interrupt bit is asserted and an interrupt message to the CIU is generated
                                                         when the specified threshold is crossed. Subsequent interrupt messages are only generated
                                                         after this bit has been cleared by writing 1. Throws PKO_INTSN_E::PKO_DQ(0..1023)_WM. */
	uint64_t reserved_0_47                : 48;
#else
	uint64_t reserved_0_47                : 48;
	uint64_t intr                         : 1;
	uint64_t reserved_49_63               : 15;
#endif
	} s;
	struct cvmx_pko_dqx_wm_ctl_w1c_s      cn78xx;
};
typedef union cvmx_pko_dqx_wm_ctl_w1c cvmx_pko_dqx_wm_ctl_w1c_t;

/**
 * cvmx_pko_dq_csr_bus_debug
 */
union cvmx_pko_dq_csr_bus_debug {
	uint64_t u64;
	struct cvmx_pko_dq_csr_bus_debug_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t csr_bus_debug                : 64; /**< N/A */
#else
	uint64_t csr_bus_debug                : 64;
#endif
	} s;
	struct cvmx_pko_dq_csr_bus_debug_s    cn78xx;
};
typedef union cvmx_pko_dq_csr_bus_debug cvmx_pko_dq_csr_bus_debug_t;

/**
 * cvmx_pko_dq_debug
 */
union cvmx_pko_dq_debug {
	uint64_t u64;
	struct cvmx_pko_dq_debug_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dbg_vec                      : 64; /**< Debug Vector. */
#else
	uint64_t dbg_vec                      : 64;
#endif
	} s;
	struct cvmx_pko_dq_debug_s            cn78xx;
};
typedef union cvmx_pko_dq_debug cvmx_pko_dq_debug_t;

/**
 * cvmx_pko_drain_irq
 */
union cvmx_pko_drain_irq {
	uint64_t u64;
	struct cvmx_pko_drain_irq_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t intr                         : 1;  /**< Interrupt. The interrupt bit is asserted and an interrupt message to the CIU is generated
                                                         when the DRAIN command reaches the PQ level. Subsequent interrupt messages are only
                                                         generated
                                                         after this bit has been cleared by writing 1. Throws PKO_INTSN_E::PKO_PSE_PQ_DRAIN. */
#else
	uint64_t intr                         : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_pko_drain_irq_s           cn78xx;
};
typedef union cvmx_pko_drain_irq cvmx_pko_drain_irq_t;

/**
 * cvmx_pko_enable
 */
union cvmx_pko_enable {
	uint64_t u64;
	struct cvmx_pko_enable_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t enable                       : 1;  /**< Enables the PKO. */
#else
	uint64_t enable                       : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_pko_enable_s              cn78xx;
};
typedef union cvmx_pko_enable cvmx_pko_enable_t;

/**
 * cvmx_pko_format#_ctl
 */
union cvmx_pko_formatx_ctl {
	uint64_t u64;
	struct cvmx_pko_formatx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t ip4_ck                       : 1;  /**< IPv4 header checksum recalculate */
	uint64_t offset                       : 11; /**< Bits to add to PKO_SEND_EXT[MARKPTR]*8 to determine where to start marking. */
	uint64_t y_mask                       : 4;  /**< Yellow mark mask. Corresponding bits in packet's data are cleared when marking packet yellow. */
	uint64_t y_val                        : 4;  /**< Yellow mark value. Corresponding bits in packet's data are set when marking packet yellow. */
	uint64_t r_mask                       : 4;  /**< Red mark mask. Corresponding bits in packet's data are cleared when marking packet red. */
	uint64_t r_val                        : 4;  /**< Red mark value. Corresponding bits in packet's data are set when marking packet red. */
#else
	uint64_t r_val                        : 4;
	uint64_t r_mask                       : 4;
	uint64_t y_val                        : 4;
	uint64_t y_mask                       : 4;
	uint64_t offset                       : 11;
	uint64_t ip4_ck                       : 1;
	uint64_t reserved_28_63               : 36;
#endif
	} s;
	struct cvmx_pko_formatx_ctl_s         cn78xx;
};
typedef union cvmx_pko_formatx_ctl cvmx_pko_formatx_ctl_t;

/**
 * cvmx_pko_l1_sq#_cir
 */
union cvmx_pko_l1_sqx_cir {
	uint64_t u64;
	struct cvmx_pko_l1_sqx_cir_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_41_63               : 23;
	uint64_t burst_exponent               : 4;  /**< Burst exponent. The burst limit is specified as 1.BURST_MANTISSA << BURST_EXPONENT. */
	uint64_t burst_mantissa               : 8;  /**< Burst mantissa. The burst limit is specified as 1.BURST_MANTISSA << BURST_EXPONENT. */
	uint64_t reserved_17_28               : 12;
	uint64_t rate_divider_exponent        : 4;  /**< Rate divider exponent. This 4-bit base-2 exponent is used to divide the credit rate by
                                                         specifying the number of time-wheel turns required before the accumulator is increased.
                                                         The rate count = (1 << RATE_COUNT_EXPONENT). The supported range for RATE_DIVIDER_EXPONENT
                                                         is 0 to 11. Programmed values greater than 11 are treated as 11. */
	uint64_t rate_exponent                : 4;  /**< Rate exponent. The rate is specified as 1.RATE_MANTISSA << RATE_EXPONENT. */
	uint64_t rate_mantissa                : 8;  /**< Rate mantissa. The rate is specified as 1.RATE_MANTISSA << RATE_EXPONENT. */
	uint64_t enable                       : 1;  /**< Enable. Enables CIR shaping. */
#else
	uint64_t enable                       : 1;
	uint64_t rate_mantissa                : 8;
	uint64_t rate_exponent                : 4;
	uint64_t rate_divider_exponent        : 4;
	uint64_t reserved_17_28               : 12;
	uint64_t burst_mantissa               : 8;
	uint64_t burst_exponent               : 4;
	uint64_t reserved_41_63               : 23;
#endif
	} s;
	struct cvmx_pko_l1_sqx_cir_s          cn78xx;
};
typedef union cvmx_pko_l1_sqx_cir cvmx_pko_l1_sqx_cir_t;

/**
 * cvmx_pko_l1_sq#_dropped_bytes
 */
union cvmx_pko_l1_sqx_dropped_bytes {
	uint64_t u64;
	struct cvmx_pko_l1_sqx_dropped_bytes_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t count                        : 48; /**< Count. The running count of bytes. Note that this count wraps. */
#else
	uint64_t count                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pko_l1_sqx_dropped_bytes_s cn78xx;
};
typedef union cvmx_pko_l1_sqx_dropped_bytes cvmx_pko_l1_sqx_dropped_bytes_t;

/**
 * cvmx_pko_l1_sq#_dropped_packets
 */
union cvmx_pko_l1_sqx_dropped_packets {
	uint64_t u64;
	struct cvmx_pko_l1_sqx_dropped_packets_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t count                        : 40; /**< Count. The running count of packets. Note that this count wraps. */
#else
	uint64_t count                        : 40;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_pko_l1_sqx_dropped_packets_s cn78xx;
};
typedef union cvmx_pko_l1_sqx_dropped_packets cvmx_pko_l1_sqx_dropped_packets_t;

/**
 * cvmx_pko_l1_sq#_green
 */
union cvmx_pko_l1_sqx_green {
	uint64_t u64;
	struct cvmx_pko_l1_sqx_green_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_41_63               : 23;
	uint64_t rr_active                    : 1;  /**< Round-robin red active. Indicates that the round-robin input is mapped to RED. */
	uint64_t active_vec                   : 20; /**< Active vector. A 10-bit vector, ordered by priority, that indicate which inputs to this
                                                         scheduling queue are active. For internal use only. */
	uint64_t reserved_19_19               : 1;
	uint64_t head                         : 9;  /**< Head pointer. The index of round-robin linked-list head. For internal use only. */
	uint64_t reserved_9_9                 : 1;
	uint64_t tail                         : 9;  /**< Tail pointer. The index of round-robin linked-list tail. For internal use only. */
#else
	uint64_t tail                         : 9;
	uint64_t reserved_9_9                 : 1;
	uint64_t head                         : 9;
	uint64_t reserved_19_19               : 1;
	uint64_t active_vec                   : 20;
	uint64_t rr_active                    : 1;
	uint64_t reserved_41_63               : 23;
#endif
	} s;
	struct cvmx_pko_l1_sqx_green_s        cn78xx;
};
typedef union cvmx_pko_l1_sqx_green cvmx_pko_l1_sqx_green_t;

/**
 * cvmx_pko_l1_sq#_green_bytes
 */
union cvmx_pko_l1_sqx_green_bytes {
	uint64_t u64;
	struct cvmx_pko_l1_sqx_green_bytes_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t count                        : 48; /**< Count. The running count of bytes. Note that this count wraps. */
#else
	uint64_t count                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pko_l1_sqx_green_bytes_s  cn78xx;
};
typedef union cvmx_pko_l1_sqx_green_bytes cvmx_pko_l1_sqx_green_bytes_t;

/**
 * cvmx_pko_l1_sq#_green_packets
 */
union cvmx_pko_l1_sqx_green_packets {
	uint64_t u64;
	struct cvmx_pko_l1_sqx_green_packets_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t count                        : 40; /**< Count. The running count of packets. Note that this count wraps. */
#else
	uint64_t count                        : 40;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_pko_l1_sqx_green_packets_s cn78xx;
};
typedef union cvmx_pko_l1_sqx_green_packets cvmx_pko_l1_sqx_green_packets_t;

/**
 * cvmx_pko_l1_sq#_link
 */
union cvmx_pko_l1_sqx_link {
	uint64_t u64;
	struct cvmx_pko_l1_sqx_link_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_49_63               : 15;
	uint64_t link                         : 5;  /**< Link index. Must match PKO_L1_SQ(0..31)_TOPOLOGY[LINK]. */
	uint64_t reserved_32_43               : 12;
	uint64_t cc_word_cnt                  : 20; /**< Channel credit word count.
                                                         This value, plus 1 MTU, represents the maximum outstanding aggregate word count (words are
                                                         16 bytes) for all channels feeding into this PQ.
                                                         Note that this 20-bit field represents a signed value that decrements towards zero as
                                                         credits are used. Packets are not allowed to flow when the count is less than zero. As
                                                         such, the most significant bit should normally be programmed as zero (positive count).
                                                         This gives a maximum value for this field of 2^19 - 1. */
	uint64_t cc_packet_cnt                : 10; /**< Channel credit packet count. This value, plus 1, represents the maximum outstanding
                                                         aggregate packet count for all channels feeding into this PQ.
                                                         Note that this 10-bit field represents a signed value that decrements towards zero as
                                                         credits are used. Packets are not allowed to flow when the count is less than zero. As
                                                         such the most significant bit should normally be programmed as zero (positive count). This
                                                         gives a maximum value for this field of 2^9 - 1. */
	uint64_t cc_enable                    : 1;  /**< Channel credit enable. Enables CC_WORD_CNT and CC_PACKET_CNT aggregate credit processing. */
	uint64_t reserved_0_0                 : 1;
#else
	uint64_t reserved_0_0                 : 1;
	uint64_t cc_enable                    : 1;
	uint64_t cc_packet_cnt                : 10;
	uint64_t cc_word_cnt                  : 20;
	uint64_t reserved_32_43               : 12;
	uint64_t link                         : 5;
	uint64_t reserved_49_63               : 15;
#endif
	} s;
	struct cvmx_pko_l1_sqx_link_s         cn78xx;
};
typedef union cvmx_pko_l1_sqx_link cvmx_pko_l1_sqx_link_t;

/**
 * cvmx_pko_l1_sq#_pick
 */
union cvmx_pko_l1_sqx_pick {
	uint64_t u64;
	struct cvmx_pko_l1_sqx_pick_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dq                           : 10; /**< Descriptor queue. Index of originating descriptor queue. */
	uint64_t color                        : 2;  /**< See PKO_L2_SQ(0..511)_SHAPE[COLOR]. */
	uint64_t child                        : 10; /**< Child index. When the C_CON bit of this result is set, indicating that this
                                                         result is connected in a flow that extends through the child result, this is the
                                                         index of that child result. */
	uint64_t bubble                       : 1;  /**< This metapacket is a fake passed forward after a prune. */
	uint64_t p_con                        : 1;  /**< Parent connected flag. This pick has more picks in front of it. */
	uint64_t c_con                        : 1;  /**< Child connected flag. This pick has more picks behind it. */
	uint64_t uid                          : 7;  /**< Unique ID. 7-bit unique value assigned at the DQ level, increments for each packet. */
	uint64_t jump                         : 1;  /**< Jump. Set when metapacket originated from a jump descriptor. */
	uint64_t fpd                          : 1;  /**< First packet descriptor. Set when metapacket was the first in a cacheline. */
	uint64_t ds                           : 1;  /**< Don't send. Set when metapacket is not to be sent. */
	uint64_t adjust                       : 9;  /**< See PKO_L2_SQ(0..511)_SHAPE[ADJUST]. */
	uint64_t pir_dis                      : 1;  /**< PIR disable. Peak shaper disabled. */
	uint64_t cir_dis                      : 1;  /**< CIR disable. Committed shaper disabled. */
	uint64_t red_algo_override            : 2;  /**< See PKO_L2_SQ(0..511)_SHAPE[RED_ALGO]. */
	uint64_t length                       : 16; /**< Packet length. The packet length in bytes. */
#else
	uint64_t length                       : 16;
	uint64_t red_algo_override            : 2;
	uint64_t cir_dis                      : 1;
	uint64_t pir_dis                      : 1;
	uint64_t adjust                       : 9;
	uint64_t ds                           : 1;
	uint64_t fpd                          : 1;
	uint64_t jump                         : 1;
	uint64_t uid                          : 7;
	uint64_t c_con                        : 1;
	uint64_t p_con                        : 1;
	uint64_t bubble                       : 1;
	uint64_t child                        : 10;
	uint64_t color                        : 2;
	uint64_t dq                           : 10;
#endif
	} s;
	struct cvmx_pko_l1_sqx_pick_s         cn78xx;
};
typedef union cvmx_pko_l1_sqx_pick cvmx_pko_l1_sqx_pick_t;

/**
 * cvmx_pko_l1_sq#_pir
 */
union cvmx_pko_l1_sqx_pir {
	uint64_t u64;
	struct cvmx_pko_l1_sqx_pir_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_41_63               : 23;
	uint64_t burst_exponent               : 4;  /**< Burst exponent. The burst limit is specified as 1.BURST_MANTISSA << BURST_EXPONENT. */
	uint64_t burst_mantissa               : 8;  /**< Burst mantissa. The burst limit is specified as 1.BURST_MANTISSA << BURST_EXPONENT. */
	uint64_t reserved_17_28               : 12;
	uint64_t rate_divider_exponent        : 4;  /**< Rate divider exponent. This 4-bit base-2 exponent is used to divide the credit rate by
                                                         specifying the number of time-wheel turns required before the accumulator is increased.
                                                         The rate count = (1 << RATE_COUNT_EXPONENT). The supported range for RATE_DIVIDER_EXPONENT
                                                         is 0 to 11. Programmed values greater than 11 are treated as 11. */
	uint64_t rate_exponent                : 4;  /**< Rate exponent. The rate is specified as 1.RATE_MANTISSA << RATE_EXPONENT. */
	uint64_t rate_mantissa                : 8;  /**< Rate mantissa. The rate is specified as 1.RATE_MANTISSA << RATE_EXPONENT. */
	uint64_t enable                       : 1;  /**< Enable. Enables CIR shaping. */
#else
	uint64_t enable                       : 1;
	uint64_t rate_mantissa                : 8;
	uint64_t rate_exponent                : 4;
	uint64_t rate_divider_exponent        : 4;
	uint64_t reserved_17_28               : 12;
	uint64_t burst_mantissa               : 8;
	uint64_t burst_exponent               : 4;
	uint64_t reserved_41_63               : 23;
#endif
	} s;
	struct cvmx_pko_l1_sqx_pir_s          cn78xx;
};
typedef union cvmx_pko_l1_sqx_pir cvmx_pko_l1_sqx_pir_t;

/**
 * cvmx_pko_l1_sq#_red
 */
union cvmx_pko_l1_sqx_red {
	uint64_t u64;
	struct cvmx_pko_l1_sqx_red_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_19_63               : 45;
	uint64_t head                         : 9;  /**< Head pointer. The index of round-robin linked-list head. For internal use only. */
	uint64_t reserved_9_9                 : 1;
	uint64_t tail                         : 9;  /**< Tail pointer. The index of round-robin linked-list tail. For internal use only. */
#else
	uint64_t tail                         : 9;
	uint64_t reserved_9_9                 : 1;
	uint64_t head                         : 9;
	uint64_t reserved_19_63               : 45;
#endif
	} s;
	struct cvmx_pko_l1_sqx_red_s          cn78xx;
};
typedef union cvmx_pko_l1_sqx_red cvmx_pko_l1_sqx_red_t;

/**
 * cvmx_pko_l1_sq#_red_bytes
 */
union cvmx_pko_l1_sqx_red_bytes {
	uint64_t u64;
	struct cvmx_pko_l1_sqx_red_bytes_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t count                        : 48; /**< Count. The running count of bytes. Note that this count wraps. */
#else
	uint64_t count                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pko_l1_sqx_red_bytes_s    cn78xx;
};
typedef union cvmx_pko_l1_sqx_red_bytes cvmx_pko_l1_sqx_red_bytes_t;

/**
 * cvmx_pko_l1_sq#_red_packets
 */
union cvmx_pko_l1_sqx_red_packets {
	uint64_t u64;
	struct cvmx_pko_l1_sqx_red_packets_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t count                        : 40; /**< Count. The running count of packets. Note that this count wraps. */
#else
	uint64_t count                        : 40;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_pko_l1_sqx_red_packets_s  cn78xx;
};
typedef union cvmx_pko_l1_sqx_red_packets cvmx_pko_l1_sqx_red_packets_t;

/**
 * cvmx_pko_l1_sq#_shape
 */
union cvmx_pko_l1_sqx_shape {
	uint64_t u64;
	struct cvmx_pko_l1_sqx_shape_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_17_63               : 47;
	uint64_t link                         : 4;  /**< Link index. Must match PKO_L1_SQ(0..31)_TOPOLOGY[LINK]. */
	uint64_t reserved_0_12                : 13;
#else
	uint64_t reserved_0_12                : 13;
	uint64_t link                         : 4;
	uint64_t reserved_17_63               : 47;
#endif
	} s;
	struct cvmx_pko_l1_sqx_shape_s        cn78xx;
};
typedef union cvmx_pko_l1_sqx_shape cvmx_pko_l1_sqx_shape_t;

/**
 * cvmx_pko_l1_sq#_shape_state
 */
union cvmx_pko_l1_sqx_shape_state {
	uint64_t u64;
	struct cvmx_pko_l1_sqx_shape_state_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t tw_timestamp                 : 6;  /**< Time-wheel timestamp. Debug access to the live time-wheel timestamp. */
	uint64_t color                        : 2;  /**< Shaper color status. Debug access to the live shaper state.
                                                         0 = Green - operating in 'committed' range
                                                         1 = Yellow - operating in 'excess/peak' range
                                                         2 = Red - operating in 'oversubscribed' range
                                                         3 = Reserved */
	uint64_t pir_accum                    : 26; /**< Peak information rate accumulator. Debug access to the live PIR accumulator. */
	uint64_t cir_accum                    : 26; /**< Committed information rate accumulator. Debug access to the live CIR accumulator. */
#else
	uint64_t cir_accum                    : 26;
	uint64_t pir_accum                    : 26;
	uint64_t color                        : 2;
	uint64_t tw_timestamp                 : 6;
	uint64_t reserved_60_63               : 4;
#endif
	} s;
	struct cvmx_pko_l1_sqx_shape_state_s  cn78xx;
};
typedef union cvmx_pko_l1_sqx_shape_state cvmx_pko_l1_sqx_shape_state_t;

/**
 * cvmx_pko_l1_sq#_sw_xoff
 */
union cvmx_pko_l1_sqx_sw_xoff {
	uint64_t u64;
	struct cvmx_pko_l1_sqx_sw_xoff_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t drain_irq                    : 1;  /**< Drain IRQ. Enables an interrupt that fires when the drain operation has completed. */
	uint64_t drain_null_link              : 1;  /**< "Drain null link. Conditions the drain path to drain through the null link (i.e. link
                                                         \#28). As such, channel credits, HW_XOFF, and shaping are disabled on the draining path
                                                         until the path has drained." */
	uint64_t drain                        : 1;  /**< Drain. This control activates a drain path through the PSE that starts at this node and
                                                         ends at the SQ1 level. The drain path is prioritized over other paths through PSE and can
                                                         be used in combination with DRAIN_NULL_LINK and DRAIN_IRQ. */
	uint64_t xoff                         : 1;  /**< XOFF. The PQ is disabled when XOFF is asserted. PQ is enabled when XOFF is de-asserted.
                                                         NOTE: The associated PKO_L1_SQ(0..31)_TOPOLOGY[LINK] must be configured before using this
                                                         register field. Writing to this register field before the associated
                                                         PKO_L1_SQ(0..31)_TOPOLOGY[LINK] value is configure can result in modifying the software
                                                         XOFF state of the wrong SQ. */
#else
	uint64_t xoff                         : 1;
	uint64_t drain                        : 1;
	uint64_t drain_null_link              : 1;
	uint64_t drain_irq                    : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_pko_l1_sqx_sw_xoff_s      cn78xx;
};
typedef union cvmx_pko_l1_sqx_sw_xoff cvmx_pko_l1_sqx_sw_xoff_t;

/**
 * cvmx_pko_l1_sq#_topology
 */
union cvmx_pko_l1_sqx_topology {
	uint64_t u64;
	struct cvmx_pko_l1_sqx_topology_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_41_63               : 23;
	uint64_t prio_anchor                  : 9;  /**< Priority Anchor. The base index positioning the static priority child
                                                         queues of this shaper. A higher-level queue is a child queue of this
                                                         shaper when its PKO_*_TOPOLOGY[PARENT] selects this shaper, and it
                                                         further is a static priority child queue when its
                                                         PKO_*_SQn_SCHEDULE[PRIO] does not equal RR_PRIO.
                                                         A static priority child queue with priority PRIO must be located at
                                                         n=PRIO_ANCHOR+PRIO, where PRIO=PKO_*_SQn_SCHEDULE[PRIO].
                                                         There can be at most one static priority child queue at each
                                                         priority. When there are no static priority child queues attached
                                                         at any priority, or if this shaper isn't used, the hardware does
                                                         not use PRIO_ANCHOR. In this case, we recommend PRIO_ANCHOR be
                                                         zero. */
	uint64_t reserved_21_31               : 11;
	uint64_t link                         : 5;  /**< Link index. Index of the link associated with this port queue. */
	uint64_t reserved_5_15                : 11;
	uint64_t rr_prio                      : 4;  /**< Round-robin priority. The priority assigned to the round-robin scheduler.
                                                         A higher-level queue is a child queue of this shaper when its
                                                         PKO_*_TOPOLOGY[PARENT] selects this shaper, and it further is a
                                                         round robin child queue when its PKO_*_SQn_SCHEDULE[PRIO] equals
                                                         RR_PRIO. All round-robin queues attached to this shaper must
                                                         have the same priority. But the number of round-robin child queues
                                                         attached (at this priority) is limited only by the number of
                                                         higher-level queues. When this shaper is not used, we recommend
                                                         RR_PRIO be zero. When a shaper is used, RR_PRIO should be 0xF
                                                         when there are no priorities with more than one child
                                                         queue (i.e. when there are no round-robin child queues), and
                                                         should otherwise be a legal priority (values 0-9). */
	uint64_t reserved_0_0                 : 1;
#else
	uint64_t reserved_0_0                 : 1;
	uint64_t rr_prio                      : 4;
	uint64_t reserved_5_15                : 11;
	uint64_t link                         : 5;
	uint64_t reserved_21_31               : 11;
	uint64_t prio_anchor                  : 9;
	uint64_t reserved_41_63               : 23;
#endif
	} s;
	struct cvmx_pko_l1_sqx_topology_s     cn78xx;
};
typedef union cvmx_pko_l1_sqx_topology cvmx_pko_l1_sqx_topology_t;

/**
 * cvmx_pko_l1_sq#_yellow
 */
union cvmx_pko_l1_sqx_yellow {
	uint64_t u64;
	struct cvmx_pko_l1_sqx_yellow_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_19_63               : 45;
	uint64_t head                         : 9;  /**< Head pointer. The index of round-robin linked-list head. For internal use only. */
	uint64_t reserved_9_9                 : 1;
	uint64_t tail                         : 9;  /**< Tail pointer. The index of round-robin linked-list tail. For internal use only. */
#else
	uint64_t tail                         : 9;
	uint64_t reserved_9_9                 : 1;
	uint64_t head                         : 9;
	uint64_t reserved_19_63               : 45;
#endif
	} s;
	struct cvmx_pko_l1_sqx_yellow_s       cn78xx;
};
typedef union cvmx_pko_l1_sqx_yellow cvmx_pko_l1_sqx_yellow_t;

/**
 * cvmx_pko_l1_sq#_yellow_bytes
 */
union cvmx_pko_l1_sqx_yellow_bytes {
	uint64_t u64;
	struct cvmx_pko_l1_sqx_yellow_bytes_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t count                        : 48; /**< Count. The running count of bytes. Note that this count wraps. */
#else
	uint64_t count                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pko_l1_sqx_yellow_bytes_s cn78xx;
};
typedef union cvmx_pko_l1_sqx_yellow_bytes cvmx_pko_l1_sqx_yellow_bytes_t;

/**
 * cvmx_pko_l1_sq#_yellow_packets
 */
union cvmx_pko_l1_sqx_yellow_packets {
	uint64_t u64;
	struct cvmx_pko_l1_sqx_yellow_packets_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t count                        : 40; /**< Count. The running count of packets. Note that this count wraps. */
#else
	uint64_t count                        : 40;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_pko_l1_sqx_yellow_packets_s cn78xx;
};
typedef union cvmx_pko_l1_sqx_yellow_packets cvmx_pko_l1_sqx_yellow_packets_t;

/**
 * cvmx_pko_l1_sq_csr_bus_debug
 */
union cvmx_pko_l1_sq_csr_bus_debug {
	uint64_t u64;
	struct cvmx_pko_l1_sq_csr_bus_debug_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t csr_bus_debug                : 64; /**< N/A */
#else
	uint64_t csr_bus_debug                : 64;
#endif
	} s;
	struct cvmx_pko_l1_sq_csr_bus_debug_s cn78xx;
};
typedef union cvmx_pko_l1_sq_csr_bus_debug cvmx_pko_l1_sq_csr_bus_debug_t;

/**
 * cvmx_pko_l1_sqa_debug
 */
union cvmx_pko_l1_sqa_debug {
	uint64_t u64;
	struct cvmx_pko_l1_sqa_debug_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dbg_vec                      : 64; /**< Debug Vector. */
#else
	uint64_t dbg_vec                      : 64;
#endif
	} s;
	struct cvmx_pko_l1_sqa_debug_s        cn78xx;
};
typedef union cvmx_pko_l1_sqa_debug cvmx_pko_l1_sqa_debug_t;

/**
 * cvmx_pko_l1_sqb_debug
 */
union cvmx_pko_l1_sqb_debug {
	uint64_t u64;
	struct cvmx_pko_l1_sqb_debug_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dbg_vec                      : 64; /**< Debug Vector. */
#else
	uint64_t dbg_vec                      : 64;
#endif
	} s;
	struct cvmx_pko_l1_sqb_debug_s        cn78xx;
};
typedef union cvmx_pko_l1_sqb_debug cvmx_pko_l1_sqb_debug_t;

/**
 * cvmx_pko_l2_sq#_cir
 */
union cvmx_pko_l2_sqx_cir {
	uint64_t u64;
	struct cvmx_pko_l2_sqx_cir_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_41_63               : 23;
	uint64_t burst_exponent               : 4;  /**< Burst exponent. The burst limit is specified as 1.BURST_MANTISSA << BURST_EXPONENT. */
	uint64_t burst_mantissa               : 8;  /**< Burst mantissa. The burst limit is specified as 1.BURST_MANTISSA << BURST_EXPONENT. */
	uint64_t reserved_17_28               : 12;
	uint64_t rate_divider_exponent        : 4;  /**< Rate divider exponent. This 4-bit base-2 exponent is used to divide the credit rate by
                                                         specifying the number of time-wheel turns required before the accumulator is increased.
                                                         The rate count = (1 << RATE_COUNT_EXPONENT). The supported range for RATE_DIVIDER_EXPONENT
                                                         is 0 to 11. Programmed values greater than 11 are treated as 11. */
	uint64_t rate_exponent                : 4;  /**< Rate exponent. The rate is specified as 1.RATE_MANTISSA << RATE_EXPONENT. */
	uint64_t rate_mantissa                : 8;  /**< Rate mantissa. The rate is specified as 1.RATE_MANTISSA << RATE_EXPONENT. */
	uint64_t enable                       : 1;  /**< Enable. Enables CIR shaping. */
#else
	uint64_t enable                       : 1;
	uint64_t rate_mantissa                : 8;
	uint64_t rate_exponent                : 4;
	uint64_t rate_divider_exponent        : 4;
	uint64_t reserved_17_28               : 12;
	uint64_t burst_mantissa               : 8;
	uint64_t burst_exponent               : 4;
	uint64_t reserved_41_63               : 23;
#endif
	} s;
	struct cvmx_pko_l2_sqx_cir_s          cn78xx;
};
typedef union cvmx_pko_l2_sqx_cir cvmx_pko_l2_sqx_cir_t;

/**
 * cvmx_pko_l2_sq#_green
 */
union cvmx_pko_l2_sqx_green {
	uint64_t u64;
	struct cvmx_pko_l2_sqx_green_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_41_63               : 23;
	uint64_t rr_active                    : 1;  /**< Round-robin red active. Indicates that the round-robin input is mapped to RED. */
	uint64_t active_vec                   : 20; /**< Active vector. A 10-bit vector, ordered by priority, that indicate which inputs to this
                                                         scheduling queue are active. For internal use only. */
	uint64_t reserved_19_19               : 1;
	uint64_t head                         : 9;  /**< Head pointer. The index of round-robin linked-list head. For internal use only. */
	uint64_t reserved_9_9                 : 1;
	uint64_t tail                         : 9;  /**< Tail pointer. The index of round-robin linked-list tail. For internal use only. */
#else
	uint64_t tail                         : 9;
	uint64_t reserved_9_9                 : 1;
	uint64_t head                         : 9;
	uint64_t reserved_19_19               : 1;
	uint64_t active_vec                   : 20;
	uint64_t rr_active                    : 1;
	uint64_t reserved_41_63               : 23;
#endif
	} s;
	struct cvmx_pko_l2_sqx_green_s        cn78xx;
};
typedef union cvmx_pko_l2_sqx_green cvmx_pko_l2_sqx_green_t;

/**
 * cvmx_pko_l2_sq#_pick
 */
union cvmx_pko_l2_sqx_pick {
	uint64_t u64;
	struct cvmx_pko_l2_sqx_pick_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dq                           : 10; /**< Descriptor queue. Index of originating descriptor queue. */
	uint64_t color                        : 2;  /**< See PKO_L2_SQ(0..511)_SHAPE[COLOR]. */
	uint64_t child                        : 10; /**< Child index. When the C_CON bit of this result is set, indicating that this
                                                         result is connected in a flow that extends through the child result, this is the
                                                         index of that child result. */
	uint64_t bubble                       : 1;  /**< This metapacket is a fake passed forward after a prune. */
	uint64_t p_con                        : 1;  /**< Parent connected flag. This pick has more picks in front of it. */
	uint64_t c_con                        : 1;  /**< Child connected flag. This pick has more picks behind it. */
	uint64_t uid                          : 7;  /**< Unique ID. 7-bit unique value assigned at the DQ level, increments for each packet. */
	uint64_t jump                         : 1;  /**< Jump. Set when metapacket originated from a jump descriptor. */
	uint64_t fpd                          : 1;  /**< First packet descriptor. Set when metapacket was the first in a cacheline. */
	uint64_t ds                           : 1;  /**< Don't send. Set when metapacket is not to be sent. */
	uint64_t adjust                       : 9;  /**< See PKO_L2_SQ(0..511)_SHAPE[ADJUST]. */
	uint64_t pir_dis                      : 1;  /**< PIR disable. Peak shaper disabled. */
	uint64_t cir_dis                      : 1;  /**< CIR disable. Committed shaper disabled. */
	uint64_t red_algo_override            : 2;  /**< See PKO_L2_SQ(0..511)_SHAPE[RED_ALGO]. */
	uint64_t length                       : 16; /**< Packet length. The packet length in bytes. */
#else
	uint64_t length                       : 16;
	uint64_t red_algo_override            : 2;
	uint64_t cir_dis                      : 1;
	uint64_t pir_dis                      : 1;
	uint64_t adjust                       : 9;
	uint64_t ds                           : 1;
	uint64_t fpd                          : 1;
	uint64_t jump                         : 1;
	uint64_t uid                          : 7;
	uint64_t c_con                        : 1;
	uint64_t p_con                        : 1;
	uint64_t bubble                       : 1;
	uint64_t child                        : 10;
	uint64_t color                        : 2;
	uint64_t dq                           : 10;
#endif
	} s;
	struct cvmx_pko_l2_sqx_pick_s         cn78xx;
};
typedef union cvmx_pko_l2_sqx_pick cvmx_pko_l2_sqx_pick_t;

/**
 * cvmx_pko_l2_sq#_pir
 */
union cvmx_pko_l2_sqx_pir {
	uint64_t u64;
	struct cvmx_pko_l2_sqx_pir_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_41_63               : 23;
	uint64_t burst_exponent               : 4;  /**< Burst exponent. The burst limit is specified as 1.BURST_MANTISSA << BURST_EXPONENT. */
	uint64_t burst_mantissa               : 8;  /**< Burst mantissa. The burst limit is specified as 1.BURST_MANTISSA << BURST_EXPONENT. */
	uint64_t reserved_17_28               : 12;
	uint64_t rate_divider_exponent        : 4;  /**< Rate divider exponent. This 4-bit base-2 exponent is used to divide the credit rate by
                                                         specifying the number of time-wheel turns required before the accumulator is increased.
                                                         The rate count = (1 << RATE_COUNT_EXPONENT). The supported range for RATE_DIVIDER_EXPONENT
                                                         is 0 to 11. Programmed values greater than 11 are treated as 11. */
	uint64_t rate_exponent                : 4;  /**< Rate exponent. The rate is specified as 1.RATE_MANTISSA << RATE_EXPONENT. */
	uint64_t rate_mantissa                : 8;  /**< Rate mantissa. The rate is specified as 1.RATE_MANTISSA << RATE_EXPONENT. */
	uint64_t enable                       : 1;  /**< Enable. Enables CIR shaping. */
#else
	uint64_t enable                       : 1;
	uint64_t rate_mantissa                : 8;
	uint64_t rate_exponent                : 4;
	uint64_t rate_divider_exponent        : 4;
	uint64_t reserved_17_28               : 12;
	uint64_t burst_mantissa               : 8;
	uint64_t burst_exponent               : 4;
	uint64_t reserved_41_63               : 23;
#endif
	} s;
	struct cvmx_pko_l2_sqx_pir_s          cn78xx;
};
typedef union cvmx_pko_l2_sqx_pir cvmx_pko_l2_sqx_pir_t;

/**
 * cvmx_pko_l2_sq#_pointers
 */
union cvmx_pko_l2_sqx_pointers {
	uint64_t u64;
	struct cvmx_pko_l2_sqx_pointers_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t prev                         : 9;  /**< Previous pointer. The linked-list previous pointer. */
	uint64_t reserved_9_15                : 7;
	uint64_t next                         : 9;  /**< Next pointer. The linked-list next pointer. */
#else
	uint64_t next                         : 9;
	uint64_t reserved_9_15                : 7;
	uint64_t prev                         : 9;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_pko_l2_sqx_pointers_s     cn78xx;
};
typedef union cvmx_pko_l2_sqx_pointers cvmx_pko_l2_sqx_pointers_t;

/**
 * cvmx_pko_l2_sq#_red
 */
union cvmx_pko_l2_sqx_red {
	uint64_t u64;
	struct cvmx_pko_l2_sqx_red_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_19_63               : 45;
	uint64_t head                         : 9;  /**< Head pointer. The index of round-robin linked-list head. For internal use only. */
	uint64_t reserved_9_9                 : 1;
	uint64_t tail                         : 9;  /**< Tail pointer. The index of round-robin linked-list tail. For internal use only. */
#else
	uint64_t tail                         : 9;
	uint64_t reserved_9_9                 : 1;
	uint64_t head                         : 9;
	uint64_t reserved_19_63               : 45;
#endif
	} s;
	struct cvmx_pko_l2_sqx_red_s          cn78xx;
};
typedef union cvmx_pko_l2_sqx_red cvmx_pko_l2_sqx_red_t;

/**
 * cvmx_pko_l2_sq#_sched_state
 */
union cvmx_pko_l2_sqx_sched_state {
	uint64_t u64;
	struct cvmx_pko_l2_sqx_sched_state_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t rr_count                     : 25; /**< Round-robin (DWRR) deficit counter. A 25-bit signed integer count. For diagnostic use. */
#else
	uint64_t rr_count                     : 25;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_pko_l2_sqx_sched_state_s  cn78xx;
};
typedef union cvmx_pko_l2_sqx_sched_state cvmx_pko_l2_sqx_sched_state_t;

/**
 * cvmx_pko_l2_sq#_schedule
 */
union cvmx_pko_l2_sqx_schedule {
	uint64_t u64;
	struct cvmx_pko_l2_sqx_schedule_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t prio                         : 4;  /**< Priority. The priority used for this SQ in the (lower-level) parent's scheduling
                                                         algorithm. When this SQ is not used, we recommend setting PRIO to zero. The legal PRIO
                                                         values are 0-9 when the SQ is used. In addition to priority, PRIO determines whether the
                                                         SQ is a static queue or not: If PRIO equals PKO_*_SQn_TOPOLOGY[RR_PRIO], where
                                                         PKO_*_TOPOLOGY[PARENT] for this SQ equals n, then this is a round-robin child queue into
                                                         the shaper at the next level. */
	uint64_t rr_quantum                   : 24; /**< Round-robin (DWRR) quantum. The deficit-weighted round-robin quantum (24-bit unsigned integer). */
#else
	uint64_t rr_quantum                   : 24;
	uint64_t prio                         : 4;
	uint64_t reserved_28_63               : 36;
#endif
	} s;
	struct cvmx_pko_l2_sqx_schedule_s     cn78xx;
};
typedef union cvmx_pko_l2_sqx_schedule cvmx_pko_l2_sqx_schedule_t;

/**
 * cvmx_pko_l2_sq#_shape
 */
union cvmx_pko_l2_sqx_shape {
	uint64_t u64;
	struct cvmx_pko_l2_sqx_shape_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t length_disable               : 1;  /**< Length disable. Disables the use of packet lengths in shaping calculations such that only
                                                         the value of PKO_L2_SQ(0..511)_SHAPE[ADJUST]. */
	uint64_t reserved_13_23               : 11;
	uint64_t yellow_disable               : 1;  /**< Disable yellow transitions. Disables green-to-yellow packet color marking transitions when set. */
	uint64_t red_disable                  : 1;  /**< Disable red transitions. Disables green-to-red and yellow-to-red packet color marking
                                                         transitions when set. */
	uint64_t red_algo                     : 2;  /**< Shaper red state algorithm.
                                                         0 = Stall packets while in RED state until YELLOW or GREEN state is reached (aka never
                                                         send RED packets).
                                                         1 = Send packets while in RED state.
                                                         2 = Same as 0 above (stall).
                                                         3 = Discard packets while in RED state (red packets are converted to drop packets). */
	uint64_t adjust                       : 9;  /**< Shaping calculation adjustment. This 9-bit signed values allows +/- 256 bytes to be added
                                                         to the packet length for the shaping calculations. */
#else
	uint64_t adjust                       : 9;
	uint64_t red_algo                     : 2;
	uint64_t red_disable                  : 1;
	uint64_t yellow_disable               : 1;
	uint64_t reserved_13_23               : 11;
	uint64_t length_disable               : 1;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_pko_l2_sqx_shape_s        cn78xx;
};
typedef union cvmx_pko_l2_sqx_shape cvmx_pko_l2_sqx_shape_t;

/**
 * cvmx_pko_l2_sq#_shape_state
 */
union cvmx_pko_l2_sqx_shape_state {
	uint64_t u64;
	struct cvmx_pko_l2_sqx_shape_state_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t tw_timestamp                 : 6;  /**< Time-wheel timestamp. Debug access to the live time-wheel timestamp. */
	uint64_t color                        : 2;  /**< Shaper color status. Debug access to the live shaper state.
                                                         0 = Green - operating in 'committed' range
                                                         1 = Yellow - operating in 'excess/peak' range
                                                         2 = Red - operating in 'oversubscribed' range
                                                         3 = Reserved */
	uint64_t pir_accum                    : 26; /**< Peak information rate accumulator. Debug access to the live PIR accumulator. */
	uint64_t cir_accum                    : 26; /**< Committed information rate accumulator. Debug access to the live CIR accumulator. */
#else
	uint64_t cir_accum                    : 26;
	uint64_t pir_accum                    : 26;
	uint64_t color                        : 2;
	uint64_t tw_timestamp                 : 6;
	uint64_t reserved_60_63               : 4;
#endif
	} s;
	struct cvmx_pko_l2_sqx_shape_state_s  cn78xx;
};
typedef union cvmx_pko_l2_sqx_shape_state cvmx_pko_l2_sqx_shape_state_t;

/**
 * cvmx_pko_l2_sq#_sw_xoff
 */
union cvmx_pko_l2_sqx_sw_xoff {
	uint64_t u64;
	struct cvmx_pko_l2_sqx_sw_xoff_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t drain_irq                    : 1;  /**< Drain IRQ. Enables an interrupt that fires when the drain operation has completed. */
	uint64_t drain_null_link              : 1;  /**< "Drain null link. Conditions the drain path to drain through the null link (i.e. link
                                                         \#28). As such, channel credits, HW_XOFF, and shaping are disabled on the draining path
                                                         until the path has drained." */
	uint64_t drain                        : 1;  /**< Drain. This control activates a drain path through the PSE that starts at this node and
                                                         ends at the SQ1 level. The drain path is prioritized over other paths through PSE and can
                                                         be used in combination with DRAIN_NULL_LINK and DRAIN_IRQ. */
	uint64_t xoff                         : 1;  /**< XOFF. The PQ is disabled when XOFF is asserted. PQ is enabled when XOFF is de-asserted.
                                                         NOTE: The associated PKO_L1_SQ(0..31)_TOPOLOGY[LINK] must be configured before using this
                                                         register field. Writing to this register field before the associated
                                                         PKO_L1_SQ(0..31)_TOPOLOGY[LINK] value is configure can result in modifying the software
                                                         XOFF state of the wrong SQ. */
#else
	uint64_t xoff                         : 1;
	uint64_t drain                        : 1;
	uint64_t drain_null_link              : 1;
	uint64_t drain_irq                    : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_pko_l2_sqx_sw_xoff_s      cn78xx;
};
typedef union cvmx_pko_l2_sqx_sw_xoff cvmx_pko_l2_sqx_sw_xoff_t;

/**
 * cvmx_pko_l2_sq#_topology
 */
union cvmx_pko_l2_sqx_topology {
	uint64_t u64;
	struct cvmx_pko_l2_sqx_topology_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_41_63               : 23;
	uint64_t prio_anchor                  : 9;  /**< See PKO_L1_SQ(0..31)_TOPOLOGY[PRIO_ANCHOR]. */
	uint64_t reserved_21_31               : 11;
	uint64_t parent                       : 5;  /**< Parent queue index. The index of the shaping element at the next lower hierarchical level
                                                         that accepts this shaping element's outputs. Refer to the
                                                         PKO_*_SQn_TOPOLOGY[PRIO_ANCHOR,RR_PRIO] descriptions for constraints on which child queues
                                                         can attach to which shapers at the next lower level. When this shaper is unused, we
                                                         recommend that PARENT be zero. */
	uint64_t reserved_5_15                : 11;
	uint64_t rr_prio                      : 4;  /**< See PKO_L1_SQ(0..31)_TOPOLOGY[RR_PRIO]. */
	uint64_t reserved_0_0                 : 1;
#else
	uint64_t reserved_0_0                 : 1;
	uint64_t rr_prio                      : 4;
	uint64_t reserved_5_15                : 11;
	uint64_t parent                       : 5;
	uint64_t reserved_21_31               : 11;
	uint64_t prio_anchor                  : 9;
	uint64_t reserved_41_63               : 23;
#endif
	} s;
	struct cvmx_pko_l2_sqx_topology_s     cn78xx;
};
typedef union cvmx_pko_l2_sqx_topology cvmx_pko_l2_sqx_topology_t;

/**
 * cvmx_pko_l2_sq#_yellow
 */
union cvmx_pko_l2_sqx_yellow {
	uint64_t u64;
	struct cvmx_pko_l2_sqx_yellow_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_19_63               : 45;
	uint64_t head                         : 9;  /**< Head pointer. The index of round-robin linked-list head. For internal use only. */
	uint64_t reserved_9_9                 : 1;
	uint64_t tail                         : 9;  /**< Tail pointer. The index of round-robin linked-list tail. For internal use only. */
#else
	uint64_t tail                         : 9;
	uint64_t reserved_9_9                 : 1;
	uint64_t head                         : 9;
	uint64_t reserved_19_63               : 45;
#endif
	} s;
	struct cvmx_pko_l2_sqx_yellow_s       cn78xx;
};
typedef union cvmx_pko_l2_sqx_yellow cvmx_pko_l2_sqx_yellow_t;

/**
 * cvmx_pko_l2_sq_csr_bus_debug
 */
union cvmx_pko_l2_sq_csr_bus_debug {
	uint64_t u64;
	struct cvmx_pko_l2_sq_csr_bus_debug_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t csr_bus_debug                : 64; /**< N/A */
#else
	uint64_t csr_bus_debug                : 64;
#endif
	} s;
	struct cvmx_pko_l2_sq_csr_bus_debug_s cn78xx;
};
typedef union cvmx_pko_l2_sq_csr_bus_debug cvmx_pko_l2_sq_csr_bus_debug_t;

/**
 * cvmx_pko_l2_sqa_debug
 */
union cvmx_pko_l2_sqa_debug {
	uint64_t u64;
	struct cvmx_pko_l2_sqa_debug_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dbg_vec                      : 64; /**< Debug Vector. */
#else
	uint64_t dbg_vec                      : 64;
#endif
	} s;
	struct cvmx_pko_l2_sqa_debug_s        cn78xx;
};
typedef union cvmx_pko_l2_sqa_debug cvmx_pko_l2_sqa_debug_t;

/**
 * cvmx_pko_l2_sqb_debug
 */
union cvmx_pko_l2_sqb_debug {
	uint64_t u64;
	struct cvmx_pko_l2_sqb_debug_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dbg_vec                      : 64; /**< Debug Vector. */
#else
	uint64_t dbg_vec                      : 64;
#endif
	} s;
	struct cvmx_pko_l2_sqb_debug_s        cn78xx;
};
typedef union cvmx_pko_l2_sqb_debug cvmx_pko_l2_sqb_debug_t;

/**
 * cvmx_pko_l3_l2_sq#_channel
 */
union cvmx_pko_l3_l2_sqx_channel {
	uint64_t u64;
	struct cvmx_pko_l3_l2_sqx_channel_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_44_63               : 20;
	uint64_t cc_channel                   : 12; /**< Channel ID. */
	uint64_t cc_word_cnt                  : 20; /**< Channel credit word count. This value, plus 1 MTU, represents the maximum outstanding word
                                                         count for this channel. (Words are 16 bytes.) Note that this 20-bit field represents a
                                                         signed value that decrements towards zero as credits are used. Packets are not allowed to
                                                         flow when the count is less than zero. As such, the most significant bit should normally
                                                         be programmed as zero (positive count). This gives a maximum value for this field of 2^18
                                                         - 1. */
	uint64_t cc_packet_cnt                : 10; /**< Channel credit packet count. This value, plus 1, represents the maximum outstanding packet
                                                         count for this channel.
                                                         Note that this 10-bit field represents a signed value that decrements towards zero as
                                                         credits are used. Packets are not allowed to flow when the count is less than zero. As
                                                         such the most significant bit should normally be programmed as zero (positive count). This
                                                         gives a maximum value for this field of 2^9 - 1. */
	uint64_t cc_enable                    : 1;  /**< Channel credit enable. Enables CC_WORD_CNT and CC_PACKET_CNT credit processing. */
	uint64_t hw_xoff                      : 1;  /**< Hardware XOFF status. The status of hardware XON/XOFF. This is writable to get around LUT
                                                         issues and for reconfiguration. */
#else
	uint64_t hw_xoff                      : 1;
	uint64_t cc_enable                    : 1;
	uint64_t cc_packet_cnt                : 10;
	uint64_t cc_word_cnt                  : 20;
	uint64_t cc_channel                   : 12;
	uint64_t reserved_44_63               : 20;
#endif
	} s;
	struct cvmx_pko_l3_l2_sqx_channel_s   cn78xx;
};
typedef union cvmx_pko_l3_l2_sqx_channel cvmx_pko_l3_l2_sqx_channel_t;

/**
 * cvmx_pko_l3_sq#_cir
 */
union cvmx_pko_l3_sqx_cir {
	uint64_t u64;
	struct cvmx_pko_l3_sqx_cir_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_41_63               : 23;
	uint64_t burst_exponent               : 4;  /**< Burst exponent. The burst limit is specified as 1.BURST_MANTISSA << BURST_EXPONENT. */
	uint64_t burst_mantissa               : 8;  /**< Burst mantissa. The burst limit is specified as 1.BURST_MANTISSA << BURST_EXPONENT. */
	uint64_t reserved_17_28               : 12;
	uint64_t rate_divider_exponent        : 4;  /**< Rate divider exponent. This 4-bit base-2 exponent is used to divide the credit rate by
                                                         specifying the number of time-wheel turns required before the accumulator is increased.
                                                         The rate count = (1 << RATE_COUNT_EXPONENT). The supported range for RATE_DIVIDER_EXPONENT
                                                         is 0 to 11. Programmed values greater than 11 are treated as 11. */
	uint64_t rate_exponent                : 4;  /**< Rate exponent. The rate is specified as 1.RATE_MANTISSA << RATE_EXPONENT. */
	uint64_t rate_mantissa                : 8;  /**< Rate mantissa. The rate is specified as 1.RATE_MANTISSA << RATE_EXPONENT. */
	uint64_t enable                       : 1;  /**< Enable. Enables CIR shaping. */
#else
	uint64_t enable                       : 1;
	uint64_t rate_mantissa                : 8;
	uint64_t rate_exponent                : 4;
	uint64_t rate_divider_exponent        : 4;
	uint64_t reserved_17_28               : 12;
	uint64_t burst_mantissa               : 8;
	uint64_t burst_exponent               : 4;
	uint64_t reserved_41_63               : 23;
#endif
	} s;
	struct cvmx_pko_l3_sqx_cir_s          cn78xx;
};
typedef union cvmx_pko_l3_sqx_cir cvmx_pko_l3_sqx_cir_t;

/**
 * cvmx_pko_l3_sq#_green
 */
union cvmx_pko_l3_sqx_green {
	uint64_t u64;
	struct cvmx_pko_l3_sqx_green_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_41_63               : 23;
	uint64_t rr_active                    : 1;  /**< Round-robin red active. Indicates that the round-robin input is mapped to RED. */
	uint64_t active_vec                   : 20; /**< Active vector. A 10-bit vector, ordered by priority, that indicate which inputs to this
                                                         scheduling queue are active. For internal use only. */
	uint64_t head                         : 10; /**< Head pointer. The index of round-robin linked-list head. For internal use only. */
	uint64_t tail                         : 10; /**< Tail pointer. The index of round-robin linked-list tail. For internal use only. */
#else
	uint64_t tail                         : 10;
	uint64_t head                         : 10;
	uint64_t active_vec                   : 20;
	uint64_t rr_active                    : 1;
	uint64_t reserved_41_63               : 23;
#endif
	} s;
	struct cvmx_pko_l3_sqx_green_s        cn78xx;
};
typedef union cvmx_pko_l3_sqx_green cvmx_pko_l3_sqx_green_t;

/**
 * cvmx_pko_l3_sq#_pick
 */
union cvmx_pko_l3_sqx_pick {
	uint64_t u64;
	struct cvmx_pko_l3_sqx_pick_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dq                           : 10; /**< Descriptor queue. Index of originating descriptor queue. */
	uint64_t color                        : 2;  /**< See PKO_L2_SQ(0..511)_SHAPE[COLOR]. */
	uint64_t child                        : 10; /**< Child index. When the C_CON bit of this result is set, indicating that this
                                                         result is connected in a flow that extends through the child result, this is the
                                                         index of that child result. */
	uint64_t bubble                       : 1;  /**< This metapacket is a fake passed forward after a prune. */
	uint64_t p_con                        : 1;  /**< Parent connected flag. This pick has more picks in front of it. */
	uint64_t c_con                        : 1;  /**< Child connected flag. This pick has more picks behind it. */
	uint64_t uid                          : 7;  /**< Unique ID. 7-bit unique value assigned at the DQ level, increments for each packet. */
	uint64_t jump                         : 1;  /**< Jump. Set when metapacket originated from a jump descriptor. */
	uint64_t fpd                          : 1;  /**< First packet descriptor. Set when metapacket was the first in a cacheline. */
	uint64_t ds                           : 1;  /**< Don't send. Set when metapacket is not to be sent. */
	uint64_t adjust                       : 9;  /**< See PKO_L2_SQ(0..511)_SHAPE[ADJUST]. */
	uint64_t pir_dis                      : 1;  /**< PIR disable. Peak shaper disabled. */
	uint64_t cir_dis                      : 1;  /**< CIR disable. Committed shaper disabled. */
	uint64_t red_algo_override            : 2;  /**< See PKO_L2_SQ(0..511)_SHAPE[RED_ALGO]. */
	uint64_t length                       : 16; /**< Packet length. The packet length in bytes. */
#else
	uint64_t length                       : 16;
	uint64_t red_algo_override            : 2;
	uint64_t cir_dis                      : 1;
	uint64_t pir_dis                      : 1;
	uint64_t adjust                       : 9;
	uint64_t ds                           : 1;
	uint64_t fpd                          : 1;
	uint64_t jump                         : 1;
	uint64_t uid                          : 7;
	uint64_t c_con                        : 1;
	uint64_t p_con                        : 1;
	uint64_t bubble                       : 1;
	uint64_t child                        : 10;
	uint64_t color                        : 2;
	uint64_t dq                           : 10;
#endif
	} s;
	struct cvmx_pko_l3_sqx_pick_s         cn78xx;
};
typedef union cvmx_pko_l3_sqx_pick cvmx_pko_l3_sqx_pick_t;

/**
 * cvmx_pko_l3_sq#_pir
 */
union cvmx_pko_l3_sqx_pir {
	uint64_t u64;
	struct cvmx_pko_l3_sqx_pir_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_41_63               : 23;
	uint64_t burst_exponent               : 4;  /**< Burst exponent. The burst limit is specified as 1.BURST_MANTISSA << BURST_EXPONENT. */
	uint64_t burst_mantissa               : 8;  /**< Burst mantissa. The burst limit is specified as 1.BURST_MANTISSA << BURST_EXPONENT. */
	uint64_t reserved_17_28               : 12;
	uint64_t rate_divider_exponent        : 4;  /**< Rate divider exponent. This 4-bit base-2 exponent is used to divide the credit rate by
                                                         specifying the number of time-wheel turns required before the accumulator is increased.
                                                         The rate count = (1 << RATE_COUNT_EXPONENT). The supported range for RATE_DIVIDER_EXPONENT
                                                         is 0 to 11. Programmed values greater than 11 are treated as 11. */
	uint64_t rate_exponent                : 4;  /**< Rate exponent. The rate is specified as 1.RATE_MANTISSA << RATE_EXPONENT. */
	uint64_t rate_mantissa                : 8;  /**< Rate mantissa. The rate is specified as 1.RATE_MANTISSA << RATE_EXPONENT. */
	uint64_t enable                       : 1;  /**< Enable. Enables CIR shaping. */
#else
	uint64_t enable                       : 1;
	uint64_t rate_mantissa                : 8;
	uint64_t rate_exponent                : 4;
	uint64_t rate_divider_exponent        : 4;
	uint64_t reserved_17_28               : 12;
	uint64_t burst_mantissa               : 8;
	uint64_t burst_exponent               : 4;
	uint64_t reserved_41_63               : 23;
#endif
	} s;
	struct cvmx_pko_l3_sqx_pir_s          cn78xx;
};
typedef union cvmx_pko_l3_sqx_pir cvmx_pko_l3_sqx_pir_t;

/**
 * cvmx_pko_l3_sq#_pointers
 */
union cvmx_pko_l3_sqx_pointers {
	uint64_t u64;
	struct cvmx_pko_l3_sqx_pointers_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t prev                         : 9;  /**< Previous pointer. The linked-list previous pointer. */
	uint64_t reserved_9_15                : 7;
	uint64_t next                         : 9;  /**< Next pointer. The linked-list next pointer. */
#else
	uint64_t next                         : 9;
	uint64_t reserved_9_15                : 7;
	uint64_t prev                         : 9;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_pko_l3_sqx_pointers_s     cn78xx;
};
typedef union cvmx_pko_l3_sqx_pointers cvmx_pko_l3_sqx_pointers_t;

/**
 * cvmx_pko_l3_sq#_red
 */
union cvmx_pko_l3_sqx_red {
	uint64_t u64;
	struct cvmx_pko_l3_sqx_red_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t head                         : 10; /**< Head pointer. The index of round-robin linked-list head. For internal use only. */
	uint64_t tail                         : 10; /**< Tail pointer. The index of round-robin linked-list tail. For internal use only. */
#else
	uint64_t tail                         : 10;
	uint64_t head                         : 10;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_pko_l3_sqx_red_s          cn78xx;
};
typedef union cvmx_pko_l3_sqx_red cvmx_pko_l3_sqx_red_t;

/**
 * cvmx_pko_l3_sq#_sched_state
 */
union cvmx_pko_l3_sqx_sched_state {
	uint64_t u64;
	struct cvmx_pko_l3_sqx_sched_state_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t rr_count                     : 25; /**< Round-robin (DWRR) deficit counter. A 25-bit signed integer count. For diagnostic use. */
#else
	uint64_t rr_count                     : 25;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_pko_l3_sqx_sched_state_s  cn78xx;
};
typedef union cvmx_pko_l3_sqx_sched_state cvmx_pko_l3_sqx_sched_state_t;

/**
 * cvmx_pko_l3_sq#_schedule
 */
union cvmx_pko_l3_sqx_schedule {
	uint64_t u64;
	struct cvmx_pko_l3_sqx_schedule_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t prio                         : 4;  /**< Priority. The priority used for this SQ in the (lower-level) parent's scheduling
                                                         algorithm. When this SQ is not used, we recommend setting PRIO to zero. The legal PRIO
                                                         values are 0-9 when the SQ is used. In addition to priority, PRIO determines whether the
                                                         SQ is a static queue or not: If PRIO equals PKO_*_SQn_TOPOLOGY[RR_PRIO], where
                                                         PKO_*_TOPOLOGY[PARENT] for this SQ equals n, then this is a round-robin child queue into
                                                         the shaper at the next level. */
	uint64_t rr_quantum                   : 24; /**< Round-robin (DWRR) quantum. The deficit-weighted round-robin quantum (24-bit unsigned integer). */
#else
	uint64_t rr_quantum                   : 24;
	uint64_t prio                         : 4;
	uint64_t reserved_28_63               : 36;
#endif
	} s;
	struct cvmx_pko_l3_sqx_schedule_s     cn78xx;
};
typedef union cvmx_pko_l3_sqx_schedule cvmx_pko_l3_sqx_schedule_t;

/**
 * cvmx_pko_l3_sq#_shape
 */
union cvmx_pko_l3_sqx_shape {
	uint64_t u64;
	struct cvmx_pko_l3_sqx_shape_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t length_disable               : 1;  /**< Length disable. Disables the use of packet lengths in shaping calculations such that only
                                                         the value of the ADJUST field is used. */
	uint64_t reserved_13_23               : 11;
	uint64_t yellow_disable               : 1;  /**< Disable yellow transitions. Disables green-to-yellow packet color marking transitions when set. */
	uint64_t red_disable                  : 1;  /**< Disable red transitions. Disables green-to-red and yellow-to-red packet color marking
                                                         transitions when set. */
	uint64_t red_algo                     : 2;  /**< See PKO_L2_SQ(0..511)_SHAPE[RED_ALGO]. */
	uint64_t adjust                       : 9;  /**< See PKO_L2_SQ(0..511)_SHAPE[ADJUST]. */
#else
	uint64_t adjust                       : 9;
	uint64_t red_algo                     : 2;
	uint64_t red_disable                  : 1;
	uint64_t yellow_disable               : 1;
	uint64_t reserved_13_23               : 11;
	uint64_t length_disable               : 1;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_pko_l3_sqx_shape_s        cn78xx;
};
typedef union cvmx_pko_l3_sqx_shape cvmx_pko_l3_sqx_shape_t;

/**
 * cvmx_pko_l3_sq#_shape_state
 */
union cvmx_pko_l3_sqx_shape_state {
	uint64_t u64;
	struct cvmx_pko_l3_sqx_shape_state_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t tw_timestamp                 : 6;  /**< Time-wheel timestamp. Debug access to the live time-wheel timestamp. */
	uint64_t color                        : 2;  /**< Shaper color status. Debug access to the live shaper state.
                                                         0 = Green - operating in 'committed' range
                                                         1 = Yellow - operating in 'excess/peak' range
                                                         2 = Red - operating in 'oversubscribed' range
                                                         3 = Reserved */
	uint64_t pir_accum                    : 26; /**< Peak information rate accumulator. Debug access to the live PIR accumulator. */
	uint64_t cir_accum                    : 26; /**< Committed information rate accumulator. Debug access to the live CIR accumulator. */
#else
	uint64_t cir_accum                    : 26;
	uint64_t pir_accum                    : 26;
	uint64_t color                        : 2;
	uint64_t tw_timestamp                 : 6;
	uint64_t reserved_60_63               : 4;
#endif
	} s;
	struct cvmx_pko_l3_sqx_shape_state_s  cn78xx;
};
typedef union cvmx_pko_l3_sqx_shape_state cvmx_pko_l3_sqx_shape_state_t;

/**
 * cvmx_pko_l3_sq#_sw_xoff
 */
union cvmx_pko_l3_sqx_sw_xoff {
	uint64_t u64;
	struct cvmx_pko_l3_sqx_sw_xoff_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t drain_irq                    : 1;  /**< Drain IRQ. Enables an interrupt that fires when the drain operation has completed. */
	uint64_t drain_null_link              : 1;  /**< "Drain null link. Conditions the drain path to drain through the null link (i.e. link
                                                         \#28). As such, channel credits, HW_XOFF, and shaping are disabled on the draining path
                                                         until the path has drained." */
	uint64_t drain                        : 1;  /**< Drain. This control activates a drain path through the PSE that starts at this node and
                                                         ends at the SQ1 level. The drain path is prioritized over other paths through PSE and can
                                                         be used in combination with DRAIN_NULL_LINK and DRAIN_IRQ. */
	uint64_t xoff                         : 1;  /**< XOFF. The PQ is disabled when XOFF is asserted. PQ is enabled when XOFF is de-asserted.
                                                         NOTE: The associated PKO_L1_SQ(0..31)_TOPOLOGY[LINK] must be configured before using this
                                                         register field. Writing to this register field before the associated
                                                         PKO_L1_SQ(0..31)_TOPOLOGY[LINK] value is configure can result in modifying the software
                                                         XOFF state of the wrong SQ. */
#else
	uint64_t xoff                         : 1;
	uint64_t drain                        : 1;
	uint64_t drain_null_link              : 1;
	uint64_t drain_irq                    : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_pko_l3_sqx_sw_xoff_s      cn78xx;
};
typedef union cvmx_pko_l3_sqx_sw_xoff cvmx_pko_l3_sqx_sw_xoff_t;

/**
 * cvmx_pko_l3_sq#_topology
 */
union cvmx_pko_l3_sqx_topology {
	uint64_t u64;
	struct cvmx_pko_l3_sqx_topology_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t prio_anchor                  : 10; /**< See PKO_L1_SQ(0..31)_TOPOLOGY[PRIO_ANCHOR]. */
	uint64_t reserved_25_31               : 7;
	uint64_t parent                       : 9;  /**< See PKO_L2_SQ(0..511)_TOPOLOGY[PARENT]. */
	uint64_t reserved_5_15                : 11;
	uint64_t rr_prio                      : 4;  /**< See PKO_L1_SQ(0..31)_TOPOLOGY[RR_PRIO]. */
	uint64_t reserved_0_0                 : 1;
#else
	uint64_t reserved_0_0                 : 1;
	uint64_t rr_prio                      : 4;
	uint64_t reserved_5_15                : 11;
	uint64_t parent                       : 9;
	uint64_t reserved_25_31               : 7;
	uint64_t prio_anchor                  : 10;
	uint64_t reserved_42_63               : 22;
#endif
	} s;
	struct cvmx_pko_l3_sqx_topology_s     cn78xx;
};
typedef union cvmx_pko_l3_sqx_topology cvmx_pko_l3_sqx_topology_t;

/**
 * cvmx_pko_l3_sq#_yellow
 */
union cvmx_pko_l3_sqx_yellow {
	uint64_t u64;
	struct cvmx_pko_l3_sqx_yellow_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t head                         : 10; /**< Head pointer. The index of round-robin linked-list head. For internal use only. */
	uint64_t tail                         : 10; /**< Tail pointer. The index of round-robin linked-list tail. For internal use only. */
#else
	uint64_t tail                         : 10;
	uint64_t head                         : 10;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_pko_l3_sqx_yellow_s       cn78xx;
};
typedef union cvmx_pko_l3_sqx_yellow cvmx_pko_l3_sqx_yellow_t;

/**
 * cvmx_pko_l3_sq_csr_bus_debug
 */
union cvmx_pko_l3_sq_csr_bus_debug {
	uint64_t u64;
	struct cvmx_pko_l3_sq_csr_bus_debug_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t csr_bus_debug                : 64; /**< N/A */
#else
	uint64_t csr_bus_debug                : 64;
#endif
	} s;
	struct cvmx_pko_l3_sq_csr_bus_debug_s cn78xx;
};
typedef union cvmx_pko_l3_sq_csr_bus_debug cvmx_pko_l3_sq_csr_bus_debug_t;

/**
 * cvmx_pko_l3_sqa_debug
 */
union cvmx_pko_l3_sqa_debug {
	uint64_t u64;
	struct cvmx_pko_l3_sqa_debug_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dbg_vec                      : 64; /**< Debug Vector. */
#else
	uint64_t dbg_vec                      : 64;
#endif
	} s;
	struct cvmx_pko_l3_sqa_debug_s        cn78xx;
};
typedef union cvmx_pko_l3_sqa_debug cvmx_pko_l3_sqa_debug_t;

/**
 * cvmx_pko_l3_sqb_debug
 */
union cvmx_pko_l3_sqb_debug {
	uint64_t u64;
	struct cvmx_pko_l3_sqb_debug_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dbg_vec                      : 64; /**< Debug Vector. */
#else
	uint64_t dbg_vec                      : 64;
#endif
	} s;
	struct cvmx_pko_l3_sqb_debug_s        cn78xx;
};
typedef union cvmx_pko_l3_sqb_debug cvmx_pko_l3_sqb_debug_t;

/**
 * cvmx_pko_l4_sq#_cir
 */
union cvmx_pko_l4_sqx_cir {
	uint64_t u64;
	struct cvmx_pko_l4_sqx_cir_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_41_63               : 23;
	uint64_t burst_exponent               : 4;  /**< Burst exponent. The burst limit is specified as 1.BURST_MANTISSA << BURST_EXPONENT. */
	uint64_t burst_mantissa               : 8;  /**< Burst mantissa. The burst limit is specified as 1.BURST_MANTISSA << BURST_EXPONENT. */
	uint64_t reserved_17_28               : 12;
	uint64_t rate_divider_exponent        : 4;  /**< Rate divider exponent. This 4-bit base-2 exponent is used to divide the credit rate by
                                                         specifying the number of time-wheel turns required before the accumulator is increased.
                                                         The rate count = (1 << RATE_COUNT_EXPONENT). The supported range for RATE_DIVIDER_EXPONENT
                                                         is 0 to 11. Programmed values greater than 11 are treated as 11. */
	uint64_t rate_exponent                : 4;  /**< Rate exponent. The rate is specified as 1.RATE_MANTISSA << RATE_EXPONENT. */
	uint64_t rate_mantissa                : 8;  /**< Rate mantissa. The rate is specified as 1.RATE_MANTISSA << RATE_EXPONENT. */
	uint64_t enable                       : 1;  /**< Enable. Enables CIR shaping. */
#else
	uint64_t enable                       : 1;
	uint64_t rate_mantissa                : 8;
	uint64_t rate_exponent                : 4;
	uint64_t rate_divider_exponent        : 4;
	uint64_t reserved_17_28               : 12;
	uint64_t burst_mantissa               : 8;
	uint64_t burst_exponent               : 4;
	uint64_t reserved_41_63               : 23;
#endif
	} s;
	struct cvmx_pko_l4_sqx_cir_s          cn78xx;
};
typedef union cvmx_pko_l4_sqx_cir cvmx_pko_l4_sqx_cir_t;

/**
 * cvmx_pko_l4_sq#_green
 */
union cvmx_pko_l4_sqx_green {
	uint64_t u64;
	struct cvmx_pko_l4_sqx_green_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_41_63               : 23;
	uint64_t rr_active                    : 1;  /**< Round-robin red active. Indicates that the round-robin input is mapped to RED. */
	uint64_t active_vec                   : 20; /**< Active vector. A 10-bit vector, ordered by priority, that indicate which inputs to this
                                                         scheduling queue are active. For internal use only. */
	uint64_t head                         : 10; /**< Head pointer. The index of round-robin linked-list head. For internal use only. */
	uint64_t tail                         : 10; /**< Tail pointer. The index of round-robin linked-list tail. For internal use only. */
#else
	uint64_t tail                         : 10;
	uint64_t head                         : 10;
	uint64_t active_vec                   : 20;
	uint64_t rr_active                    : 1;
	uint64_t reserved_41_63               : 23;
#endif
	} s;
	struct cvmx_pko_l4_sqx_green_s        cn78xx;
};
typedef union cvmx_pko_l4_sqx_green cvmx_pko_l4_sqx_green_t;

/**
 * cvmx_pko_l4_sq#_pick
 */
union cvmx_pko_l4_sqx_pick {
	uint64_t u64;
	struct cvmx_pko_l4_sqx_pick_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dq                           : 10; /**< Descriptor queue. Index of originating descriptor queue. */
	uint64_t color                        : 2;  /**< See PKO_L2_SQ(0..511)_SHAPE[COLOR]. */
	uint64_t child                        : 10; /**< Child index. When the C_CON bit of this result is set, indicating that this
                                                         result is connected in a flow that extends through the child result, this is the
                                                         index of that child result. */
	uint64_t bubble                       : 1;  /**< This metapacket is a fake passed forward after a prune. */
	uint64_t p_con                        : 1;  /**< Parent connected flag. This pick has more picks in front of it. */
	uint64_t c_con                        : 1;  /**< Child connected flag. This pick has more picks behind it. */
	uint64_t uid                          : 7;  /**< Unique ID. 7-bit unique value assigned at the DQ level, increments for each packet. */
	uint64_t jump                         : 1;  /**< Jump. Set when metapacket originated from a jump descriptor. */
	uint64_t fpd                          : 1;  /**< First packet descriptor. Set when metapacket was the first in a cacheline. */
	uint64_t ds                           : 1;  /**< Don't send. Set when metapacket is not to be sent. */
	uint64_t adjust                       : 9;  /**< See PKO_L2_SQ(0..511)_SHAPE[ADJUST]. */
	uint64_t pir_dis                      : 1;  /**< PIR disable. Peak shaper disabled. */
	uint64_t cir_dis                      : 1;  /**< CIR disable. Committed shaper disabled. */
	uint64_t red_algo_override            : 2;  /**< See PKO_L2_SQ(0..511)_SHAPE[RED_ALGO]. */
	uint64_t length                       : 16; /**< Packet length. The packet length in bytes. */
#else
	uint64_t length                       : 16;
	uint64_t red_algo_override            : 2;
	uint64_t cir_dis                      : 1;
	uint64_t pir_dis                      : 1;
	uint64_t adjust                       : 9;
	uint64_t ds                           : 1;
	uint64_t fpd                          : 1;
	uint64_t jump                         : 1;
	uint64_t uid                          : 7;
	uint64_t c_con                        : 1;
	uint64_t p_con                        : 1;
	uint64_t bubble                       : 1;
	uint64_t child                        : 10;
	uint64_t color                        : 2;
	uint64_t dq                           : 10;
#endif
	} s;
	struct cvmx_pko_l4_sqx_pick_s         cn78xx;
};
typedef union cvmx_pko_l4_sqx_pick cvmx_pko_l4_sqx_pick_t;

/**
 * cvmx_pko_l4_sq#_pir
 */
union cvmx_pko_l4_sqx_pir {
	uint64_t u64;
	struct cvmx_pko_l4_sqx_pir_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_41_63               : 23;
	uint64_t burst_exponent               : 4;  /**< Burst exponent. The burst limit is specified as 1.BURST_MANTISSA << BURST_EXPONENT. */
	uint64_t burst_mantissa               : 8;  /**< Burst mantissa. The burst limit is specified as 1.BURST_MANTISSA << BURST_EXPONENT. */
	uint64_t reserved_17_28               : 12;
	uint64_t rate_divider_exponent        : 4;  /**< Rate divider exponent. This 4-bit base-2 exponent is used to divide the credit rate by
                                                         specifying the number of time-wheel turns required before the accumulator is increased.
                                                         The rate count = (1 << RATE_COUNT_EXPONENT). The supported range for RATE_DIVIDER_EXPONENT
                                                         is 0 to 11. Programmed values greater than 11 are treated as 11. */
	uint64_t rate_exponent                : 4;  /**< Rate exponent. The rate is specified as 1.RATE_MANTISSA << RATE_EXPONENT. */
	uint64_t rate_mantissa                : 8;  /**< Rate mantissa. The rate is specified as 1.RATE_MANTISSA << RATE_EXPONENT. */
	uint64_t enable                       : 1;  /**< Enable. Enables CIR shaping. */
#else
	uint64_t enable                       : 1;
	uint64_t rate_mantissa                : 8;
	uint64_t rate_exponent                : 4;
	uint64_t rate_divider_exponent        : 4;
	uint64_t reserved_17_28               : 12;
	uint64_t burst_mantissa               : 8;
	uint64_t burst_exponent               : 4;
	uint64_t reserved_41_63               : 23;
#endif
	} s;
	struct cvmx_pko_l4_sqx_pir_s          cn78xx;
};
typedef union cvmx_pko_l4_sqx_pir cvmx_pko_l4_sqx_pir_t;

/**
 * cvmx_pko_l4_sq#_pointers
 */
union cvmx_pko_l4_sqx_pointers {
	uint64_t u64;
	struct cvmx_pko_l4_sqx_pointers_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_26_63               : 38;
	uint64_t prev                         : 10; /**< See PKO_L2_SQ(0..511)_POINTERS[PREV]. */
	uint64_t reserved_10_15               : 6;
	uint64_t next                         : 10; /**< See PKO_L2_SQ(0..511)_POINTERS[NEXT]. */
#else
	uint64_t next                         : 10;
	uint64_t reserved_10_15               : 6;
	uint64_t prev                         : 10;
	uint64_t reserved_26_63               : 38;
#endif
	} s;
	struct cvmx_pko_l4_sqx_pointers_s     cn78xx;
};
typedef union cvmx_pko_l4_sqx_pointers cvmx_pko_l4_sqx_pointers_t;

/**
 * cvmx_pko_l4_sq#_red
 */
union cvmx_pko_l4_sqx_red {
	uint64_t u64;
	struct cvmx_pko_l4_sqx_red_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t head                         : 10; /**< Head pointer. The index of round-robin linked-list head. For internal use only. */
	uint64_t tail                         : 10; /**< Tail pointer. The index of round-robin linked-list tail. For internal use only. */
#else
	uint64_t tail                         : 10;
	uint64_t head                         : 10;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_pko_l4_sqx_red_s          cn78xx;
};
typedef union cvmx_pko_l4_sqx_red cvmx_pko_l4_sqx_red_t;

/**
 * cvmx_pko_l4_sq#_sched_state
 */
union cvmx_pko_l4_sqx_sched_state {
	uint64_t u64;
	struct cvmx_pko_l4_sqx_sched_state_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t rr_count                     : 25; /**< Round-robin (DWRR) deficit counter. A 25-bit signed integer count. For diagnostic use. */
#else
	uint64_t rr_count                     : 25;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_pko_l4_sqx_sched_state_s  cn78xx;
};
typedef union cvmx_pko_l4_sqx_sched_state cvmx_pko_l4_sqx_sched_state_t;

/**
 * cvmx_pko_l4_sq#_schedule
 */
union cvmx_pko_l4_sqx_schedule {
	uint64_t u64;
	struct cvmx_pko_l4_sqx_schedule_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t prio                         : 4;  /**< Priority. The priority used for this SQ in the (lower-level) parent's scheduling
                                                         algorithm. When this SQ is not used, we recommend setting PRIO to zero. The legal PRIO
                                                         values are 0-9 when the SQ is used. In addition to priority, PRIO determines whether the
                                                         SQ is a static queue or not: If PRIO equals PKO_*_SQn_TOPOLOGY[RR_PRIO], where
                                                         PKO_*_TOPOLOGY[PARENT] for this SQ equals n, then this is a round-robin child queue into
                                                         the shaper at the next level. */
	uint64_t rr_quantum                   : 24; /**< Round-robin (DWRR) quantum. The deficit-weighted round-robin quantum (24-bit unsigned integer). */
#else
	uint64_t rr_quantum                   : 24;
	uint64_t prio                         : 4;
	uint64_t reserved_28_63               : 36;
#endif
	} s;
	struct cvmx_pko_l4_sqx_schedule_s     cn78xx;
};
typedef union cvmx_pko_l4_sqx_schedule cvmx_pko_l4_sqx_schedule_t;

/**
 * cvmx_pko_l4_sq#_shape
 */
union cvmx_pko_l4_sqx_shape {
	uint64_t u64;
	struct cvmx_pko_l4_sqx_shape_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t length_disable               : 1;  /**< Length disable. Disables the use of packet lengths in shaping calculations such that only
                                                         the value of the ADJUST field is used. */
	uint64_t reserved_13_23               : 11;
	uint64_t yellow_disable               : 1;  /**< Disable yellow transitions. Disables green-to-yellow packet color marking transitions when set. */
	uint64_t red_disable                  : 1;  /**< Disable red transitions. Disables green-to-red and yellow-to-red packet color marking
                                                         transitions when set. */
	uint64_t red_algo                     : 2;  /**< See PKO_L2_SQ(0..511)_SHAPE[RED_ALGO]. */
	uint64_t adjust                       : 9;  /**< See PKO_L2_SQ(0..511)_SHAPE[ADJUST]. */
#else
	uint64_t adjust                       : 9;
	uint64_t red_algo                     : 2;
	uint64_t red_disable                  : 1;
	uint64_t yellow_disable               : 1;
	uint64_t reserved_13_23               : 11;
	uint64_t length_disable               : 1;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_pko_l4_sqx_shape_s        cn78xx;
};
typedef union cvmx_pko_l4_sqx_shape cvmx_pko_l4_sqx_shape_t;

/**
 * cvmx_pko_l4_sq#_shape_state
 */
union cvmx_pko_l4_sqx_shape_state {
	uint64_t u64;
	struct cvmx_pko_l4_sqx_shape_state_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t tw_timestamp                 : 6;  /**< Time-wheel timestamp. Debug access to the live time-wheel timestamp. */
	uint64_t color                        : 2;  /**< Shaper color status. Debug access to the live shaper state.
                                                         0 = Green - operating in 'committed' range
                                                         1 = Yellow - operating in 'excess/peak' range
                                                         2 = Red - operating in 'oversubscribed' range
                                                         3 = Reserved */
	uint64_t pir_accum                    : 26; /**< Peak information rate accumulator. Debug access to the live PIR accumulator. */
	uint64_t cir_accum                    : 26; /**< Committed information rate accumulator. Debug access to the live CIR accumulator. */
#else
	uint64_t cir_accum                    : 26;
	uint64_t pir_accum                    : 26;
	uint64_t color                        : 2;
	uint64_t tw_timestamp                 : 6;
	uint64_t reserved_60_63               : 4;
#endif
	} s;
	struct cvmx_pko_l4_sqx_shape_state_s  cn78xx;
};
typedef union cvmx_pko_l4_sqx_shape_state cvmx_pko_l4_sqx_shape_state_t;

/**
 * cvmx_pko_l4_sq#_sw_xoff
 */
union cvmx_pko_l4_sqx_sw_xoff {
	uint64_t u64;
	struct cvmx_pko_l4_sqx_sw_xoff_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t drain_irq                    : 1;  /**< Drain IRQ. Enables an interrupt that fires when the drain operation has completed. */
	uint64_t drain_null_link              : 1;  /**< "Drain null link. Conditions the drain path to drain through the null link (i.e. link
                                                         \#28). As such, channel credits, HW_XOFF, and shaping are disabled on the draining path
                                                         until the path has drained." */
	uint64_t drain                        : 1;  /**< Drain. This control activates a drain path through the PSE that starts at this node and
                                                         ends at the SQ1 level. The drain path is prioritized over other paths through PSE and can
                                                         be used in combination with DRAIN_NULL_LINK and DRAIN_IRQ. */
	uint64_t xoff                         : 1;  /**< XOFF. The PQ is disabled when XOFF is asserted. PQ is enabled when XOFF is de-asserted.
                                                         NOTE: The associated PKO_L1_SQ(0..31)_TOPOLOGY[LINK] must be configured before using this
                                                         register field. Writing to this register field before the associated
                                                         PKO_L1_SQ(0..31)_TOPOLOGY[LINK] value is configure can result in modifying the software
                                                         XOFF state of the wrong SQ. */
#else
	uint64_t xoff                         : 1;
	uint64_t drain                        : 1;
	uint64_t drain_null_link              : 1;
	uint64_t drain_irq                    : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_pko_l4_sqx_sw_xoff_s      cn78xx;
};
typedef union cvmx_pko_l4_sqx_sw_xoff cvmx_pko_l4_sqx_sw_xoff_t;

/**
 * cvmx_pko_l4_sq#_topology
 */
union cvmx_pko_l4_sqx_topology {
	uint64_t u64;
	struct cvmx_pko_l4_sqx_topology_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t prio_anchor                  : 10; /**< See PKO_L1_SQ(0..31)_TOPOLOGY[PRIO_ANCHOR]. */
	uint64_t reserved_25_31               : 7;
	uint64_t parent                       : 9;  /**< See PKO_L2_SQ(0..511)_TOPOLOGY[PARENT]. */
	uint64_t reserved_5_15                : 11;
	uint64_t rr_prio                      : 4;  /**< See PKO_L1_SQ(0..31)_TOPOLOGY[RR_PRIO]. */
	uint64_t reserved_0_0                 : 1;
#else
	uint64_t reserved_0_0                 : 1;
	uint64_t rr_prio                      : 4;
	uint64_t reserved_5_15                : 11;
	uint64_t parent                       : 9;
	uint64_t reserved_25_31               : 7;
	uint64_t prio_anchor                  : 10;
	uint64_t reserved_42_63               : 22;
#endif
	} s;
	struct cvmx_pko_l4_sqx_topology_s     cn78xx;
};
typedef union cvmx_pko_l4_sqx_topology cvmx_pko_l4_sqx_topology_t;

/**
 * cvmx_pko_l4_sq#_yellow
 */
union cvmx_pko_l4_sqx_yellow {
	uint64_t u64;
	struct cvmx_pko_l4_sqx_yellow_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t head                         : 10; /**< Head pointer. The index of round-robin linked-list head. For internal use only. */
	uint64_t tail                         : 10; /**< Tail pointer. The index of round-robin linked-list tail. For internal use only. */
#else
	uint64_t tail                         : 10;
	uint64_t head                         : 10;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_pko_l4_sqx_yellow_s       cn78xx;
};
typedef union cvmx_pko_l4_sqx_yellow cvmx_pko_l4_sqx_yellow_t;

/**
 * cvmx_pko_l4_sq_csr_bus_debug
 */
union cvmx_pko_l4_sq_csr_bus_debug {
	uint64_t u64;
	struct cvmx_pko_l4_sq_csr_bus_debug_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t csr_bus_debug                : 64; /**< N/A */
#else
	uint64_t csr_bus_debug                : 64;
#endif
	} s;
	struct cvmx_pko_l4_sq_csr_bus_debug_s cn78xx;
};
typedef union cvmx_pko_l4_sq_csr_bus_debug cvmx_pko_l4_sq_csr_bus_debug_t;

/**
 * cvmx_pko_l4_sqa_debug
 */
union cvmx_pko_l4_sqa_debug {
	uint64_t u64;
	struct cvmx_pko_l4_sqa_debug_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dbg_vec                      : 64; /**< Debug Vector. */
#else
	uint64_t dbg_vec                      : 64;
#endif
	} s;
	struct cvmx_pko_l4_sqa_debug_s        cn78xx;
};
typedef union cvmx_pko_l4_sqa_debug cvmx_pko_l4_sqa_debug_t;

/**
 * cvmx_pko_l4_sqb_debug
 */
union cvmx_pko_l4_sqb_debug {
	uint64_t u64;
	struct cvmx_pko_l4_sqb_debug_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dbg_vec                      : 64; /**< Debug Vector. */
#else
	uint64_t dbg_vec                      : 64;
#endif
	} s;
	struct cvmx_pko_l4_sqb_debug_s        cn78xx;
};
typedef union cvmx_pko_l4_sqb_debug cvmx_pko_l4_sqb_debug_t;

/**
 * cvmx_pko_l5_sq#_cir
 */
union cvmx_pko_l5_sqx_cir {
	uint64_t u64;
	struct cvmx_pko_l5_sqx_cir_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_41_63               : 23;
	uint64_t burst_exponent               : 4;  /**< Burst exponent. The burst limit is specified as 1.BURST_MANTISSA << BURST_EXPONENT. */
	uint64_t burst_mantissa               : 8;  /**< Burst mantissa. The burst limit is specified as 1.BURST_MANTISSA << BURST_EXPONENT. */
	uint64_t reserved_17_28               : 12;
	uint64_t rate_divider_exponent        : 4;  /**< Rate divider exponent. This 4-bit base-2 exponent is used to divide the credit rate by
                                                         specifying the number of time-wheel turns required before the accumulator is increased.
                                                         The rate count = (1 << RATE_COUNT_EXPONENT). The supported range for RATE_DIVIDER_EXPONENT
                                                         is 0 to 11. Programmed values greater than 11 are treated as 11. */
	uint64_t rate_exponent                : 4;  /**< Rate exponent. The rate is specified as 1.RATE_MANTISSA << RATE_EXPONENT. */
	uint64_t rate_mantissa                : 8;  /**< Rate mantissa. The rate is specified as 1.RATE_MANTISSA << RATE_EXPONENT. */
	uint64_t enable                       : 1;  /**< Enable. Enables CIR shaping. */
#else
	uint64_t enable                       : 1;
	uint64_t rate_mantissa                : 8;
	uint64_t rate_exponent                : 4;
	uint64_t rate_divider_exponent        : 4;
	uint64_t reserved_17_28               : 12;
	uint64_t burst_mantissa               : 8;
	uint64_t burst_exponent               : 4;
	uint64_t reserved_41_63               : 23;
#endif
	} s;
	struct cvmx_pko_l5_sqx_cir_s          cn78xx;
};
typedef union cvmx_pko_l5_sqx_cir cvmx_pko_l5_sqx_cir_t;

/**
 * cvmx_pko_l5_sq#_green
 */
union cvmx_pko_l5_sqx_green {
	uint64_t u64;
	struct cvmx_pko_l5_sqx_green_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_41_63               : 23;
	uint64_t rr_active                    : 1;  /**< Round-robin red active. Indicates that the round-robin input is mapped to RED. */
	uint64_t active_vec                   : 20; /**< Active vector. A 10-bit vector, ordered by priority, that indicate which inputs to this
                                                         scheduling queue are active. For internal use only. */
	uint64_t head                         : 10; /**< Head pointer. The index of round-robin linked-list head. For internal use only. */
	uint64_t tail                         : 10; /**< Tail pointer. The index of round-robin linked-list tail. For internal use only. */
#else
	uint64_t tail                         : 10;
	uint64_t head                         : 10;
	uint64_t active_vec                   : 20;
	uint64_t rr_active                    : 1;
	uint64_t reserved_41_63               : 23;
#endif
	} s;
	struct cvmx_pko_l5_sqx_green_s        cn78xx;
};
typedef union cvmx_pko_l5_sqx_green cvmx_pko_l5_sqx_green_t;

/**
 * cvmx_pko_l5_sq#_pick
 */
union cvmx_pko_l5_sqx_pick {
	uint64_t u64;
	struct cvmx_pko_l5_sqx_pick_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dq                           : 10; /**< Descriptor queue. Index of originating descriptor queue. */
	uint64_t color                        : 2;  /**< See PKO_L2_SQ(0..511)_SHAPE[COLOR]. */
	uint64_t child                        : 10; /**< Child index. When the C_CON bit of this result is set, indicating that this
                                                         result is connected in a flow that extends through the child result, this is the
                                                         index of that child result. */
	uint64_t bubble                       : 1;  /**< This metapacket is a fake passed forward after a prune. */
	uint64_t p_con                        : 1;  /**< Parent connected flag. This pick has more picks in front of it. */
	uint64_t c_con                        : 1;  /**< Child connected flag. This pick has more picks behind it. */
	uint64_t uid                          : 7;  /**< Unique ID. 7-bit unique value assigned at the DQ level, increments for each packet. */
	uint64_t jump                         : 1;  /**< Jump. Set when metapacket originated from a jump descriptor. */
	uint64_t fpd                          : 1;  /**< First packet descriptor. Set when metapacket was the first in a cacheline. */
	uint64_t ds                           : 1;  /**< Don't send. Set when metapacket is not to be sent. */
	uint64_t adjust                       : 9;  /**< See PKO_L2_SQ(0..511)_SHAPE[ADJUST]. */
	uint64_t pir_dis                      : 1;  /**< PIR disable. Peak shaper disabled. */
	uint64_t cir_dis                      : 1;  /**< CIR disable. Committed shaper disabled. */
	uint64_t red_algo_override            : 2;  /**< See PKO_L2_SQ(0..511)_SHAPE[RED_ALGO]. */
	uint64_t length                       : 16; /**< Packet length. The packet length in bytes. */
#else
	uint64_t length                       : 16;
	uint64_t red_algo_override            : 2;
	uint64_t cir_dis                      : 1;
	uint64_t pir_dis                      : 1;
	uint64_t adjust                       : 9;
	uint64_t ds                           : 1;
	uint64_t fpd                          : 1;
	uint64_t jump                         : 1;
	uint64_t uid                          : 7;
	uint64_t c_con                        : 1;
	uint64_t p_con                        : 1;
	uint64_t bubble                       : 1;
	uint64_t child                        : 10;
	uint64_t color                        : 2;
	uint64_t dq                           : 10;
#endif
	} s;
	struct cvmx_pko_l5_sqx_pick_s         cn78xx;
};
typedef union cvmx_pko_l5_sqx_pick cvmx_pko_l5_sqx_pick_t;

/**
 * cvmx_pko_l5_sq#_pir
 */
union cvmx_pko_l5_sqx_pir {
	uint64_t u64;
	struct cvmx_pko_l5_sqx_pir_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_41_63               : 23;
	uint64_t burst_exponent               : 4;  /**< Burst exponent. The burst limit is specified as 1.BURST_MANTISSA << BURST_EXPONENT. */
	uint64_t burst_mantissa               : 8;  /**< Burst mantissa. The burst limit is specified as 1.BURST_MANTISSA << BURST_EXPONENT. */
	uint64_t reserved_17_28               : 12;
	uint64_t rate_divider_exponent        : 4;  /**< Rate divider exponent. This 4-bit base-2 exponent is used to divide the credit rate by
                                                         specifying the number of time-wheel turns required before the accumulator is increased.
                                                         The rate count = (1 << RATE_COUNT_EXPONENT). The supported range for RATE_DIVIDER_EXPONENT
                                                         is 0 to 11. Programmed values greater than 11 are treated as 11. */
	uint64_t rate_exponent                : 4;  /**< Rate exponent. The rate is specified as 1.RATE_MANTISSA << RATE_EXPONENT. */
	uint64_t rate_mantissa                : 8;  /**< Rate mantissa. The rate is specified as 1.RATE_MANTISSA << RATE_EXPONENT. */
	uint64_t enable                       : 1;  /**< Enable. Enables CIR shaping. */
#else
	uint64_t enable                       : 1;
	uint64_t rate_mantissa                : 8;
	uint64_t rate_exponent                : 4;
	uint64_t rate_divider_exponent        : 4;
	uint64_t reserved_17_28               : 12;
	uint64_t burst_mantissa               : 8;
	uint64_t burst_exponent               : 4;
	uint64_t reserved_41_63               : 23;
#endif
	} s;
	struct cvmx_pko_l5_sqx_pir_s          cn78xx;
};
typedef union cvmx_pko_l5_sqx_pir cvmx_pko_l5_sqx_pir_t;

/**
 * cvmx_pko_l5_sq#_pointers
 */
union cvmx_pko_l5_sqx_pointers {
	uint64_t u64;
	struct cvmx_pko_l5_sqx_pointers_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_26_63               : 38;
	uint64_t prev                         : 10; /**< See PKO_L2_SQ(0..511)_POINTERS[PREV]. */
	uint64_t reserved_10_15               : 6;
	uint64_t next                         : 10; /**< See PKO_L2_SQ(0..511)_POINTERS[NEXT]. */
#else
	uint64_t next                         : 10;
	uint64_t reserved_10_15               : 6;
	uint64_t prev                         : 10;
	uint64_t reserved_26_63               : 38;
#endif
	} s;
	struct cvmx_pko_l5_sqx_pointers_s     cn78xx;
};
typedef union cvmx_pko_l5_sqx_pointers cvmx_pko_l5_sqx_pointers_t;

/**
 * cvmx_pko_l5_sq#_red
 */
union cvmx_pko_l5_sqx_red {
	uint64_t u64;
	struct cvmx_pko_l5_sqx_red_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t head                         : 10; /**< Head pointer. The index of round-robin linked-list head. For internal use only. */
	uint64_t tail                         : 10; /**< Tail pointer. The index of round-robin linked-list tail. For internal use only. */
#else
	uint64_t tail                         : 10;
	uint64_t head                         : 10;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_pko_l5_sqx_red_s          cn78xx;
};
typedef union cvmx_pko_l5_sqx_red cvmx_pko_l5_sqx_red_t;

/**
 * cvmx_pko_l5_sq#_sched_state
 */
union cvmx_pko_l5_sqx_sched_state {
	uint64_t u64;
	struct cvmx_pko_l5_sqx_sched_state_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t rr_count                     : 25; /**< Round-robin (DWRR) deficit counter. A 25-bit signed integer count. For diagnostic use. */
#else
	uint64_t rr_count                     : 25;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_pko_l5_sqx_sched_state_s  cn78xx;
};
typedef union cvmx_pko_l5_sqx_sched_state cvmx_pko_l5_sqx_sched_state_t;

/**
 * cvmx_pko_l5_sq#_schedule
 */
union cvmx_pko_l5_sqx_schedule {
	uint64_t u64;
	struct cvmx_pko_l5_sqx_schedule_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t prio                         : 4;  /**< Priority. The priority used for this SQ in the (lower-level) parent's scheduling
                                                         algorithm. When this SQ is not used, we recommend setting PRIO to zero. The legal PRIO
                                                         values are 0-9 when the SQ is used. In addition to priority, PRIO determines whether the
                                                         SQ is a static queue or not: If PRIO equals PKO_*_SQn_TOPOLOGY[RR_PRIO], where
                                                         PKO_*_TOPOLOGY[PARENT] for this SQ equals n, then this is a round-robin child queue into
                                                         the shaper at the next level. */
	uint64_t rr_quantum                   : 24; /**< Round-robin (DWRR) quantum. The deficit-weighted round-robin quantum (24-bit unsigned integer). */
#else
	uint64_t rr_quantum                   : 24;
	uint64_t prio                         : 4;
	uint64_t reserved_28_63               : 36;
#endif
	} s;
	struct cvmx_pko_l5_sqx_schedule_s     cn78xx;
};
typedef union cvmx_pko_l5_sqx_schedule cvmx_pko_l5_sqx_schedule_t;

/**
 * cvmx_pko_l5_sq#_shape
 */
union cvmx_pko_l5_sqx_shape {
	uint64_t u64;
	struct cvmx_pko_l5_sqx_shape_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t length_disable               : 1;  /**< Length disable. Disables the use of packet lengths in shaping calculations such that only
                                                         the value of PKO_L5_SQ(0..1023)_SHAPE[ADJUST]. */
	uint64_t reserved_13_23               : 11;
	uint64_t yellow_disable               : 1;  /**< Disable yellow transitions. Disables green-to-yellow packet color marking transitions when set. */
	uint64_t red_disable                  : 1;  /**< Disable red transitions. Disables green-to-red and yellow-to-red packet color marking
                                                         transitions when set. */
	uint64_t red_algo                     : 2;  /**< See PKO_L2_SQ(0..511)_SHAPE[RED_ALGO]. */
	uint64_t adjust                       : 9;  /**< See PKO_L2_SQ(0..511)_SHAPE[ADJUST]. */
#else
	uint64_t adjust                       : 9;
	uint64_t red_algo                     : 2;
	uint64_t red_disable                  : 1;
	uint64_t yellow_disable               : 1;
	uint64_t reserved_13_23               : 11;
	uint64_t length_disable               : 1;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_pko_l5_sqx_shape_s        cn78xx;
};
typedef union cvmx_pko_l5_sqx_shape cvmx_pko_l5_sqx_shape_t;

/**
 * cvmx_pko_l5_sq#_shape_state
 */
union cvmx_pko_l5_sqx_shape_state {
	uint64_t u64;
	struct cvmx_pko_l5_sqx_shape_state_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t tw_timestamp                 : 6;  /**< Time-wheel timestamp. Debug access to the live time-wheel timestamp. */
	uint64_t color                        : 2;  /**< Shaper color status. Debug access to the live shaper state.
                                                         0 = Green - operating in 'committed' range
                                                         1 = Yellow - operating in 'excess/peak' range
                                                         2 = Red - operating in 'oversubscribed' range
                                                         3 = Reserved */
	uint64_t pir_accum                    : 26; /**< Peak information rate accumulator. Debug access to the live PIR accumulator. */
	uint64_t cir_accum                    : 26; /**< Committed information rate accumulator. Debug access to the live CIR accumulator. */
#else
	uint64_t cir_accum                    : 26;
	uint64_t pir_accum                    : 26;
	uint64_t color                        : 2;
	uint64_t tw_timestamp                 : 6;
	uint64_t reserved_60_63               : 4;
#endif
	} s;
	struct cvmx_pko_l5_sqx_shape_state_s  cn78xx;
};
typedef union cvmx_pko_l5_sqx_shape_state cvmx_pko_l5_sqx_shape_state_t;

/**
 * cvmx_pko_l5_sq#_sw_xoff
 */
union cvmx_pko_l5_sqx_sw_xoff {
	uint64_t u64;
	struct cvmx_pko_l5_sqx_sw_xoff_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t drain_irq                    : 1;  /**< Drain IRQ. Enables an interrupt that fires when the drain operation has completed. */
	uint64_t drain_null_link              : 1;  /**< "Drain null link. Conditions the drain path to drain through the null link (i.e. link
                                                         \#28). As such, channel credits, HW_XOFF, and shaping are disabled on the draining path
                                                         until the path has drained." */
	uint64_t drain                        : 1;  /**< Drain. This control activates a drain path through the PSE that starts at this node and
                                                         ends at the SQ1 level. The drain path is prioritized over other paths through PSE and can
                                                         be used in combination with DRAIN_NULL_LINK and DRAIN_IRQ. */
	uint64_t xoff                         : 1;  /**< XOFF. The PQ is disabled when XOFF is asserted. PQ is enabled when XOFF is de-asserted.
                                                         NOTE: The associated PKO_L1_SQ(0..31)_TOPOLOGY[LINK] must be configured before using this
                                                         register field. Writing to this register field before the associated
                                                         PKO_L1_SQ(0..31)_TOPOLOGY[LINK] value is configure can result in modifying the software
                                                         XOFF state of the wrong SQ. */
#else
	uint64_t xoff                         : 1;
	uint64_t drain                        : 1;
	uint64_t drain_null_link              : 1;
	uint64_t drain_irq                    : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_pko_l5_sqx_sw_xoff_s      cn78xx;
};
typedef union cvmx_pko_l5_sqx_sw_xoff cvmx_pko_l5_sqx_sw_xoff_t;

/**
 * cvmx_pko_l5_sq#_topology
 */
union cvmx_pko_l5_sqx_topology {
	uint64_t u64;
	struct cvmx_pko_l5_sqx_topology_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t prio_anchor                  : 10; /**< See PKO_L1_SQ(0..31)_TOPOLOGY[PRIO_ANCHOR]. */
	uint64_t reserved_26_31               : 6;
	uint64_t parent                       : 10; /**< See PKO_L2_SQ(0..511)_TOPOLOGY[PARENT]. */
	uint64_t reserved_5_15                : 11;
	uint64_t rr_prio                      : 4;  /**< See PKO_L1_SQ(0..31)_TOPOLOGY[RR_PRIO]. */
	uint64_t reserved_0_0                 : 1;
#else
	uint64_t reserved_0_0                 : 1;
	uint64_t rr_prio                      : 4;
	uint64_t reserved_5_15                : 11;
	uint64_t parent                       : 10;
	uint64_t reserved_26_31               : 6;
	uint64_t prio_anchor                  : 10;
	uint64_t reserved_42_63               : 22;
#endif
	} s;
	struct cvmx_pko_l5_sqx_topology_s     cn78xx;
};
typedef union cvmx_pko_l5_sqx_topology cvmx_pko_l5_sqx_topology_t;

/**
 * cvmx_pko_l5_sq#_yellow
 */
union cvmx_pko_l5_sqx_yellow {
	uint64_t u64;
	struct cvmx_pko_l5_sqx_yellow_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t head                         : 10; /**< Head pointer. The index of round-robin linked-list head. For internal use only. */
	uint64_t tail                         : 10; /**< Tail pointer. The index of round-robin linked-list tail. For internal use only. */
#else
	uint64_t tail                         : 10;
	uint64_t head                         : 10;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_pko_l5_sqx_yellow_s       cn78xx;
};
typedef union cvmx_pko_l5_sqx_yellow cvmx_pko_l5_sqx_yellow_t;

/**
 * cvmx_pko_l5_sq_csr_bus_debug
 */
union cvmx_pko_l5_sq_csr_bus_debug {
	uint64_t u64;
	struct cvmx_pko_l5_sq_csr_bus_debug_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t csr_bus_debug                : 64; /**< N/A */
#else
	uint64_t csr_bus_debug                : 64;
#endif
	} s;
	struct cvmx_pko_l5_sq_csr_bus_debug_s cn78xx;
};
typedef union cvmx_pko_l5_sq_csr_bus_debug cvmx_pko_l5_sq_csr_bus_debug_t;

/**
 * cvmx_pko_l5_sqa_debug
 */
union cvmx_pko_l5_sqa_debug {
	uint64_t u64;
	struct cvmx_pko_l5_sqa_debug_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dbg_vec                      : 64; /**< Debug Vector. */
#else
	uint64_t dbg_vec                      : 64;
#endif
	} s;
	struct cvmx_pko_l5_sqa_debug_s        cn78xx;
};
typedef union cvmx_pko_l5_sqa_debug cvmx_pko_l5_sqa_debug_t;

/**
 * cvmx_pko_l5_sqb_debug
 */
union cvmx_pko_l5_sqb_debug {
	uint64_t u64;
	struct cvmx_pko_l5_sqb_debug_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dbg_vec                      : 64; /**< Debug Vector. */
#else
	uint64_t dbg_vec                      : 64;
#endif
	} s;
	struct cvmx_pko_l5_sqb_debug_s        cn78xx;
};
typedef union cvmx_pko_l5_sqb_debug cvmx_pko_l5_sqb_debug_t;

/**
 * cvmx_pko_lut#
 */
union cvmx_pko_lutx {
	uint64_t u64;
	struct cvmx_pko_lutx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t valid                        : 1;  /**< Declares if the index in the LUT is valid. */
	uint64_t level                        : 1;  /**< 0 = level2, 1 = level3. */
	uint64_t pq_idx                       : 5;  /**< PQ index for channel return processing in the PSE. */
	uint64_t queue_number                 : 9;  /**< Mapping from this channel to the programmed queue number. */
#else
	uint64_t queue_number                 : 9;
	uint64_t pq_idx                       : 5;
	uint64_t level                        : 1;
	uint64_t valid                        : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_pko_lutx_s                cn78xx;
};
typedef union cvmx_pko_lutx cvmx_pko_lutx_t;

/**
 * cvmx_pko_lut_bist_status
 */
union cvmx_pko_lut_bist_status {
	uint64_t u64;
	struct cvmx_pko_lut_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t bist_status                  : 1;  /**< C2Q LUT BIST status. */
#else
	uint64_t bist_status                  : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_pko_lut_bist_status_s     cn78xx;
};
typedef union cvmx_pko_lut_bist_status cvmx_pko_lut_bist_status_t;

/**
 * cvmx_pko_lut_ecc_ctl0
 */
union cvmx_pko_lut_ecc_ctl0 {
	uint64_t u64;
	struct cvmx_pko_lut_ecc_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t c2q_lut_ram_flip             : 2;  /**< C2Q_LUT_RAM flip syndrome bits on write. */
	uint64_t c2q_lut_ram_cdis             : 1;  /**< C2Q_LUT_RAM ECC correction disable. */
	uint64_t reserved_0_60                : 61;
#else
	uint64_t reserved_0_60                : 61;
	uint64_t c2q_lut_ram_cdis             : 1;
	uint64_t c2q_lut_ram_flip             : 2;
#endif
	} s;
	struct cvmx_pko_lut_ecc_ctl0_s        cn78xx;
};
typedef union cvmx_pko_lut_ecc_ctl0 cvmx_pko_lut_ecc_ctl0_t;

/**
 * cvmx_pko_lut_ecc_dbe_sts0
 */
union cvmx_pko_lut_ecc_dbe_sts0 {
	uint64_t u64;
	struct cvmx_pko_lut_ecc_dbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t c2q_lut_ram_dbe              : 1;  /**< Double-bit error for C2Q_LUT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.nonpse.pko_c2q_lut.pko_c2q_lut_ram_i */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t c2q_lut_ram_dbe              : 1;
#endif
	} s;
	struct cvmx_pko_lut_ecc_dbe_sts0_s    cn78xx;
};
typedef union cvmx_pko_lut_ecc_dbe_sts0 cvmx_pko_lut_ecc_dbe_sts0_t;

/**
 * cvmx_pko_lut_ecc_dbe_sts_cmb0
 */
union cvmx_pko_lut_ecc_dbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pko_lut_ecc_dbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t lut_dbe_cmb0                 : 1;  /**< This bit is the logical OR of all bits in PKO_LUT_ECC_DBE_STS.
                                                         To clear this bit, software must clear bits in PKO_LUT_ECC_DBE_STS.
                                                         When this bit is set, the corresponding interrupt is set.
                                                         Throws PKO_INTSN_E::PKO_LUT_DBE_CMB0.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.nonpse.pko_c2q_lut.pko_c2q_lut_ram_i */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t lut_dbe_cmb0                 : 1;
#endif
	} s;
	struct cvmx_pko_lut_ecc_dbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pko_lut_ecc_dbe_sts_cmb0 cvmx_pko_lut_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pko_lut_ecc_sbe_sts0
 */
union cvmx_pko_lut_ecc_sbe_sts0 {
	uint64_t u64;
	struct cvmx_pko_lut_ecc_sbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t c2q_lut_ram_sbe              : 1;  /**< Single-bit error for C2Q_LUT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.nonpse.pko_c2q_lut.pko_c2q_lut_ram_i */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t c2q_lut_ram_sbe              : 1;
#endif
	} s;
	struct cvmx_pko_lut_ecc_sbe_sts0_s    cn78xx;
};
typedef union cvmx_pko_lut_ecc_sbe_sts0 cvmx_pko_lut_ecc_sbe_sts0_t;

/**
 * cvmx_pko_lut_ecc_sbe_sts_cmb0
 */
union cvmx_pko_lut_ecc_sbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pko_lut_ecc_sbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t lut_sbe_cmb0                 : 1;  /**< This bit is the logical OR of all bits in PKO_LUT_ECC_SBE_STS.
                                                         To clear this bit, software must clear bits in PKO_LUT_ECC_SBE_STS.
                                                         When this bit is set, the corresponding interrupt is set.
                                                         Throws PKO_INTSN_E::PKO_LUT_SBE_CMB0.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.nonpse.pko_c2q_lut.pko_c2q_lut_ram_i */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t lut_sbe_cmb0                 : 1;
#endif
	} s;
	struct cvmx_pko_lut_ecc_sbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pko_lut_ecc_sbe_sts_cmb0 cvmx_pko_lut_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pko_mac#_cfg
 */
union cvmx_pko_macx_cfg {
	uint64_t u64;
	struct cvmx_pko_macx_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_17_63               : 47;
	uint64_t min_pad_ena                  : 1;  /**< Minimum padding is enabled for this MAC/FIFO */
	uint64_t fcs_ena                      : 1;  /**< Enable outside FCS for this MAC/FIFO */
	uint64_t fcs_sop_off                  : 8;  /**< FCS start of packet offset.  For this MAC, the number of bytes in the front
                                                         of each packet to exclude from FCS. */
	uint64_t skid_max_cnt                 : 2;  /**< Maximum number of SKID credits. 0x0 = 16; 0x1 = 32; 0x2 = 64. */
	uint64_t fifo_num                     : 5;  /**< The PEB TX FIFO number assigned to the given MAC. A value of 0x1F means unassigned. Unused
                                                         MACs must be assigned a FIFO_NUM = 0x1F. For each active MAC, a unique FIFO_NUM must be
                                                         assigned. Legal values depend on the values in PKO_PTGF(0..7)_CFG[SIZE]. Assigning the
                                                         same FIFO_NUM to more than a single active MAC will have unpredictable results.  FIFOs
                                                         0x1E and 0x1D are invalid and will cause unpredictable results if used. */
#else
	uint64_t fifo_num                     : 5;
	uint64_t skid_max_cnt                 : 2;
	uint64_t fcs_sop_off                  : 8;
	uint64_t fcs_ena                      : 1;
	uint64_t min_pad_ena                  : 1;
	uint64_t reserved_17_63               : 47;
#endif
	} s;
	struct cvmx_pko_macx_cfg_s            cn78xx;
};
typedef union cvmx_pko_macx_cfg cvmx_pko_macx_cfg_t;

/**
 * cvmx_pko_mci0_cred_cnt#
 */
union cvmx_pko_mci0_cred_cntx {
	uint64_t u64;
	struct cvmx_pko_mci0_cred_cntx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_13_63               : 51;
	uint64_t cred_cnt                     : 13; /**< Credit count. */
#else
	uint64_t cred_cnt                     : 13;
	uint64_t reserved_13_63               : 51;
#endif
	} s;
	struct cvmx_pko_mci0_cred_cntx_s      cn78xx;
};
typedef union cvmx_pko_mci0_cred_cntx cvmx_pko_mci0_cred_cntx_t;

/**
 * cvmx_pko_mci0_max_cred#
 */
union cvmx_pko_mci0_max_credx {
	uint64_t u64;
	struct cvmx_pko_mci0_max_credx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t max_cred_lim                 : 12; /**< Max credit limit. */
#else
	uint64_t max_cred_lim                 : 12;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_pko_mci0_max_credx_s      cn78xx;
};
typedef union cvmx_pko_mci0_max_credx cvmx_pko_mci0_max_credx_t;

/**
 * cvmx_pko_mci1_cred_cnt#
 */
union cvmx_pko_mci1_cred_cntx {
	uint64_t u64;
	struct cvmx_pko_mci1_cred_cntx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_13_63               : 51;
	uint64_t cred_cnt                     : 13; /**< Credit count. */
#else
	uint64_t cred_cnt                     : 13;
	uint64_t reserved_13_63               : 51;
#endif
	} s;
	struct cvmx_pko_mci1_cred_cntx_s      cn78xx;
};
typedef union cvmx_pko_mci1_cred_cntx cvmx_pko_mci1_cred_cntx_t;

/**
 * cvmx_pko_mci1_max_cred#
 */
union cvmx_pko_mci1_max_credx {
	uint64_t u64;
	struct cvmx_pko_mci1_max_credx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t max_cred_lim                 : 12; /**< Max credit limit. */
#else
	uint64_t max_cred_lim                 : 12;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_pko_mci1_max_credx_s      cn78xx;
};
typedef union cvmx_pko_mci1_max_credx cvmx_pko_mci1_max_credx_t;

/**
 * cvmx_pko_mem_count0
 *
 * Notes:
 * Total number of packets seen by PKO, per port
 * A write to this address will clear the entry whose index is specified as COUNT[5:0].
 * This CSR is a memory of 44 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  A read of any entry that has not been
 * previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_count0 {
	uint64_t u64;
	struct cvmx_pko_mem_count0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t count                        : 32; /**< Total number of packets seen by PKO */
#else
	uint64_t count                        : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_pko_mem_count0_s          cn30xx;
	struct cvmx_pko_mem_count0_s          cn31xx;
	struct cvmx_pko_mem_count0_s          cn38xx;
	struct cvmx_pko_mem_count0_s          cn38xxp2;
	struct cvmx_pko_mem_count0_s          cn50xx;
	struct cvmx_pko_mem_count0_s          cn52xx;
	struct cvmx_pko_mem_count0_s          cn52xxp1;
	struct cvmx_pko_mem_count0_s          cn56xx;
	struct cvmx_pko_mem_count0_s          cn56xxp1;
	struct cvmx_pko_mem_count0_s          cn58xx;
	struct cvmx_pko_mem_count0_s          cn58xxp1;
	struct cvmx_pko_mem_count0_s          cn61xx;
	struct cvmx_pko_mem_count0_s          cn63xx;
	struct cvmx_pko_mem_count0_s          cn63xxp1;
	struct cvmx_pko_mem_count0_s          cn66xx;
	struct cvmx_pko_mem_count0_s          cn68xx;
	struct cvmx_pko_mem_count0_s          cn68xxp1;
	struct cvmx_pko_mem_count0_s          cn70xx;
	struct cvmx_pko_mem_count0_s          cnf71xx;
};
typedef union cvmx_pko_mem_count0 cvmx_pko_mem_count0_t;

/**
 * cvmx_pko_mem_count1
 *
 * Notes:
 * Total number of bytes seen by PKO, per port
 * A write to this address will clear the entry whose index is specified as COUNT[5:0].
 * This CSR is a memory of 44 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  A read of any entry that has not been
 * previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_count1 {
	uint64_t u64;
	struct cvmx_pko_mem_count1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t count                        : 48; /**< Total number of bytes seen by PKO */
#else
	uint64_t count                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_pko_mem_count1_s          cn30xx;
	struct cvmx_pko_mem_count1_s          cn31xx;
	struct cvmx_pko_mem_count1_s          cn38xx;
	struct cvmx_pko_mem_count1_s          cn38xxp2;
	struct cvmx_pko_mem_count1_s          cn50xx;
	struct cvmx_pko_mem_count1_s          cn52xx;
	struct cvmx_pko_mem_count1_s          cn52xxp1;
	struct cvmx_pko_mem_count1_s          cn56xx;
	struct cvmx_pko_mem_count1_s          cn56xxp1;
	struct cvmx_pko_mem_count1_s          cn58xx;
	struct cvmx_pko_mem_count1_s          cn58xxp1;
	struct cvmx_pko_mem_count1_s          cn61xx;
	struct cvmx_pko_mem_count1_s          cn63xx;
	struct cvmx_pko_mem_count1_s          cn63xxp1;
	struct cvmx_pko_mem_count1_s          cn66xx;
	struct cvmx_pko_mem_count1_s          cn68xx;
	struct cvmx_pko_mem_count1_s          cn68xxp1;
	struct cvmx_pko_mem_count1_s          cn70xx;
	struct cvmx_pko_mem_count1_s          cnf71xx;
};
typedef union cvmx_pko_mem_count1 cvmx_pko_mem_count1_t;

/**
 * cvmx_pko_mem_debug0
 *
 * Notes:
 * Internal per-port state intended for debug use only - pko_prt_psb.cmnd[63:0]
 * This CSR is a memory of 12 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug0 {
	uint64_t u64;
	struct cvmx_pko_mem_debug0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t fau                          : 28; /**< Fetch and add command words */
	uint64_t cmd                          : 14; /**< Command word */
	uint64_t segs                         : 6;  /**< Number of segments/gather size */
	uint64_t size                         : 16; /**< Packet length in bytes */
#else
	uint64_t size                         : 16;
	uint64_t segs                         : 6;
	uint64_t cmd                          : 14;
	uint64_t fau                          : 28;
#endif
	} s;
	struct cvmx_pko_mem_debug0_s          cn30xx;
	struct cvmx_pko_mem_debug0_s          cn31xx;
	struct cvmx_pko_mem_debug0_s          cn38xx;
	struct cvmx_pko_mem_debug0_s          cn38xxp2;
	struct cvmx_pko_mem_debug0_s          cn50xx;
	struct cvmx_pko_mem_debug0_s          cn52xx;
	struct cvmx_pko_mem_debug0_s          cn52xxp1;
	struct cvmx_pko_mem_debug0_s          cn56xx;
	struct cvmx_pko_mem_debug0_s          cn56xxp1;
	struct cvmx_pko_mem_debug0_s          cn58xx;
	struct cvmx_pko_mem_debug0_s          cn58xxp1;
	struct cvmx_pko_mem_debug0_s          cn61xx;
	struct cvmx_pko_mem_debug0_s          cn63xx;
	struct cvmx_pko_mem_debug0_s          cn63xxp1;
	struct cvmx_pko_mem_debug0_s          cn66xx;
	struct cvmx_pko_mem_debug0_s          cn68xx;
	struct cvmx_pko_mem_debug0_s          cn68xxp1;
	struct cvmx_pko_mem_debug0_s          cn70xx;
	struct cvmx_pko_mem_debug0_s          cnf71xx;
};
typedef union cvmx_pko_mem_debug0 cvmx_pko_mem_debug0_t;

/**
 * cvmx_pko_mem_debug1
 *
 * Notes:
 * Internal per-port state intended for debug use only - pko_prt_psb.curr[63:0]
 * This CSR is a memory of 12 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug1 {
	uint64_t u64;
	struct cvmx_pko_mem_debug1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t i                            : 1;  /**< "I"  value used for free operation */
	uint64_t back                         : 4;  /**< Back value used for free operation */
	uint64_t pool                         : 3;  /**< Pool value used for free operation */
	uint64_t size                         : 16; /**< Size in bytes */
	uint64_t ptr                          : 40; /**< Data pointer */
#else
	uint64_t ptr                          : 40;
	uint64_t size                         : 16;
	uint64_t pool                         : 3;
	uint64_t back                         : 4;
	uint64_t i                            : 1;
#endif
	} s;
	struct cvmx_pko_mem_debug1_s          cn30xx;
	struct cvmx_pko_mem_debug1_s          cn31xx;
	struct cvmx_pko_mem_debug1_s          cn38xx;
	struct cvmx_pko_mem_debug1_s          cn38xxp2;
	struct cvmx_pko_mem_debug1_s          cn50xx;
	struct cvmx_pko_mem_debug1_s          cn52xx;
	struct cvmx_pko_mem_debug1_s          cn52xxp1;
	struct cvmx_pko_mem_debug1_s          cn56xx;
	struct cvmx_pko_mem_debug1_s          cn56xxp1;
	struct cvmx_pko_mem_debug1_s          cn58xx;
	struct cvmx_pko_mem_debug1_s          cn58xxp1;
	struct cvmx_pko_mem_debug1_s          cn61xx;
	struct cvmx_pko_mem_debug1_s          cn63xx;
	struct cvmx_pko_mem_debug1_s          cn63xxp1;
	struct cvmx_pko_mem_debug1_s          cn66xx;
	struct cvmx_pko_mem_debug1_s          cn68xx;
	struct cvmx_pko_mem_debug1_s          cn68xxp1;
	struct cvmx_pko_mem_debug1_s          cn70xx;
	struct cvmx_pko_mem_debug1_s          cnf71xx;
};
typedef union cvmx_pko_mem_debug1 cvmx_pko_mem_debug1_t;

/**
 * cvmx_pko_mem_debug10
 *
 * Notes:
 * Internal per-engine state intended for debug use only - pko.dat.ptr.ptrs1, pko.dat.ptr.ptrs2
 * This CSR is a memory of 10 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug10 {
	uint64_t u64;
	struct cvmx_pko_mem_debug10_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_pko_mem_debug10_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t fau                          : 28; /**< Fetch and add command words */
	uint64_t cmd                          : 14; /**< Command word */
	uint64_t segs                         : 6;  /**< Number of segments/gather size */
	uint64_t size                         : 16; /**< Packet length in bytes */
#else
	uint64_t size                         : 16;
	uint64_t segs                         : 6;
	uint64_t cmd                          : 14;
	uint64_t fau                          : 28;
#endif
	} cn30xx;
	struct cvmx_pko_mem_debug10_cn30xx    cn31xx;
	struct cvmx_pko_mem_debug10_cn30xx    cn38xx;
	struct cvmx_pko_mem_debug10_cn30xx    cn38xxp2;
	struct cvmx_pko_mem_debug10_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_49_63               : 15;
	uint64_t ptrs1                        : 17; /**< Internal state */
	uint64_t reserved_17_31               : 15;
	uint64_t ptrs2                        : 17; /**< Internal state */
#else
	uint64_t ptrs2                        : 17;
	uint64_t reserved_17_31               : 15;
	uint64_t ptrs1                        : 17;
	uint64_t reserved_49_63               : 15;
#endif
	} cn50xx;
	struct cvmx_pko_mem_debug10_cn50xx    cn52xx;
	struct cvmx_pko_mem_debug10_cn50xx    cn52xxp1;
	struct cvmx_pko_mem_debug10_cn50xx    cn56xx;
	struct cvmx_pko_mem_debug10_cn50xx    cn56xxp1;
	struct cvmx_pko_mem_debug10_cn50xx    cn58xx;
	struct cvmx_pko_mem_debug10_cn50xx    cn58xxp1;
	struct cvmx_pko_mem_debug10_cn50xx    cn61xx;
	struct cvmx_pko_mem_debug10_cn50xx    cn63xx;
	struct cvmx_pko_mem_debug10_cn50xx    cn63xxp1;
	struct cvmx_pko_mem_debug10_cn50xx    cn66xx;
	struct cvmx_pko_mem_debug10_cn50xx    cn68xx;
	struct cvmx_pko_mem_debug10_cn50xx    cn68xxp1;
	struct cvmx_pko_mem_debug10_cn50xx    cn70xx;
	struct cvmx_pko_mem_debug10_cn50xx    cnf71xx;
};
typedef union cvmx_pko_mem_debug10 cvmx_pko_mem_debug10_t;

/**
 * cvmx_pko_mem_debug11
 *
 * Notes:
 * Internal per-engine state intended for debug use only - pko.out.sta.state[22:0]
 * This CSR is a memory of 10 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug11 {
	uint64_t u64;
	struct cvmx_pko_mem_debug11_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t i                            : 1;  /**< "I"  value used for free operation */
	uint64_t back                         : 4;  /**< Back value used for free operation */
	uint64_t pool                         : 3;  /**< Pool value used for free operation */
	uint64_t size                         : 16; /**< Size in bytes */
	uint64_t reserved_0_39                : 40;
#else
	uint64_t reserved_0_39                : 40;
	uint64_t size                         : 16;
	uint64_t pool                         : 3;
	uint64_t back                         : 4;
	uint64_t i                            : 1;
#endif
	} s;
	struct cvmx_pko_mem_debug11_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t i                            : 1;  /**< "I"  value used for free operation */
	uint64_t back                         : 4;  /**< Back value used for free operation */
	uint64_t pool                         : 3;  /**< Pool value used for free operation */
	uint64_t size                         : 16; /**< Size in bytes */
	uint64_t ptr                          : 40; /**< Data pointer */
#else
	uint64_t ptr                          : 40;
	uint64_t size                         : 16;
	uint64_t pool                         : 3;
	uint64_t back                         : 4;
	uint64_t i                            : 1;
#endif
	} cn30xx;
	struct cvmx_pko_mem_debug11_cn30xx    cn31xx;
	struct cvmx_pko_mem_debug11_cn30xx    cn38xx;
	struct cvmx_pko_mem_debug11_cn30xx    cn38xxp2;
	struct cvmx_pko_mem_debug11_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_23_63               : 41;
	uint64_t maj                          : 1;  /**< Internal state */
	uint64_t uid                          : 3;  /**< Internal state */
	uint64_t sop                          : 1;  /**< Internal state */
	uint64_t len                          : 1;  /**< Internal state */
	uint64_t chk                          : 1;  /**< Internal state */
	uint64_t cnt                          : 13; /**< Internal state */
	uint64_t mod                          : 3;  /**< Internal state */
#else
	uint64_t mod                          : 3;
	uint64_t cnt                          : 13;
	uint64_t chk                          : 1;
	uint64_t len                          : 1;
	uint64_t sop                          : 1;
	uint64_t uid                          : 3;
	uint64_t maj                          : 1;
	uint64_t reserved_23_63               : 41;
#endif
	} cn50xx;
	struct cvmx_pko_mem_debug11_cn50xx    cn52xx;
	struct cvmx_pko_mem_debug11_cn50xx    cn52xxp1;
	struct cvmx_pko_mem_debug11_cn50xx    cn56xx;
	struct cvmx_pko_mem_debug11_cn50xx    cn56xxp1;
	struct cvmx_pko_mem_debug11_cn50xx    cn58xx;
	struct cvmx_pko_mem_debug11_cn50xx    cn58xxp1;
	struct cvmx_pko_mem_debug11_cn50xx    cn61xx;
	struct cvmx_pko_mem_debug11_cn50xx    cn63xx;
	struct cvmx_pko_mem_debug11_cn50xx    cn63xxp1;
	struct cvmx_pko_mem_debug11_cn50xx    cn66xx;
	struct cvmx_pko_mem_debug11_cn50xx    cn68xx;
	struct cvmx_pko_mem_debug11_cn50xx    cn68xxp1;
	struct cvmx_pko_mem_debug11_cn50xx    cn70xx;
	struct cvmx_pko_mem_debug11_cn50xx    cnf71xx;
};
typedef union cvmx_pko_mem_debug11 cvmx_pko_mem_debug11_t;

/**
 * cvmx_pko_mem_debug12
 *
 * Notes:
 * Internal per-engine x4 state intended for debug use only - pko.out.ctl.cmnd[63:0]
 * This CSR is a memory of 40 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug12 {
	uint64_t u64;
	struct cvmx_pko_mem_debug12_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_pko_mem_debug12_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< WorkQ data or Store0 pointer */
#else
	uint64_t data                         : 64;
#endif
	} cn30xx;
	struct cvmx_pko_mem_debug12_cn30xx    cn31xx;
	struct cvmx_pko_mem_debug12_cn30xx    cn38xx;
	struct cvmx_pko_mem_debug12_cn30xx    cn38xxp2;
	struct cvmx_pko_mem_debug12_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t fau                          : 28; /**< Fetch and add command words */
	uint64_t cmd                          : 14; /**< Command word */
	uint64_t segs                         : 6;  /**< Number of segments/gather size */
	uint64_t size                         : 16; /**< Packet length in bytes */
#else
	uint64_t size                         : 16;
	uint64_t segs                         : 6;
	uint64_t cmd                          : 14;
	uint64_t fau                          : 28;
#endif
	} cn50xx;
	struct cvmx_pko_mem_debug12_cn50xx    cn52xx;
	struct cvmx_pko_mem_debug12_cn50xx    cn52xxp1;
	struct cvmx_pko_mem_debug12_cn50xx    cn56xx;
	struct cvmx_pko_mem_debug12_cn50xx    cn56xxp1;
	struct cvmx_pko_mem_debug12_cn50xx    cn58xx;
	struct cvmx_pko_mem_debug12_cn50xx    cn58xxp1;
	struct cvmx_pko_mem_debug12_cn50xx    cn61xx;
	struct cvmx_pko_mem_debug12_cn50xx    cn63xx;
	struct cvmx_pko_mem_debug12_cn50xx    cn63xxp1;
	struct cvmx_pko_mem_debug12_cn50xx    cn66xx;
	struct cvmx_pko_mem_debug12_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t state                        : 64; /**< Internal state */
#else
	uint64_t state                        : 64;
#endif
	} cn68xx;
	struct cvmx_pko_mem_debug12_cn68xx    cn68xxp1;
	struct cvmx_pko_mem_debug12_cn50xx    cn70xx;
	struct cvmx_pko_mem_debug12_cn50xx    cnf71xx;
};
typedef union cvmx_pko_mem_debug12 cvmx_pko_mem_debug12_t;

/**
 * cvmx_pko_mem_debug13
 *
 * Notes:
 * Internal per-engine x4 state intended for debug use only - pko.out.ctl.head[63:0]
 * This CSR is a memory of 40 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug13 {
	uint64_t u64;
	struct cvmx_pko_mem_debug13_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_pko_mem_debug13_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_51_63               : 13;
	uint64_t widx                         : 17; /**< PDB widx */
	uint64_t ridx2                        : 17; /**< PDB ridx2 */
	uint64_t widx2                        : 17; /**< PDB widx2 */
#else
	uint64_t widx2                        : 17;
	uint64_t ridx2                        : 17;
	uint64_t widx                         : 17;
	uint64_t reserved_51_63               : 13;
#endif
	} cn30xx;
	struct cvmx_pko_mem_debug13_cn30xx    cn31xx;
	struct cvmx_pko_mem_debug13_cn30xx    cn38xx;
	struct cvmx_pko_mem_debug13_cn30xx    cn38xxp2;
	struct cvmx_pko_mem_debug13_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t i                            : 1;  /**< "I"  value used for free operation */
	uint64_t back                         : 4;  /**< Back value used for free operation */
	uint64_t pool                         : 3;  /**< Pool value used for free operation */
	uint64_t size                         : 16; /**< Size in bytes */
	uint64_t ptr                          : 40; /**< Data pointer */
#else
	uint64_t ptr                          : 40;
	uint64_t size                         : 16;
	uint64_t pool                         : 3;
	uint64_t back                         : 4;
	uint64_t i                            : 1;
#endif
	} cn50xx;
	struct cvmx_pko_mem_debug13_cn50xx    cn52xx;
	struct cvmx_pko_mem_debug13_cn50xx    cn52xxp1;
	struct cvmx_pko_mem_debug13_cn50xx    cn56xx;
	struct cvmx_pko_mem_debug13_cn50xx    cn56xxp1;
	struct cvmx_pko_mem_debug13_cn50xx    cn58xx;
	struct cvmx_pko_mem_debug13_cn50xx    cn58xxp1;
	struct cvmx_pko_mem_debug13_cn50xx    cn61xx;
	struct cvmx_pko_mem_debug13_cn50xx    cn63xx;
	struct cvmx_pko_mem_debug13_cn50xx    cn63xxp1;
	struct cvmx_pko_mem_debug13_cn50xx    cn66xx;
	struct cvmx_pko_mem_debug13_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t state                        : 64; /**< Internal state */
#else
	uint64_t state                        : 64;
#endif
	} cn68xx;
	struct cvmx_pko_mem_debug13_cn68xx    cn68xxp1;
	struct cvmx_pko_mem_debug13_cn50xx    cn70xx;
	struct cvmx_pko_mem_debug13_cn50xx    cnf71xx;
};
typedef union cvmx_pko_mem_debug13 cvmx_pko_mem_debug13_t;

/**
 * cvmx_pko_mem_debug14
 *
 * Notes:
 * Internal per-port state intended for debug use only - pko.prt.psb.save[63:0]
 * This CSR is a memory of 132 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug14 {
	uint64_t u64;
	struct cvmx_pko_mem_debug14_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_pko_mem_debug14_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_17_63               : 47;
	uint64_t ridx                         : 17; /**< PDB ridx */
#else
	uint64_t ridx                         : 17;
	uint64_t reserved_17_63               : 47;
#endif
	} cn30xx;
	struct cvmx_pko_mem_debug14_cn30xx    cn31xx;
	struct cvmx_pko_mem_debug14_cn30xx    cn38xx;
	struct cvmx_pko_mem_debug14_cn30xx    cn38xxp2;
	struct cvmx_pko_mem_debug14_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Command words */
#else
	uint64_t data                         : 64;
#endif
	} cn52xx;
	struct cvmx_pko_mem_debug14_cn52xx    cn52xxp1;
	struct cvmx_pko_mem_debug14_cn52xx    cn56xx;
	struct cvmx_pko_mem_debug14_cn52xx    cn56xxp1;
	struct cvmx_pko_mem_debug14_cn52xx    cn61xx;
	struct cvmx_pko_mem_debug14_cn52xx    cn63xx;
	struct cvmx_pko_mem_debug14_cn52xx    cn63xxp1;
	struct cvmx_pko_mem_debug14_cn52xx    cn66xx;
	struct cvmx_pko_mem_debug14_cn52xx    cn70xx;
	struct cvmx_pko_mem_debug14_cn52xx    cnf71xx;
};
typedef union cvmx_pko_mem_debug14 cvmx_pko_mem_debug14_t;

/**
 * cvmx_pko_mem_debug2
 *
 * Notes:
 * Internal per-port state intended for debug use only - pko_prt_psb.head[63:0]
 * This CSR is a memory of 12 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug2 {
	uint64_t u64;
	struct cvmx_pko_mem_debug2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t i                            : 1;  /**< "I"  value used for free operation */
	uint64_t back                         : 4;  /**< Back value used for free operation */
	uint64_t pool                         : 3;  /**< Pool value used for free operation */
	uint64_t size                         : 16; /**< Size in bytes */
	uint64_t ptr                          : 40; /**< Data pointer */
#else
	uint64_t ptr                          : 40;
	uint64_t size                         : 16;
	uint64_t pool                         : 3;
	uint64_t back                         : 4;
	uint64_t i                            : 1;
#endif
	} s;
	struct cvmx_pko_mem_debug2_s          cn30xx;
	struct cvmx_pko_mem_debug2_s          cn31xx;
	struct cvmx_pko_mem_debug2_s          cn38xx;
	struct cvmx_pko_mem_debug2_s          cn38xxp2;
	struct cvmx_pko_mem_debug2_s          cn50xx;
	struct cvmx_pko_mem_debug2_s          cn52xx;
	struct cvmx_pko_mem_debug2_s          cn52xxp1;
	struct cvmx_pko_mem_debug2_s          cn56xx;
	struct cvmx_pko_mem_debug2_s          cn56xxp1;
	struct cvmx_pko_mem_debug2_s          cn58xx;
	struct cvmx_pko_mem_debug2_s          cn58xxp1;
	struct cvmx_pko_mem_debug2_s          cn61xx;
	struct cvmx_pko_mem_debug2_s          cn63xx;
	struct cvmx_pko_mem_debug2_s          cn63xxp1;
	struct cvmx_pko_mem_debug2_s          cn66xx;
	struct cvmx_pko_mem_debug2_s          cn68xx;
	struct cvmx_pko_mem_debug2_s          cn68xxp1;
	struct cvmx_pko_mem_debug2_s          cn70xx;
	struct cvmx_pko_mem_debug2_s          cnf71xx;
};
typedef union cvmx_pko_mem_debug2 cvmx_pko_mem_debug2_t;

/**
 * cvmx_pko_mem_debug3
 *
 * Notes:
 * Internal per-port state intended for debug use only - pko_prt_psb.resp[63:0]
 * This CSR is a memory of 12 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug3 {
	uint64_t u64;
	struct cvmx_pko_mem_debug3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_pko_mem_debug3_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t i                            : 1;  /**< "I"  value used for free operation */
	uint64_t back                         : 4;  /**< Back value used for free operation */
	uint64_t pool                         : 3;  /**< Pool value used for free operation */
	uint64_t size                         : 16; /**< Size in bytes */
	uint64_t ptr                          : 40; /**< Data pointer */
#else
	uint64_t ptr                          : 40;
	uint64_t size                         : 16;
	uint64_t pool                         : 3;
	uint64_t back                         : 4;
	uint64_t i                            : 1;
#endif
	} cn30xx;
	struct cvmx_pko_mem_debug3_cn30xx     cn31xx;
	struct cvmx_pko_mem_debug3_cn30xx     cn38xx;
	struct cvmx_pko_mem_debug3_cn30xx     cn38xxp2;
	struct cvmx_pko_mem_debug3_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< WorkQ data or Store0 pointer */
#else
	uint64_t data                         : 64;
#endif
	} cn50xx;
	struct cvmx_pko_mem_debug3_cn50xx     cn52xx;
	struct cvmx_pko_mem_debug3_cn50xx     cn52xxp1;
	struct cvmx_pko_mem_debug3_cn50xx     cn56xx;
	struct cvmx_pko_mem_debug3_cn50xx     cn56xxp1;
	struct cvmx_pko_mem_debug3_cn50xx     cn58xx;
	struct cvmx_pko_mem_debug3_cn50xx     cn58xxp1;
	struct cvmx_pko_mem_debug3_cn50xx     cn61xx;
	struct cvmx_pko_mem_debug3_cn50xx     cn63xx;
	struct cvmx_pko_mem_debug3_cn50xx     cn63xxp1;
	struct cvmx_pko_mem_debug3_cn50xx     cn66xx;
	struct cvmx_pko_mem_debug3_cn50xx     cn68xx;
	struct cvmx_pko_mem_debug3_cn50xx     cn68xxp1;
	struct cvmx_pko_mem_debug3_cn50xx     cn70xx;
	struct cvmx_pko_mem_debug3_cn50xx     cnf71xx;
};
typedef union cvmx_pko_mem_debug3 cvmx_pko_mem_debug3_t;

/**
 * cvmx_pko_mem_debug4
 *
 * Notes:
 * Internal per-port state intended for debug use only - pko_prt_psb.state[63:0]
 * This CSR is a memory of 12 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug4 {
	uint64_t u64;
	struct cvmx_pko_mem_debug4_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_pko_mem_debug4_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< WorkQ data or Store0 pointer */
#else
	uint64_t data                         : 64;
#endif
	} cn30xx;
	struct cvmx_pko_mem_debug4_cn30xx     cn31xx;
	struct cvmx_pko_mem_debug4_cn30xx     cn38xx;
	struct cvmx_pko_mem_debug4_cn30xx     cn38xxp2;
	struct cvmx_pko_mem_debug4_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t cmnd_segs                    : 3;  /**< Internal state */
	uint64_t cmnd_siz                     : 16; /**< Internal state */
	uint64_t cmnd_off                     : 6;  /**< Internal state */
	uint64_t uid                          : 3;  /**< Internal state */
	uint64_t dread_sop                    : 1;  /**< Internal state */
	uint64_t init_dwrite                  : 1;  /**< Internal state */
	uint64_t chk_once                     : 1;  /**< Internal state */
	uint64_t chk_mode                     : 1;  /**< Internal state */
	uint64_t active                       : 1;  /**< Internal state */
	uint64_t static_p                     : 1;  /**< Internal state */
	uint64_t qos                          : 3;  /**< Internal state */
	uint64_t qcb_ridx                     : 5;  /**< Internal state */
	uint64_t qid_off_max                  : 4;  /**< Internal state */
	uint64_t qid_off                      : 4;  /**< Internal state */
	uint64_t qid_base                     : 8;  /**< Internal state */
	uint64_t wait                         : 1;  /**< Internal state */
	uint64_t minor                        : 2;  /**< Internal state */
	uint64_t major                        : 3;  /**< Internal state */
#else
	uint64_t major                        : 3;
	uint64_t minor                        : 2;
	uint64_t wait                         : 1;
	uint64_t qid_base                     : 8;
	uint64_t qid_off                      : 4;
	uint64_t qid_off_max                  : 4;
	uint64_t qcb_ridx                     : 5;
	uint64_t qos                          : 3;
	uint64_t static_p                     : 1;
	uint64_t active                       : 1;
	uint64_t chk_mode                     : 1;
	uint64_t chk_once                     : 1;
	uint64_t init_dwrite                  : 1;
	uint64_t dread_sop                    : 1;
	uint64_t uid                          : 3;
	uint64_t cmnd_off                     : 6;
	uint64_t cmnd_siz                     : 16;
	uint64_t cmnd_segs                    : 3;
#endif
	} cn50xx;
	struct cvmx_pko_mem_debug4_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t curr_siz                     : 8;  /**< Internal state */
	uint64_t curr_off                     : 16; /**< Internal state */
	uint64_t cmnd_segs                    : 6;  /**< Internal state */
	uint64_t cmnd_siz                     : 16; /**< Internal state */
	uint64_t cmnd_off                     : 6;  /**< Internal state */
	uint64_t uid                          : 2;  /**< Internal state */
	uint64_t dread_sop                    : 1;  /**< Internal state */
	uint64_t init_dwrite                  : 1;  /**< Internal state */
	uint64_t chk_once                     : 1;  /**< Internal state */
	uint64_t chk_mode                     : 1;  /**< Internal state */
	uint64_t wait                         : 1;  /**< Internal state */
	uint64_t minor                        : 2;  /**< Internal state */
	uint64_t major                        : 3;  /**< Internal state */
#else
	uint64_t major                        : 3;
	uint64_t minor                        : 2;
	uint64_t wait                         : 1;
	uint64_t chk_mode                     : 1;
	uint64_t chk_once                     : 1;
	uint64_t init_dwrite                  : 1;
	uint64_t dread_sop                    : 1;
	uint64_t uid                          : 2;
	uint64_t cmnd_off                     : 6;
	uint64_t cmnd_siz                     : 16;
	uint64_t cmnd_segs                    : 6;
	uint64_t curr_off                     : 16;
	uint64_t curr_siz                     : 8;
#endif
	} cn52xx;
	struct cvmx_pko_mem_debug4_cn52xx     cn52xxp1;
	struct cvmx_pko_mem_debug4_cn52xx     cn56xx;
	struct cvmx_pko_mem_debug4_cn52xx     cn56xxp1;
	struct cvmx_pko_mem_debug4_cn50xx     cn58xx;
	struct cvmx_pko_mem_debug4_cn50xx     cn58xxp1;
	struct cvmx_pko_mem_debug4_cn52xx     cn61xx;
	struct cvmx_pko_mem_debug4_cn52xx     cn63xx;
	struct cvmx_pko_mem_debug4_cn52xx     cn63xxp1;
	struct cvmx_pko_mem_debug4_cn52xx     cn66xx;
	struct cvmx_pko_mem_debug4_cn52xx     cn68xx;
	struct cvmx_pko_mem_debug4_cn52xx     cn68xxp1;
	struct cvmx_pko_mem_debug4_cn52xx     cn70xx;
	struct cvmx_pko_mem_debug4_cn52xx     cnf71xx;
};
typedef union cvmx_pko_mem_debug4 cvmx_pko_mem_debug4_t;

/**
 * cvmx_pko_mem_debug5
 *
 * Notes:
 * Internal per-port state intended for debug use only - pko_prt_psb.state[127:64]
 * This CSR is a memory of 12 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug5 {
	uint64_t u64;
	struct cvmx_pko_mem_debug5_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_pko_mem_debug5_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dwri_mod                     : 1;  /**< Dwrite mod */
	uint64_t dwri_sop                     : 1;  /**< Dwrite sop needed */
	uint64_t dwri_len                     : 1;  /**< Dwrite len */
	uint64_t dwri_cnt                     : 13; /**< Dwrite count */
	uint64_t cmnd_siz                     : 16; /**< Copy of cmnd.size */
	uint64_t uid                          : 1;  /**< UID */
	uint64_t xfer_wor                     : 1;  /**< Transfer work needed */
	uint64_t xfer_dwr                     : 1;  /**< Transfer dwrite needed */
	uint64_t cbuf_fre                     : 1;  /**< Cbuf needs free */
	uint64_t reserved_27_27               : 1;
	uint64_t chk_mode                     : 1;  /**< Checksum mode */
	uint64_t active                       : 1;  /**< Port is active */
	uint64_t qos                          : 3;  /**< Current QOS round */
	uint64_t qcb_ridx                     : 5;  /**< Buffer read  index for QCB */
	uint64_t qid_off                      : 3;  /**< Offset to be added to QID_BASE for current queue */
	uint64_t qid_base                     : 7;  /**< Absolute QID of the queue array base = &QUEUES[0] */
	uint64_t wait                         : 1;  /**< State wait when set */
	uint64_t minor                        : 2;  /**< State minor code */
	uint64_t major                        : 4;  /**< State major code */
#else
	uint64_t major                        : 4;
	uint64_t minor                        : 2;
	uint64_t wait                         : 1;
	uint64_t qid_base                     : 7;
	uint64_t qid_off                      : 3;
	uint64_t qcb_ridx                     : 5;
	uint64_t qos                          : 3;
	uint64_t active                       : 1;
	uint64_t chk_mode                     : 1;
	uint64_t reserved_27_27               : 1;
	uint64_t cbuf_fre                     : 1;
	uint64_t xfer_dwr                     : 1;
	uint64_t xfer_wor                     : 1;
	uint64_t uid                          : 1;
	uint64_t cmnd_siz                     : 16;
	uint64_t dwri_cnt                     : 13;
	uint64_t dwri_len                     : 1;
	uint64_t dwri_sop                     : 1;
	uint64_t dwri_mod                     : 1;
#endif
	} cn30xx;
	struct cvmx_pko_mem_debug5_cn30xx     cn31xx;
	struct cvmx_pko_mem_debug5_cn30xx     cn38xx;
	struct cvmx_pko_mem_debug5_cn30xx     cn38xxp2;
	struct cvmx_pko_mem_debug5_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t curr_ptr                     : 29; /**< Internal state */
	uint64_t curr_siz                     : 16; /**< Internal state */
	uint64_t curr_off                     : 16; /**< Internal state */
	uint64_t cmnd_segs                    : 3;  /**< Internal state */
#else
	uint64_t cmnd_segs                    : 3;
	uint64_t curr_off                     : 16;
	uint64_t curr_siz                     : 16;
	uint64_t curr_ptr                     : 29;
#endif
	} cn50xx;
	struct cvmx_pko_mem_debug5_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_54_63               : 10;
	uint64_t nxt_inflt                    : 6;  /**< Internal state */
	uint64_t curr_ptr                     : 40; /**< Internal state */
	uint64_t curr_siz                     : 8;  /**< Internal state */
#else
	uint64_t curr_siz                     : 8;
	uint64_t curr_ptr                     : 40;
	uint64_t nxt_inflt                    : 6;
	uint64_t reserved_54_63               : 10;
#endif
	} cn52xx;
	struct cvmx_pko_mem_debug5_cn52xx     cn52xxp1;
	struct cvmx_pko_mem_debug5_cn52xx     cn56xx;
	struct cvmx_pko_mem_debug5_cn52xx     cn56xxp1;
	struct cvmx_pko_mem_debug5_cn50xx     cn58xx;
	struct cvmx_pko_mem_debug5_cn50xx     cn58xxp1;
	struct cvmx_pko_mem_debug5_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t ptp                          : 1;  /**< Internal state */
	uint64_t major_3                      : 1;  /**< Internal state */
	uint64_t nxt_inflt                    : 6;  /**< Internal state */
	uint64_t curr_ptr                     : 40; /**< Internal state */
	uint64_t curr_siz                     : 8;  /**< Internal state */
#else
	uint64_t curr_siz                     : 8;
	uint64_t curr_ptr                     : 40;
	uint64_t nxt_inflt                    : 6;
	uint64_t major_3                      : 1;
	uint64_t ptp                          : 1;
	uint64_t reserved_56_63               : 8;
#endif
	} cn61xx;
	struct cvmx_pko_mem_debug5_cn61xx     cn63xx;
	struct cvmx_pko_mem_debug5_cn61xx     cn63xxp1;
	struct cvmx_pko_mem_debug5_cn61xx     cn66xx;
	struct cvmx_pko_mem_debug5_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_57_63               : 7;
	uint64_t uid_2                        : 1;  /**< Internal state */
	uint64_t ptp                          : 1;  /**< Internal state */
	uint64_t major_3                      : 1;  /**< Internal state */
	uint64_t nxt_inflt                    : 6;  /**< Internal state */
	uint64_t curr_ptr                     : 40; /**< Internal state */
	uint64_t curr_siz                     : 8;  /**< Internal state */
#else
	uint64_t curr_siz                     : 8;
	uint64_t curr_ptr                     : 40;
	uint64_t nxt_inflt                    : 6;
	uint64_t major_3                      : 1;
	uint64_t ptp                          : 1;
	uint64_t uid_2                        : 1;
	uint64_t reserved_57_63               : 7;
#endif
	} cn68xx;
	struct cvmx_pko_mem_debug5_cn68xx     cn68xxp1;
	struct cvmx_pko_mem_debug5_cn61xx     cn70xx;
	struct cvmx_pko_mem_debug5_cn61xx     cnf71xx;
};
typedef union cvmx_pko_mem_debug5 cvmx_pko_mem_debug5_t;

/**
 * cvmx_pko_mem_debug6
 *
 * Notes:
 * Internal per-port state intended for debug use only - pko_prt_psb.port[63:0]
 * This CSR is a memory of 44 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug6 {
	uint64_t u64;
	struct cvmx_pko_mem_debug6_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_37_63               : 27;
	uint64_t qid_offres                   : 4;  /**< Internal state */
	uint64_t qid_offths                   : 4;  /**< Internal state */
	uint64_t preempter                    : 1;  /**< Internal state */
	uint64_t preemptee                    : 1;  /**< Internal state */
	uint64_t preempted                    : 1;  /**< Internal state */
	uint64_t active                       : 1;  /**< Internal state */
	uint64_t reserved_24_24               : 1;
	uint64_t qos                          : 3;  /**< Internal state */
	uint64_t qcb_ridx                     : 5;  /**< Internal state */
	uint64_t qid_offmax                   : 4;  /**< Internal state */
	uint64_t reserved_0_11                : 12;
#else
	uint64_t reserved_0_11                : 12;
	uint64_t qid_offmax                   : 4;
	uint64_t qcb_ridx                     : 5;
	uint64_t qos                          : 3;
	uint64_t reserved_24_24               : 1;
	uint64_t active                       : 1;
	uint64_t preempted                    : 1;
	uint64_t preemptee                    : 1;
	uint64_t preempter                    : 1;
	uint64_t qid_offths                   : 4;
	uint64_t qid_offres                   : 4;
	uint64_t reserved_37_63               : 27;
#endif
	} s;
	struct cvmx_pko_mem_debug6_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t qid_offm                     : 3;  /**< Qid offset max */
	uint64_t static_p                     : 1;  /**< Static port when set */
	uint64_t work_min                     : 3;  /**< Work minor */
	uint64_t dwri_chk                     : 1;  /**< Dwrite checksum mode */
	uint64_t dwri_uid                     : 1;  /**< Dwrite UID */
	uint64_t dwri_mod                     : 2;  /**< Dwrite mod */
#else
	uint64_t dwri_mod                     : 2;
	uint64_t dwri_uid                     : 1;
	uint64_t dwri_chk                     : 1;
	uint64_t work_min                     : 3;
	uint64_t static_p                     : 1;
	uint64_t qid_offm                     : 3;
	uint64_t reserved_11_63               : 53;
#endif
	} cn30xx;
	struct cvmx_pko_mem_debug6_cn30xx     cn31xx;
	struct cvmx_pko_mem_debug6_cn30xx     cn38xx;
	struct cvmx_pko_mem_debug6_cn30xx     cn38xxp2;
	struct cvmx_pko_mem_debug6_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t curr_ptr                     : 11; /**< Internal state */
#else
	uint64_t curr_ptr                     : 11;
	uint64_t reserved_11_63               : 53;
#endif
	} cn50xx;
	struct cvmx_pko_mem_debug6_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_37_63               : 27;
	uint64_t qid_offres                   : 4;  /**< Internal state */
	uint64_t qid_offths                   : 4;  /**< Internal state */
	uint64_t preempter                    : 1;  /**< Internal state */
	uint64_t preemptee                    : 1;  /**< Internal state */
	uint64_t preempted                    : 1;  /**< Internal state */
	uint64_t active                       : 1;  /**< Internal state */
	uint64_t statc                        : 1;  /**< Internal state */
	uint64_t qos                          : 3;  /**< Internal state */
	uint64_t qcb_ridx                     : 5;  /**< Internal state */
	uint64_t qid_offmax                   : 4;  /**< Internal state */
	uint64_t qid_off                      : 4;  /**< Internal state */
	uint64_t qid_base                     : 8;  /**< Internal state */
#else
	uint64_t qid_base                     : 8;
	uint64_t qid_off                      : 4;
	uint64_t qid_offmax                   : 4;
	uint64_t qcb_ridx                     : 5;
	uint64_t qos                          : 3;
	uint64_t statc                        : 1;
	uint64_t active                       : 1;
	uint64_t preempted                    : 1;
	uint64_t preemptee                    : 1;
	uint64_t preempter                    : 1;
	uint64_t qid_offths                   : 4;
	uint64_t qid_offres                   : 4;
	uint64_t reserved_37_63               : 27;
#endif
	} cn52xx;
	struct cvmx_pko_mem_debug6_cn52xx     cn52xxp1;
	struct cvmx_pko_mem_debug6_cn52xx     cn56xx;
	struct cvmx_pko_mem_debug6_cn52xx     cn56xxp1;
	struct cvmx_pko_mem_debug6_cn50xx     cn58xx;
	struct cvmx_pko_mem_debug6_cn50xx     cn58xxp1;
	struct cvmx_pko_mem_debug6_cn52xx     cn61xx;
	struct cvmx_pko_mem_debug6_cn52xx     cn63xx;
	struct cvmx_pko_mem_debug6_cn52xx     cn63xxp1;
	struct cvmx_pko_mem_debug6_cn52xx     cn66xx;
	struct cvmx_pko_mem_debug6_cn52xx     cn68xx;
	struct cvmx_pko_mem_debug6_cn52xx     cn68xxp1;
	struct cvmx_pko_mem_debug6_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_37_63               : 27;
	uint64_t qid_offres                   : 4;  /**< Internal state */
	uint64_t qid_offths                   : 4;  /**< Internal state */
	uint64_t preempter                    : 1;  /**< Internal state */
	uint64_t preemptee                    : 1;  /**< Internal state */
	uint64_t preempted                    : 1;  /**< Internal state */
	uint64_t active                       : 1;  /**< Internal state */
	uint64_t staticb                      : 1;  /**< Internal state */
	uint64_t qos                          : 3;  /**< Internal state */
	uint64_t qcb_ridx                     : 5;  /**< Internal state */
	uint64_t qid_offmax                   : 4;  /**< Internal state */
	uint64_t qid_off                      : 4;  /**< Internal state */
	uint64_t qid_base                     : 8;  /**< Internal state */
#else
	uint64_t qid_base                     : 8;
	uint64_t qid_off                      : 4;
	uint64_t qid_offmax                   : 4;
	uint64_t qcb_ridx                     : 5;
	uint64_t qos                          : 3;
	uint64_t staticb                      : 1;
	uint64_t active                       : 1;
	uint64_t preempted                    : 1;
	uint64_t preemptee                    : 1;
	uint64_t preempter                    : 1;
	uint64_t qid_offths                   : 4;
	uint64_t qid_offres                   : 4;
	uint64_t reserved_37_63               : 27;
#endif
	} cn70xx;
	struct cvmx_pko_mem_debug6_cn52xx     cnf71xx;
};
typedef union cvmx_pko_mem_debug6 cvmx_pko_mem_debug6_t;

/**
 * cvmx_pko_mem_debug7
 *
 * Notes:
 * Internal per-queue state intended for debug use only - pko_prt_qsb.state[63:0]
 * This CSR is a memory of 256 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug7 {
	uint64_t u64;
	struct cvmx_pko_mem_debug7_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_pko_mem_debug7_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_58_63               : 6;
	uint64_t dwb                          : 9;  /**< Calculated DWB count used for free operation */
	uint64_t start                        : 33; /**< Calculated start address used for free operation */
	uint64_t size                         : 16; /**< Packet length in bytes */
#else
	uint64_t size                         : 16;
	uint64_t start                        : 33;
	uint64_t dwb                          : 9;
	uint64_t reserved_58_63               : 6;
#endif
	} cn30xx;
	struct cvmx_pko_mem_debug7_cn30xx     cn31xx;
	struct cvmx_pko_mem_debug7_cn30xx     cn38xx;
	struct cvmx_pko_mem_debug7_cn30xx     cn38xxp2;
	struct cvmx_pko_mem_debug7_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t qos                          : 5;  /**< QOS mask to enable the queue when set */
	uint64_t tail                         : 1;  /**< This queue is the last (tail) in the queue array */
	uint64_t buf_siz                      : 13; /**< Command buffer remaining size in words */
	uint64_t buf_ptr                      : 33; /**< Command word pointer */
	uint64_t qcb_widx                     : 6;  /**< Buffer write index for QCB */
	uint64_t qcb_ridx                     : 6;  /**< Buffer read  index for QCB */
#else
	uint64_t qcb_ridx                     : 6;
	uint64_t qcb_widx                     : 6;
	uint64_t buf_ptr                      : 33;
	uint64_t buf_siz                      : 13;
	uint64_t tail                         : 1;
	uint64_t qos                          : 5;
#endif
	} cn50xx;
	struct cvmx_pko_mem_debug7_cn50xx     cn52xx;
	struct cvmx_pko_mem_debug7_cn50xx     cn52xxp1;
	struct cvmx_pko_mem_debug7_cn50xx     cn56xx;
	struct cvmx_pko_mem_debug7_cn50xx     cn56xxp1;
	struct cvmx_pko_mem_debug7_cn50xx     cn58xx;
	struct cvmx_pko_mem_debug7_cn50xx     cn58xxp1;
	struct cvmx_pko_mem_debug7_cn50xx     cn61xx;
	struct cvmx_pko_mem_debug7_cn50xx     cn63xx;
	struct cvmx_pko_mem_debug7_cn50xx     cn63xxp1;
	struct cvmx_pko_mem_debug7_cn50xx     cn66xx;
	struct cvmx_pko_mem_debug7_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t qos                          : 3;  /**< QOS mask to enable the queue when set */
	uint64_t tail                         : 1;  /**< This queue is the last (tail) in the queue array */
	uint64_t buf_siz                      : 13; /**< Command buffer remaining size in words */
	uint64_t buf_ptr                      : 33; /**< Command word pointer */
	uint64_t qcb_widx                     : 7;  /**< Buffer write index for QCB */
	uint64_t qcb_ridx                     : 7;  /**< Buffer read  index for QCB */
#else
	uint64_t qcb_ridx                     : 7;
	uint64_t qcb_widx                     : 7;
	uint64_t buf_ptr                      : 33;
	uint64_t buf_siz                      : 13;
	uint64_t tail                         : 1;
	uint64_t qos                          : 3;
#endif
	} cn68xx;
	struct cvmx_pko_mem_debug7_cn68xx     cn68xxp1;
	struct cvmx_pko_mem_debug7_cn50xx     cn70xx;
	struct cvmx_pko_mem_debug7_cn50xx     cnf71xx;
};
typedef union cvmx_pko_mem_debug7 cvmx_pko_mem_debug7_t;

/**
 * cvmx_pko_mem_debug8
 *
 * Notes:
 * Internal per-queue state intended for debug use only - pko_prt_qsb.state[91:64]
 * This CSR is a memory of 256 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug8 {
	uint64_t u64;
	struct cvmx_pko_mem_debug8_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_59_63               : 5;
	uint64_t tail                         : 1;  /**< This queue is the last (tail) in the queue array */
	uint64_t buf_siz                      : 13; /**< Command buffer remaining size in words */
	uint64_t reserved_0_44                : 45;
#else
	uint64_t reserved_0_44                : 45;
	uint64_t buf_siz                      : 13;
	uint64_t tail                         : 1;
	uint64_t reserved_59_63               : 5;
#endif
	} s;
	struct cvmx_pko_mem_debug8_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t qos                          : 5;  /**< QOS mask to enable the queue when set */
	uint64_t tail                         : 1;  /**< This queue is the last (tail) in the queue array */
	uint64_t buf_siz                      : 13; /**< Command buffer remaining size in words */
	uint64_t buf_ptr                      : 33; /**< Command word pointer */
	uint64_t qcb_widx                     : 6;  /**< Buffer write index for QCB */
	uint64_t qcb_ridx                     : 6;  /**< Buffer read  index for QCB */
#else
	uint64_t qcb_ridx                     : 6;
	uint64_t qcb_widx                     : 6;
	uint64_t buf_ptr                      : 33;
	uint64_t buf_siz                      : 13;
	uint64_t tail                         : 1;
	uint64_t qos                          : 5;
#endif
	} cn30xx;
	struct cvmx_pko_mem_debug8_cn30xx     cn31xx;
	struct cvmx_pko_mem_debug8_cn30xx     cn38xx;
	struct cvmx_pko_mem_debug8_cn30xx     cn38xxp2;
	struct cvmx_pko_mem_debug8_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t doorbell                     : 20; /**< Doorbell count */
	uint64_t reserved_6_7                 : 2;
	uint64_t static_p                     : 1;  /**< Static priority */
	uint64_t s_tail                       : 1;  /**< Static tail */
	uint64_t static_q                     : 1;  /**< Static priority */
	uint64_t qos                          : 3;  /**< QOS mask to enable the queue when set */
#else
	uint64_t qos                          : 3;
	uint64_t static_q                     : 1;
	uint64_t s_tail                       : 1;
	uint64_t static_p                     : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t doorbell                     : 20;
	uint64_t reserved_28_63               : 36;
#endif
	} cn50xx;
	struct cvmx_pko_mem_debug8_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t preempter                    : 1;  /**< Preempter */
	uint64_t doorbell                     : 20; /**< Doorbell count */
	uint64_t reserved_7_7                 : 1;
	uint64_t preemptee                    : 1;  /**< Preemptee */
	uint64_t static_p                     : 1;  /**< Static priority */
	uint64_t s_tail                       : 1;  /**< Static tail */
	uint64_t static_q                     : 1;  /**< Static priority */
	uint64_t qos                          : 3;  /**< QOS mask to enable the queue when set */
#else
	uint64_t qos                          : 3;
	uint64_t static_q                     : 1;
	uint64_t s_tail                       : 1;
	uint64_t static_p                     : 1;
	uint64_t preemptee                    : 1;
	uint64_t reserved_7_7                 : 1;
	uint64_t doorbell                     : 20;
	uint64_t preempter                    : 1;
	uint64_t reserved_29_63               : 35;
#endif
	} cn52xx;
	struct cvmx_pko_mem_debug8_cn52xx     cn52xxp1;
	struct cvmx_pko_mem_debug8_cn52xx     cn56xx;
	struct cvmx_pko_mem_debug8_cn52xx     cn56xxp1;
	struct cvmx_pko_mem_debug8_cn50xx     cn58xx;
	struct cvmx_pko_mem_debug8_cn50xx     cn58xxp1;
	struct cvmx_pko_mem_debug8_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t qid_qqos                     : 8;  /**< QOS_MASK */
	uint64_t reserved_33_33               : 1;
	uint64_t qid_idx                      : 4;  /**< IDX */
	uint64_t preempter                    : 1;  /**< Preempter */
	uint64_t doorbell                     : 20; /**< Doorbell count */
	uint64_t reserved_7_7                 : 1;
	uint64_t preemptee                    : 1;  /**< Preemptee */
	uint64_t static_p                     : 1;  /**< Static priority */
	uint64_t s_tail                       : 1;  /**< Static tail */
	uint64_t static_q                     : 1;  /**< Static priority */
	uint64_t qos                          : 3;  /**< QOS mask to enable the queue when set */
#else
	uint64_t qos                          : 3;
	uint64_t static_q                     : 1;
	uint64_t s_tail                       : 1;
	uint64_t static_p                     : 1;
	uint64_t preemptee                    : 1;
	uint64_t reserved_7_7                 : 1;
	uint64_t doorbell                     : 20;
	uint64_t preempter                    : 1;
	uint64_t qid_idx                      : 4;
	uint64_t reserved_33_33               : 1;
	uint64_t qid_qqos                     : 8;
	uint64_t reserved_42_63               : 22;
#endif
	} cn61xx;
	struct cvmx_pko_mem_debug8_cn52xx     cn63xx;
	struct cvmx_pko_mem_debug8_cn52xx     cn63xxp1;
	struct cvmx_pko_mem_debug8_cn61xx     cn66xx;
	struct cvmx_pko_mem_debug8_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_37_63               : 27;
	uint64_t preempter                    : 1;  /**< Preempter */
	uint64_t doorbell                     : 20; /**< Doorbell count */
	uint64_t reserved_9_15                : 7;
	uint64_t preemptee                    : 1;  /**< Preemptee */
	uint64_t static_p                     : 1;  /**< Static priority */
	uint64_t s_tail                       : 1;  /**< Static tail */
	uint64_t static_q                     : 1;  /**< Static priority */
	uint64_t qos                          : 5;  /**< QOS mask to enable the queue when set */
#else
	uint64_t qos                          : 5;
	uint64_t static_q                     : 1;
	uint64_t s_tail                       : 1;
	uint64_t static_p                     : 1;
	uint64_t preemptee                    : 1;
	uint64_t reserved_9_15                : 7;
	uint64_t doorbell                     : 20;
	uint64_t preempter                    : 1;
	uint64_t reserved_37_63               : 27;
#endif
	} cn68xx;
	struct cvmx_pko_mem_debug8_cn68xx     cn68xxp1;
	struct cvmx_pko_mem_debug8_cn61xx     cn70xx;
	struct cvmx_pko_mem_debug8_cn61xx     cnf71xx;
};
typedef union cvmx_pko_mem_debug8 cvmx_pko_mem_debug8_t;

/**
 * cvmx_pko_mem_debug9
 *
 * Notes:
 * Internal per-engine state intended for debug use only - pko.dat.ptr.ptrs0, pko.dat.ptr.ptrs3
 * This CSR is a memory of 10 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_pko_mem_debug9 {
	uint64_t u64;
	struct cvmx_pko_mem_debug9_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_49_63               : 15;
	uint64_t ptrs0                        : 17; /**< Internal state */
	uint64_t reserved_0_31                : 32;
#else
	uint64_t reserved_0_31                : 32;
	uint64_t ptrs0                        : 17;
	uint64_t reserved_49_63               : 15;
#endif
	} s;
	struct cvmx_pko_mem_debug9_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t doorbell                     : 20; /**< Doorbell count */
	uint64_t reserved_5_7                 : 3;
	uint64_t s_tail                       : 1;  /**< reads as zero (S_TAIL cannot be read) */
	uint64_t static_q                     : 1;  /**< reads as zero (STATIC_Q cannot be read) */
	uint64_t qos                          : 3;  /**< QOS mask to enable the queue when set */
#else
	uint64_t qos                          : 3;
	uint64_t static_q                     : 1;
	uint64_t s_tail                       : 1;
	uint64_t reserved_5_7                 : 3;
	uint64_t doorbell                     : 20;
	uint64_t reserved_28_63               : 36;
#endif
	} cn30xx;
	struct cvmx_pko_mem_debug9_cn30xx     cn31xx;
	struct cvmx_pko_mem_debug9_cn38xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t doorbell                     : 20; /**< Doorbell count */
	uint64_t reserved_6_7                 : 2;
	uint64_t static_p                     : 1;  /**< Static priority (port) */
	uint64_t s_tail                       : 1;  /**< Static tail */
	uint64_t static_q                     : 1;  /**< Static priority */
	uint64_t qos                          : 3;  /**< QOS mask to enable the queue when set */
#else
	uint64_t qos                          : 3;
	uint64_t static_q                     : 1;
	uint64_t s_tail                       : 1;
	uint64_t static_p                     : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t doorbell                     : 20;
	uint64_t reserved_28_63               : 36;
#endif
	} cn38xx;
	struct cvmx_pko_mem_debug9_cn38xx     cn38xxp2;
	struct cvmx_pko_mem_debug9_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_49_63               : 15;
	uint64_t ptrs0                        : 17; /**< Internal state */
	uint64_t reserved_17_31               : 15;
	uint64_t ptrs3                        : 17; /**< Internal state */
#else
	uint64_t ptrs3                        : 17;
	uint64_t reserved_17_31               : 15;
	uint64_t ptrs0                        : 17;
	uint64_t reserved_49_63               : 15;
#endif
	} cn50xx;
	struct cvmx_pko_mem_debug9_cn50xx     cn52xx;
	struct cvmx_pko_mem_debug9_cn50xx     cn52xxp1;
	struct cvmx_pko_mem_debug9_cn50xx     cn56xx;
	struct cvmx_pko_mem_debug9_cn50xx     cn56xxp1;
	struct cvmx_pko_mem_debug9_cn50xx     cn58xx;
	struct cvmx_pko_mem_debug9_cn50xx     cn58xxp1;
	struct cvmx_pko_mem_debug9_cn50xx     cn61xx;
	struct cvmx_pko_mem_debug9_cn50xx     cn63xx;
	struct cvmx_pko_mem_debug9_cn50xx     cn63xxp1;
	struct cvmx_pko_mem_debug9_cn50xx     cn66xx;
	struct cvmx_pko_mem_debug9_cn50xx     cn68xx;
	struct cvmx_pko_mem_debug9_cn50xx     cn68xxp1;
	struct cvmx_pko_mem_debug9_cn50xx     cn70xx;
	struct cvmx_pko_mem_debug9_cn50xx     cnf71xx;
};
typedef union cvmx_pko_mem_debug9 cvmx_pko_mem_debug9_t;

/**
 * cvmx_pko_mem_iport_ptrs
 *
 * Notes:
 * This CSR is a memory of 128 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  The index to this CSR is an IPORT.  A read of any
 * entry that has not been previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_iport_ptrs {
	uint64_t u64;
	struct cvmx_pko_mem_iport_ptrs_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_63_63               : 1;
	uint64_t crc                          : 1;  /**< Set if this IPID uses CRC */
	uint64_t static_p                     : 1;  /**< Set if this IPID has static priority */
	uint64_t qos_mask                     : 8;  /**< Mask to control priority across 8 QOS rounds */
	uint64_t min_pkt                      : 3;  /**< Min packet size specified by PKO_REG_MIN_PKT[MIN_PKT] */
	uint64_t reserved_31_49               : 19;
	uint64_t pipe                         : 7;  /**< The PKO pipe or loopback port
                                                         When INT != PIP/IPD:
                                                          PIPE is the PKO pipe to which this port is mapped
                                                          All used PKO-internal ports that map to the same
                                                          PIPE must also map to the same INT and EID in
                                                          this case.
                                                         When INT == PIP/IPD:
                                                          PIPE must be in the range
                                                                  0..PKO_REG_LOOPBACK[NUM_PORTS]-1
                                                          in this case and selects one of the loopback
                                                          ports. */
	uint64_t reserved_21_23               : 3;
	uint64_t intr                         : 5;  /**< The interface to which this port is mapped
                                                         All used PKO-internal ports that map to the same EID
                                                         must also map to the same INT. All used PKO-internal
                                                         ports that map to the same INT must also map to the
                                                         same EID.
                                                         Encoding:
                                                           0 = GMX0 XAUI/DXAUI/RXAUI0 or SGMII0
                                                           1 = GMX0 SGMII1
                                                           2 = GMX0 SGMII2
                                                           3 = GMX0 SGMII3
                                                           4 = GMX1 RXAUI
                                                           8 = GMX2 XAUI/DXAUI or SGMII0
                                                           9 = GMX2 SGMII1
                                                          10 = GMX2 SGMII2
                                                          11 = GMX2 SGMII3
                                                          12 = GMX3 XAUI/DXAUI or SGMII0
                                                          13 = GMX3 SGMII1
                                                          14 = GMX3 SGMII2
                                                          15 = GMX3 SGMII3
                                                          16 = GMX4 XAUI/DXAUI or SGMII0
                                                          17 = GMX4 SGMII1
                                                          18 = GMX4 SGMII2
                                                          19 = GMX4 SGMII3
                                                          28 = ILK interface 0
                                                          29 = ILK interface 1
                                                          30 = DPI
                                                          31 = PIP/IPD
                                                          others = reserved */
	uint64_t reserved_13_15               : 3;
	uint64_t eid                          : 5;  /**< Engine ID to which this port is mapped
                                                         EID==31 can be used with unused PKO-internal ports.
                                                         Otherwise, 0-19 are legal EID values. */
	uint64_t reserved_7_7                 : 1;
	uint64_t ipid                         : 7;  /**< PKO-internal Port ID to be accessed */
#else
	uint64_t ipid                         : 7;
	uint64_t reserved_7_7                 : 1;
	uint64_t eid                          : 5;
	uint64_t reserved_13_15               : 3;
	uint64_t intr                         : 5;
	uint64_t reserved_21_23               : 3;
	uint64_t pipe                         : 7;
	uint64_t reserved_31_49               : 19;
	uint64_t min_pkt                      : 3;
	uint64_t qos_mask                     : 8;
	uint64_t static_p                     : 1;
	uint64_t crc                          : 1;
	uint64_t reserved_63_63               : 1;
#endif
	} s;
	struct cvmx_pko_mem_iport_ptrs_s      cn68xx;
	struct cvmx_pko_mem_iport_ptrs_s      cn68xxp1;
};
typedef union cvmx_pko_mem_iport_ptrs cvmx_pko_mem_iport_ptrs_t;

/**
 * cvmx_pko_mem_iport_qos
 *
 * Notes:
 * Sets the QOS mask, per port.  These QOS_MASK bits are logically and physically the same QOS_MASK
 * bits in PKO_MEM_IPORT_PTRS.  This CSR address allows the QOS_MASK bits to be written during PKO
 * operation without affecting any other port state.  The engine to which port PID is mapped is engine
 * EID.  Note that the port to engine mapping must be the same as was previously programmed via the
 * PKO_MEM_IPORT_PTRS CSR.
 * This CSR is a memory of 128 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  The index to this CSR is an IPORT.  A read of
 * any entry that has not been previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_iport_qos {
	uint64_t u64;
	struct cvmx_pko_mem_iport_qos_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_61_63               : 3;
	uint64_t qos_mask                     : 8;  /**< Mask to control priority across 8 QOS rounds */
	uint64_t reserved_13_52               : 40;
	uint64_t eid                          : 5;  /**< Engine ID to which this port is mapped */
	uint64_t reserved_7_7                 : 1;
	uint64_t ipid                         : 7;  /**< PKO-internal Port ID */
#else
	uint64_t ipid                         : 7;
	uint64_t reserved_7_7                 : 1;
	uint64_t eid                          : 5;
	uint64_t reserved_13_52               : 40;
	uint64_t qos_mask                     : 8;
	uint64_t reserved_61_63               : 3;
#endif
	} s;
	struct cvmx_pko_mem_iport_qos_s       cn68xx;
	struct cvmx_pko_mem_iport_qos_s       cn68xxp1;
};
typedef union cvmx_pko_mem_iport_qos cvmx_pko_mem_iport_qos_t;

/**
 * cvmx_pko_mem_iqueue_ptrs
 *
 * Notes:
 * Sets the queue to port mapping and the initial command buffer pointer, per queue.  Unused queues must
 * set BUF_PTR=0.  Each queue may map to at most one port.  No more than 32 queues may map to a port.
 * The set of queues that is mapped to a port must be a contiguous array of queues.  The port to which
 * queue QID is mapped is port IPID.  The index of queue QID in port IPID's queue list is IDX.  The last
 * queue in port IPID's queue array must have its TAIL bit set.
 * STATIC_Q marks queue QID as having static priority.  STATIC_P marks the port IPID to which QID is
 * mapped as having at least one queue with static priority.  If any QID that maps to IPID has static
 * priority, then all QID that map to IPID must have STATIC_P set.  Queues marked as static priority
 * must be contiguous and begin at IDX 0.  The last queue that is marked as having static priority
 * must have its S_TAIL bit set.
 * This CSR is a memory of 256 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  The index to this CSR is an IQUEUE.  A read of any
 * entry that has not been previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_iqueue_ptrs {
	uint64_t u64;
	struct cvmx_pko_mem_iqueue_ptrs_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t s_tail                       : 1;  /**< Set if this QID is the tail of the static queues */
	uint64_t static_p                     : 1;  /**< Set if any QID in this IPID has static priority */
	uint64_t static_q                     : 1;  /**< Set if this QID has static priority */
	uint64_t qos_mask                     : 8;  /**< Mask to control priority across 8 QOS rounds */
	uint64_t buf_ptr                      : 31; /**< Command buffer pointer[37:7] */
	uint64_t tail                         : 1;  /**< Set if this QID is the tail of the queue array */
	uint64_t index                        : 5;  /**< Index (distance from head) in the queue array */
	uint64_t reserved_15_15               : 1;
	uint64_t ipid                         : 7;  /**< PKO-Internal Port ID to which this queue is mapped */
	uint64_t qid                          : 8;  /**< Queue ID */
#else
	uint64_t qid                          : 8;
	uint64_t ipid                         : 7;
	uint64_t reserved_15_15               : 1;
	uint64_t index                        : 5;
	uint64_t tail                         : 1;
	uint64_t buf_ptr                      : 31;
	uint64_t qos_mask                     : 8;
	uint64_t static_q                     : 1;
	uint64_t static_p                     : 1;
	uint64_t s_tail                       : 1;
#endif
	} s;
	struct cvmx_pko_mem_iqueue_ptrs_s     cn68xx;
	struct cvmx_pko_mem_iqueue_ptrs_s     cn68xxp1;
};
typedef union cvmx_pko_mem_iqueue_ptrs cvmx_pko_mem_iqueue_ptrs_t;

/**
 * cvmx_pko_mem_iqueue_qos
 *
 * Notes:
 * Sets the QOS mask, per queue.  These QOS_MASK bits are logically and physically the same QOS_MASK
 * bits in PKO_MEM_IQUEUE_PTRS.  This CSR address allows the QOS_MASK bits to be written during PKO
 * operation without affecting any other queue state.  The port to which queue QID is mapped is port
 * IPID.  Note that the queue to port mapping must be the same as was previously programmed via the
 * PKO_MEM_IQUEUE_PTRS CSR.
 * This CSR is a memory of 256 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  The index to this CSR is an IQUEUE.  A read of any
 * entry that has not been previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_iqueue_qos {
	uint64_t u64;
	struct cvmx_pko_mem_iqueue_qos_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_61_63               : 3;
	uint64_t qos_mask                     : 8;  /**< Mask to control priority across 8 QOS rounds */
	uint64_t reserved_15_52               : 38;
	uint64_t ipid                         : 7;  /**< PKO-Internal Port ID to which this queue is mapped */
	uint64_t qid                          : 8;  /**< Queue ID */
#else
	uint64_t qid                          : 8;
	uint64_t ipid                         : 7;
	uint64_t reserved_15_52               : 38;
	uint64_t qos_mask                     : 8;
	uint64_t reserved_61_63               : 3;
#endif
	} s;
	struct cvmx_pko_mem_iqueue_qos_s      cn68xx;
	struct cvmx_pko_mem_iqueue_qos_s      cn68xxp1;
};
typedef union cvmx_pko_mem_iqueue_qos cvmx_pko_mem_iqueue_qos_t;

/**
 * cvmx_pko_mem_port_ptrs
 *
 * Notes:
 * Sets the port to engine mapping, per port.  Ports marked as static priority need not be contiguous,
 * but they must be the lowest numbered PIDs mapped to this EID and must have QOS_MASK=0xff.  If EID==8
 * or EID==9, then PID[1:0] is used to direct the packet to the correct port on that interface.
 * EID==15 can be used for unused PKO-internal ports.
 * The reset configuration is the following:
 *   PID EID(ext port) BP_PORT QOS_MASK STATIC_P
 *   -------------------------------------------
 *     0   0( 0)             0     0xff        0
 *     1   1( 1)             1     0xff        0
 *     2   2( 2)             2     0xff        0
 *     3   3( 3)             3     0xff        0
 *     4   0( 0)             4     0xff        0
 *     5   1( 1)             5     0xff        0
 *     6   2( 2)             6     0xff        0
 *     7   3( 3)             7     0xff        0
 *     8   0( 0)             8     0xff        0
 *     9   1( 1)             9     0xff        0
 *    10   2( 2)            10     0xff        0
 *    11   3( 3)            11     0xff        0
 *    12   0( 0)            12     0xff        0
 *    13   1( 1)            13     0xff        0
 *    14   2( 2)            14     0xff        0
 *    15   3( 3)            15     0xff        0
 *   -------------------------------------------
 *    16   4(16)            16     0xff        0
 *    17   5(17)            17     0xff        0
 *    18   6(18)            18     0xff        0
 *    19   7(19)            19     0xff        0
 *    20   4(16)            20     0xff        0
 *    21   5(17)            21     0xff        0
 *    22   6(18)            22     0xff        0
 *    23   7(19)            23     0xff        0
 *    24   4(16)            24     0xff        0
 *    25   5(17)            25     0xff        0
 *    26   6(18)            26     0xff        0
 *    27   7(19)            27     0xff        0
 *    28   4(16)            28     0xff        0
 *    29   5(17)            29     0xff        0
 *    30   6(18)            30     0xff        0
 *    31   7(19)            31     0xff        0
 *   -------------------------------------------
 *    32   8(32)            32     0xff        0
 *    33   8(33)            33     0xff        0
 *    34   8(34)            34     0xff        0
 *    35   8(35)            35     0xff        0
 *   -------------------------------------------
 *    36   9(36)            36     0xff        0
 *    37   9(37)            37     0xff        0
 *    38   9(38)            38     0xff        0
 *    39   9(39)            39     0xff        0
 *
 * This CSR is a memory of 48 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  A read of any entry that has not been
 * previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_port_ptrs {
	uint64_t u64;
	struct cvmx_pko_mem_port_ptrs_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t static_p                     : 1;  /**< Set if this PID has static priority */
	uint64_t qos_mask                     : 8;  /**< Mask to control priority across 8 QOS rounds */
	uint64_t reserved_16_52               : 37;
	uint64_t bp_port                      : 6;  /**< RESERVED, must be 63 */
	uint64_t eid                          : 4;  /**< Engine ID to which this port is mapped
                                                         Legal EIDs: 0-1, 8-9, 15 (15 only if port not used) */
	uint64_t pid                          : 6;  /**< Port ID[5:0] */
#else
	uint64_t pid                          : 6;
	uint64_t eid                          : 4;
	uint64_t bp_port                      : 6;
	uint64_t reserved_16_52               : 37;
	uint64_t qos_mask                     : 8;
	uint64_t static_p                     : 1;
	uint64_t reserved_62_63               : 2;
#endif
	} s;
	struct cvmx_pko_mem_port_ptrs_s       cn52xx;
	struct cvmx_pko_mem_port_ptrs_s       cn52xxp1;
	struct cvmx_pko_mem_port_ptrs_s       cn56xx;
	struct cvmx_pko_mem_port_ptrs_s       cn56xxp1;
	struct cvmx_pko_mem_port_ptrs_s       cn61xx;
	struct cvmx_pko_mem_port_ptrs_s       cn63xx;
	struct cvmx_pko_mem_port_ptrs_s       cn63xxp1;
	struct cvmx_pko_mem_port_ptrs_s       cn66xx;
	struct cvmx_pko_mem_port_ptrs_s       cn70xx;
	struct cvmx_pko_mem_port_ptrs_s       cnf71xx;
};
typedef union cvmx_pko_mem_port_ptrs cvmx_pko_mem_port_ptrs_t;

/**
 * cvmx_pko_mem_port_qos
 *
 * Notes:
 * Sets the QOS mask, per port.  These QOS_MASK bits are logically and physically the same QOS_MASK
 * bits in PKO_MEM_PORT_PTRS.  This CSR address allows the QOS_MASK bits to be written during PKO
 * operation without affecting any other port state.  The engine to which port PID is mapped is engine
 * EID.  Note that the port to engine mapping must be the same as was previously programmed via the
 * PKO_MEM_PORT_PTRS CSR.
 * This CSR is a memory of 44 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  A read of any entry that has not been
 * previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_port_qos {
	uint64_t u64;
	struct cvmx_pko_mem_port_qos_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_61_63               : 3;
	uint64_t qos_mask                     : 8;  /**< Mask to control priority across 8 QOS rounds */
	uint64_t reserved_10_52               : 43;
	uint64_t eid                          : 4;  /**< Engine ID to which this port is mapped
                                                         Legal EIDs: 0-1, 8-9 */
	uint64_t pid                          : 6;  /**< Port ID[5:0] */
#else
	uint64_t pid                          : 6;
	uint64_t eid                          : 4;
	uint64_t reserved_10_52               : 43;
	uint64_t qos_mask                     : 8;
	uint64_t reserved_61_63               : 3;
#endif
	} s;
	struct cvmx_pko_mem_port_qos_s        cn52xx;
	struct cvmx_pko_mem_port_qos_s        cn52xxp1;
	struct cvmx_pko_mem_port_qos_s        cn56xx;
	struct cvmx_pko_mem_port_qos_s        cn56xxp1;
	struct cvmx_pko_mem_port_qos_s        cn61xx;
	struct cvmx_pko_mem_port_qos_s        cn63xx;
	struct cvmx_pko_mem_port_qos_s        cn63xxp1;
	struct cvmx_pko_mem_port_qos_s        cn66xx;
	struct cvmx_pko_mem_port_qos_s        cn70xx;
	struct cvmx_pko_mem_port_qos_s        cnf71xx;
};
typedef union cvmx_pko_mem_port_qos cvmx_pko_mem_port_qos_t;

/**
 * cvmx_pko_mem_port_rate0
 *
 * Notes:
 * This CSR is a memory of 44 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  A read of any entry that has not been
 * previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_port_rate0 {
	uint64_t u64;
	struct cvmx_pko_mem_port_rate0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_51_63               : 13;
	uint64_t rate_word                    : 19; /**< Rate limiting adder per 8 byte */
	uint64_t rate_pkt                     : 24; /**< Rate limiting adder per packet */
	uint64_t reserved_7_7                 : 1;
	uint64_t pid                          : 7;  /**< Port ID[5:0] */
#else
	uint64_t pid                          : 7;
	uint64_t reserved_7_7                 : 1;
	uint64_t rate_pkt                     : 24;
	uint64_t rate_word                    : 19;
	uint64_t reserved_51_63               : 13;
#endif
	} s;
	struct cvmx_pko_mem_port_rate0_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_51_63               : 13;
	uint64_t rate_word                    : 19; /**< Rate limiting adder per 8 byte */
	uint64_t rate_pkt                     : 24; /**< Rate limiting adder per packet */
	uint64_t reserved_6_7                 : 2;
	uint64_t pid                          : 6;  /**< Port ID[5:0] */
#else
	uint64_t pid                          : 6;
	uint64_t reserved_6_7                 : 2;
	uint64_t rate_pkt                     : 24;
	uint64_t rate_word                    : 19;
	uint64_t reserved_51_63               : 13;
#endif
	} cn52xx;
	struct cvmx_pko_mem_port_rate0_cn52xx cn52xxp1;
	struct cvmx_pko_mem_port_rate0_cn52xx cn56xx;
	struct cvmx_pko_mem_port_rate0_cn52xx cn56xxp1;
	struct cvmx_pko_mem_port_rate0_cn52xx cn61xx;
	struct cvmx_pko_mem_port_rate0_cn52xx cn63xx;
	struct cvmx_pko_mem_port_rate0_cn52xx cn63xxp1;
	struct cvmx_pko_mem_port_rate0_cn52xx cn66xx;
	struct cvmx_pko_mem_port_rate0_s      cn68xx;
	struct cvmx_pko_mem_port_rate0_s      cn68xxp1;
	struct cvmx_pko_mem_port_rate0_cn52xx cn70xx;
	struct cvmx_pko_mem_port_rate0_cn52xx cnf71xx;
};
typedef union cvmx_pko_mem_port_rate0 cvmx_pko_mem_port_rate0_t;

/**
 * cvmx_pko_mem_port_rate1
 *
 * Notes:
 * Writing PKO_MEM_PORT_RATE1[PID,RATE_LIM] has the side effect of setting the corresponding
 * accumulator to zero.
 * This CSR is a memory of 44 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  A read of any entry that has not been
 * previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_port_rate1 {
	uint64_t u64;
	struct cvmx_pko_mem_port_rate1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t rate_lim                     : 24; /**< Rate limiting accumulator limit */
	uint64_t reserved_7_7                 : 1;
	uint64_t pid                          : 7;  /**< Port ID[5:0] */
#else
	uint64_t pid                          : 7;
	uint64_t reserved_7_7                 : 1;
	uint64_t rate_lim                     : 24;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_pko_mem_port_rate1_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t rate_lim                     : 24; /**< Rate limiting accumulator limit */
	uint64_t reserved_6_7                 : 2;
	uint64_t pid                          : 6;  /**< Port ID[5:0] */
#else
	uint64_t pid                          : 6;
	uint64_t reserved_6_7                 : 2;
	uint64_t rate_lim                     : 24;
	uint64_t reserved_32_63               : 32;
#endif
	} cn52xx;
	struct cvmx_pko_mem_port_rate1_cn52xx cn52xxp1;
	struct cvmx_pko_mem_port_rate1_cn52xx cn56xx;
	struct cvmx_pko_mem_port_rate1_cn52xx cn56xxp1;
	struct cvmx_pko_mem_port_rate1_cn52xx cn61xx;
	struct cvmx_pko_mem_port_rate1_cn52xx cn63xx;
	struct cvmx_pko_mem_port_rate1_cn52xx cn63xxp1;
	struct cvmx_pko_mem_port_rate1_cn52xx cn66xx;
	struct cvmx_pko_mem_port_rate1_s      cn68xx;
	struct cvmx_pko_mem_port_rate1_s      cn68xxp1;
	struct cvmx_pko_mem_port_rate1_cn52xx cn70xx;
	struct cvmx_pko_mem_port_rate1_cn52xx cnf71xx;
};
typedef union cvmx_pko_mem_port_rate1 cvmx_pko_mem_port_rate1_t;

/**
 * cvmx_pko_mem_queue_ptrs
 *
 * Notes:
 * Sets the queue to port mapping and the initial command buffer pointer, per queue
 * Each queue may map to at most one port.  No more than 16 queues may map to a port.  The set of
 * queues that is mapped to a port must be a contiguous array of queues.  The port to which queue QID
 * is mapped is port PID.  The index of queue QID in port PID's queue list is IDX.  The last queue in
 * port PID's queue array must have its TAIL bit set.  Unused queues must be mapped to port 63.
 * STATIC_Q marks queue QID as having static priority.  STATIC_P marks the port PID to which QID is
 * mapped as having at least one queue with static priority.  If any QID that maps to PID has static
 * priority, then all QID that map to PID must have STATIC_P set.  Queues marked as static priority
 * must be contiguous and begin at IDX 0.  The last queue that is marked as having static priority
 * must have its S_TAIL bit set.
 * This CSR is a memory of 256 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  A read of any entry that has not been
 * previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_queue_ptrs {
	uint64_t u64;
	struct cvmx_pko_mem_queue_ptrs_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t s_tail                       : 1;  /**< Set if this QID is the tail of the static queues */
	uint64_t static_p                     : 1;  /**< Set if any QID in this PID has static priority */
	uint64_t static_q                     : 1;  /**< Set if this QID has static priority */
	uint64_t qos_mask                     : 8;  /**< Mask to control priority across 8 QOS rounds */
	uint64_t buf_ptr                      : 36; /**< Command buffer pointer, <23:17> MBZ */
	uint64_t tail                         : 1;  /**< Set if this QID is the tail of the queue array */
	uint64_t index                        : 3;  /**< Index[2:0] (distance from head) in the queue array */
	uint64_t port                         : 6;  /**< Port ID to which this queue is mapped */
	uint64_t queue                        : 7;  /**< Queue ID[6:0] */
#else
	uint64_t queue                        : 7;
	uint64_t port                         : 6;
	uint64_t index                        : 3;
	uint64_t tail                         : 1;
	uint64_t buf_ptr                      : 36;
	uint64_t qos_mask                     : 8;
	uint64_t static_q                     : 1;
	uint64_t static_p                     : 1;
	uint64_t s_tail                       : 1;
#endif
	} s;
	struct cvmx_pko_mem_queue_ptrs_s      cn30xx;
	struct cvmx_pko_mem_queue_ptrs_s      cn31xx;
	struct cvmx_pko_mem_queue_ptrs_s      cn38xx;
	struct cvmx_pko_mem_queue_ptrs_s      cn38xxp2;
	struct cvmx_pko_mem_queue_ptrs_s      cn50xx;
	struct cvmx_pko_mem_queue_ptrs_s      cn52xx;
	struct cvmx_pko_mem_queue_ptrs_s      cn52xxp1;
	struct cvmx_pko_mem_queue_ptrs_s      cn56xx;
	struct cvmx_pko_mem_queue_ptrs_s      cn56xxp1;
	struct cvmx_pko_mem_queue_ptrs_s      cn58xx;
	struct cvmx_pko_mem_queue_ptrs_s      cn58xxp1;
	struct cvmx_pko_mem_queue_ptrs_s      cn61xx;
	struct cvmx_pko_mem_queue_ptrs_s      cn63xx;
	struct cvmx_pko_mem_queue_ptrs_s      cn63xxp1;
	struct cvmx_pko_mem_queue_ptrs_s      cn66xx;
	struct cvmx_pko_mem_queue_ptrs_s      cn70xx;
	struct cvmx_pko_mem_queue_ptrs_s      cnf71xx;
};
typedef union cvmx_pko_mem_queue_ptrs cvmx_pko_mem_queue_ptrs_t;

/**
 * cvmx_pko_mem_queue_qos
 *
 * Notes:
 * Sets the QOS mask, per queue.  These QOS_MASK bits are logically and physically the same QOS_MASK
 * bits in PKO_MEM_QUEUE_PTRS.  This CSR address allows the QOS_MASK bits to be written during PKO
 * operation without affecting any other queue state.  The port to which queue QID is mapped is port
 * PID.  Note that the queue to port mapping must be the same as was previously programmed via the
 * PKO_MEM_QUEUE_PTRS CSR.
 * This CSR is a memory of 256 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  A read of any entry that has not been
 * previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_queue_qos {
	uint64_t u64;
	struct cvmx_pko_mem_queue_qos_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_61_63               : 3;
	uint64_t qos_mask                     : 8;  /**< Mask to control priority across 8 QOS rounds */
	uint64_t reserved_13_52               : 40;
	uint64_t pid                          : 6;  /**< Port ID to which this queue is mapped */
	uint64_t qid                          : 7;  /**< Queue ID */
#else
	uint64_t qid                          : 7;
	uint64_t pid                          : 6;
	uint64_t reserved_13_52               : 40;
	uint64_t qos_mask                     : 8;
	uint64_t reserved_61_63               : 3;
#endif
	} s;
	struct cvmx_pko_mem_queue_qos_s       cn30xx;
	struct cvmx_pko_mem_queue_qos_s       cn31xx;
	struct cvmx_pko_mem_queue_qos_s       cn38xx;
	struct cvmx_pko_mem_queue_qos_s       cn38xxp2;
	struct cvmx_pko_mem_queue_qos_s       cn50xx;
	struct cvmx_pko_mem_queue_qos_s       cn52xx;
	struct cvmx_pko_mem_queue_qos_s       cn52xxp1;
	struct cvmx_pko_mem_queue_qos_s       cn56xx;
	struct cvmx_pko_mem_queue_qos_s       cn56xxp1;
	struct cvmx_pko_mem_queue_qos_s       cn58xx;
	struct cvmx_pko_mem_queue_qos_s       cn58xxp1;
	struct cvmx_pko_mem_queue_qos_s       cn61xx;
	struct cvmx_pko_mem_queue_qos_s       cn63xx;
	struct cvmx_pko_mem_queue_qos_s       cn63xxp1;
	struct cvmx_pko_mem_queue_qos_s       cn66xx;
	struct cvmx_pko_mem_queue_qos_s       cn70xx;
	struct cvmx_pko_mem_queue_qos_s       cnf71xx;
};
typedef union cvmx_pko_mem_queue_qos cvmx_pko_mem_queue_qos_t;

/**
 * cvmx_pko_mem_throttle_int
 *
 * Notes:
 * Writing PACKET and WORD with 0 resets both counts for INT to 0 rather than add 0.
 * Otherwise, writes to this CSR add to the existing WORD/PACKET counts for the interface INT.
 *
 * PKO tracks the number of (8-byte) WORD's and PACKET's in-flight (sum total in both PKO
 * and the interface MAC) on the interface. (When PKO first selects a packet from a PKO queue, it
 * increments the counts appropriately. When the interface MAC has (largely) completed sending
 * the words/packet, PKO decrements the count appropriately.) When PKO_REG_FLAGS[ENA_THROTTLE]
 * is set and the most-significant bit of the WORD or packet count for a interface is set,
 * PKO will not transfer any packets over the interface. Software can limit the amount of
 * packet data and/or the number of packets that OCTEON can send out the chip after receiving backpressure
 * from the interface/pipe via these per-pipe throttle counts when PKO_REG_FLAGS[ENA_THROTTLE]=1.
 * For example, to limit the number of packets outstanding in the interface to N, preset PACKET for
 * the pipe to the value 0x20-N (0x20 is the smallest PACKET value with the most-significant bit set).
 *
 * This CSR is a memory of 32 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  The index to this CSR is an INTERFACE.  A read of any
 * entry that has not been previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_throttle_int {
	uint64_t u64;
	struct cvmx_pko_mem_throttle_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_47_63               : 17;
	uint64_t word                         : 15; /**< On a write, the amount to add to the interface
                                                         throttle word count selected by INT. On a read,
                                                         returns the current value of the interface throttle
                                                         word count selected by PKO_REG_READ_IDX[IDX]. */
	uint64_t reserved_14_31               : 18;
	uint64_t packet                       : 6;  /**< On a write, the amount to add to the interface
                                                         throttle packet count selected by INT. On a read,
                                                         returns the current value of the interface throttle
                                                         packet count selected by PKO_REG_READ_IDX[IDX]. */
	uint64_t reserved_5_7                 : 3;
	uint64_t intr                         : 5;  /**< Selected interface for writes. Undefined on a read.
                                                         See PKO_MEM_IPORT_PTRS[INT] for encoding. */
#else
	uint64_t intr                         : 5;
	uint64_t reserved_5_7                 : 3;
	uint64_t packet                       : 6;
	uint64_t reserved_14_31               : 18;
	uint64_t word                         : 15;
	uint64_t reserved_47_63               : 17;
#endif
	} s;
	struct cvmx_pko_mem_throttle_int_s    cn68xx;
	struct cvmx_pko_mem_throttle_int_s    cn68xxp1;
};
typedef union cvmx_pko_mem_throttle_int cvmx_pko_mem_throttle_int_t;

/**
 * cvmx_pko_mem_throttle_pipe
 *
 * Notes:
 * Writing PACKET and WORD with 0 resets both counts for PIPE to 0 rather than add 0.
 * Otherwise, writes to this CSR add to the existing WORD/PACKET counts for the PKO pipe PIPE.
 *
 * PKO tracks the number of (8-byte) WORD's and PACKET's in-flight (sum total in both PKO
 * and the interface MAC) on the pipe. (When PKO first selects a packet from a PKO queue, it
 * increments the counts appropriately. When the interface MAC has (largely) completed sending
 * the words/packet, PKO decrements the count appropriately.) When PKO_REG_FLAGS[ENA_THROTTLE]
 * is set and the most-significant bit of the WORD or packet count for a PKO pipe is set,
 * PKO will not transfer any packets over the PKO pipe. Software can limit the amount of
 * packet data and/or the number of packets that OCTEON can send out the chip after receiving backpressure
 * from the interface/pipe via these per-pipe throttle counts when PKO_REG_FLAGS[ENA_THROTTLE]=1.
 * For example, to limit the number of packets outstanding in the pipe to N, preset PACKET for
 * the pipe to the value 0x20-N (0x20 is the smallest PACKET value with the most-significant bit set).
 *
 * This CSR is a memory of 128 entries, and thus, the PKO_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.  The index to this CSR is a PIPE.  A read of any
 * entry that has not been previously written is illegal and will result in unpredictable CSR read data.
 */
union cvmx_pko_mem_throttle_pipe {
	uint64_t u64;
	struct cvmx_pko_mem_throttle_pipe_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_47_63               : 17;
	uint64_t word                         : 15; /**< On a write, the amount to add to the pipe throttle
                                                         word count selected by PIPE. On a read, returns
                                                         the current value of the pipe throttle word count
                                                         selected by PKO_REG_READ_IDX[IDX]. */
	uint64_t reserved_14_31               : 18;
	uint64_t packet                       : 6;  /**< On a write, the amount to add to the pipe throttle
                                                         packet count selected by PIPE. On a read, returns
                                                         the current value of the pipe throttle packet count
                                                         selected by PKO_REG_READ_IDX[IDX]. */
	uint64_t reserved_7_7                 : 1;
	uint64_t pipe                         : 7;  /**< Selected PKO pipe for writes. Undefined on a read. */
#else
	uint64_t pipe                         : 7;
	uint64_t reserved_7_7                 : 1;
	uint64_t packet                       : 6;
	uint64_t reserved_14_31               : 18;
	uint64_t word                         : 15;
	uint64_t reserved_47_63               : 17;
#endif
	} s;
	struct cvmx_pko_mem_throttle_pipe_s   cn68xx;
	struct cvmx_pko_mem_throttle_pipe_s   cn68xxp1;
};
typedef union cvmx_pko_mem_throttle_pipe cvmx_pko_mem_throttle_pipe_t;

/**
 * cvmx_pko_ncb_bist_status
 *
 * Each bit is the BIST result of an individual memory (per bit, 0 = pass and 1 = fail).
 *
 */
union cvmx_pko_ncb_bist_status {
	uint64_t u64;
	struct cvmx_pko_ncb_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ncbi_l2_out_ram_bist_status  : 1;  /**< BIST status for NCBI_L2_OUT_RAM. */
	uint64_t ncbi_pp_out_ram_bist_status  : 1;  /**< BIST status for NCBI_PP_OUT_RAM. */
	uint64_t ncbo_pdm_cmd_dat_ram_bist_status : 1;/**< BIST status for NCBO_PDM_CMD_DAT_RAM. */
	uint64_t ncbi_l2_pdm_pref_ram_bist_status : 1;/**< BIST status for NCBI_L2_PDM_PREF_RAM. */
	uint64_t ncbo_pp_fif_ram_bist_status  : 1;  /**< BIST status for NCBO_PP_FIF_RAM. */
	uint64_t reserved_0_58                : 59;
#else
	uint64_t reserved_0_58                : 59;
	uint64_t ncbo_pp_fif_ram_bist_status  : 1;
	uint64_t ncbi_l2_pdm_pref_ram_bist_status : 1;
	uint64_t ncbo_pdm_cmd_dat_ram_bist_status : 1;
	uint64_t ncbi_pp_out_ram_bist_status  : 1;
	uint64_t ncbi_l2_out_ram_bist_status  : 1;
#endif
	} s;
	struct cvmx_pko_ncb_bist_status_s     cn78xx;
};
typedef union cvmx_pko_ncb_bist_status cvmx_pko_ncb_bist_status_t;

/**
 * cvmx_pko_ncb_ecc_ctl0
 */
union cvmx_pko_ncb_ecc_ctl0 {
	uint64_t u64;
	struct cvmx_pko_ncb_ecc_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ncbi_l2_out_ram_flip         : 2;  /**< NCBI_L2_OUT_RAM flip syndrome bits on write. */
	uint64_t ncbi_l2_out_ram_cdis         : 1;  /**< NCBI_L2_OUT_RAM ECC correction disable. */
	uint64_t ncbi_pp_out_ram_flip         : 2;  /**< NCBI_PP_OUT_RAM flip syndrome bits on write. */
	uint64_t ncbi_pp_out_ram_cdis         : 1;  /**< NCBI_PP_OUT_RAM ECC correction disable. */
	uint64_t ncbo_pdm_cmd_dat_ram_flip    : 2;  /**< NCBO_PDM_CMD_DAT_RAM flip syndrome bits on write. */
	uint64_t ncbo_pdm_cmd_dat_ram_cdis    : 1;  /**< NCBO_PDM_CMD_DAT_RAM ECC correction disable. */
	uint64_t ncbi_l2_pdm_pref_ram_flip    : 2;  /**< NCBI_L2_PDM_PREF_RAM flip syndrome bits on write. */
	uint64_t ncbi_l2_pdm_pref_ram_cdis    : 1;  /**< NCBI_L2_PDM_PREF_RAM ECC correction disable. */
	uint64_t ncbo_pp_fif_ram_flip         : 2;  /**< NCBO_PP_FIF_RAM flip syndrome bits on write. */
	uint64_t ncbo_pp_fif_ram_cdis         : 1;  /**< NCBO_PP_FIF_RAM ECC correction disable. */
	uint64_t reserved_0_48                : 49;
#else
	uint64_t reserved_0_48                : 49;
	uint64_t ncbo_pp_fif_ram_cdis         : 1;
	uint64_t ncbo_pp_fif_ram_flip         : 2;
	uint64_t ncbi_l2_pdm_pref_ram_cdis    : 1;
	uint64_t ncbi_l2_pdm_pref_ram_flip    : 2;
	uint64_t ncbo_pdm_cmd_dat_ram_cdis    : 1;
	uint64_t ncbo_pdm_cmd_dat_ram_flip    : 2;
	uint64_t ncbi_pp_out_ram_cdis         : 1;
	uint64_t ncbi_pp_out_ram_flip         : 2;
	uint64_t ncbi_l2_out_ram_cdis         : 1;
	uint64_t ncbi_l2_out_ram_flip         : 2;
#endif
	} s;
	struct cvmx_pko_ncb_ecc_ctl0_s        cn78xx;
};
typedef union cvmx_pko_ncb_ecc_ctl0 cvmx_pko_ncb_ecc_ctl0_t;

/**
 * cvmx_pko_ncb_ecc_dbe_sts0
 */
union cvmx_pko_ncb_ecc_dbe_sts0 {
	uint64_t u64;
	struct cvmx_pko_ncb_ecc_dbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ncbi_l2_out_ram_dbe          : 1;  /**< Double-bit error for NCBI_L2_OUT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.nonpse.ncb.pko_ncbi_outb.ncbi_txr.l2_out_fifo */
	uint64_t ncbi_pp_out_ram_dbe          : 1;  /**< Double-bit error for NCBI_PP_OUT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.nonpse.ncb.pko_ncbi_outb.ncbi_txr.pp_out_fifo */
	uint64_t ncbo_pdm_cmd_dat_ram_dbe     : 1;  /**< Double-bit error for NCBO_PDM_CMD_DAT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.nonpse.ncb.pko_ncbo_inb.splitter.ncb__pdm_cmnd_data_fifo */
	uint64_t ncbi_l2_pdm_pref_ram_dbe     : 1;  /**< Double-bit error for NCBI_L2_PDM_PREF_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.nonpse.ncb.pko_ncbi_outb.ncbi_l2_pipe.pdm_prefbuf_fifo */
	uint64_t ncbo_pp_fif_ram_dbe          : 1;  /**< Double-bit error for NCBO_PP_FIF_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.nonpse.ncb.pko_ncbo_inb.splitter.pp_fifo */
	uint64_t reserved_0_58                : 59;
#else
	uint64_t reserved_0_58                : 59;
	uint64_t ncbo_pp_fif_ram_dbe          : 1;
	uint64_t ncbi_l2_pdm_pref_ram_dbe     : 1;
	uint64_t ncbo_pdm_cmd_dat_ram_dbe     : 1;
	uint64_t ncbi_pp_out_ram_dbe          : 1;
	uint64_t ncbi_l2_out_ram_dbe          : 1;
#endif
	} s;
	struct cvmx_pko_ncb_ecc_dbe_sts0_s    cn78xx;
};
typedef union cvmx_pko_ncb_ecc_dbe_sts0 cvmx_pko_ncb_ecc_dbe_sts0_t;

/**
 * cvmx_pko_ncb_ecc_dbe_sts_cmb0
 */
union cvmx_pko_ncb_ecc_dbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pko_ncb_ecc_dbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ncb_dbe_cmb0                 : 1;  /**< This bit is the logical OR of all bits in PKO_NCB_ECC_DBE_STS.
                                                         To clear this bit, software must clear bits in PKO_NCB_ECC_DBE_STS.
                                                         When this bit is set, the corresponding interrupt is set.
                                                         Throws PKO_INTSN_E::PKO_NCB_DBE_CMB0.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.nonpse.ncb.pko_ncbi_outb.ncbi_txr.l2_out_fifo
                                                         pko_pnr2.nonpse.ncb.pko_ncbi_outb.ncbi_txr.pp_out_fifo
                                                         pko_pnr2.nonpse.ncb.pko_ncbo_inb.splitter.ncb__pdm_cmnd_data_fifo
                                                         pko_pnr2.nonpse.ncb.pko_ncbi_outb.ncbi_l2_pipe.pdm_prefbuf_fifo
                                                         pko_pnr2.nonpse.ncb.pko_ncbo_inb.splitter.pp_fifo */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t ncb_dbe_cmb0                 : 1;
#endif
	} s;
	struct cvmx_pko_ncb_ecc_dbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pko_ncb_ecc_dbe_sts_cmb0 cvmx_pko_ncb_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pko_ncb_ecc_sbe_sts0
 */
union cvmx_pko_ncb_ecc_sbe_sts0 {
	uint64_t u64;
	struct cvmx_pko_ncb_ecc_sbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ncbi_l2_out_ram_sbe          : 1;  /**< Single-bit error for NCBI_L2_OUT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.nonpse.ncb.pko_ncbi_outb.ncbi_txr.l2_out_fifo */
	uint64_t ncbi_pp_out_ram_sbe          : 1;  /**< Single-bit error for NCBI_PP_OUT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.nonpse.ncb.pko_ncbi_outb.ncbi_txr.pp_out_fifo */
	uint64_t ncbo_pdm_cmd_dat_ram_sbe     : 1;  /**< Single-bit error for NCBO_PDM_CMD_DAT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.nonpse.ncb.pko_ncbo_inb.splitter.ncb__pdm_cmnd_data_fifo */
	uint64_t ncbi_l2_pdm_pref_ram_sbe     : 1;  /**< Single-bit error for NCBI_L2_PDM_PREF_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.nonpse.ncb.pko_ncbi_outb.ncbi_l2_pipe.pdm_prefbuf_fifo */
	uint64_t ncbo_pp_fif_ram_sbe          : 1;  /**< Single-bit error for NCBO_PP_FIF_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.nonpse.ncb.pko_ncbo_inb.splitter.pp_fifo */
	uint64_t reserved_0_58                : 59;
#else
	uint64_t reserved_0_58                : 59;
	uint64_t ncbo_pp_fif_ram_sbe          : 1;
	uint64_t ncbi_l2_pdm_pref_ram_sbe     : 1;
	uint64_t ncbo_pdm_cmd_dat_ram_sbe     : 1;
	uint64_t ncbi_pp_out_ram_sbe          : 1;
	uint64_t ncbi_l2_out_ram_sbe          : 1;
#endif
	} s;
	struct cvmx_pko_ncb_ecc_sbe_sts0_s    cn78xx;
};
typedef union cvmx_pko_ncb_ecc_sbe_sts0 cvmx_pko_ncb_ecc_sbe_sts0_t;

/**
 * cvmx_pko_ncb_ecc_sbe_sts_cmb0
 */
union cvmx_pko_ncb_ecc_sbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pko_ncb_ecc_sbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ncb_sbe_cmb0                 : 1;  /**< This bit is the logical OR of all bits in PKO_NCB_ECC_SBE_STS.
                                                         To clear this bit, software must clear bits in PKO_NCB_ECC_SBE_STS.
                                                         When this bit is set, the corresponding interrupt is set.
                                                         Throws PKO_INTSN_E::PKO_NCB_SBE_CMB0.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.nonpse.ncb.pko_ncbi_outb.ncbi_txr.l2_out_fifo
                                                         pko_pnr2.nonpse.ncb.pko_ncbi_outb.ncbi_txr.pp_out_fifo
                                                         pko_pnr2.nonpse.ncb.pko_ncbo_inb.splitter.ncb__pdm_cmnd_data_fifo
                                                         pko_pnr2.nonpse.ncb.pko_ncbi_outb.ncbi_l2_pipe.pdm_prefbuf_fifo
                                                         pko_pnr2.nonpse.ncb.pko_ncbo_inb.splitter.pp_fifo */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t ncb_sbe_cmb0                 : 1;
#endif
	} s;
	struct cvmx_pko_ncb_ecc_sbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pko_ncb_ecc_sbe_sts_cmb0 cvmx_pko_ncb_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pko_ncb_int
 */
union cvmx_pko_ncb_int {
	uint64_t u64;
	struct cvmx_pko_ncb_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t ncb_tx_error                 : 1;  /**< NCB transaction error occurred (error/unpredictable/undefined). Throws
                                                         PKO_INTSN_E::PKO_NCB_TX_ERR. */
#else
	uint64_t ncb_tx_error                 : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_pko_ncb_int_s             cn78xx;
};
typedef union cvmx_pko_ncb_int cvmx_pko_ncb_int_t;

/**
 * cvmx_pko_ncb_tx_err_info
 */
union cvmx_pko_ncb_tx_err_info {
	uint64_t u64;
	struct cvmx_pko_ncb_tx_err_info_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t wcnt                         : 5;  /**< Word count. */
	uint64_t src                          : 12; /**< Source of the IOI outbound transaction. */
	uint64_t dst                          : 8;  /**< Destination of the IOI outbound transaction. */
	uint64_t tag                          : 4;  /**< Tag of the IOI outbound transaction. */
	uint64_t eot                          : 1;  /**< EOT bit of the NCBO transaction. */
	uint64_t sot                          : 1;  /**< SOT bit of the NCBO transaction. */
	uint64_t valid                        : 1;  /**< Valid bit for transaction (should always be 1 on capture). */
#else
	uint64_t valid                        : 1;
	uint64_t sot                          : 1;
	uint64_t eot                          : 1;
	uint64_t tag                          : 4;
	uint64_t dst                          : 8;
	uint64_t src                          : 12;
	uint64_t wcnt                         : 5;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_pko_ncb_tx_err_info_s     cn78xx;
};
typedef union cvmx_pko_ncb_tx_err_info cvmx_pko_ncb_tx_err_info_t;

/**
 * cvmx_pko_ncb_tx_err_word
 */
union cvmx_pko_ncb_tx_err_word {
	uint64_t u64;
	struct cvmx_pko_ncb_tx_err_word_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t err_word                     : 64; /**< PKO NCB error word (first word of erroneous transaction).
                                                         Note: This is only the 64-bit data word; the NCB info that goes with it is in
                                                         PKO_NCB_TX_ERR_INFO. */
#else
	uint64_t err_word                     : 64;
#endif
	} s;
	struct cvmx_pko_ncb_tx_err_word_s     cn78xx;
};
typedef union cvmx_pko_ncb_tx_err_word cvmx_pko_ncb_tx_err_word_t;

/**
 * cvmx_pko_pdm_bist_status
 *
 * Each bit is the BIST result of an individual memory (per bit, 0 = pass and 1 = fail).
 *
 */
union cvmx_pko_pdm_bist_status {
	uint64_t u64;
	struct cvmx_pko_pdm_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t flshb_cache_lo_ram_bist_status : 1;/**< BIST status for FLSHB_CACHE_LO_RAM. */
	uint64_t flshb_cache_hi_ram_bist_status : 1;/**< BIST status for FLSHB_CACHE_HI_RAM. */
	uint64_t isrm_ca_iinst_ram_bist_status : 1; /**< BIST status for ISRM_CA_IINST_RAM. */
	uint64_t isrm_ca_cm_ram_bist_status   : 1;  /**< BIST status for ISRM_CA_CM_RAM. */
	uint64_t isrm_st_ram2_bist_status     : 1;  /**< BIST status for ISRM_ST_RAM2. */
	uint64_t isrm_st_ram1_bist_status     : 1;  /**< BIST status for ISRM_ST_RAM1. */
	uint64_t isrm_st_ram0_bist_status     : 1;  /**< BIST status for ISRM_ST_RAM0. */
	uint64_t isrd_st_ram3_bist_status     : 1;  /**< BIST status for ISRD_ST_RAM3. */
	uint64_t isrd_st_ram2_bist_status     : 1;  /**< BIST status for ISRD_ST_RAM2. */
	uint64_t isrd_st_ram1_bist_status     : 1;  /**< BIST status for ISRD_ST_RAM1. */
	uint64_t isrd_st_ram0_bist_status     : 1;  /**< BIST status for ISRD_ST_RAM0. */
	uint64_t drp_hi_ram_bist_status       : 1;  /**< BIST status for DRP_HI_RAM. */
	uint64_t drp_lo_ram_bist_status       : 1;  /**< BIST status for DRP_LO_RAM. */
	uint64_t dwp_hi_ram_bist_status       : 1;  /**< BIST status for DWP_HI_RAM. */
	uint64_t dwp_lo_ram_bist_status       : 1;  /**< BIST status for DWP_LO_RAM. */
	uint64_t mwp_hi_ram_bist_status       : 1;  /**< BIST status for MWP_HI_RAM. */
	uint64_t mwp_lo_ram_bist_status       : 1;  /**< BIST status for MWP_LO_RAM. */
	uint64_t fillb_m_rsp_ram_hi_bist_status : 1;/**< BIST status for FILLB_M_RSP_RAM_HI. */
	uint64_t fillb_m_rsp_ram_lo_bist_status : 1;/**< BIST status for FILLB_M_RSP_RAM_LO. */
	uint64_t fillb_d_rsp_ram_hi_bist_status : 1;/**< BIST status for FILLB_D_RSP_RAM_HI. */
	uint64_t fillb_d_rsp_ram_lo_bist_status : 1;/**< BIST status for FILLB_D_RSP_RAM_LO. */
	uint64_t fillb_d_rsp_dat_fifo_bist_status : 1;/**< BIST status for FILLB_FLSHB_M_DAT_RAM. */
	uint64_t fillb_m_rsp_dat_fifo_bist_status : 1;/**< BIST status for FILLB_M_DAT_FIFO. */
	uint64_t flshb_m_dat_ram_bist_status  : 1;  /**< BIST status for FLSHB_M_DAT_RAM. */
	uint64_t flshb_d_dat_ram_bist_status  : 1;  /**< BIST status for FLSHB_M_DAT_RAM. */
	uint64_t minpad_ram_bist_status       : 1;  /**< BIST status for MINPAD_RAM. */
	uint64_t reserved_0_37                : 38;
#else
	uint64_t reserved_0_37                : 38;
	uint64_t minpad_ram_bist_status       : 1;
	uint64_t flshb_d_dat_ram_bist_status  : 1;
	uint64_t flshb_m_dat_ram_bist_status  : 1;
	uint64_t fillb_m_rsp_dat_fifo_bist_status : 1;
	uint64_t fillb_d_rsp_dat_fifo_bist_status : 1;
	uint64_t fillb_d_rsp_ram_lo_bist_status : 1;
	uint64_t fillb_d_rsp_ram_hi_bist_status : 1;
	uint64_t fillb_m_rsp_ram_lo_bist_status : 1;
	uint64_t fillb_m_rsp_ram_hi_bist_status : 1;
	uint64_t mwp_lo_ram_bist_status       : 1;
	uint64_t mwp_hi_ram_bist_status       : 1;
	uint64_t dwp_lo_ram_bist_status       : 1;
	uint64_t dwp_hi_ram_bist_status       : 1;
	uint64_t drp_lo_ram_bist_status       : 1;
	uint64_t drp_hi_ram_bist_status       : 1;
	uint64_t isrd_st_ram0_bist_status     : 1;
	uint64_t isrd_st_ram1_bist_status     : 1;
	uint64_t isrd_st_ram2_bist_status     : 1;
	uint64_t isrd_st_ram3_bist_status     : 1;
	uint64_t isrm_st_ram0_bist_status     : 1;
	uint64_t isrm_st_ram1_bist_status     : 1;
	uint64_t isrm_st_ram2_bist_status     : 1;
	uint64_t isrm_ca_cm_ram_bist_status   : 1;
	uint64_t isrm_ca_iinst_ram_bist_status : 1;
	uint64_t flshb_cache_hi_ram_bist_status : 1;
	uint64_t flshb_cache_lo_ram_bist_status : 1;
#endif
	} s;
	struct cvmx_pko_pdm_bist_status_s     cn78xx;
};
typedef union cvmx_pko_pdm_bist_status cvmx_pko_pdm_bist_status_t;

/**
 * cvmx_pko_pdm_cfg
 */
union cvmx_pko_pdm_cfg {
	uint64_t u64;
	struct cvmx_pko_pdm_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_13_63               : 51;
	uint64_t dis_lpd_w2r_fill             : 1;  /**< Set to disable the write to read fill caused by LPD in the PDM. If disabled, must wait for
                                                         FPD bit from PEB, which is a performance penalty when the time is large for the PEB
                                                         request to make it back to PDM. For diagnostic use only. */
	uint64_t en_fr_w2r_ptr_swp            : 1;  /**< Set to enable pointer swap on a fill response when we go in-sync (only one cacheline in
                                                         DQ).
                                                         For diagnostic use only. */
	uint64_t dis_flsh_cache               : 1;  /**< Set to disable the flush buffer's cache. This makes all fills require full memory latency.
                                                         For diagnostic use only. */
	uint64_t pko_pad_minlen               : 7;  /**< Minimum frame padding min length. */
	uint64_t diag_mode                    : 1;  /**< Set to enable read/write to memories in PDM through CSR interface.  For diagnostic use only. */
	uint64_t alloc_lds                    : 1;  /**< Allocate LDS. This signal prevents the loads to IOBP from being allocated in on-chip cache
                                                         (LDWB vs. LDD). Two modes as follows: 0 = No allocate (LDWB); 1 = Allocate (LDD). */
	uint64_t alloc_sts                    : 1;  /**< Allocate STS. This signal prevents the stores to NCB from being allocated in on-chip cache
                                                         (STF vs. STT). Two modes as follows: 0 = No allocate (STT); 1 = Allocate (STF). */
#else
	uint64_t alloc_sts                    : 1;
	uint64_t alloc_lds                    : 1;
	uint64_t diag_mode                    : 1;
	uint64_t pko_pad_minlen               : 7;
	uint64_t dis_flsh_cache               : 1;
	uint64_t en_fr_w2r_ptr_swp            : 1;
	uint64_t dis_lpd_w2r_fill             : 1;
	uint64_t reserved_13_63               : 51;
#endif
	} s;
	struct cvmx_pko_pdm_cfg_s             cn78xx;
};
typedef union cvmx_pko_pdm_cfg cvmx_pko_pdm_cfg_t;

/**
 * cvmx_pko_pdm_cfg_dbg
 */
union cvmx_pko_pdm_cfg_dbg {
	uint64_t u64;
	struct cvmx_pko_pdm_cfg_dbg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t cp_stall_thrshld             : 32; /**< Program this register to the 32-bit number of cycles to test for the PDM(CP) stalled on
                                                         inputs going into the ISRs. PKO_PDM_STS[CP_STALL_THRSHLD_HIT] indicates the threshold has
                                                         been hit. INTERNAL: Do not list field in HRM. For lab debug only; will likely disappear in
                                                         pass 2. */
#else
	uint64_t cp_stall_thrshld             : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_pko_pdm_cfg_dbg_s         cn78xx;
};
typedef union cvmx_pko_pdm_cfg_dbg cvmx_pko_pdm_cfg_dbg_t;

/**
 * cvmx_pko_pdm_cp_dbg
 */
union cvmx_pko_pdm_cp_dbg {
	uint64_t u64;
	struct cvmx_pko_pdm_cp_dbg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t stateless_fif_cnt            : 6;  /**< Stateless fifo count. */
	uint64_t reserved_5_9                 : 5;
	uint64_t op_fif_not_full              : 5;  /**< Output fifo not full signals. The order of the bits is:
                                                         - 4: ISR CMD FIFO not full
                                                         - 3: DESC DAT FIFO HIGH not full
                                                         - 2: DESC DAT FIFO LOW not full
                                                         - 1: MP DAT FIFO not full
                                                         - 0: PSE CMD RESP FIFO has credit */
#else
	uint64_t op_fif_not_full              : 5;
	uint64_t reserved_5_9                 : 5;
	uint64_t stateless_fif_cnt            : 6;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_pko_pdm_cp_dbg_s          cn78xx;
};
typedef union cvmx_pko_pdm_cp_dbg cvmx_pko_pdm_cp_dbg_t;

/**
 * cvmx_pko_pdm_dq#_minpad
 */
union cvmx_pko_pdm_dqx_minpad {
	uint64_t u64;
	struct cvmx_pko_pdm_dqx_minpad_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t minpad                       : 1;  /**< MINPAD setting per DQ. Each DQ has a separate CSR address; and bit 0 of the data
                                                         read/write value is the MINPAD bit. When MINPAD is set, the send-packet header has the
                                                         total field adjusted by MINLEN (PKO_PDM_CFG.PKO_PAD_MINLEN) as follows:
                                                         if (MINPAD)
                                                         if (send_hdr.total < MINLEN) send_hdr.total = MINLEN */
#else
	uint64_t minpad                       : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_pko_pdm_dqx_minpad_s      cn78xx;
};
typedef union cvmx_pko_pdm_dqx_minpad cvmx_pko_pdm_dqx_minpad_t;

/**
 * cvmx_pko_pdm_drpbuf_dbg
 */
union cvmx_pko_pdm_drpbuf_dbg {
	uint64_t u64;
	struct cvmx_pko_pdm_drpbuf_dbg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_43_63               : 21;
	uint64_t sel_nxt_ptr                  : 1;  /**< Sel_nxt_ptr signal. */
	uint64_t load_val                     : 1;  /**< Load valid signal. */
	uint64_t rdy                          : 1;  /**< Ready signal. */
	uint64_t cur_state                    : 3;  /**< Current state from the pbuf controller. */
	uint64_t reserved_33_36               : 4;
	uint64_t track_rd_cnt                 : 6;  /**< Track read count value. */
	uint64_t track_wr_cnt                 : 6;  /**< Track write count value. */
	uint64_t reserved_17_20               : 4;
	uint64_t mem_addr                     : 13; /**< Memory address for pbuf ram. */
	uint64_t mem_en                       : 4;  /**< Memory write/chip enable signals. The order of the bits is:
                                                         - 3: low wen; 2: low cen; 1: high wen; 0: high cen. */
#else
	uint64_t mem_en                       : 4;
	uint64_t mem_addr                     : 13;
	uint64_t reserved_17_20               : 4;
	uint64_t track_wr_cnt                 : 6;
	uint64_t track_rd_cnt                 : 6;
	uint64_t reserved_33_36               : 4;
	uint64_t cur_state                    : 3;
	uint64_t rdy                          : 1;
	uint64_t load_val                     : 1;
	uint64_t sel_nxt_ptr                  : 1;
	uint64_t reserved_43_63               : 21;
#endif
	} s;
	struct cvmx_pko_pdm_drpbuf_dbg_s      cn78xx;
};
typedef union cvmx_pko_pdm_drpbuf_dbg cvmx_pko_pdm_drpbuf_dbg_t;

/**
 * cvmx_pko_pdm_dwpbuf_dbg
 */
union cvmx_pko_pdm_dwpbuf_dbg {
	uint64_t u64;
	struct cvmx_pko_pdm_dwpbuf_dbg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_43_63               : 21;
	uint64_t sel_nxt_ptr                  : 1;  /**< Sel_nxt_ptr signal. */
	uint64_t load_val                     : 1;  /**< Load valid signal. */
	uint64_t rdy                          : 1;  /**< Ready signal. */
	uint64_t cur_state                    : 3;  /**< Current state from the pbuf controller. */
	uint64_t reserved_33_36               : 4;
	uint64_t track_rd_cnt                 : 6;  /**< Track read count value. */
	uint64_t track_wr_cnt                 : 6;  /**< Track write count value. */
	uint64_t reserved_17_20               : 4;
	uint64_t mem_addr                     : 13; /**< Memory address for pbuf ram. */
	uint64_t mem_en                       : 4;  /**< Memory write/chip enable signals. The order of the bits is:
                                                         - 3: low wen; 2: low cen; 1: high wen; 0: high cen. */
#else
	uint64_t mem_en                       : 4;
	uint64_t mem_addr                     : 13;
	uint64_t reserved_17_20               : 4;
	uint64_t track_wr_cnt                 : 6;
	uint64_t track_rd_cnt                 : 6;
	uint64_t reserved_33_36               : 4;
	uint64_t cur_state                    : 3;
	uint64_t rdy                          : 1;
	uint64_t load_val                     : 1;
	uint64_t sel_nxt_ptr                  : 1;
	uint64_t reserved_43_63               : 21;
#endif
	} s;
	struct cvmx_pko_pdm_dwpbuf_dbg_s      cn78xx;
};
typedef union cvmx_pko_pdm_dwpbuf_dbg cvmx_pko_pdm_dwpbuf_dbg_t;

/**
 * cvmx_pko_pdm_ecc_ctl0
 */
union cvmx_pko_pdm_ecc_ctl0 {
	uint64_t u64;
	struct cvmx_pko_pdm_ecc_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t flshb_cache_lo_ram_flip      : 2;  /**< FLSHB_CACHE_LO_RAM flip syndrome bits on write. */
	uint64_t flshb_cache_lo_ram_cdis      : 1;  /**< FLSHB_CACHE_LO_RAM ECC correction disable. */
	uint64_t flshb_cache_hi_ram_flip      : 2;  /**< FLSHB_CACHE_HI_RAM flip syndrome bits on write. */
	uint64_t flshb_cache_hi_ram_cdis      : 1;  /**< FLSHB_CACHE_HI_RAM ECC correction disable. */
	uint64_t isrm_ca_iinst_ram_flip       : 2;  /**< ISRM_CA_IINST_RAM flip syndrome bits on write. */
	uint64_t isrm_ca_iinst_ram_cdis       : 1;  /**< ISRM_CA_IINST_RAM ECC correction disable. */
	uint64_t isrm_ca_cm_ram_flip          : 2;  /**< ISRM_CA_CM_RAM flip syndrome bits on write. */
	uint64_t isrm_ca_cm_ram_cdis          : 1;  /**< ISRM_CA_CM_RAM ECC correction disable. */
	uint64_t isrm_st_ram2_flip            : 2;  /**< ISRM_ST_RAM2 flip syndrome bits on write. */
	uint64_t isrm_st_ram2_cdis            : 1;  /**< ISRM_ST_RAM2 ECC correction disable. */
	uint64_t isrm_st_ram1_flip            : 2;  /**< ISRM_ST_RAM1 flip syndrome bits on write. */
	uint64_t isrm_st_ram1_cdis            : 1;  /**< ISRM_ST_RAM1 ECC correction disable. */
	uint64_t isrm_st_ram0_flip            : 2;  /**< ISRM_ST_RAM0 flip syndrome bits on write. */
	uint64_t isrm_st_ram0_cdis            : 1;  /**< ISRM_ST_RAM0 ECC correction disable. */
	uint64_t isrd_st_ram3_flip            : 2;  /**< ISRD_ST_RAM3 flip syndrome bits on write. */
	uint64_t isrd_st_ram3_cdis            : 1;  /**< ISRD_ST_RAM3 ECC correction disable. */
	uint64_t isrd_st_ram2_flip            : 2;  /**< ISRD_ST_RAM2 flip syndrome bits on write. */
	uint64_t isrd_st_ram2_cdis            : 1;  /**< ISRD_ST_RAM2 ECC correction disable. */
	uint64_t isrd_st_ram1_flip            : 2;  /**< ISRD_ST_RAM1 flip syndrome bits on write. */
	uint64_t isrd_st_ram1_cdis            : 1;  /**< ISRD_ST_RAM1 ECC correction disable. */
	uint64_t isrd_st_ram0_flip            : 2;  /**< ISRD_ST_RAM0 flip syndrome bits on write. */
	uint64_t isrd_st_ram0_cdis            : 1;  /**< ISRD_ST_RAM0 ECC correction disable. */
	uint64_t drp_hi_ram_flip              : 2;  /**< DRP_HI_RAM flip syndrome bits on write. */
	uint64_t drp_hi_ram_cdis              : 1;  /**< DRP_HI_RAM ECC correction disable. */
	uint64_t drp_lo_ram_flip              : 2;  /**< DRP_LO_RAM flip syndrome bits on write. */
	uint64_t drp_lo_ram_cdis              : 1;  /**< DRP_LO_RAM ECC correction disable. */
	uint64_t dwp_hi_ram_flip              : 2;  /**< DWP_HI_RAM flip syndrome bits on write. */
	uint64_t dwp_hi_ram_cdis              : 1;  /**< DWP_HI_RAM ECC correction disable. */
	uint64_t dwp_lo_ram_flip              : 2;  /**< DWP_LO_RAM flip syndrome bits on write. */
	uint64_t dwp_lo_ram_cdis              : 1;  /**< DWP_LO_RAM ECC correction disable. */
	uint64_t mwp_hi_ram_flip              : 2;  /**< MWP_HI_RAM flip syndrome bits on write. */
	uint64_t mwp_hi_ram_cdis              : 1;  /**< MWP_HI_RAM ECC correction disable. */
	uint64_t mwp_lo_ram_flip              : 2;  /**< MWP_LO_RAM flip syndrome bits on write. */
	uint64_t mwp_lo_ram_cdis              : 1;  /**< MWP_LO_RAM ECC correction disable. */
	uint64_t fillb_m_rsp_ram_hi_flip      : 2;  /**< FILLB_M_RSP_RAM_HI flip syndrome bits on write. */
	uint64_t fillb_m_rsp_ram_hi_cdis      : 1;  /**< FILLB_M_RSP_RAM_HI ECC correction disable. */
	uint64_t fillb_m_rsp_ram_lo_flip      : 2;  /**< FILLB_M_RSP_RAM_LO flip syndrome bits on write. */
	uint64_t fillb_m_rsp_ram_lo_cdis      : 1;  /**< FILLB_M_RSP_RAM_LO ECC correction disable. */
	uint64_t fillb_d_rsp_ram_hi_flip      : 2;  /**< FILLB_D_RSP_RAM_LO flip syndrome bits on write. */
	uint64_t fillb_d_rsp_ram_hi_cdis      : 1;  /**< FILLB_D_RSP_RAM_HI ECC correction disable. */
	uint64_t fillb_d_rsp_ram_lo_flip      : 2;  /**< FILLB_D_DAT_RAM_LO flip syndrome bits on write. */
	uint64_t fillb_d_rsp_ram_lo_cdis      : 1;  /**< FILLB_D_RSP_RAM_LO ECC correction disable. */
	uint64_t reserved_0_0                 : 1;
#else
	uint64_t reserved_0_0                 : 1;
	uint64_t fillb_d_rsp_ram_lo_cdis      : 1;
	uint64_t fillb_d_rsp_ram_lo_flip      : 2;
	uint64_t fillb_d_rsp_ram_hi_cdis      : 1;
	uint64_t fillb_d_rsp_ram_hi_flip      : 2;
	uint64_t fillb_m_rsp_ram_lo_cdis      : 1;
	uint64_t fillb_m_rsp_ram_lo_flip      : 2;
	uint64_t fillb_m_rsp_ram_hi_cdis      : 1;
	uint64_t fillb_m_rsp_ram_hi_flip      : 2;
	uint64_t mwp_lo_ram_cdis              : 1;
	uint64_t mwp_lo_ram_flip              : 2;
	uint64_t mwp_hi_ram_cdis              : 1;
	uint64_t mwp_hi_ram_flip              : 2;
	uint64_t dwp_lo_ram_cdis              : 1;
	uint64_t dwp_lo_ram_flip              : 2;
	uint64_t dwp_hi_ram_cdis              : 1;
	uint64_t dwp_hi_ram_flip              : 2;
	uint64_t drp_lo_ram_cdis              : 1;
	uint64_t drp_lo_ram_flip              : 2;
	uint64_t drp_hi_ram_cdis              : 1;
	uint64_t drp_hi_ram_flip              : 2;
	uint64_t isrd_st_ram0_cdis            : 1;
	uint64_t isrd_st_ram0_flip            : 2;
	uint64_t isrd_st_ram1_cdis            : 1;
	uint64_t isrd_st_ram1_flip            : 2;
	uint64_t isrd_st_ram2_cdis            : 1;
	uint64_t isrd_st_ram2_flip            : 2;
	uint64_t isrd_st_ram3_cdis            : 1;
	uint64_t isrd_st_ram3_flip            : 2;
	uint64_t isrm_st_ram0_cdis            : 1;
	uint64_t isrm_st_ram0_flip            : 2;
	uint64_t isrm_st_ram1_cdis            : 1;
	uint64_t isrm_st_ram1_flip            : 2;
	uint64_t isrm_st_ram2_cdis            : 1;
	uint64_t isrm_st_ram2_flip            : 2;
	uint64_t isrm_ca_cm_ram_cdis          : 1;
	uint64_t isrm_ca_cm_ram_flip          : 2;
	uint64_t isrm_ca_iinst_ram_cdis       : 1;
	uint64_t isrm_ca_iinst_ram_flip       : 2;
	uint64_t flshb_cache_hi_ram_cdis      : 1;
	uint64_t flshb_cache_hi_ram_flip      : 2;
	uint64_t flshb_cache_lo_ram_cdis      : 1;
	uint64_t flshb_cache_lo_ram_flip      : 2;
#endif
	} s;
	struct cvmx_pko_pdm_ecc_ctl0_s        cn78xx;
};
typedef union cvmx_pko_pdm_ecc_ctl0 cvmx_pko_pdm_ecc_ctl0_t;

/**
 * cvmx_pko_pdm_ecc_ctl1
 */
union cvmx_pko_pdm_ecc_ctl1 {
	uint64_t u64;
	struct cvmx_pko_pdm_ecc_ctl1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t minpad_ram_flip              : 2;  /**< MINPAD_RAM flip syndrome bits on write. */
	uint64_t minpad_ram_cdis              : 1;  /**< MINPAD_RAM ECC correction disable. */
#else
	uint64_t minpad_ram_cdis              : 1;
	uint64_t minpad_ram_flip              : 2;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_pko_pdm_ecc_ctl1_s        cn78xx;
};
typedef union cvmx_pko_pdm_ecc_ctl1 cvmx_pko_pdm_ecc_ctl1_t;

/**
 * cvmx_pko_pdm_ecc_dbe_sts0
 */
union cvmx_pko_pdm_ecc_dbe_sts0 {
	uint64_t u64;
	struct cvmx_pko_pdm_ecc_dbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t flshb_cache_lo_ram_dbe       : 1;  /**< Double-bit error for FLSHB_CACHE_LO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.flshb.flshb_cache_lo */
	uint64_t flshb_cache_hi_ram_dbe       : 1;  /**< Double-bit error for FLSHB_CACHE_HI_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.flshb.flshb_cache_hi */
	uint64_t isrm_ca_iinst_ram_dbe        : 1;  /**< Double-bit error for ISRM_CA_IINST_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.isr.mp_isr.cred_accum.iinst_in_fif */
	uint64_t isrm_ca_cm_ram_dbe           : 1;  /**< Double-bit error for ISRM_CA_CM_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.isr.mp_isr.cred_accum.cred_accum_ctrlr_and_mem.cred_accum_spr */
	uint64_t isrm_st_ram2_dbe             : 1;  /**< Double-bit error for ISRM_ST_RAM2.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.isr.mp_isr.st_mem2 */
	uint64_t isrm_st_ram1_dbe             : 1;  /**< Double-bit error for ISRM_ST_RAM1.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.isr.mp_isr.st_mem1 */
	uint64_t isrm_st_ram0_dbe             : 1;  /**< Double-bit error for ISRM_ST_RAM0.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.isr.mp_isr.st_mem0 */
	uint64_t isrd_st_ram3_dbe             : 1;  /**< Double-bit error for ISRD_ST_RAM3.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.isr.d_isr.st_mem3 */
	uint64_t isrd_st_ram2_dbe             : 1;  /**< Double-bit error for ISRD_ST_RAM2.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.isr.d_isr.st_mem2 */
	uint64_t isrd_st_ram1_dbe             : 1;  /**< Double-bit error for ISRD_ST_RAM1.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.isr.d_isr.st_mem1 */
	uint64_t isrd_st_ram0_dbe             : 1;  /**< Double-bit error for ISRD_ST_RAM0.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.isr.d_isr.st_mem0 */
	uint64_t drp_hi_ram_dbe               : 1;  /**< Double-bit error for DRP_HI_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.drpbuf.ram_128k_pbuf_1 */
	uint64_t drp_lo_ram_dbe               : 1;  /**< Double-bit error for DRP_LO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.drpbuf.ram_128k_pbuf_2 */
	uint64_t dwp_hi_ram_dbe               : 1;  /**< Double-bit error for DWP_HI_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.dwpbuf.ram_128k_pbuf_high */
	uint64_t dwp_lo_ram_dbe               : 1;  /**< Double-bit error for DWP_LO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.dwpbuf.ram_128k_pbuf_low */
	uint64_t mwp_hi_ram_dbe               : 1;  /**< Double-bit error for MWP_HI_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.mwpbuf.ram_128k_pbuf_high */
	uint64_t mwp_lo_ram_dbe               : 1;  /**< Double-bit error for MWP_LO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.mwpbuf.ram_128k_pbuf_low */
	uint64_t fillb_m_rsp_ram_hi_dbe       : 1;  /**< Double-bit error for FILLB_M_DAT_RAM_HI.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.fillb.m_rsp_ram_hi */
	uint64_t fillb_m_rsp_ram_lo_dbe       : 1;  /**< Double-bit error for FILLB_D_DAT_RAM_LO.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.fillb.m_rsp_ram_lo */
	uint64_t fillb_d_rsp_ram_hi_dbe       : 1;  /**< Double-bit error for FILLB_D_DAT_RAM_HI.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.fillb.d_rsp_ram_hi */
	uint64_t fillb_d_rsp_ram_lo_dbe       : 1;  /**< Double-bit error for FILLB_D_DAT_RAM_LO.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.fillb.d_rsp_ram_lo */
	uint64_t minpad_ram_dbe               : 1;  /**< Double-bit error for MINPAD_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.cp.minpad_ram */
	uint64_t reserved_0_41                : 42;
#else
	uint64_t reserved_0_41                : 42;
	uint64_t minpad_ram_dbe               : 1;
	uint64_t fillb_d_rsp_ram_lo_dbe       : 1;
	uint64_t fillb_d_rsp_ram_hi_dbe       : 1;
	uint64_t fillb_m_rsp_ram_lo_dbe       : 1;
	uint64_t fillb_m_rsp_ram_hi_dbe       : 1;
	uint64_t mwp_lo_ram_dbe               : 1;
	uint64_t mwp_hi_ram_dbe               : 1;
	uint64_t dwp_lo_ram_dbe               : 1;
	uint64_t dwp_hi_ram_dbe               : 1;
	uint64_t drp_lo_ram_dbe               : 1;
	uint64_t drp_hi_ram_dbe               : 1;
	uint64_t isrd_st_ram0_dbe             : 1;
	uint64_t isrd_st_ram1_dbe             : 1;
	uint64_t isrd_st_ram2_dbe             : 1;
	uint64_t isrd_st_ram3_dbe             : 1;
	uint64_t isrm_st_ram0_dbe             : 1;
	uint64_t isrm_st_ram1_dbe             : 1;
	uint64_t isrm_st_ram2_dbe             : 1;
	uint64_t isrm_ca_cm_ram_dbe           : 1;
	uint64_t isrm_ca_iinst_ram_dbe        : 1;
	uint64_t flshb_cache_hi_ram_dbe       : 1;
	uint64_t flshb_cache_lo_ram_dbe       : 1;
#endif
	} s;
	struct cvmx_pko_pdm_ecc_dbe_sts0_s    cn78xx;
};
typedef union cvmx_pko_pdm_ecc_dbe_sts0 cvmx_pko_pdm_ecc_dbe_sts0_t;

/**
 * cvmx_pko_pdm_ecc_dbe_sts_cmb0
 */
union cvmx_pko_pdm_ecc_dbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pko_pdm_ecc_dbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pdm_dbe_cmb0                 : 1;  /**< This bit is the logical OR of all bits in PKO_PDM_ECC_DBE_STS.
                                                         To clear this bit, software must clear bits in PKO_PDM_ECC_DBE_STS.
                                                         When this bit is set, the corresponding interrupt is set.
                                                         Throws PKO_INTSN_E::PKO_PDM_DBE_CMB0.
                                                         INTERNAL: Instances:
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.flshb.flshb_cache_hi
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.flshb.flshb_cache_lo
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.isr.mp_isr.cred_accum.iinst_in_fif
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.isr.mp_isr.cred_accum.cred_accum_ctrlr_and_mem.cred_
                                                         accum_spr
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.isr.mp_isr.st_mem0
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.isr.mp_isr.st_mem1
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.isr.mp_isr.st_mem2
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.isr.d_isr.st_mem0
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.isr.d_isr.st_mem1
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.isr.d_isr.st_mem2
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.isr.d_isr.st_mem3
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.drpbuf.ram_128k_pbuf_1
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.drpbuf.ram_128k_pbuf_2
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.dwpbuf.ram_128k_pbuf_low
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.dwpbuf.ram_128k_pbuf_high
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.mwpbuf.ram_128k_pbuf_low
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.mwpbuf.ram_128k_pbuf_high
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.fillb.d_rsp_ram_hi
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.fillb.d_rsp_ram_lo
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.fillb.m_rsp_dat_fifo
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.cp.minpad_ram */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pdm_dbe_cmb0                 : 1;
#endif
	} s;
	struct cvmx_pko_pdm_ecc_dbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pko_pdm_ecc_dbe_sts_cmb0 cvmx_pko_pdm_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pko_pdm_ecc_sbe_sts0
 */
union cvmx_pko_pdm_ecc_sbe_sts0 {
	uint64_t u64;
	struct cvmx_pko_pdm_ecc_sbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t flshb_cache_lo_ram_sbe       : 1;  /**< Single-bit error for FLSHB_CACHE_LO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.flshb.flshb_cache_lo */
	uint64_t flshb_cache_hi_ram_sbe       : 1;  /**< Single-bit error for FLSHB_CACHE_HI_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.flshb.flshb_cache_hi */
	uint64_t isrm_ca_iinst_ram_sbe        : 1;  /**< Single-bit error for ISRM_CA_IINST_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.isr.mp_isr.cred_accum.iinst_in_fif */
	uint64_t isrm_ca_cm_ram_sbe           : 1;  /**< Single-bit error for ISRM_CA_CM_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.isr.mp_isr.cred_accum.cred_accum_ctrlr_and_mem.cred_accum_spr */
	uint64_t isrm_st_ram2_sbe             : 1;  /**< Single-bit error for ISRM_ST_RAM2.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.isr.mp_isr.st_mem2 */
	uint64_t isrm_st_ram1_sbe             : 1;  /**< Single-bit error for ISRM_ST_RAM1.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.isr.mp_isr.st_mem1 */
	uint64_t isrm_st_ram0_sbe             : 1;  /**< Single-bit error for ISRM_ST_RAM0.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.isr.mp_isr.st_mem0 */
	uint64_t isrd_st_ram3_sbe             : 1;  /**< Single-bit error for ISRD_ST_RAM3.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.isr.d_isr.st_mem3 */
	uint64_t isrd_st_ram2_sbe             : 1;  /**< Single-bit error for ISRD_ST_RAM2.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.isr.d_isr.st_mem2 */
	uint64_t isrd_st_ram1_sbe             : 1;  /**< Single-bit error for ISRD_ST_RAM1.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.isr.d_isr.st_mem1 */
	uint64_t isrd_st_ram0_sbe             : 1;  /**< Single-bit error for ISRD_ST_RAM0.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.isr.d_isr.st_mem0 */
	uint64_t drp_hi_ram_sbe               : 1;  /**< Single-bit error for DRP_HI_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.drpbuf.ram_128k_pbuf_1 */
	uint64_t drp_lo_ram_sbe               : 1;  /**< Single-bit error for DRP_LO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.drpbuf.ram_128k_pbuf_2 */
	uint64_t dwp_hi_ram_sbe               : 1;  /**< Single-bit error for DWP_HI_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.dwpbuf.ram_128k_pbuf_high */
	uint64_t dwp_lo_ram_sbe               : 1;  /**< Single-bit error for DWP_LO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.dwpbuf.ram_128k_pbuf_low */
	uint64_t mwp_hi_ram_sbe               : 1;  /**< Single-bit error for MWP_HI_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.mwpbuf.ram_128k_pbuf_high */
	uint64_t mwp_lo_ram_sbe               : 1;  /**< Single-bit error for MWP_LO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.mwpbuf.ram_128k_pbuf_low */
	uint64_t fillb_m_rsp_ram_hi_sbe       : 1;  /**< Single-bit error for FILLB_M_RSP_RAM_HI.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.fillb.m_rsp_ram_hi */
	uint64_t fillb_m_rsp_ram_lo_sbe       : 1;  /**< Single-bit error for FILLB_M_RSP_RAM_LO.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.fillb.m_rsp_ram_lo */
	uint64_t fillb_d_rsp_ram_hi_sbe       : 1;  /**< Single-bit error for FILLB_D_RSP_RAM_HI.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.fillb.d_rsp_ram_hi */
	uint64_t fillb_d_rsp_ram_lo_sbe       : 1;  /**< Single-bit error for FILLB_D_RSP_RAM_LO.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.fillb.d_rsp_ram_lo */
	uint64_t minpad_ram_sbe               : 1;  /**< Single-bit error for MINPAD_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr1.pko_pnr1_pdm.cp.minpad_ram */
	uint64_t reserved_0_41                : 42;
#else
	uint64_t reserved_0_41                : 42;
	uint64_t minpad_ram_sbe               : 1;
	uint64_t fillb_d_rsp_ram_lo_sbe       : 1;
	uint64_t fillb_d_rsp_ram_hi_sbe       : 1;
	uint64_t fillb_m_rsp_ram_lo_sbe       : 1;
	uint64_t fillb_m_rsp_ram_hi_sbe       : 1;
	uint64_t mwp_lo_ram_sbe               : 1;
	uint64_t mwp_hi_ram_sbe               : 1;
	uint64_t dwp_lo_ram_sbe               : 1;
	uint64_t dwp_hi_ram_sbe               : 1;
	uint64_t drp_lo_ram_sbe               : 1;
	uint64_t drp_hi_ram_sbe               : 1;
	uint64_t isrd_st_ram0_sbe             : 1;
	uint64_t isrd_st_ram1_sbe             : 1;
	uint64_t isrd_st_ram2_sbe             : 1;
	uint64_t isrd_st_ram3_sbe             : 1;
	uint64_t isrm_st_ram0_sbe             : 1;
	uint64_t isrm_st_ram1_sbe             : 1;
	uint64_t isrm_st_ram2_sbe             : 1;
	uint64_t isrm_ca_cm_ram_sbe           : 1;
	uint64_t isrm_ca_iinst_ram_sbe        : 1;
	uint64_t flshb_cache_hi_ram_sbe       : 1;
	uint64_t flshb_cache_lo_ram_sbe       : 1;
#endif
	} s;
	struct cvmx_pko_pdm_ecc_sbe_sts0_s    cn78xx;
};
typedef union cvmx_pko_pdm_ecc_sbe_sts0 cvmx_pko_pdm_ecc_sbe_sts0_t;

/**
 * cvmx_pko_pdm_ecc_sbe_sts_cmb0
 */
union cvmx_pko_pdm_ecc_sbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pko_pdm_ecc_sbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pdm_sbe_cmb0                 : 1;  /**< This bit is the logical OR of all bits in PKO_PDM_ECC_SBE_STS.
                                                         To clear this bit, software must clear bits in PKO_PDM_ECC_SBE_STS.
                                                         When this bit is set, the corresponding interrupt is set.
                                                         Throws PKO_INTSN_E::PKO_PDM_SBE_CMB0.
                                                         INTERNAL: Instances:
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.flshb.flshb_cache_hi
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.flshb.flshb_cache_lo
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.isr.mp_isr.cred_accum.iinst_in_fif
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.isr.mp_isr.cred_accum.cred_accum_ctrlr_and_mem.cred_
                                                         accum_spr
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.isr.mp_isr.st_mem0
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.isr.mp_isr.st_mem1
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.isr.mp_isr.st_mem2
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.isr.d_isr.st_mem0
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.isr.d_isr.st_mem1
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.isr.d_isr.st_mem2
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.isr.d_isr.st_mem3
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.drpbuf.ram_128k_pbuf_1
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.drpbuf.ram_128k_pbuf_2
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.dwpbuf.ram_128k_pbuf_low
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.dwpbuf.ram_128k_pbuf_high
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.mwpbuf.ram_128k_pbuf_low
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.mwpbuf.ram_128k_pbuf_high
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.fillb.d_rsp_ram_hi
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.fillb.d_rsp_ram_lo
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.fillb.m_rsp_dat_fifo
                                                         fc.core.roc.pko.pko_pnr1.pko_pnr1_pdm.cp.minpad_ram */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pdm_sbe_cmb0                 : 1;
#endif
	} s;
	struct cvmx_pko_pdm_ecc_sbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pko_pdm_ecc_sbe_sts_cmb0 cvmx_pko_pdm_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pko_pdm_fillb_dbg0
 */
union cvmx_pko_pdm_fillb_dbg0 {
	uint64_t u64;
	struct cvmx_pko_pdm_fillb_dbg0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_57_63               : 7;
	uint64_t pd_seq                       : 5;  /**< Sequence number for next Packet Descriptor fill request */
	uint64_t resp_pd_seq                  : 5;  /**< Sequence number for next response to be written into Packet Descriptor Buffer RAM */
	uint64_t d_rsp_lo_ram_addr_sel        : 2;  /**< Source of read/write address to Low PD Fill buffer RAM.
                                                         0x0 = No access.
                                                         0x1 = Read access sourced by PD Fill Buffer Response FIFO (feeding DRPBUF).
                                                         0x2 = Write access sourced by IOBP0.
                                                         0x3 = Write access sourced by Flush Buffer. */
	uint64_t d_rsp_hi_ram_addr_sel        : 2;  /**< Source of read/write address to High PD Fill buffer RAM.
                                                         0x0 = No access.
                                                         0x1 = Read  access sourced by PD Fill Buffer Response FIFO (feeding DRPBUF).
                                                         0x2 = Write access sourced by IOBP0.
                                                         0x3 = Write access sourced by Flush Buffer. */
	uint64_t d_rsp_rd_seq                 : 5;  /**< Sequence number for next response to be read from Packet Descriptor Buffer RAM */
	uint64_t d_rsp_fifo_rd_seq            : 5;  /**< Sequence number for next PD Fill response to be sent to DRPBUF */
	uint64_t d_fill_req_fifo_val          : 1;  /**< Fill Buffer PD read request FIFO has a valid entry */
	uint64_t d_rsp_ram_valid              : 32; /**< Fill Buffer Packet Descriptor RAM valid flags */
#else
	uint64_t d_rsp_ram_valid              : 32;
	uint64_t d_fill_req_fifo_val          : 1;
	uint64_t d_rsp_fifo_rd_seq            : 5;
	uint64_t d_rsp_rd_seq                 : 5;
	uint64_t d_rsp_hi_ram_addr_sel        : 2;
	uint64_t d_rsp_lo_ram_addr_sel        : 2;
	uint64_t resp_pd_seq                  : 5;
	uint64_t pd_seq                       : 5;
	uint64_t reserved_57_63               : 7;
#endif
	} s;
	struct cvmx_pko_pdm_fillb_dbg0_s      cn78xx;
};
typedef union cvmx_pko_pdm_fillb_dbg0 cvmx_pko_pdm_fillb_dbg0_t;

/**
 * cvmx_pko_pdm_fillb_dbg1
 */
union cvmx_pko_pdm_fillb_dbg1 {
	uint64_t u64;
	struct cvmx_pko_pdm_fillb_dbg1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_57_63               : 7;
	uint64_t mp_seq                       : 5;  /**< Sequence number for next Meta-Packet Cache-line fill request */
	uint64_t resp_mp_seq                  : 5;  /**< Sequence number for next response to be written into Meta-Packet Buffer RAM */
	uint64_t m_rsp_lo_ram_addr_sel        : 2;  /**< Source of read/write address to Low MP Fill buffer RAM.
                                                         0x0 = No access.
                                                         0x1 = Read  access sourced by MP Fill Buffer Response FIFO (feeding DRPBUF).
                                                         0x2 = Write access sourced by IOBP0.
                                                         0x3 = Write access sourced by Flush Buffer. */
	uint64_t m_rsp_hi_ram_addr_sel        : 2;  /**< Source of read/write address to High MP Fill buffer RAM.
                                                         0x0 = No access.
                                                         0x1 = Read access sourced by MP Fill Buffer Response FIFO (feeding DRPBUF).
                                                         0x2 = Write access sourced by IOBP0.
                                                         0x3 = Write access sourced by Flush Buffer. */
	uint64_t m_rsp_rd_seq                 : 5;  /**< Sequence number for next response to be read from Meta-Packet Buffer RAM */
	uint64_t m_rsp_fifo_rd_seq            : 5;  /**< Sequence number for next MP Fill response to be sent to DRPBUF */
	uint64_t m_fill_req_fifo_val          : 1;  /**< Fill Buffer MP read request FIFO has a valid entry */
	uint64_t m_rsp_ram_valid              : 32; /**< Fill Buffer Meta-Packet RAM valid flags */
#else
	uint64_t m_rsp_ram_valid              : 32;
	uint64_t m_fill_req_fifo_val          : 1;
	uint64_t m_rsp_fifo_rd_seq            : 5;
	uint64_t m_rsp_rd_seq                 : 5;
	uint64_t m_rsp_hi_ram_addr_sel        : 2;
	uint64_t m_rsp_lo_ram_addr_sel        : 2;
	uint64_t resp_mp_seq                  : 5;
	uint64_t mp_seq                       : 5;
	uint64_t reserved_57_63               : 7;
#endif
	} s;
	struct cvmx_pko_pdm_fillb_dbg1_s      cn78xx;
};
typedef union cvmx_pko_pdm_fillb_dbg1 cvmx_pko_pdm_fillb_dbg1_t;

/**
 * cvmx_pko_pdm_fillb_dbg2
 */
union cvmx_pko_pdm_fillb_dbg2 {
	uint64_t u64;
	struct cvmx_pko_pdm_fillb_dbg2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t fillb_sm                     : 5;  /**< Fill Buffer State Machine */
	uint64_t reserved_3_3                 : 1;
	uint64_t iobp0_credit_cntr            : 3;  /**< IOBP0 Read Request credit counter */
#else
	uint64_t iobp0_credit_cntr            : 3;
	uint64_t reserved_3_3                 : 1;
	uint64_t fillb_sm                     : 5;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_pko_pdm_fillb_dbg2_s      cn78xx;
};
typedef union cvmx_pko_pdm_fillb_dbg2 cvmx_pko_pdm_fillb_dbg2_t;

/**
 * cvmx_pko_pdm_flshb_dbg0
 */
union cvmx_pko_pdm_flshb_dbg0 {
	uint64_t u64;
	struct cvmx_pko_pdm_flshb_dbg0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_44_63               : 20;
	uint64_t flshb_sm                     : 7;  /**< Flush Buffer State Machine */
	uint64_t flshb_ctl_sm                 : 9;  /**< Flush Buffer Control State Machine */
	uint64_t cam_hptr                     : 5;  /**< Flush Buffer CAM Head Pointer */
	uint64_t cam_tptr                     : 5;  /**< Flush Buffer CAM Tail Pointer */
	uint64_t expected_stdns               : 6;  /**< Number of Store Done Responses still pending */
	uint64_t d_flshb_eot_cntr             : 3;  /**< Number of Packet Descriptor Flush requests pending */
	uint64_t m_flshb_eot_cntr             : 3;  /**< Number of Meta-packet Flush requests pending */
	uint64_t ncbi_credit_cntr             : 6;  /**< NCBI FIFO credit counter */
#else
	uint64_t ncbi_credit_cntr             : 6;
	uint64_t m_flshb_eot_cntr             : 3;
	uint64_t d_flshb_eot_cntr             : 3;
	uint64_t expected_stdns               : 6;
	uint64_t cam_tptr                     : 5;
	uint64_t cam_hptr                     : 5;
	uint64_t flshb_ctl_sm                 : 9;
	uint64_t flshb_sm                     : 7;
	uint64_t reserved_44_63               : 20;
#endif
	} s;
	struct cvmx_pko_pdm_flshb_dbg0_s      cn78xx;
};
typedef union cvmx_pko_pdm_flshb_dbg0 cvmx_pko_pdm_flshb_dbg0_t;

/**
 * cvmx_pko_pdm_flshb_dbg1
 */
union cvmx_pko_pdm_flshb_dbg1 {
	uint64_t u64;
	struct cvmx_pko_pdm_flshb_dbg1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t cam_stdn                     : 32; /**< Flush Buffer Entry Store Done Request flags */
	uint64_t cam_valid                    : 32; /**< Flush Buffer Entry Valid flags */
#else
	uint64_t cam_valid                    : 32;
	uint64_t cam_stdn                     : 32;
#endif
	} s;
	struct cvmx_pko_pdm_flshb_dbg1_s      cn78xx;
};
typedef union cvmx_pko_pdm_flshb_dbg1 cvmx_pko_pdm_flshb_dbg1_t;

/**
 * cvmx_pko_pdm_isrd_dbg
 */
union cvmx_pko_pdm_isrd_dbg {
	uint64_t u64;
	struct cvmx_pko_pdm_isrd_dbg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_44_63               : 20;
	uint64_t in_arb_reqs                  : 8;  /**< Input arbitration request signals. The order of the bits is:
                                                         - 43: Fill response - normal path request
                                                         - 42: Fill response - flushb path request
                                                         - 41: CP queue-open request
                                                         - 40: CP queue-closed request
                                                         - 39: CP queue-query request
                                                         - 38: CP send-packet request
                                                         - 37: PEB fill request
                                                         - 36: PEB read request */
	uint64_t in_arb_gnts                  : 7;  /**< Input arbitration grant signals. The order of the bits is:
                                                         - 35: Fill response grant
                                                         - 34: CP - queue-open grant
                                                         - 33: CP - queue-close grant
                                                         - 32: CP - queue-query grant
                                                         - 31: CP - send-packet grant
                                                         - 30: PEB fill grant
                                                         - 29: PEB read grant */
	uint64_t cmt_arb_reqs                 : 7;  /**< Commit arbitration request signals. The order of the bits is:
                                                         - 28: Fill response grant
                                                         - 27: CP - queue-open grant
                                                         - 26: CP - queue-close grant
                                                         - 25: CP - queue-query grant
                                                         - 24: CP - send-packet grant
                                                         - 23: PEB fill grant
                                                         - 22: PEB read grant */
	uint64_t cmt_arb_gnts                 : 7;  /**< Commit arbitration grant signals. The order of the bits is:
                                                         - 21: Fill response grant
                                                         - 20: CP - queue-open grant
                                                         - 19: CP - queue-close grant
                                                         - 18: CP - queue-query grant
                                                         - 17: CP - send-packet grant
                                                         - 16: PEB fill grant
                                                         - 15: PEB read grant */
	uint64_t in_use                       : 4;  /**< In use signals indicate the execution units are in use. The order of the bits is:
                                                         - 14: PEB fill unit
                                                         - 13: PEB read unit
                                                         - 12: CP unit
                                                         - 11: Fill response unit */
	uint64_t has_cred                     : 4;  /**< Has credit signals indicate there is sufficient credit to commit. The order of the bits
                                                          is:
                                                          - 10: Flush buffer has credit
                                                         - 9: Fill buffer has credit
                                                         - 8: DW command output FIFO has credit
                                                         - 7: DR command output FIFO has credit */
	uint64_t val_exec                     : 7;  /**< Valid bits for the execution units; means the unit can commit if it gets the grant of the
                                                          commit arb and other conditions are met. The order of the bits is:
                                                         - 6: Fill response unit
                                                         - 5: CP unit - queue-open
                                                         - 4: CP unit - queue-close
                                                         - 3: CP unit - queue-probe
                                                         - 2: CP unit - send-packet
                                                         - 1: PEB fill unit
                                                         - 0: PEB read unit */
#else
	uint64_t val_exec                     : 7;
	uint64_t has_cred                     : 4;
	uint64_t in_use                       : 4;
	uint64_t cmt_arb_gnts                 : 7;
	uint64_t cmt_arb_reqs                 : 7;
	uint64_t in_arb_gnts                  : 7;
	uint64_t in_arb_reqs                  : 8;
	uint64_t reserved_44_63               : 20;
#endif
	} s;
	struct cvmx_pko_pdm_isrd_dbg_s        cn78xx;
};
typedef union cvmx_pko_pdm_isrd_dbg cvmx_pko_pdm_isrd_dbg_t;

/**
 * cvmx_pko_pdm_isrd_dbg_dq
 */
union cvmx_pko_pdm_isrd_dbg_dq {
	uint64_t u64;
	struct cvmx_pko_pdm_isrd_dbg_dq_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_46_63               : 18;
	uint64_t pebrd_sic_dq                 : 10; /**< CP SIC's DQ number. */
	uint64_t reserved_34_35               : 2;
	uint64_t pebfill_sic_dq               : 10; /**< CP SIC's DQ number. */
	uint64_t reserved_22_23               : 2;
	uint64_t fr_sic_dq                    : 10; /**< CP SIC's DQ number. */
	uint64_t reserved_10_11               : 2;
	uint64_t cp_sic_dq                    : 10; /**< CP SIC's DQ number. */
#else
	uint64_t cp_sic_dq                    : 10;
	uint64_t reserved_10_11               : 2;
	uint64_t fr_sic_dq                    : 10;
	uint64_t reserved_22_23               : 2;
	uint64_t pebfill_sic_dq               : 10;
	uint64_t reserved_34_35               : 2;
	uint64_t pebrd_sic_dq                 : 10;
	uint64_t reserved_46_63               : 18;
#endif
	} s;
	struct cvmx_pko_pdm_isrd_dbg_dq_s     cn78xx;
};
typedef union cvmx_pko_pdm_isrd_dbg_dq cvmx_pko_pdm_isrd_dbg_dq_t;

/**
 * cvmx_pko_pdm_isrm_dbg
 */
union cvmx_pko_pdm_isrm_dbg {
	uint64_t u64;
	struct cvmx_pko_pdm_isrm_dbg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_34_63               : 30;
	uint64_t in_arb_reqs                  : 7;  /**< Input arbitration request signals. The order of the bits is:
                                                         - 33: PSE ACK
                                                         - 32: Fill Response - normal path request
                                                         - 31: Fill Response - flushb path request
                                                         - 30: CP queue-open
                                                         - 29: CP queue-closed
                                                         - 28: CP queue-query
                                                         - 27: CP send-packet */
	uint64_t in_arb_gnts                  : 6;  /**< Input arbitration grant signals. The order of the bits is:
                                                         - 26: PSE ACK
                                                         - 25: Fill Response
                                                         - 24: CP - queue-open
                                                         - 23: CP - queue-close
                                                         - 22: CP - queue-query
                                                         - 21: CP - send-packet */
	uint64_t cmt_arb_reqs                 : 6;  /**< Commit arbitration request signals. The order of the bits is:
                                                         - 20: PSE ACK
                                                         - 19: Fill Response
                                                         - 18: CP - queue-open
                                                         - 17: CP - queue-close
                                                         - 16: CP - queue-query
                                                         - 15: CP - send-packet */
	uint64_t cmt_arb_gnts                 : 6;  /**< Commit arbitration grant signals. The order of the bits is:
                                                          - 14: PSE ACK
                                                          - 13: Fill Response
                                                          - 12: CP - queue-open
                                                          - 11: CP - queue-close
                                                          - 10: CP - queue-query
                                                         - 9: CP - send-packet */
	uint64_t in_use                       : 3;  /**< In use signals indicate the execution units are in use. The order of the bits is:
                                                         - 8: (PSE) ACK unit
                                                         - 7: Fill response unit
                                                         - 6: CP unit */
	uint64_t has_cred                     : 3;  /**< Has credit signals indicate there is sufficient credit to commit. The order of the bits
                                                          is:
                                                         - 5: Flush buffer has credit
                                                         - 4: Fill buffer has credit
                                                         - 3: MWP command output FIFO has credit */
	uint64_t val_exec                     : 3;  /**< Valid bits for the execution units; means the unit can commit if it gets the grant of the
                                                          commit arb and other conditions are met. The order of the bits is:
                                                         - 2: (PSE) ACK unit
                                                         - 1: Fill response unit
                                                         - 0: CP unit - ALL */
#else
	uint64_t val_exec                     : 3;
	uint64_t has_cred                     : 3;
	uint64_t in_use                       : 3;
	uint64_t cmt_arb_gnts                 : 6;
	uint64_t cmt_arb_reqs                 : 6;
	uint64_t in_arb_gnts                  : 6;
	uint64_t in_arb_reqs                  : 7;
	uint64_t reserved_34_63               : 30;
#endif
	} s;
	struct cvmx_pko_pdm_isrm_dbg_s        cn78xx;
};
typedef union cvmx_pko_pdm_isrm_dbg cvmx_pko_pdm_isrm_dbg_t;

/**
 * cvmx_pko_pdm_isrm_dbg_dq
 */
union cvmx_pko_pdm_isrm_dbg_dq {
	uint64_t u64;
	struct cvmx_pko_pdm_isrm_dbg_dq_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_34_63               : 30;
	uint64_t ack_sic_dq                   : 10; /**< CP SIC's DQ number. */
	uint64_t reserved_22_23               : 2;
	uint64_t fr_sic_dq                    : 10; /**< CP SIC's DQ number. */
	uint64_t reserved_10_11               : 2;
	uint64_t cp_sic_dq                    : 10; /**< CP SIC's DQ number. */
#else
	uint64_t cp_sic_dq                    : 10;
	uint64_t reserved_10_11               : 2;
	uint64_t fr_sic_dq                    : 10;
	uint64_t reserved_22_23               : 2;
	uint64_t ack_sic_dq                   : 10;
	uint64_t reserved_34_63               : 30;
#endif
	} s;
	struct cvmx_pko_pdm_isrm_dbg_dq_s     cn78xx;
};
typedef union cvmx_pko_pdm_isrm_dbg_dq cvmx_pko_pdm_isrm_dbg_dq_t;

/**
 * cvmx_pko_pdm_mem_addr
 */
union cvmx_pko_pdm_mem_addr {
	uint64_t u64;
	struct cvmx_pko_pdm_mem_addr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t memsel                       : 3;  /**< Memory select. Selects the RAM to read or write to.
                                                         0 = Invalid, 1 = ISRM states, 2 = ISRD states, 3 = DWPBUF, 4 = DRPBUF, 5 = MWPBUF */
	uint64_t reserved_17_60               : 44;
	uint64_t memaddr                      : 14; /**< Memory address for the RAM. */
	uint64_t reserved_2_2                 : 1;
	uint64_t membanksel                   : 2;  /**< Memory bank select. Selects the bank to write to. Note that bit 0 is the only bit used in
                                                         the PBUF's because there are only 2 banks per each PBUF. In the ISRM bank sel 3 is
                                                         illegal. */
#else
	uint64_t membanksel                   : 2;
	uint64_t reserved_2_2                 : 1;
	uint64_t memaddr                      : 14;
	uint64_t reserved_17_60               : 44;
	uint64_t memsel                       : 3;
#endif
	} s;
	struct cvmx_pko_pdm_mem_addr_s        cn78xx;
};
typedef union cvmx_pko_pdm_mem_addr cvmx_pko_pdm_mem_addr_t;

/**
 * cvmx_pko_pdm_mem_data
 */
union cvmx_pko_pdm_mem_data {
	uint64_t u64;
	struct cvmx_pko_pdm_mem_data_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Raw data to write into the memory, or the raw data read out from the memory.
                                                         Note that the ISR RAMs are only 57 bits wide, so [56:0] are the only bits that can be read
                                                         or written to them. The PBUFs are 64 bits wide. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_pko_pdm_mem_data_s        cn78xx;
};
typedef union cvmx_pko_pdm_mem_data cvmx_pko_pdm_mem_data_t;

/**
 * cvmx_pko_pdm_mem_rw_ctl
 */
union cvmx_pko_pdm_mem_rw_ctl {
	uint64_t u64;
	struct cvmx_pko_pdm_mem_rw_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t read                         : 1;  /**< Set to 1 to read memory. */
	uint64_t write                        : 1;  /**< Set to 1 to write memory. */
#else
	uint64_t write                        : 1;
	uint64_t read                         : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_pko_pdm_mem_rw_ctl_s      cn78xx;
};
typedef union cvmx_pko_pdm_mem_rw_ctl cvmx_pko_pdm_mem_rw_ctl_t;

/**
 * cvmx_pko_pdm_mem_rw_sts
 */
union cvmx_pko_pdm_mem_rw_sts {
	uint64_t u64;
	struct cvmx_pko_pdm_mem_rw_sts_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t readdone                     : 1;  /**< This bit is set to 1 when the read is complete and the data is valid in the data register. */
#else
	uint64_t readdone                     : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_pko_pdm_mem_rw_sts_s      cn78xx;
};
typedef union cvmx_pko_pdm_mem_rw_sts cvmx_pko_pdm_mem_rw_sts_t;

/**
 * cvmx_pko_pdm_mwpbuf_dbg
 */
union cvmx_pko_pdm_mwpbuf_dbg {
	uint64_t u64;
	struct cvmx_pko_pdm_mwpbuf_dbg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_43_63               : 21;
	uint64_t sel_nxt_ptr                  : 1;  /**< Sel_nxt_ptr signal. */
	uint64_t load_val                     : 1;  /**< Load valid signal. */
	uint64_t rdy                          : 1;  /**< Ready signal. */
	uint64_t cur_state                    : 3;  /**< Current state from the pbuf controller. */
	uint64_t reserved_33_36               : 4;
	uint64_t track_rd_cnt                 : 6;  /**< Track read count value. */
	uint64_t track_wr_cnt                 : 6;  /**< Track write count value. */
	uint64_t reserved_17_20               : 4;
	uint64_t mem_addr                     : 13; /**< Memory address for pbuf ram. */
	uint64_t mem_en                       : 4;  /**< Memory write/chip enable signals. The order of the bits is:
                                                         - 3: low wen; 2: low cen; 1: high wen; 0: high cen. */
#else
	uint64_t mem_en                       : 4;
	uint64_t mem_addr                     : 13;
	uint64_t reserved_17_20               : 4;
	uint64_t track_wr_cnt                 : 6;
	uint64_t track_rd_cnt                 : 6;
	uint64_t reserved_33_36               : 4;
	uint64_t cur_state                    : 3;
	uint64_t rdy                          : 1;
	uint64_t load_val                     : 1;
	uint64_t sel_nxt_ptr                  : 1;
	uint64_t reserved_43_63               : 21;
#endif
	} s;
	struct cvmx_pko_pdm_mwpbuf_dbg_s      cn78xx;
};
typedef union cvmx_pko_pdm_mwpbuf_dbg cvmx_pko_pdm_mwpbuf_dbg_t;

/**
 * cvmx_pko_pdm_sts
 */
union cvmx_pko_pdm_sts {
	uint64_t u64;
	struct cvmx_pko_pdm_sts_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_38_63               : 26;
	uint64_t cp_stalled_thrshld_hit       : 1;  /**< This register is set to 1 if the PDM stalls the inputs for more than
                                                         PKO_PDM_CFG_DBG[CP_STALL_THRSHLD]. INTERNAL: Do not list field in HRM. For lab debug only;
                                                         will likely disapear in pass 2. */
	uint64_t reserved_35_36               : 2;
	uint64_t mwpbuf_data_val_err          : 1;  /**< Received signal that MWPBUF had data valid error. Throws
                                                         PKO_INTSN_E::PKO_MWPBUF_DATA_VAL_ERR. */
	uint64_t drpbuf_data_val_err          : 1;  /**< Received signal that DRPBUF had data valid error. Throws
                                                         PKO_INTSN_E::PKO_DRPBUF_DATA_VAL_ERR. */
	uint64_t dwpbuf_data_val_err          : 1;  /**< Received signal that DWPBUF had data valid error. Throws
                                                         PKO_INTSN_E::PKO_DWPBUF_DATA_VAL_ERR. */
	uint64_t reserved_30_31               : 2;
	uint64_t qcmd_iobx_err_sts            : 4;  /**< When PKO_PDM_STS[QCMD_IOBX_ERR] is set, this contains the queue command response's status
                                                         field for the response causing the error. Note that if multiple errors occur, only the
                                                         first error status is captured here until PKO_PDM_STS[QCMD_IOBX_ERR] is cleared.
                                                         Enumerated by PKO_DQSTATUS_E. */
	uint64_t qcmd_iobx_err                : 1;  /**< Queue command IOBDMA/IOBLD error status occurred in PKO/PDM.
                                                         PKO_PDM_STS[QCMD_IOBX_ERR_STS] contains the status code. Note that FPA being out of
                                                         pointers does not set this bit. (See PKO_FPA_NO_PTRS). Throws
                                                         PKO_INTSN_E::PKO_QCMD_IOBX_ERR. */
	uint64_t sendpkt_lmtdma_err_sts       : 4;  /**< Status field of the command response on the LMTDMA failure indicated by
                                                         PKO_PDM_STS[SENDPKT_LMTDMA_ERR] bits being asserted. Note that if multiple errors occur,
                                                         only the first error status is captured here until PKO_PDM_STS[SENDPKT_LMTDMA_ERR] is
                                                         cleared. Enumerated by PKO_DQSTATUS_E. */
	uint64_t sendpkt_lmtdma_err           : 1;  /**< Send-packet of type LMTDMA error status occurred in PKO/PDM.
                                                         PKO_PDM_STS[SENDPKT_LMTDMA_ERR_STS] contains the status code. Note that FPA being out of
                                                         pointers does not set this bit. (See PKO_FPA_NO_PTRS). Throws
                                                         PKO_INTSN_E::PKO_SENDPKT_LMTDMA_ERR. */
	uint64_t sendpkt_lmtst_err_sts        : 4;  /**< Status field of the command response on the LMTST failure indicated by
                                                         PKO_PDM_STS[SENDPKT_LMTST_ERR] bits being asserted.
                                                         Note that if multiple errors occur only the first error status will be captured here until
                                                         PKO_PDM_STS[SENDPKT_LMTST_ERR] is cleared.
                                                         Enumerated by PKO_DQSTATUS_E. */
	uint64_t sendpkt_lmtst_err            : 1;  /**< Send-packet of type LMTST error status occurred in PKO/PDM.
                                                         PKO_PDM_STS[SENDPKT_LMTST_ERR_STS] contains the status code. Note that FPA being out of
                                                         pointers does not set this bit. (See PKO_FPA_NO_PTRS). Throws
                                                         PKO_INTSN_E::PKO_SENDPKT_LMTST_ERR. */
	uint64_t fpa_no_ptrs                  : 1;  /**< FPA signalled PKO that FPA can not allocate pointers. This is a fatal error.
                                                         Throws PKO_INTSN_E::PKO_FPA_NO_PTRS. */
	uint64_t reserved_12_13               : 2;
	uint64_t cp_sendpkt_err_no_drp_code   : 2;  /**< This field stores the error code for illegally constructed send-packets that did not drop.
                                                         Note that if multiple errors occur, only the first error code is captured here until
                                                         PKO_PDM_STS[CP_SENDPKT_ERR_NO_DRP] is cleared. Codes:
                                                         2'b00: NO ERROR CODE
                                                         2'b01: SEND_JUMP not at end of descriptor. */
	uint64_t cp_sendpkt_err_no_drp        : 1;  /**< PKO/PDM/CP did not drop a send-packet; however, the SEND_JUMP command is not at end of the
                                                         descriptor. The error code is captured in PKO_PDM_STS[CP_SENDPKT_ERR_NO_DRP_CODE]. Throws
                                                         PKO_INTSN_E::PKO_CP_SENDPKT_ERR_NO_DRP. */
	uint64_t reserved_7_8                 : 2;
	uint64_t cp_sendpkt_err_drop_code     : 3;  /**< This field stores the error code for illegally constructed send-packet drops. Note that if
                                                         multiple errors occur, only the first error code is captured here until
                                                         PKO_PDM_STS[CP_SENDPKT_ERR_DROP] is cleared. PKO_CPSENDDROP_E enumerates the codes and
                                                         conditions. */
	uint64_t cp_sendpkt_err_drop          : 1;  /**< Dropped a send-packet in PDM/CP due to a rule violation. The error code is captured in
                                                         PKO_PDM_STS[CP_SENDPKT_ERR_DROP_CODE]. Throws PKO_INTSN_E::PKO_CP_SENDPKT_ERR_DROP. */
	uint64_t reserved_1_2                 : 2;
	uint64_t desc_crc_err                 : 1;  /**< CRC error occurred in a descriptor. (State may have been corrupted.) INTERNAL: Note that
                                                         this is a pass 2 feature. Throws PKO_INTSN_E::PKO_DESC_CRC_ERR. */
#else
	uint64_t desc_crc_err                 : 1;
	uint64_t reserved_1_2                 : 2;
	uint64_t cp_sendpkt_err_drop          : 1;
	uint64_t cp_sendpkt_err_drop_code     : 3;
	uint64_t reserved_7_8                 : 2;
	uint64_t cp_sendpkt_err_no_drp        : 1;
	uint64_t cp_sendpkt_err_no_drp_code   : 2;
	uint64_t reserved_12_13               : 2;
	uint64_t fpa_no_ptrs                  : 1;
	uint64_t sendpkt_lmtst_err            : 1;
	uint64_t sendpkt_lmtst_err_sts        : 4;
	uint64_t sendpkt_lmtdma_err           : 1;
	uint64_t sendpkt_lmtdma_err_sts       : 4;
	uint64_t qcmd_iobx_err                : 1;
	uint64_t qcmd_iobx_err_sts            : 4;
	uint64_t reserved_30_31               : 2;
	uint64_t dwpbuf_data_val_err          : 1;
	uint64_t drpbuf_data_val_err          : 1;
	uint64_t mwpbuf_data_val_err          : 1;
	uint64_t reserved_35_36               : 2;
	uint64_t cp_stalled_thrshld_hit       : 1;
	uint64_t reserved_38_63               : 26;
#endif
	} s;
	struct cvmx_pko_pdm_sts_s             cn78xx;
};
typedef union cvmx_pko_pdm_sts cvmx_pko_pdm_sts_t;

/**
 * cvmx_pko_peb_bist_status
 *
 * Each bit is the BIST result of an individual memory (per bit, 0 = pass and 1 = fail).
 *
 */
union cvmx_pko_peb_bist_status {
	uint64_t u64;
	struct cvmx_pko_peb_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_26_63               : 38;
	uint64_t add_work_fifo                : 1;  /**< ADD_WORK_FIFO RAM BIST status. */
	uint64_t pdm_pse_buf_ram              : 1;  /**< PDM_PSE_BUF RAM BIST status. */
	uint64_t iobp0_fifo_ram               : 1;  /**< IOBP0_FIFO RAM BIST status. */
	uint64_t iobp1_fifo_ram               : 1;  /**< IOBP1_FIFO RAM BIST status. */
	uint64_t state_mem0                   : 1;  /**< STATE_MEM0 RAM BIST status. */
	uint64_t state_mem1                   : 1;  /**< STATE_MEM1 RAM BIST status. */
	uint64_t state_mem2                   : 1;  /**< STATE_MEM2 RAM BIST status. */
	uint64_t state_mem3                   : 1;  /**< STATE_MEM3 RAM BIST status. */
	uint64_t iobp1_uid_fifo_ram           : 1;  /**< IOBP1_UID_FIFO RAM BIST status. */
	uint64_t nxt_link_ptr_ram             : 1;  /**< NXT_LINK_PTR RAM BIST status. */
	uint64_t pd_bank0_ram                 : 1;  /**< PD_BANK0 RAM BIST status. */
	uint64_t pd_bank1_ram                 : 1;  /**< PD_BANK1 RAM BIST status. */
	uint64_t pd_bank2_ram                 : 1;  /**< PD_BANK2 RAM BIST status. */
	uint64_t pd_bank3_ram                 : 1;  /**< PD_BANK3 RAM BIST status. */
	uint64_t pd_var_bank_ram              : 1;  /**< PD_VAR_BANK RAM BIST status. */
	uint64_t pdm_resp_buf_ram             : 1;  /**< PDM_RESP_BUF RAM BIST status. */
	uint64_t tx_fifo_pkt_ram              : 1;  /**< TX_FIFO_PKT RAM BIST status. */
	uint64_t tx_fifo_hdr_ram              : 1;  /**< TX_FIFO_HDR RAM BIST status. */
	uint64_t tx_fifo_crc_ram              : 1;  /**< TX_FIFO_CRC RAM BIST status. */
	uint64_t ts_addwork_ram               : 1;  /**< TS_ADDWORK RAM BIST status. */
	uint64_t send_mem_ts_fifo             : 1;  /**< SEND_MEM_TS_FIFO RAM BIST status. */
	uint64_t send_mem_stdn_fifo           : 1;  /**< SEND_MEM_STDN_FIFO RAM BIST status. */
	uint64_t send_mem_fifo                : 1;  /**< SEND_MEM_FIFO RAM BIST status. */
	uint64_t pkt_mrk_ram                  : 1;  /**< PKT_MRK RAM BIST status. */
	uint64_t peb_st_inf_ram               : 1;  /**< PEB_ST_INF RAM BIST status. */
	uint64_t peb_sm_jmp_ram               : 1;  /**< PEB_SM_JMP RAM BIST status. */
#else
	uint64_t peb_sm_jmp_ram               : 1;
	uint64_t peb_st_inf_ram               : 1;
	uint64_t pkt_mrk_ram                  : 1;
	uint64_t send_mem_fifo                : 1;
	uint64_t send_mem_stdn_fifo           : 1;
	uint64_t send_mem_ts_fifo             : 1;
	uint64_t ts_addwork_ram               : 1;
	uint64_t tx_fifo_crc_ram              : 1;
	uint64_t tx_fifo_hdr_ram              : 1;
	uint64_t tx_fifo_pkt_ram              : 1;
	uint64_t pdm_resp_buf_ram             : 1;
	uint64_t pd_var_bank_ram              : 1;
	uint64_t pd_bank3_ram                 : 1;
	uint64_t pd_bank2_ram                 : 1;
	uint64_t pd_bank1_ram                 : 1;
	uint64_t pd_bank0_ram                 : 1;
	uint64_t nxt_link_ptr_ram             : 1;
	uint64_t iobp1_uid_fifo_ram           : 1;
	uint64_t state_mem3                   : 1;
	uint64_t state_mem2                   : 1;
	uint64_t state_mem1                   : 1;
	uint64_t state_mem0                   : 1;
	uint64_t iobp1_fifo_ram               : 1;
	uint64_t iobp0_fifo_ram               : 1;
	uint64_t pdm_pse_buf_ram              : 1;
	uint64_t add_work_fifo                : 1;
	uint64_t reserved_26_63               : 38;
#endif
	} s;
	struct cvmx_pko_peb_bist_status_s     cn78xx;
};
typedef union cvmx_pko_peb_bist_status cvmx_pko_peb_bist_status_t;

/**
 * cvmx_pko_peb_ecc_ctl0
 */
union cvmx_pko_peb_ecc_ctl0 {
	uint64_t u64;
	struct cvmx_pko_peb_ecc_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t iobp1_uid_fifo_ram_flip      : 2;  /**< IOBP1_UID_FIFO_RAM flip syndrome bits on write. */
	uint64_t iobp1_uid_fifo_ram_cdis      : 1;  /**< IOBP1_UID_FIFO_RAM ECC correction disable. */
	uint64_t iobp0_fifo_ram_flip          : 2;  /**< IOBP0_FIFO_RAM flip syndrome bits on write. */
	uint64_t iobp0_fifo_ram_cdis          : 1;  /**< IOBP0_FIFO_RAM ECC correction disable. */
	uint64_t iobp1_fifo_ram_flip          : 2;  /**< IOBP1_FIFO_RAM flip syndrome bits on write. */
	uint64_t iobp1_fifo_ram_cdis          : 1;  /**< IOBP1_FIFO_RAM ECC correction disable. */
	uint64_t pdm_resp_buf_ram_flip        : 2;  /**< PDM_RESP_BUF_RAM flip syndrome bits on write. */
	uint64_t pdm_resp_buf_ram_cdis        : 1;  /**< PDM_RESP_BUF_RAM ECC correction disable. */
	uint64_t pdm_pse_buf_ram_flip         : 2;  /**< PDM_PSE_BUF_RAM flip syndrome bits on write. */
	uint64_t pdm_pse_buf_ram_cdis         : 1;  /**< PDM_PSE_BUF_RAM ECC correction disable. */
	uint64_t peb_sm_jmp_ram_flip          : 2;  /**< PEB_SM_JMP_RAM flip syndrome bits on write. */
	uint64_t peb_sm_jmp_ram_cdis          : 1;  /**< PEB_SM_JMP_RAM ECC correction disable. */
	uint64_t peb_st_inf_ram_flip          : 2;  /**< PEB_ST_INF_RAM flip syndrome bits on write. */
	uint64_t peb_st_inf_ram_cdis          : 1;  /**< PEB_ST_INF_RAM ECC correction disable. */
	uint64_t pd_bank3_ram_flip            : 2;  /**< PD_BANK3_RAM flip syndrome bits on write. */
	uint64_t pd_bank3_ram_cdis            : 1;  /**< PD_BANK3_RAM ECC correction disable. */
	uint64_t pd_bank2_ram_flip            : 2;  /**< PD_BANK2_RAM flip syndrome bits on write. */
	uint64_t pd_bank2_ram_cdis            : 1;  /**< PD_BANK2_RAM ECC correction disable. */
	uint64_t pd_bank1_ram_flip            : 2;  /**< PD_BANK1_RAM flip syndrome bits on write. */
	uint64_t pd_bank1_ram_cdis            : 1;  /**< PD_BANK1_RAM ECC correction disable. */
	uint64_t pd_bank0_ram_flip            : 2;  /**< PD_BANK0_RAM flip syndrome bits on write. */
	uint64_t pd_bank0_ram_cdis            : 1;  /**< PD_BANK0_RAM ECC correction disable. */
	uint64_t pd_var_bank_ram_flip         : 2;  /**< PD_VAR_BANK_RAM flip syndrome bits on write. */
	uint64_t pd_var_bank_ram_cdis         : 1;  /**< PD_VAR_BANK_RAM ECC correction disable. */
	uint64_t tx_fifo_crc_ram_flip         : 2;  /**< TX_FIFO_CRC_RAM flip syndrome bits on write. */
	uint64_t tx_fifo_crc_ram_cdis         : 1;  /**< TX_FIFO_CRC_RAM ECC correction disable. */
	uint64_t tx_fifo_hdr_ram_flip         : 2;  /**< TX_FIFO_HDR_RAM flip syndrome bits on write. */
	uint64_t tx_fifo_hdr_ram_cdis         : 1;  /**< TX_FIFO_HDR_RAM ECC correction disable. */
	uint64_t tx_fifo_pkt_ram_flip         : 2;  /**< TX_FIFO_PKT_RAM flip syndrome bits on write. */
	uint64_t tx_fifo_pkt_ram_cdis         : 1;  /**< TX_FIFO_PKT_RAM ECC correction disable. */
	uint64_t add_work_fifo_flip           : 2;  /**< ADD_WORK_FIFO flip syndrome bits on write. */
	uint64_t add_work_fifo_cdis           : 1;  /**< ADD_WORK_FIFO ECC correction disable. */
	uint64_t send_mem_fifo_flip           : 2;  /**< SEND_MEM_FIFO flip syndrome bits on write. */
	uint64_t send_mem_fifo_cdis           : 1;  /**< SEND_MEM_FIFO ECC correction disable. */
	uint64_t send_mem_stdn_fifo_flip      : 2;  /**< SEND_MEM_STDN_FIFO flip syndrome bits on write. */
	uint64_t send_mem_stdn_fifo_cdis      : 1;  /**< SEND_MEM_STDN_FIFO ECC correction disable. */
	uint64_t send_mem_ts_fifo_flip        : 2;  /**< SEND_MEM_TS_FIFO flip syndrome bits on write. */
	uint64_t send_mem_ts_fifo_cdis        : 1;  /**< SEND_MEM_TS_FIFO ECC correction disable. */
	uint64_t nxt_link_ptr_ram_flip        : 2;  /**< NXT_LINK_PTR_RAM flip syndrome bits on write. */
	uint64_t nxt_link_ptr_ram_cdis        : 1;  /**< NXT_LINK_PTR_RAM ECC correction disable. */
	uint64_t pkt_mrk_ram_flip             : 2;  /**< PKT_MRK_RAM flip syndrome bits on write. */
	uint64_t pkt_mrk_ram_cdis             : 1;  /**< PKT_MRK_RAM ECC correction disable. */
	uint64_t reserved_0_0                 : 1;
#else
	uint64_t reserved_0_0                 : 1;
	uint64_t pkt_mrk_ram_cdis             : 1;
	uint64_t pkt_mrk_ram_flip             : 2;
	uint64_t nxt_link_ptr_ram_cdis        : 1;
	uint64_t nxt_link_ptr_ram_flip        : 2;
	uint64_t send_mem_ts_fifo_cdis        : 1;
	uint64_t send_mem_ts_fifo_flip        : 2;
	uint64_t send_mem_stdn_fifo_cdis      : 1;
	uint64_t send_mem_stdn_fifo_flip      : 2;
	uint64_t send_mem_fifo_cdis           : 1;
	uint64_t send_mem_fifo_flip           : 2;
	uint64_t add_work_fifo_cdis           : 1;
	uint64_t add_work_fifo_flip           : 2;
	uint64_t tx_fifo_pkt_ram_cdis         : 1;
	uint64_t tx_fifo_pkt_ram_flip         : 2;
	uint64_t tx_fifo_hdr_ram_cdis         : 1;
	uint64_t tx_fifo_hdr_ram_flip         : 2;
	uint64_t tx_fifo_crc_ram_cdis         : 1;
	uint64_t tx_fifo_crc_ram_flip         : 2;
	uint64_t pd_var_bank_ram_cdis         : 1;
	uint64_t pd_var_bank_ram_flip         : 2;
	uint64_t pd_bank0_ram_cdis            : 1;
	uint64_t pd_bank0_ram_flip            : 2;
	uint64_t pd_bank1_ram_cdis            : 1;
	uint64_t pd_bank1_ram_flip            : 2;
	uint64_t pd_bank2_ram_cdis            : 1;
	uint64_t pd_bank2_ram_flip            : 2;
	uint64_t pd_bank3_ram_cdis            : 1;
	uint64_t pd_bank3_ram_flip            : 2;
	uint64_t peb_st_inf_ram_cdis          : 1;
	uint64_t peb_st_inf_ram_flip          : 2;
	uint64_t peb_sm_jmp_ram_cdis          : 1;
	uint64_t peb_sm_jmp_ram_flip          : 2;
	uint64_t pdm_pse_buf_ram_cdis         : 1;
	uint64_t pdm_pse_buf_ram_flip         : 2;
	uint64_t pdm_resp_buf_ram_cdis        : 1;
	uint64_t pdm_resp_buf_ram_flip        : 2;
	uint64_t iobp1_fifo_ram_cdis          : 1;
	uint64_t iobp1_fifo_ram_flip          : 2;
	uint64_t iobp0_fifo_ram_cdis          : 1;
	uint64_t iobp0_fifo_ram_flip          : 2;
	uint64_t iobp1_uid_fifo_ram_cdis      : 1;
	uint64_t iobp1_uid_fifo_ram_flip      : 2;
#endif
	} s;
	struct cvmx_pko_peb_ecc_ctl0_s        cn78xx;
};
typedef union cvmx_pko_peb_ecc_ctl0 cvmx_pko_peb_ecc_ctl0_t;

/**
 * cvmx_pko_peb_ecc_ctl1
 */
union cvmx_pko_peb_ecc_ctl1 {
	uint64_t u64;
	struct cvmx_pko_peb_ecc_ctl1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ts_addwork_ram_flip          : 2;  /**< TS_ADDWORK_RAM flip syndrome bits on write. */
	uint64_t ts_addwork_ram_cdis          : 1;  /**< TS_ADDWORK_RAM ECC correction disable. */
	uint64_t reserved_0_60                : 61;
#else
	uint64_t reserved_0_60                : 61;
	uint64_t ts_addwork_ram_cdis          : 1;
	uint64_t ts_addwork_ram_flip          : 2;
#endif
	} s;
	struct cvmx_pko_peb_ecc_ctl1_s        cn78xx;
};
typedef union cvmx_pko_peb_ecc_ctl1 cvmx_pko_peb_ecc_ctl1_t;

/**
 * cvmx_pko_peb_ecc_dbe_sts0
 */
union cvmx_pko_peb_ecc_dbe_sts0 {
	uint64_t u64;
	struct cvmx_pko_peb_ecc_dbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t iobp1_uid_fifo_ram_dbe       : 1;  /**< Double-bit error for IOBP1_UID_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_iobp1_uid_fifo_i */
	uint64_t iobp0_fifo_ram_dbe           : 1;  /**< Double-bit error for IOBP0_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_iobp0_fifo_i */
	uint64_t iobp1_fifo_ram_dbe           : 1;  /**< Double-bit error for IOBP1_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_iobp1_fifo_i */
	uint64_t pdm_resp_buf_ram_dbe         : 1;  /**< Double-bit error for PDM_RESP_BUF_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_pdm_intf_i.pko_peb_pdm_resp_buf_i */
	uint64_t pdm_pse_buf_ram_dbe          : 1;  /**< Double-bit error for PDM_PSE_BUF_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_pdm_intf_i.pko_peb_pse_buf_i */
	uint64_t peb_sm_jmp_ram_dbe           : 1;  /**< Double-bit error for PEB_SM_JMP_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_sm_jmp_uid_mem_i */
	uint64_t peb_st_inf_ram_dbe           : 1;  /**< Double-bit error for PEB_ST_INF_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_state_info_mem_i */
	uint64_t pd_bank3_ram_dbe             : 1;  /**< Double-bit error for PD_BANK3_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_pd_mem_bank3_i */
	uint64_t pd_bank2_ram_dbe             : 1;  /**< Double-bit error for PD_BANK2_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_pd_mem_bank2_i */
	uint64_t pd_bank1_ram_dbe             : 1;  /**< Double-bit error for PD_BANK1_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_pd_mem_bank1_i */
	uint64_t pd_bank0_ram_dbe             : 1;  /**< Double-bit error for PD_BANK0_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_pd_mem_bank0_i */
	uint64_t pd_var_bank_ram_dbe          : 1;  /**< Double-bit error for PD_VAR_BANK_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_pd_var_mem_bank_i */
	uint64_t tx_fifo_crc_ram_dbe          : 1;  /**< Double-bit error for TX_FIFO_CRC_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_tx_fifo_i.pko_peb_tx_fifo_crc_i */
	uint64_t tx_fifo_hdr_ram_dbe          : 1;  /**< Double-bit error for TX_FIFO_HDR_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_tx_fifo_i.pko_peb_tx_fifo_hdr_i */
	uint64_t tx_fifo_pkt_ram_dbe          : 1;  /**< Double-bit error for TX_FIFO_PKT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_tx_fifo_i.pko_peb_tx_fifo_pkt_i */
	uint64_t add_work_fifo_dbe            : 1;  /**< Double-bit error for ADD_WORK_FIFO.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_addwork_proc_i.pko_peb_add_work_fifo_i */
	uint64_t send_mem_fifo_dbe            : 1;  /**< Double-bit error for SEND_MEM_FIFO.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_sendmem_proc_i.pko_peb_send_mem_fifo_i */
	uint64_t send_mem_stdn_fifo_dbe       : 1;  /**< Double-bit error for SEND_MEM_STDN_FIFO.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_sendmem_proc_i.pko_peb_send_mem_stdn_fifo_i */
	uint64_t send_mem_ts_fifo_dbe         : 1;  /**< Double-bit error for SEND_MEM_TS_FIFO.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_sendmem_proc_i.pko_peb_send_mem_ts_fifo_i */
	uint64_t nxt_link_ptr_ram_dbe         : 1;  /**< Double-bit error for NXT_LINK_PTR_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_nxt_link_ptr_mem_i */
	uint64_t pkt_mrk_ram_dbe              : 1;  /**< Double-bit error for PKT_MRK_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_pkt_mrk_mem_i */
	uint64_t ts_addwork_ram_dbe           : 1;  /**< Double-bit error for TS_ADDWORK_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_addwork_proc_i.pko_peb_ts_addwork_mem_i */
	uint64_t reserved_0_41                : 42;
#else
	uint64_t reserved_0_41                : 42;
	uint64_t ts_addwork_ram_dbe           : 1;
	uint64_t pkt_mrk_ram_dbe              : 1;
	uint64_t nxt_link_ptr_ram_dbe         : 1;
	uint64_t send_mem_ts_fifo_dbe         : 1;
	uint64_t send_mem_stdn_fifo_dbe       : 1;
	uint64_t send_mem_fifo_dbe            : 1;
	uint64_t add_work_fifo_dbe            : 1;
	uint64_t tx_fifo_pkt_ram_dbe          : 1;
	uint64_t tx_fifo_hdr_ram_dbe          : 1;
	uint64_t tx_fifo_crc_ram_dbe          : 1;
	uint64_t pd_var_bank_ram_dbe          : 1;
	uint64_t pd_bank0_ram_dbe             : 1;
	uint64_t pd_bank1_ram_dbe             : 1;
	uint64_t pd_bank2_ram_dbe             : 1;
	uint64_t pd_bank3_ram_dbe             : 1;
	uint64_t peb_st_inf_ram_dbe           : 1;
	uint64_t peb_sm_jmp_ram_dbe           : 1;
	uint64_t pdm_pse_buf_ram_dbe          : 1;
	uint64_t pdm_resp_buf_ram_dbe         : 1;
	uint64_t iobp1_fifo_ram_dbe           : 1;
	uint64_t iobp0_fifo_ram_dbe           : 1;
	uint64_t iobp1_uid_fifo_ram_dbe       : 1;
#endif
	} s;
	struct cvmx_pko_peb_ecc_dbe_sts0_s    cn78xx;
};
typedef union cvmx_pko_peb_ecc_dbe_sts0 cvmx_pko_peb_ecc_dbe_sts0_t;

/**
 * cvmx_pko_peb_ecc_dbe_sts_cmb0
 */
union cvmx_pko_peb_ecc_dbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pko_peb_ecc_dbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t peb_dbe_cmb0                 : 1;  /**< This bit is the logical OR of all bits in PKO_PEB_ECC_DBE_STS.
                                                         To clear this bit, software must clear bits in PKO_PEB_ECC_DBE_STS.
                                                         When this bit is set, the corresponding interrupt is set.
                                                         Throws PKO_INTSN_E::PKO_PEB_DBE_CMB0.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_iobp1_uid_fifo_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_iobp0_fifo_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_iobp1_fifo_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_pdm_intf_i.pko_peb_pdm_resp_buf_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_pdm_intf_i.pko_peb_pse_buf_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_sm_jmp_uid_mem_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_state_info_mem_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_pd_mem_bank3_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_pd_mem_bank0_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_pd_mem_bank1_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_pd_mem_bank2_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_pd_var_mem_bank_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_tx_fifo_i.pko_peb_tx_fifo_crc_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_tx_fifo_i.pko_peb_tx_fifo_hdr_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_tx_fifo_i.pko_peb_tx_fifo_pkt_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_addwork_proc_i.pko_peb_add_work_fifo_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_sendmem_proc_i.pko_peb_send_mem_fifo_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_sendmem_proc_i.pko_peb_send_mem_stdn_fifo_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_sendmem_proc_i.pko_peb_send_mem_ts_fifo_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_nxt_link_ptr_mem_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_pkt_mrk_mem_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_addwork_proc_i.pko_peb_ts_addwork_mem_i */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t peb_dbe_cmb0                 : 1;
#endif
	} s;
	struct cvmx_pko_peb_ecc_dbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pko_peb_ecc_dbe_sts_cmb0 cvmx_pko_peb_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pko_peb_ecc_sbe_sts0
 */
union cvmx_pko_peb_ecc_sbe_sts0 {
	uint64_t u64;
	struct cvmx_pko_peb_ecc_sbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t iobp1_uid_fifo_ram_sbe       : 1;  /**< Single-bit error for IOBP1_UID_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_iobp1_uid_fifo_i */
	uint64_t iobp0_fifo_ram_sbe           : 1;  /**< Single-bit error for IOBP0_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_iobp0_fifo_i */
	uint64_t iobp1_fifo_ram_sbe           : 1;  /**< Single-bit error for IOBP1_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_iobp1_fifo_i */
	uint64_t pdm_resp_buf_ram_sbe         : 1;  /**< Single-bit error for PDM_RESP_BUF_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_pdm_intf_i.pko_peb_pdm_resp_buf_i */
	uint64_t pdm_pse_buf_ram_sbe          : 1;  /**< Single-bit error for PDM_PSE_BUF_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_pdm_intf_i.pko_peb_pse_buf_i */
	uint64_t peb_sm_jmp_ram_sbe           : 1;  /**< Single-bit error for PEB_SM_JMP_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_sm_jmp_uid_mem_i */
	uint64_t peb_st_inf_ram_sbe           : 1;  /**< Single-bit error for PEB_ST_INF_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_state_info_mem_i */
	uint64_t pd_bank3_ram_sbe             : 1;  /**< Single-bit error for PD_BANK3_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_pd_mem_bank3_i */
	uint64_t pd_bank2_ram_sbe             : 1;  /**< Single-bit error for PD_BANK2_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_pd_mem_bank2_i */
	uint64_t pd_bank1_ram_sbe             : 1;  /**< Single-bit error for PD_BANK1_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_pd_mem_bank1_i */
	uint64_t pd_bank0_ram_sbe             : 1;  /**< Single-bit error for PD_BANK1_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_pd_mem_bank0_i */
	uint64_t pd_var_bank_ram_sbe          : 1;  /**< Single-bit error for PD_VAR_BANK_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_pd_var_mem_bank_i */
	uint64_t tx_fifo_crc_ram_sbe          : 1;  /**< Single-bit error for TX_FIFO_CRC_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_tx_fifo_i.pko_peb_tx_fifo_crc_i */
	uint64_t tx_fifo_hdr_ram_sbe          : 1;  /**< Single-bit error for TX_FIFO_HDR_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_tx_fifo_i.pko_peb_tx_fifo_hdr_i */
	uint64_t tx_fifo_pkt_ram_sbe          : 1;  /**< Single-bit error for TX_FIFO_PKT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_tx_fifo_i.pko_peb_tx_fifo_pkt_i */
	uint64_t add_work_fifo_sbe            : 1;  /**< Single-bit error for ADD_WORK_FIFO.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_addwork_proc_i.pko_peb_add_work_fifo_i */
	uint64_t send_mem_fifo_sbe            : 1;  /**< Single-bit error for SEND_MEM_FIFO.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_sendmem_proc_i.pko_peb_send_mem_fifo_i */
	uint64_t send_mem_stdn_fifo_sbe       : 1;  /**< Single-bit error for SEND_MEM_STDN_FIFO.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_sendmem_proc_i.pko_peb_send_mem_stdn_fifo_i */
	uint64_t send_mem_ts_fifo_sbe         : 1;  /**< Single-bit error for SEND_MEM_TS_FIFO.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_sendmem_proc_i.pko_peb_send_mem_ts_fifo_i */
	uint64_t nxt_link_ptr_ram_sbe         : 1;  /**< Single-bit error for NXT_LINK_PTR_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_nxt_link_ptr_mem_i */
	uint64_t pkt_mrk_ram_sbe              : 1;  /**< Single-bit error for PKT_MRK_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_pkt_mrk_mem_i */
	uint64_t ts_addwork_ram_sbe           : 1;  /**< Single-bit error for TS_ADDWORK_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_addwork_proc_i.pko_peb_ts_addwork_mem_i */
	uint64_t reserved_0_41                : 42;
#else
	uint64_t reserved_0_41                : 42;
	uint64_t ts_addwork_ram_sbe           : 1;
	uint64_t pkt_mrk_ram_sbe              : 1;
	uint64_t nxt_link_ptr_ram_sbe         : 1;
	uint64_t send_mem_ts_fifo_sbe         : 1;
	uint64_t send_mem_stdn_fifo_sbe       : 1;
	uint64_t send_mem_fifo_sbe            : 1;
	uint64_t add_work_fifo_sbe            : 1;
	uint64_t tx_fifo_pkt_ram_sbe          : 1;
	uint64_t tx_fifo_hdr_ram_sbe          : 1;
	uint64_t tx_fifo_crc_ram_sbe          : 1;
	uint64_t pd_var_bank_ram_sbe          : 1;
	uint64_t pd_bank0_ram_sbe             : 1;
	uint64_t pd_bank1_ram_sbe             : 1;
	uint64_t pd_bank2_ram_sbe             : 1;
	uint64_t pd_bank3_ram_sbe             : 1;
	uint64_t peb_st_inf_ram_sbe           : 1;
	uint64_t peb_sm_jmp_ram_sbe           : 1;
	uint64_t pdm_pse_buf_ram_sbe          : 1;
	uint64_t pdm_resp_buf_ram_sbe         : 1;
	uint64_t iobp1_fifo_ram_sbe           : 1;
	uint64_t iobp0_fifo_ram_sbe           : 1;
	uint64_t iobp1_uid_fifo_ram_sbe       : 1;
#endif
	} s;
	struct cvmx_pko_peb_ecc_sbe_sts0_s    cn78xx;
};
typedef union cvmx_pko_peb_ecc_sbe_sts0 cvmx_pko_peb_ecc_sbe_sts0_t;

/**
 * cvmx_pko_peb_ecc_sbe_sts_cmb0
 */
union cvmx_pko_peb_ecc_sbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pko_peb_ecc_sbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t peb_sbe_cmb0                 : 1;  /**< This bit is the logical OR of all bits in PKO_PEB_ECC_SBE_STS.
                                                         To clear this bit, software must clear bits in PKO_PEB_ECC_SBE_STS.
                                                         When this bit is set, the corresponding interrupt is set.
                                                         Throws PKO_INTSN_E::PKO_PEB_SBE_CMB0.
                                                         INTERNAL: Instances:
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_iobp1_uid_fifo_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_iobp0_fifo_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_iobp1_fifo_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_pdm_intf_i.pko_peb_pdm_resp_buf_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_pdm_intf_i.pko_peb_pse_buf_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_sm_jmp_uid_mem_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_state_info_mem_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_pd_mem_bank3_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_pd_mem_bank0_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_pd_mem_bank1_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_pd_mem_bank2_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_pd_var_mem_bank_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_tx_fifo_i.pko_peb_tx_fifo_crc_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_tx_fifo_i.pko_peb_tx_fifo_hdr_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_tx_fifo_i.pko_peb_tx_fifo_pkt_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_addwork_proc_i.pko_peb_add_work_fifo_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_sendmem_proc_i.pko_peb_send_mem_fifo_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_sendmem_proc_i.pko_peb_send_mem_stdn_fifo_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_sendmem_proc_i.pko_peb_send_mem_ts_fifo_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_nxt_link_ptr_mem_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_state_mem_i.pko_peb_pkt_mrk_mem_i
                                                         pko_pnr3.pko_pnr3_peb.pko_peb_proc_i.pko_peb_addwork_proc_i.pko_peb_ts_addwork_mem_i */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t peb_sbe_cmb0                 : 1;
#endif
	} s;
	struct cvmx_pko_peb_ecc_sbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pko_peb_ecc_sbe_sts_cmb0 cvmx_pko_peb_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pko_peb_err_int
 */
union cvmx_pko_peb_err_int {
	uint64_t u64;
	struct cvmx_pko_peb_err_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t peb_macx_cfg_wr_err          : 1;  /**< Asserted when software writes a FIFO number to PKO_MACx_CFG when that FIFO is already
                                                         assigned. Throws PKO_INTSN_E::PEB_MACX_CFG_WR_ERR. */
	uint64_t peb_max_link_err             : 1;  /**< Asserted when 200 LINK segments have been followed.  Indicates likelihood of infinite
                                                         loop.  Throws PKO_INTSN_E::PEB_MAX_LINK_ERR. */
	uint64_t peb_subd_size_err            : 1;  /**< Asserted when a SEND_LINK/GATHER/IMM/JUMP subD has size=0.  Throws
                                                         PKO_INTSN_E::PEB_SUBD_SIZE_ERR. */
	uint64_t peb_subd_addr_err            : 1;  /**< Asserted when the address of a FREE/MEM/LINK/LINK segment/JUMP/GATHER subD is 0x0.  Throws
                                                         PKO_INTSN_E::PEB_SUBD_ADDR_ERR. */
	uint64_t peb_trunc_err                : 1;  /**< Asserted when a PD has truncated data.  Throws PKO_INTSN_E::PEB_TRUNC_ERR. */
	uint64_t peb_pad_err                  : 1;  /**< Asserted when a PD has data padded to it (SEND_HDR[TOTAL] < sum(SEND_DATA[size])).  Throws
                                                         PKO_INTSN_E::PEB_PAD_ERR. */
	uint64_t peb_pse_fifo_err             : 1;  /**< Asserted when PSE sends PD information for a nonconfigured FIFO. Throws
                                                         PKO_INTSN_E::PEB_PSE_FIFO_ERR. */
	uint64_t peb_fcs_sop_err              : 1;  /**< Asserted when FCS SOP value greater than packet size detected.  Throws
                                                         PKO_INTSN_E::PEB_FCS_SOP_ERR. */
	uint64_t peb_jump_def_err             : 1;  /**< Asserted when JUMP subdescriptor is not last in a PD. Throws
                                                         PKO_INTSN_E::PEB_JUMP_DEF_ERR. */
	uint64_t peb_ext_hdr_def_err          : 1;  /**< Asserted when EXT_HDR is not the second sub-descriptor in a PD.  Throws
                                                         PKO_INTSN_E::PEB_EXT_HDR_DEF_ERR. */
#else
	uint64_t peb_ext_hdr_def_err          : 1;
	uint64_t peb_jump_def_err             : 1;
	uint64_t peb_fcs_sop_err              : 1;
	uint64_t peb_pse_fifo_err             : 1;
	uint64_t peb_pad_err                  : 1;
	uint64_t peb_trunc_err                : 1;
	uint64_t peb_subd_addr_err            : 1;
	uint64_t peb_subd_size_err            : 1;
	uint64_t peb_max_link_err             : 1;
	uint64_t peb_macx_cfg_wr_err          : 1;
	uint64_t reserved_10_63               : 54;
#endif
	} s;
	struct cvmx_pko_peb_err_int_s         cn78xx;
};
typedef union cvmx_pko_peb_err_int cvmx_pko_peb_err_int_t;

/**
 * cvmx_pko_peb_ext_hdr_def_err_info
 */
union cvmx_pko_peb_ext_hdr_def_err_info {
	uint64_t u64;
	struct cvmx_pko_peb_ext_hdr_def_err_info_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t val                          : 1;  /**< Asserted when PKO_PEB_ERR_INT[PEB_EXT_HDR_DEF_ERR] is set. */
	uint64_t fifo                         : 7;  /**< FIFO number associated with the captured PEB_EXT_HDR_DEF_ERR. */
	uint64_t chan                         : 12; /**< Channel number associated with the captured PEB_EXT_HDR_DEF_ERR. */
#else
	uint64_t chan                         : 12;
	uint64_t fifo                         : 7;
	uint64_t val                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_pko_peb_ext_hdr_def_err_info_s cn78xx;
};
typedef union cvmx_pko_peb_ext_hdr_def_err_info cvmx_pko_peb_ext_hdr_def_err_info_t;

/**
 * cvmx_pko_peb_fcs_sop_err_info
 */
union cvmx_pko_peb_fcs_sop_err_info {
	uint64_t u64;
	struct cvmx_pko_peb_fcs_sop_err_info_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t val                          : 1;  /**< Asserted when PKO_PEB_ERR_INT[PEB_FCS_SOP_ERR] is set. */
	uint64_t fifo                         : 7;  /**< FIFO number associated with the captured PEB_FCS_SOP_ERR. */
	uint64_t chan                         : 12; /**< Channel number associated with the captured PEB_FCS_SOP_ERR. */
#else
	uint64_t chan                         : 12;
	uint64_t fifo                         : 7;
	uint64_t val                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_pko_peb_fcs_sop_err_info_s cn78xx;
};
typedef union cvmx_pko_peb_fcs_sop_err_info cvmx_pko_peb_fcs_sop_err_info_t;

/**
 * cvmx_pko_peb_jump_def_err_info
 */
union cvmx_pko_peb_jump_def_err_info {
	uint64_t u64;
	struct cvmx_pko_peb_jump_def_err_info_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t val                          : 1;  /**< Asserted when PKO_PEB_ERR_INT[PEB_JUMP_DEF_ERR] is set. */
	uint64_t fifo                         : 7;  /**< FIFO number associated with the captured PEB_JUMP_DEF_ERR. */
	uint64_t chan                         : 12; /**< Channel number associated with the captured PEB_JUMP_DEF_ERR. */
#else
	uint64_t chan                         : 12;
	uint64_t fifo                         : 7;
	uint64_t val                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_pko_peb_jump_def_err_info_s cn78xx;
};
typedef union cvmx_pko_peb_jump_def_err_info cvmx_pko_peb_jump_def_err_info_t;

/**
 * cvmx_pko_peb_macx_cfg_wr_err_info
 */
union cvmx_pko_peb_macx_cfg_wr_err_info {
	uint64_t u64;
	struct cvmx_pko_peb_macx_cfg_wr_err_info_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t val                          : 1;  /**< Asserted when PKO_PEB_ERR_INT[PEB_MACX_CFG_WR_ERR] is set. */
	uint64_t mac                          : 7;  /**< MAC number associated with the captured PEB_MACX_CFG_WR_ERR. */
#else
	uint64_t mac                          : 7;
	uint64_t val                          : 1;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_pko_peb_macx_cfg_wr_err_info_s cn78xx;
};
typedef union cvmx_pko_peb_macx_cfg_wr_err_info cvmx_pko_peb_macx_cfg_wr_err_info_t;

/**
 * cvmx_pko_peb_max_link_err_info
 */
union cvmx_pko_peb_max_link_err_info {
	uint64_t u64;
	struct cvmx_pko_peb_max_link_err_info_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t val                          : 1;  /**< Asserted when PKO_PEB_ERR_INT[PEB_MAX_LINK_ERR] is set. */
	uint64_t fifo                         : 7;  /**< FIFO number associated with the captured PEB_MAX_LINK_ERR. */
	uint64_t chan                         : 12; /**< Channel number associated with the captured PEB_MAX_LINK_ERR. */
#else
	uint64_t chan                         : 12;
	uint64_t fifo                         : 7;
	uint64_t val                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_pko_peb_max_link_err_info_s cn78xx;
};
typedef union cvmx_pko_peb_max_link_err_info cvmx_pko_peb_max_link_err_info_t;

/**
 * cvmx_pko_peb_pad_err_info
 */
union cvmx_pko_peb_pad_err_info {
	uint64_t u64;
	struct cvmx_pko_peb_pad_err_info_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t val                          : 1;  /**< Asserted when PKO_PEB_ERR_INT[PEB_PAD_ERR] is set. */
	uint64_t fifo                         : 7;  /**< FIFO number associated with the captured PEB_PAD_ERR. */
	uint64_t chan                         : 12; /**< Channel number associated with the captured PEB_PAD_ERR. */
#else
	uint64_t chan                         : 12;
	uint64_t fifo                         : 7;
	uint64_t val                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_pko_peb_pad_err_info_s    cn78xx;
};
typedef union cvmx_pko_peb_pad_err_info cvmx_pko_peb_pad_err_info_t;

/**
 * cvmx_pko_peb_pse_fifo_err_info
 */
union cvmx_pko_peb_pse_fifo_err_info {
	uint64_t u64;
	struct cvmx_pko_peb_pse_fifo_err_info_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t val                          : 1;  /**< Asserted when PKO_PEB_ERR_INT[PEB_PSE_FIFO_ERR] is set. */
	uint64_t fifo                         : 7;  /**< FIFO number associated with the captured PEB_PSE_FIFO_ERR. */
	uint64_t chan                         : 12; /**< Channel number associated with the captured PEB_PSE_FIFO_ERR. */
#else
	uint64_t chan                         : 12;
	uint64_t fifo                         : 7;
	uint64_t val                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_pko_peb_pse_fifo_err_info_s cn78xx;
};
typedef union cvmx_pko_peb_pse_fifo_err_info cvmx_pko_peb_pse_fifo_err_info_t;

/**
 * cvmx_pko_peb_subd_addr_err_info
 */
union cvmx_pko_peb_subd_addr_err_info {
	uint64_t u64;
	struct cvmx_pko_peb_subd_addr_err_info_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t val                          : 1;  /**< Asserted when PKO_PEB_ERR_INT[PEB_SUBD_ADDR_ERR] is set. */
	uint64_t fifo                         : 7;  /**< FIFO number associated with the captured PEB_SUBD_ADDR_ERR. */
	uint64_t chan                         : 12; /**< Channel number associated with the captured PEB_SUBD_ADDR_ERR. */
#else
	uint64_t chan                         : 12;
	uint64_t fifo                         : 7;
	uint64_t val                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_pko_peb_subd_addr_err_info_s cn78xx;
};
typedef union cvmx_pko_peb_subd_addr_err_info cvmx_pko_peb_subd_addr_err_info_t;

/**
 * cvmx_pko_peb_subd_size_err_info
 */
union cvmx_pko_peb_subd_size_err_info {
	uint64_t u64;
	struct cvmx_pko_peb_subd_size_err_info_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t val                          : 1;  /**< Asserted when PKO_PEB_ERR_INT[PEB_SUBD_SIZE_ERR] is set. */
	uint64_t fifo                         : 7;  /**< FIFO number associated with the captured PEB_SUBD_SIZE_ERR. */
	uint64_t chan                         : 12; /**< Channel number associated with the captured PEB_SUBD_SIZE_ERR. */
#else
	uint64_t chan                         : 12;
	uint64_t fifo                         : 7;
	uint64_t val                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_pko_peb_subd_size_err_info_s cn78xx;
};
typedef union cvmx_pko_peb_subd_size_err_info cvmx_pko_peb_subd_size_err_info_t;

/**
 * cvmx_pko_peb_trunc_err_info
 */
union cvmx_pko_peb_trunc_err_info {
	uint64_t u64;
	struct cvmx_pko_peb_trunc_err_info_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t val                          : 1;  /**< Asserted when PKO_PEB_ERR_INT[PEB_TRUNC_ERR] is set. */
	uint64_t fifo                         : 7;  /**< FIFO number associated with the captured PEB_TRUNC_ERR. */
	uint64_t chan                         : 12; /**< Channel number associated with the captured PEB_TRUNC_ERR. */
#else
	uint64_t chan                         : 12;
	uint64_t fifo                         : 7;
	uint64_t val                          : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_pko_peb_trunc_err_info_s  cn78xx;
};
typedef union cvmx_pko_peb_trunc_err_info cvmx_pko_peb_trunc_err_info_t;

/**
 * cvmx_pko_pq_csr_bus_debug
 */
union cvmx_pko_pq_csr_bus_debug {
	uint64_t u64;
	struct cvmx_pko_pq_csr_bus_debug_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t csr_bus_debug                : 64; /**< N/A */
#else
	uint64_t csr_bus_debug                : 64;
#endif
	} s;
	struct cvmx_pko_pq_csr_bus_debug_s    cn78xx;
};
typedef union cvmx_pko_pq_csr_bus_debug cvmx_pko_pq_csr_bus_debug_t;

/**
 * cvmx_pko_pq_debug_green
 */
union cvmx_pko_pq_debug_green {
	uint64_t u64;
	struct cvmx_pko_pq_debug_green_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t g_valid                      : 32; /**< g_valid vector. */
	uint64_t cred_ok_n                    : 32; /**< cred_ok_n vector. */
#else
	uint64_t cred_ok_n                    : 32;
	uint64_t g_valid                      : 32;
#endif
	} s;
	struct cvmx_pko_pq_debug_green_s      cn78xx;
};
typedef union cvmx_pko_pq_debug_green cvmx_pko_pq_debug_green_t;

/**
 * cvmx_pko_pq_debug_links
 */
union cvmx_pko_pq_debug_links {
	uint64_t u64;
	struct cvmx_pko_pq_debug_links_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t links_ready                  : 32; /**< links_ready vector. */
	uint64_t peb_lnk_rdy_ir               : 32; /**< peb_lnk_rdy_ir vector. */
#else
	uint64_t peb_lnk_rdy_ir               : 32;
	uint64_t links_ready                  : 32;
#endif
	} s;
	struct cvmx_pko_pq_debug_links_s      cn78xx;
};
typedef union cvmx_pko_pq_debug_links cvmx_pko_pq_debug_links_t;

/**
 * cvmx_pko_pq_debug_yellow
 */
union cvmx_pko_pq_debug_yellow {
	uint64_t u64;
	struct cvmx_pko_pq_debug_yellow_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t y_valid                      : 32; /**< y_valid vector. */
	uint64_t reserved_28_31               : 4;
	uint64_t link_vv                      : 28; /**< link_vv vector. */
#else
	uint64_t link_vv                      : 28;
	uint64_t reserved_28_31               : 4;
	uint64_t y_valid                      : 32;
#endif
	} s;
	struct cvmx_pko_pq_debug_yellow_s     cn78xx;
};
typedef union cvmx_pko_pq_debug_yellow cvmx_pko_pq_debug_yellow_t;

/**
 * cvmx_pko_pqa_debug
 */
union cvmx_pko_pqa_debug {
	uint64_t u64;
	struct cvmx_pko_pqa_debug_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dbg_vec                      : 64; /**< Debug Vector. */
#else
	uint64_t dbg_vec                      : 64;
#endif
	} s;
	struct cvmx_pko_pqa_debug_s           cn78xx;
};
typedef union cvmx_pko_pqa_debug cvmx_pko_pqa_debug_t;

/**
 * cvmx_pko_pqb_debug
 */
union cvmx_pko_pqb_debug {
	uint64_t u64;
	struct cvmx_pko_pqb_debug_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dbg_vec                      : 64; /**< Debug Vector. */
#else
	uint64_t dbg_vec                      : 64;
#endif
	} s;
	struct cvmx_pko_pqb_debug_s           cn78xx;
};
typedef union cvmx_pko_pqb_debug cvmx_pko_pqb_debug_t;

/**
 * cvmx_pko_pse_dq_bist_status
 *
 * Each bit is the BIST result of an individual memory (per bit, 0 = pass and 1 = fail).
 *
 */
union cvmx_pko_pse_dq_bist_status {
	uint64_t u64;
	struct cvmx_pko_pse_dq_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t wt_sram                      : 1;  /**< Work table. */
	uint64_t rt7_sram                     : 1;  /**< Result table 7 - DQ FIFO[1023:896]. */
	uint64_t rt6_sram                     : 1;  /**< Result table 6 - DQ FIFO[895:768]. */
	uint64_t rt5_sram                     : 1;  /**< Result table 5 - DQ FIFO[767:640]. */
	uint64_t rt4_sram                     : 1;  /**< Result table 4 - DQ FIFO[639:512]. */
	uint64_t rt3_sram                     : 1;  /**< Result table 3 - DQ FIFO[511:384]. */
	uint64_t rt2_sram                     : 1;  /**< Result table 2 - DQ FIFO[383:256]. */
	uint64_t rt1_sram                     : 1;  /**< Result table 1 - DQ FIFO[255:128]. */
	uint64_t rt0_sram                     : 1;  /**< Result table 0 - DQ FIFO[127:0]. */
#else
	uint64_t rt0_sram                     : 1;
	uint64_t rt1_sram                     : 1;
	uint64_t rt2_sram                     : 1;
	uint64_t rt3_sram                     : 1;
	uint64_t rt4_sram                     : 1;
	uint64_t rt5_sram                     : 1;
	uint64_t rt6_sram                     : 1;
	uint64_t rt7_sram                     : 1;
	uint64_t wt_sram                      : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_pko_pse_dq_bist_status_s  cn78xx;
};
typedef union cvmx_pko_pse_dq_bist_status cvmx_pko_pse_dq_bist_status_t;

/**
 * cvmx_pko_pse_dq_ecc_ctl0
 */
union cvmx_pko_pse_dq_ecc_ctl0 {
	uint64_t u64;
	struct cvmx_pko_pse_dq_ecc_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dq_wt_ram_flip               : 2;  /**< DQ_WT_RAM flip syndrome bits on write. */
	uint64_t dq_wt_ram_cdis               : 1;  /**< DQ_WT_RAM ECC correction disable. */
	uint64_t dq_rt7_flip                  : 2;  /**< DQ_RT7 flip syndrome bits on write. */
	uint64_t dq_rt7_cdis                  : 1;  /**< DQ_RT7 ECC correction disable. */
	uint64_t dq_rt6_flip                  : 2;  /**< DQ_RT6 flip syndrome bits on write. */
	uint64_t dq_rt6_cdis                  : 1;  /**< DQ_RT6 ECC correction disable. */
	uint64_t dq_rt5_flip                  : 2;  /**< DQ_RT5 flip syndrome bits on write. */
	uint64_t dq_rt5_cdis                  : 1;  /**< DQ_RT5 ECC correction disable. */
	uint64_t dq_rt4_flip                  : 2;  /**< DQ_RT4 flip syndrome bits on write. */
	uint64_t dq_rt4_cdis                  : 1;  /**< DQ_RT4 ECC correction disable. */
	uint64_t dq_rt3_flip                  : 2;  /**< DQ_RT3 flip syndrome bits on write. */
	uint64_t dq_rt3_cdis                  : 1;  /**< DQ_RT3 ECC correction disable. */
	uint64_t dq_rt2_flip                  : 2;  /**< DQ_RT2 flip syndrome bits on write. */
	uint64_t dq_rt2_cdis                  : 1;  /**< DQ_RT2 ECC correction disable. */
	uint64_t dq_rt1_flip                  : 2;  /**< DQ_RT1 flip syndrome bits on write. */
	uint64_t dq_rt1_cdis                  : 1;  /**< DQ_RT1 ECC correction disable. */
	uint64_t dq_rt0_flip                  : 2;  /**< DQ_RT0 flip syndrome bits on write. */
	uint64_t dq_rt0_cdis                  : 1;  /**< DQ_RT0 ECC correction disable. */
	uint64_t reserved_0_36                : 37;
#else
	uint64_t reserved_0_36                : 37;
	uint64_t dq_rt0_cdis                  : 1;
	uint64_t dq_rt0_flip                  : 2;
	uint64_t dq_rt1_cdis                  : 1;
	uint64_t dq_rt1_flip                  : 2;
	uint64_t dq_rt2_cdis                  : 1;
	uint64_t dq_rt2_flip                  : 2;
	uint64_t dq_rt3_cdis                  : 1;
	uint64_t dq_rt3_flip                  : 2;
	uint64_t dq_rt4_cdis                  : 1;
	uint64_t dq_rt4_flip                  : 2;
	uint64_t dq_rt5_cdis                  : 1;
	uint64_t dq_rt5_flip                  : 2;
	uint64_t dq_rt6_cdis                  : 1;
	uint64_t dq_rt6_flip                  : 2;
	uint64_t dq_rt7_cdis                  : 1;
	uint64_t dq_rt7_flip                  : 2;
	uint64_t dq_wt_ram_cdis               : 1;
	uint64_t dq_wt_ram_flip               : 2;
#endif
	} s;
	struct cvmx_pko_pse_dq_ecc_ctl0_s     cn78xx;
};
typedef union cvmx_pko_pse_dq_ecc_ctl0 cvmx_pko_pse_dq_ecc_ctl0_t;

/**
 * cvmx_pko_pse_dq_ecc_dbe_sts0
 */
union cvmx_pko_pse_dq_ecc_dbe_sts0 {
	uint64_t u64;
	struct cvmx_pko_pse_dq_ecc_dbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dq_wt_ram_dbe                : 1;  /**< Double-bit error for DQ_WT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_dq.wt_sram */
	uint64_t dq_rt7_dbe                   : 1;  /**< Double-bit error for DQ_RT7_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_dq.rt7 */
	uint64_t dq_rt6_dbe                   : 1;  /**< Double-bit error for DQ_RT6_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_dq.rt6 */
	uint64_t dq_rt5_dbe                   : 1;  /**< Double-bit error for DQ_RT5_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_dq.rt5 */
	uint64_t dq_rt4_dbe                   : 1;  /**< Double-bit error for DQ_RT4_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_dq.rt4 */
	uint64_t dq_rt3_dbe                   : 1;  /**< Double-bit error for DQ_RT3_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_dq.rt3 */
	uint64_t dq_rt2_dbe                   : 1;  /**< Double-bit error for DQ_RT2_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_dq.rt2 */
	uint64_t dq_rt1_dbe                   : 1;  /**< Double-bit error for DQ_RT1_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_dq.rt1 */
	uint64_t dq_rt0_dbe                   : 1;  /**< Double-bit error for DQ_RT0_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_dq.rt0 */
	uint64_t reserved_0_54                : 55;
#else
	uint64_t reserved_0_54                : 55;
	uint64_t dq_rt0_dbe                   : 1;
	uint64_t dq_rt1_dbe                   : 1;
	uint64_t dq_rt2_dbe                   : 1;
	uint64_t dq_rt3_dbe                   : 1;
	uint64_t dq_rt4_dbe                   : 1;
	uint64_t dq_rt5_dbe                   : 1;
	uint64_t dq_rt6_dbe                   : 1;
	uint64_t dq_rt7_dbe                   : 1;
	uint64_t dq_wt_ram_dbe                : 1;
#endif
	} s;
	struct cvmx_pko_pse_dq_ecc_dbe_sts0_s cn78xx;
};
typedef union cvmx_pko_pse_dq_ecc_dbe_sts0 cvmx_pko_pse_dq_ecc_dbe_sts0_t;

/**
 * cvmx_pko_pse_dq_ecc_dbe_sts_cmb0
 */
union cvmx_pko_pse_dq_ecc_dbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pko_pse_dq_ecc_dbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_dq_dbe_cmb0              : 1;  /**< This bit is the logical OR of all bits in PKO_PSE_DQ_ECC_DBE_STS.
                                                         To clear this bit, software must clear bits in PKO_PSE_DQ_ECC_DBE_STS.
                                                         When this bit is set, the corresponding interrupt is set.
                                                         Throws PKO_INTSN_E::PKO_PSE_DQ_DBE_CMB0.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_dq.wt_sram
                                                         pko_pnr2.pko_pse.pse_dq.rt0
                                                         pko_pnr2.pko_pse.pse_dq.rt1
                                                         pko_pnr2.pko_pse.pse_dq.rt2
                                                         pko_pnr2.pko_pse.pse_dq.rt3
                                                         pko_pnr2.pko_pse.pse_dq.rt4
                                                         pko_pnr2.pko_pse.pse_dq.rt5
                                                         pko_pnr2.pko_pse.pse_dq.rt6
                                                         pko_pnr2.pko_pse.pse_dq.rt7 */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_dq_dbe_cmb0              : 1;
#endif
	} s;
	struct cvmx_pko_pse_dq_ecc_dbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pko_pse_dq_ecc_dbe_sts_cmb0 cvmx_pko_pse_dq_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_dq_ecc_sbe_sts0
 */
union cvmx_pko_pse_dq_ecc_sbe_sts0 {
	uint64_t u64;
	struct cvmx_pko_pse_dq_ecc_sbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dq_wt_ram_sbe                : 1;  /**< Single-bit error for DQ_WT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_dq.wt_sram */
	uint64_t dq_rt7_sbe                   : 1;  /**< Single-bit error for DQ_RT7_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_dq.rt7 */
	uint64_t dq_rt6_sbe                   : 1;  /**< Single-bit error for DQ_RT6_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_dq.rt6 */
	uint64_t dq_rt5_sbe                   : 1;  /**< Single-bit error for DQ_RT5_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_dq.rt5 */
	uint64_t dq_rt4_sbe                   : 1;  /**< Single-bit error for DQ_RT4_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_dq.rt4 */
	uint64_t dq_rt3_sbe                   : 1;  /**< Single-bit error for DQ_RT3_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_dq.rt3 */
	uint64_t dq_rt2_sbe                   : 1;  /**< Single-bit error for DQ_RT2_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_dq.rt2 */
	uint64_t dq_rt1_sbe                   : 1;  /**< Single-bit error for DQ_RT1_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_dq.rt1 */
	uint64_t dq_rt0_sbe                   : 1;  /**< Single-bit error for DQ_RT0_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_dq.rt0 */
	uint64_t reserved_0_54                : 55;
#else
	uint64_t reserved_0_54                : 55;
	uint64_t dq_rt0_sbe                   : 1;
	uint64_t dq_rt1_sbe                   : 1;
	uint64_t dq_rt2_sbe                   : 1;
	uint64_t dq_rt3_sbe                   : 1;
	uint64_t dq_rt4_sbe                   : 1;
	uint64_t dq_rt5_sbe                   : 1;
	uint64_t dq_rt6_sbe                   : 1;
	uint64_t dq_rt7_sbe                   : 1;
	uint64_t dq_wt_ram_sbe                : 1;
#endif
	} s;
	struct cvmx_pko_pse_dq_ecc_sbe_sts0_s cn78xx;
};
typedef union cvmx_pko_pse_dq_ecc_sbe_sts0 cvmx_pko_pse_dq_ecc_sbe_sts0_t;

/**
 * cvmx_pko_pse_dq_ecc_sbe_sts_cmb0
 */
union cvmx_pko_pse_dq_ecc_sbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pko_pse_dq_ecc_sbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_dq_sbe_cmb0              : 1;  /**< This bit is the logical OR of all bits in PKO_PSE_DQ_ECC_SBE_STS.
                                                         To clear this bit, software must clear bits in PKO_PSE_DQ_ECC_SBE_STS.
                                                         When this bit is set, the corresponding interrupt is set.
                                                         Throws PKO_INTSN_E::PKO_PSE_DQ_SBE_CMB0.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_dq.wt_sram
                                                         pko_pnr2.pko_pse.pse_dq.rt0
                                                         pko_pnr2.pko_pse.pse_dq.rt1
                                                         pko_pnr2.pko_pse.pse_dq.rt2
                                                         pko_pnr2.pko_pse.pse_dq.rt3
                                                         pko_pnr2.pko_pse.pse_dq.rt4
                                                         pko_pnr2.pko_pse.pse_dq.rt5
                                                         pko_pnr2.pko_pse.pse_dq.rt6
                                                         pko_pnr2.pko_pse.pse_dq.rt7 */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_dq_sbe_cmb0              : 1;
#endif
	} s;
	struct cvmx_pko_pse_dq_ecc_sbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pko_pse_dq_ecc_sbe_sts_cmb0 cvmx_pko_pse_dq_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_pq_bist_status
 *
 * Each bit is the BIST result of an individual memory (per bit, 0 = pass and 1 = fail).
 *
 */
union cvmx_pko_pse_pq_bist_status {
	uint64_t u64;
	struct cvmx_pko_pse_pq_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t tp_sram                      : 1;  /**< Topology parent - pko_pse_pq_srf32x5e */
	uint64_t irq_fifo_sram                : 1;  /**< Interrupt message FIFO - pko_pse_pq_srf1024x10e */
	uint64_t wmd_sram                     : 1;  /**< Dynamic watermark state - pko_pse_wmd_srf1024x49e */
	uint64_t wms_sram                     : 1;  /**< Static watermark configuration - pko_pse_wms_srf1024x50e */
	uint64_t cxd_sram                     : 1;  /**< Dynamic channel state - pko_pse_cxd_srf32x31e */
	uint64_t dqd_sram                     : 1;  /**< DQ dropped stats - pko_pse_stats_srf1024x88 */
	uint64_t dqs_sram                     : 1;  /**< DQ sent stats - pko_pse_stats_srf1024x88 */
	uint64_t pqd_sram                     : 1;  /**< PQ dropped stats - pko_pse_stats_srf32x88 */
	uint64_t pqr_sram                     : 1;  /**< PQ read stats - pko_pse_stats_srf32x88 */
	uint64_t pqy_sram                     : 1;  /**< PQ yellow stats - pko_pse_stats_srf32x88 */
	uint64_t pqg_sram                     : 1;  /**< PQ green stats - pko_pse_stats_srf32x88 */
	uint64_t std_sram                     : 1;  /**< Dynamic shaping state - pko_pse_std_srf32x105e */
	uint64_t st_sram                      : 1;  /**< Static shaping configuration - pko_pse_sts_srf32x74e */
	uint64_t reserved_1_1                 : 1;
	uint64_t cxs_sram                     : 1;  /**< Static channel credit configuration - pko_pse_cx0_srf32x6e */
#else
	uint64_t cxs_sram                     : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t st_sram                      : 1;
	uint64_t std_sram                     : 1;
	uint64_t pqg_sram                     : 1;
	uint64_t pqy_sram                     : 1;
	uint64_t pqr_sram                     : 1;
	uint64_t pqd_sram                     : 1;
	uint64_t dqs_sram                     : 1;
	uint64_t dqd_sram                     : 1;
	uint64_t cxd_sram                     : 1;
	uint64_t wms_sram                     : 1;
	uint64_t wmd_sram                     : 1;
	uint64_t irq_fifo_sram                : 1;
	uint64_t tp_sram                      : 1;
	uint64_t reserved_15_63               : 49;
#endif
	} s;
	struct cvmx_pko_pse_pq_bist_status_s  cn78xx;
};
typedef union cvmx_pko_pse_pq_bist_status cvmx_pko_pse_pq_bist_status_t;

/**
 * cvmx_pko_pse_pq_ecc_ctl0
 */
union cvmx_pko_pse_pq_ecc_ctl0 {
	uint64_t u64;
	struct cvmx_pko_pse_pq_ecc_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pq_cxs_ram_flip              : 2;  /**< PQ_CXS_RAM flip syndrome bits on write. */
	uint64_t pq_cxs_ram_cdis              : 1;  /**< PQ_CXS_RAM ECC correction disable. */
	uint64_t pq_cxd_ram_flip              : 2;  /**< PQ_CXD_RAM flip syndrome bits on write. */
	uint64_t pq_cxd_ram_cdis              : 1;  /**< PQ_CXD_RAM ECC correction disable. */
	uint64_t irq_fifo_sram_flip           : 2;  /**< IRQ_FIFO_SRAM flip syndrome bits on write. */
	uint64_t irq_fifo_sram_cdis           : 1;  /**< IRQ_FIFO_SRAM ECC correction disable. */
	uint64_t tp_sram_flip                 : 2;  /**< TP_SRAM flip syndrome bits on write. */
	uint64_t tp_sram_cdis                 : 1;  /**< TP_SRAM ECC correction disable. */
	uint64_t pq_std_ram_flip              : 2;  /**< PQ_STD_RAM flip syndrome bits on write. */
	uint64_t pq_std_ram_cdis              : 1;  /**< PQ_STD_RAM ECC correction disable. */
	uint64_t pq_st_ram_flip               : 2;  /**< PQ_ST_RAM flip syndrome bits on write. */
	uint64_t pq_st_ram_cdis               : 1;  /**< PQ_ST_RAM ECC correction disable. */
	uint64_t pq_wmd_ram_flip              : 2;  /**< PQ_WMD_RAM flip syndrome bits on write. */
	uint64_t pq_wmd_ram_cdis              : 1;  /**< PQ_WMD_RAM ECC correction disable. */
	uint64_t pq_wms_ram_flip              : 2;  /**< PQ_WMS_RAM flip syndrome bits on write. */
	uint64_t pq_wms_ram_cdis              : 1;  /**< PQ_WMS_RAM ECC correction disable. */
	uint64_t reserved_0_39                : 40;
#else
	uint64_t reserved_0_39                : 40;
	uint64_t pq_wms_ram_cdis              : 1;
	uint64_t pq_wms_ram_flip              : 2;
	uint64_t pq_wmd_ram_cdis              : 1;
	uint64_t pq_wmd_ram_flip              : 2;
	uint64_t pq_st_ram_cdis               : 1;
	uint64_t pq_st_ram_flip               : 2;
	uint64_t pq_std_ram_cdis              : 1;
	uint64_t pq_std_ram_flip              : 2;
	uint64_t tp_sram_cdis                 : 1;
	uint64_t tp_sram_flip                 : 2;
	uint64_t irq_fifo_sram_cdis           : 1;
	uint64_t irq_fifo_sram_flip           : 2;
	uint64_t pq_cxd_ram_cdis              : 1;
	uint64_t pq_cxd_ram_flip              : 2;
	uint64_t pq_cxs_ram_cdis              : 1;
	uint64_t pq_cxs_ram_flip              : 2;
#endif
	} s;
	struct cvmx_pko_pse_pq_ecc_ctl0_s     cn78xx;
};
typedef union cvmx_pko_pse_pq_ecc_ctl0 cvmx_pko_pse_pq_ecc_ctl0_t;

/**
 * cvmx_pko_pse_pq_ecc_dbe_sts0
 */
union cvmx_pko_pse_pq_ecc_dbe_sts0 {
	uint64_t u64;
	struct cvmx_pko_pse_pq_ecc_dbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pq_cxs_ram_dbe               : 1;  /**< Double-bit error for PQ_CXS_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.cxs_sram */
	uint64_t pq_cxd_ram_dbe               : 1;  /**< Double-bit error for PQ_CXD_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.cxd_sram */
	uint64_t irq_fifo_sram_dbe            : 1;  /**< Double-bit error for IRQ_FIFO_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.irq_fifo_sram */
	uint64_t tp_sram_dbe                  : 1;  /**< Double-bit error for TP_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.tp_sram */
	uint64_t pq_std_ram_dbe               : 1;  /**< Double-bit error for PQ_STD_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.std_sram */
	uint64_t pq_st_ram_dbe                : 1;  /**< Double-bit error for PQ_ST_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.st_sram */
	uint64_t pq_wmd_ram_dbe               : 1;  /**< Double-bit error for PQ_WMD_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.wmd_sram */
	uint64_t pq_wms_ram_dbe               : 1;  /**< Double-bit error for PQ_WMS_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.wms_sram */
	uint64_t reserved_0_55                : 56;
#else
	uint64_t reserved_0_55                : 56;
	uint64_t pq_wms_ram_dbe               : 1;
	uint64_t pq_wmd_ram_dbe               : 1;
	uint64_t pq_st_ram_dbe                : 1;
	uint64_t pq_std_ram_dbe               : 1;
	uint64_t tp_sram_dbe                  : 1;
	uint64_t irq_fifo_sram_dbe            : 1;
	uint64_t pq_cxd_ram_dbe               : 1;
	uint64_t pq_cxs_ram_dbe               : 1;
#endif
	} s;
	struct cvmx_pko_pse_pq_ecc_dbe_sts0_s cn78xx;
};
typedef union cvmx_pko_pse_pq_ecc_dbe_sts0 cvmx_pko_pse_pq_ecc_dbe_sts0_t;

/**
 * cvmx_pko_pse_pq_ecc_dbe_sts_cmb0
 */
union cvmx_pko_pse_pq_ecc_dbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pko_pse_pq_ecc_dbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_pq_dbe_cmb0              : 1;  /**< This bit is the logical OR of all bits in PKO_PSE_PQ_ECC_DBE_STS.
                                                         To clear this bit, software must clear bits in PKO_PSE_PQ_ECC_DBE_STS.
                                                         When this bit is set, the corresponding interrupt is set.
                                                         Throws PKO_INTSN_E::PKO_PSE_PQ_DBE_CMB0.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.cxs_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.cxd_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.irq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.tp_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.std_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.st_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.wmd_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.wms_sram */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_pq_dbe_cmb0              : 1;
#endif
	} s;
	struct cvmx_pko_pse_pq_ecc_dbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pko_pse_pq_ecc_dbe_sts_cmb0 cvmx_pko_pse_pq_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_pq_ecc_sbe_sts0
 */
union cvmx_pko_pse_pq_ecc_sbe_sts0 {
	uint64_t u64;
	struct cvmx_pko_pse_pq_ecc_sbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pq_cxs_ram_sbe               : 1;  /**< Single-bit error for PQ_CXS_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.cxs_sram */
	uint64_t pq_cxd_ram_sbe               : 1;  /**< Single-bit error for PQ_CXD_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.cxd_sram */
	uint64_t irq_fifo_sram_sbe            : 1;  /**< Single-bit error for IRQ_FIFO_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.irq_fifo_sram */
	uint64_t tp_sram_sbe                  : 1;  /**< Single-bit error for TP_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.tp_sram */
	uint64_t pq_std_ram_sbe               : 1;  /**< Single-bit error for PQ_STD_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.std_sram */
	uint64_t pq_st_ram_sbe                : 1;  /**< Single-bit error for PQ_ST_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.st_sram */
	uint64_t pq_wmd_ram_sbe               : 1;  /**< Single-bit error for PQ_WMD_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.wmd_sram */
	uint64_t pq_wms_ram_sbe               : 1;  /**< Single-bit error for PQ_WMS_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.wms_sram */
	uint64_t reserved_0_55                : 56;
#else
	uint64_t reserved_0_55                : 56;
	uint64_t pq_wms_ram_sbe               : 1;
	uint64_t pq_wmd_ram_sbe               : 1;
	uint64_t pq_st_ram_sbe                : 1;
	uint64_t pq_std_ram_sbe               : 1;
	uint64_t tp_sram_sbe                  : 1;
	uint64_t irq_fifo_sram_sbe            : 1;
	uint64_t pq_cxd_ram_sbe               : 1;
	uint64_t pq_cxs_ram_sbe               : 1;
#endif
	} s;
	struct cvmx_pko_pse_pq_ecc_sbe_sts0_s cn78xx;
};
typedef union cvmx_pko_pse_pq_ecc_sbe_sts0 cvmx_pko_pse_pq_ecc_sbe_sts0_t;

/**
 * cvmx_pko_pse_pq_ecc_sbe_sts_cmb0
 */
union cvmx_pko_pse_pq_ecc_sbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pko_pse_pq_ecc_sbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_pq_sbe_cmb0              : 1;  /**< This bit is the logical OR of all bits in PKO_PSE_PQ_ECC_SBE_STS.
                                                         To clear this bit, software must clear bits in PKO_PSE_PQ_ECC_SBE_STS.
                                                         When this bit is set, the corresponding interrupt is set.
                                                         Throws PKO_INTSN_E::PKO_PSE_PQ_SBE_CMB0.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.cxs_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.cxd_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.irq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.tp_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.std_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.st_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.wmd_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.pq.wms_sram */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_pq_sbe_cmb0              : 1;
#endif
	} s;
	struct cvmx_pko_pse_pq_ecc_sbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pko_pse_pq_ecc_sbe_sts_cmb0 cvmx_pko_pse_pq_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_sq1_bist_status
 *
 * Each bit is the BIST result of an individual memory (per bit, 0 = pass and 1 = fail).
 *
 */
union cvmx_pko_pse_sq1_bist_status {
	uint64_t u64;
	struct cvmx_pko_pse_sq1_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t sc_sram                      : 1;  /**< SQ[5:1] scheduling configuration */
	uint64_t pc_sram                      : 1;  /**< SQ[1] physical channel - pko_pse_pc_srf32x12e */
	uint64_t xon_sram                     : 1;  /**< XON SRAM */
	uint64_t cc_sram                      : 1;  /**< SQ[1] channel credit OK state array */
	uint64_t vc1_sram                     : 1;  /**< SQ[1] virtual channel - pko_pse_sq1_vc_srf256x9e */
	uint64_t vc0_sram                     : 1;  /**< SQ[1] virtual channel - pko_pse_sq1_vc_srf256x9e */
	uint64_t reserved_21_22               : 2;
	uint64_t tp1_sram                     : 1;  /**< SQ[5:1] topology parent configuration */
	uint64_t tp0_sram                     : 1;  /**< SQ[5:1] topology parent configuration */
	uint64_t xo_sram                      : 1;  /**< XOFF SRAM */
	uint64_t rt_sram                      : 1;  /**< Result table */
	uint64_t reserved_9_16                : 8;
	uint64_t tw1_cmd_fifo                 : 1;  /**< SQ[5:1] time wheel 1 command FIFO SRAM */
	uint64_t std_sram                     : 1;  /**< Dynamic shaping state */
	uint64_t sts_sram                     : 1;  /**< Static shaping configuration */
	uint64_t tw0_cmd_fifo                 : 1;  /**< SQ[5:1] time wheel 0 command FIFO SRAM */
	uint64_t cxd_sram                     : 1;  /**< SQ[1] dynamic channel credit state */
	uint64_t cxs_sram                     : 1;  /**< SQ[1] static channel credit configuration */
	uint64_t nt_sram                      : 1;  /**< SQ[5:1] next pointer table */
	uint64_t pt_sram                      : 1;  /**< SQ[5:1] previous pointer table */
	uint64_t wt_sram                      : 1;  /**< SQ[5:1] work table */
#else
	uint64_t wt_sram                      : 1;
	uint64_t pt_sram                      : 1;
	uint64_t nt_sram                      : 1;
	uint64_t cxs_sram                     : 1;
	uint64_t cxd_sram                     : 1;
	uint64_t tw0_cmd_fifo                 : 1;
	uint64_t sts_sram                     : 1;
	uint64_t std_sram                     : 1;
	uint64_t tw1_cmd_fifo                 : 1;
	uint64_t reserved_9_16                : 8;
	uint64_t rt_sram                      : 1;
	uint64_t xo_sram                      : 1;
	uint64_t tp0_sram                     : 1;
	uint64_t tp1_sram                     : 1;
	uint64_t reserved_21_22               : 2;
	uint64_t vc0_sram                     : 1;
	uint64_t vc1_sram                     : 1;
	uint64_t cc_sram                      : 1;
	uint64_t xon_sram                     : 1;
	uint64_t pc_sram                      : 1;
	uint64_t sc_sram                      : 1;
	uint64_t reserved_29_63               : 35;
#endif
	} s;
	struct cvmx_pko_pse_sq1_bist_status_s cn78xx;
};
typedef union cvmx_pko_pse_sq1_bist_status cvmx_pko_pse_sq1_bist_status_t;

/**
 * cvmx_pko_pse_sq1_ecc_ctl0
 */
union cvmx_pko_pse_sq1_ecc_ctl0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq1_ecc_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t cxs_ram_flip                 : 2;  /**< CXS_RAM flip syndrome bits on write. */
	uint64_t cxs_ram_cdis                 : 1;  /**< CXS_RAM ECC correction disable. */
	uint64_t cxd_ram_flip                 : 2;  /**< CXD_RAM flip syndrome bits on write. */
	uint64_t cxd_ram_cdis                 : 1;  /**< CXD_RAM ECC correction disable. */
	uint64_t vc1_sram_flip                : 2;  /**< VC1_SRAM flip syndrome bits on write. */
	uint64_t vc1_sram_cdis                : 1;  /**< VC1_SRAM ECC correction disable. */
	uint64_t vc0_sram_flip                : 2;  /**< VC0_SRAM flip syndrome bits on write. */
	uint64_t vc0_sram_cdis                : 1;  /**< VC0_SRAM ECC correction disable. */
	uint64_t sq_pt_ram_flip               : 2;  /**< SQ_PT_RAM flip syndrome bits on write. */
	uint64_t sq_pt_ram_cdis               : 1;  /**< SQ_PT_RAM ECC correction disable. */
	uint64_t sq_nt_ram_flip               : 2;  /**< SQ_NT_RAM flip syndrome bits on write. */
	uint64_t sq_nt_ram_cdis               : 1;  /**< SQ_NT_RAM ECC correction disable. */
	uint64_t rt_ram_flip                  : 2;  /**< RT_RAM flip syndrome bits on write. */
	uint64_t rt_ram_cdis                  : 1;  /**< RT_RAM ECC correction disable. */
	uint64_t pc_ram_flip                  : 2;  /**< PC_RAM flip syndrome bits on write. */
	uint64_t pc_ram_cdis                  : 1;  /**< PC_RAM ECC correction disable. */
	uint64_t tw1_cmd_fifo_ram_flip        : 2;  /**< TW1_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t tw1_cmd_fifo_ram_cdis        : 1;  /**< TW1_CMD_FIFO_RAM ECC correction disable. */
	uint64_t tw0_cmd_fifo_ram_flip        : 2;  /**< TW0_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t tw0_cmd_fifo_ram_cdis        : 1;  /**< TW0_CMD_FIFO_RAM ECC correction disable. */
	uint64_t tp1_sram_flip                : 2;  /**< TP1_SRAM flip syndrome bits on write. */
	uint64_t tp1_sram_cdis                : 1;  /**< TP1_SRAM ECC correction disable. */
	uint64_t tp0_sram_flip                : 2;  /**< TP0_SRAM flip syndrome bits on write. */
	uint64_t tp0_sram_cdis                : 1;  /**< TP0_SRAM ECC correction disable. */
	uint64_t sts1_ram_flip                : 2;  /**< STS1_RAM flip syndrome bits on write. */
	uint64_t sts1_ram_cdis                : 1;  /**< STS1_RAM ECC correction disable. */
	uint64_t sts0_ram_flip                : 2;  /**< STS0_RAM flip syndrome bits on write. */
	uint64_t sts0_ram_cdis                : 1;  /**< STS0_RAM ECC correction disable. */
	uint64_t std1_ram_flip                : 2;  /**< STD1_RAM flip syndrome bits on write. */
	uint64_t std1_ram_cdis                : 1;  /**< STD1_RAM ECC correction disable. */
	uint64_t std0_ram_flip                : 2;  /**< STD0_RAM flip syndrome bits on write. */
	uint64_t std0_ram_cdis                : 1;  /**< STD0_RAM ECC correction disable. */
	uint64_t wt_ram_flip                  : 2;  /**< WT_RAM flip syndrome bits on write. */
	uint64_t wt_ram_cdis                  : 1;  /**< WT_RAM ECC correction disable. */
	uint64_t sc_ram_flip                  : 2;  /**< SC_RAM flip syndrome bits on write. */
	uint64_t sc_ram_cdis                  : 1;  /**< SC_RAM ECC correction disable. */
	uint64_t reserved_0_9                 : 10;
#else
	uint64_t reserved_0_9                 : 10;
	uint64_t sc_ram_cdis                  : 1;
	uint64_t sc_ram_flip                  : 2;
	uint64_t wt_ram_cdis                  : 1;
	uint64_t wt_ram_flip                  : 2;
	uint64_t std0_ram_cdis                : 1;
	uint64_t std0_ram_flip                : 2;
	uint64_t std1_ram_cdis                : 1;
	uint64_t std1_ram_flip                : 2;
	uint64_t sts0_ram_cdis                : 1;
	uint64_t sts0_ram_flip                : 2;
	uint64_t sts1_ram_cdis                : 1;
	uint64_t sts1_ram_flip                : 2;
	uint64_t tp0_sram_cdis                : 1;
	uint64_t tp0_sram_flip                : 2;
	uint64_t tp1_sram_cdis                : 1;
	uint64_t tp1_sram_flip                : 2;
	uint64_t tw0_cmd_fifo_ram_cdis        : 1;
	uint64_t tw0_cmd_fifo_ram_flip        : 2;
	uint64_t tw1_cmd_fifo_ram_cdis        : 1;
	uint64_t tw1_cmd_fifo_ram_flip        : 2;
	uint64_t pc_ram_cdis                  : 1;
	uint64_t pc_ram_flip                  : 2;
	uint64_t rt_ram_cdis                  : 1;
	uint64_t rt_ram_flip                  : 2;
	uint64_t sq_nt_ram_cdis               : 1;
	uint64_t sq_nt_ram_flip               : 2;
	uint64_t sq_pt_ram_cdis               : 1;
	uint64_t sq_pt_ram_flip               : 2;
	uint64_t vc0_sram_cdis                : 1;
	uint64_t vc0_sram_flip                : 2;
	uint64_t vc1_sram_cdis                : 1;
	uint64_t vc1_sram_flip                : 2;
	uint64_t cxd_ram_cdis                 : 1;
	uint64_t cxd_ram_flip                 : 2;
	uint64_t cxs_ram_cdis                 : 1;
	uint64_t cxs_ram_flip                 : 2;
#endif
	} s;
	struct cvmx_pko_pse_sq1_ecc_ctl0_s    cn78xx;
};
typedef union cvmx_pko_pse_sq1_ecc_ctl0 cvmx_pko_pse_sq1_ecc_ctl0_t;

/**
 * cvmx_pko_pse_sq1_ecc_dbe_sts0
 */
union cvmx_pko_pse_sq1_ecc_dbe_sts0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq1_ecc_dbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t cxs_ram_dbe                  : 1;  /**< Double-bit error for CXS_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.cxs_sram */
	uint64_t cxd_ram_dbe                  : 1;  /**< Double-bit error for CXD_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.cxd_sram */
	uint64_t vc1_sram_dbe                 : 1;  /**< Double-bit error for VC1_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.vc1_sram */
	uint64_t vc0_sram_dbe                 : 1;  /**< Double-bit error for VC0_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.vc0_sram */
	uint64_t sq_pt_ram_dbe                : 1;  /**< Double-bit error for SQ_PT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.nt_sram */
	uint64_t sq_nt_ram_dbe                : 1;  /**< Double-bit error for SQ_NT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.pt_sram */
	uint64_t rt_ram_dbe                   : 1;  /**< Double-bit error for RT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.rt_sram */
	uint64_t pc_ram_dbe                   : 1;  /**< Double-bit error for PC_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.pc_sram */
	uint64_t tw1_cmd_fifo_ram_dbe         : 1;  /**< Double-bit error for TW1_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.tw_0.sq_fifo_sram */
	uint64_t tw0_cmd_fifo_ram_dbe         : 1;  /**< Double-bit error for TW0_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.tw_1.sq_fifo_sram */
	uint64_t tp1_sram_dbe                 : 1;  /**< Double-bit error for TP1_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.tp0_sram */
	uint64_t tp0_sram_dbe                 : 1;  /**< Double-bit error for TP0_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.tp1_sram */
	uint64_t sts1_ram_dbe                 : 1;  /**< Double-bit error for STS1_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.sts1_sram */
	uint64_t sts0_ram_dbe                 : 1;  /**< Double-bit error for STS0_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.sts0_sram */
	uint64_t std1_ram_dbe                 : 1;  /**< Double-bit error for STD1_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.std1_sram */
	uint64_t std0_ram_dbe                 : 1;  /**< Double-bit error for STD0_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.std0_sram */
	uint64_t wt_ram_dbe                   : 1;  /**< Double-bit error for WT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.wt_sram */
	uint64_t sc_ram_dbe                   : 1;  /**< Double-bit error for SC_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.sc_sram */
	uint64_t reserved_0_45                : 46;
#else
	uint64_t reserved_0_45                : 46;
	uint64_t sc_ram_dbe                   : 1;
	uint64_t wt_ram_dbe                   : 1;
	uint64_t std0_ram_dbe                 : 1;
	uint64_t std1_ram_dbe                 : 1;
	uint64_t sts0_ram_dbe                 : 1;
	uint64_t sts1_ram_dbe                 : 1;
	uint64_t tp0_sram_dbe                 : 1;
	uint64_t tp1_sram_dbe                 : 1;
	uint64_t tw0_cmd_fifo_ram_dbe         : 1;
	uint64_t tw1_cmd_fifo_ram_dbe         : 1;
	uint64_t pc_ram_dbe                   : 1;
	uint64_t rt_ram_dbe                   : 1;
	uint64_t sq_nt_ram_dbe                : 1;
	uint64_t sq_pt_ram_dbe                : 1;
	uint64_t vc0_sram_dbe                 : 1;
	uint64_t vc1_sram_dbe                 : 1;
	uint64_t cxd_ram_dbe                  : 1;
	uint64_t cxs_ram_dbe                  : 1;
#endif
	} s;
	struct cvmx_pko_pse_sq1_ecc_dbe_sts0_s cn78xx;
};
typedef union cvmx_pko_pse_sq1_ecc_dbe_sts0 cvmx_pko_pse_sq1_ecc_dbe_sts0_t;

/**
 * cvmx_pko_pse_sq1_ecc_dbe_sts_cmb0
 */
union cvmx_pko_pse_sq1_ecc_dbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq1_ecc_dbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_sq1_dbe_cmb0             : 1;  /**< This bit is the logical OR of all bits in PKO_PSE_SQ1_ECC_DBE_STS.
                                                         To clear this bit, software must clear bits in PKO_PSE_SQ1_ECC_DBE_STS.
                                                         When this bit is set, the corresponding interrupt is set.
                                                         Throws PKO_INTSN_E::PKO_PSE_SQ1_DBE_CMB0.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.cxs_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.cxd_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.vc0_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.vc1_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.nt_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.pt_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.pc_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.rt_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.tw_0.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.tw_1.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.tp0_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.tp1_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.std0_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.std1_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.sts0_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.sts1_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.wt_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.sc_sram */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_sq1_dbe_cmb0             : 1;
#endif
	} s;
	struct cvmx_pko_pse_sq1_ecc_dbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pko_pse_sq1_ecc_dbe_sts_cmb0 cvmx_pko_pse_sq1_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_sq1_ecc_sbe_sts0
 */
union cvmx_pko_pse_sq1_ecc_sbe_sts0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq1_ecc_sbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t cxs_ram_sbe                  : 1;  /**< Single-bit error for CXS_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.cxs_sram */
	uint64_t cxd_ram_sbe                  : 1;  /**< Single-bit error for CXD_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.cxd_sram */
	uint64_t vc1_sram_sbe                 : 1;  /**< Single-bit error for VC1_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.vc0_sram */
	uint64_t vc0_sram_sbe                 : 1;  /**< Single-bit error for VC0_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.vc1_sram */
	uint64_t sq_pt_ram_sbe                : 1;  /**< Single-bit error for SQ_PT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.nt_sram */
	uint64_t sq_nt_ram_sbe                : 1;  /**< Single-bit error for SQ_NT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.pt_sram */
	uint64_t rt_ram_sbe                   : 1;  /**< Single-bit error for RT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.rt_sram */
	uint64_t pc_ram_sbe                   : 1;  /**< Single-bit error for PC_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.pc_sram */
	uint64_t tw1_cmd_fifo_ram_sbe         : 1;  /**< Single-bit error for TW1_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.tw_0.sq_fifo_sram */
	uint64_t tw0_cmd_fifo_ram_sbe         : 1;  /**< Single-bit error for TW0_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.tw_1.sq_fifo_sram */
	uint64_t tp1_sram_sbe                 : 1;  /**< Single-bit error for TP1_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.tp0_sram */
	uint64_t tp0_sram_sbe                 : 1;  /**< Single-bit error for TP0_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.tp1_sram */
	uint64_t sts1_ram_sbe                 : 1;  /**< Single-bit error for STS1_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.sts0_sram */
	uint64_t sts0_ram_sbe                 : 1;  /**< Single-bit error for STS0_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.sts1_sram */
	uint64_t std1_ram_sbe                 : 1;  /**< Single-bit error for STD1_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.std0_sram */
	uint64_t std0_ram_sbe                 : 1;  /**< Single-bit error for STD0_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.std1_sram */
	uint64_t wt_ram_sbe                   : 1;  /**< Single-bit error for WT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.wt_sram */
	uint64_t sc_ram_sbe                   : 1;  /**< Single-bit error for SC_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.sc_sram */
	uint64_t reserved_0_45                : 46;
#else
	uint64_t reserved_0_45                : 46;
	uint64_t sc_ram_sbe                   : 1;
	uint64_t wt_ram_sbe                   : 1;
	uint64_t std0_ram_sbe                 : 1;
	uint64_t std1_ram_sbe                 : 1;
	uint64_t sts0_ram_sbe                 : 1;
	uint64_t sts1_ram_sbe                 : 1;
	uint64_t tp0_sram_sbe                 : 1;
	uint64_t tp1_sram_sbe                 : 1;
	uint64_t tw0_cmd_fifo_ram_sbe         : 1;
	uint64_t tw1_cmd_fifo_ram_sbe         : 1;
	uint64_t pc_ram_sbe                   : 1;
	uint64_t rt_ram_sbe                   : 1;
	uint64_t sq_nt_ram_sbe                : 1;
	uint64_t sq_pt_ram_sbe                : 1;
	uint64_t vc0_sram_sbe                 : 1;
	uint64_t vc1_sram_sbe                 : 1;
	uint64_t cxd_ram_sbe                  : 1;
	uint64_t cxs_ram_sbe                  : 1;
#endif
	} s;
	struct cvmx_pko_pse_sq1_ecc_sbe_sts0_s cn78xx;
};
typedef union cvmx_pko_pse_sq1_ecc_sbe_sts0 cvmx_pko_pse_sq1_ecc_sbe_sts0_t;

/**
 * cvmx_pko_pse_sq1_ecc_sbe_sts_cmb0
 */
union cvmx_pko_pse_sq1_ecc_sbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq1_ecc_sbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_sq1_sbe_cmb0             : 1;  /**< This bit is the logical OR of all bits in PKO_PSE_SQ1_ECC_SBE_STS.
                                                         To clear this bit, software must clear bits in PKO_PSE_SQ1_ECC_SBE_STS.
                                                         When this bit is set, the corresponding interrupt is set.
                                                         Throws PKO_INTSN_E::PKO_PSE_SQ1_SBE_CMB0.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.cxs_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.cxd_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.vc0_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.vc1_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.nt_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.pt_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.pc_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.rt_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.tw_0.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.tw_1.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.tp0_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.tp1_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.std0_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.std1_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.sts0_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.sts1_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.wt_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.sc_sram */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_sq1_sbe_cmb0             : 1;
#endif
	} s;
	struct cvmx_pko_pse_sq1_ecc_sbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pko_pse_sq1_ecc_sbe_sts_cmb0 cvmx_pko_pse_sq1_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_sq2_bist_status
 *
 * Each bit is the BIST result of an individual memory (per bit, 0 = pass and 1 = fail).
 *
 */
union cvmx_pko_pse_sq2_bist_status {
	uint64_t u64;
	struct cvmx_pko_pse_sq2_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t sc_sram                      : 1;  /**< Scheduling configuration. */
	uint64_t reserved_21_27               : 7;
	uint64_t tp1_sram                     : 1;  /**< SQ[5:1] topology parent configuration. */
	uint64_t tp0_sram                     : 1;  /**< SQ[5:1] topology parent configuration. */
	uint64_t reserved_18_18               : 1;
	uint64_t rt_sram                      : 1;  /**< Result table. */
	uint64_t reserved_9_16                : 8;
	uint64_t tw1_cmd_fifo                 : 1;  /**< SQ[5:1] time wheel 1 command FIFO SRAM. */
	uint64_t std_sram                     : 1;  /**< Dynamic shaping state */
	uint64_t sts_sram                     : 1;  /**< Static shaping configuration. */
	uint64_t tw0_cmd_fifo                 : 1;  /**< SQ[5:1] time wheel 0 command FIFO SRAM. */
	uint64_t reserved_3_4                 : 2;
	uint64_t nt_sram                      : 1;  /**< Next pointer table. */
	uint64_t pt_sram                      : 1;  /**< Previous pointer table. */
	uint64_t wt_sram                      : 1;  /**< Work table. */
#else
	uint64_t wt_sram                      : 1;
	uint64_t pt_sram                      : 1;
	uint64_t nt_sram                      : 1;
	uint64_t reserved_3_4                 : 2;
	uint64_t tw0_cmd_fifo                 : 1;
	uint64_t sts_sram                     : 1;
	uint64_t std_sram                     : 1;
	uint64_t tw1_cmd_fifo                 : 1;
	uint64_t reserved_9_16                : 8;
	uint64_t rt_sram                      : 1;
	uint64_t reserved_18_18               : 1;
	uint64_t tp0_sram                     : 1;
	uint64_t tp1_sram                     : 1;
	uint64_t reserved_21_27               : 7;
	uint64_t sc_sram                      : 1;
	uint64_t reserved_29_63               : 35;
#endif
	} s;
	struct cvmx_pko_pse_sq2_bist_status_s cn78xx;
};
typedef union cvmx_pko_pse_sq2_bist_status cvmx_pko_pse_sq2_bist_status_t;

/**
 * cvmx_pko_pse_sq2_ecc_ctl0
 */
union cvmx_pko_pse_sq2_ecc_ctl0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq2_ecc_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sq_pt_ram_flip               : 2;  /**< SQ_PT_RAM flip syndrome bits on write. */
	uint64_t sq_pt_ram_cdis               : 1;  /**< SQ_PT_RAM ECC correction disable. */
	uint64_t sq_nt_ram_flip               : 2;  /**< SQ_NT_RAM flip syndrome bits on write. */
	uint64_t sq_nt_ram_cdis               : 1;  /**< SQ_NT_RAM ECC correction disable. */
	uint64_t rt_ram_flip                  : 2;  /**< RT_RAM flip syndrome bits on write. */
	uint64_t rt_ram_cdis                  : 1;  /**< RT_RAM ECC correction disable. */
	uint64_t tw1_cmd_fifo_ram_flip        : 2;  /**< TW1_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t tw1_cmd_fifo_ram_cdis        : 1;  /**< TW1_CMD_FIFO_RAM ECC correction disable. */
	uint64_t tw0_cmd_fifo_ram_flip        : 2;  /**< TW0_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t tw0_cmd_fifo_ram_cdis        : 1;  /**< TW0_CMD_FIFO_RAM ECC correction disable. */
	uint64_t tp1_sram_flip                : 2;  /**< TP1_SRAM flip syndrome bits on write. */
	uint64_t tp1_sram_cdis                : 1;  /**< TP1_SRAM ECC correction disable. */
	uint64_t tp0_sram_flip                : 2;  /**< TP0_SRAM flip syndrome bits on write. */
	uint64_t tp0_sram_cdis                : 1;  /**< TP0_SRAM ECC correction disable. */
	uint64_t sts1_ram_flip                : 2;  /**< STS1_RAM flip syndrome bits on write. */
	uint64_t sts1_ram_cdis                : 1;  /**< STS1_RAM ECC correction disable. */
	uint64_t sts0_ram_flip                : 2;  /**< STS0_RAM flip syndrome bits on write. */
	uint64_t sts0_ram_cdis                : 1;  /**< STS0_RAM ECC correction disable. */
	uint64_t std1_ram_flip                : 2;  /**< STD1_RAM flip syndrome bits on write. */
	uint64_t std1_ram_cdis                : 1;  /**< STD1_RAM ECC correction disable. */
	uint64_t std0_ram_flip                : 2;  /**< STD0_RAM flip syndrome bits on write. */
	uint64_t std0_ram_cdis                : 1;  /**< STD0_RAM ECC correction disable. */
	uint64_t wt_ram_flip                  : 2;  /**< WT_RAM flip syndrome bits on write. */
	uint64_t wt_ram_cdis                  : 1;  /**< WT_RAM ECC correction disable. */
	uint64_t sc_ram_flip                  : 2;  /**< SC_RAM flip syndrome bits on write. */
	uint64_t sc_ram_cdis                  : 1;  /**< SC_RAM ECC correction disable. */
	uint64_t reserved_0_24                : 25;
#else
	uint64_t reserved_0_24                : 25;
	uint64_t sc_ram_cdis                  : 1;
	uint64_t sc_ram_flip                  : 2;
	uint64_t wt_ram_cdis                  : 1;
	uint64_t wt_ram_flip                  : 2;
	uint64_t std0_ram_cdis                : 1;
	uint64_t std0_ram_flip                : 2;
	uint64_t std1_ram_cdis                : 1;
	uint64_t std1_ram_flip                : 2;
	uint64_t sts0_ram_cdis                : 1;
	uint64_t sts0_ram_flip                : 2;
	uint64_t sts1_ram_cdis                : 1;
	uint64_t sts1_ram_flip                : 2;
	uint64_t tp0_sram_cdis                : 1;
	uint64_t tp0_sram_flip                : 2;
	uint64_t tp1_sram_cdis                : 1;
	uint64_t tp1_sram_flip                : 2;
	uint64_t tw0_cmd_fifo_ram_cdis        : 1;
	uint64_t tw0_cmd_fifo_ram_flip        : 2;
	uint64_t tw1_cmd_fifo_ram_cdis        : 1;
	uint64_t tw1_cmd_fifo_ram_flip        : 2;
	uint64_t rt_ram_cdis                  : 1;
	uint64_t rt_ram_flip                  : 2;
	uint64_t sq_nt_ram_cdis               : 1;
	uint64_t sq_nt_ram_flip               : 2;
	uint64_t sq_pt_ram_cdis               : 1;
	uint64_t sq_pt_ram_flip               : 2;
#endif
	} s;
	struct cvmx_pko_pse_sq2_ecc_ctl0_s    cn78xx;
};
typedef union cvmx_pko_pse_sq2_ecc_ctl0 cvmx_pko_pse_sq2_ecc_ctl0_t;

/**
 * cvmx_pko_pse_sq2_ecc_dbe_sts0
 */
union cvmx_pko_pse_sq2_ecc_dbe_sts0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq2_ecc_dbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sq_pt_ram_dbe                : 1;  /**< Double-bit error for SQ_PT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.nt_sram */
	uint64_t sq_nt_ram_dbe                : 1;  /**< Double-bit error for SQ_NT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.pt_sram */
	uint64_t rt_ram_dbe                   : 1;  /**< Double-bit error for RT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.rt_sram */
	uint64_t tw1_cmd_fifo_ram_dbe         : 1;  /**< Double-bit error for TW1_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.tw_1.sq_fifo_sram */
	uint64_t tw0_cmd_fifo_ram_dbe         : 1;  /**< Double-bit error for TW0_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.tw_0.sq_fifo_sram */
	uint64_t tp1_sram_dbe                 : 1;  /**< Double-bit error for TP1_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.tp1_sram */
	uint64_t tp0_sram_dbe                 : 1;  /**< Double-bit error for TP0_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.tp0_sram */
	uint64_t sts1_ram_dbe                 : 1;  /**< Double-bit error for STS1_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.sts1_sram */
	uint64_t sts0_ram_dbe                 : 1;  /**< Double-bit error for STS0_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.sts0_sram */
	uint64_t std1_ram_dbe                 : 1;  /**< Double-bit error for STD1_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.std1_sram */
	uint64_t std0_ram_dbe                 : 1;  /**< Double-bit error for STD0_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.std0_sram */
	uint64_t wt_ram_dbe                   : 1;  /**< Double-bit error for WT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.wt_sram */
	uint64_t sc_ram_dbe                   : 1;  /**< Double-bit error for SC_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.sc_sram */
	uint64_t reserved_0_50                : 51;
#else
	uint64_t reserved_0_50                : 51;
	uint64_t sc_ram_dbe                   : 1;
	uint64_t wt_ram_dbe                   : 1;
	uint64_t std0_ram_dbe                 : 1;
	uint64_t std1_ram_dbe                 : 1;
	uint64_t sts0_ram_dbe                 : 1;
	uint64_t sts1_ram_dbe                 : 1;
	uint64_t tp0_sram_dbe                 : 1;
	uint64_t tp1_sram_dbe                 : 1;
	uint64_t tw0_cmd_fifo_ram_dbe         : 1;
	uint64_t tw1_cmd_fifo_ram_dbe         : 1;
	uint64_t rt_ram_dbe                   : 1;
	uint64_t sq_nt_ram_dbe                : 1;
	uint64_t sq_pt_ram_dbe                : 1;
#endif
	} s;
	struct cvmx_pko_pse_sq2_ecc_dbe_sts0_s cn78xx;
};
typedef union cvmx_pko_pse_sq2_ecc_dbe_sts0 cvmx_pko_pse_sq2_ecc_dbe_sts0_t;

/**
 * cvmx_pko_pse_sq2_ecc_dbe_sts_cmb0
 */
union cvmx_pko_pse_sq2_ecc_dbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq2_ecc_dbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_sq2_dbe_cmb0             : 1;  /**< This bit is the logical OR of all bits in PKO_PSE_SQ2_ECC_DBE_STS.
                                                         To clear this bit, software must clear bits in PKO_PSE_SQ2_ECC_DBE_STS.
                                                         When this bit is set, the corresponding interrupt is set.
                                                         Throws PKO_INTSN_E::PKO_PSE_SQ2_DBE_CMB0.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.nt_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.pt_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.rt_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.tw_0.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.tw_1.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.tp0_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.tp1_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.std0_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.std1_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.sts0_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.sts1_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.wt_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.sc_sram */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_sq2_dbe_cmb0             : 1;
#endif
	} s;
	struct cvmx_pko_pse_sq2_ecc_dbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pko_pse_sq2_ecc_dbe_sts_cmb0 cvmx_pko_pse_sq2_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_sq2_ecc_sbe_sts0
 */
union cvmx_pko_pse_sq2_ecc_sbe_sts0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq2_ecc_sbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sq_pt_ram_sbe                : 1;  /**< Single-bit error for SQ_PT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.nt_sram */
	uint64_t sq_nt_ram_sbe                : 1;  /**< Single-bit error for SQ_NT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.pt_sram */
	uint64_t rt_ram_sbe                   : 1;  /**< Single-bit error for RT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.rt_sram */
	uint64_t tw1_cmd_fifo_ram_sbe         : 1;  /**< Single-bit error for TW1_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.tw_1.sq_fifo_sram */
	uint64_t tw0_cmd_fifo_ram_sbe         : 1;  /**< Single-bit error for TW0_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.tw_0.sq_fifo_sram */
	uint64_t tp1_sram_sbe                 : 1;  /**< Single-bit error for TP1_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.tp1_sram */
	uint64_t tp0_sram_sbe                 : 1;  /**< Single-bit error for TP0_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.tp0_sram */
	uint64_t sts1_ram_sbe                 : 1;  /**< Single-bit error for STS1_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.sts1_sram */
	uint64_t sts0_ram_sbe                 : 1;  /**< Single-bit error for STS0_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.sts0_sram */
	uint64_t std1_ram_sbe                 : 1;  /**< Single-bit error for STD1_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.std1_sram */
	uint64_t std0_ram_sbe                 : 1;  /**< Single-bit error for STD0_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.std0_sram */
	uint64_t wt_ram_sbe                   : 1;  /**< Single-bit error for WT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.wt_sram */
	uint64_t sc_ram_sbe                   : 1;  /**< Single-bit error for SC_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.sc_sram */
	uint64_t reserved_0_50                : 51;
#else
	uint64_t reserved_0_50                : 51;
	uint64_t sc_ram_sbe                   : 1;
	uint64_t wt_ram_sbe                   : 1;
	uint64_t std0_ram_sbe                 : 1;
	uint64_t std1_ram_sbe                 : 1;
	uint64_t sts0_ram_sbe                 : 1;
	uint64_t sts1_ram_sbe                 : 1;
	uint64_t tp0_sram_sbe                 : 1;
	uint64_t tp1_sram_sbe                 : 1;
	uint64_t tw0_cmd_fifo_ram_sbe         : 1;
	uint64_t tw1_cmd_fifo_ram_sbe         : 1;
	uint64_t rt_ram_sbe                   : 1;
	uint64_t sq_nt_ram_sbe                : 1;
	uint64_t sq_pt_ram_sbe                : 1;
#endif
	} s;
	struct cvmx_pko_pse_sq2_ecc_sbe_sts0_s cn78xx;
};
typedef union cvmx_pko_pse_sq2_ecc_sbe_sts0 cvmx_pko_pse_sq2_ecc_sbe_sts0_t;

/**
 * cvmx_pko_pse_sq2_ecc_sbe_sts_cmb0
 */
union cvmx_pko_pse_sq2_ecc_sbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq2_ecc_sbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_sq2_sbe_cmb0             : 1;  /**< This bit is the logical OR of all bits in PKO_PSE_SQ2_ECC_SBE_STS.
                                                         To clear this bit, software must clear bits in PKO_PSE_SQ2_ECC_SBE_STS.
                                                         When this bit is set, the corresponding interrupt is set.
                                                         Throws PKO_INTSN_E::PKO_PSE_SQ2_SBE_CMB0.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.nt_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.pt_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.rt_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.tw_0.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.tw_1.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.tp0_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.tp1_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.std0_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.std1_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.sts0_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq1.sts1_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.wt_sram
                                                         pko_pnr2.pko_pse.pse_sq2_pq.sq2.sc_sram */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_sq2_sbe_cmb0             : 1;
#endif
	} s;
	struct cvmx_pko_pse_sq2_ecc_sbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pko_pse_sq2_ecc_sbe_sts_cmb0 cvmx_pko_pse_sq2_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_sq3_bist_status
 *
 * Each bit is the BIST result of an individual memory (per bit, 0 = pass and 1 = fail).
 *
 */
union cvmx_pko_pse_sq3_bist_status {
	uint64_t u64;
	struct cvmx_pko_pse_sq3_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t sc_sram                      : 1;  /**< Scheduling configuration */
	uint64_t reserved_23_27               : 5;
	uint64_t tp3_sram                     : 1;  /**< SQ[5:3] topology parent configuration */
	uint64_t tp2_sram                     : 1;  /**< SQ[5:3] topology parent configuration */
	uint64_t tp1_sram                     : 1;  /**< SQ[5:1] topology parent configuration */
	uint64_t tp0_sram                     : 1;  /**< SQ[5:1] topology parent configuration */
	uint64_t reserved_18_18               : 1;
	uint64_t rt_sram                      : 1;  /**< Result table */
	uint64_t reserved_15_16               : 2;
	uint64_t tw3_cmd_fifo                 : 1;  /**< SQ[5:3] time wheel 3 command FIFO SRAM */
	uint64_t reserved_12_13               : 2;
	uint64_t tw2_cmd_fifo                 : 1;  /**< SQ[5:3] time wheel 2 command FIFO SRAM */
	uint64_t reserved_9_10                : 2;
	uint64_t tw1_cmd_fifo                 : 1;  /**< SQ[5:1] time wheel 1 command FIFO SRAM */
	uint64_t std_sram                     : 1;  /**< Dynamic shaping state */
	uint64_t sts_sram                     : 1;  /**< Static shaping configuration */
	uint64_t tw0_cmd_fifo                 : 1;  /**< SQ[5:1] time wheel 0 command FIFO SRAM */
	uint64_t reserved_3_4                 : 2;
	uint64_t nt_sram                      : 1;  /**< Next pointer table */
	uint64_t pt_sram                      : 1;  /**< Previous pointer table */
	uint64_t wt_sram                      : 1;  /**< Work table */
#else
	uint64_t wt_sram                      : 1;
	uint64_t pt_sram                      : 1;
	uint64_t nt_sram                      : 1;
	uint64_t reserved_3_4                 : 2;
	uint64_t tw0_cmd_fifo                 : 1;
	uint64_t sts_sram                     : 1;
	uint64_t std_sram                     : 1;
	uint64_t tw1_cmd_fifo                 : 1;
	uint64_t reserved_9_10                : 2;
	uint64_t tw2_cmd_fifo                 : 1;
	uint64_t reserved_12_13               : 2;
	uint64_t tw3_cmd_fifo                 : 1;
	uint64_t reserved_15_16               : 2;
	uint64_t rt_sram                      : 1;
	uint64_t reserved_18_18               : 1;
	uint64_t tp0_sram                     : 1;
	uint64_t tp1_sram                     : 1;
	uint64_t tp2_sram                     : 1;
	uint64_t tp3_sram                     : 1;
	uint64_t reserved_23_27               : 5;
	uint64_t sc_sram                      : 1;
	uint64_t reserved_29_63               : 35;
#endif
	} s;
	struct cvmx_pko_pse_sq3_bist_status_s cn78xx;
};
typedef union cvmx_pko_pse_sq3_bist_status cvmx_pko_pse_sq3_bist_status_t;

/**
 * cvmx_pko_pse_sq3_ecc_ctl0
 */
union cvmx_pko_pse_sq3_ecc_ctl0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq3_ecc_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sq_pt_ram_flip               : 2;  /**< SQ_PT_RAM flip syndrome bits on write. */
	uint64_t sq_pt_ram_cdis               : 1;  /**< SQ_PT_RAM ECC correction disable. */
	uint64_t sq_nt_ram_flip               : 2;  /**< SQ_NT_RAM flip syndrome bits on write. */
	uint64_t sq_nt_ram_cdis               : 1;  /**< SQ_NT_RAM ECC correction disable. */
	uint64_t rt_ram_flip                  : 2;  /**< RT_RAM flip syndrome bits on write. */
	uint64_t rt_ram_cdis                  : 1;  /**< RT_RAM ECC correction disable. */
	uint64_t tw3_cmd_fifo_ram_flip        : 2;  /**< TW3_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t tw3_cmd_fifo_ram_cdis        : 1;  /**< TW3_CMD_FIFO_RAM ECC correction disable. */
	uint64_t tw2_cmd_fifo_ram_flip        : 2;  /**< TW2_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t tw2_cmd_fifo_ram_cdis        : 1;  /**< TW2_CMD_FIFO_RAM ECC correction disable. */
	uint64_t tw1_cmd_fifo_ram_flip        : 2;  /**< TW1_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t tw1_cmd_fifo_ram_cdis        : 1;  /**< TW1_CMD_FIFO_RAM ECC correction disable. */
	uint64_t tw0_cmd_fifo_ram_flip        : 2;  /**< TW0_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t tw0_cmd_fifo_ram_cdis        : 1;  /**< TW0_CMD_FIFO_RAM ECC correction disable. */
	uint64_t tp3_sram_flip                : 2;  /**< TP3_SRAM flip syndrome bits on write. */
	uint64_t tp3_sram_cdis                : 1;  /**< TP3_SRAM ECC correction disable. */
	uint64_t tp2_sram_flip                : 2;  /**< TP2_SRAM flip syndrome bits on write. */
	uint64_t tp2_sram_cdis                : 1;  /**< TP2_SRAM ECC correction disable. */
	uint64_t tp1_sram_flip                : 2;  /**< TP1_SRAM flip syndrome bits on write. */
	uint64_t tp1_sram_cdis                : 1;  /**< TP1_SRAM ECC correction disable. */
	uint64_t tp0_sram_flip                : 2;  /**< TP0_SRAM flip syndrome bits on write. */
	uint64_t tp0_sram_cdis                : 1;  /**< TP0_SRAM ECC correction disable. */
	uint64_t sts3_ram_flip                : 2;  /**< STS3_RAM flip syndrome bits on write. */
	uint64_t sts3_ram_cdis                : 1;  /**< STS3_RAM ECC correction disable. */
	uint64_t sts2_ram_flip                : 2;  /**< STS2_RAM flip syndrome bits on write. */
	uint64_t sts2_ram_cdis                : 1;  /**< STS2_RAM ECC correction disable. */
	uint64_t sts1_ram_flip                : 2;  /**< STS1_RAM flip syndrome bits on write. */
	uint64_t sts1_ram_cdis                : 1;  /**< STS1_RAM ECC correction disable. */
	uint64_t sts0_ram_flip                : 2;  /**< STS0_RAM flip syndrome bits on write. */
	uint64_t sts0_ram_cdis                : 1;  /**< STS0_RAM ECC correction disable. */
	uint64_t std3_ram_flip                : 2;  /**< STD3_RAM flip syndrome bits on write. */
	uint64_t std3_ram_cdis                : 1;  /**< STD3_RAM ECC correction disable. */
	uint64_t std2_ram_flip                : 2;  /**< STD2_RAM flip syndrome bits on write. */
	uint64_t std2_ram_cdis                : 1;  /**< STD2_RAM ECC correction disable. */
	uint64_t std1_ram_flip                : 2;  /**< STD1_RAM flip syndrome bits on write. */
	uint64_t std1_ram_cdis                : 1;  /**< STD1_RAM ECC correction disable. */
	uint64_t std0_ram_flip                : 2;  /**< STD0_RAM flip syndrome bits on write. */
	uint64_t std0_ram_cdis                : 1;  /**< STD0_RAM ECC correction disable. */
	uint64_t wt_ram_flip                  : 2;  /**< WT_RAM flip syndrome bits on write. */
	uint64_t wt_ram_cdis                  : 1;  /**< WT_RAM ECC correction disable. */
	uint64_t sc_ram_flip                  : 2;  /**< SC_RAM flip syndrome bits on write. */
	uint64_t sc_ram_cdis                  : 1;  /**< SC_RAM ECC correction disable. */
	uint64_t reserved_0_0                 : 1;
#else
	uint64_t reserved_0_0                 : 1;
	uint64_t sc_ram_cdis                  : 1;
	uint64_t sc_ram_flip                  : 2;
	uint64_t wt_ram_cdis                  : 1;
	uint64_t wt_ram_flip                  : 2;
	uint64_t std0_ram_cdis                : 1;
	uint64_t std0_ram_flip                : 2;
	uint64_t std1_ram_cdis                : 1;
	uint64_t std1_ram_flip                : 2;
	uint64_t std2_ram_cdis                : 1;
	uint64_t std2_ram_flip                : 2;
	uint64_t std3_ram_cdis                : 1;
	uint64_t std3_ram_flip                : 2;
	uint64_t sts0_ram_cdis                : 1;
	uint64_t sts0_ram_flip                : 2;
	uint64_t sts1_ram_cdis                : 1;
	uint64_t sts1_ram_flip                : 2;
	uint64_t sts2_ram_cdis                : 1;
	uint64_t sts2_ram_flip                : 2;
	uint64_t sts3_ram_cdis                : 1;
	uint64_t sts3_ram_flip                : 2;
	uint64_t tp0_sram_cdis                : 1;
	uint64_t tp0_sram_flip                : 2;
	uint64_t tp1_sram_cdis                : 1;
	uint64_t tp1_sram_flip                : 2;
	uint64_t tp2_sram_cdis                : 1;
	uint64_t tp2_sram_flip                : 2;
	uint64_t tp3_sram_cdis                : 1;
	uint64_t tp3_sram_flip                : 2;
	uint64_t tw0_cmd_fifo_ram_cdis        : 1;
	uint64_t tw0_cmd_fifo_ram_flip        : 2;
	uint64_t tw1_cmd_fifo_ram_cdis        : 1;
	uint64_t tw1_cmd_fifo_ram_flip        : 2;
	uint64_t tw2_cmd_fifo_ram_cdis        : 1;
	uint64_t tw2_cmd_fifo_ram_flip        : 2;
	uint64_t tw3_cmd_fifo_ram_cdis        : 1;
	uint64_t tw3_cmd_fifo_ram_flip        : 2;
	uint64_t rt_ram_cdis                  : 1;
	uint64_t rt_ram_flip                  : 2;
	uint64_t sq_nt_ram_cdis               : 1;
	uint64_t sq_nt_ram_flip               : 2;
	uint64_t sq_pt_ram_cdis               : 1;
	uint64_t sq_pt_ram_flip               : 2;
#endif
	} s;
	struct cvmx_pko_pse_sq3_ecc_ctl0_s    cn78xx;
};
typedef union cvmx_pko_pse_sq3_ecc_ctl0 cvmx_pko_pse_sq3_ecc_ctl0_t;

/**
 * cvmx_pko_pse_sq3_ecc_dbe_sts0
 */
union cvmx_pko_pse_sq3_ecc_dbe_sts0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq3_ecc_dbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sq_pt_ram_dbe                : 1;  /**< Double-bit error for SQ_PT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.nt_sram */
	uint64_t sq_nt_ram_dbe                : 1;  /**< Double-bit error for SQ_NT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.pt_sram */
	uint64_t rt_ram_dbe                   : 1;  /**< Double-bit error for RT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.rt_sram */
	uint64_t tw3_cmd_fifo_ram_dbe         : 1;  /**< Double-bit error for TW3_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tw_3.sq_fifo_sram */
	uint64_t tw2_cmd_fifo_ram_dbe         : 1;  /**< Double-bit error for TW2_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tw_2.sq_fifo_sram */
	uint64_t tw1_cmd_fifo_ram_dbe         : 1;  /**< Double-bit error for TW1_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tw_1.sq_fifo_sram */
	uint64_t tw0_cmd_fifo_ram_dbe         : 1;  /**< Double-bit error for TW0_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tw_0.sq_fifo_sram */
	uint64_t tp3_sram_dbe                 : 1;  /**< Double-bit error for TP3_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tp3_sram */
	uint64_t tp2_sram_dbe                 : 1;  /**< Double-bit error for TP2_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tp2_sram */
	uint64_t tp1_sram_dbe                 : 1;  /**< Double-bit error for TP1_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tp1_sram */
	uint64_t tp0_sram_dbe                 : 1;  /**< Double-bit error for TP0_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tp0_sram */
	uint64_t sts3_ram_dbe                 : 1;  /**< Double-bit error for STS3_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.sts3_sram */
	uint64_t sts2_ram_dbe                 : 1;  /**< Double-bit error for STS2_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.sts2_sram */
	uint64_t sts1_ram_dbe                 : 1;  /**< Double-bit error for STS1_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.sts1_sram */
	uint64_t sts0_ram_dbe                 : 1;  /**< Double-bit error for STS0_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.sts0_sram */
	uint64_t std3_ram_dbe                 : 1;  /**< Double-bit error for STD3_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.std3_sram */
	uint64_t std2_ram_dbe                 : 1;  /**< Double-bit error for STD2_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.std2_sram */
	uint64_t std1_ram_dbe                 : 1;  /**< Double-bit error for STD1_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.std1_sram */
	uint64_t std0_ram_dbe                 : 1;  /**< Double-bit error for STD0_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.std0_sram */
	uint64_t wt_ram_dbe                   : 1;  /**< Double-bit error for WT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.wt_sram */
	uint64_t sc_ram_dbe                   : 1;  /**< Double-bit error for SC_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.sc_sram */
	uint64_t reserved_0_42                : 43;
#else
	uint64_t reserved_0_42                : 43;
	uint64_t sc_ram_dbe                   : 1;
	uint64_t wt_ram_dbe                   : 1;
	uint64_t std0_ram_dbe                 : 1;
	uint64_t std1_ram_dbe                 : 1;
	uint64_t std2_ram_dbe                 : 1;
	uint64_t std3_ram_dbe                 : 1;
	uint64_t sts0_ram_dbe                 : 1;
	uint64_t sts1_ram_dbe                 : 1;
	uint64_t sts2_ram_dbe                 : 1;
	uint64_t sts3_ram_dbe                 : 1;
	uint64_t tp0_sram_dbe                 : 1;
	uint64_t tp1_sram_dbe                 : 1;
	uint64_t tp2_sram_dbe                 : 1;
	uint64_t tp3_sram_dbe                 : 1;
	uint64_t tw0_cmd_fifo_ram_dbe         : 1;
	uint64_t tw1_cmd_fifo_ram_dbe         : 1;
	uint64_t tw2_cmd_fifo_ram_dbe         : 1;
	uint64_t tw3_cmd_fifo_ram_dbe         : 1;
	uint64_t rt_ram_dbe                   : 1;
	uint64_t sq_nt_ram_dbe                : 1;
	uint64_t sq_pt_ram_dbe                : 1;
#endif
	} s;
	struct cvmx_pko_pse_sq3_ecc_dbe_sts0_s cn78xx;
};
typedef union cvmx_pko_pse_sq3_ecc_dbe_sts0 cvmx_pko_pse_sq3_ecc_dbe_sts0_t;

/**
 * cvmx_pko_pse_sq3_ecc_dbe_sts_cmb0
 */
union cvmx_pko_pse_sq3_ecc_dbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq3_ecc_dbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_sq3_dbe_cmb0             : 1;  /**< This bit is the logical OR of all bits in PKO_PSE_SQ3_ECC_DBE_STS.
                                                         To clear this bit, software must clear bits in PKO_PSE_SQ3_ECC_DBE_STS.
                                                         When this bit is set, the corresponding interrupt is set.
                                                         Throws PKO_INTSN_E::PKO_PSE_SQ3_DBE_CMB0.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.nt_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.pt_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.rt_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tw_0.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tw_1.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tw_2.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tw_3.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tp0_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tp1_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tp2_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tp3_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.std0_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.std1_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.std2_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.std3_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.sts0_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.sts1_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.sts2_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.sts3_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.wt_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.sc_sram */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_sq3_dbe_cmb0             : 1;
#endif
	} s;
	struct cvmx_pko_pse_sq3_ecc_dbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pko_pse_sq3_ecc_dbe_sts_cmb0 cvmx_pko_pse_sq3_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_sq3_ecc_sbe_sts0
 */
union cvmx_pko_pse_sq3_ecc_sbe_sts0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq3_ecc_sbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sq_pt_ram_sbe                : 1;  /**< Single-bit error for SQ_PT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.nt_sram */
	uint64_t sq_nt_ram_sbe                : 1;  /**< Single-bit error for SQ_NT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.pt_sram */
	uint64_t rt_ram_sbe                   : 1;  /**< Single-bit error for RT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.rt_sram */
	uint64_t tw3_cmd_fifo_ram_sbe         : 1;  /**< Single-bit error for TW3_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tw_3.sq_fifo_sram */
	uint64_t tw2_cmd_fifo_ram_sbe         : 1;  /**< Single-bit error for TW2_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tw_2.sq_fifo_sram */
	uint64_t tw1_cmd_fifo_ram_sbe         : 1;  /**< Single-bit error for TW1_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tw_1.sq_fifo_sram */
	uint64_t tw0_cmd_fifo_ram_sbe         : 1;  /**< Single-bit error for TW0_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tw_0.sq_fifo_sram */
	uint64_t tp3_sram_sbe                 : 1;  /**< Single-bit error for TP3_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tp3_sram */
	uint64_t tp2_sram_sbe                 : 1;  /**< Single-bit error for TP2_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tp2_sram */
	uint64_t tp1_sram_sbe                 : 1;  /**< Single-bit error for TP1_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tp1_sram */
	uint64_t tp0_sram_sbe                 : 1;  /**< Single-bit error for TP0_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tp0_sram */
	uint64_t sts3_ram_sbe                 : 1;  /**< Single-bit error for STS3_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.sts3_sram */
	uint64_t sts2_ram_sbe                 : 1;  /**< Single-bit error for STS2_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.sts2_sram */
	uint64_t sts1_ram_sbe                 : 1;  /**< Single-bit error for STS1_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.sts1_sram */
	uint64_t sts0_ram_sbe                 : 1;  /**< Single-bit error for STS0_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.sts0_sram */
	uint64_t std3_ram_sbe                 : 1;  /**< Single-bit error for STD3_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.std3_sram */
	uint64_t std2_ram_sbe                 : 1;  /**< Single-bit error for STD2_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.std2_sram */
	uint64_t std1_ram_sbe                 : 1;  /**< Single-bit error for STD1_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.std1_sram */
	uint64_t std0_ram_sbe                 : 1;  /**< Single-bit error for STD0_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.std0_sram */
	uint64_t wt_ram_sbe                   : 1;  /**< Single-bit error for WT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.wt_sram */
	uint64_t sc_ram_sbe                   : 1;  /**< Single-bit error for SC_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.sc_sram */
	uint64_t reserved_0_42                : 43;
#else
	uint64_t reserved_0_42                : 43;
	uint64_t sc_ram_sbe                   : 1;
	uint64_t wt_ram_sbe                   : 1;
	uint64_t std0_ram_sbe                 : 1;
	uint64_t std1_ram_sbe                 : 1;
	uint64_t std2_ram_sbe                 : 1;
	uint64_t std3_ram_sbe                 : 1;
	uint64_t sts0_ram_sbe                 : 1;
	uint64_t sts1_ram_sbe                 : 1;
	uint64_t sts2_ram_sbe                 : 1;
	uint64_t sts3_ram_sbe                 : 1;
	uint64_t tp0_sram_sbe                 : 1;
	uint64_t tp1_sram_sbe                 : 1;
	uint64_t tp2_sram_sbe                 : 1;
	uint64_t tp3_sram_sbe                 : 1;
	uint64_t tw0_cmd_fifo_ram_sbe         : 1;
	uint64_t tw1_cmd_fifo_ram_sbe         : 1;
	uint64_t tw2_cmd_fifo_ram_sbe         : 1;
	uint64_t tw3_cmd_fifo_ram_sbe         : 1;
	uint64_t rt_ram_sbe                   : 1;
	uint64_t sq_nt_ram_sbe                : 1;
	uint64_t sq_pt_ram_sbe                : 1;
#endif
	} s;
	struct cvmx_pko_pse_sq3_ecc_sbe_sts0_s cn78xx;
};
typedef union cvmx_pko_pse_sq3_ecc_sbe_sts0 cvmx_pko_pse_sq3_ecc_sbe_sts0_t;

/**
 * cvmx_pko_pse_sq3_ecc_sbe_sts_cmb0
 */
union cvmx_pko_pse_sq3_ecc_sbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq3_ecc_sbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_sq3_sbe_cmb0             : 1;  /**< This bit is the logical OR of all bits in PKO_PSE_SQ3_ECC_SBE_STS.
                                                         To clear this bit, software must clear bits in PKO_PSE_SQ3_ECC_SBE_STS.
                                                         When this bit is set, the corresponding interrupt is set.
                                                         Throws PKO_INTSN_E::PKO_PSE_SQ3_SBE_CMB0.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.nt_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.pt_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.rt_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tw_0.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tw_1.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tw_2.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tw_3.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tp0_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tp1_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tp2_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tp3_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.std0_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.std1_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.std2_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.std3_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.sts0_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.sts1_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.sts2_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.sts3_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.wt_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.sc_sram */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_sq3_sbe_cmb0             : 1;
#endif
	} s;
	struct cvmx_pko_pse_sq3_ecc_sbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pko_pse_sq3_ecc_sbe_sts_cmb0 cvmx_pko_pse_sq3_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_sq4_bist_status
 *
 * Each bit is the BIST result of an individual memory (per bit, 0 = pass and 1 = fail).
 *
 */
union cvmx_pko_pse_sq4_bist_status {
	uint64_t u64;
	struct cvmx_pko_pse_sq4_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t sc_sram                      : 1;  /**< Scheduling configuration */
	uint64_t reserved_23_27               : 5;
	uint64_t tp3_sram                     : 1;  /**< SQ[5:3] topology parent configuration */
	uint64_t tp2_sram                     : 1;  /**< SQ[5:3] topology parent configuration */
	uint64_t tp1_sram                     : 1;  /**< SQ[5:1] topology parent configuration */
	uint64_t tp0_sram                     : 1;  /**< SQ[5:1] topology parent configuration */
	uint64_t reserved_18_18               : 1;
	uint64_t rt_sram                      : 1;  /**< Result Table */
	uint64_t reserved_15_16               : 2;
	uint64_t tw3_cmd_fifo                 : 1;  /**< SQ[5:3] time wheel 3 command FIFO SRAM */
	uint64_t reserved_12_13               : 2;
	uint64_t tw2_cmd_fifo                 : 1;  /**< SQ[5:3] time wheel 2 command FIFO SRAM. */
	uint64_t reserved_9_10                : 2;
	uint64_t tw1_cmd_fifo                 : 1;  /**< SQ[5:1] time wheel 1 command FIFO SRAM. */
	uint64_t std_sram                     : 1;  /**< Dynamic shaping state. */
	uint64_t sts_sram                     : 1;  /**< Static shaping configuration. */
	uint64_t tw0_cmd_fifo                 : 1;  /**< SQ[5:1] time wheel 0 command FIFO SRAM. */
	uint64_t reserved_3_4                 : 2;
	uint64_t nt_sram                      : 1;  /**< Next pointer table. */
	uint64_t pt_sram                      : 1;  /**< Previous pointer table. */
	uint64_t wt_sram                      : 1;  /**< Work table. */
#else
	uint64_t wt_sram                      : 1;
	uint64_t pt_sram                      : 1;
	uint64_t nt_sram                      : 1;
	uint64_t reserved_3_4                 : 2;
	uint64_t tw0_cmd_fifo                 : 1;
	uint64_t sts_sram                     : 1;
	uint64_t std_sram                     : 1;
	uint64_t tw1_cmd_fifo                 : 1;
	uint64_t reserved_9_10                : 2;
	uint64_t tw2_cmd_fifo                 : 1;
	uint64_t reserved_12_13               : 2;
	uint64_t tw3_cmd_fifo                 : 1;
	uint64_t reserved_15_16               : 2;
	uint64_t rt_sram                      : 1;
	uint64_t reserved_18_18               : 1;
	uint64_t tp0_sram                     : 1;
	uint64_t tp1_sram                     : 1;
	uint64_t tp2_sram                     : 1;
	uint64_t tp3_sram                     : 1;
	uint64_t reserved_23_27               : 5;
	uint64_t sc_sram                      : 1;
	uint64_t reserved_29_63               : 35;
#endif
	} s;
	struct cvmx_pko_pse_sq4_bist_status_s cn78xx;
};
typedef union cvmx_pko_pse_sq4_bist_status cvmx_pko_pse_sq4_bist_status_t;

/**
 * cvmx_pko_pse_sq4_ecc_ctl0
 */
union cvmx_pko_pse_sq4_ecc_ctl0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq4_ecc_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sq_pt_ram_flip               : 2;  /**< SQ_PT_RAM flip syndrome bits on write. */
	uint64_t sq_pt_ram_cdis               : 1;  /**< SQ_PT_RAM ECC correction disable. */
	uint64_t sq_nt_ram_flip               : 2;  /**< SQ_NT_RAM flip syndrome bits on write. */
	uint64_t sq_nt_ram_cdis               : 1;  /**< SQ_NT_RAM ECC correction disable. */
	uint64_t rt_ram_flip                  : 2;  /**< RT_RAM flip syndrome bits on write. */
	uint64_t rt_ram_cdis                  : 1;  /**< RT_RAM ECC correction disable. */
	uint64_t tw3_cmd_fifo_ram_flip        : 2;  /**< TW3_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t tw3_cmd_fifo_ram_cdis        : 1;  /**< TW3_CMD_FIFO_RAM ECC correction disable. */
	uint64_t tw2_cmd_fifo_ram_flip        : 2;  /**< TW2_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t tw2_cmd_fifo_ram_cdis        : 1;  /**< TW2_CMD_FIFO_RAM ECC correction disable. */
	uint64_t tw1_cmd_fifo_ram_flip        : 2;  /**< TW1_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t tw1_cmd_fifo_ram_cdis        : 1;  /**< TW1_CMD_FIFO_RAM ECC correction disable. */
	uint64_t tw0_cmd_fifo_ram_flip        : 2;  /**< TW0_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t tw0_cmd_fifo_ram_cdis        : 1;  /**< TW0_CMD_FIFO_RAM ECC correction disable. */
	uint64_t tp3_sram_flip                : 2;  /**< TP3_SRAM flip syndrome bits on write. */
	uint64_t tp3_sram_cdis                : 1;  /**< TP3_SRAM ECC correction disable. */
	uint64_t tp2_sram_flip                : 2;  /**< TP2_SRAM flip syndrome bits on write. */
	uint64_t tp2_sram_cdis                : 1;  /**< TP2_SRAM ECC correction disable. */
	uint64_t tp1_sram_flip                : 2;  /**< TP1_SRAM flip syndrome bits on write. */
	uint64_t tp1_sram_cdis                : 1;  /**< TP1_SRAM ECC correction disable. */
	uint64_t tp0_sram_flip                : 2;  /**< TP0_SRAM flip syndrome bits on write. */
	uint64_t tp0_sram_cdis                : 1;  /**< TP0_SRAM ECC correction disable. */
	uint64_t sts3_ram_flip                : 2;  /**< STS3_RAM flip syndrome bits on write. */
	uint64_t sts3_ram_cdis                : 1;  /**< STS3_RAM ECC correction disable. */
	uint64_t sts2_ram_flip                : 2;  /**< STS2_RAM flip syndrome bits on write. */
	uint64_t sts2_ram_cdis                : 1;  /**< STS2_RAM ECC correction disable. */
	uint64_t sts1_ram_flip                : 2;  /**< STS1_RAM flip syndrome bits on write. */
	uint64_t sts1_ram_cdis                : 1;  /**< STS1_RAM ECC correction disable. */
	uint64_t sts0_ram_flip                : 2;  /**< STS0_RAM flip syndrome bits on write. */
	uint64_t sts0_ram_cdis                : 1;  /**< STS0_RAM ECC correction disable. */
	uint64_t std3_ram_flip                : 2;  /**< STD3_RAM flip syndrome bits on write. */
	uint64_t std3_ram_cdis                : 1;  /**< STD3_RAM ECC correction disable. */
	uint64_t std2_ram_flip                : 2;  /**< STD2_RAM flip syndrome bits on write. */
	uint64_t std2_ram_cdis                : 1;  /**< STD2_RAM ECC correction disable. */
	uint64_t std1_ram_flip                : 2;  /**< STD1_RAM flip syndrome bits on write. */
	uint64_t std1_ram_cdis                : 1;  /**< STD1_RAM ECC correction disable. */
	uint64_t std0_ram_flip                : 2;  /**< STD0_RAM flip syndrome bits on write. */
	uint64_t std0_ram_cdis                : 1;  /**< STD0_RAM ECC correction disable. */
	uint64_t wt_ram_flip                  : 2;  /**< WT_RAM flip syndrome bits on write. */
	uint64_t wt_ram_cdis                  : 1;  /**< WT_RAM ECC correction disable. */
	uint64_t sc_ram_flip                  : 2;  /**< SC_RAM flip syndrome bits on write. */
	uint64_t sc_ram_cdis                  : 1;  /**< SC_RAM ECC correction disable. */
	uint64_t reserved_0_0                 : 1;
#else
	uint64_t reserved_0_0                 : 1;
	uint64_t sc_ram_cdis                  : 1;
	uint64_t sc_ram_flip                  : 2;
	uint64_t wt_ram_cdis                  : 1;
	uint64_t wt_ram_flip                  : 2;
	uint64_t std0_ram_cdis                : 1;
	uint64_t std0_ram_flip                : 2;
	uint64_t std1_ram_cdis                : 1;
	uint64_t std1_ram_flip                : 2;
	uint64_t std2_ram_cdis                : 1;
	uint64_t std2_ram_flip                : 2;
	uint64_t std3_ram_cdis                : 1;
	uint64_t std3_ram_flip                : 2;
	uint64_t sts0_ram_cdis                : 1;
	uint64_t sts0_ram_flip                : 2;
	uint64_t sts1_ram_cdis                : 1;
	uint64_t sts1_ram_flip                : 2;
	uint64_t sts2_ram_cdis                : 1;
	uint64_t sts2_ram_flip                : 2;
	uint64_t sts3_ram_cdis                : 1;
	uint64_t sts3_ram_flip                : 2;
	uint64_t tp0_sram_cdis                : 1;
	uint64_t tp0_sram_flip                : 2;
	uint64_t tp1_sram_cdis                : 1;
	uint64_t tp1_sram_flip                : 2;
	uint64_t tp2_sram_cdis                : 1;
	uint64_t tp2_sram_flip                : 2;
	uint64_t tp3_sram_cdis                : 1;
	uint64_t tp3_sram_flip                : 2;
	uint64_t tw0_cmd_fifo_ram_cdis        : 1;
	uint64_t tw0_cmd_fifo_ram_flip        : 2;
	uint64_t tw1_cmd_fifo_ram_cdis        : 1;
	uint64_t tw1_cmd_fifo_ram_flip        : 2;
	uint64_t tw2_cmd_fifo_ram_cdis        : 1;
	uint64_t tw2_cmd_fifo_ram_flip        : 2;
	uint64_t tw3_cmd_fifo_ram_cdis        : 1;
	uint64_t tw3_cmd_fifo_ram_flip        : 2;
	uint64_t rt_ram_cdis                  : 1;
	uint64_t rt_ram_flip                  : 2;
	uint64_t sq_nt_ram_cdis               : 1;
	uint64_t sq_nt_ram_flip               : 2;
	uint64_t sq_pt_ram_cdis               : 1;
	uint64_t sq_pt_ram_flip               : 2;
#endif
	} s;
	struct cvmx_pko_pse_sq4_ecc_ctl0_s    cn78xx;
};
typedef union cvmx_pko_pse_sq4_ecc_ctl0 cvmx_pko_pse_sq4_ecc_ctl0_t;

/**
 * cvmx_pko_pse_sq4_ecc_dbe_sts0
 */
union cvmx_pko_pse_sq4_ecc_dbe_sts0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq4_ecc_dbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sq_pt_ram_dbe                : 1;  /**< Double-bit error for SQ_PT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.pt_sram */
	uint64_t sq_nt_ram_dbe                : 1;  /**< Double-bit error for SQ_NT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.nt_sram */
	uint64_t rt_ram_dbe                   : 1;  /**< Double-bit error for RT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.rt_sram */
	uint64_t tw3_cmd_fifo_ram_dbe         : 1;  /**< Double-bit error for TW3_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tw_3.sq_fifo_sram */
	uint64_t tw2_cmd_fifo_ram_dbe         : 1;  /**< Double-bit error for TW2_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tw_2.sq_fifo_sram */
	uint64_t tw1_cmd_fifo_ram_dbe         : 1;  /**< Double-bit error for TW1_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tw_1.sq_fifo_sram */
	uint64_t tw0_cmd_fifo_ram_dbe         : 1;  /**< Double-bit error for TW0_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tw_0.sq_fifo_sram */
	uint64_t tp3_sram_dbe                 : 1;  /**< Double-bit error for TP3_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tp3_sram */
	uint64_t tp2_sram_dbe                 : 1;  /**< Double-bit error for TP2_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tp2_sram */
	uint64_t tp1_sram_dbe                 : 1;  /**< Double-bit error for TP1_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tp1_sram */
	uint64_t tp0_sram_dbe                 : 1;  /**< Double-bit error for TP0_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tp0_sram */
	uint64_t sts3_ram_dbe                 : 1;  /**< Double-bit error for STS3_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.sts3_sram */
	uint64_t sts2_ram_dbe                 : 1;  /**< Double-bit error for STS2_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.sts2_sram */
	uint64_t sts1_ram_dbe                 : 1;  /**< Double-bit error for STS1_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.sts1_sram */
	uint64_t sts0_ram_dbe                 : 1;  /**< Double-bit error for STS0_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.sts0_sram */
	uint64_t std3_ram_dbe                 : 1;  /**< Double-bit error for STD3_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.std3_sram */
	uint64_t std2_ram_dbe                 : 1;  /**< Double-bit error for STD2_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.std2_sram */
	uint64_t std1_ram_dbe                 : 1;  /**< Double-bit error for STD1_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.std1_sram */
	uint64_t std0_ram_dbe                 : 1;  /**< Double-bit error for STD0_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.std0_sram */
	uint64_t wt_ram_dbe                   : 1;  /**< Double-bit error for WT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.wt_sram */
	uint64_t sc_ram_dbe                   : 1;  /**< Double-bit error for SC_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.sc_sram */
	uint64_t reserved_0_42                : 43;
#else
	uint64_t reserved_0_42                : 43;
	uint64_t sc_ram_dbe                   : 1;
	uint64_t wt_ram_dbe                   : 1;
	uint64_t std0_ram_dbe                 : 1;
	uint64_t std1_ram_dbe                 : 1;
	uint64_t std2_ram_dbe                 : 1;
	uint64_t std3_ram_dbe                 : 1;
	uint64_t sts0_ram_dbe                 : 1;
	uint64_t sts1_ram_dbe                 : 1;
	uint64_t sts2_ram_dbe                 : 1;
	uint64_t sts3_ram_dbe                 : 1;
	uint64_t tp0_sram_dbe                 : 1;
	uint64_t tp1_sram_dbe                 : 1;
	uint64_t tp2_sram_dbe                 : 1;
	uint64_t tp3_sram_dbe                 : 1;
	uint64_t tw0_cmd_fifo_ram_dbe         : 1;
	uint64_t tw1_cmd_fifo_ram_dbe         : 1;
	uint64_t tw2_cmd_fifo_ram_dbe         : 1;
	uint64_t tw3_cmd_fifo_ram_dbe         : 1;
	uint64_t rt_ram_dbe                   : 1;
	uint64_t sq_nt_ram_dbe                : 1;
	uint64_t sq_pt_ram_dbe                : 1;
#endif
	} s;
	struct cvmx_pko_pse_sq4_ecc_dbe_sts0_s cn78xx;
};
typedef union cvmx_pko_pse_sq4_ecc_dbe_sts0 cvmx_pko_pse_sq4_ecc_dbe_sts0_t;

/**
 * cvmx_pko_pse_sq4_ecc_dbe_sts_cmb0
 */
union cvmx_pko_pse_sq4_ecc_dbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq4_ecc_dbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_sq4_dbe_cmb0             : 1;  /**< This bit is the logical OR of all bits in PKO_PSE_SQ4_ECC_DBE_STS.
                                                         To clear this bit, software must clear bits in PKO_PSE_SQ4_ECC_DBE_STS.
                                                         When this bit is set, the corresponding interrupt is set.
                                                         Throws PKO_INTSN_E::PKO_PSE_SQ4_DBE_CMB0.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.nt_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.pt_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.rt_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tw_0.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tw_1.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tw_2.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tw_3.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tp0_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tp1_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tp2_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tp3_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.std0_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.std1_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.std2_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.std3_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.sts0_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.sts1_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.sts2_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.sts3_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.wt_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.sc_sram */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_sq4_dbe_cmb0             : 1;
#endif
	} s;
	struct cvmx_pko_pse_sq4_ecc_dbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pko_pse_sq4_ecc_dbe_sts_cmb0 cvmx_pko_pse_sq4_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_sq4_ecc_sbe_sts0
 */
union cvmx_pko_pse_sq4_ecc_sbe_sts0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq4_ecc_sbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sq_pt_ram_sbe                : 1;  /**< Single-bit error for SQ_PT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.nt_sram */
	uint64_t sq_nt_ram_sbe                : 1;  /**< Single-bit error for SQ_NT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.pt_sram */
	uint64_t rt_ram_sbe                   : 1;  /**< Single-bit error for RT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.rt_sram */
	uint64_t tw3_cmd_fifo_ram_sbe         : 1;  /**< Single-bit error for TW3_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tw_3.sq_fifo_sram */
	uint64_t tw2_cmd_fifo_ram_sbe         : 1;  /**< Single-bit error for TW2_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tw_2.sq_fifo_sram */
	uint64_t tw1_cmd_fifo_ram_sbe         : 1;  /**< Single-bit error for TW1_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tw_1.sq_fifo_sram */
	uint64_t tw0_cmd_fifo_ram_sbe         : 1;  /**< Single-bit error for TW0_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tw_0.sq_fifo_sram */
	uint64_t tp3_sram_sbe                 : 1;  /**< Single-bit error for TP3_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tp3_sram */
	uint64_t tp2_sram_sbe                 : 1;  /**< Single-bit error for TP2_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tp2_sram */
	uint64_t tp1_sram_sbe                 : 1;  /**< Single-bit error for TP1_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tp1_sram */
	uint64_t tp0_sram_sbe                 : 1;  /**< Single-bit error for TP0_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tp0_sram */
	uint64_t sts3_ram_sbe                 : 1;  /**< Single-bit error for STS3_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.sts3_sram */
	uint64_t sts2_ram_sbe                 : 1;  /**< Single-bit error for STS2_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.sts2_sram */
	uint64_t sts1_ram_sbe                 : 1;  /**< Single-bit error for STS1_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.sts1_sram */
	uint64_t sts0_ram_sbe                 : 1;  /**< Single-bit error for STS0_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.sts0_sram */
	uint64_t std3_ram_sbe                 : 1;  /**< Single-bit error for STD3_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.std3_sram */
	uint64_t std2_ram_sbe                 : 1;  /**< Single-bit error for STD2_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.std2_sram */
	uint64_t std1_ram_sbe                 : 1;  /**< Single-bit error for STD1_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.std1_sram */
	uint64_t std0_ram_sbe                 : 1;  /**< Single-bit error for STD0_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.std0_sram */
	uint64_t wt_ram_sbe                   : 1;  /**< Single-bit error for WT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.wt_sram */
	uint64_t sc_ram_sbe                   : 1;  /**< Single-bit error for SC_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.sc_sram */
	uint64_t reserved_0_42                : 43;
#else
	uint64_t reserved_0_42                : 43;
	uint64_t sc_ram_sbe                   : 1;
	uint64_t wt_ram_sbe                   : 1;
	uint64_t std0_ram_sbe                 : 1;
	uint64_t std1_ram_sbe                 : 1;
	uint64_t std2_ram_sbe                 : 1;
	uint64_t std3_ram_sbe                 : 1;
	uint64_t sts0_ram_sbe                 : 1;
	uint64_t sts1_ram_sbe                 : 1;
	uint64_t sts2_ram_sbe                 : 1;
	uint64_t sts3_ram_sbe                 : 1;
	uint64_t tp0_sram_sbe                 : 1;
	uint64_t tp1_sram_sbe                 : 1;
	uint64_t tp2_sram_sbe                 : 1;
	uint64_t tp3_sram_sbe                 : 1;
	uint64_t tw0_cmd_fifo_ram_sbe         : 1;
	uint64_t tw1_cmd_fifo_ram_sbe         : 1;
	uint64_t tw2_cmd_fifo_ram_sbe         : 1;
	uint64_t tw3_cmd_fifo_ram_sbe         : 1;
	uint64_t rt_ram_sbe                   : 1;
	uint64_t sq_nt_ram_sbe                : 1;
	uint64_t sq_pt_ram_sbe                : 1;
#endif
	} s;
	struct cvmx_pko_pse_sq4_ecc_sbe_sts0_s cn78xx;
};
typedef union cvmx_pko_pse_sq4_ecc_sbe_sts0 cvmx_pko_pse_sq4_ecc_sbe_sts0_t;

/**
 * cvmx_pko_pse_sq4_ecc_sbe_sts_cmb0
 */
union cvmx_pko_pse_sq4_ecc_sbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq4_ecc_sbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_sq4_sbe_cmb0             : 1;  /**< This bit is the logical OR of all bits in PKO_PSE_SQ4_ECC_SBE_STS.
                                                         To clear this bit, software must clear bits in PKO_PSE_SQ4_ECC_SBE_STS.
                                                         When this bit is set, the corresponding interrupt is set.
                                                         Throws PKO_INTSN_E::PKO_PSE_SQ4_SBE_CMB0.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.nt_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.pt_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.rt_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tw_0.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tw_1.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tw_2.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tw_3.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tp0_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tp1_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tp2_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.tp3_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.std0_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.std1_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.std2_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.std3_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.sts0_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.sts1_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.sts2_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.sts3_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.wt_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.sc_sram */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_sq4_sbe_cmb0             : 1;
#endif
	} s;
	struct cvmx_pko_pse_sq4_ecc_sbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pko_pse_sq4_ecc_sbe_sts_cmb0 cvmx_pko_pse_sq4_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_sq5_bist_status
 *
 * Each bit is the BIST result of an individual memory (per bit, 0 = pass and 1 = fail).
 *
 */
union cvmx_pko_pse_sq5_bist_status {
	uint64_t u64;
	struct cvmx_pko_pse_sq5_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t sc_sram                      : 1;  /**< Scheduling configuration. */
	uint64_t reserved_23_27               : 5;
	uint64_t tp3_sram                     : 1;  /**< SQ[5:3] topology parent configuration. */
	uint64_t tp2_sram                     : 1;  /**< SQ[5:3] topology parent configuration. */
	uint64_t tp1_sram                     : 1;  /**< SQ[5:1] topology parent configuration. */
	uint64_t tp0_sram                     : 1;  /**< SQ[5:1] topology parent configuration. */
	uint64_t reserved_18_18               : 1;
	uint64_t rt_sram                      : 1;  /**< Result table. */
	uint64_t reserved_15_16               : 2;
	uint64_t tw3_cmd_fifo                 : 1;  /**< SQ[5:3] time wheel 3 command FIFO SRAM. */
	uint64_t reserved_12_13               : 2;
	uint64_t tw2_cmd_fifo                 : 1;  /**< SQ[5:3] time wheel 2 command FIFO SRAM. */
	uint64_t reserved_9_10                : 2;
	uint64_t tw1_cmd_fifo                 : 1;  /**< SQ[5:1] time wheel 1 command FIFO SRAM. */
	uint64_t std_sram                     : 1;  /**< Dynamic shaping state. */
	uint64_t sts_sram                     : 1;  /**< Static shaping configuration. */
	uint64_t tw0_cmd_fifo                 : 1;  /**< SQ[5:1] time wheel 0 command FIFO SRAM. */
	uint64_t reserved_3_4                 : 2;
	uint64_t nt_sram                      : 1;  /**< Next pointer table. */
	uint64_t pt_sram                      : 1;  /**< Previous pointer table. */
	uint64_t wt_sram                      : 1;  /**< Work table. */
#else
	uint64_t wt_sram                      : 1;
	uint64_t pt_sram                      : 1;
	uint64_t nt_sram                      : 1;
	uint64_t reserved_3_4                 : 2;
	uint64_t tw0_cmd_fifo                 : 1;
	uint64_t sts_sram                     : 1;
	uint64_t std_sram                     : 1;
	uint64_t tw1_cmd_fifo                 : 1;
	uint64_t reserved_9_10                : 2;
	uint64_t tw2_cmd_fifo                 : 1;
	uint64_t reserved_12_13               : 2;
	uint64_t tw3_cmd_fifo                 : 1;
	uint64_t reserved_15_16               : 2;
	uint64_t rt_sram                      : 1;
	uint64_t reserved_18_18               : 1;
	uint64_t tp0_sram                     : 1;
	uint64_t tp1_sram                     : 1;
	uint64_t tp2_sram                     : 1;
	uint64_t tp3_sram                     : 1;
	uint64_t reserved_23_27               : 5;
	uint64_t sc_sram                      : 1;
	uint64_t reserved_29_63               : 35;
#endif
	} s;
	struct cvmx_pko_pse_sq5_bist_status_s cn78xx;
};
typedef union cvmx_pko_pse_sq5_bist_status cvmx_pko_pse_sq5_bist_status_t;

/**
 * cvmx_pko_pse_sq5_ecc_ctl0
 */
union cvmx_pko_pse_sq5_ecc_ctl0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq5_ecc_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sq_pt_ram_flip               : 2;  /**< SQ_PT_RAM flip syndrome bits on write. */
	uint64_t sq_pt_ram_cdis               : 1;  /**< SQ_PT_RAM ECC correction disable. */
	uint64_t sq_nt_ram_flip               : 2;  /**< SQ_NT_RAM flip syndrome bits on write. */
	uint64_t sq_nt_ram_cdis               : 1;  /**< SQ_NT_RAM ECC correction disable. */
	uint64_t rt_ram_flip                  : 2;  /**< RT_RAM flip syndrome bits on write. */
	uint64_t rt_ram_cdis                  : 1;  /**< RT_RAM ECC correction disable. */
	uint64_t tw3_cmd_fifo_ram_flip        : 2;  /**< TW3_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t tw3_cmd_fifo_ram_cdis        : 1;  /**< TW3_CMD_FIFO_RAM ECC correction disable. */
	uint64_t tw2_cmd_fifo_ram_flip        : 2;  /**< TW2_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t tw2_cmd_fifo_ram_cdis        : 1;  /**< TW2_CMD_FIFO_RAM ECC correction disable. */
	uint64_t tw1_cmd_fifo_ram_flip        : 2;  /**< TW1_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t tw1_cmd_fifo_ram_cdis        : 1;  /**< TW1_CMD_FIFO_RAM ECC correction disable. */
	uint64_t tw0_cmd_fifo_ram_flip        : 2;  /**< TW0_CMD_FIFO_RAM flip syndrome bits on write. */
	uint64_t tw0_cmd_fifo_ram_cdis        : 1;  /**< TW0_CMD_FIFO_RAM ECC correction disable. */
	uint64_t tp3_sram_flip                : 2;  /**< TP3_SRAM flip syndrome bits on write. */
	uint64_t tp3_sram_cdis                : 1;  /**< TP3_SRAM ECC correction disable. */
	uint64_t tp2_sram_flip                : 2;  /**< TP2_SRAM flip syndrome bits on write. */
	uint64_t tp2_sram_cdis                : 1;  /**< TP2_SRAM ECC correction disable. */
	uint64_t tp1_sram_flip                : 2;  /**< TP1_SRAM flip syndrome bits on write. */
	uint64_t tp1_sram_cdis                : 1;  /**< TP1_SRAM ECC correction disable. */
	uint64_t tp0_sram_flip                : 2;  /**< TP0_SRAM flip syndrome bits on write. */
	uint64_t tp0_sram_cdis                : 1;  /**< TP0_SRAM ECC correction disable. */
	uint64_t sts3_ram_flip                : 2;  /**< STS3_RAM flip syndrome bits on write. */
	uint64_t sts3_ram_cdis                : 1;  /**< STS3_RAM ECC correction disable. */
	uint64_t sts2_ram_flip                : 2;  /**< STS2_RAM flip syndrome bits on write. */
	uint64_t sts2_ram_cdis                : 1;  /**< STS2_RAM ECC correction disable. */
	uint64_t sts1_ram_flip                : 2;  /**< STS1_RAM flip syndrome bits on write. */
	uint64_t sts1_ram_cdis                : 1;  /**< STS1_RAM ECC correction disable. */
	uint64_t sts0_ram_flip                : 2;  /**< STS0_RAM flip syndrome bits on write. */
	uint64_t sts0_ram_cdis                : 1;  /**< STS0_RAM ECC correction disable. */
	uint64_t std3_ram_flip                : 2;  /**< STD3_RAM flip syndrome bits on write. */
	uint64_t std3_ram_cdis                : 1;  /**< STD3_RAM ECC correction disable. */
	uint64_t std2_ram_flip                : 2;  /**< STD2_RAM flip syndrome bits on write. */
	uint64_t std2_ram_cdis                : 1;  /**< STD2_RAM ECC correction disable. */
	uint64_t std1_ram_flip                : 2;  /**< STD1_RAM flip syndrome bits on write. */
	uint64_t std1_ram_cdis                : 1;  /**< STD1_RAM ECC correction disable. */
	uint64_t std0_ram_flip                : 2;  /**< STD0_RAM flip syndrome bits on write. */
	uint64_t std0_ram_cdis                : 1;  /**< STD0_RAM ECC correction disable. */
	uint64_t wt_ram_flip                  : 2;  /**< WT_RAM flip syndrome bits on write. */
	uint64_t wt_ram_cdis                  : 1;  /**< WT_RAM ECC correction disable. */
	uint64_t sc_ram_flip                  : 2;  /**< SC_RAM flip syndrome bits on write. */
	uint64_t sc_ram_cdis                  : 1;  /**< SC_RAM ECC correction disable. */
	uint64_t reserved_0_0                 : 1;
#else
	uint64_t reserved_0_0                 : 1;
	uint64_t sc_ram_cdis                  : 1;
	uint64_t sc_ram_flip                  : 2;
	uint64_t wt_ram_cdis                  : 1;
	uint64_t wt_ram_flip                  : 2;
	uint64_t std0_ram_cdis                : 1;
	uint64_t std0_ram_flip                : 2;
	uint64_t std1_ram_cdis                : 1;
	uint64_t std1_ram_flip                : 2;
	uint64_t std2_ram_cdis                : 1;
	uint64_t std2_ram_flip                : 2;
	uint64_t std3_ram_cdis                : 1;
	uint64_t std3_ram_flip                : 2;
	uint64_t sts0_ram_cdis                : 1;
	uint64_t sts0_ram_flip                : 2;
	uint64_t sts1_ram_cdis                : 1;
	uint64_t sts1_ram_flip                : 2;
	uint64_t sts2_ram_cdis                : 1;
	uint64_t sts2_ram_flip                : 2;
	uint64_t sts3_ram_cdis                : 1;
	uint64_t sts3_ram_flip                : 2;
	uint64_t tp0_sram_cdis                : 1;
	uint64_t tp0_sram_flip                : 2;
	uint64_t tp1_sram_cdis                : 1;
	uint64_t tp1_sram_flip                : 2;
	uint64_t tp2_sram_cdis                : 1;
	uint64_t tp2_sram_flip                : 2;
	uint64_t tp3_sram_cdis                : 1;
	uint64_t tp3_sram_flip                : 2;
	uint64_t tw0_cmd_fifo_ram_cdis        : 1;
	uint64_t tw0_cmd_fifo_ram_flip        : 2;
	uint64_t tw1_cmd_fifo_ram_cdis        : 1;
	uint64_t tw1_cmd_fifo_ram_flip        : 2;
	uint64_t tw2_cmd_fifo_ram_cdis        : 1;
	uint64_t tw2_cmd_fifo_ram_flip        : 2;
	uint64_t tw3_cmd_fifo_ram_cdis        : 1;
	uint64_t tw3_cmd_fifo_ram_flip        : 2;
	uint64_t rt_ram_cdis                  : 1;
	uint64_t rt_ram_flip                  : 2;
	uint64_t sq_nt_ram_cdis               : 1;
	uint64_t sq_nt_ram_flip               : 2;
	uint64_t sq_pt_ram_cdis               : 1;
	uint64_t sq_pt_ram_flip               : 2;
#endif
	} s;
	struct cvmx_pko_pse_sq5_ecc_ctl0_s    cn78xx;
};
typedef union cvmx_pko_pse_sq5_ecc_ctl0 cvmx_pko_pse_sq5_ecc_ctl0_t;

/**
 * cvmx_pko_pse_sq5_ecc_dbe_sts0
 */
union cvmx_pko_pse_sq5_ecc_dbe_sts0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq5_ecc_dbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sq_pt_ram_dbe                : 1;  /**< Double-bit error for SQ_PT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.pt_sram */
	uint64_t sq_nt_ram_dbe                : 1;  /**< Double-bit error for SQ_NT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.nt_sram */
	uint64_t rt_ram_dbe                   : 1;  /**< Double-bit error for RT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.rt_sram */
	uint64_t tw3_cmd_fifo_ram_dbe         : 1;  /**< Double-bit error for TW3_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tw_3.sq_fifo_sram */
	uint64_t tw2_cmd_fifo_ram_dbe         : 1;  /**< Double-bit error for TW2_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tw_2.sq_fifo_sram */
	uint64_t tw1_cmd_fifo_ram_dbe         : 1;  /**< Double-bit error for TW1_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tw_1.sq_fifo_sram */
	uint64_t tw0_cmd_fifo_ram_dbe         : 1;  /**< Double-bit error for TW0_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tw_0.sq_fifo_sram */
	uint64_t tp3_sram_dbe                 : 1;  /**< Double-bit error for TP3_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tp3_sram */
	uint64_t tp2_sram_dbe                 : 1;  /**< Double-bit error for TP2_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tp2_sram */
	uint64_t tp1_sram_dbe                 : 1;  /**< Double-bit error for TP1_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tp1_sram */
	uint64_t tp0_sram_dbe                 : 1;  /**< Double-bit error for TP0_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tp0_sram */
	uint64_t sts3_ram_dbe                 : 1;  /**< Double-bit error for STS3_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.sts3_sram */
	uint64_t sts2_ram_dbe                 : 1;  /**< Double-bit error for STS2_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.sts2_sram */
	uint64_t sts1_ram_dbe                 : 1;  /**< Double-bit error for STS1_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.sts1_sram */
	uint64_t sts0_ram_dbe                 : 1;  /**< Double-bit error for STS0_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.sts0_sram */
	uint64_t std3_ram_dbe                 : 1;  /**< Double-bit error for STD3_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.std3_sram */
	uint64_t std2_ram_dbe                 : 1;  /**< Double-bit error for STD2_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.std2_sram */
	uint64_t std1_ram_dbe                 : 1;  /**< Double-bit error for STD1_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.std1_sram */
	uint64_t std0_ram_dbe                 : 1;  /**< Double-bit error for STD0_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.std0_sram */
	uint64_t wt_ram_dbe                   : 1;  /**< Double-bit error for WT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.wt_sram */
	uint64_t sc_ram_dbe                   : 1;  /**< Double-bit error for SC_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.sc_sram */
	uint64_t reserved_0_42                : 43;
#else
	uint64_t reserved_0_42                : 43;
	uint64_t sc_ram_dbe                   : 1;
	uint64_t wt_ram_dbe                   : 1;
	uint64_t std0_ram_dbe                 : 1;
	uint64_t std1_ram_dbe                 : 1;
	uint64_t std2_ram_dbe                 : 1;
	uint64_t std3_ram_dbe                 : 1;
	uint64_t sts0_ram_dbe                 : 1;
	uint64_t sts1_ram_dbe                 : 1;
	uint64_t sts2_ram_dbe                 : 1;
	uint64_t sts3_ram_dbe                 : 1;
	uint64_t tp0_sram_dbe                 : 1;
	uint64_t tp1_sram_dbe                 : 1;
	uint64_t tp2_sram_dbe                 : 1;
	uint64_t tp3_sram_dbe                 : 1;
	uint64_t tw0_cmd_fifo_ram_dbe         : 1;
	uint64_t tw1_cmd_fifo_ram_dbe         : 1;
	uint64_t tw2_cmd_fifo_ram_dbe         : 1;
	uint64_t tw3_cmd_fifo_ram_dbe         : 1;
	uint64_t rt_ram_dbe                   : 1;
	uint64_t sq_nt_ram_dbe                : 1;
	uint64_t sq_pt_ram_dbe                : 1;
#endif
	} s;
	struct cvmx_pko_pse_sq5_ecc_dbe_sts0_s cn78xx;
};
typedef union cvmx_pko_pse_sq5_ecc_dbe_sts0 cvmx_pko_pse_sq5_ecc_dbe_sts0_t;

/**
 * cvmx_pko_pse_sq5_ecc_dbe_sts_cmb0
 */
union cvmx_pko_pse_sq5_ecc_dbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq5_ecc_dbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_sq5_dbe_cmb0             : 1;  /**< This bit is the logical OR of all bits in PKO_PSE_SQ5_ECC_DBE_STS.
                                                         To clear this bit, software must clear bits in PKO_PSE_SQ5_ECC_DBE_STS.
                                                         When this bit is set, the corresponding interrupt is set.
                                                         Throws PKO_INTSN_E::PKO_PSE_SQ5_DBE_CMB0.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.nt_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.pt_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.rt_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tw_0.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tw_1.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tw_2.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tw_3.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tp0_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tp1_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tp2_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tp3_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.std0_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.std1_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.std2_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.std3_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.sts0_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.sts1_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.sts2_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.sts3_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.wt_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.sc_sram */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_sq5_dbe_cmb0             : 1;
#endif
	} s;
	struct cvmx_pko_pse_sq5_ecc_dbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pko_pse_sq5_ecc_dbe_sts_cmb0 cvmx_pko_pse_sq5_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pko_pse_sq5_ecc_sbe_sts0
 */
union cvmx_pko_pse_sq5_ecc_sbe_sts0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq5_ecc_sbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sq_pt_ram_sbe                : 1;  /**< Single-bit error for SQ_PT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.pt_sram */
	uint64_t sq_nt_ram_sbe                : 1;  /**< Single-bit error for SQ_NT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.nt_sram */
	uint64_t rt_ram_sbe                   : 1;  /**< Single-bit error for RT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.rt_sram */
	uint64_t tw3_cmd_fifo_ram_sbe         : 1;  /**< Single-bit error for TW3_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tw_3.sq_fifo_sram */
	uint64_t tw2_cmd_fifo_ram_sbe         : 1;  /**< Single-bit error for TW2_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tw_2.sq_fifo_sram */
	uint64_t tw1_cmd_fifo_ram_sbe         : 1;  /**< Single-bit error for TW1_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tw_1.sq_fifo_sram */
	uint64_t tw0_cmd_fifo_ram_sbe         : 1;  /**< Single-bit error for TW0_CMD_FIFO_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tw_0.sq_fifo_sram */
	uint64_t tp3_sram_sbe                 : 1;  /**< Single-bit error for TP3_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tp3_sram */
	uint64_t tp2_sram_sbe                 : 1;  /**< Single-bit error for TP2_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tp2_sram */
	uint64_t tp1_sram_sbe                 : 1;  /**< Single-bit error for TP1_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tp1_sram */
	uint64_t tp0_sram_sbe                 : 1;  /**< Single-bit error for TP0_SRAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tp0_sram */
	uint64_t sts3_ram_sbe                 : 1;  /**< Single-bit error for STS3_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.sts3_sram */
	uint64_t sts2_ram_sbe                 : 1;  /**< Single-bit error for STS2_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.sts2_sram */
	uint64_t sts1_ram_sbe                 : 1;  /**< Single-bit error for STS1_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.sts1_sram */
	uint64_t sts0_ram_sbe                 : 1;  /**< Single-bit error for STS0_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.sts0_sram */
	uint64_t std3_ram_sbe                 : 1;  /**< Single-bit error for STD3_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.std3_sram */
	uint64_t std2_ram_sbe                 : 1;  /**< Single-bit error for STD2_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.std2_sram */
	uint64_t std1_ram_sbe                 : 1;  /**< Single-bit error for STD1_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.std1_sram */
	uint64_t std0_ram_sbe                 : 1;  /**< Single-bit error for STD0_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.std0_sram */
	uint64_t wt_ram_sbe                   : 1;  /**< Single-bit error for WT_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.wt_sram */
	uint64_t sc_ram_sbe                   : 1;  /**< Single-bit error for SC_RAM.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.sc_sram */
	uint64_t reserved_0_42                : 43;
#else
	uint64_t reserved_0_42                : 43;
	uint64_t sc_ram_sbe                   : 1;
	uint64_t wt_ram_sbe                   : 1;
	uint64_t std0_ram_sbe                 : 1;
	uint64_t std1_ram_sbe                 : 1;
	uint64_t std2_ram_sbe                 : 1;
	uint64_t std3_ram_sbe                 : 1;
	uint64_t sts0_ram_sbe                 : 1;
	uint64_t sts1_ram_sbe                 : 1;
	uint64_t sts2_ram_sbe                 : 1;
	uint64_t sts3_ram_sbe                 : 1;
	uint64_t tp0_sram_sbe                 : 1;
	uint64_t tp1_sram_sbe                 : 1;
	uint64_t tp2_sram_sbe                 : 1;
	uint64_t tp3_sram_sbe                 : 1;
	uint64_t tw0_cmd_fifo_ram_sbe         : 1;
	uint64_t tw1_cmd_fifo_ram_sbe         : 1;
	uint64_t tw2_cmd_fifo_ram_sbe         : 1;
	uint64_t tw3_cmd_fifo_ram_sbe         : 1;
	uint64_t rt_ram_sbe                   : 1;
	uint64_t sq_nt_ram_sbe                : 1;
	uint64_t sq_pt_ram_sbe                : 1;
#endif
	} s;
	struct cvmx_pko_pse_sq5_ecc_sbe_sts0_s cn78xx;
};
typedef union cvmx_pko_pse_sq5_ecc_sbe_sts0 cvmx_pko_pse_sq5_ecc_sbe_sts0_t;

/**
 * cvmx_pko_pse_sq5_ecc_sbe_sts_cmb0
 */
union cvmx_pko_pse_sq5_ecc_sbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pko_pse_sq5_ecc_sbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pse_sq5_sbe_cmb0             : 1;  /**< This bit is the logical OR of all bits in PKO_PSE_SQ5_ECC_SBE_STS.
                                                         To clear this bit, software must clear bits in PKO_PSE_SQ5_ECC_SBE_STS.
                                                         When this bit is set, the corresponding interrupt is set.
                                                         Throws PKO_INTSN_E::PKO_PSE_SQ5_SBE_CMB0.
                                                         INTERNAL: Instances:
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.nt_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.pt_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.rt_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tw_0.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tw_1.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tw_2.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.tw_3.sq_fifo_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tp0_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tp1_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tp2_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq4.tp3_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.std0_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.std1_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.std2_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.std3_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.sts0_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.sts1_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.sts2_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq3.sts3_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.wt_sram
                                                         pko_pnr2.pko_pse.pse_sq5_sq3.sq5.sc_sram */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pse_sq5_sbe_cmb0             : 1;
#endif
	} s;
	struct cvmx_pko_pse_sq5_ecc_sbe_sts_cmb0_s cn78xx;
};
typedef union cvmx_pko_pse_sq5_ecc_sbe_sts_cmb0 cvmx_pko_pse_sq5_ecc_sbe_sts_cmb0_t;

/**
 * cvmx_pko_ptf#_status
 */
union cvmx_pko_ptfx_status {
	uint64_t u64;
	struct cvmx_pko_ptfx_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t total_in_flight_cnt          : 8;  /**< Total number of packets currently in-flight within PEB.  Useful
                                                         both for reconfiguration (able to disable a FIFO when it is empty) and debugging. */
	uint64_t in_flight_cnt                : 7;  /**< Number of packets currently in-flight within PEB for this link.
                                                         Useful both for reconfiguration (able to disable a FIFO when it is empty) and debugging. */
	uint64_t mac_num                      : 5;  /**< MAC assigned to the given PKO TX FIFO. A value of 0x1F means unassigned. These
                                                         register values are derived automatically by the hardware from the
                                                         PKO_MAC(0..27)_CFG[FIFO_NUM] settings. */
#else
	uint64_t mac_num                      : 5;
	uint64_t in_flight_cnt                : 7;
	uint64_t total_in_flight_cnt          : 8;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_pko_ptfx_status_s         cn78xx;
};
typedef union cvmx_pko_ptfx_status cvmx_pko_ptfx_status_t;

/**
 * cvmx_pko_ptf_iobp_cfg
 */
union cvmx_pko_ptf_iobp_cfg {
	uint64_t u64;
	struct cvmx_pko_ptf_iobp_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_43_63               : 21;
	uint64_t iobp0_l2_allocate            : 1;  /**< Determine L2 allocation (1 = no allocation) when performing IOBP0 requests. */
	uint64_t iobp1_magic_addr             : 35; /**< IOBP1 read address to be used for any dummy reads */
	uint64_t max_read_size                : 7;  /**< Maximum number of IOBP1 read requests outstanding to be allowed by any given PEB TX FIFO. */
#else
	uint64_t max_read_size                : 7;
	uint64_t iobp1_magic_addr             : 35;
	uint64_t iobp0_l2_allocate            : 1;
	uint64_t reserved_43_63               : 21;
#endif
	} s;
	struct cvmx_pko_ptf_iobp_cfg_s        cn78xx;
};
typedef union cvmx_pko_ptf_iobp_cfg cvmx_pko_ptf_iobp_cfg_t;

/**
 * cvmx_pko_ptgf#_cfg
 *
 * This register configurations specific to the PKO TX FIFO groups (clusters of four PKO TX FIFOs).
 *
 */
union cvmx_pko_ptgfx_cfg {
	uint64_t u64;
	struct cvmx_pko_ptgfx_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t reset                        : 1;  /**< This bit resets the address pointers for the FIFOs in this group. This should only be
                                                         performed when a PTGF is empty and the SIZE field is to be being changed. */
	uint64_t rate                         : 3;  /**< Each PTGF can support up to 100Gbs. The total aggregate rate across all FIFOs (including
                                                         the NULL) should never exceed 250Gbs.
                                                         This field represents the rate for each active FIFO in PEB; thus the calculation for
                                                         throughput is a function of the SIZE field and whether or not the FIFO is assigned to a
                                                         MAC in PKO_MACx_CFG.
                                                         RATE: Throughput
                                                         ----------------
                                                         - 000:    6.25Gbs
                                                         - 001:   12.5 Gbs
                                                         - 010:   25   Gbs
                                                         - 011:   50   Gbs
                                                         - 100:  100   Gbs
                                                         Note: 101-111 are illegal RATE values and should not be used. */
	uint64_t size                         : 3;  /**< "PKO supports up to 29 independent TX FIFOs where 0-27 are physical and 28 is virtual. The
                                                         FIFOs are grouped into 8 sets of four contiguously numbered queues where each FIFO has a
                                                         base storage amount of 2.5K bytes of buffering.
                                                         PKO_PTGF(0)_CFG -> FIFO#  0-3
                                                         PKO_PTGF(1)_CFG -> FIFO#  4-7
                                                         PKO_PTGF(2)_CFG -> FIFO#  8-11
                                                         PKO_PTGF(3)_CFG -> FIFO#  12-15
                                                         PKO_PTGF(4)_CFG -> FIFO#  16-19
                                                         PKO_PTGF(5)_CFG -> FIFO#  20-23
                                                         PKO_PTGF(6)_CFG -> FIFO#  24-27
                                                         PKO_PTGF(7)_CFG -> FIFO#  28   (Virtual/NULL)
                                                         Within each set, 2 or 4 FIFOs can be combined to produce a larger FIFO if desired.
                                                         The SIZE field is used to configure the number and depth of the FIFOs in a set. The
                                                         supported options for a FIFO set are as follows:
                                                         SIZE: Set of 4 Contiguously Numbered FIFOs
                                                         ------------------------------------------
                                                         xxx      Queue0  Queue1  Queue2  Queue3
                                                         000 :     2.5k    2.5k    2.5k    2.5k
                                                         001 :     5.0k    0.0k    2.5k    2.5k
                                                         010 :     2.5k    2.5k    5.0k    0.0k
                                                         011 :     5.0k    0.0k    5.0k    0.0k
                                                         100 :    10.0k    0.0k    0.0k    0.0k
                                                         Note: 101-111 are illegal SIZE values and should not be used.
                                                         Note that when a FIFO is set to a size of 0K bytes, FIFO_NUM is no longer legal and cannot
                                                         be assigned to an active MAC. For example, for the set of FIFOs 8-11, if the
                                                         PKO_PTGF(2)_CFG[SIZE] = 3'b100, then FIFO_NUMs 9, 10 and 11 are no longer valid. Only
                                                         FIFO_NUM = 8 is available from this set for assignment to a MAC because all of the 10
                                                         Kbytes of buffering was configured to FIFO#8.
                                                         FIFO_NUM = 28 is a virtual FIFO and is used exclusively to indicate the NULL FIFO. Packets
                                                         targeting the NULL FIFO are dropped by PKO and their buffers returned to the FPA. The SIZE
                                                         field for PKO_PTGF(7) should always be set to zero.
                                                         Modifications to this field require two writes. The first write must assert
                                                         PKO_PTGFx_CFG[RESET] to reset the address pointers for the FIFOS in this group. The second
                                                         write clears the RESET bit as well as configures the new SIZE values." */
#else
	uint64_t size                         : 3;
	uint64_t rate                         : 3;
	uint64_t reset                        : 1;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_pko_ptgfx_cfg_s           cn78xx;
};
typedef union cvmx_pko_ptgfx_cfg cvmx_pko_ptgfx_cfg_t;

/**
 * cvmx_pko_reg_bist_result
 *
 * Notes:
 * Access to the internal BiST results
 * Each bit is the BiST result of an individual memory (per bit, 0=pass and 1=fail).
 */
union cvmx_pko_reg_bist_result {
	uint64_t u64;
	struct cvmx_pko_reg_bist_result_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_pko_reg_bist_result_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_27_63               : 37;
	uint64_t psb2                         : 5;  /**< BiST result of the PSB   memories (0=pass, !0=fail) */
	uint64_t count                        : 1;  /**< BiST result of the COUNT memories (0=pass, !0=fail) */
	uint64_t rif                          : 1;  /**< BiST result of the RIF   memories (0=pass, !0=fail) */
	uint64_t wif                          : 1;  /**< BiST result of the WIF   memories (0=pass, !0=fail) */
	uint64_t ncb                          : 1;  /**< BiST result of the NCB   memories (0=pass, !0=fail) */
	uint64_t out                          : 1;  /**< BiST result of the OUT   memories (0=pass, !0=fail) */
	uint64_t crc                          : 1;  /**< BiST result of the CRC   memories (0=pass, !0=fail) */
	uint64_t chk                          : 1;  /**< BiST result of the CHK   memories (0=pass, !0=fail) */
	uint64_t qsb                          : 2;  /**< BiST result of the QSB   memories (0=pass, !0=fail) */
	uint64_t qcb                          : 2;  /**< BiST result of the QCB   memories (0=pass, !0=fail) */
	uint64_t pdb                          : 4;  /**< BiST result of the PDB   memories (0=pass, !0=fail) */
	uint64_t psb                          : 7;  /**< BiST result of the PSB   memories (0=pass, !0=fail) */
#else
	uint64_t psb                          : 7;
	uint64_t pdb                          : 4;
	uint64_t qcb                          : 2;
	uint64_t qsb                          : 2;
	uint64_t chk                          : 1;
	uint64_t crc                          : 1;
	uint64_t out                          : 1;
	uint64_t ncb                          : 1;
	uint64_t wif                          : 1;
	uint64_t rif                          : 1;
	uint64_t count                        : 1;
	uint64_t psb2                         : 5;
	uint64_t reserved_27_63               : 37;
#endif
	} cn30xx;
	struct cvmx_pko_reg_bist_result_cn30xx cn31xx;
	struct cvmx_pko_reg_bist_result_cn30xx cn38xx;
	struct cvmx_pko_reg_bist_result_cn30xx cn38xxp2;
	struct cvmx_pko_reg_bist_result_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_33_63               : 31;
	uint64_t csr                          : 1;  /**< BiST result of CSR      memories (0=pass, !0=fail) */
	uint64_t iob                          : 1;  /**< BiST result of IOB      memories (0=pass, !0=fail) */
	uint64_t out_crc                      : 1;  /**< BiST result of OUT_CRC  memories (0=pass, !0=fail) */
	uint64_t out_ctl                      : 3;  /**< BiST result of OUT_CTL  memories (0=pass, !0=fail) */
	uint64_t out_sta                      : 1;  /**< BiST result of OUT_STA  memories (0=pass, !0=fail) */
	uint64_t out_wif                      : 1;  /**< BiST result of OUT_WIF  memories (0=pass, !0=fail) */
	uint64_t prt_chk                      : 3;  /**< BiST result of PRT_CHK  memories (0=pass, !0=fail) */
	uint64_t prt_nxt                      : 1;  /**< BiST result of PRT_NXT  memories (0=pass, !0=fail) */
	uint64_t prt_psb                      : 6;  /**< BiST result of PRT_PSB  memories (0=pass, !0=fail) */
	uint64_t ncb_inb                      : 2;  /**< BiST result of NCB_INB  memories (0=pass, !0=fail) */
	uint64_t prt_qcb                      : 2;  /**< BiST result of PRT_QCB  memories (0=pass, !0=fail) */
	uint64_t prt_qsb                      : 3;  /**< BiST result of PRT_QSB  memories (0=pass, !0=fail) */
	uint64_t dat_dat                      : 4;  /**< BiST result of DAT_DAT  memories (0=pass, !0=fail) */
	uint64_t dat_ptr                      : 4;  /**< BiST result of DAT_PTR  memories (0=pass, !0=fail) */
#else
	uint64_t dat_ptr                      : 4;
	uint64_t dat_dat                      : 4;
	uint64_t prt_qsb                      : 3;
	uint64_t prt_qcb                      : 2;
	uint64_t ncb_inb                      : 2;
	uint64_t prt_psb                      : 6;
	uint64_t prt_nxt                      : 1;
	uint64_t prt_chk                      : 3;
	uint64_t out_wif                      : 1;
	uint64_t out_sta                      : 1;
	uint64_t out_ctl                      : 3;
	uint64_t out_crc                      : 1;
	uint64_t iob                          : 1;
	uint64_t csr                          : 1;
	uint64_t reserved_33_63               : 31;
#endif
	} cn50xx;
	struct cvmx_pko_reg_bist_result_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_35_63               : 29;
	uint64_t csr                          : 1;  /**< BiST result of CSR      memories (0=pass, !0=fail) */
	uint64_t iob                          : 1;  /**< BiST result of IOB      memories (0=pass, !0=fail) */
	uint64_t out_dat                      : 1;  /**< BiST result of OUT_DAT  memories (0=pass, !0=fail) */
	uint64_t out_ctl                      : 3;  /**< BiST result of OUT_CTL  memories (0=pass, !0=fail) */
	uint64_t out_sta                      : 1;  /**< BiST result of OUT_STA  memories (0=pass, !0=fail) */
	uint64_t out_wif                      : 1;  /**< BiST result of OUT_WIF  memories (0=pass, !0=fail) */
	uint64_t prt_chk                      : 3;  /**< BiST result of PRT_CHK  memories (0=pass, !0=fail) */
	uint64_t prt_nxt                      : 1;  /**< BiST result of PRT_NXT  memories (0=pass, !0=fail) */
	uint64_t prt_psb                      : 8;  /**< BiST result of PRT_PSB  memories (0=pass, !0=fail) */
	uint64_t ncb_inb                      : 2;  /**< BiST result of NCB_INB  memories (0=pass, !0=fail) */
	uint64_t prt_qcb                      : 2;  /**< BiST result of PRT_QCB  memories (0=pass, !0=fail) */
	uint64_t prt_qsb                      : 3;  /**< BiST result of PRT_QSB  memories (0=pass, !0=fail) */
	uint64_t prt_ctl                      : 2;  /**< BiST result of PRT_CTL  memories (0=pass, !0=fail) */
	uint64_t dat_dat                      : 2;  /**< BiST result of DAT_DAT  memories (0=pass, !0=fail) */
	uint64_t dat_ptr                      : 4;  /**< BiST result of DAT_PTR  memories (0=pass, !0=fail) */
#else
	uint64_t dat_ptr                      : 4;
	uint64_t dat_dat                      : 2;
	uint64_t prt_ctl                      : 2;
	uint64_t prt_qsb                      : 3;
	uint64_t prt_qcb                      : 2;
	uint64_t ncb_inb                      : 2;
	uint64_t prt_psb                      : 8;
	uint64_t prt_nxt                      : 1;
	uint64_t prt_chk                      : 3;
	uint64_t out_wif                      : 1;
	uint64_t out_sta                      : 1;
	uint64_t out_ctl                      : 3;
	uint64_t out_dat                      : 1;
	uint64_t iob                          : 1;
	uint64_t csr                          : 1;
	uint64_t reserved_35_63               : 29;
#endif
	} cn52xx;
	struct cvmx_pko_reg_bist_result_cn52xx cn52xxp1;
	struct cvmx_pko_reg_bist_result_cn52xx cn56xx;
	struct cvmx_pko_reg_bist_result_cn52xx cn56xxp1;
	struct cvmx_pko_reg_bist_result_cn50xx cn58xx;
	struct cvmx_pko_reg_bist_result_cn50xx cn58xxp1;
	struct cvmx_pko_reg_bist_result_cn52xx cn61xx;
	struct cvmx_pko_reg_bist_result_cn52xx cn63xx;
	struct cvmx_pko_reg_bist_result_cn52xx cn63xxp1;
	struct cvmx_pko_reg_bist_result_cn52xx cn66xx;
	struct cvmx_pko_reg_bist_result_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t crc                          : 1;  /**< BiST result of CRC      memories (0=pass, !0=fail) */
	uint64_t csr                          : 1;  /**< BiST result of CSR      memories (0=pass, !0=fail) */
	uint64_t iob                          : 1;  /**< BiST result of IOB      memories (0=pass, !0=fail) */
	uint64_t out_dat                      : 1;  /**< BiST result of OUT_DAT  memories (0=pass, !0=fail) */
	uint64_t reserved_31_31               : 1;
	uint64_t out_ctl                      : 2;  /**< BiST result of OUT_CTL  memories (0=pass, !0=fail) */
	uint64_t out_sta                      : 1;  /**< BiST result of OUT_STA  memories (0=pass, !0=fail) */
	uint64_t out_wif                      : 1;  /**< BiST result of OUT_WIF  memories (0=pass, !0=fail) */
	uint64_t prt_chk                      : 3;  /**< BiST result of PRT_CHK  memories (0=pass, !0=fail) */
	uint64_t prt_nxt                      : 1;  /**< BiST result of PRT_NXT  memories (0=pass, !0=fail) */
	uint64_t prt_psb7                     : 1;  /**< BiST result of PRT_PSB  memories (0=pass, !0=fail) */
	uint64_t reserved_21_21               : 1;
	uint64_t prt_psb                      : 6;  /**< BiST result of PRT_PSB  memories (0=pass, !0=fail) */
	uint64_t ncb_inb                      : 2;  /**< BiST result of NCB_INB  memories (0=pass, !0=fail) */
	uint64_t prt_qcb                      : 2;  /**< BiST result of PRT_QCB  memories (0=pass, !0=fail) */
	uint64_t prt_qsb                      : 3;  /**< BiST result of PRT_QSB  memories (0=pass, !0=fail) */
	uint64_t prt_ctl                      : 2;  /**< BiST result of PRT_CTL  memories (0=pass, !0=fail) */
	uint64_t dat_dat                      : 2;  /**< BiST result of DAT_DAT  memories (0=pass, !0=fail) */
	uint64_t dat_ptr                      : 4;  /**< BiST result of DAT_PTR  memories (0=pass, !0=fail) */
#else
	uint64_t dat_ptr                      : 4;
	uint64_t dat_dat                      : 2;
	uint64_t prt_ctl                      : 2;
	uint64_t prt_qsb                      : 3;
	uint64_t prt_qcb                      : 2;
	uint64_t ncb_inb                      : 2;
	uint64_t prt_psb                      : 6;
	uint64_t reserved_21_21               : 1;
	uint64_t prt_psb7                     : 1;
	uint64_t prt_nxt                      : 1;
	uint64_t prt_chk                      : 3;
	uint64_t out_wif                      : 1;
	uint64_t out_sta                      : 1;
	uint64_t out_ctl                      : 2;
	uint64_t reserved_31_31               : 1;
	uint64_t out_dat                      : 1;
	uint64_t iob                          : 1;
	uint64_t csr                          : 1;
	uint64_t crc                          : 1;
	uint64_t reserved_36_63               : 28;
#endif
	} cn68xx;
	struct cvmx_pko_reg_bist_result_cn68xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_35_63               : 29;
	uint64_t csr                          : 1;  /**< BiST result of CSR      memories (0=pass, !0=fail) */
	uint64_t iob                          : 1;  /**< BiST result of IOB      memories (0=pass, !0=fail) */
	uint64_t out_dat                      : 1;  /**< BiST result of OUT_DAT  memories (0=pass, !0=fail) */
	uint64_t reserved_31_31               : 1;
	uint64_t out_ctl                      : 2;  /**< BiST result of OUT_CTL  memories (0=pass, !0=fail) */
	uint64_t out_sta                      : 1;  /**< BiST result of OUT_STA  memories (0=pass, !0=fail) */
	uint64_t out_wif                      : 1;  /**< BiST result of OUT_WIF  memories (0=pass, !0=fail) */
	uint64_t prt_chk                      : 3;  /**< BiST result of PRT_CHK  memories (0=pass, !0=fail) */
	uint64_t prt_nxt                      : 1;  /**< BiST result of PRT_NXT  memories (0=pass, !0=fail) */
	uint64_t prt_psb7                     : 1;  /**< BiST result of PRT_PSB  memories (0=pass, !0=fail) */
	uint64_t reserved_21_21               : 1;
	uint64_t prt_psb                      : 6;  /**< BiST result of PRT_PSB  memories (0=pass, !0=fail) */
	uint64_t ncb_inb                      : 2;  /**< BiST result of NCB_INB  memories (0=pass, !0=fail) */
	uint64_t prt_qcb                      : 2;  /**< BiST result of PRT_QCB  memories (0=pass, !0=fail) */
	uint64_t prt_qsb                      : 3;  /**< BiST result of PRT_QSB  memories (0=pass, !0=fail) */
	uint64_t prt_ctl                      : 2;  /**< BiST result of PRT_CTL  memories (0=pass, !0=fail) */
	uint64_t dat_dat                      : 2;  /**< BiST result of DAT_DAT  memories (0=pass, !0=fail) */
	uint64_t dat_ptr                      : 4;  /**< BiST result of DAT_PTR  memories (0=pass, !0=fail) */
#else
	uint64_t dat_ptr                      : 4;
	uint64_t dat_dat                      : 2;
	uint64_t prt_ctl                      : 2;
	uint64_t prt_qsb                      : 3;
	uint64_t prt_qcb                      : 2;
	uint64_t ncb_inb                      : 2;
	uint64_t prt_psb                      : 6;
	uint64_t reserved_21_21               : 1;
	uint64_t prt_psb7                     : 1;
	uint64_t prt_nxt                      : 1;
	uint64_t prt_chk                      : 3;
	uint64_t out_wif                      : 1;
	uint64_t out_sta                      : 1;
	uint64_t out_ctl                      : 2;
	uint64_t reserved_31_31               : 1;
	uint64_t out_dat                      : 1;
	uint64_t iob                          : 1;
	uint64_t csr                          : 1;
	uint64_t reserved_35_63               : 29;
#endif
	} cn68xxp1;
	struct cvmx_pko_reg_bist_result_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_30_63               : 34;
	uint64_t csr                          : 1;  /**< BiST result of CSR      memories (0=pass, !0=fail) */
	uint64_t iob                          : 1;  /**< BiST result of IOB      memories (0=pass, !0=fail) */
	uint64_t out_dat                      : 1;  /**< BiST result of OUT_DAT  memories (0=pass, !0=fail) */
	uint64_t out_ctl                      : 1;  /**< BiST result of OUT_CTL  memories (0=pass, !0=fail) */
	uint64_t out_sta                      : 1;  /**< BiST result of OUT_STA  memories (0=pass, !0=fail) */
	uint64_t out_wif                      : 1;  /**< BiST result of OUT_WIF  memories (0=pass, !0=fail) */
	uint64_t prt_chk                      : 3;  /**< BiST result of PRT_CHK  memories (0=pass, !0=fail) */
	uint64_t prt_nxt                      : 1;  /**< BiST result of PRT_NXT  memories (0=pass, !0=fail) */
	uint64_t prt_psb                      : 8;  /**< BiST result of PRT_PSB  memories (0=pass, !0=fail) */
	uint64_t ncb_inb                      : 1;  /**< BiST result of NCB_INB  memories (0=pass, !0=fail) */
	uint64_t prt_qcb                      : 1;  /**< BiST result of PRT_QCB  memories (0=pass, !0=fail) */
	uint64_t prt_qsb                      : 2;  /**< BiST result of PRT_QSB  memories (0=pass, !0=fail) */
	uint64_t prt_ctl                      : 2;  /**< BiST result of PRT_CTL  memories (0=pass, !0=fail) */
	uint64_t dat_dat                      : 2;  /**< BiST result of DAT_DAT  memories (0=pass, !0=fail) */
	uint64_t dat_ptr                      : 4;  /**< BiST result of DAT_PTR  memories (0=pass, !0=fail) */
#else
	uint64_t dat_ptr                      : 4;
	uint64_t dat_dat                      : 2;
	uint64_t prt_ctl                      : 2;
	uint64_t prt_qsb                      : 2;
	uint64_t prt_qcb                      : 1;
	uint64_t ncb_inb                      : 1;
	uint64_t prt_psb                      : 8;
	uint64_t prt_nxt                      : 1;
	uint64_t prt_chk                      : 3;
	uint64_t out_wif                      : 1;
	uint64_t out_sta                      : 1;
	uint64_t out_ctl                      : 1;
	uint64_t out_dat                      : 1;
	uint64_t iob                          : 1;
	uint64_t csr                          : 1;
	uint64_t reserved_30_63               : 34;
#endif
	} cn70xx;
	struct cvmx_pko_reg_bist_result_cn52xx cnf71xx;
};
typedef union cvmx_pko_reg_bist_result cvmx_pko_reg_bist_result_t;

/**
 * cvmx_pko_reg_cmd_buf
 *
 * Notes:
 * Sets the command buffer parameters
 * The size of the command buffer segments is measured in uint64s.  The pool specifies (1 of 8 free
 * lists to be used when freeing command buffer segments.
 */
union cvmx_pko_reg_cmd_buf {
	uint64_t u64;
	struct cvmx_pko_reg_cmd_buf_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_23_63               : 41;
	uint64_t pool                         : 3;  /**< Free list used to free command buffer segments */
	uint64_t reserved_13_19               : 7;
	uint64_t size                         : 13; /**< Number of uint64s per command buffer segment */
#else
	uint64_t size                         : 13;
	uint64_t reserved_13_19               : 7;
	uint64_t pool                         : 3;
	uint64_t reserved_23_63               : 41;
#endif
	} s;
	struct cvmx_pko_reg_cmd_buf_s         cn30xx;
	struct cvmx_pko_reg_cmd_buf_s         cn31xx;
	struct cvmx_pko_reg_cmd_buf_s         cn38xx;
	struct cvmx_pko_reg_cmd_buf_s         cn38xxp2;
	struct cvmx_pko_reg_cmd_buf_s         cn50xx;
	struct cvmx_pko_reg_cmd_buf_s         cn52xx;
	struct cvmx_pko_reg_cmd_buf_s         cn52xxp1;
	struct cvmx_pko_reg_cmd_buf_s         cn56xx;
	struct cvmx_pko_reg_cmd_buf_s         cn56xxp1;
	struct cvmx_pko_reg_cmd_buf_s         cn58xx;
	struct cvmx_pko_reg_cmd_buf_s         cn58xxp1;
	struct cvmx_pko_reg_cmd_buf_s         cn61xx;
	struct cvmx_pko_reg_cmd_buf_s         cn63xx;
	struct cvmx_pko_reg_cmd_buf_s         cn63xxp1;
	struct cvmx_pko_reg_cmd_buf_s         cn66xx;
	struct cvmx_pko_reg_cmd_buf_s         cn68xx;
	struct cvmx_pko_reg_cmd_buf_s         cn68xxp1;
	struct cvmx_pko_reg_cmd_buf_s         cn70xx;
	struct cvmx_pko_reg_cmd_buf_s         cnf71xx;
};
typedef union cvmx_pko_reg_cmd_buf cvmx_pko_reg_cmd_buf_t;

/**
 * cvmx_pko_reg_crc_ctl#
 *
 * Notes:
 * Controls datapath reflection when calculating CRC
 *
 */
union cvmx_pko_reg_crc_ctlx {
	uint64_t u64;
	struct cvmx_pko_reg_crc_ctlx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t invres                       : 1;  /**< Invert the result */
	uint64_t refin                        : 1;  /**< Reflect the bits in each byte.
                                                          Byte order does not change.
                                                         - 0: CRC is calculated MSB to LSB
                                                         - 1: CRC is calculated MLB to MSB */
#else
	uint64_t refin                        : 1;
	uint64_t invres                       : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_pko_reg_crc_ctlx_s        cn38xx;
	struct cvmx_pko_reg_crc_ctlx_s        cn38xxp2;
	struct cvmx_pko_reg_crc_ctlx_s        cn58xx;
	struct cvmx_pko_reg_crc_ctlx_s        cn58xxp1;
};
typedef union cvmx_pko_reg_crc_ctlx cvmx_pko_reg_crc_ctlx_t;

/**
 * cvmx_pko_reg_crc_enable
 *
 * Notes:
 * Enables CRC for the GMX ports.
 *
 */
union cvmx_pko_reg_crc_enable {
	uint64_t u64;
	struct cvmx_pko_reg_crc_enable_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t enable                       : 32; /**< Mask for ports 31-0 to enable CRC
                                                         Mask bit==0 means CRC not enabled
                                                         Mask bit==1 means CRC     enabled
                                                         Note that CRC should be enabled only when using SPI4.2 */
#else
	uint64_t enable                       : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_pko_reg_crc_enable_s      cn38xx;
	struct cvmx_pko_reg_crc_enable_s      cn38xxp2;
	struct cvmx_pko_reg_crc_enable_s      cn58xx;
	struct cvmx_pko_reg_crc_enable_s      cn58xxp1;
};
typedef union cvmx_pko_reg_crc_enable cvmx_pko_reg_crc_enable_t;

/**
 * cvmx_pko_reg_crc_iv#
 *
 * Notes:
 * Determines the IV used by the CRC algorithm
 * * PKO_CRC_IV
 *  PKO_CRC_IV controls the initial state of the CRC algorithm.  Octane can
 *  support a wide range of CRC algorithms and as such, the IV must be
 *  carefully constructed to meet the specific algorithm.  The code below
 *  determines the value to program into Octane based on the algorthim's IV
 *  and width.  In the case of Octane, the width should always be 32.
 *
 *  PKO_CRC_IV0 sets the IV for ports 0-15 while PKO_CRC_IV1 sets the IV for
 *  ports 16-31.
 *
 *   @verbatim
 *   unsigned octane_crc_iv(unsigned algorithm_iv, unsigned poly, unsigned w)
 *   [
 *     int i;
 *     int doit;
 *     unsigned int current_val = algorithm_iv;
 *
 *     for(i = 0; i < w; i++) [
 *       doit = current_val & 0x1;
 *
 *       if(doit) current_val ^= poly;
 *       assert(!(current_val & 0x1));
 *
 *       current_val = (current_val >> 1) | (doit << (w-1));
 *     ]
 *
 *     return current_val;
 *   ]
 *   @endverbatim
 */
union cvmx_pko_reg_crc_ivx {
	uint64_t u64;
	struct cvmx_pko_reg_crc_ivx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t iv                           : 32; /**< IV used by the CRC algorithm.  Default is FCS32. */
#else
	uint64_t iv                           : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_pko_reg_crc_ivx_s         cn38xx;
	struct cvmx_pko_reg_crc_ivx_s         cn38xxp2;
	struct cvmx_pko_reg_crc_ivx_s         cn58xx;
	struct cvmx_pko_reg_crc_ivx_s         cn58xxp1;
};
typedef union cvmx_pko_reg_crc_ivx cvmx_pko_reg_crc_ivx_t;

/**
 * cvmx_pko_reg_debug0
 *
 * Notes:
 * Note that this CSR is present only in chip revisions beginning with pass2.
 *
 */
union cvmx_pko_reg_debug0 {
	uint64_t u64;
	struct cvmx_pko_reg_debug0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t asserts                      : 64; /**< Various assertion checks */
#else
	uint64_t asserts                      : 64;
#endif
	} s;
	struct cvmx_pko_reg_debug0_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_17_63               : 47;
	uint64_t asserts                      : 17; /**< Various assertion checks */
#else
	uint64_t asserts                      : 17;
	uint64_t reserved_17_63               : 47;
#endif
	} cn30xx;
	struct cvmx_pko_reg_debug0_cn30xx     cn31xx;
	struct cvmx_pko_reg_debug0_cn30xx     cn38xx;
	struct cvmx_pko_reg_debug0_cn30xx     cn38xxp2;
	struct cvmx_pko_reg_debug0_s          cn50xx;
	struct cvmx_pko_reg_debug0_s          cn52xx;
	struct cvmx_pko_reg_debug0_s          cn52xxp1;
	struct cvmx_pko_reg_debug0_s          cn56xx;
	struct cvmx_pko_reg_debug0_s          cn56xxp1;
	struct cvmx_pko_reg_debug0_s          cn58xx;
	struct cvmx_pko_reg_debug0_s          cn58xxp1;
	struct cvmx_pko_reg_debug0_s          cn61xx;
	struct cvmx_pko_reg_debug0_s          cn63xx;
	struct cvmx_pko_reg_debug0_s          cn63xxp1;
	struct cvmx_pko_reg_debug0_s          cn66xx;
	struct cvmx_pko_reg_debug0_s          cn68xx;
	struct cvmx_pko_reg_debug0_s          cn68xxp1;
	struct cvmx_pko_reg_debug0_s          cn70xx;
	struct cvmx_pko_reg_debug0_s          cnf71xx;
};
typedef union cvmx_pko_reg_debug0 cvmx_pko_reg_debug0_t;

/**
 * cvmx_pko_reg_debug1
 */
union cvmx_pko_reg_debug1 {
	uint64_t u64;
	struct cvmx_pko_reg_debug1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t asserts                      : 64; /**< Various assertion checks */
#else
	uint64_t asserts                      : 64;
#endif
	} s;
	struct cvmx_pko_reg_debug1_s          cn50xx;
	struct cvmx_pko_reg_debug1_s          cn52xx;
	struct cvmx_pko_reg_debug1_s          cn52xxp1;
	struct cvmx_pko_reg_debug1_s          cn56xx;
	struct cvmx_pko_reg_debug1_s          cn56xxp1;
	struct cvmx_pko_reg_debug1_s          cn58xx;
	struct cvmx_pko_reg_debug1_s          cn58xxp1;
	struct cvmx_pko_reg_debug1_s          cn61xx;
	struct cvmx_pko_reg_debug1_s          cn63xx;
	struct cvmx_pko_reg_debug1_s          cn63xxp1;
	struct cvmx_pko_reg_debug1_s          cn66xx;
	struct cvmx_pko_reg_debug1_s          cn68xx;
	struct cvmx_pko_reg_debug1_s          cn68xxp1;
	struct cvmx_pko_reg_debug1_s          cn70xx;
	struct cvmx_pko_reg_debug1_s          cnf71xx;
};
typedef union cvmx_pko_reg_debug1 cvmx_pko_reg_debug1_t;

/**
 * cvmx_pko_reg_debug2
 */
union cvmx_pko_reg_debug2 {
	uint64_t u64;
	struct cvmx_pko_reg_debug2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t asserts                      : 64; /**< Various assertion checks */
#else
	uint64_t asserts                      : 64;
#endif
	} s;
	struct cvmx_pko_reg_debug2_s          cn50xx;
	struct cvmx_pko_reg_debug2_s          cn52xx;
	struct cvmx_pko_reg_debug2_s          cn52xxp1;
	struct cvmx_pko_reg_debug2_s          cn56xx;
	struct cvmx_pko_reg_debug2_s          cn56xxp1;
	struct cvmx_pko_reg_debug2_s          cn58xx;
	struct cvmx_pko_reg_debug2_s          cn58xxp1;
	struct cvmx_pko_reg_debug2_s          cn61xx;
	struct cvmx_pko_reg_debug2_s          cn63xx;
	struct cvmx_pko_reg_debug2_s          cn63xxp1;
	struct cvmx_pko_reg_debug2_s          cn66xx;
	struct cvmx_pko_reg_debug2_s          cn68xx;
	struct cvmx_pko_reg_debug2_s          cn68xxp1;
	struct cvmx_pko_reg_debug2_s          cn70xx;
	struct cvmx_pko_reg_debug2_s          cnf71xx;
};
typedef union cvmx_pko_reg_debug2 cvmx_pko_reg_debug2_t;

/**
 * cvmx_pko_reg_debug3
 */
union cvmx_pko_reg_debug3 {
	uint64_t u64;
	struct cvmx_pko_reg_debug3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t asserts                      : 64; /**< Various assertion checks */
#else
	uint64_t asserts                      : 64;
#endif
	} s;
	struct cvmx_pko_reg_debug3_s          cn50xx;
	struct cvmx_pko_reg_debug3_s          cn52xx;
	struct cvmx_pko_reg_debug3_s          cn52xxp1;
	struct cvmx_pko_reg_debug3_s          cn56xx;
	struct cvmx_pko_reg_debug3_s          cn56xxp1;
	struct cvmx_pko_reg_debug3_s          cn58xx;
	struct cvmx_pko_reg_debug3_s          cn58xxp1;
	struct cvmx_pko_reg_debug3_s          cn61xx;
	struct cvmx_pko_reg_debug3_s          cn63xx;
	struct cvmx_pko_reg_debug3_s          cn63xxp1;
	struct cvmx_pko_reg_debug3_s          cn66xx;
	struct cvmx_pko_reg_debug3_s          cn68xx;
	struct cvmx_pko_reg_debug3_s          cn68xxp1;
	struct cvmx_pko_reg_debug3_s          cn70xx;
	struct cvmx_pko_reg_debug3_s          cnf71xx;
};
typedef union cvmx_pko_reg_debug3 cvmx_pko_reg_debug3_t;

/**
 * cvmx_pko_reg_debug4
 */
union cvmx_pko_reg_debug4 {
	uint64_t u64;
	struct cvmx_pko_reg_debug4_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t asserts                      : 64; /**< Various assertion checks */
#else
	uint64_t asserts                      : 64;
#endif
	} s;
	struct cvmx_pko_reg_debug4_s          cn68xx;
	struct cvmx_pko_reg_debug4_s          cn68xxp1;
};
typedef union cvmx_pko_reg_debug4 cvmx_pko_reg_debug4_t;

/**
 * cvmx_pko_reg_engine_inflight
 *
 * Notes:
 * Sets the maximum number of inflight packets, per engine.  Values greater than 4 are illegal.
 * Setting an engine's value to 0 effectively stops the engine.
 */
union cvmx_pko_reg_engine_inflight {
	uint64_t u64;
	struct cvmx_pko_reg_engine_inflight_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t engine15                     : 4;  /**< Maximum number of inflight packets for engine15 */
	uint64_t engine14                     : 4;  /**< Maximum number of inflight packets for engine14 */
	uint64_t engine13                     : 4;  /**< Reserved */
	uint64_t engine12                     : 4;  /**< Reserved */
	uint64_t engine11                     : 4;  /**< Reserved */
	uint64_t engine10                     : 4;  /**< Reserved */
	uint64_t engine9                      : 4;  /**< Maximum number of inflight packets for engine9 */
	uint64_t engine8                      : 4;  /**< Maximum number of inflight packets for engine8 */
	uint64_t engine7                      : 4;  /**< Reserved */
	uint64_t engine6                      : 4;  /**< Reserved */
	uint64_t engine5                      : 4;  /**< Reserved */
	uint64_t engine4                      : 4;  /**< Reserved */
	uint64_t engine3                      : 4;  /**< Reserved */
	uint64_t engine2                      : 4;  /**< Reserved */
	uint64_t engine1                      : 4;  /**< Maximum number of inflight packets for engine1 */
	uint64_t engine0                      : 4;  /**< Maximum number of inflight packets for engine0 */
#else
	uint64_t engine0                      : 4;
	uint64_t engine1                      : 4;
	uint64_t engine2                      : 4;
	uint64_t engine3                      : 4;
	uint64_t engine4                      : 4;
	uint64_t engine5                      : 4;
	uint64_t engine6                      : 4;
	uint64_t engine7                      : 4;
	uint64_t engine8                      : 4;
	uint64_t engine9                      : 4;
	uint64_t engine10                     : 4;
	uint64_t engine11                     : 4;
	uint64_t engine12                     : 4;
	uint64_t engine13                     : 4;
	uint64_t engine14                     : 4;
	uint64_t engine15                     : 4;
#endif
	} s;
	struct cvmx_pko_reg_engine_inflight_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t engine9                      : 4;  /**< Maximum number of inflight packets for engine9 */
	uint64_t engine8                      : 4;  /**< Maximum number of inflight packets for engine8 */
	uint64_t engine7                      : 4;  /**< MBZ */
	uint64_t engine6                      : 4;  /**< MBZ */
	uint64_t engine5                      : 4;  /**< MBZ */
	uint64_t engine4                      : 4;  /**< MBZ */
	uint64_t engine3                      : 4;  /**< Maximum number of inflight packets for engine3 */
	uint64_t engine2                      : 4;  /**< Maximum number of inflight packets for engine2 */
	uint64_t engine1                      : 4;  /**< Maximum number of inflight packets for engine1 */
	uint64_t engine0                      : 4;  /**< Maximum number of inflight packets for engine0 */
#else
	uint64_t engine0                      : 4;
	uint64_t engine1                      : 4;
	uint64_t engine2                      : 4;
	uint64_t engine3                      : 4;
	uint64_t engine4                      : 4;
	uint64_t engine5                      : 4;
	uint64_t engine6                      : 4;
	uint64_t engine7                      : 4;
	uint64_t engine8                      : 4;
	uint64_t engine9                      : 4;
	uint64_t reserved_40_63               : 24;
#endif
	} cn52xx;
	struct cvmx_pko_reg_engine_inflight_cn52xx cn52xxp1;
	struct cvmx_pko_reg_engine_inflight_cn52xx cn56xx;
	struct cvmx_pko_reg_engine_inflight_cn52xx cn56xxp1;
	struct cvmx_pko_reg_engine_inflight_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t engine13                     : 4;  /**< Reserved */
	uint64_t engine12                     : 4;  /**< Reserved */
	uint64_t engine11                     : 4;  /**< Reserved */
	uint64_t engine10                     : 4;  /**< Reserved */
	uint64_t engine9                      : 4;  /**< Maximum number of inflight packets for engine9 */
	uint64_t engine8                      : 4;  /**< Maximum number of inflight packets for engine8 */
	uint64_t engine7                      : 4;  /**< Maximum number of inflight packets for engine7 */
	uint64_t engine6                      : 4;  /**< Maximum number of inflight packets for engine6 */
	uint64_t engine5                      : 4;  /**< Maximum number of inflight packets for engine5 */
	uint64_t engine4                      : 4;  /**< Maximum number of inflight packets for engine4 */
	uint64_t engine3                      : 4;  /**< Maximum number of inflight packets for engine3 */
	uint64_t engine2                      : 4;  /**< Maximum number of inflight packets for engine2 */
	uint64_t engine1                      : 4;  /**< Maximum number of inflight packets for engine1 */
	uint64_t engine0                      : 4;  /**< Maximum number of inflight packets for engine0 */
#else
	uint64_t engine0                      : 4;
	uint64_t engine1                      : 4;
	uint64_t engine2                      : 4;
	uint64_t engine3                      : 4;
	uint64_t engine4                      : 4;
	uint64_t engine5                      : 4;
	uint64_t engine6                      : 4;
	uint64_t engine7                      : 4;
	uint64_t engine8                      : 4;
	uint64_t engine9                      : 4;
	uint64_t engine10                     : 4;
	uint64_t engine11                     : 4;
	uint64_t engine12                     : 4;
	uint64_t engine13                     : 4;
	uint64_t reserved_56_63               : 8;
#endif
	} cn61xx;
	struct cvmx_pko_reg_engine_inflight_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t engine11                     : 4;  /**< Maximum number of inflight packets for engine11 */
	uint64_t engine10                     : 4;  /**< Maximum number of inflight packets for engine10 */
	uint64_t engine9                      : 4;  /**< Maximum number of inflight packets for engine9 */
	uint64_t engine8                      : 4;  /**< Maximum number of inflight packets for engine8 */
	uint64_t engine7                      : 4;  /**< MBZ */
	uint64_t engine6                      : 4;  /**< MBZ */
	uint64_t engine5                      : 4;  /**< MBZ */
	uint64_t engine4                      : 4;  /**< MBZ */
	uint64_t engine3                      : 4;  /**< Maximum number of inflight packets for engine3 */
	uint64_t engine2                      : 4;  /**< Maximum number of inflight packets for engine2 */
	uint64_t engine1                      : 4;  /**< Maximum number of inflight packets for engine1 */
	uint64_t engine0                      : 4;  /**< Maximum number of inflight packets for engine0 */
#else
	uint64_t engine0                      : 4;
	uint64_t engine1                      : 4;
	uint64_t engine2                      : 4;
	uint64_t engine3                      : 4;
	uint64_t engine4                      : 4;
	uint64_t engine5                      : 4;
	uint64_t engine6                      : 4;
	uint64_t engine7                      : 4;
	uint64_t engine8                      : 4;
	uint64_t engine9                      : 4;
	uint64_t engine10                     : 4;
	uint64_t engine11                     : 4;
	uint64_t reserved_48_63               : 16;
#endif
	} cn63xx;
	struct cvmx_pko_reg_engine_inflight_cn63xx cn63xxp1;
	struct cvmx_pko_reg_engine_inflight_cn61xx cn66xx;
	struct cvmx_pko_reg_engine_inflight_s cn68xx;
	struct cvmx_pko_reg_engine_inflight_s cn68xxp1;
	struct cvmx_pko_reg_engine_inflight_cn61xx cn70xx;
	struct cvmx_pko_reg_engine_inflight_cn61xx cnf71xx;
};
typedef union cvmx_pko_reg_engine_inflight cvmx_pko_reg_engine_inflight_t;

/**
 * cvmx_pko_reg_engine_inflight1
 *
 * Notes:
 * Sets the maximum number of inflight packets, per engine.  Values greater than 8 are illegal.
 * Setting an engine's value to 0 effectively stops the engine.
 */
union cvmx_pko_reg_engine_inflight1 {
	uint64_t u64;
	struct cvmx_pko_reg_engine_inflight1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t engine19                     : 4;  /**< Maximum number of inflight packets for engine19 */
	uint64_t engine18                     : 4;  /**< Maximum number of inflight packets for engine18 */
	uint64_t engine17                     : 4;  /**< Maximum number of inflight packets for engine17 */
	uint64_t engine16                     : 4;  /**< Maximum number of inflight packets for engine16 */
#else
	uint64_t engine16                     : 4;
	uint64_t engine17                     : 4;
	uint64_t engine18                     : 4;
	uint64_t engine19                     : 4;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_pko_reg_engine_inflight1_s cn68xx;
	struct cvmx_pko_reg_engine_inflight1_s cn68xxp1;
};
typedef union cvmx_pko_reg_engine_inflight1 cvmx_pko_reg_engine_inflight1_t;

/**
 * cvmx_pko_reg_engine_storage#
 *
 * Notes:
 * The PKO has 40KB of local storage, consisting of 20, 2KB chunks.  Up to 15 contiguous chunks may be mapped per engine.
 * The total of all mapped storage must not exceed 40KB.
 */
union cvmx_pko_reg_engine_storagex {
	uint64_t u64;
	struct cvmx_pko_reg_engine_storagex_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t engine15                     : 4;  /**< Number of contiguous 2KB chunks allocated to
                                                         engine (X * 16) + 15.
                                                         ENGINE15 does not exist and is reserved in
                                                         PKO_REG_ENGINE_STORAGE1. */
	uint64_t engine14                     : 4;  /**< Number of contiguous 2KB chunks allocated to
                                                         engine (X * 16) + 14.
                                                         ENGINE14 does not exist and is reserved in
                                                         PKO_REG_ENGINE_STORAGE1. */
	uint64_t engine13                     : 4;  /**< Number of contiguous 2KB chunks allocated to
                                                         engine (X * 16) + 13.
                                                         ENGINE13 does not exist and is reserved in
                                                         PKO_REG_ENGINE_STORAGE1. */
	uint64_t engine12                     : 4;  /**< Number of contiguous 2KB chunks allocated to
                                                         engine (X * 16) + 12.
                                                         ENGINE12 does not exist and is reserved in
                                                         PKO_REG_ENGINE_STORAGE1. */
	uint64_t engine11                     : 4;  /**< Number of contiguous 2KB chunks allocated to
                                                         engine (X * 16) + 11.
                                                         ENGINE11 does not exist and is reserved in
                                                         PKO_REG_ENGINE_STORAGE1. */
	uint64_t engine10                     : 4;  /**< Number of contiguous 2KB chunks allocated to
                                                         engine (X * 16) + 10.
                                                         ENGINE10 does not exist and is reserved in
                                                         PKO_REG_ENGINE_STORAGE1. */
	uint64_t engine9                      : 4;  /**< Number of contiguous 2KB chunks allocated to
                                                         engine (X * 16) + 9.
                                                         ENGINE9 does not exist and is reserved in
                                                         PKO_REG_ENGINE_STORAGE1. */
	uint64_t engine8                      : 4;  /**< Number of contiguous 2KB chunks allocated to
                                                         engine (X * 16) + 8.
                                                         ENGINE8 does not exist and is reserved in
                                                         PKO_REG_ENGINE_STORAGE1. */
	uint64_t engine7                      : 4;  /**< Number of contiguous 2KB chunks allocated to
                                                         engine (X * 16) + 7.
                                                         ENGINE7 does not exist and is reserved in
                                                         PKO_REG_ENGINE_STORAGE1. */
	uint64_t engine6                      : 4;  /**< Number of contiguous 2KB chunks allocated to
                                                         engine (X * 16) + 6.
                                                         ENGINE6 does not exist and is reserved in
                                                         PKO_REG_ENGINE_STORAGE1. */
	uint64_t engine5                      : 4;  /**< Number of contiguous 2KB chunks allocated to
                                                         engine (X * 16) + 5.
                                                         ENGINE5 does not exist and is reserved in
                                                         PKO_REG_ENGINE_STORAGE1. */
	uint64_t engine4                      : 4;  /**< Number of contiguous 2KB chunks allocated to
                                                         engine (X * 16) + 4.
                                                         ENGINE4 does not exist and is reserved in
                                                         PKO_REG_ENGINE_STORAGE1. */
	uint64_t engine3                      : 4;  /**< Number of contiguous 2KB chunks allocated to
                                                         engine (X * 16) + 3. */
	uint64_t engine2                      : 4;  /**< Number of contiguous 2KB chunks allocated to
                                                         engine (X * 16) + 2. */
	uint64_t engine1                      : 4;  /**< Number of contiguous 2KB chunks allocated to
                                                         engine (X * 16) + 1. */
	uint64_t engine0                      : 4;  /**< Number of contiguous 2KB chunks allocated to
                                                         engine (X * 16) + 0. */
#else
	uint64_t engine0                      : 4;
	uint64_t engine1                      : 4;
	uint64_t engine2                      : 4;
	uint64_t engine3                      : 4;
	uint64_t engine4                      : 4;
	uint64_t engine5                      : 4;
	uint64_t engine6                      : 4;
	uint64_t engine7                      : 4;
	uint64_t engine8                      : 4;
	uint64_t engine9                      : 4;
	uint64_t engine10                     : 4;
	uint64_t engine11                     : 4;
	uint64_t engine12                     : 4;
	uint64_t engine13                     : 4;
	uint64_t engine14                     : 4;
	uint64_t engine15                     : 4;
#endif
	} s;
	struct cvmx_pko_reg_engine_storagex_s cn68xx;
	struct cvmx_pko_reg_engine_storagex_s cn68xxp1;
};
typedef union cvmx_pko_reg_engine_storagex cvmx_pko_reg_engine_storagex_t;

/**
 * cvmx_pko_reg_engine_thresh
 *
 * Notes:
 * When not enabled, packet data may be sent as soon as it is written into PKO's internal buffers.
 * When enabled and the packet fits entirely in the PKO's internal buffer, none of the packet data will
 * be sent until all of it has been written into the PKO's internal buffer.  Note that a packet is
 * considered to fit entirely only if the packet's size is <= BUFFER_SIZE-8.  When enabled and the
 * packet does not fit entirely in the PKO's internal buffer, none of the packet data will be sent until
 * at least BUFFER_SIZE-256 bytes of the packet have been written into the PKO's internal buffer
 * (note that BUFFER_SIZE is a function of PKO_REG_GMX_PORT_MODE above)
 */
union cvmx_pko_reg_engine_thresh {
	uint64_t u64;
	struct cvmx_pko_reg_engine_thresh_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t mask                         : 20; /**< Mask[n]=0 disables packet send threshold for engine n
                                                         Mask[n]=1 enables  packet send threshold for engine n  $PR       NS
                                                         Bits <13:10,7:2> are unused */
#else
	uint64_t mask                         : 20;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_pko_reg_engine_thresh_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t mask                         : 10; /**< Mask[n]=0 disables packet send threshold for eng n
                                                         Mask[n]=1 enables  packet send threshold for eng n     $PR       NS
                                                         Mask[n] MBZ for n = 4-7, as engines 4-7 dont exist */
#else
	uint64_t mask                         : 10;
	uint64_t reserved_10_63               : 54;
#endif
	} cn52xx;
	struct cvmx_pko_reg_engine_thresh_cn52xx cn52xxp1;
	struct cvmx_pko_reg_engine_thresh_cn52xx cn56xx;
	struct cvmx_pko_reg_engine_thresh_cn52xx cn56xxp1;
	struct cvmx_pko_reg_engine_thresh_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t mask                         : 14; /**< Mask[n]=0 disables packet send threshold for engine n
                                                         Mask[n]=1 enables  packet send threshold for engine n  $PR       NS
                                                         Bits <13:10> are unused */
#else
	uint64_t mask                         : 14;
	uint64_t reserved_14_63               : 50;
#endif
	} cn61xx;
	struct cvmx_pko_reg_engine_thresh_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t mask                         : 12; /**< Mask[n]=0 disables packet send threshold for engine n
                                                         Mask[n]=1 enables  packet send threshold for engine n  $PR       NS
                                                         Mask[n] MBZ for n = 4-7, as engines 4-7 dont exist */
#else
	uint64_t mask                         : 12;
	uint64_t reserved_12_63               : 52;
#endif
	} cn63xx;
	struct cvmx_pko_reg_engine_thresh_cn63xx cn63xxp1;
	struct cvmx_pko_reg_engine_thresh_cn61xx cn66xx;
	struct cvmx_pko_reg_engine_thresh_s   cn68xx;
	struct cvmx_pko_reg_engine_thresh_s   cn68xxp1;
	struct cvmx_pko_reg_engine_thresh_cn61xx cn70xx;
	struct cvmx_pko_reg_engine_thresh_cn61xx cnf71xx;
};
typedef union cvmx_pko_reg_engine_thresh cvmx_pko_reg_engine_thresh_t;

/**
 * cvmx_pko_reg_error
 *
 * Notes:
 * Note that this CSR is present only in chip revisions beginning with pass2.
 *
 */
union cvmx_pko_reg_error {
	uint64_t u64;
	struct cvmx_pko_reg_error_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t loopback                     : 1;  /**< A packet was sent to an illegal loopback port */
	uint64_t currzero                     : 1;  /**< A packet data pointer has size=0 */
	uint64_t doorbell                     : 1;  /**< A doorbell count has overflowed */
	uint64_t parity                       : 1;  /**< Read parity error at port data buffer */
#else
	uint64_t parity                       : 1;
	uint64_t doorbell                     : 1;
	uint64_t currzero                     : 1;
	uint64_t loopback                     : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_pko_reg_error_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t doorbell                     : 1;  /**< A doorbell count has overflowed */
	uint64_t parity                       : 1;  /**< Read parity error at port data buffer */
#else
	uint64_t parity                       : 1;
	uint64_t doorbell                     : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} cn30xx;
	struct cvmx_pko_reg_error_cn30xx      cn31xx;
	struct cvmx_pko_reg_error_cn30xx      cn38xx;
	struct cvmx_pko_reg_error_cn30xx      cn38xxp2;
	struct cvmx_pko_reg_error_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t currzero                     : 1;  /**< A packet data pointer has size=0 */
	uint64_t doorbell                     : 1;  /**< A doorbell count has overflowed */
	uint64_t parity                       : 1;  /**< Read parity error at port data buffer */
#else
	uint64_t parity                       : 1;
	uint64_t doorbell                     : 1;
	uint64_t currzero                     : 1;
	uint64_t reserved_3_63                : 61;
#endif
	} cn50xx;
	struct cvmx_pko_reg_error_cn50xx      cn52xx;
	struct cvmx_pko_reg_error_cn50xx      cn52xxp1;
	struct cvmx_pko_reg_error_cn50xx      cn56xx;
	struct cvmx_pko_reg_error_cn50xx      cn56xxp1;
	struct cvmx_pko_reg_error_cn50xx      cn58xx;
	struct cvmx_pko_reg_error_cn50xx      cn58xxp1;
	struct cvmx_pko_reg_error_cn50xx      cn61xx;
	struct cvmx_pko_reg_error_cn50xx      cn63xx;
	struct cvmx_pko_reg_error_cn50xx      cn63xxp1;
	struct cvmx_pko_reg_error_cn50xx      cn66xx;
	struct cvmx_pko_reg_error_s           cn68xx;
	struct cvmx_pko_reg_error_s           cn68xxp1;
	struct cvmx_pko_reg_error_cn50xx      cn70xx;
	struct cvmx_pko_reg_error_cn50xx      cnf71xx;
};
typedef union cvmx_pko_reg_error cvmx_pko_reg_error_t;

/**
 * cvmx_pko_reg_flags
 *
 * Notes:
 * When set, ENA_PKO enables the PKO picker and places the PKO in normal operation.  When set, ENA_DWB
 * enables the use of DontWriteBacks during the buffer freeing operations.  When not set, STORE_BE inverts
 * bits[2:0] of the STORE0 byte write address.  When set, RESET causes a 4-cycle reset pulse to the
 * entire box.
 */
union cvmx_pko_reg_flags {
	uint64_t u64;
	struct cvmx_pko_reg_flags_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t dis_perf3                    : 1;  /**< Set to disable inactive queue QOS skipping */
	uint64_t dis_perf2                    : 1;  /**< Set to disable inactive queue skipping */
	uint64_t dis_perf1                    : 1;  /**< Set to disable command word prefetching */
	uint64_t dis_perf0                    : 1;  /**< Set to disable read performance optimizations */
	uint64_t ena_throttle                 : 1;  /**< Set to enable the PKO picker throttle logic
                                                         When ENA_THROTTLE=1 and the most-significant
                                                         bit of any of the pipe or interface, word or
                                                         packet throttle count is set, then PKO will
                                                         not output any packets to the interface/pipe.
                                                         See PKO_MEM_THROTTLE_PIPE and
                                                         PKO_MEM_THROTTLE_INT. */
	uint64_t reset                        : 1;  /**< Reset oneshot pulse */
	uint64_t store_be                     : 1;  /**< Force STORE0 byte write address to big endian */
	uint64_t ena_dwb                      : 1;  /**< Set to enable DontWriteBacks */
	uint64_t ena_pko                      : 1;  /**< Set to enable the PKO picker */
#else
	uint64_t ena_pko                      : 1;
	uint64_t ena_dwb                      : 1;
	uint64_t store_be                     : 1;
	uint64_t reset                        : 1;
	uint64_t ena_throttle                 : 1;
	uint64_t dis_perf0                    : 1;
	uint64_t dis_perf1                    : 1;
	uint64_t dis_perf2                    : 1;
	uint64_t dis_perf3                    : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_pko_reg_flags_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t reset                        : 1;  /**< Reset oneshot pulse */
	uint64_t store_be                     : 1;  /**< Force STORE0 byte write address to big endian */
	uint64_t ena_dwb                      : 1;  /**< Set to enable DontWriteBacks */
	uint64_t ena_pko                      : 1;  /**< Set to enable the PKO picker */
#else
	uint64_t ena_pko                      : 1;
	uint64_t ena_dwb                      : 1;
	uint64_t store_be                     : 1;
	uint64_t reset                        : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} cn30xx;
	struct cvmx_pko_reg_flags_cn30xx      cn31xx;
	struct cvmx_pko_reg_flags_cn30xx      cn38xx;
	struct cvmx_pko_reg_flags_cn30xx      cn38xxp2;
	struct cvmx_pko_reg_flags_cn30xx      cn50xx;
	struct cvmx_pko_reg_flags_cn30xx      cn52xx;
	struct cvmx_pko_reg_flags_cn30xx      cn52xxp1;
	struct cvmx_pko_reg_flags_cn30xx      cn56xx;
	struct cvmx_pko_reg_flags_cn30xx      cn56xxp1;
	struct cvmx_pko_reg_flags_cn30xx      cn58xx;
	struct cvmx_pko_reg_flags_cn30xx      cn58xxp1;
	struct cvmx_pko_reg_flags_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t dis_perf3                    : 1;  /**< Set to disable inactive queue QOS skipping */
	uint64_t dis_perf2                    : 1;  /**< Set to disable inactive queue skipping */
	uint64_t reserved_4_6                 : 3;
	uint64_t reset                        : 1;  /**< Reset oneshot pulse */
	uint64_t store_be                     : 1;  /**< Force STORE0 byte write address to big endian */
	uint64_t ena_dwb                      : 1;  /**< Set to enable DontWriteBacks */
	uint64_t ena_pko                      : 1;  /**< Set to enable the PKO picker */
#else
	uint64_t ena_pko                      : 1;
	uint64_t ena_dwb                      : 1;
	uint64_t store_be                     : 1;
	uint64_t reset                        : 1;
	uint64_t reserved_4_6                 : 3;
	uint64_t dis_perf2                    : 1;
	uint64_t dis_perf3                    : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} cn61xx;
	struct cvmx_pko_reg_flags_cn30xx      cn63xx;
	struct cvmx_pko_reg_flags_cn30xx      cn63xxp1;
	struct cvmx_pko_reg_flags_cn61xx      cn66xx;
	struct cvmx_pko_reg_flags_s           cn68xx;
	struct cvmx_pko_reg_flags_cn68xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t dis_perf1                    : 1;  /**< Set to disable command word prefetching */
	uint64_t dis_perf0                    : 1;  /**< Set to disable read performance optimizations */
	uint64_t ena_throttle                 : 1;  /**< Set to enable the PKO picker throttle logic
                                                         When ENA_THROTTLE=1 and the most-significant
                                                         bit of any of the pipe or interface, word or
                                                         packet throttle count is set, then PKO will
                                                         not output any packets to the interface/pipe.
                                                         See PKO_MEM_THROTTLE_PIPE and
                                                         PKO_MEM_THROTTLE_INT. */
	uint64_t reset                        : 1;  /**< Reset oneshot pulse */
	uint64_t store_be                     : 1;  /**< Force STORE0 byte write address to big endian */
	uint64_t ena_dwb                      : 1;  /**< Set to enable DontWriteBacks */
	uint64_t ena_pko                      : 1;  /**< Set to enable the PKO picker */
#else
	uint64_t ena_pko                      : 1;
	uint64_t ena_dwb                      : 1;
	uint64_t store_be                     : 1;
	uint64_t reset                        : 1;
	uint64_t ena_throttle                 : 1;
	uint64_t dis_perf0                    : 1;
	uint64_t dis_perf1                    : 1;
	uint64_t reserved_7_63                : 57;
#endif
	} cn68xxp1;
	struct cvmx_pko_reg_flags_cn61xx      cn70xx;
	struct cvmx_pko_reg_flags_cn61xx      cnf71xx;
};
typedef union cvmx_pko_reg_flags cvmx_pko_reg_flags_t;

/**
 * cvmx_pko_reg_gmx_port_mode
 *
 * Notes:
 * The system has a total of 2 + 4 + 4 ports and 2 + 1 + 1 engines (GM0 + PCI + LOOP).
 * This CSR sets the number of GMX0 ports and amount of local storage per engine.
 * It has no effect on the number of ports or amount of local storage per engine for PCI and LOOP.
 * When both GMX ports are used (MODE0=3), each GMX engine has 10kB of local
 * storage.  Increasing MODE0 to 4 decreases the number of GMX ports to 1 and
 * increases the local storage for the one remaining PKO GMX engine to 20kB.
 * MODE0 value 0, 1, and 2, or greater than 4 are illegal.
 *
 * MODE0   GMX0  PCI   LOOP  GMX0                       PCI            LOOP
 *         ports ports ports storage/engine             storage/engine storage/engine
 * 3       2     4     4      10.0kB                    2.5kB          2.5kB
 * 4       1     4     4      20.0kB                    2.5kB          2.5kB
 */
union cvmx_pko_reg_gmx_port_mode {
	uint64_t u64;
	struct cvmx_pko_reg_gmx_port_mode_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t mode1                        : 3;  /**< RESERVED, must be 5 */
	uint64_t mode0                        : 3;  /**< # of GM0 ports = 16 >> MODE0 */
#else
	uint64_t mode0                        : 3;
	uint64_t mode1                        : 3;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_pko_reg_gmx_port_mode_s   cn30xx;
	struct cvmx_pko_reg_gmx_port_mode_s   cn31xx;
	struct cvmx_pko_reg_gmx_port_mode_s   cn38xx;
	struct cvmx_pko_reg_gmx_port_mode_s   cn38xxp2;
	struct cvmx_pko_reg_gmx_port_mode_s   cn50xx;
	struct cvmx_pko_reg_gmx_port_mode_s   cn52xx;
	struct cvmx_pko_reg_gmx_port_mode_s   cn52xxp1;
	struct cvmx_pko_reg_gmx_port_mode_s   cn56xx;
	struct cvmx_pko_reg_gmx_port_mode_s   cn56xxp1;
	struct cvmx_pko_reg_gmx_port_mode_s   cn58xx;
	struct cvmx_pko_reg_gmx_port_mode_s   cn58xxp1;
	struct cvmx_pko_reg_gmx_port_mode_s   cn61xx;
	struct cvmx_pko_reg_gmx_port_mode_s   cn63xx;
	struct cvmx_pko_reg_gmx_port_mode_s   cn63xxp1;
	struct cvmx_pko_reg_gmx_port_mode_s   cn66xx;
	struct cvmx_pko_reg_gmx_port_mode_s   cn70xx;
	struct cvmx_pko_reg_gmx_port_mode_s   cnf71xx;
};
typedef union cvmx_pko_reg_gmx_port_mode cvmx_pko_reg_gmx_port_mode_t;

/**
 * cvmx_pko_reg_int_mask
 *
 * Notes:
 * When a mask bit is set, the corresponding interrupt is enabled.
 *
 */
union cvmx_pko_reg_int_mask {
	uint64_t u64;
	struct cvmx_pko_reg_int_mask_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t loopback                     : 1;  /**< Bit mask corresponding to PKO_REG_ERROR[3] above */
	uint64_t currzero                     : 1;  /**< Bit mask corresponding to PKO_REG_ERROR[2] above */
	uint64_t doorbell                     : 1;  /**< Bit mask corresponding to PKO_REG_ERROR[1] above */
	uint64_t parity                       : 1;  /**< Bit mask corresponding to PKO_REG_ERROR[0] above */
#else
	uint64_t parity                       : 1;
	uint64_t doorbell                     : 1;
	uint64_t currzero                     : 1;
	uint64_t loopback                     : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_pko_reg_int_mask_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t doorbell                     : 1;  /**< Bit mask corresponding to PKO_REG_ERROR[1] above */
	uint64_t parity                       : 1;  /**< Bit mask corresponding to PKO_REG_ERROR[0] above */
#else
	uint64_t parity                       : 1;
	uint64_t doorbell                     : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} cn30xx;
	struct cvmx_pko_reg_int_mask_cn30xx   cn31xx;
	struct cvmx_pko_reg_int_mask_cn30xx   cn38xx;
	struct cvmx_pko_reg_int_mask_cn30xx   cn38xxp2;
	struct cvmx_pko_reg_int_mask_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t currzero                     : 1;  /**< Bit mask corresponding to PKO_REG_ERROR[2] above */
	uint64_t doorbell                     : 1;  /**< Bit mask corresponding to PKO_REG_ERROR[1] above */
	uint64_t parity                       : 1;  /**< Bit mask corresponding to PKO_REG_ERROR[0] above */
#else
	uint64_t parity                       : 1;
	uint64_t doorbell                     : 1;
	uint64_t currzero                     : 1;
	uint64_t reserved_3_63                : 61;
#endif
	} cn50xx;
	struct cvmx_pko_reg_int_mask_cn50xx   cn52xx;
	struct cvmx_pko_reg_int_mask_cn50xx   cn52xxp1;
	struct cvmx_pko_reg_int_mask_cn50xx   cn56xx;
	struct cvmx_pko_reg_int_mask_cn50xx   cn56xxp1;
	struct cvmx_pko_reg_int_mask_cn50xx   cn58xx;
	struct cvmx_pko_reg_int_mask_cn50xx   cn58xxp1;
	struct cvmx_pko_reg_int_mask_cn50xx   cn61xx;
	struct cvmx_pko_reg_int_mask_cn50xx   cn63xx;
	struct cvmx_pko_reg_int_mask_cn50xx   cn63xxp1;
	struct cvmx_pko_reg_int_mask_cn50xx   cn66xx;
	struct cvmx_pko_reg_int_mask_s        cn68xx;
	struct cvmx_pko_reg_int_mask_s        cn68xxp1;
	struct cvmx_pko_reg_int_mask_cn50xx   cn70xx;
	struct cvmx_pko_reg_int_mask_cn50xx   cnf71xx;
};
typedef union cvmx_pko_reg_int_mask cvmx_pko_reg_int_mask_t;

/**
 * cvmx_pko_reg_loopback_bpid
 *
 * Notes:
 * None.
 *
 */
union cvmx_pko_reg_loopback_bpid {
	uint64_t u64;
	struct cvmx_pko_reg_loopback_bpid_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_59_63               : 5;
	uint64_t bpid7                        : 6;  /**< Loopback port 7 backpressure-ID */
	uint64_t reserved_52_52               : 1;
	uint64_t bpid6                        : 6;  /**< Loopback port 6 backpressure-ID */
	uint64_t reserved_45_45               : 1;
	uint64_t bpid5                        : 6;  /**< Loopback port 5 backpressure-ID */
	uint64_t reserved_38_38               : 1;
	uint64_t bpid4                        : 6;  /**< Loopback port 4 backpressure-ID */
	uint64_t reserved_31_31               : 1;
	uint64_t bpid3                        : 6;  /**< Loopback port 3 backpressure-ID */
	uint64_t reserved_24_24               : 1;
	uint64_t bpid2                        : 6;  /**< Loopback port 2 backpressure-ID */
	uint64_t reserved_17_17               : 1;
	uint64_t bpid1                        : 6;  /**< Loopback port 1 backpressure-ID */
	uint64_t reserved_10_10               : 1;
	uint64_t bpid0                        : 6;  /**< Loopback port 0 backpressure-ID */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t bpid0                        : 6;
	uint64_t reserved_10_10               : 1;
	uint64_t bpid1                        : 6;
	uint64_t reserved_17_17               : 1;
	uint64_t bpid2                        : 6;
	uint64_t reserved_24_24               : 1;
	uint64_t bpid3                        : 6;
	uint64_t reserved_31_31               : 1;
	uint64_t bpid4                        : 6;
	uint64_t reserved_38_38               : 1;
	uint64_t bpid5                        : 6;
	uint64_t reserved_45_45               : 1;
	uint64_t bpid6                        : 6;
	uint64_t reserved_52_52               : 1;
	uint64_t bpid7                        : 6;
	uint64_t reserved_59_63               : 5;
#endif
	} s;
	struct cvmx_pko_reg_loopback_bpid_s   cn68xx;
	struct cvmx_pko_reg_loopback_bpid_s   cn68xxp1;
};
typedef union cvmx_pko_reg_loopback_bpid cvmx_pko_reg_loopback_bpid_t;

/**
 * cvmx_pko_reg_loopback_pkind
 *
 * Notes:
 * None.
 *
 */
union cvmx_pko_reg_loopback_pkind {
	uint64_t u64;
	struct cvmx_pko_reg_loopback_pkind_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_59_63               : 5;
	uint64_t pkind7                       : 6;  /**< Loopback port 7 port-kind */
	uint64_t reserved_52_52               : 1;
	uint64_t pkind6                       : 6;  /**< Loopback port 6 port-kind */
	uint64_t reserved_45_45               : 1;
	uint64_t pkind5                       : 6;  /**< Loopback port 5 port-kind */
	uint64_t reserved_38_38               : 1;
	uint64_t pkind4                       : 6;  /**< Loopback port 4 port-kind */
	uint64_t reserved_31_31               : 1;
	uint64_t pkind3                       : 6;  /**< Loopback port 3 port-kind */
	uint64_t reserved_24_24               : 1;
	uint64_t pkind2                       : 6;  /**< Loopback port 2 port-kind */
	uint64_t reserved_17_17               : 1;
	uint64_t pkind1                       : 6;  /**< Loopback port 1 port-kind */
	uint64_t reserved_10_10               : 1;
	uint64_t pkind0                       : 6;  /**< Loopback port 0 port-kind */
	uint64_t num_ports                    : 4;  /**< Number of loopback ports, 0 <= NUM_PORTS <= 8 */
#else
	uint64_t num_ports                    : 4;
	uint64_t pkind0                       : 6;
	uint64_t reserved_10_10               : 1;
	uint64_t pkind1                       : 6;
	uint64_t reserved_17_17               : 1;
	uint64_t pkind2                       : 6;
	uint64_t reserved_24_24               : 1;
	uint64_t pkind3                       : 6;
	uint64_t reserved_31_31               : 1;
	uint64_t pkind4                       : 6;
	uint64_t reserved_38_38               : 1;
	uint64_t pkind5                       : 6;
	uint64_t reserved_45_45               : 1;
	uint64_t pkind6                       : 6;
	uint64_t reserved_52_52               : 1;
	uint64_t pkind7                       : 6;
	uint64_t reserved_59_63               : 5;
#endif
	} s;
	struct cvmx_pko_reg_loopback_pkind_s  cn68xx;
	struct cvmx_pko_reg_loopback_pkind_s  cn68xxp1;
};
typedef union cvmx_pko_reg_loopback_pkind cvmx_pko_reg_loopback_pkind_t;

/**
 * cvmx_pko_reg_min_pkt
 *
 * Notes:
 * This CSR is used with PKO_MEM_IPORT_PTRS[MIN_PKT] to select the minimum packet size.  Packets whose
 * size in bytes < (SIZEn+1) are zero-padded to (SIZEn+1) bytes.  Note that this does not include CRC bytes.
 * SIZE0=0 is read-only and is used when no padding is desired.
 */
union cvmx_pko_reg_min_pkt {
	uint64_t u64;
	struct cvmx_pko_reg_min_pkt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t size7                        : 8;  /**< Minimum packet size-1 in bytes                                NS */
	uint64_t size6                        : 8;  /**< Minimum packet size-1 in bytes                                NS */
	uint64_t size5                        : 8;  /**< Minimum packet size-1 in bytes                                NS */
	uint64_t size4                        : 8;  /**< Minimum packet size-1 in bytes                                NS */
	uint64_t size3                        : 8;  /**< Minimum packet size-1 in bytes                                NS */
	uint64_t size2                        : 8;  /**< Minimum packet size-1 in bytes                                NS */
	uint64_t size1                        : 8;  /**< Minimum packet size-1 in bytes                                NS */
	uint64_t size0                        : 8;  /**< Minimum packet size-1 in bytes                                NS */
#else
	uint64_t size0                        : 8;
	uint64_t size1                        : 8;
	uint64_t size2                        : 8;
	uint64_t size3                        : 8;
	uint64_t size4                        : 8;
	uint64_t size5                        : 8;
	uint64_t size6                        : 8;
	uint64_t size7                        : 8;
#endif
	} s;
	struct cvmx_pko_reg_min_pkt_s         cn68xx;
	struct cvmx_pko_reg_min_pkt_s         cn68xxp1;
};
typedef union cvmx_pko_reg_min_pkt cvmx_pko_reg_min_pkt_t;

/**
 * cvmx_pko_reg_preempt
 */
union cvmx_pko_reg_preempt {
	uint64_t u64;
	struct cvmx_pko_reg_preempt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t min_size                     : 16; /**< Threshhold for packet preemption, measured in bytes.
                                                         Only packets which have at least MIN_SIZE bytes
                                                         remaining to be read can be preempted. */
#else
	uint64_t min_size                     : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_pko_reg_preempt_s         cn52xx;
	struct cvmx_pko_reg_preempt_s         cn52xxp1;
	struct cvmx_pko_reg_preempt_s         cn56xx;
	struct cvmx_pko_reg_preempt_s         cn56xxp1;
	struct cvmx_pko_reg_preempt_s         cn61xx;
	struct cvmx_pko_reg_preempt_s         cn63xx;
	struct cvmx_pko_reg_preempt_s         cn63xxp1;
	struct cvmx_pko_reg_preempt_s         cn66xx;
	struct cvmx_pko_reg_preempt_s         cn68xx;
	struct cvmx_pko_reg_preempt_s         cn68xxp1;
	struct cvmx_pko_reg_preempt_s         cn70xx;
	struct cvmx_pko_reg_preempt_s         cnf71xx;
};
typedef union cvmx_pko_reg_preempt cvmx_pko_reg_preempt_t;

/**
 * cvmx_pko_reg_queue_mode
 *
 * Notes:
 * Sets the number of queues and amount of local storage per queue
 * The system has a total of 256 queues and (256*8) words of local command storage.  This CSR sets the
 * number of queues that are used.  Increasing the value of MODE by 1 decreases the number of queues
 * by a power of 2 and increases the local storage per queue by a power of 2.
 * MODEn queues storage/queue
 * 0     256     64B ( 8 words)
 * 1     128    128B (16 words)
 * 2      64    256B (32 words)
 */
union cvmx_pko_reg_queue_mode {
	uint64_t u64;
	struct cvmx_pko_reg_queue_mode_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t mode                         : 2;  /**< # of queues = 256 >> MODE, 0 <= MODE <=2 */
#else
	uint64_t mode                         : 2;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_pko_reg_queue_mode_s      cn30xx;
	struct cvmx_pko_reg_queue_mode_s      cn31xx;
	struct cvmx_pko_reg_queue_mode_s      cn38xx;
	struct cvmx_pko_reg_queue_mode_s      cn38xxp2;
	struct cvmx_pko_reg_queue_mode_s      cn50xx;
	struct cvmx_pko_reg_queue_mode_s      cn52xx;
	struct cvmx_pko_reg_queue_mode_s      cn52xxp1;
	struct cvmx_pko_reg_queue_mode_s      cn56xx;
	struct cvmx_pko_reg_queue_mode_s      cn56xxp1;
	struct cvmx_pko_reg_queue_mode_s      cn58xx;
	struct cvmx_pko_reg_queue_mode_s      cn58xxp1;
	struct cvmx_pko_reg_queue_mode_s      cn61xx;
	struct cvmx_pko_reg_queue_mode_s      cn63xx;
	struct cvmx_pko_reg_queue_mode_s      cn63xxp1;
	struct cvmx_pko_reg_queue_mode_s      cn66xx;
	struct cvmx_pko_reg_queue_mode_s      cn68xx;
	struct cvmx_pko_reg_queue_mode_s      cn68xxp1;
	struct cvmx_pko_reg_queue_mode_s      cn70xx;
	struct cvmx_pko_reg_queue_mode_s      cnf71xx;
};
typedef union cvmx_pko_reg_queue_mode cvmx_pko_reg_queue_mode_t;

/**
 * cvmx_pko_reg_queue_preempt
 *
 * Notes:
 * Per QID, setting both PREEMPTER=1 and PREEMPTEE=1 is illegal and sets only PREEMPTER=1.
 * This CSR is used with PKO_MEM_QUEUE_PTRS and PKO_REG_QUEUE_PTRS1.  When programming queues, the
 * programming sequence must first write PKO_REG_QUEUE_PREEMPT, then PKO_REG_QUEUE_PTRS1 and then
 * PKO_MEM_QUEUE_PTRS for each queue.  Preemption is supported only on queues that are ultimately
 * mapped to engines 0-7.  It is illegal to set preemptee or preempter for a queue that is ultimately
 * mapped to engines 8-11.
 *
 * Also, PKO_REG_ENGINE_INFLIGHT must be at least 2 for any engine on which preemption is enabled.
 *
 * See the descriptions of PKO_MEM_QUEUE_PTRS for further explanation of queue programming.
 */
union cvmx_pko_reg_queue_preempt {
	uint64_t u64;
	struct cvmx_pko_reg_queue_preempt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t preemptee                    : 1;  /**< Allow this QID to be preempted.
                                                         0=cannot be preempted, 1=can be preempted */
	uint64_t preempter                    : 1;  /**< Preempts the servicing of packet on PID to
                                                         allow this QID immediate servicing.  0=do not cause
                                                         preemption, 1=cause preemption.  Per PID, at most
                                                         1 QID can have this bit set. */
#else
	uint64_t preempter                    : 1;
	uint64_t preemptee                    : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_pko_reg_queue_preempt_s   cn52xx;
	struct cvmx_pko_reg_queue_preempt_s   cn52xxp1;
	struct cvmx_pko_reg_queue_preempt_s   cn56xx;
	struct cvmx_pko_reg_queue_preempt_s   cn56xxp1;
	struct cvmx_pko_reg_queue_preempt_s   cn61xx;
	struct cvmx_pko_reg_queue_preempt_s   cn63xx;
	struct cvmx_pko_reg_queue_preempt_s   cn63xxp1;
	struct cvmx_pko_reg_queue_preempt_s   cn66xx;
	struct cvmx_pko_reg_queue_preempt_s   cn68xx;
	struct cvmx_pko_reg_queue_preempt_s   cn68xxp1;
	struct cvmx_pko_reg_queue_preempt_s   cn70xx;
	struct cvmx_pko_reg_queue_preempt_s   cnf71xx;
};
typedef union cvmx_pko_reg_queue_preempt cvmx_pko_reg_queue_preempt_t;

/**
 * cvmx_pko_reg_queue_ptrs1
 *
 * Notes:
 * This CSR is used with PKO_MEM_QUEUE_PTRS and PKO_MEM_QUEUE_QOS to allow access to queues 128-255
 * and to allow up mapping of up to 16 queues per port.  When programming queues 128-255, the
 * programming sequence must first write PKO_REG_QUEUE_PTRS1 and then write PKO_MEM_QUEUE_PTRS or
 * PKO_MEM_QUEUE_QOS for each queue.
 * See the descriptions of PKO_MEM_QUEUE_PTRS and PKO_MEM_QUEUE_QOS for further explanation of queue
 * programming.
 */
union cvmx_pko_reg_queue_ptrs1 {
	uint64_t u64;
	struct cvmx_pko_reg_queue_ptrs1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t idx3                         : 1;  /**< [3] of Index (distance from head) in the queue array */
	uint64_t qid7                         : 1;  /**< [7] of Queue ID */
#else
	uint64_t qid7                         : 1;
	uint64_t idx3                         : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_pko_reg_queue_ptrs1_s     cn50xx;
	struct cvmx_pko_reg_queue_ptrs1_s     cn52xx;
	struct cvmx_pko_reg_queue_ptrs1_s     cn52xxp1;
	struct cvmx_pko_reg_queue_ptrs1_s     cn56xx;
	struct cvmx_pko_reg_queue_ptrs1_s     cn56xxp1;
	struct cvmx_pko_reg_queue_ptrs1_s     cn58xx;
	struct cvmx_pko_reg_queue_ptrs1_s     cn58xxp1;
	struct cvmx_pko_reg_queue_ptrs1_s     cn61xx;
	struct cvmx_pko_reg_queue_ptrs1_s     cn63xx;
	struct cvmx_pko_reg_queue_ptrs1_s     cn63xxp1;
	struct cvmx_pko_reg_queue_ptrs1_s     cn66xx;
	struct cvmx_pko_reg_queue_ptrs1_s     cn70xx;
	struct cvmx_pko_reg_queue_ptrs1_s     cnf71xx;
};
typedef union cvmx_pko_reg_queue_ptrs1 cvmx_pko_reg_queue_ptrs1_t;

/**
 * cvmx_pko_reg_read_idx
 *
 * Notes:
 * Provides the read index during a CSR read operation to any of the CSRs that are physically stored
 * as memories.  The names of these CSRs begin with the prefix "PKO_MEM_".
 * IDX[7:0] is the read index.  INC[7:0] is an increment that is added to IDX[7:0] after any CSR read.
 * The intended use is to initially write this CSR such that IDX=0 and INC=1.  Then, the entire
 * contents of a CSR memory can be read with consecutive CSR read commands.
 */
union cvmx_pko_reg_read_idx {
	uint64_t u64;
	struct cvmx_pko_reg_read_idx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t inc                          : 8;  /**< Increment to add to current index for next index */
	uint64_t index                        : 8;  /**< Index to use for next memory CSR read */
#else
	uint64_t index                        : 8;
	uint64_t inc                          : 8;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_pko_reg_read_idx_s        cn30xx;
	struct cvmx_pko_reg_read_idx_s        cn31xx;
	struct cvmx_pko_reg_read_idx_s        cn38xx;
	struct cvmx_pko_reg_read_idx_s        cn38xxp2;
	struct cvmx_pko_reg_read_idx_s        cn50xx;
	struct cvmx_pko_reg_read_idx_s        cn52xx;
	struct cvmx_pko_reg_read_idx_s        cn52xxp1;
	struct cvmx_pko_reg_read_idx_s        cn56xx;
	struct cvmx_pko_reg_read_idx_s        cn56xxp1;
	struct cvmx_pko_reg_read_idx_s        cn58xx;
	struct cvmx_pko_reg_read_idx_s        cn58xxp1;
	struct cvmx_pko_reg_read_idx_s        cn61xx;
	struct cvmx_pko_reg_read_idx_s        cn63xx;
	struct cvmx_pko_reg_read_idx_s        cn63xxp1;
	struct cvmx_pko_reg_read_idx_s        cn66xx;
	struct cvmx_pko_reg_read_idx_s        cn68xx;
	struct cvmx_pko_reg_read_idx_s        cn68xxp1;
	struct cvmx_pko_reg_read_idx_s        cn70xx;
	struct cvmx_pko_reg_read_idx_s        cnf71xx;
};
typedef union cvmx_pko_reg_read_idx cvmx_pko_reg_read_idx_t;

/**
 * cvmx_pko_reg_throttle
 *
 * Notes:
 * This CSR is used with PKO_MEM_THROTTLE_PIPE and PKO_MEM_THROTTLE_INT.  INT_MASK corresponds to the
 * interfaces listed in the description for PKO_MEM_IPORT_PTRS[INT].  Set INT_MASK[N] to enable the
 * updating of PKO_MEM_THROTTLE_PIPE and PKO_MEM_THROTTLE_INT counts for packets destined for
 * interface N.  INT_MASK has no effect on the updates caused by CSR writes to PKO_MEM_THROTTLE_PIPE
 * and PKO_MEM_THROTTLE_INT.  Note that this does not disable the throttle logic, just the updating of
 * the interface counts.
 */
union cvmx_pko_reg_throttle {
	uint64_t u64;
	struct cvmx_pko_reg_throttle_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t int_mask                     : 32; /**< Mask to enable THROTTLE count updates per interface           NS */
#else
	uint64_t int_mask                     : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_pko_reg_throttle_s        cn68xx;
	struct cvmx_pko_reg_throttle_s        cn68xxp1;
};
typedef union cvmx_pko_reg_throttle cvmx_pko_reg_throttle_t;

/**
 * cvmx_pko_reg_timestamp
 *
 * Notes:
 * None.
 *
 */
union cvmx_pko_reg_timestamp {
	uint64_t u64;
	struct cvmx_pko_reg_timestamp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t wqe_word                     : 4;  /**< Specifies the 8-byte word in the WQE to which a PTP
                                                         timestamp is written.  Values 0 and 1 are illegal. */
#else
	uint64_t wqe_word                     : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_pko_reg_timestamp_s       cn61xx;
	struct cvmx_pko_reg_timestamp_s       cn63xx;
	struct cvmx_pko_reg_timestamp_s       cn63xxp1;
	struct cvmx_pko_reg_timestamp_s       cn66xx;
	struct cvmx_pko_reg_timestamp_s       cn68xx;
	struct cvmx_pko_reg_timestamp_s       cn68xxp1;
	struct cvmx_pko_reg_timestamp_s       cn70xx;
	struct cvmx_pko_reg_timestamp_s       cnf71xx;
};
typedef union cvmx_pko_reg_timestamp cvmx_pko_reg_timestamp_t;

/**
 * cvmx_pko_status
 */
union cvmx_pko_status {
	uint64_t u64;
	struct cvmx_pko_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pko_rdy                      : 1;  /**< PKO ready for configuration. */
	uint64_t reserved_24_62               : 39;
	uint64_t c2qlut_rdy                   : 1;  /**< PKO C2Q LUT block ready for configuration. */
	uint64_t ppfi_rdy                     : 1;  /**< PKO PPFI block ready for configuration. */
	uint64_t iobp1_rdy                    : 1;  /**< PKO IOBP1 block ready for configuration. */
	uint64_t ncb_rdy                      : 1;  /**< PKO NCB block ready for configuration. */
	uint64_t pse_rdy                      : 1;  /**< PKO PSE block ready for configuration. */
	uint64_t pdm_rdy                      : 1;  /**< PKO PDM block ready for configuration. */
	uint64_t peb_rdy                      : 1;  /**< PKO PEB block ready for configuration. */
	uint64_t csi_rdy                      : 1;  /**< PKO CSI block ready for configuration. */
	uint64_t reserved_5_15                : 11;
	uint64_t ncb_bist_status              : 1;  /**< PKO NCB block BIST status. 0 = BIST passed; 1 = BIST failed. */
	uint64_t c2qlut_bist_status           : 1;  /**< PKO C2QLUT block BIST status. 0 = BIST passed; 1 = BIST failed. */
	uint64_t pdm_bist_status              : 1;  /**< PKO PDM block BIST status. 0 = BIST passed; 1 = BIST failed. */
	uint64_t peb_bist_status              : 1;  /**< PKO PEB block BIST status. 0 = BIST passed; 1 = BIST failed. */
	uint64_t pse_bist_status              : 1;  /**< PKO PSE block BIST status. 0 = BIST passed; 1 = BIST failed. */
#else
	uint64_t pse_bist_status              : 1;
	uint64_t peb_bist_status              : 1;
	uint64_t pdm_bist_status              : 1;
	uint64_t c2qlut_bist_status           : 1;
	uint64_t ncb_bist_status              : 1;
	uint64_t reserved_5_15                : 11;
	uint64_t csi_rdy                      : 1;
	uint64_t peb_rdy                      : 1;
	uint64_t pdm_rdy                      : 1;
	uint64_t pse_rdy                      : 1;
	uint64_t ncb_rdy                      : 1;
	uint64_t iobp1_rdy                    : 1;
	uint64_t ppfi_rdy                     : 1;
	uint64_t c2qlut_rdy                   : 1;
	uint64_t reserved_24_62               : 39;
	uint64_t pko_rdy                      : 1;
#endif
	} s;
	struct cvmx_pko_status_s              cn78xx;
};
typedef union cvmx_pko_status cvmx_pko_status_t;

#endif
