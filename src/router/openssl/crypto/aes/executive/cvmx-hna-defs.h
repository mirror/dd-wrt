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
 * cvmx-hna-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon hna.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_HNA_DEFS_H__
#define __CVMX_HNA_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_BIST0 CVMX_HNA_BIST0_FUNC()
static inline uint64_t CVMX_HNA_BIST0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_BIST0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800470007F0ull);
}
#else
#define CVMX_HNA_BIST0 (CVMX_ADD_IO_SEG(0x00011800470007F0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_BIST1 CVMX_HNA_BIST1_FUNC()
static inline uint64_t CVMX_HNA_BIST1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_BIST1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800470007F8ull);
}
#else
#define CVMX_HNA_BIST1 (CVMX_ADD_IO_SEG(0x00011800470007F8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_CONFIG CVMX_HNA_CONFIG_FUNC()
static inline uint64_t CVMX_HNA_CONFIG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_CONFIG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000000ull);
}
#else
#define CVMX_HNA_CONFIG (CVMX_ADD_IO_SEG(0x0001180047000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_CONTROL CVMX_HNA_CONTROL_FUNC()
static inline uint64_t CVMX_HNA_CONTROL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_CONTROL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000020ull);
}
#else
#define CVMX_HNA_CONTROL (CVMX_ADD_IO_SEG(0x0001180047000020ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_DBELL CVMX_HNA_DBELL_FUNC()
static inline uint64_t CVMX_HNA_DBELL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_DBELL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001470000000000ull);
}
#else
#define CVMX_HNA_DBELL (CVMX_ADD_IO_SEG(0x0001470000000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_DIFCTL CVMX_HNA_DIFCTL_FUNC()
static inline uint64_t CVMX_HNA_DIFCTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_DIFCTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001470600000000ull);
}
#else
#define CVMX_HNA_DIFCTL (CVMX_ADD_IO_SEG(0x0001470600000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_DIFRDPTR CVMX_HNA_DIFRDPTR_FUNC()
static inline uint64_t CVMX_HNA_DIFRDPTR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_DIFRDPTR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001470200000000ull);
}
#else
#define CVMX_HNA_DIFRDPTR (CVMX_ADD_IO_SEG(0x0001470200000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_ERROR CVMX_HNA_ERROR_FUNC()
static inline uint64_t CVMX_HNA_ERROR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_ERROR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000028ull);
}
#else
#define CVMX_HNA_ERROR (CVMX_ADD_IO_SEG(0x0001180047000028ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_HPU_CSR CVMX_HNA_HPU_CSR_FUNC()
static inline uint64_t CVMX_HNA_HPU_CSR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_HPU_CSR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000010ull);
}
#else
#define CVMX_HNA_HPU_CSR (CVMX_ADD_IO_SEG(0x0001180047000010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_HPU_DBG CVMX_HNA_HPU_DBG_FUNC()
static inline uint64_t CVMX_HNA_HPU_DBG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_HPU_DBG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000008ull);
}
#else
#define CVMX_HNA_HPU_DBG (CVMX_ADD_IO_SEG(0x0001180047000008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_HPU_EIR CVMX_HNA_HPU_EIR_FUNC()
static inline uint64_t CVMX_HNA_HPU_EIR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_HPU_EIR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000018ull);
}
#else
#define CVMX_HNA_HPU_EIR (CVMX_ADD_IO_SEG(0x0001180047000018ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_PFC0_CNT CVMX_HNA_PFC0_CNT_FUNC()
static inline uint64_t CVMX_HNA_PFC0_CNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_PFC0_CNT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000090ull);
}
#else
#define CVMX_HNA_PFC0_CNT (CVMX_ADD_IO_SEG(0x0001180047000090ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_PFC0_CTL CVMX_HNA_PFC0_CTL_FUNC()
static inline uint64_t CVMX_HNA_PFC0_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_PFC0_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000088ull);
}
#else
#define CVMX_HNA_PFC0_CTL (CVMX_ADD_IO_SEG(0x0001180047000088ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_PFC1_CNT CVMX_HNA_PFC1_CNT_FUNC()
static inline uint64_t CVMX_HNA_PFC1_CNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_PFC1_CNT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800470000A0ull);
}
#else
#define CVMX_HNA_PFC1_CNT (CVMX_ADD_IO_SEG(0x00011800470000A0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_PFC1_CTL CVMX_HNA_PFC1_CTL_FUNC()
static inline uint64_t CVMX_HNA_PFC1_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_PFC1_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000098ull);
}
#else
#define CVMX_HNA_PFC1_CTL (CVMX_ADD_IO_SEG(0x0001180047000098ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_PFC2_CNT CVMX_HNA_PFC2_CNT_FUNC()
static inline uint64_t CVMX_HNA_PFC2_CNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_PFC2_CNT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800470000B0ull);
}
#else
#define CVMX_HNA_PFC2_CNT (CVMX_ADD_IO_SEG(0x00011800470000B0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_PFC2_CTL CVMX_HNA_PFC2_CTL_FUNC()
static inline uint64_t CVMX_HNA_PFC2_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_PFC2_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800470000A8ull);
}
#else
#define CVMX_HNA_PFC2_CTL (CVMX_ADD_IO_SEG(0x00011800470000A8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_PFC3_CNT CVMX_HNA_PFC3_CNT_FUNC()
static inline uint64_t CVMX_HNA_PFC3_CNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_PFC3_CNT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800470000C0ull);
}
#else
#define CVMX_HNA_PFC3_CNT (CVMX_ADD_IO_SEG(0x00011800470000C0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_PFC3_CTL CVMX_HNA_PFC3_CTL_FUNC()
static inline uint64_t CVMX_HNA_PFC3_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_PFC3_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800470000B8ull);
}
#else
#define CVMX_HNA_PFC3_CTL (CVMX_ADD_IO_SEG(0x00011800470000B8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_PFC_GCTL CVMX_HNA_PFC_GCTL_FUNC()
static inline uint64_t CVMX_HNA_PFC_GCTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_PFC_GCTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000080ull);
}
#else
#define CVMX_HNA_PFC_GCTL (CVMX_ADD_IO_SEG(0x0001180047000080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_SBD_DBG0 CVMX_HNA_SBD_DBG0_FUNC()
static inline uint64_t CVMX_HNA_SBD_DBG0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_SBD_DBG0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000040ull);
}
#else
#define CVMX_HNA_SBD_DBG0 (CVMX_ADD_IO_SEG(0x0001180047000040ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_SBD_DBG1 CVMX_HNA_SBD_DBG1_FUNC()
static inline uint64_t CVMX_HNA_SBD_DBG1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_SBD_DBG1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000048ull);
}
#else
#define CVMX_HNA_SBD_DBG1 (CVMX_ADD_IO_SEG(0x0001180047000048ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_SBD_DBG2 CVMX_HNA_SBD_DBG2_FUNC()
static inline uint64_t CVMX_HNA_SBD_DBG2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_SBD_DBG2 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000050ull);
}
#else
#define CVMX_HNA_SBD_DBG2 (CVMX_ADD_IO_SEG(0x0001180047000050ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_HNA_SBD_DBG3 CVMX_HNA_SBD_DBG3_FUNC()
static inline uint64_t CVMX_HNA_SBD_DBG3_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_HNA_SBD_DBG3 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180047000058ull);
}
#else
#define CVMX_HNA_SBD_DBG3 (CVMX_ADD_IO_SEG(0x0001180047000058ull))
#endif

/**
 * cvmx_hna_bist0
 *
 * Description:
 *
 */
union cvmx_hna_bist0 {
	uint64_t u64;
	struct cvmx_hna_bist0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t hpc3                         : 12; /**< Bist Results for HPC3 RAM(s) (per-HPU)
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t reserved_44_47               : 4;
	uint64_t hpc2                         : 12; /**< Bist Results for HPC2 RAM(s) (per-HPU)
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t reserved_28_31               : 4;
	uint64_t hpc1                         : 12; /**< Bist Results for HPC1 RAM(s) (per-HPU)
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t reserved_12_15               : 4;
	uint64_t hpc0                         : 12; /**< Bist Results for HPC0 RAM(s) (per-HPU)
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
#else
	uint64_t hpc0                         : 12;
	uint64_t reserved_12_15               : 4;
	uint64_t hpc1                         : 12;
	uint64_t reserved_28_31               : 4;
	uint64_t hpc2                         : 12;
	uint64_t reserved_44_47               : 4;
	uint64_t hpc3                         : 12;
	uint64_t reserved_60_63               : 4;
#endif
	} s;
	struct cvmx_hna_bist0_s               cn78xx;
};
typedef union cvmx_hna_bist0 cvmx_hna_bist0_t;

/**
 * cvmx_hna_bist1
 *
 * Description:
 *
 */
union cvmx_hna_bist1 {
	uint64_t u64;
	struct cvmx_hna_bist1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t hnc1                         : 1;  /**< "SC1 Bist Results for cumulative HNC1 RAMs
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD" */
	uint64_t hnc0                         : 1;  /**< "SC0 Bist Results for cumulative HNC0 RAMs
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD" */
	uint64_t mrp1                         : 1;  /**< Bist Results for DSM-DLC:MRP1 RAM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t mrp0                         : 1;  /**< Bist Results for DSM-DLC:MRP0 RAM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t reserved_1_2                 : 2;
	uint64_t gib                          : 1;  /**< Bist Results for GIB RAM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
#else
	uint64_t gib                          : 1;
	uint64_t reserved_1_2                 : 2;
	uint64_t mrp0                         : 1;
	uint64_t mrp1                         : 1;
	uint64_t hnc0                         : 1;
	uint64_t hnc1                         : 1;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_hna_bist1_s               cn78xx;
};
typedef union cvmx_hna_bist1 cvmx_hna_bist1_t;

/**
 * cvmx_hna_config
 *
 * This register specifies the HNA HPU programmable controls.
 *
 */
union cvmx_hna_config {
	uint64_t u64;
	struct cvmx_hna_config_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t stk_ll_dis                   : 1;  /**< Stack Linked-List Disable.
                                                         When set, the linked-list mechanism for run stack
                                                         and save stack structures will be disabled.  In this mode,
                                                         the linked-list chunk boundary checking is not done, and
                                                         therefore the previous/next pointers are non-existent.  The
                                                         stacks are effectively in an infinite linear buffer, bounded
                                                         only by the maximum sizes provided in the instruction
                                                         (IWORD3[RUNSTACKSZ] and IWORD6[SVSTACKSZ]).  There is no
                                                         space reserved for the previous and next pointers, and
                                                         [STK_CHKSZ] will be ignored.
                                                         When the STK_LL_DIS is cleared, the stack linked-list mechanism
                                                         will operate as per spec. */
	uint64_t reserved_23_23               : 1;
	uint64_t stk_chksz                    : 3;  /**< Stack Chunk Size
                                                          This encoded value specifies the chunk size for both the RNSTK/SVSTK data structures.
                                                          The RNSTK/SVSTK use a doubly linked list where EACH Chunk's first two 64bit
                                                          entries contain the PREVIOUS and NEXT chunk pointers.
                                                         - 0: 32 entries or 256 bytes
                                                         - 1: 64 entries or 512 Bytes
                                                         - 2: 128 entries or 1K bytes
                                                         - 3: 256 entries or 2K bytes    <= DEFAULT power on
                                                         - 4: 512 entries or 4K bytes
                                                         - 5: 1024 entries or 8K bytes
                                                         - 6: 2048 entries or 16K bytes
                                                         - 7: 4096 entries or 32K bytes
                                                          NOTE: This field can only be changed at initialization/power on time before
                                                          the HNA is fed instructions. */
	uint64_t rnstk_lwm                    : 4;  /**< "RNSTK Low Water Mark
                                                         This field specifies the low watermark for the run stack. Valid Range: [0..15]
                                                         Once the run stack goes below the low water mark, HNA will fill entries from the
                                                         global run stack head to the local run stack tail.
                                                         The granularity of this field is represented as number of 128B cachelines.
                                                         NOTE: This field can only be changed at initialization/power on time before
                                                         the HNA is fed instructions." */
	uint64_t rnstk_hwm                    : 4;  /**< "RNSTK High Water Mark
                                                         This field specifies the hi watermark for the run stack. Valid Range: [0..15]
                                                         Once the local run stack level goes above the hi water mark, the HNA will spill
                                                         entries from the local run stack tail to the global run stack head (in DDR memory).
                                                         The granularity of this field is represented as number of 128B cachelines.
                                                         NOTE: This field can only be changed at initialization/power on time before
                                                         the HNA is fed instructions." */
	uint64_t reserved_9_11                : 3;
	uint64_t ecccordis                    : 1;  /**< ECC Correction Disable
                                                         When set, all HNA ECC protected data structures will disable their ECC correction
                                                         logic. When clear (default) ECC correction is always enabled. */
	uint64_t clmskcrip                    : 4;  /**< Cluster Cripple Mask
                                                         A one in each bit of the mask represents which HPC cluster to
                                                         cripple. o78 HNA has 4 clusters, where all CLMSKCRIP mask bits are used.
                                                         SWNOTE: The MIO_FUS___HNA_CLMASK_CRIPPLE[3:0] fuse bits will
                                                         be forced into this register at reset. Any fuse bits that
                                                         contain '1' will be disallowed during a write and will always
                                                         be read as '1'. */
	uint64_t hpu_clcrip                   : 3;  /**< "HPU Cluster Cripple
                                                         Encoding which represents number of HPUs to cripple for each
                                                         cluster. Typically HPU_CLCRIP=0 which enables all HPUs
                                                         within each cluster. However, when the HNA performance
                                                         counters are used, SW may want to limit the number of HPUs
                                                         per cluster available, as there are only 4 parallel
                                                         performance counters.
                                                         HPU_CLCRIP | \#HPUs crippled(per cluster)
                                                         -----------+-----------------------------
                                                            0       |  0      HPU[9:0]:ON                   All engines enabled
                                                            1       |  1      HPU[9]:OFF    /HPU[8:0]:ON    (n-1) engines enabled
                                                            2       |  3      HPU[9:7]:OFF  /HPU[6:0]:ON    (n-3) engines enabled
                                                            3       |  4      HPU[9:6]:OFF  /HPU[5:0]:ON    (n-4) engines enabled
                                                            4       |  5      HPU[9:5]:OFF  /HPU[4:0]:ON    (n-5) engines enabled
                                                            5       |  6      HPU[9:4]:OFF  /HPU[3:0]:ON    (n-6) engines enabled
                                                            6       |  8      HPU[9:2]:OFF  /HPU[1:0]:ON    (n-8) engines enabled
                                                            7       |  9      HPU[9:1]:OFF  /HPU[0]:ON      (n-9) single engine enabled
                                                         NOTE: Higher numbered HPUs are crippled first. For instance,
                                                         on o78 (with 10 HPUs/cluster), if HPU_CLCRIP=0x1, then
                                                         HPU#s [9] within the cluster are crippled and only
                                                         HPU#s [8:0] are available.
                                                         IMPNOTE: The encodings are done in such a way as to later
                                                         be used with fuses (for future revisions which will disable
                                                         some number of HPUs). Blowing a fuse has the effect that there will
                                                         always be fewer HPUs available. [ie: we never want a customer
                                                         to blow additional fuses to get more HPUs].
                                                         SWNOTE: The MIO_FUS___HNA_NUMHPU_CRIPPLE[2:0] fuse bits will
                                                         be forced into this register at reset. Any fuse bits that
                                                         contain '1' will be disallowed during a write and will always
                                                         be read as '1'." */
	uint64_t hpuclkdis                    : 1;  /**< HNA Clock Disable Source
                                                         When SET, the HNA clocks for HPU(thread engine)
                                                         operation are disabled (to conserve overall chip clocking
                                                         power when the HNA function is not used).
                                                         NOTE: When SET, SW MUST NEVER issue NCB-Direct CSR
                                                         operations to the HNA (will result in NCB Bus Timeout
                                                         errors).
                                                         NOTE: This should only be written to a different value
                                                         during power-on SW initialization.
                                                         SWNOTE: The MIO_FUS___HNA_HPU_DISABLE fuse bit will
                                                         be forced into this register at reset. If the fuse bit
                                                         contains '1', writes to HPUCLKDIS are disallowed and
                                                         will always be read as '1'. */
#else
	uint64_t hpuclkdis                    : 1;
	uint64_t hpu_clcrip                   : 3;
	uint64_t clmskcrip                    : 4;
	uint64_t ecccordis                    : 1;
	uint64_t reserved_9_11                : 3;
	uint64_t rnstk_hwm                    : 4;
	uint64_t rnstk_lwm                    : 4;
	uint64_t stk_chksz                    : 3;
	uint64_t reserved_23_23               : 1;
	uint64_t stk_ll_dis                   : 1;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_hna_config_s              cn78xx;
};
typedef union cvmx_hna_config cvmx_hna_config_t;

/**
 * cvmx_hna_control
 *
 * This register specifies the HNA CTL/HNC programmable controls.
 *
 */
union cvmx_hna_control {
	uint64_t u64;
	struct cvmx_hna_control_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_13_63               : 51;
	uint64_t frcperr                      : 1;  /**< Force Parity Error during CLOAD (HNC-write)
                                                         When SET, a parity error is forced during the HNC CLOAD
                                                         instruction. SW can force a single line in the HNC to contain
                                                         a parity error by setting this bit and performance a CLOAD
                                                         for a single line (DLEN=32), then clearing the bit. */
	uint64_t sbdnum                       : 6;  /**< "SBD Debug Entry#
                                                         INTERNAL:
                                                         HNA Scoreboard debug control
                                                         Selects which one of 48 HNA Scoreboard entries is
                                                         latched into the HNA_SBD_DBG[0-3] registers." */
	uint64_t sbdlck                       : 1;  /**< HNA Scoreboard LOCK Strobe
                                                         INTERNAL:
                                                         HNA Scoreboard debug control
                                                         When written with a '1', the HNA Scoreboard Debug
                                                         registers (HNA_SBD_DBG[0-3]) are all locked down.
                                                         This allows SW to lock down the contents of the entire
                                                         SBD for a single instant in time. All subsequent reads
                                                         of the HNA scoreboard registers will return the data
                                                         from that instant in time. */
	uint64_t reserved_3_4                 : 2;
	uint64_t pmode                        : 1;  /**< NCB-NRP Arbiter Mode
                                                         (0=Fixed Priority [LP=DFF,HP=RGF]/1=RR
                                                         NOTE: This should only be written to a different value
                                                         during power-on SW initialization. */
	uint64_t qmode                        : 1;  /**< NCB-NRQ Arbiter Mode
                                                         (0=Fixed Priority [LP=NPF,IRF,WRF,PRF,RSRF,HP=SLL]/1=RR
                                                         NOTE: This should only be written to a different value
                                                         during power-on SW initialization. */
	uint64_t imode                        : 1;  /**< NCB-Inbound Arbiter
                                                         (0=FP [LP=NRQ,HP=NRP], 1=RR)
                                                         NOTE: This should only be written to a different value
                                                         during power-on SW initialization. */
#else
	uint64_t imode                        : 1;
	uint64_t qmode                        : 1;
	uint64_t pmode                        : 1;
	uint64_t reserved_3_4                 : 2;
	uint64_t sbdlck                       : 1;
	uint64_t sbdnum                       : 6;
	uint64_t frcperr                      : 1;
	uint64_t reserved_13_63               : 51;
#endif
	} s;
	struct cvmx_hna_control_s             cn78xx;
};
typedef union cvmx_hna_control cvmx_hna_control_t;

/**
 * cvmx_hna_dbell
 *
 * Description:
 * NOTE: To write to the HNA_DBELL register, a device would issue an IOBST directed at the HNA
 * with addr[34:32] = 0x0 or 0x1.
 * To read the HNA_DBELL register, a device would issue an IOBLD64 directed at the HNA with
 * addr[34:32] = 0x0 or 0x1.
 * NOTE: If HNA_CONFIG[HPUCLKDIS]=1 (HNA-HPU clocks disabled), reads/writes to the HNA_DBELL
 * register do not take effect.
 * NOTE: If FUSE[TBD]="HNA HPU disable" is blown, reads/writes to the HNA_DBELL register do not
 * take effect.
 */
union cvmx_hna_dbell {
	uint64_t u64;
	struct cvmx_hna_dbell_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t dbell                        : 20; /**< Represents the cumulative total of pending
                                                         HNA instructions which SW has previously written
                                                         into the HNA Instruction FIFO (DIF) in main memory.
                                                         Each HNA instruction contains a fixed size 64B
                                                         instruction word which is executed by the HNA HW.
                                                         The DBELL field can hold up to 1M-1 (2^20-1)
                                                         pending HNA instruction requests.
                                                         During a read (by SW), the 'most recent' contents
                                                         of the HNA_DBELL register are returned at the time
                                                         the NCB-INB bus is driven.
                                                         NOTE: Since HNA HW updates this register, its
                                                         contents are unpredictable in SW. */
#else
	uint64_t dbell                        : 20;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_hna_dbell_s               cn78xx;
};
typedef union cvmx_hna_dbell cvmx_hna_dbell_t;

/**
 * cvmx_hna_difctl
 *
 * Description:
 * NOTE: To write to the HNA_DIFCTL register, a device would issue an IOBST directed at the HNA
 * with addr[34:32]=0x6.
 * To read the HNA_DIFCTL register, a device would issue an IOBLD64 directed at the HNA with
 * addr[34:32]=0x6.
 * NOTE: This register is intended to ONLY be written once (at power-up). Any future writes could
 * cause the HNA and FPA HW to become unpredictable.
 * NOTE: If HNA_CONFIG[HPUCLKDIS]=1 (HNA-HPU clocks disabled), reads/writes to the HNA_DIFCTL
 * register do not take effect.
 * NOTE: If FUSE[TBD]="HNA HPU disable" is blown, reads/writes to the HNA_DIFCTL register do not
 * take effect.
 */
union cvmx_hna_difctl {
	uint64_t u64;
	struct cvmx_hna_difctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t aura                         : 16; /**< Represents the 16bit Aura-id  used by HNA HW
                                                         when the HNA instruction chunk is recycled back
                                                         to the Free Page List maintained by the FPA HW
                                                         (once the HNA instruction has been issued). */
	uint64_t reserved_13_25               : 13;
	uint64_t ldwb                         : 1;  /**< Load Don't Write Back.
                                                         When set, the HW will issue LDWB command towards the cache when
                                                         fetching last word of instructions, as a result the line will not be written back when
                                                         replaced.
                                                         When clear, the HW will issue regular load towards cache which will cause
                                                         the line to be written back before being replaced. */
	uint64_t reserved_9_11                : 3;
	uint64_t size                         : 9;  /**< "Represents the number of 64B instructions contained
                                                         within each HNA instruction chunk. At Power-on,
                                                         SW will seed the SIZE register with a fixed
                                                         chunk-size. (Must be at least 3)
                                                         HNA HW uses this field to determine the size
                                                         of each HNA instruction chunk, in order to:
                                                         a) determine when to read the next HNA
                                                         instruction chunk pointer which is
                                                         written by SW at the end of the current
                                                         HNA instruction chunk (see HNA description
                                                         of next chunk buffer Ptr for format).
                                                         b) determine when a HNA instruction chunk
                                                         can be returned to the Free Page List
                                                         maintained by the FPA HW." */
#else
	uint64_t size                         : 9;
	uint64_t reserved_9_11                : 3;
	uint64_t ldwb                         : 1;
	uint64_t reserved_13_25               : 13;
	uint64_t aura                         : 16;
	uint64_t reserved_42_63               : 22;
#endif
	} s;
	struct cvmx_hna_difctl_s              cn78xx;
};
typedef union cvmx_hna_difctl cvmx_hna_difctl_t;

/**
 * cvmx_hna_difrdptr
 *
 * Description:
 * NOTE: To write to the HNA_DIFRDPTR register, a device would issue an IOBST directed at the HNA
 * with addr[34:32] = 0x2 or 0x3.
 * To read the HNA_DIFRDPTR register, a device would issue an IOBLD64 directed at the HNA with
 * addr[34:32] = 0x2 or 0x3.
 * NOTE: If HNA_CONFIG[HPUCLKDIS]=1 (HNA-HPU clocks disabled), reads/writes to the HNA_DIFRDPTR
 * register do not take effect.
 * NOTE: If FUSE[TBD]="HNA HPU disable" is blown, reads/writes to the HNA_DIFRDPTR register do
 * not take effect.
 */
union cvmx_hna_difrdptr {
	uint64_t u64;
	struct cvmx_hna_difrdptr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t rdptr                        : 36; /**< Represents the 64B-aligned address of the current
                                                         instruction in the HNA Instruction FIFO in main
                                                         memory. The RDPTR must be seeded by software at
                                                         boot time, and is then maintained thereafter
                                                         by HNA HW.
                                                         During the seed write (by SW), RDPTR[6]=0,
                                                         since HNA instruction chunks must be 128B aligned.
                                                         During a read (by SW), the 'most recent' contents
                                                         of the RDPTR register are returned at the time
                                                         the NCB-INB bus is driven.
                                                         NOTE: Since HNA HW updates this register, its
                                                         contents are unpredictable in SW (unless
                                                         its guaranteed that no new DoorBell register
                                                         writes have occurred and the DoorBell register is
                                                         read as zero). */
	uint64_t reserved_0_5                 : 6;
#else
	uint64_t reserved_0_5                 : 6;
	uint64_t rdptr                        : 36;
	uint64_t reserved_42_63               : 22;
#endif
	} s;
	struct cvmx_hna_difrdptr_s            cn78xx;
};
typedef union cvmx_hna_difrdptr cvmx_hna_difrdptr_t;

/**
 * cvmx_hna_error
 *
 * This register contains error status information.
 *
 */
union cvmx_hna_error {
	uint64_t u64;
	struct cvmx_hna_error_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t osmerr                       : 1;  /**< OSM reported an Error with the response data. */
	uint64_t replerr                      : 1;  /**< HNA Illegal Replication Factor Error
                                                         HNA only supports 1x, 2x, and 4x port replication.
                                                         Legal configurations for memory are to support 2 port or
                                                         4 port configurations.
                                                         The REPLERR interrupt will be set in the following illegal
                                                         configuration cases:
                                                         1) An 8x replication factor is detected for any memory reference.
                                                         2) A 4x replication factor is detected for any memory reference
                                                         when only 2 memory ports are enabled.
                                                         NOTE: If REPLERR is set during a HNA Graph Walk operation,
                                                         then the walk will prematurely terminate with RWORD0[REA]=ERR.
                                                         If REPLERR is set during a NCB-Direct CSR read access to HNA
                                                         Memory REGION, then the CSR read response data is UNPREDICTABLE. */
	uint64_t hnanxm                       : 1;  /**< HNA Non-existent Memory Access
                                                         HPUs (and backdoor CSR HNA Memory REGION reads)
                                                         have access to the following 40bit L2/DRAM address space
                                                         which maps to a 38bit physical DDR3 SDRAM address space [256GB(max)].
                                                         see:
                                                         DR0: 0x0 0000 0000 0000 to 0x0 0000 0FFF FFFF
                                                         maps to lower 256MB of physical DDR3 SDRAM
                                                         DR1: 0x0 0000 2000 0000 to 0x0 0020 0FFF FFFF
                                                         maps to upper 127.75GB of DDR3 SDRAM
                                                         NOTE: the 2nd 256MB HOLE maps to IO and is unused(nonexistent) for memory.
                                                         L2/DRAM address space                     Physical DDR3 SDRAM Address space
                                                         (40bit address)                           (38bit address)
                                                         +-----------+ 0x0040.0FFF.FFFF
                                                         |   DR1     |                            +-----------+ 0x003F.FFFF.FFFF
                                                         |           | (256GB-256MB)
                                                         |           |                     =>     |   DR1
                                                         +-----------+ 0x0000.1FFF.FFFF           |           | (256GB-256MB)
                                                         |   HOLE    | 256MB (DO NOT USE)
                                                         +-----------+ 0x0000.0FFF.FFFF           +-----------+ 0x0000.0FFF.FFFF
                                                         |    DR0    | 256MB                      |   DR0     | (256MB)
                                                         +-----------+ 0x0000.0000.0000           +-----------+ 0x0000.0000.0000
                                                         In the event the HNA generates a reference to the L2/DRAM
                                                         address hole (0x0000.0FFF.FFFF - 0x0000.1FFF.FFFF) the HNANXM
                                                         programmable interrupt bit will be set.
                                                         SWNOTE: Both the 1) SW HNA Graph compiler and the 2) SW NCB-Direct CSR
                                                         accesses to HNA Memory REGION MUST avoid making references
                                                         to this 2nd 256MB HOLE which is non-existent memory region.
                                                         NOTE: If HNANXM is set during a HNA Graph Walk operation,
                                                         then the walk will prematurely terminate with RWORD0[REA]=ERR.
                                                         If HNANXM is set during a NCB-Direct CSR read access to HNA
                                                         Memory REGION, then the CSR read response data is forced to
                                                         128'hBADE_FEED_DEAD_BEEF_FACE_CAFE_BEAD_C0DE. (NOTE: the QW
                                                         being accessed, either the upper or lower QW will be returned). */
	uint64_t reserved_15_16               : 2;
	uint64_t dlc1_ovferr                  : 1;  /**< DLC1 Fifo Overflow Error Detected
                                                         This condition should NEVER architecturally occur, and
                                                         is here in case HW credit/debit scheme is not working. */
	uint64_t dlc0_ovferr                  : 1;  /**< DLC0 Fifo Overflow Error Detected
                                                         This condition should NEVER architecturally occur, and
                                                         is here in case HW credit/debit scheme is not working. */
	uint64_t reserved_1_12                : 12;
	uint64_t dblovf                       : 1;  /**< Doorbell Overflow detected - Status bit
                                                         When set, the 20b accumulated doorbell register
                                                         had overflowed (SW wrote too many doorbell requests).
                                                         If the DBLINA had previously been enabled(set),
                                                         an interrupt will be posted. Software can clear
                                                         the interrupt by writing a 1 to this register bit.
                                                         NOTE: Detection of a Doorbell Register overflow
                                                         is a catastrophic error which may leave the HNA
                                                         HW in an unrecoverable state.
                                                         Throws HNA_INTSN_E::HNA_ERROR_DBLOVF. */
#else
	uint64_t dblovf                       : 1;
	uint64_t reserved_1_12                : 12;
	uint64_t dlc0_ovferr                  : 1;
	uint64_t dlc1_ovferr                  : 1;
	uint64_t reserved_15_16               : 2;
	uint64_t hnanxm                       : 1;
	uint64_t replerr                      : 1;
	uint64_t osmerr                       : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_hna_error_s               cn78xx;
};
typedef union cvmx_hna_error cvmx_hna_error_t;

/**
 * cvmx_hna_hpu_csr
 *
 * "To read one of the HPU internal CSRs for debug (ie: HPU_STATUS, DBG_CURSTK,
 * DBG_GENERAL),
 * first a CSR WRITE of the HNA_HPU_DBG is done to specify the HPU CSR#, cluster#=CLID and
 * HPU#=HPUID,
 * which is followed by a CSR READ of the HPA_HPU_CSR which returns the contents of the specified
 * HPU CSR."
 */
union cvmx_hna_hpu_csr {
	uint64_t u64;
	struct cvmx_hna_hpu_csr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t csrdat                       : 64; /**< HPU CSR contents specified by the HNA_HPU_DBG CSR. */
#else
	uint64_t csrdat                       : 64;
#endif
	} s;
	struct cvmx_hna_hpu_csr_s             cn78xx;
};
typedef union cvmx_hna_hpu_csr cvmx_hna_hpu_csr_t;

/**
 * cvmx_hna_hpu_dbg
 *
 * "This register specifies the HPU CSR#, cluster#=CLID and HPU#=HPUID used during a
 * a CSR READ of the HNA_HPU_CSR register."
 */
union cvmx_hna_hpu_dbg {
	uint64_t u64;
	struct cvmx_hna_hpu_dbg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t csrnum                       : 2;  /**< "HPU CSR#
                                                         - 0: HPU_STATUS
                                                         - 1: DBG_CURSTK
                                                         - 2: DBG_GENERAL" */
	uint64_t clid                         : 2;  /**< HPC Cluster# Valid Range=[0..3] */
	uint64_t hpuid                        : 4;  /**< HPU Engine ID# Valid Range=[0..11] */
#else
	uint64_t hpuid                        : 4;
	uint64_t clid                         : 2;
	uint64_t csrnum                       : 2;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_hna_hpu_dbg_s             cn78xx;
};
typedef union cvmx_hna_hpu_dbg cvmx_hna_hpu_dbg_t;

/**
 * cvmx_hna_hpu_eir
 *
 * "Used by SW to force Parity or ECC errors on some internal HPU data structures.
 * A CSR WRITE of this register will force either a Parity or ECC error on the next access
 * at cluster#=CLID, HPU#=HPUID."
 */
union cvmx_hna_hpu_eir {
	uint64_t u64;
	struct cvmx_hna_hpu_eir_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t wrdone                       : 1;  /**< When the HNA_HPU_EIR register is written, this bit will
                                                         be cleared by HW. When the targeted HPU has received
                                                         the error injection command (ie: error injection armed),
                                                         the WRDONE bit will be SET.
                                                         SW will first write the HNA_HPU_EIR register, then do a
                                                         polling read of the WRDONE bit (until it becomes 1),
                                                         before issueing an HNA instruction to the targeted HPU
                                                         which will inject the intended error type for a single
                                                         occurrence (one-shot). */
	uint64_t pdperr                       : 1;  /**< Packet Data buffer Parity error
                                                         Forces parity error on next Packet data buffer read. */
	uint64_t svflipsyn                    : 2;  /**< Save stack flip syndrome control.
                                                         Forces 1-bit/2-bit errors on next save stack read. */
	uint64_t rsflipsyn                    : 2;  /**< Run stack flip syndrome control.
                                                         Forces 1-bit/2-bit errors on next run stack read. */
	uint64_t clid                         : 2;  /**< HPC Cluster# Valid Range=[0..3] */
	uint64_t hpuid                        : 4;  /**< HPU Engine ID# Valid Range=[0..11] */
#else
	uint64_t hpuid                        : 4;
	uint64_t clid                         : 2;
	uint64_t rsflipsyn                    : 2;
	uint64_t svflipsyn                    : 2;
	uint64_t pdperr                       : 1;
	uint64_t wrdone                       : 1;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_hna_hpu_eir_s             cn78xx;
};
typedef union cvmx_hna_hpu_eir cvmx_hna_hpu_eir_t;

/**
 * cvmx_hna_pfc0_cnt
 */
union cvmx_hna_pfc0_cnt {
	uint64_t u64;
	struct cvmx_hna_pfc0_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pfcnt                        : 64; /**< "HNA Performance Counter 0.
                                                         When HNA_PFC_GCTL[CNT0ENA]=1, the event selected
                                                         by HNA_PFC0_CTL[EVSEL] is counted.
                                                         See also HNA_PFC_GCTL[CNT0WCLR] and HNA_PFC_GCTL
                                                         [CNT0RCLR] for special clear count cases available
                                                         for SW data collection." */
#else
	uint64_t pfcnt                        : 64;
#endif
	} s;
	struct cvmx_hna_pfc0_cnt_s            cn78xx;
};
typedef union cvmx_hna_pfc0_cnt cvmx_hna_pfc0_cnt_t;

/**
 * cvmx_hna_pfc0_ctl
 */
union cvmx_hna_pfc0_ctl {
	uint64_t u64;
	struct cvmx_hna_pfc0_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t evsel                        : 6;  /**< Performance Counter#0 Event Selector (64 total) */
	uint64_t reserved_6_7                 : 2;
	uint64_t clhpu                        : 4;  /**< "Performance Counter 0 Cluster HPU Selector.
                                                         When HNA_PFC_GCTL[PMODE]=0 (per-cluster HPU), this field
                                                         is used to select/monitor the cluster's HPU# for all events
                                                         associated with Performance Counter#0." */
	uint64_t clnum                        : 2;  /**< "Performance Counter 0 Cluster Selector.
                                                         When HNA_PFC_GCTL[PMODE]=0 (per-cluster HPU), this field
                                                         is used to select/monitor the cluster# for all events
                                                         associated with Performance Counter#0." */
#else
	uint64_t clnum                        : 2;
	uint64_t clhpu                        : 4;
	uint64_t reserved_6_7                 : 2;
	uint64_t evsel                        : 6;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_hna_pfc0_ctl_s            cn78xx;
};
typedef union cvmx_hna_pfc0_ctl cvmx_hna_pfc0_ctl_t;

/**
 * cvmx_hna_pfc1_cnt
 */
union cvmx_hna_pfc1_cnt {
	uint64_t u64;
	struct cvmx_hna_pfc1_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pfcnt                        : 64; /**< "HNA Performance Counter 1.
                                                         When HNA_PFC_GCTL[CNT1ENA]=1, the event selected
                                                         by HNA_PFC1_CTL[EVSEL] is counted.
                                                         See also HNA_PFC_GCTL[CNT1WCLR] and HNA_PFC_GCTL
                                                         [CNT1RCLR] for special clear count cases available
                                                         for SW data collection." */
#else
	uint64_t pfcnt                        : 64;
#endif
	} s;
	struct cvmx_hna_pfc1_cnt_s            cn78xx;
};
typedef union cvmx_hna_pfc1_cnt cvmx_hna_pfc1_cnt_t;

/**
 * cvmx_hna_pfc1_ctl
 */
union cvmx_hna_pfc1_ctl {
	uint64_t u64;
	struct cvmx_hna_pfc1_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t evsel                        : 6;  /**< Performance Counter#1 Event Selector (64 total) */
	uint64_t reserved_6_7                 : 2;
	uint64_t clhpu                        : 4;  /**< "Performance Counter 1 Cluster HPU Selector.
                                                         When HNA_PFC_GCTL[PMODE]=0 (per-cluster HPU), this field
                                                         is used to select/monitor the cluster's HPU# for all events
                                                         associated with Performance Counter#1." */
	uint64_t clnum                        : 2;  /**< "Performance Counter 1 Cluster Selector.
                                                         When HNA_PFC_GCTL[PMODE]=0 (per-cluster HPU), this field
                                                         is used to select/monitor the cluster# for all events
                                                         associated with Performance Counter#1." */
#else
	uint64_t clnum                        : 2;
	uint64_t clhpu                        : 4;
	uint64_t reserved_6_7                 : 2;
	uint64_t evsel                        : 6;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_hna_pfc1_ctl_s            cn78xx;
};
typedef union cvmx_hna_pfc1_ctl cvmx_hna_pfc1_ctl_t;

/**
 * cvmx_hna_pfc2_cnt
 */
union cvmx_hna_pfc2_cnt {
	uint64_t u64;
	struct cvmx_hna_pfc2_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pfcnt                        : 64; /**< "HNA Performance Counter 2.
                                                         When HNA_PFC_GCTL[CNT2ENA]=1, the event selected
                                                         by HNA_PFC2_CTL[EVSEL] is counted.
                                                         See also HNA_PFC_GCTL[CNT2WCLR] and HNA_PFC_GCTL
                                                         [CNT2RCLR] for special clear count cases available
                                                         for SW data collection." */
#else
	uint64_t pfcnt                        : 64;
#endif
	} s;
	struct cvmx_hna_pfc2_cnt_s            cn78xx;
};
typedef union cvmx_hna_pfc2_cnt cvmx_hna_pfc2_cnt_t;

/**
 * cvmx_hna_pfc2_ctl
 */
union cvmx_hna_pfc2_ctl {
	uint64_t u64;
	struct cvmx_hna_pfc2_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t evsel                        : 6;  /**< Performance Counter#2 Event Selector (64 total) */
	uint64_t reserved_6_7                 : 2;
	uint64_t clhpu                        : 4;  /**< "Performance Counter#2 Cluster HPU Selector.
                                                         When HNA_PFC_GCTL[PMODE]=0 (per-cluster HPU), this field
                                                         is used to select/monitor the cluster's HPU# for all events
                                                         associated with Performance Counter#2." */
	uint64_t clnum                        : 2;  /**< "Performance Counter#2 Cluster Selector.
                                                         When HNA_PFC_GCTL[PMODE]=0 (per-cluster HPU), this field
                                                         is used to select/monitor the cluster# for all events
                                                         associated with Performance Counter#2." */
#else
	uint64_t clnum                        : 2;
	uint64_t clhpu                        : 4;
	uint64_t reserved_6_7                 : 2;
	uint64_t evsel                        : 6;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_hna_pfc2_ctl_s            cn78xx;
};
typedef union cvmx_hna_pfc2_ctl cvmx_hna_pfc2_ctl_t;

/**
 * cvmx_hna_pfc3_cnt
 */
union cvmx_hna_pfc3_cnt {
	uint64_t u64;
	struct cvmx_hna_pfc3_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pfcnt                        : 64; /**< "HNA Performance Counter 3.
                                                         When HNA_PFC_GCTL[CNT3ENA]=1, the event selected
                                                         by HNA_PFC3_CTL[EVSEL] is counted.
                                                         See also HNA_PFC_GCTL[CNT3WCLR] and HNA_PFC_GCTL
                                                         [CNT3RCLR] for special clear count cases available
                                                         for SW data collection." */
#else
	uint64_t pfcnt                        : 64;
#endif
	} s;
	struct cvmx_hna_pfc3_cnt_s            cn78xx;
};
typedef union cvmx_hna_pfc3_cnt cvmx_hna_pfc3_cnt_t;

/**
 * cvmx_hna_pfc3_ctl
 */
union cvmx_hna_pfc3_ctl {
	uint64_t u64;
	struct cvmx_hna_pfc3_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t evsel                        : 6;  /**< Performance Counter 3 Event Selector (64 total) */
	uint64_t reserved_6_7                 : 2;
	uint64_t clhpu                        : 4;  /**< "Performance Counter 3 Cluster HPU Selector.
                                                         When HNA_PFC_GCTL[PMODE]=0 (per-cluster HPU), this field
                                                         is used to select/monitor the cluster's HPU# for all events
                                                         associated with Performance Counter#3." */
	uint64_t clnum                        : 2;  /**< "Performance Counter 3 Cluster Selector.
                                                         When HNA_PFC_GCTL[PMODE]=0 (per-cluster HPU), this field
                                                         is used to select/monitor the cluster# for all events
                                                         associated with Performance Counter 3." */
#else
	uint64_t clnum                        : 2;
	uint64_t clhpu                        : 4;
	uint64_t reserved_6_7                 : 2;
	uint64_t evsel                        : 6;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_hna_pfc3_ctl_s            cn78xx;
};
typedef union cvmx_hna_pfc3_ctl cvmx_hna_pfc3_ctl_t;

/**
 * cvmx_hna_pfc_gctl
 *
 * Global control across all performance counters.
 *
 */
union cvmx_hna_pfc_gctl {
	uint64_t u64;
	struct cvmx_hna_pfc_gctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t cnt3rclr                     : 1;  /**< "Performance Counter 3 Read Clear.
                                                         If this bit is set, CSR reads to the HNA_PFC3_CNT
                                                         will clear the count value. This allows SW to maintain
                                                         'cumulative' counters to avoid HW wraparound." */
	uint64_t cnt2rclr                     : 1;  /**< "Performance Counter 2 Read Clear.
                                                         If this bit is set, CSR reads to the HNA_PFC2_CNT
                                                         will clear the count value. This allows SW to maintain
                                                         'cumulative' counters to avoid HW wraparound." */
	uint64_t cnt1rclr                     : 1;  /**< "Performance Counter 1 Read Clear.
                                                         If this bit is set, CSR reads to the HNA_PFC1_CNT
                                                         will clear the count value. This allows SW to maintain
                                                         'cumulative' counters to avoid HW wraparound." */
	uint64_t cnt0rclr                     : 1;  /**< "Performance Counter 0 Read Clear.
                                                         If this bit is set, CSR reads to the HNA_PFC0_CNT
                                                         will clear the count value. This allows SW to maintain
                                                         'cumulative' counters to avoid HW wraparound." */
	uint64_t cnt3wclr                     : 1;  /**< "Performance Counter 3 Write Clear.
                                                         If this bit is set, CSR writes to the HNA_PFC3_CNT
                                                         will clear the count value.
                                                         If this bit is clear, CSR writes to the HNA_PFC3_CNT
                                                         will continue the count from the written value." */
	uint64_t cnt2wclr                     : 1;  /**< "Performance Counter 2 Write Clear.
                                                         If this bit is set, CSR writes to the HNA_PFC2_CNT
                                                         will clear the count value.
                                                         If this bit is clear, CSR writes to the HNA_PFC2_CNT
                                                         will continue the count from the written value." */
	uint64_t cnt1wclr                     : 1;  /**< "Performance Counter 1 Write Clear.
                                                         If this bit is set, CSR writes to the HNA_PFC1_CNT
                                                         will clear the count value.
                                                         If this bit is clear, CSR writes to the HNA_PFC1_CNT
                                                         will continue the count from the written value." */
	uint64_t cnt0wclr                     : 1;  /**< "Performance Counter 0 Write Clear.
                                                         If this bit is set, CSR writes to the HNA_PFC0_CNT
                                                         will clear the count value.
                                                         If this bit is clear, CSR writes to the HNA_PFC0_CNT
                                                         will continue the count from the written value." */
	uint64_t cnt3ena                      : 1;  /**< "Performance Counter 3 Enable.
                                                         When this bit is set, the performance counter \#3
                                                         is enabled." */
	uint64_t cnt2ena                      : 1;  /**< "Performance Counter 2 Enable.
                                                         When this bit is set, the performance counter \#2
                                                         is enabled." */
	uint64_t cnt1ena                      : 1;  /**< "Performance Counter 1 Enable.
                                                         When this bit is set, the performance counter \#1
                                                         is enabled." */
	uint64_t cnt0ena                      : 1;  /**< "Performance Counter 0 Enable.
                                                         When this bit is set, the performance counter \#0
                                                         is enabled." */
#else
	uint64_t cnt0ena                      : 1;
	uint64_t cnt1ena                      : 1;
	uint64_t cnt2ena                      : 1;
	uint64_t cnt3ena                      : 1;
	uint64_t cnt0wclr                     : 1;
	uint64_t cnt1wclr                     : 1;
	uint64_t cnt2wclr                     : 1;
	uint64_t cnt3wclr                     : 1;
	uint64_t cnt0rclr                     : 1;
	uint64_t cnt1rclr                     : 1;
	uint64_t cnt2rclr                     : 1;
	uint64_t cnt3rclr                     : 1;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_hna_pfc_gctl_s            cn78xx;
};
typedef union cvmx_hna_pfc_gctl cvmx_hna_pfc_gctl_t;

/**
 * cvmx_hna_sbd_dbg0
 *
 * When the HNA_CONTROL[SBDLCK] bit is written '1', the contents of this register
 * are locked down. Otherwise, the contents of this register are the 'active' contents of the
 * HNA Scoreboard at the time of the CSR read.
 * INTERNAL: VERIFICATION NOTE: Read data is unsafe. X's(undefined data) can propagate (in the
 * behavioral
 * model) on the reads unless the HPU Engine specified by HNA_CONTROL[SBDNUM] has previously been
 * assigned an instruction.
 */
union cvmx_hna_sbd_dbg0 {
	uint64_t u64;
	struct cvmx_hna_sbd_dbg0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sbd                          : 64; /**< "HNA ScoreBoard 0 Data.
                                                         [63:38]   (26) rptr[28:3]: Result Base Pointer (QW-aligned)
                                                         [37:22]   (16) Cumulative Result Write Counter (for HDR write)
                                                         [21]       (1) Waiting for GRdRsp EOT
                                                         [20]       (1) Waiting for GRdReq Issue (to NRQ)
                                                         [19]       (1) GLPTR/GLCNT Valid
                                                         [18]       (1) Completion Mark Detected
                                                         [17:15]    (3) Completion Code [0=PDGONE/1=PERR/2=RFULL/3=TERM]
                                                         [14]       (1) Completion Detected
                                                         [13]       (1) Waiting for HDR RWrCmtRsp
                                                         [12]       (1) Waiting for LAST RESULT RWrCmtRsp
                                                         [11]       (1) Waiting for HDR RWrReq
                                                         [10]        (1) Waiting for RWrReq
                                                         [9]        (1) Waiting for WQWrReq issue
                                                         [8]        (1) Waiting for PRdRsp EOT
                                                         [7]        (1) Waiting for PRdReq Issue (to NRQ)
                                                         [6]        (1) Packet Data Valid
                                                         [5]        (1) WQVLD
                                                         [4]        (1) WQ Done Point (either WQWrReq issued (for WQPTR<>0) OR HDR RWrCmtRsp)
                                                         [3]        (1) Resultant write STF/P Mode
                                                         [2]        (1) Packet Data LDT mode
                                                         [1]        (1) Gather Mode
                                                         [0]        (1) Valid" */
#else
	uint64_t sbd                          : 64;
#endif
	} s;
	struct cvmx_hna_sbd_dbg0_s            cn78xx;
};
typedef union cvmx_hna_sbd_dbg0 cvmx_hna_sbd_dbg0_t;

/**
 * cvmx_hna_sbd_dbg1
 *
 * When the HNA_CONTROL[SBDLCK] bit is written '1', the contents of this register
 * are locked down. Otherwise, the contents of this register are the 'active' contents of the
 * HNA Scoreboard at the time of the CSR read.
 * INTERNAL: VERIFICATION NOTE: Read data is unsafe. X's(undefined data) can propagate (in the
 * behavioral
 * model) on the reads unless the HPU Engine specified by HNA_CONTROL[SBDNUM] has previously been
 * assigned an instruction.
 */
union cvmx_hna_sbd_dbg1 {
	uint64_t u64;
	struct cvmx_hna_sbd_dbg1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sbd                          : 64; /**< "HNA ScoreBoard 1 Data.
                                                         [63:56]   (8) UNUSED
                                                         [55:16]  (40) Packet Data Pointer
                                                         [15:0]   (16) Packet Data Counter" */
#else
	uint64_t sbd                          : 64;
#endif
	} s;
	struct cvmx_hna_sbd_dbg1_s            cn78xx;
};
typedef union cvmx_hna_sbd_dbg1 cvmx_hna_sbd_dbg1_t;

/**
 * cvmx_hna_sbd_dbg2
 *
 * When the HNA_CONTROL[SBDLCK] bit is written '1', the contents of this register
 * are locked down. Otherwise, the contents of this register are the 'active' contents of the
 * HNA Scoreboard at the time of the CSR read.
 * INTERNAL: VERIFICATION NOTE: Read data is unsafe. X's(undefined data) can propagate (in the
 * behavioral
 * model) on the reads unless the HPU Engine specified by HNA_CONTROL[SBDNUM] has previously been
 * assigned an instruction.
 */
union cvmx_hna_sbd_dbg2 {
	uint64_t u64;
	struct cvmx_hna_sbd_dbg2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sbd                          : 64; /**< "HNA ScoreBoard 2 Data.
                                                         [63:45] (19) UNUSED
                                                         [44:42]  (3) Instruction Type
                                                         [41:5]  (37) rwptr[39:3]: Result Write Pointer
                                                         [4:0]    (5) prwcnt[4:0]: Pending Result Write Counter" */
#else
	uint64_t sbd                          : 64;
#endif
	} s;
	struct cvmx_hna_sbd_dbg2_s            cn78xx;
};
typedef union cvmx_hna_sbd_dbg2 cvmx_hna_sbd_dbg2_t;

/**
 * cvmx_hna_sbd_dbg3
 *
 * When the HNA_CONTROL[SBDLCK] bit is written '1', the contents of this register
 * are locked down. Otherwise, the contents of this register are the 'active' contents of the
 * HNA Scoreboard at the time of the CSR read.
 * INTERNAL: VERIFICATION NOTE: Read data is unsafe. X's(undefined data) can propagate (in the
 * behavioral
 * model) on the reads unless the HPU Engine specified by HNA_CONTROL[SBDNUM] has previously been
 * assigned an instruction.
 */
union cvmx_hna_sbd_dbg3 {
	uint64_t u64;
	struct cvmx_hna_sbd_dbg3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sbd                          : 64; /**< "HNA ScoreBoard 3 Data.
                                                         [63:52] (11) rptr[39:29]: Result Base Pointer (QW-aligned)
                                                         [52:16] (37) glptr[39:3]: Gather List Pointer
                                                         [15:0]  (16) glcnt Gather List Counter" */
#else
	uint64_t sbd                          : 64;
#endif
	} s;
	struct cvmx_hna_sbd_dbg3_s            cn78xx;
};
typedef union cvmx_hna_sbd_dbg3 cvmx_hna_sbd_dbg3_t;

#endif
