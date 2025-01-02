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
 * cvmx-iobp-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon iobp.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_IOBP_DEFS_H__
#define __CVMX_IOBP_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOBP_BIST_STATUS CVMX_IOBP_BIST_STATUS_FUNC()
static inline uint64_t CVMX_IOBP_BIST_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_IOBP_BIST_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0010018ull);
}
#else
#define CVMX_IOBP_BIST_STATUS (CVMX_ADD_IO_SEG(0x00011800F0010018ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOBP_CREDITS CVMX_IOBP_CREDITS_FUNC()
static inline uint64_t CVMX_IOBP_CREDITS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_IOBP_CREDITS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0010028ull);
}
#else
#define CVMX_IOBP_CREDITS (CVMX_ADD_IO_SEG(0x00011800F0010028ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOBP_ECC CVMX_IOBP_ECC_FUNC()
static inline uint64_t CVMX_IOBP_ECC_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_IOBP_ECC not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0010010ull);
}
#else
#define CVMX_IOBP_ECC (CVMX_ADD_IO_SEG(0x00011800F0010010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOBP_INT_SUM CVMX_IOBP_INT_SUM_FUNC()
static inline uint64_t CVMX_IOBP_INT_SUM_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_IOBP_INT_SUM not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0010020ull);
}
#else
#define CVMX_IOBP_INT_SUM (CVMX_ADD_IO_SEG(0x00011800F0010020ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOBP_PP_BIST_STATUS CVMX_IOBP_PP_BIST_STATUS_FUNC()
static inline uint64_t CVMX_IOBP_PP_BIST_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_IOBP_PP_BIST_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0010700ull);
}
#else
#define CVMX_IOBP_PP_BIST_STATUS (CVMX_ADD_IO_SEG(0x00011800F0010700ull))
#endif

/**
 * cvmx_iobp_bist_status
 *
 * This register contains the result of the BIST run on the IOB memories.
 *
 */
union cvmx_iobp_bist_status {
	uint64_t u64;
	struct cvmx_iobp_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t rsdm1                        : 1;  /**< rsd_mem1_bstatus */
	uint64_t rsdm0                        : 1;  /**< rsd_mem0_bstatus */
	uint64_t xmcf                         : 1;  /**< xmcfif_bstatus */
	uint64_t icx3                         : 1;  /**< icc0_xmc_fifo_ecc_bstatus */
	uint64_t icx2                         : 1;  /**< icc0_xmc_fifo_ecc_bstatus */
	uint64_t icx1                         : 1;  /**< icc0_xmc_fifo_ecc_bstatus */
	uint64_t icx0                         : 1;  /**< icc0_xmc_fifo_ecc_bstatus */
	uint64_t immx3                        : 1;  /**< icm_mem_mask_xmd3_bstatus */
	uint64_t imdx3                        : 1;  /**< icm_mem_data_xmd3_bstatus */
	uint64_t immx2                        : 1;  /**< icm_mem_mask_xmd2_bstatus */
	uint64_t imdx2                        : 1;  /**< icm_mem_data_xmd2_bstatus */
#else
	uint64_t imdx2                        : 1;
	uint64_t immx2                        : 1;
	uint64_t imdx3                        : 1;
	uint64_t immx3                        : 1;
	uint64_t icx0                         : 1;
	uint64_t icx1                         : 1;
	uint64_t icx2                         : 1;
	uint64_t icx3                         : 1;
	uint64_t xmcf                         : 1;
	uint64_t rsdm0                        : 1;
	uint64_t rsdm1                        : 1;
	uint64_t reserved_11_63               : 53;
#endif
	} s;
	struct cvmx_iobp_bist_status_s        cn73xx;
	struct cvmx_iobp_bist_status_s        cn78xx;
	struct cvmx_iobp_bist_status_s        cn78xxp1;
	struct cvmx_iobp_bist_status_s        cnf75xx;
};
typedef union cvmx_iobp_bist_status cvmx_iobp_bist_status_t;

/**
 * cvmx_iobp_credits
 *
 * This register controls number of loads and stores PKO and PKI can have to the L2.
 *
 */
union cvmx_iobp_credits {
	uint64_t u64;
	struct cvmx_iobp_credits_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_31_63               : 33;
	uint64_t pki1_crd                     : 7;  /**< Number of stores PKI1 can have in flight to the L2. */
	uint64_t reserved_23_23               : 1;
	uint64_t pki0_crd                     : 7;  /**< Number of stores PKI0 can have in flight to the L2. */
	uint64_t reserved_14_15               : 2;
	uint64_t pko1_crd                     : 6;  /**< Number of reads PKO1 can have in flight to the L2. */
	uint64_t reserved_6_7                 : 2;
	uint64_t pko0_crd                     : 6;  /**< Number of reads PKO0 can have in flight to the L2. */
#else
	uint64_t pko0_crd                     : 6;
	uint64_t reserved_6_7                 : 2;
	uint64_t pko1_crd                     : 6;
	uint64_t reserved_14_15               : 2;
	uint64_t pki0_crd                     : 7;
	uint64_t reserved_23_23               : 1;
	uint64_t pki1_crd                     : 7;
	uint64_t reserved_31_63               : 33;
#endif
	} s;
	struct cvmx_iobp_credits_s            cn73xx;
	struct cvmx_iobp_credits_s            cn78xx;
	struct cvmx_iobp_credits_s            cn78xxp1;
	struct cvmx_iobp_credits_s            cnf75xx;
};
typedef union cvmx_iobp_credits cvmx_iobp_credits_t;

/**
 * cvmx_iobp_ecc
 *
 * This register contains various control bits for IOBP ECC functionality.
 *
 */
union cvmx_iobp_ecc {
	uint64_t u64;
	struct cvmx_iobp_ecc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_22_63               : 42;
	uint64_t rsd1_ecc                     : 1;  /**< When set, PKO1 response data has ECC generated and checked. */
	uint64_t rsd1_fs                      : 2;  /**< Used to flip the syndrome PKO1 response data. */
	uint64_t rsd0_ecc                     : 1;  /**< When set, PKO0 response data has ECC generated and checked. */
	uint64_t rsd0_fs                      : 2;  /**< Used to flip the syndrome PKO0 response data. */
	uint64_t xmc3_ecc                     : 1;  /**< When set, PKI1 commands to L2C have ECC generated and checked. */
	uint64_t xmc3_fs                      : 2;  /**< Used to flip the syndrome for commands from PKI1 to the L2C. */
	uint64_t xmc2_ecc                     : 1;  /**< When set, PKI0 commands to L2C have ECC generated and checked. */
	uint64_t xmc2_fs                      : 2;  /**< Used to flip the syndrome for commands from PKI0 to the L2C. */
	uint64_t xmc1_ecc                     : 1;  /**< When set, PKO1 commands to L2C have ECC generated and checked. */
	uint64_t xmc1_fs                      : 2;  /**< Used to flip the syndrome for commands from PKO1 to the L2C. */
	uint64_t xmc0_ecc                     : 1;  /**< When set, PKO commands to L2C have ECC generated and checked. */
	uint64_t xmc0_fs                      : 2;  /**< Used to flip the syndrome for commands from PKO to the L2C. */
	uint64_t xmd3_ecc                     : 1;  /**< When set, PKI1 data to L2C has ECC generated and checked. */
	uint64_t xmd2_ecc                     : 1;  /**< When set, PKI0 data to L2C has ECC generated and checked. */
	uint64_t reserved_0_1                 : 2;
#else
	uint64_t reserved_0_1                 : 2;
	uint64_t xmd2_ecc                     : 1;
	uint64_t xmd3_ecc                     : 1;
	uint64_t xmc0_fs                      : 2;
	uint64_t xmc0_ecc                     : 1;
	uint64_t xmc1_fs                      : 2;
	uint64_t xmc1_ecc                     : 1;
	uint64_t xmc2_fs                      : 2;
	uint64_t xmc2_ecc                     : 1;
	uint64_t xmc3_fs                      : 2;
	uint64_t xmc3_ecc                     : 1;
	uint64_t rsd0_fs                      : 2;
	uint64_t rsd0_ecc                     : 1;
	uint64_t rsd1_fs                      : 2;
	uint64_t rsd1_ecc                     : 1;
	uint64_t reserved_22_63               : 42;
#endif
	} s;
	struct cvmx_iobp_ecc_s                cn73xx;
	struct cvmx_iobp_ecc_s                cn78xx;
	struct cvmx_iobp_ecc_s                cn78xxp1;
	struct cvmx_iobp_ecc_s                cnf75xx;
};
typedef union cvmx_iobp_ecc cvmx_iobp_ecc_t;

/**
 * cvmx_iobp_int_sum
 *
 * This is the IOBP interrupt summary register.
 *
 */
union cvmx_iobp_int_sum {
	uint64_t u64;
	struct cvmx_iobp_int_sum_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_46_63               : 18;
	uint64_t icc3_dbe                     : 1;  /**< Double-bit error for ICC MEM3.
                                                         Throws IOBP_INTSN_E::IOBP_INT_SUM_ICC3_DBE. */
	uint64_t icc3_sbe                     : 1;  /**< Single-bit error for ICC MEM3.
                                                         Throws IOBP_INTSN_E::IOBP_INT_SUM_ICC3_SBE. */
	uint64_t icc2_dbe                     : 1;  /**< Double-bit error for ICC MEM2.
                                                         Throws IOBP_INTSN_E::IOBP_INT_SUM_ICC2_DBE. */
	uint64_t icc2_sbe                     : 1;  /**< Single-bit error for ICC MEM2.
                                                         Throws IOBP_INTSN_E::IOBP_INT_SUM_ICC2_SBE. */
	uint64_t icc1_dbe                     : 1;  /**< Double-bit error for ICC MEM1.
                                                         Throws IOBP_INTSN_E::IOBP_INT_SUM_ICC1_DBE. */
	uint64_t icc1_sbe                     : 1;  /**< Single-bit error for ICC MEM1.
                                                         Throws IOBP_INTSN_E::IOBP_INT_SUM_ICC1_SBE. */
	uint64_t icc0_dbe                     : 1;  /**< Double-bit error for ICC MEM0.
                                                         Throws IOBP_INTSN_E::IOBP_INT_SUM_ICC0_DBE. */
	uint64_t icc0_sbe                     : 1;  /**< Single-bit error for ICC MEM0.
                                                         Throws IOBP_INTSN_E::IOBN_INT_SUM_ICC0_SBE. */
	uint64_t reserved_34_37               : 4;
	uint64_t xmdb3_dbe                    : 1;  /**< Double-bit error for XMDB MEM3.
                                                         Throws IOBP_INTSN_E::IOBP_INT_SUM_XMDB3_DBE. */
	uint64_t xmdb3_sbe                    : 1;  /**< Single-bit error for XMDB MEM3.
                                                         Throws IOBP_INTSN_E::IOBP_INT_SUM_XMDB3_SBE. */
	uint64_t xmdb2_dbe                    : 1;  /**< Double-bit error for XMDB MEM2.
                                                         Throws IOBP_INTSN_E::IOBP_INT_SUM_XMDB2_DBE. */
	uint64_t xmdb2_sbe                    : 1;  /**< Single-bit error for XMDB MEM2.
                                                         Throws IOBP_INTSN_E::IOBP_INT_SUM_XMDB2_SBE. */
	uint64_t reserved_26_29               : 4;
	uint64_t xmda3_dbe                    : 1;  /**< Double-bit error for XMDA MEM3.
                                                         Throws IOBP_INTSN_E::IOBP_INT_SUM_XMDA3_DBE. */
	uint64_t xmda3_sbe                    : 1;  /**< Single-bit error for XMDA MEM3.
                                                         Throws IOBP_INTSN_E::IOBP_INT_SUM_XMDA3_SBE. */
	uint64_t xmda2_dbe                    : 1;  /**< Double-bit error for XMDA MEM2.
                                                         Throws IOBP_INTSN_E::IOBP_INT_SUM_XMDA2_DBE. */
	uint64_t xmda2_sbe                    : 1;  /**< Single-bit error for XMDA MEM2.
                                                         Throws IOBP_INTSN_E::IOBP_INT_SUM_XMDA2_SBE. */
	uint64_t reserved_4_21                : 18;
	uint64_t rsd1_dbe                     : 1;  /**< Double-bit error for RSD MEM1.
                                                         Throws IOBP_INTSN_E::IOBP_INT_SUM_RSD1_DBE. */
	uint64_t rsd1_sbe                     : 1;  /**< Single-bit error for RSD MEM1.
                                                         Throws IOBP_INTSN_E::IOBP_INT_SUM_RSD1_SBE. */
	uint64_t rsd0_dbe                     : 1;  /**< Double-bit error for RSD MEM0.
                                                         Throws IOBP_INTSN_E::IOBP_INT_SUM_RSD0_DBE. */
	uint64_t rsd0_sbe                     : 1;  /**< Single-bit error for RSD MEM0.
                                                         Throws IOBP_INTSN_E::IOBP_INT_SUM_RSD0_SBE. */
#else
	uint64_t rsd0_sbe                     : 1;
	uint64_t rsd0_dbe                     : 1;
	uint64_t rsd1_sbe                     : 1;
	uint64_t rsd1_dbe                     : 1;
	uint64_t reserved_4_21                : 18;
	uint64_t xmda2_sbe                    : 1;
	uint64_t xmda2_dbe                    : 1;
	uint64_t xmda3_sbe                    : 1;
	uint64_t xmda3_dbe                    : 1;
	uint64_t reserved_26_29               : 4;
	uint64_t xmdb2_sbe                    : 1;
	uint64_t xmdb2_dbe                    : 1;
	uint64_t xmdb3_sbe                    : 1;
	uint64_t xmdb3_dbe                    : 1;
	uint64_t reserved_34_37               : 4;
	uint64_t icc0_sbe                     : 1;
	uint64_t icc0_dbe                     : 1;
	uint64_t icc1_sbe                     : 1;
	uint64_t icc1_dbe                     : 1;
	uint64_t icc2_sbe                     : 1;
	uint64_t icc2_dbe                     : 1;
	uint64_t icc3_sbe                     : 1;
	uint64_t icc3_dbe                     : 1;
	uint64_t reserved_46_63               : 18;
#endif
	} s;
	struct cvmx_iobp_int_sum_s            cn73xx;
	struct cvmx_iobp_int_sum_s            cn78xx;
	struct cvmx_iobp_int_sum_s            cn78xxp1;
	struct cvmx_iobp_int_sum_s            cnf75xx;
};
typedef union cvmx_iobp_int_sum cvmx_iobp_int_sum_t;

/**
 * cvmx_iobp_pp_bist_status
 *
 * This register contains the result of the BIST run on the cores.
 *
 */
union cvmx_iobp_pp_bist_status {
	uint64_t u64;
	struct cvmx_iobp_pp_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t pp_bstat                     : 48; /**< BIST status of cores. Bit vector position is the physical number of the core
                                                         (i.e. core 1 == PP_BSTAT<1>). Only odd number bits are valid; all even number
                                                         bits are read as 0. For even number cores, see IOBN_PP_BIST_STATUS.
                                                         Software must bit-wise logical AND IOBN_PP_BIST_STATUS with CIU_FUSE before using
                                                         it. */
#else
	uint64_t pp_bstat                     : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_iobp_pp_bist_status_s     cn73xx;
	struct cvmx_iobp_pp_bist_status_s     cn78xx;
	struct cvmx_iobp_pp_bist_status_s     cn78xxp1;
	struct cvmx_iobp_pp_bist_status_s     cnf75xx;
};
typedef union cvmx_iobp_pp_bist_status cvmx_iobp_pp_bist_status_t;

#endif
