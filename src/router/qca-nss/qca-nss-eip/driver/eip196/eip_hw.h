/*
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __EIP_HW_H
#define __EIP_HW_H

#include <linux/types.h>

/*
 * Common configuration data for command and result
 * descriptor rings
 */
#define EIP_HW_DESC_WORDS (sizeof(struct eip_hw_desc) >> 2)
#define EIP_HW_ENB_DMA ((0x6U << 28) | (0x7U << 24))
#define EIP_HW_CMD_SZ 0xc0100010U
#define EIP_HW_CMD_FETCH_SZ ((0x10U << 16) | 0x10U)
#define EIP_HW_RST_CMD 0x1FU

/*
 * Result Descriptor ring specific Macros
 */
#define EIP_HW_RES_SZ 0x80100010U
#define EIP_HW_RES_FETCH_SZ 0x2100010U
#define EIP_HW_RES_PROC_SZ (0x1U << 10)
#define EIP_HW_RST_RES 0xFFU

/*
 * EIP196 Internal DMA specific configuration
 */
#define EIP_HW_CLR_CNT (0x1U << 31)
#define EIP_HW_CLR_DMA (0x1U << 24)
#define EIP_HW_CLR_DMA_CTRL (0x1U << 23)

/*
 * Data Fetch Engine related Fields
 */
#define EIP_HW_DFE_RST_DONE 0x0F000U
#define EIP_HW_DFE_RST_CTRL 0x80000000U
#define EIP_HW_DFE_CFG 0xA7150915U

/*
 * Data Store Engine related fields
 */
#define EIP_HW_DSE_RST_DONE 0xF000U
#define EIP_HW_DSE_AGGRESIVE_MODE (0x1U << 31)
#define EIP_HW_DSE_EN_SINGLE_WR (0x1U << 29)
#define EIP_HW_DSE_BUFFER_CTRL (0x1U << 15)
#define EIP_HW_DSE_RD_CACHE (0x1U << 4)
#define EIP_HW_DSE_OUTPUT_BUF_MIN 7U
#define EIP_HW_DSE_OUTPUT_BUF_MAX (0x8U << 8)
#define EIP_HW_ENB_DSE_THREAD (0x1U << 30)

#define EIP_HW_DSE_CFG \
	(EIP_HW_DSE_AGGRESIVE_MODE | EIP_HW_DSE_EN_SINGLE_WR | \
	EIP_HW_DSE_BUFFER_CTRL | EIP_HW_DSE_OUTPUT_BUF_MAX | \
	EIP_HW_DSE_RD_CACHE | EIP_HW_DSE_OUTPUT_BUF_MIN)

/*
 * HIA LA mode control value
 */
#define EIP_HW_HIA_MST_CFG 0xfe000033U
#define EIP_HW_MST_CFG 0x22U

/*
 * EIP_HW modules reset value
 */
#define EIP_HW_HIA_RST (0x1U << 31)
#define EIP_HW_DSE_CFG_RST ((0x1U << 31) | (0x1U << 15))
#define EIP_HW_TOKEN_CTRL_RST 0x4004U
#define EIP_HW_OUT_TRANS_RST 0xfc400847U
#define EIP_HW_RST_PE 0xc001U
#define EIP_HW_TOKEN_CTRL2_RST 0x8U

/*
 * Interrupt value
 */
#define EIP_HW_HIA_IRQ_DISABLE 0x0
#define EIP_HW_HIA_IRQ_CLEAR 0xFFFFFFFFU

#define EIP_HW_RDR_PROC_IRQ_STATUS 0x10U /* proc_cd_thresh_irq */
#define EIP_HW_CDR_PROC_IRQ_STATUS 0x10U /* proc_cd_thresh_irq */
#define EIP_HW_RDR_PREP_IRQ_STATUS 0x02U /* prep_cd_thresh_irq */
#define EIP_HW_CDR_PREP_IRQ_STATUS 0x02U /* prep_cd_thresh_irq */

/*
 * HIA Ring Arbiter Macros
 * Used to enable Rings
 */
#define EIP_HW_ENB_ALL_RINGS 0x400002FFU

/*
 * We need 4 words of metadata to be passed from
 * input to output descriptor. The following macro
 * is used to enable this feature
 */
#define EIP_HW_ENB_MDATA (0x1U << 31)

/*
 * Disable ECN check.
 */
#define EIP_HW_PE_EIP96_ECN_DISABLE 0x0

/*
 * Configure Processing engine token control register
 */
#define EIP_HW_TOKEN_CFG 0x40424004U

/*
 * Macros used to configure Processing engine:
 * - IPBUF: Input buffer max and min threshold
 * - ITBUF: Input token max and min threshold
 * - OPBUF: Output buffer max and min threshold
 */
#define EIP_HW_PE_IPBUF_MAX (0x9U << 12)
#define EIP_HW_PE_IPBUF_MIN (0x5U << 8)
#define EIP_HW_PE_ITBUF_MAX (0x7U << 12)
#define EIP_HW_PE_ITBUF_MIN (0x5U << 8)
#define EIP_HW_PE_OPBUF_MAX (0x8U << 4)
#define EIP_HW_PE_OPBUF_MIN 0x7U

#define EIP_HW_INDATA_THR \
	(EIP_HW_PE_IPBUF_MAX | EIP_HW_PE_IPBUF_MIN)
#define EIP_HW_INTOKEN_THR \
	(EIP_HW_PE_ITBUF_MAX | EIP_HW_PE_ITBUF_MIN)
#define EIP_HW_OUTDATA_THR \
	(EIP_HW_PE_OPBUF_MAX | EIP_HW_PE_OPBUF_MIN)

#define EIP_HW_ENB_SIMPLE_TRANS_REC 0x100U

/*
 * Simple Transform Record Cache (STRC) configuration data
 */
#define EIP_HW_STRC_CONFIG_VAL 0x8000352EU

/*
 * Flow lookup config
 */
#define EIP_HW_ENB_FLUE 0xc0000000U
#define EIP_HW_FLUE_OFFSET_CFG 0x400004U

/*
 * HIA base register
 */
#define EIP_HW_BASE 0x39880000U

/*
 * Command Descriptor Ring and Result Descriptor Ring registers
 */
#define EIP_HW_HIA_BASE 0x0
#define EIP_HW_HIA_CDR_RING_BASE_ADDR_LO(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x0)
#define EIP_HW_HIA_CDR_RING_BASE_ADDR_HI(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x4U)
#define EIP_HW_HIA_CDR_DATA_BASE_ADDR_LO(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x8U)
#define EIP_HW_HIA_CDR_DATA_BASE_ADDR_HI(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0xCU)
#define EIP_HW_HIA_CDR_ATOK_BASE_ADDR_LO(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x10U)
#define EIP_HW_HIA_CDR_ATOK_BASE_ADDR_HI(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x14U)
#define EIP_HW_HIA_CDR_RING_SIZE(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x18U)
#define EIP_HW_HIA_CDR_DESC_SIZE(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x1CU)
#define EIP_HW_HIA_CDR_CFG(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x20U)
#define EIP_HW_HIA_CDR_DMA_CFG(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x24U)
#define EIP_HW_HIA_CDR_THRESH(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x28U)
#define EIP_HW_HIA_CDR_PREP_COUNT(i) (EIP_HW_HIA_BASE + (0x1000U * (i)) + 0x2CU)
#define EIP_HW_HIA_CDR_PROC_COUNT(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x30U)
#define EIP_HW_HIA_CDR_PREP_PNTR(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x34U)
#define EIP_HW_HIA_CDR_PROC_PNTR(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x38U)
#define EIP_HW_HIA_CDR_STAT(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x3CU)
#define EIP_HW_HIA_CDR_OPTIONS(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x7F8U)
#define EIP_HW_HIA_CDR_VERSION(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x7FCU)
#define EIP_HW_HIA_RDR_RING_BASE_ADDR_LO(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x800U)
#define EIP_HW_HIA_RDR_RING_BASE_ADDR_HI(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x804U)
#define EIP_HW_HIA_RDR_DATA_BASE_ADDR_LO(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x808U)
#define EIP_HW_HIA_RDR_DATA_BASE_ADDR_HI(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x80CU)
#define EIP_HW_HIA_RDR_RING_SIZE(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x818U)
#define EIP_HW_HIA_RDR_DESC_SIZE(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x81CU)
#define EIP_HW_HIA_RDR_CFG(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x820U)
#define EIP_HW_HIA_RDR_DMA_CFG(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x824U)
#define EIP_HW_HIA_RDR_THRESH(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x828U)
#define EIP_HW_HIA_RDR_PREP_COUNT(i) (EIP_HW_HIA_BASE + (0x1000U * (i)) + 0x82CU)
#define EIP_HW_HIA_RDR_PROC_COUNT(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x830U)
#define EIP_HW_HIA_RDR_PREP_PNTR(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x834U)
#define EIP_HW_HIA_RDR_PROC_PNTR(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x838U)
#define EIP_HW_HIA_RDR_STAT(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0x83CU)
#define EIP_HW_HIA_RDR_OPTIONS(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0xFF8U)
#define EIP_HW_HIA_RDR_VERSION(i) (EIP_HW_HIA_BASE + (0x1000U * i) + 0xFFCU)

/*
 * Data Fetch Engine registers
 */
#define EIP_HW_HIA_DFE_CFG (EIP_HW_HIA_BASE + 0xC000U)
#define EIP_HW_HIA_DFE_THR_CTRL (EIP_HW_HIA_BASE + 0xC040U)
#define EIP_HW_HIA_DFE_THR_STAT (EIP_HW_HIA_BASE + 0xC044U)
#define EIP_HW_HIA_DFE_THR_DESC_CTRL (EIP_HW_HIA_BASE + 0xC048U)
#define EIP_HW_HIA_DFE_THR_DESC_DPTR_L (EIP_HW_HIA_BASE + 0xC050U)
#define EIP_HW_HIA_DFE_THR_DESC_DPTR_H (EIP_HW_HIA_BASE + 0xC054U)
#define EIP_HW_HIA_DFE_THR_DESC_ACDPTR_L (EIP_HW_HIA_BASE + 0xC058U)
#define EIP_HW_HIA_DFE_THR_DESC_ACDPTR_H (EIP_HW_HIA_BASE + 0xC05CU)
#define EIP_HW_HIA_DFE_OPTION (EIP_HW_HIA_BASE + 0xC078U)
#define EIP_HW_HIA_DFE_VERSION (EIP_HW_HIA_BASE + 0xC07CU)

/*
 * Data Store Engine registers
 */
#define EIP_HW_HIA_DSE_CFG (EIP_HW_HIA_BASE + 0xD000U)
#define EIP_HW_HIA_DSE_THR_CTRL (EIP_HW_HIA_BASE + 0xD040U)
#define EIP_HW_HIA_DSE_THR_STAT (EIP_HW_HIA_BASE + 0xD044U)
#define EIP_HW_HIA_DSE_THR_DESC_CTRL (EIP_HW_HIA_BASE + 0xD048U)
#define EIP_HW_HIA_DSE_OPTION (EIP_HW_HIA_BASE + 0xD078U)
#define EIP_HW_HIA_DSE_VERSION (EIP_HW_HIA_BASE + 0xD07CU)

/*
 * Ring arbiter registers
 */
#define EIP_HW_HIA_RA_AIC_BASE 0x10000U
#define EIP_HW_HIA_RA_PRIO0 (EIP_HW_HIA_RA_AIC_BASE + 0x0)
#define EIP_HW_HIA_RA_PRIO1 (EIP_HW_HIA_RA_AIC_BASE + 0x4U)
#define EIP_HW_HIA_RA_PRIO2 (EIP_HW_HIA_RA_AIC_BASE + 0x8U)
#define EIP_HW_HIA_RA_PE_CTRL (EIP_HW_HIA_RA_AIC_BASE + 0x10U)
#define EIP_HW_HIA_RA_PE_STAT (EIP_HW_HIA_RA_AIC_BASE + 0x14U)

#define EIP_HW_HIA_AIC_G_ENABLE_CTRL (EIP_HW_HIA_RA_AIC_BASE + 0xF808U)
#define EIP_HW_HIA_AIC_G_ACK (EIP_HW_HIA_RA_AIC_BASE + 0xF810U)

#define EIP_HW_HIA_AIC_R_ENABLE_CTRL(i) (EIP_HW_HIA_RA_AIC_BASE + 0xE808U - (0x1000U * (i)))
#define EIP_HW_HIA_AIC_R0_ENABLE_CTRL (EIP_HW_HIA_RA_AIC_BASE + 0xE808U)
#define EIP_HW_HIA_AIC_R1_ENABLE_CTRL (EIP_HW_HIA_RA_AIC_BASE + 0xD808U)
#define EIP_HW_HIA_AIC_R2_ENABLE_CTRL (EIP_HW_HIA_RA_AIC_BASE + 0xC808U)
#define EIP_HW_HIA_AIC_R3_ENABLE_CTRL (EIP_HW_HIA_RA_AIC_BASE + 0xB808U)

#define EIP_HW_HIA_AIC_R0_ACK (EIP_HW_HIA_RA_AIC_BASE + 0xE810U)
#define EIP_HW_HIA_AIC_R1_ACK (EIP_HW_HIA_RA_AIC_BASE + 0xD810U)
#define EIP_HW_HIA_AIC_R2_ACK (EIP_HW_HIA_RA_AIC_BASE + 0xC810U)
#define EIP_HW_HIA_AIC_R3_ACK (EIP_HW_HIA_RA_AIC_BASE + 0xB810U)

#define EIP_HW_HIA_MST_TIMEOUT_ERR (EIP_HW_HIA_RA_AIC_BASE + 0xFFD0U)
#define EIP_HW_HIA_OPTIONS2 (EIP_HW_HIA_RA_AIC_BASE + 0xFFF0U)
#define EIP_HW_HIA_MST_CTRL (EIP_HW_HIA_RA_AIC_BASE + 0xFFF4U)

/*
 * Processing engine registers
 */
#define EIP_HW_PE_BASE 0x20000U
#define EIP_HW_PE_IN_DBUF_THR (EIP_HW_PE_BASE + 0x0)
#define EIP_HW_PE_IN_TBUF_THR (EIP_HW_PE_BASE + 0x100U)
#define EIP_HW_PE_MID_DBUF_THR (EIP_HW_PE_BASE + 0x400U)
#define EIP_HW_PE_MID_TBUF_THR (EIP_HW_PE_BASE + 0x500U)

/*
 * Context Registers
 */
#define EIP_HW_PE_EIP96_BASE 0x21000U
#define EIP_HW_PE_EIP96_TOKEN_CTRL (EIP_HW_PE_EIP96_BASE + 0x0)
#define EIP_HW_PE_EIP96_FUNC_EN (EIP_HW_PE_EIP96_BASE + 0x4U)
#define EIP_HW_PE_EIP96_CONTEXT_CTRL (EIP_HW_PE_EIP96_BASE + 0x8U)
#define EIP_HW_PE_EIP96_CONTEXT_STAT (EIP_HW_PE_EIP96_BASE + 0xCU)
#define EIP_HW_PE_EIP96_OUT_BUF_CTRL (EIP_HW_PE_EIP96_BASE + 0x1CU)
#define EIP_HW_PE_EIP96_TOKEN_CTRL2 (EIP_HW_PE_EIP96_BASE + 0x2CU)
#define EIP_HW_PE_EIP96_LFSR_LO (EIP_HW_PE_EIP96_BASE + 0x70U)
#define EIP_HW_PE_EIP96_LFSR_HI (EIP_HW_PE_EIP96_BASE + 0x74U)
#define EIP_HW_PE_EIP96_VERSION (EIP_HW_PE_EIP96_BASE + 0x3FCU)
#define EIP_HW_PE_EIP96_ECN_TABLE(x) (EIP_HW_PE_EIP96_BASE + 0x3E0U + ((x) * 4U))

#define EIP_HW_PE_IV0 (EIP_HW_PE_EIP96_BASE + 0x210U)
#define EIP_HW_PE_IV1 (EIP_HW_PE_EIP96_BASE + 0x214U)
#define EIP_HW_PE_IV2 (EIP_HW_PE_EIP96_BASE + 0x218U)
#define EIP_HW_PE_IV3 (EIP_HW_PE_EIP96_BASE + 0x21CU)

/*
 * DRBG Registers
 */
#define EIP_HW_DRBG_BASE		0x77000U
#define EIP_HW_DRBG_IO_0		(EIP_HW_DRBG_BASE + 0x00)
#define EIP_HW_DRBG_IO_1		(EIP_HW_DRBG_BASE + 0x04U)
#define EIP_HW_DRBG_IO_2		(EIP_HW_DRBG_BASE + 0x08U)
#define EIP_HW_DRBG_IO_3		(EIP_HW_DRBG_BASE + 0x0CU)
#define EIP_HW_DRBG_STATUS		(EIP_HW_DRBG_BASE + 0x10U)
#define EIP_HW_DRBG_INTACK		(EIP_HW_DRBG_BASE + 0x10U)
#define EIP_HW_DRBG_CONTROL		(EIP_HW_DRBG_BASE + 0x14U)
#define EIP_HW_DRBG_GENERATE_CNT	(EIP_HW_DRBG_BASE + 0x20U)
#define EIP_HW_DRBG_RESEED_THR_EARLY	(EIP_HW_DRBG_BASE + 0x24U)
#define EIP_HW_DRBG_RESEED_THR		(EIP_HW_DRBG_BASE + 0x28U)
#define EIP_HW_DRBG_GEN_BLK_SIZE	(EIP_HW_DRBG_BASE + 0x2CU)
#define EIP_HW_DRBG_KEY(x)		(EIP_HW_DRBG_BASE + 0x40U + (x << 2))
#define EIP_HW_DRBG_PS_AI(x)		(EIP_HW_DRBG_BASE + 0x40U + (x << 2))
#define EIP_HW_DRBG_TEST		(EIP_HW_DRBG_BASE + 0x70U)
#define EIP_HW_DRBG_VERSION		(EIP_HW_DRBG_BASE + 0x7CU)

#define EIP_HW_DRBG_INITIAL_SEED0		0xe53dbcf0U
#define EIP_HW_DRBG_INITIAL_SEED1		0x97e51ca6U
#define EIP_HW_DRBG_INITIAL_SEED2		0x2776fe0eU
#define EIP_HW_DRBG_INITIAL_SEED3		0x2a52cc12U
#define EIP_HW_DRBG_INITIAL_SEED4		0xa49e11b7U
#define EIP_HW_DRBG_INITIAL_SEED5		0xbcfe4435U
#define EIP_HW_DRBG_INITIAL_SEED6		0x1a39ec92U
#define EIP_HW_DRBG_INITIAL_SEED7		0x2e02d2a6U
#define EIP_HW_DRBG_INITIAL_SEED8		0xcbbe9598U
#define EIP_HW_DRBG_INITIAL_SEED9		0x20b2bddaU
#define EIP_HW_DRBG_INITIAL_SEED10		0x8c2fd968U
#define EIP_HW_DRBG_INITIAL_SEED11		0xfe4c07a2U

#define EIP_HW_DRBG_AI_STATUS_MASK (0x1U << 1)
#define EIP_HW_DRBG_CTRL_STUCKOUT (0x1U << 2)
#define EIP_HW_DRBG_CTRL_EN (0x1U << 10)

/*
 * Cipher Keys and digest are only available in debug mode
 *
 * Note: We will need to add this flag in Makefile to make use of it.
 */
#if defined(NSS_CRYPTO_DEBUG_KEYS)
#define EIP_HW_PE_KEY0 (EIP_HW_PE_EIP96_BASE + 0x220U)
#define EIP_HW_PE_KEY1 (EIP_HW_PE_EIP96_BASE + 0x224U)
#define EIP_HW_PE_KEY2 (EIP_HW_PE_EIP96_BASE + 0x228U)
#define EIP_HW_PE_KEY3 (EIP_HW_PE_EIP96_BASE + 0x22CU)
#define EIP_HW_PE_KEY4 (EIP_HW_PE_EIP96_BASE + 0x230U)
#define EIP_HW_PE_KEY5 (EIP_HW_PE_EIP96_BASE + 0x234U)
#define EIP_HW_PE_KEY6 (EIP_HW_PE_EIP96_BASE + 0x238U)
#define EIP_HW_PE_KEY7 (EIP_HW_PE_EIP96_BASE + 0x23CU)

#define EIP_HW_PE_IDIGEST0 (EIP_HW_PE_EIP96_BASE + 0x240U)
#define EIP_HW_PE_IDIGEST1 (EIP_HW_PE_EIP96_BASE + 0x244U)
#define EIP_HW_PE_IDIGEST2 (EIP_HW_PE_EIP96_BASE + 0x248U)
#define EIP_HW_PE_IDIGEST3 (EIP_HW_PE_EIP96_BASE + 0x24CU)
#define EIP_HW_PE_IDIGEST4 (EIP_HW_PE_EIP96_BASE + 0x250U)
#define EIP_HW_PE_IDIGEST5 (EIP_HW_PE_EIP96_BASE + 0x254U)
#define EIP_HW_PE_IDIGEST6 (EIP_HW_PE_EIP96_BASE + 0x258U)
#define EIP_HW_PE_IDIGEST7 (EIP_HW_PE_EIP96_BASE + 0x25CU)
#define EIP_HW_PE_ODIGEST0 (EIP_HW_PE_EIP96_BASE + 0x260U)
#define EIP_HW_PE_ODIGEST1 (EIP_HW_PE_EIP96_BASE + 0x264U)
#define EIP_HW_PE_ODIGEST2 (EIP_HW_PE_EIP96_BASE + 0x268U)
#define EIP_HW_PE_ODIGEST3 (EIP_HW_PE_EIP96_BASE + 0x26CU)
#define EIP_HW_PE_ODIGEST4 (EIP_HW_PE_EIP96_BASE + 0x270U)
#define EIP_HW_PE_ODIGEST5 (EIP_HW_PE_EIP96_BASE + 0x274U)
#define EIP_HW_PE_ODIGEST6 (EIP_HW_PE_EIP96_BASE + 0x278U)
#define EIP_HW_PE_ODIGEST7 (EIP_HW_PE_EIP96_BASE + 0x27CU)
#endif

/*
 * Output classification engine registers
 */
#define EIP_HW_PE_OUT_DBUF_THRES (EIP_HW_PE_EIP96_BASE + 0xC00U)
#define EIP_HW_PE_OUT_TBUF_THRES (EIP_HW_PE_EIP96_BASE + 0xD00U)
#define EIP_HW_PE_PSE_TOKEN_CTRL_STAT (EIP_HW_PE_EIP96_BASE + 0xE00U)
#define EIP_HW_PE_PSE_FUNC_EN (EIP_HW_PE_EIP96_BASE + 0xE04U)
#define EIP_HW_PE_PSE_CONTEXT_CTRL (EIP_HW_PE_EIP96_BASE + 0xE08U)
#define EIP_HW_PE_PSE_CONTEXT_STAT (EIP_HW_PE_EIP96_BASE + 0xE0CU)
#define EIP_HW_PE_PSE_NXT_ACT_TOKEN_VAL (EIP_HW_PE_EIP96_BASE + 0xE10U)
#define EIP_HW_PE_PSE_OUT_TRANS_CTRL_STAT (EIP_HW_PE_EIP96_BASE + 0xE18U)
#define EIP_HW_PE_PSE_OUT_BUF_CTRL (EIP_HW_PE_EIP96_BASE + 0xE1CU)
#define EIP_HW_PE_PSE_OPTIONS (EIP_HW_PE_EIP96_BASE + 0xEF8U)
#define EIP_HW_PE_PSE_VERSION (EIP_HW_PE_EIP96_BASE + 0xEFCU)
#define EIP_HW_PE_PE_INFLIGHT (EIP_HW_PE_EIP96_BASE + 0xFF0U)
#define EIP_HW_PE_PE_DEBUG (EIP_HW_PE_EIP96_BASE + 0xFF4U)
#define EIP_HW_PE_PE_OPTIONS (EIP_HW_PE_EIP96_BASE + 0xFF8U)
#define EIP_HW_PE_PE_VERSION (EIP_HW_PE_EIP96_BASE + 0xFFCU)

/*
 * Simple Transform Record Cache registers
 */
#define EIP_HW_STRC_BASE 0x74000U
#define EIP_HW_STRC_RECORD_STATE(i) (EIP_HW_STRC_BASE + (16U * i) + 0x0)
#define EIP_HW_STRC_RECORD_ADDR_LO(i) (EIP_HW_STRC_BASE + (16U * i) + 0x8U)
#define EIP_HW_STRC_RECORD_ADDR_HI(i) (EIP_HW_STRC_BASE + (16U * i) + 0xCU)
#define EIP_HW_STRC_CONFIG (EIP_HW_STRC_BASE + 0x3F0U)
#define EIP_HW_STRC_ERROR_STATUS (EIP_HW_STRC_BASE + 0x3F4U)
#define EIP_HW_STRC_OPTIONS (EIP_HW_STRC_BASE + 0x3F8U)
#define EIP_HW_STRC_VERSION (EIP_HW_STRC_BASE + 0x3FCU)

/*
 * Flow lookup engine registers
 */
#define EIP_HW_FLUE_BASE 0x76000U
#define EIP_HW_FLUE_CONFIG(i) (EIP_HW_FLUE_BASE + (i * 0x20U) + 0x10U)
#define EIP_HW_FLUE_IFC_LUT0 (EIP_HW_FLUE_BASE + 0x820U)
#define EIP_HW_FLUE_IFC_LUT1 (EIP_HW_FLUE_BASE + 0x824U)
#define EIP_HW_FLUE_IFC_LUT2 (EIP_HW_FLUE_BASE + 0x828U)

/*
 * Classification registers
 */
#define EIP_HW_CS_RAM_CTRL 0x77FF0U
#define EIP_HW_CS_OPTIONS 0x77FF8U
#define EIP_HW_CS_VERSION 0x77FFCU

/*
 * Debug and clock registers
 */
#define EIP_HW_DEBUG_BASE 0x7F000U
#define EIP_HW_CLK_STATE (EIP_HW_DEBUG_BASE + 0xFE4U)
#define EIP_HW_FORCE_CLOCK_ON (EIP_HW_DEBUG_BASE + 0xFE8U)
#define EIP_HW_FORCE_CLOCK_OFF (EIP_HW_DEBUG_BASE + 0xFECU)
#define EIP_HW_MST_CTRL (EIP_HW_DEBUG_BASE + 0XFF4U)
#define EIP_HW_OPTION (EIP_HW_DEBUG_BASE + 0xFF8U)
#define EIP_HW_VERSION (EIP_HW_DEBUG_BASE + 0xFFCU)
#define EIP_HW_CDR_CLK_ON (1U << 3)

/*
 * Reset
 */
#define EIP_HW_HIA_CDR_PREP_COUNT_RST 0x80000000U
#define EIP_HW_HIA_CDR_PROC_COUNT_RST 0x80000000U
#define EIP_HW_HIA_CDR_DMA_CFG_RST 0x1000000U
#define EIP_HW_HIA_RDR_PREP_COUNT_RST 0x80000000U
#define EIP_HW_HIA_RDR_PROC_COUNT_RST 0x80000000U
#define EIP_HW_HIA_RDR_DMA_CFG_RST 0x1800000U

/*
 * Context Field cipher algorithm
 */
#define EIP_HW_CTX_ALGO_DES 0x0
#define EIP_HW_CTX_ALGO_3DES (0x2U << 17)
#define EIP_HW_CTX_ALGO_AES128 (0x5U << 17)
#define EIP_HW_CTX_ALGO_AES192 (0x6U << 17)
#define EIP_HW_CTX_ALGO_AES256 (0x7U << 17)

/*
 * Context Field auth algorithm
 */
#define EIP_HW_CTX_ALGO_MD5 0x0
#define EIP_HW_CTX_ALGO_SHA1 (0x2U << 23)
#define EIP_HW_CTX_ALGO_SHA256 (0x3U << 23)
#define EIP_HW_CTX_ALGO_SHA224 (0x4U << 23)
#define EIP_HW_CTX_ALGO_SHA512 (0x5U << 23)
#define EIP_HW_CTX_ALGO_SHA384 (0x6U << 23)
#define EIP_HW_CTX_ALGO_GHASH  (0x4U << 23)

/*
 * Context Field digest type
 */
#define EIP_HW_CTX_AUTH_MODE_HASH (0x1U << 21)
#define EIP_HW_CTX_AUTH_MODE_GMAC (0x2U << 21)
#define EIP_HW_CTX_AUTH_MODE_HMAC (0x3U << 21)

/*
 * Context field enable key
 */
#define EIP_HW_CTX_WITH_KEY (0x1U << 16)

/*
 * Number of context control words
 */
#define EIP_HW_MAX_CTRL 2U

/*
 * Context Mode
 */
#define EIP_HW_CTX_CIPHER_MODE_ECB 0x0
#define EIP_HW_CTX_CIPHER_MODE_CBC 0x1U
#define EIP_HW_CTX_CIPHER_MODE_CTR 0x2U
#define EIP_HW_CTX_CIPHER_MODE_GCM ((0x1U << 17) | 0x2)

#define EIP_HW_CTX_TYPE_SMALL 0x2U
#define EIP_HW_CTX_TYPE_LARGE 0x3U
#define EIP_HW_CTX_SIZE_LARGE_WORDS (62U)
#define EIP_HW_CTX_SIZE_LARGE_BYTES (EIP_HW_CTX_SIZE_LARGE_WORDS * sizeof(uint32_t))
#define EIP_HW_CTX_SIZE_SMALL_WORDS (46U)
#define EIP_HW_CTX_SIZE_SMALL_BYTES (EIP_HW_CTX_SIZE_SMALL_WORDS * sizeof(uint32_t))

/*
 * Pad key size
 */
#define EIP_HW_PAD_KEYSZ_MD5 16U
#define EIP_HW_PAD_KEYSZ_SHA1 20U
#define EIP_HW_PAD_KEYSZ_SHA224 32U
#define EIP_HW_PAD_KEYSZ_SHA256 32U
#define EIP_HW_PAD_KEYSZ_SHA384 64U
#define EIP_HW_PAD_KEYSZ_SHA512 64U

#define EIP_HW_CTX_SIZE_4WORDS (0x4U << 8)
#define EIP_HW_CTX_SIZE_6WORDS (0x6U << 8)
#define EIP_HW_CTX_SIZE_7WORDS (0x7U << 8)
#define EIP_HW_CTX_SIZE_8WORDS (0x8U << 8)
#define EIP_HW_CTX_SIZE_9WORDS (0x9U << 8)
#define EIP_HW_CTX_SIZE_10WORDS (0xAU << 8)
#define EIP_HW_CTX_SIZE_11WORDS (0xBU << 8)
#define EIP_HW_CTX_SIZE_12WORDS (0xCU << 8)
#define EIP_HW_CTX_SIZE_14WORDS (0xEU << 8)
#define EIP_HW_CTX_SIZE_16WORDS (0x10U << 8)
#define EIP_HW_CTX_SIZE_18WORDS (0x12U << 8)
#define EIP_HW_CTX_SIZE_19WORDS (0x13U << 8)
#define EIP_HW_CTX_SIZE_20WORDS (0x14U << 8)
#define EIP_HW_CTX_SIZE_22WORDS (0x16U << 8)
#define EIP_HW_CTX_SIZE_24WORDS (0x18U << 8)
#define EIP_HW_CTX_SIZE_25WORDS (0x19U << 8)
#define EIP_HW_CTX_SIZE_26WORDS (0x1AU << 8)
#define EIP_HW_CTX_SIZE_32WORDS (0x20U << 8)
#define EIP_HW_CTX_SIZE_36WORDS (0x24U << 8)
#define EIP_HW_CTX_SIZE_38WORDS (0x26U << 8)
#define EIP_HW_CTX_SIZE_40WORDS (0x28U << 8)

/*
 * Control word fields.
 */
#define EIP_HW_CTRL_WORDS 2U
#define EIP_HW_CTRL_LEN(x) (((x) & 0x3FU) << 8)
#define EIP_HW_CTRL_LEN_MASK (0x3FU << 8)
#define EIP_HW_CTRL_OPTION_UNFINISH_HASH_TO_CTX (0x5U << 4)
#define EIP_HW_CTRL_HASH_OP(x) (((x) & 0xFU) << 23)
#define EIP_HW_CTRL_HASH_OP_MASK (0xFU << 23)
#define EIP_HW_CTRL_DIGEST_TYPE (0x3U << 21)

#define EIP_HW_CTRL_ALGO_AES128 (0xBU << 16)
#define EIP_HW_CTRL_ALGO_AES192 (0xDU << 16)
#define EIP_HW_CTRL_ALGO_AES256 (0xFU << 16)
#define EIP_HW_CTRL_ALGO_MD5 0x0
#define EIP_HW_CTRL_ALGO_SHA1 0x2U
#define EIP_HW_CTRL_ALGO_SHA224 0x4U
#define EIP_HW_CTRL_ALGO_SHA256 0x3U
#define EIP_HW_CTRL_ALGO_SHA384 0x6U
#define EIP_HW_CTRL_ALGO_SHA512 0x5U

#define EIP_HW_CTRL_TOP_HMAC_ADD (0x2U << 0)

/*
 * HW token macros
 */
#define EIP_HW_TOKEN_HDR_OUTBOUND 0x0 		/* outbound packet */
#define EIP_HW_TOKEN_HDR_APP_ID 0x00020000U 		/* Application ID is valid */
#define EIP_HW_TOKEN_HDR_CTX_PTR 0x00040000U 	/* context address 64-bit */
#define EIP_HW_TOKEN_HDR_REUSE_CTX 0x00200000U	/* Reuse context */
#define EIP_HW_TOKEN_HDR_INBOUND 0x01800000U 	/* inbound packet */
#define EIP_HW_TOKEN_HDR_BASIC 0xC0000000U 		/* token is basic crypto */
#define EIP_HW_TOKEN_HDR_CTX_CTRL 0x02000000U 	/* context control present in token. */
#define EIP_HW_TOKEN_HDR_IV8 0x18000000U 		/* 8 bytes of IV. */
#define EIP_HW_TOKEN_HDR_IV16 0x1C000000U 		/* 16 bytes of IV. */
#define EIP_HW_TOKEN_HDR_IV_PRNG 0x04000000U 		/* IV source from PRNG */

#define EIP_HW_TOKEN_HDR_CMN (EIP_HW_TOKEN_HDR_BASIC | EIP_HW_TOKEN_HDR_CTX_PTR | EIP_HW_TOKEN_HDR_APP_ID)
#define EIP_HW_TOKEN_HDR_DIGEST (EIP_HW_TOKEN_HDR_CMN | EIP_HW_TOKEN_HDR_CTX_CTRL)
#define EIP_HW_TOKEN_HDR_EXTENDED EIP_HW_TOKEN_HDR_CMN
#define EIP_HW_TOKEN_HDR_CRYPTO_CMN (EIP_HW_TOKEN_HDR_APP_ID | EIP_HW_TOKEN_HDR_CTX_PTR \
				| EIP_HW_TOKEN_HDR_REUSE_CTX | EIP_HW_TOKEN_HDR_CTX_CTRL \
				| EIP_HW_TOKEN_HDR_BASIC)
#define EIP_HW_TOKEN_HDR_CRYPTO_IV8 (EIP_HW_TOKEN_HDR_CRYPTO_CMN | EIP_HW_TOKEN_HDR_IV8)
#define EIP_HW_TOKEN_HDR_CRYPTO_IV16 (EIP_HW_TOKEN_HDR_CRYPTO_CMN | EIP_HW_TOKEN_HDR_IV16)

#define EIP_HW_TOKEN_HDR_IPSEC_CMN (EIP_HW_TOKEN_HDR_APP_ID | EIP_HW_TOKEN_HDR_CTX_PTR \
				| EIP_HW_TOKEN_HDR_REUSE_CTX | EIP_HW_TOKEN_HDR_BASIC)
#define EIP_HW_TOKEN_HDR_IPSEC_IV16 (EIP_HW_TOKEN_HDR_IPSEC_CMN | EIP_HW_TOKEN_HDR_IV16)

#define EIP_HW_TOKEN_HDR_DTLS_CMN (EIP_HW_TOKEN_HDR_APP_ID | EIP_HW_TOKEN_HDR_CTX_PTR \
				| EIP_HW_TOKEN_HDR_REUSE_CTX | EIP_HW_TOKEN_HDR_BASIC)
#define EIP_HW_TOKEN_HDR_DTLS_IV16 (EIP_HW_TOKEN_HDR_DTLS_CMN | EIP_HW_TOKEN_HDR_IV16)

/*
 * Command descriptor ring related macros
 */
#define EIP_HW_CDR_CD_SIZE 0x10U
#define EIP_HW_CDR_CD_OFFSET (0x10U << 16)
#define EIP_HW_CDR_ACDP_PRESENT (0x1U << 30)
#define EIP_HW_CDR_MODE_64BIT (0x1U << 31)
#define EIP_HW_CDR_CD_FETCH_SIZE 0x10U
#define EIP_HW_CDR_CD_FETCH_THR (0x10U << 16)

#define EIP_HW_CDR_DESC_SIZE_VAL (EIP_HW_CDR_MODE_64BIT | EIP_HW_CDR_ACDP_PRESENT | \
			EIP_HW_CDR_CD_OFFSET | EIP_HW_CDR_CD_SIZE)
#define EIP_HW_CDR_CFG_VAL (EIP_HW_CDR_CD_FETCH_THR | EIP_HW_CDR_CD_FETCH_SIZE)
#define EIP_HW_CDR_HIA_THR_VAL (0x10U | (1U << 22)) /* proc_mode & thresh value */
#define EIP_HW_CDR_DMA_CFG_VAL 0x22000000U
#define EIP_HW_HIA_AIC_R_ENABLE_CTRL_CDR(i) (1U << (0 + (2U * (i))))

/*
 * command descriptor fields
 */
#define EIP_HW_CMD_FRAG_LEN(len) ((len) & 0xFFFFU)
#define EIP_HW_CMD_FLAGS(first, last) (((first) << 23) | ((last) << 22))
#define EIP_HW_CMD_FLAGS_FIRST (0x1U << 23)
#define EIP_HW_CMD_FLAGS_LAST (0x1U << 22)
#define EIP_HW_CMD_FLAGS_MASK (EIP_HW_CMD_FLAGS_FIRST | EIP_HW_CMD_FLAGS_LAST)
#define EIP_HW_CMD_TOKEN_WORDS(len) ((len) << 24)
#define EIP_HW_CMD_APP_ID(res) (((res) << 9) & 0xFE00U)
#define EIP_HW_CMD_DATA_LEN(len) ((len) & 0xFFFFU)
#define EIP_HW_CMD_OFFSET(x) (((x) & 0xFFU) << 8)

/*
 * Result descriptor ring related macros
 */
#define EIP_HW_RDR_CD_SIZE 0x10U
#define EIP_HW_RDR_CD_OFFSET (0x10U << 16)
#define EIP_HW_RDR_MODE_64BIT (0x1U << 31)
#define EIP_HW_RDR_CD_FETCH_SIZE 0x10U
#define EIP_HW_RDR_CD_FETCH_THR (0x10U << 16)
#define EIP_HW_RDR_OFFLOAD_IRQ_EN (0x1U << 25)

#define EIP_HW_RDR_DESC_SIZE_VAL (EIP_HW_RDR_MODE_64BIT | EIP_HW_RDR_CD_OFFSET | EIP_HW_RDR_CD_SIZE)

/*
 * TODO: this need to be revisited during performance analysis.
 */
#define EIP_HW_RDR_CFG_VAL (EIP_HW_RDR_CD_FETCH_THR | EIP_HW_RDR_CD_FETCH_SIZE)
#define EIP_HW_RDR_HIA_THR_VAL 0x10U
#define EIP_HW_RDR_DMA_CFG_VAL 0x22000000U
#define EIP_HW_HIA_AIC_R_ENABLE_CTRL_RDR(i) (1U << (1 + (2 * (i))))

/*
 * Result descriptor field.
 */
#define EIP_HW_RES_DATA_LEN(len) ((len) & 0xFFFFU)
#define EIP_HW_RES_FRAG_LEN(len) ((len) & 0xFFFFU)
#define EIP_HW_RES_FLAGS(first, last) (((first) << 23) | ((last) << 22))
#define EIP_HW_RES_FLAGS_FIRST (0x1U << 23)
#define EIP_HW_RES_FLAGS_LAST (0x1U << 22)
#define EIP_HW_RES_FLAGS_MASK (EIP_HW_RES_FLAGS_FIRST | EIP_HW_RES_FLAGS_LAST)
#define EIP_HW_RES_FLAGS_LINEAR (EIP_HW_RES_FLAGS_MASK)
#define EIP_HW_RES_APP_ID(res_word) (((res_word) >> 9) & 0x7FU)
#define EIP_HW_RES_ERROR_MASK 0xFFU /* Detailed error code */
#define EIP_HW_RES_ERROR_CLE_MASK 0x1FU
#define EIP_HW_RES_ERROR_CLE_MAX (EIP_HW_RES_ERROR_CLE_MASK + 1)
#define EIP_HW_RES_ERROR(res) (((res) >> 17) & EIP_HW_RES_ERROR_MASK)
#define EIP_HW_RES_ERROR_CLE(res) (((res) >> 16) & EIP_HW_RES_ERROR_CLE_MASK)
#define EIP_HW_RES_NEXT_HDR(res) ((res) & 0xFFU)
#define EIP_HW_RES_TR_ADDR(res) ((res) & 0xFFFFFFFCU)
#define EIP_HW_RES_ERR_NAT_T 163U
#define EIP_HW_RES_ERR_IKE 164U
#define EIP_HW_RES_ERR_FLOW_FAIL 188U
#define EIP_HW_RES_BYPASS_SRC_PORT(by) ((by) & 0xFFFFU)

/*
 * HW specific service number.
 */
#define EIP_HW_HWSERVICE_IPSEC 0x3U
#define EIP_HW_HWSERVICE_CRYPTO 0x4U
#define EIP_HW_CMD_HWSERVICE_INV_TRC (0x6U << 24)
#define EIP_HW_CMD_HWSERVICE_LIP (EIP_HW_HWSERVICE_IPSEC << 24)
#define EIP_HW_CMD_HWSERVICE_LAC (EIP_HW_HWSERVICE_CRYPTO << 24)

/*
 * Prepared and processed register specific macro
 */
#define EIP_HW_PREP_CNT_DESC_SZ(x) ((x) << 2)
#define EIP_HW_PROC_CNT_DESC_SZ(x) ((x) << 2)
#define EIP_HW_PROC_DESC_SZ(x) (((x) & 0xFFFFFFU) >> 2)
#define EIP_HW_PREP_DESC_SZ(x) (((x) & 0xFFFFFFU) >> 2)
#define EIP_HW_RING_DESC_SZ(x) ((x) << 2)

/*
 * error macros.
 */
#define EIP_HW_RES_ERROR_E0_NUM 0
#define EIP_HW_RES_ERROR_E1_NUM 1U
#define EIP_HW_RES_ERROR_E2_NUM 2U
#define EIP_HW_RES_ERROR_E3_NUM 3U
#define EIP_HW_RES_ERROR_E4_NUM 4U
#define EIP_HW_RES_ERROR_E5_NUM 5U
#define EIP_HW_RES_ERROR_E6_NUM 6U
#define EIP_HW_RES_ERROR_E7_NUM 7U
#define EIP_HW_RES_ERROR_E8_NUM 8U
#define EIP_HW_RES_ERROR_E9_NUM 9U
#define EIP_HW_RES_ERROR_E10_NUM 10U
#define EIP_HW_RES_ERROR_E11_NUM 11U
#define EIP_HW_RES_ERROR_E12_NUM 12U
#define EIP_HW_RES_ERROR_E13_NUM 13U
#define EIP_HW_RES_ERROR_E14_NUM 14U
#define EIP_HW_RES_ERROR_MAX 15U

/*
 * eip_hw_result_cle_error
 *	result classification engine errors(cle)
 */
enum eip_hw_result_cle_error {
	EIP_HW_RES_CLE_ERROR_E0 = 0x0,	/* No error */
	EIP_HW_RES_CLE_ERROR_E1 = 0x1,	/* Input token type or transformation is not supported or not found */
	EIP_HW_RES_CLE_ERROR_E2 = 0x2,	/* Input token type is supported but transformation is not supported */
	EIP_HW_RES_CLE_ERROR_E3 = 0x3,	/* Packet did not pass sanity checks during header inspection */
	EIP_HW_RES_CLE_ERROR_E5 = 0x5,	/* External DMA error during transform record fetching */
	EIP_HW_RES_CLE_ERROR_E6 = 0x6,	/* Transform record cache error. New record cannot be fetched */
	EIP_HW_RES_CLE_ERROR_E7 = 0x7,	/* External DMA error during flow record fetching */
	EIP_HW_RES_CLE_ERROR_E8 = 0x8,	/* Transform record cache error */
	EIP_HW_RES_CLE_ERROR_E9 = 0x9,	/* External DMA error on fetching LookAside token instructions */
	EIP_HW_RES_CLE_ERROR_E10 = 0xA,	/* Lookup table disabled error */
	EIP_HW_RES_CLE_ERROR_E12 = 0xC,	/* DMA error during flow lookup */
	EIP_HW_RES_CLE_ERROR_E13 = 0xD,	/* ECC error in Input Packet Buffer Manager */
	EIP_HW_RES_CLE_ERROR_E14 = 0xE,	/* ECC error in Transform Record Cache */
	EIP_HW_RES_CLE_ERROR_E15 = 0xF,	/* ECC error in Flow Record Cache */
	EIP_HW_RES_CLE_ERROR_E16 = 0x10,	/* ECC error in Intermediate Packet Buffer Manager */
	EIP_HW_RES_CLE_ERROR_E18 = 0x12,	/* Reserved; needs to be ignored in NULL encryption mode */
	EIP_HW_RES_CLE_ERROR_E20 = 0x14,	/* ECN error: Outer ECN = b01, Inner ECN = b00, Flag bit = 0 */
	EIP_HW_RES_CLE_ERROR_E21 = 0x15,	/* ECN error: Outer ECN = b01, Inner ECN = b10, Flag bit = 1 */
	EIP_HW_RES_CLE_ERROR_E22 = 0x16,	/* ECN error: Outer ECN = b10, Inner ECN = b00, Flag bit = 2 */
	EIP_HW_RES_CLE_ERROR_E23 = 0x17,	/* ECN error: Outer ECN = b10, Inner ECN = b11, Flag bit = 3 */
	EIP_HW_RES_CLE_ERROR_E24 = 0x18,	/* ECN error: Outer ECN = b11, Inner ECN = b00, Flag bit = 4 */
	EIP_HW_RES_CLE_ERROR_E25 = 0x19,	/* OPUE Parsing output error */
};

int eip_hw_init(struct platform_device *pdev);
void eip_hw_deinit(struct platform_device *pdev);

#endif /* __EIP_HW_H */
