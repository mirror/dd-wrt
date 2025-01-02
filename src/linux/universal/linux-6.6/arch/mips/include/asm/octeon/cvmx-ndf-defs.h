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
 * cvmx-ndf-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon ndf.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_NDF_DEFS_H__
#define __CVMX_NDF_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_NDF_BT_PG_INFO CVMX_NDF_BT_PG_INFO_FUNC()
static inline uint64_t CVMX_NDF_BT_PG_INFO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_NDF_BT_PG_INFO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070001000018ull);
}
#else
#define CVMX_NDF_BT_PG_INFO (CVMX_ADD_IO_SEG(0x0001070001000018ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_NDF_CMD CVMX_NDF_CMD_FUNC()
static inline uint64_t CVMX_NDF_CMD_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_NDF_CMD not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070001000000ull);
}
#else
#define CVMX_NDF_CMD (CVMX_ADD_IO_SEG(0x0001070001000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_NDF_DMA_ADR CVMX_NDF_DMA_ADR_FUNC()
static inline uint64_t CVMX_NDF_DMA_ADR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_NDF_DMA_ADR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070001000058ull);
}
#else
#define CVMX_NDF_DMA_ADR (CVMX_ADD_IO_SEG(0x0001070001000058ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_NDF_DMA_CFG CVMX_NDF_DMA_CFG_FUNC()
static inline uint64_t CVMX_NDF_DMA_CFG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_NDF_DMA_CFG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070001000050ull);
}
#else
#define CVMX_NDF_DMA_CFG (CVMX_ADD_IO_SEG(0x0001070001000050ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_NDF_DRBELL CVMX_NDF_DRBELL_FUNC()
static inline uint64_t CVMX_NDF_DRBELL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_NDF_DRBELL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070001000030ull);
}
#else
#define CVMX_NDF_DRBELL (CVMX_ADD_IO_SEG(0x0001070001000030ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_NDF_ECC_CNT CVMX_NDF_ECC_CNT_FUNC()
static inline uint64_t CVMX_NDF_ECC_CNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_NDF_ECC_CNT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070001000010ull);
}
#else
#define CVMX_NDF_ECC_CNT (CVMX_ADD_IO_SEG(0x0001070001000010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_NDF_INT CVMX_NDF_INT_FUNC()
static inline uint64_t CVMX_NDF_INT_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070001000020ull);
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070001000040ull);
			break;
	}
	cvmx_warn("CVMX_NDF_INT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070001000040ull);
}
#else
#define CVMX_NDF_INT CVMX_NDF_INT_FUNC()
static inline uint64_t CVMX_NDF_INT_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070001000020ull);
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001070001000040ull);
	}
	return CVMX_ADD_IO_SEG(0x0001070001000040ull);
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_NDF_INT_EN CVMX_NDF_INT_EN_FUNC()
static inline uint64_t CVMX_NDF_INT_EN_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_NDF_INT_EN not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070001000028ull);
}
#else
#define CVMX_NDF_INT_EN (CVMX_ADD_IO_SEG(0x0001070001000028ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_NDF_INT_W1S CVMX_NDF_INT_W1S_FUNC()
static inline uint64_t CVMX_NDF_INT_W1S_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_NDF_INT_W1S not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070001000048ull);
}
#else
#define CVMX_NDF_INT_W1S (CVMX_ADD_IO_SEG(0x0001070001000048ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_NDF_MISC CVMX_NDF_MISC_FUNC()
static inline uint64_t CVMX_NDF_MISC_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_NDF_MISC not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070001000008ull);
}
#else
#define CVMX_NDF_MISC (CVMX_ADD_IO_SEG(0x0001070001000008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_NDF_ST_REG CVMX_NDF_ST_REG_FUNC()
static inline uint64_t CVMX_NDF_ST_REG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_NDF_ST_REG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001070001000038ull);
}
#else
#define CVMX_NDF_ST_REG (CVMX_ADD_IO_SEG(0x0001070001000038ull))
#endif

/**
 * cvmx_ndf_bt_pg_info
 *
 * This register provides the page size and the number of column-plus-row address-cycle
 * information. Software writes to this register during a boot operation from a NAND flash
 * device.
 *
 * Additionally, software also writes the multiplier value for timing parameters that is used
 * during the boot process, in the SET_TM_PARAM command. The multiplier value is used only by the
 * boot-load state machine during boot operations. Boot DMA operations do not use this value.
 *
 * Sixty-four-bit operations must be used to access this register.
 */
union cvmx_ndf_bt_pg_info {
	uint64_t u64;
	struct cvmx_ndf_bt_pg_info_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t t_mult                       : 4;  /**< The boot-time TIM_MULT field of the SET_TM_PARAM command. */
	uint64_t adr_cyc                      : 4;  /**< Number of column-address cycles. Legal values are 0x3 - 0x8. Values written to this field
                                                         smaller than 0x3 are converted to 0x3; values larger than 0x8 are converted to 0x8. */
	uint64_t size                         : 3;  /**< Number of bytes per page in the NAND flash device = 2^SIZE+1 * 256.
                                                         0x0 = 512 bytes/page.
                                                         0x1 = 1 KB/page.
                                                         0x2 = 2 KB/page.
                                                         0x3 = 4 KB/page.
                                                         0x4 = 8 KB/page.
                                                         0x5 = 16 KB/page.
                                                         0x6 = 32 KB/page.
                                                         0x7 = 64 KB/page. */
#else
	uint64_t size                         : 3;
	uint64_t adr_cyc                      : 4;
	uint64_t t_mult                       : 4;
	uint64_t reserved_11_63               : 53;
#endif
	} s;
	struct cvmx_ndf_bt_pg_info_s          cn52xx;
	struct cvmx_ndf_bt_pg_info_s          cn63xx;
	struct cvmx_ndf_bt_pg_info_s          cn63xxp1;
	struct cvmx_ndf_bt_pg_info_s          cn66xx;
	struct cvmx_ndf_bt_pg_info_s          cn68xx;
	struct cvmx_ndf_bt_pg_info_s          cn68xxp1;
	struct cvmx_ndf_bt_pg_info_s          cn70xx;
	struct cvmx_ndf_bt_pg_info_s          cn70xxp1;
	struct cvmx_ndf_bt_pg_info_s          cn73xx;
	struct cvmx_ndf_bt_pg_info_s          cnf75xx;
};
typedef union cvmx_ndf_bt_pg_info cvmx_ndf_bt_pg_info_t;

/**
 * cvmx_ndf_cmd
 *
 * When software reads this register, NDF_MISC[RD_VAL] is cleared to 0. Software must always
 * write all eight bytes whenever it writes this register. If there are fewer than eight bytes
 * left in the command sequence that software wants the NAND flash controller to execute, it must
 * insert Idle (WAIT) commands to make up eight bytes. Software must also ensure that there is
 * enough space in the NDF_CMD queue to accept these eight bytes by first reading
 * NDF_MISC[FR_BYT].
 *
 * Sixty-four-bit operations must be used to access this register.
 */
union cvmx_ndf_cmd {
	uint64_t u64;
	struct cvmx_ndf_cmd_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t nf_cmd                       : 64; /**< Eight NAND flash memory command bytes. */
#else
	uint64_t nf_cmd                       : 64;
#endif
	} s;
	struct cvmx_ndf_cmd_s                 cn52xx;
	struct cvmx_ndf_cmd_s                 cn63xx;
	struct cvmx_ndf_cmd_s                 cn63xxp1;
	struct cvmx_ndf_cmd_s                 cn66xx;
	struct cvmx_ndf_cmd_s                 cn68xx;
	struct cvmx_ndf_cmd_s                 cn68xxp1;
	struct cvmx_ndf_cmd_s                 cn70xx;
	struct cvmx_ndf_cmd_s                 cn70xxp1;
	struct cvmx_ndf_cmd_s                 cn73xx;
	struct cvmx_ndf_cmd_s                 cnf75xx;
};
typedef union cvmx_ndf_cmd cvmx_ndf_cmd_t;

/**
 * cvmx_ndf_dma_adr
 *
 * Sixty-four-bit operations must be used to access this register.
 *
 */
union cvmx_ndf_dma_adr {
	uint64_t u64;
	struct cvmx_ndf_dma_adr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t adr                          : 39; /**< DMA engine address. 64-bit aligned. */
	uint64_t reserved_0_2                 : 3;
#else
	uint64_t reserved_0_2                 : 3;
	uint64_t adr                          : 39;
	uint64_t reserved_42_63               : 22;
#endif
	} s;
	struct cvmx_ndf_dma_adr_s             cn73xx;
	struct cvmx_ndf_dma_adr_s             cnf75xx;
};
typedef union cvmx_ndf_dma_adr cvmx_ndf_dma_adr_t;

/**
 * cvmx_ndf_dma_cfg
 *
 * Sixty-four-bit operations must be used to access this register.
 *
 */
union cvmx_ndf_dma_cfg {
	uint64_t u64;
	struct cvmx_ndf_dma_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t en                           : 1;  /**< DMA engine enable.  This bit is cleared at the termination of the DMA. */
	uint64_t rw                           : 1;  /**< DMA engine R/W bit: 0 = read, 1 = write. */
	uint64_t clr                          : 1;  /**< DMA engine clear EN. When set to 1, DMA is terminated and EN is cleared and the DMA_DONE
                                                         interrupt
                                                         occurs when either the SIZE is exhausted (normal termination) or the NDF BUS_REL is
                                                         issued. */
	uint64_t reserved_60_60               : 1;
	uint64_t swap32                       : 1;  /**< DMA engine 32-bit swap. */
	uint64_t swap16                       : 1;  /**< DMA engine enable 16-bit swap. */
	uint64_t swap8                        : 1;  /**< DMA engine enable 8-bit swap. */
	uint64_t endian                       : 1;  /**< DMA engine endian mode: 0 = big-endian, 1 = little-endian. */
	uint64_t size                         : 20; /**< DMA engine size. Specified in the number of 64-bit transfers (encoded in -1 notation). */
	uint64_t reserved_0_35                : 36;
#else
	uint64_t reserved_0_35                : 36;
	uint64_t size                         : 20;
	uint64_t endian                       : 1;
	uint64_t swap8                        : 1;
	uint64_t swap16                       : 1;
	uint64_t swap32                       : 1;
	uint64_t reserved_60_60               : 1;
	uint64_t clr                          : 1;
	uint64_t rw                           : 1;
	uint64_t en                           : 1;
#endif
	} s;
	struct cvmx_ndf_dma_cfg_s             cn73xx;
	struct cvmx_ndf_dma_cfg_s             cnf75xx;
};
typedef union cvmx_ndf_dma_cfg cvmx_ndf_dma_cfg_t;

/**
 * cvmx_ndf_drbell
 *
 * This register is designed to control the execution of the NAND flash commands. The NDF
 * command-execution unit must arbitrate for the boot bus before it can enable a NAND flash
 * device connected to the CNXXXX, which it then does by asserting the device's chip-enable
 * signal. Therefore software must first load the NDF_CMD queue, with a full sequence of commands
 * to perform a NAND flash device task.
 *
 * This command sequence starts with a BUS_ACQ command, and the last command in the sequence must
 * be a BUS_REL command. The execution unit starts execution of the sequence only if the
 * NDF_DRBELL[CNT] is nonzero when it fetches the BUS_ACQ command.
 *
 * Software can load multiple such sequences, each starting with a CHIP_EN command and ending
 * with a CHIP_DIS command, and then write a data value to this register to increment the CNT
 * field by the number of the command sequences loaded to the NDF_CMD queue.
 *
 * Software register-write operations increment CNT by the signed 8-bit value being written.
 * Software register-read operations return the current CNT value.
 *
 * Hardware can also modifies the value of CNT. Every time hardware executes a BUS_ACQ command to
 * arbitrate and win the boot bus, it decrements CNT by 1. If CNT is already 0 or negative, the
 * hardware command-execution unit stalls when it fetches the new BUS_ACQ command from the
 * NDF_CMD queue. Only when the software writes to this register with a nonzero data value can
 * the execution unit come out of the stalled condition, and resume execution.
 *
 * Sixty-four-bit operations must be used to access this register.
 */
union cvmx_ndf_drbell {
	uint64_t u64;
	struct cvmx_ndf_drbell_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t cnt                          : 8;  /**< Doorbell count, in 2s-complement format. */
#else
	uint64_t cnt                          : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_ndf_drbell_s              cn52xx;
	struct cvmx_ndf_drbell_s              cn63xx;
	struct cvmx_ndf_drbell_s              cn63xxp1;
	struct cvmx_ndf_drbell_s              cn66xx;
	struct cvmx_ndf_drbell_s              cn68xx;
	struct cvmx_ndf_drbell_s              cn68xxp1;
	struct cvmx_ndf_drbell_s              cn70xx;
	struct cvmx_ndf_drbell_s              cn70xxp1;
	struct cvmx_ndf_drbell_s              cn73xx;
	struct cvmx_ndf_drbell_s              cnf75xx;
};
typedef union cvmx_ndf_drbell cvmx_ndf_drbell_t;

/**
 * cvmx_ndf_ecc_cnt
 *
 * Sixty-four-bit operations must be used to access this register.
 *
 */
union cvmx_ndf_ecc_cnt {
	uint64_t u64;
	struct cvmx_ndf_ecc_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t xor_ecc                      : 24; /**< Result of XOR operation of ECC read bytes and ECC generated bytes. The value pertains to
                                                         the last single-bit ECC error.
                                                         _ XOR_ECC =[ECC_gen_byt258, ECC_gen_byt257, ECC_gen_byt256] ^
                                                           [ECC_258, ECC_257, ECC_256]
                                                         ECC_258, ECC_257 and ECC_256 are bytes stored in the NAND flash device and read out during
                                                         boot.
                                                         ECC_gen_byt258, ECC_gen_byt257, ECC_gen_byt256 are generated from data read out from the
                                                         NAND flash device. */
	uint64_t ecc_err                      : 8;  /**< ECC error count. The number of single-bit errors fixed during boot. */
#else
	uint64_t ecc_err                      : 8;
	uint64_t xor_ecc                      : 24;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_ndf_ecc_cnt_s             cn52xx;
	struct cvmx_ndf_ecc_cnt_s             cn63xx;
	struct cvmx_ndf_ecc_cnt_s             cn63xxp1;
	struct cvmx_ndf_ecc_cnt_s             cn66xx;
	struct cvmx_ndf_ecc_cnt_s             cn68xx;
	struct cvmx_ndf_ecc_cnt_s             cn68xxp1;
	struct cvmx_ndf_ecc_cnt_s             cn70xx;
	struct cvmx_ndf_ecc_cnt_s             cn70xxp1;
	struct cvmx_ndf_ecc_cnt_s             cn73xx;
	struct cvmx_ndf_ecc_cnt_s             cnf75xx;
};
typedef union cvmx_ndf_ecc_cnt cvmx_ndf_ecc_cnt_t;

/**
 * cvmx_ndf_int
 *
 * This register contains the bits that can trigger an error interrupt. Sixty-four-bit operations
 * must be used to access this register.
 */
union cvmx_ndf_int {
	uint64_t u64;
	struct cvmx_ndf_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t dma_done                     : 1;  /**< DMA engine request completion interrupt. */
	uint64_t ovrf                         : 1;  /**< NDF_CMD write when FIFO is full. Generally a fatal error. */
	uint64_t ecc_mult                     : 1;  /**< Multibit ECC error detected during boot. */
	uint64_t ecc_1bit                     : 1;  /**< Single-bit ECC error detected and fixed during boot. */
	uint64_t sm_bad                       : 1;  /**< One of the state machines is in a bad state, */
	uint64_t wdog                         : 1;  /**< Watchdog timer expired during command execution. */
	uint64_t full                         : 1;  /**< NDF_CMD queue is full. FULL status is updated when the NDF_CMD queue becomes full as a
                                                         result of software writing a new command to it. */
	uint64_t empty                        : 1;  /**< NDF_CMD queue is empty. EMPTY status is updated when the NDF_CMD queue becomes empty as a
                                                         result of command execution unit fetching the last instruction out of the NDF_CMD queue. */
#else
	uint64_t empty                        : 1;
	uint64_t full                         : 1;
	uint64_t wdog                         : 1;
	uint64_t sm_bad                       : 1;
	uint64_t ecc_1bit                     : 1;
	uint64_t ecc_mult                     : 1;
	uint64_t ovrf                         : 1;
	uint64_t dma_done                     : 1;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_ndf_int_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t ovrf                         : 1;  /**< NDF_CMD write when fifo is full. Generally a
                                                         fatal error. */
	uint64_t ecc_mult                     : 1;  /**< Multi bit ECC error detected during boot */
	uint64_t ecc_1bit                     : 1;  /**< Single bit ECC error detected and fixed during boot */
	uint64_t sm_bad                       : 1;  /**< One of the state machines in a bad state */
	uint64_t wdog                         : 1;  /**< Watch Dog timer expired during command execution */
	uint64_t full                         : 1;  /**< Command fifo is full */
	uint64_t empty                        : 1;  /**< Command fifo is empty */
#else
	uint64_t empty                        : 1;
	uint64_t full                         : 1;
	uint64_t wdog                         : 1;
	uint64_t sm_bad                       : 1;
	uint64_t ecc_1bit                     : 1;
	uint64_t ecc_mult                     : 1;
	uint64_t ovrf                         : 1;
	uint64_t reserved_7_63                : 57;
#endif
	} cn52xx;
	struct cvmx_ndf_int_cn52xx            cn63xx;
	struct cvmx_ndf_int_cn52xx            cn63xxp1;
	struct cvmx_ndf_int_cn52xx            cn66xx;
	struct cvmx_ndf_int_cn52xx            cn68xx;
	struct cvmx_ndf_int_cn52xx            cn68xxp1;
	struct cvmx_ndf_int_cn52xx            cn70xx;
	struct cvmx_ndf_int_cn52xx            cn70xxp1;
	struct cvmx_ndf_int_s                 cn73xx;
	struct cvmx_ndf_int_s                 cnf75xx;
};
typedef union cvmx_ndf_int cvmx_ndf_int_t;

/**
 * cvmx_ndf_int_en
 *
 * Notes:
 * Like all NDF_... registers, 64-bit operations must be used to access this register
 *
 */
union cvmx_ndf_int_en {
	uint64_t u64;
	struct cvmx_ndf_int_en_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t ovrf                         : 1;  /**< Wrote to a full command fifo */
	uint64_t ecc_mult                     : 1;  /**< Multi bit ECC error detected during boot */
	uint64_t ecc_1bit                     : 1;  /**< Single bit ECC error detected and fixed during boot */
	uint64_t sm_bad                       : 1;  /**< One of the state machines in a bad state */
	uint64_t wdog                         : 1;  /**< Watch Dog timer expired during command execution */
	uint64_t full                         : 1;  /**< Command fifo is full */
	uint64_t empty                        : 1;  /**< Command fifo is empty */
#else
	uint64_t empty                        : 1;
	uint64_t full                         : 1;
	uint64_t wdog                         : 1;
	uint64_t sm_bad                       : 1;
	uint64_t ecc_1bit                     : 1;
	uint64_t ecc_mult                     : 1;
	uint64_t ovrf                         : 1;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_ndf_int_en_s              cn52xx;
	struct cvmx_ndf_int_en_s              cn63xx;
	struct cvmx_ndf_int_en_s              cn63xxp1;
	struct cvmx_ndf_int_en_s              cn66xx;
	struct cvmx_ndf_int_en_s              cn68xx;
	struct cvmx_ndf_int_en_s              cn68xxp1;
	struct cvmx_ndf_int_en_s              cn70xx;
	struct cvmx_ndf_int_en_s              cn70xxp1;
};
typedef union cvmx_ndf_int_en cvmx_ndf_int_en_t;

/**
 * cvmx_ndf_int_w1s
 */
union cvmx_ndf_int_w1s {
	uint64_t u64;
	struct cvmx_ndf_int_w1s_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t dma_done                     : 1;  /**< DMA engine request completion interrupt. */
	uint64_t ovrf                         : 1;  /**< NDF_CMD write when FIFO is full. Generally a fatal error. */
	uint64_t ecc_mult                     : 1;  /**< Multibit ECC error detected during boot. */
	uint64_t ecc_1bit                     : 1;  /**< Single-bit ECC error detected and fixed during boot. */
	uint64_t sm_bad                       : 1;  /**< One of the state machines is in a bad state, */
	uint64_t wdog                         : 1;  /**< Watchdog timer expired during command execution. */
	uint64_t full                         : 1;  /**< NDF_CMD queue is full. FULL status is updated when the NDF_CMD queue becomes full as a
                                                         result of software writing a new command to it. */
	uint64_t empty                        : 1;  /**< NDF_CMD queue is empty. EMPTY status is updated when the NDF_CMD queue becomes empty as a
                                                         result of command execution unit fetching the last instruction out of the NDF_CMD queue. */
#else
	uint64_t empty                        : 1;
	uint64_t full                         : 1;
	uint64_t wdog                         : 1;
	uint64_t sm_bad                       : 1;
	uint64_t ecc_1bit                     : 1;
	uint64_t ecc_mult                     : 1;
	uint64_t ovrf                         : 1;
	uint64_t dma_done                     : 1;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_ndf_int_w1s_s             cn73xx;
	struct cvmx_ndf_int_w1s_s             cnf75xx;
};
typedef union cvmx_ndf_int_w1s cvmx_ndf_int_w1s_t;

/**
 * cvmx_ndf_misc
 *
 * Sixty-four-bit operations must be used to access this register.
 *
 */
union cvmx_ndf_misc {
	uint64_t u64;
	struct cvmx_ndf_misc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t mb_dis                       : 1;  /**< Set to disable multi-bit error hangs. Allows boot loads and boot DMAs to proceed as if no
                                                         multi-bit errors occurred. Hardware fixes single bit errors as usual. */
	uint64_t nbr_hwm                      : 3;  /**< High watermark for NBR FIFO or load/store operations. Specifies the high
                                                         watermark for the IOI outbound load/store commands receive FIFO. NBR_HWM+1 is used
                                                         as the high watermark.  So a value of 0 allows 1 entry in the FIFO at a time.  The
                                                         FIFO size is 8 entries. */
	uint64_t wait_cnt                     : 6;  /**< Wait input filter count. Represents the number of coprocessor-clock cycles for glitch
                                                         filtering of BOOT_WAIT_L from the NAND flash device. */
	uint64_t fr_byt                       : 11; /**< Unfilled NDF_CMD queue bytes. Specifies the number of unfilled bytes in the
                                                         NDF_CMD queue. Bytes become unfilled as commands complete execution and exit. (FIFO is 256
                                                         bytes when BT_DIS = 0 and 1536 bytes when BT_DIS = 1.) */
	uint64_t rd_done                      : 1;  /**< Read done. This bit is set to 1 by hardware when it reads the last eight bytes out of the
                                                         NDF_CMD queue in response to [RD_CMD] being set to 1 by software. */
	uint64_t rd_val                       : 1;  /**< This read-only bit is set to 1 by hardware when it reads the next eight bytes from NDF_CMD
                                                         queue in response to RD_CMD being set to 1. A software read of NDF_CMD clears this bit to
                                                         0. */
	uint64_t rd_cmd                       : 1;  /**< Read command. When set to 1, the hardware reads the contents of the NDF_CMD queue eight
                                                         bytes at a time and places the data into NDF_CMD. Software should first read RD_VAL to see
                                                         if the next eight bytes from the NDF_CMD queue are available in NDF_CMD.
                                                         All NDF_CMD queue read operations start and end on an eight-byte boundary. A RD_CMD
                                                         command in the middle of command execution causes the execution to freeze until RD_DONE is
                                                         set to 1.
                                                         This bit is cleared on any NDF_CMD software write command. */
	uint64_t bt_dma                       : 1;  /**< Boot-time DMA enable. When set to 1, boot-time DMA is enabled. This indicates to the NAND
                                                         flash boot-control state machine that boot DMA read operations can begin. Software should
                                                         set this bit to 1 after loading the NDF_CMD queue. Hardware sets the bit to 0 when boot
                                                         DMA command execution is complete. If chip-enable 0 is not a NAND flash device, this bit
                                                         is permanently 0 with software write operations ignored.
                                                         When [BT_DIS] = 1, this bit is 0. */
	uint64_t bt_dis                       : 1;  /**< Boot disable. When the boot operation is over, software must set this field to 1, which
                                                         causes the boot-control state machines to sleep.
                                                         This bit indicates to the NAND flash boot-control state machine that boot operation has
                                                         ended. When this bit changes from 0 -> 1, the NDF_CMD queue is emptied as a side effect.
                                                         This bit must never be set when booting from NAND flash and region zero is enabled. */
	uint64_t ex_dis                       : 1;  /**< Execution disable. When set to 1, this bit stops command execution after completing the
                                                         execution of all commands currently in the NDF_CMD queue. Once command execution has
                                                         stopped and then new commands are loaded into the NDF_CMD queue, command execution does
                                                         not resume while this bit is 1.
                                                         When this bit is set to 0, execution resumes if the NDF_CMD queue is not empty. */
	uint64_t rst_ff                       : 1;  /**< Reset FIFO.
                                                         0 = Normal operation.
                                                         1 = Reset NDF_CMD queue to empty it; any command in flight is not aborted before
                                                         resetting. The FIFO comes up empty at the end of power on reset. */
#else
	uint64_t rst_ff                       : 1;
	uint64_t ex_dis                       : 1;
	uint64_t bt_dis                       : 1;
	uint64_t bt_dma                       : 1;
	uint64_t rd_cmd                       : 1;
	uint64_t rd_val                       : 1;
	uint64_t rd_done                      : 1;
	uint64_t fr_byt                       : 11;
	uint64_t wait_cnt                     : 6;
	uint64_t nbr_hwm                      : 3;
	uint64_t mb_dis                       : 1;
	uint64_t reserved_28_63               : 36;
#endif
	} s;
	struct cvmx_ndf_misc_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_27_63               : 37;
	uint64_t nbr_hwm                      : 3;  /**< Hi Water mark for NBR fifo or load/stores */
	uint64_t wait_cnt                     : 6;  /**< WAIT input filter count */
	uint64_t fr_byt                       : 11; /**< Number of unfilled Command fifo bytes */
	uint64_t rd_done                      : 1;  /**< This W1C bit is set to 1 by HW when it completes
                                                         command fifo read out, in response to RD_CMD */
	uint64_t rd_val                       : 1;  /**< This RO bit is set to 1 by HW when it reads next 8
                                                         bytes from Command fifo into the NDF_CMD csr
                                                         SW reads NDF_CMD csr, HW clears this bit to 0 */
	uint64_t rd_cmd                       : 1;  /**< When 1, HW reads out contents of the Command fifo 8
                                                         bytes at a time into the NDF_CMD csr */
	uint64_t bt_dma                       : 1;  /**< When set to 1, boot time dma is enabled */
	uint64_t bt_dis                       : 1;  /**< When boot operation is over SW must set to 1
                                                         causes boot state mchines to sleep */
	uint64_t ex_dis                       : 1;  /**< When set to 1, suspends execution of commands at
                                                         next command in the fifo. */
	uint64_t rst_ff                       : 1;  /**< 1=reset command fifo to make it empty,
                                                         0=normal operation */
#else
	uint64_t rst_ff                       : 1;
	uint64_t ex_dis                       : 1;
	uint64_t bt_dis                       : 1;
	uint64_t bt_dma                       : 1;
	uint64_t rd_cmd                       : 1;
	uint64_t rd_val                       : 1;
	uint64_t rd_done                      : 1;
	uint64_t fr_byt                       : 11;
	uint64_t wait_cnt                     : 6;
	uint64_t nbr_hwm                      : 3;
	uint64_t reserved_27_63               : 37;
#endif
	} cn52xx;
	struct cvmx_ndf_misc_s                cn63xx;
	struct cvmx_ndf_misc_s                cn63xxp1;
	struct cvmx_ndf_misc_s                cn66xx;
	struct cvmx_ndf_misc_s                cn68xx;
	struct cvmx_ndf_misc_s                cn68xxp1;
	struct cvmx_ndf_misc_s                cn70xx;
	struct cvmx_ndf_misc_s                cn70xxp1;
	struct cvmx_ndf_misc_s                cn73xx;
	struct cvmx_ndf_misc_s                cnf75xx;
};
typedef union cvmx_ndf_misc cvmx_ndf_misc_t;

/**
 * cvmx_ndf_st_reg
 *
 * This register aggregates all state machines used in NAND flash controller for debug purposes.
 *
 */
union cvmx_ndf_st_reg {
	uint64_t u64;
	struct cvmx_ndf_st_reg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t exe_idle                     : 1;  /**< Command execution status
                                                         0 = Busy.
                                                         1 = Idle (execution of command sequence is complete and NDF_CMD queue is empty). */
	uint64_t exe_sm                       : 4;  /**< Command-execution state-machine states. */
	uint64_t bt_sm                        : 4;  /**< Boot-load and boot-DMA state-machine states. */
	uint64_t rd_ff_bad                    : 1;  /**< The NDF_CMD-queue read-back state machine is in a 'bad' state. */
	uint64_t rd_ff                        : 2;  /**< NDF_CMD-queue read-back state machine states. */
	uint64_t main_bad                     : 1;  /**< The main state machine is in a 'bad' state. */
	uint64_t main_sm                      : 3;  /**< Main state machine states. */
#else
	uint64_t main_sm                      : 3;
	uint64_t main_bad                     : 1;
	uint64_t rd_ff                        : 2;
	uint64_t rd_ff_bad                    : 1;
	uint64_t bt_sm                        : 4;
	uint64_t exe_sm                       : 4;
	uint64_t exe_idle                     : 1;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_ndf_st_reg_s              cn52xx;
	struct cvmx_ndf_st_reg_s              cn63xx;
	struct cvmx_ndf_st_reg_s              cn63xxp1;
	struct cvmx_ndf_st_reg_s              cn66xx;
	struct cvmx_ndf_st_reg_s              cn68xx;
	struct cvmx_ndf_st_reg_s              cn68xxp1;
	struct cvmx_ndf_st_reg_s              cn70xx;
	struct cvmx_ndf_st_reg_s              cn70xxp1;
	struct cvmx_ndf_st_reg_s              cn73xx;
	struct cvmx_ndf_st_reg_s              cnf75xx;
};
typedef union cvmx_ndf_st_reg cvmx_ndf_st_reg_t;

#endif
