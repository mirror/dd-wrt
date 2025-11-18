/*******************************************************************************
Copyright (C) 2015 Annapurna Labs Ltd.

This file may be licensed under the terms of the Annapurna Labs Commercial
License Agreement.

Alternatively, this file can be distributed under the terms of the GNU General
Public License V2 as published by the Free Software Foundation and can be
found at http://www.gnu.org/licenses/gpl-2.0.html

Alternatively, redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following conditions are
met:

    *     Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

    *     Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the
distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/


#ifndef __AL_PLL_REG_H
#define __AL_PLL_REG_H

#ifdef __cplusplus
extern "C" {
#endif
/*
* Unit Registers
*/

#define AL_PLL_NUM_CHNLS		16
#define AL_PLL_NUM_CHNL_PAIRS		((AL_PLL_NUM_CHNLS) / 2)

struct al_pll_regs {
	uint32_t setup_0;				/* 0 - 0x00 */
	uint32_t setup_1;				/* 1 - 0x04 */
	uint32_t flow_counters_0;			/* 2 - 0x08 */
	uint32_t flow_counters_1;			/* 3 - 0x0c */
	uint32_t al_tst_mode_ctl;			/* 4 - 0x10 */
	uint32_t al_tst_mode_res;			/* 5 - 0x14 */
	uint32_t tci_tst_mode_spcl_ctl;			/* 6 - 0x18 */
	uint32_t status;				/* 7 - 0x1c */
	uint32_t clk_div_setup[AL_PLL_NUM_CHNL_PAIRS];	/* 8-15 - 0x20-0x3c */
};

/*
* Registers Fields
*/

/**** setup_0 register ****/
/* PLL NF minus 1 */
#define AL_PLL_SETUP_0_NF_MASK			AL_FIELD_MASK(12, 0)
#define AL_PLL_SETUP_0_NF_SHIFT			0

/* PLL NR minus 1 */
#define AL_PLL_SETUP_0_NR_MASK			AL_FIELD_MASK(21, 16)
#define AL_PLL_SETUP_0_NR_SHIFT			16

/* PLL OD minus 1 */
#define AL_PLL_SETUP_0_OD_MASK			AL_FIELD_MASK(27, 24)
#define AL_PLL_SETUP_0_OD_SHIFT			24

/* Relock request (auto clears) */
#define AL_PLL_SETUP_0_RELOCK_EN		AL_BIT(31)

/**** setup_1 register ****/
/* PLL BWADJ minus 1 */
#define AL_PLL_SETUP_1_BWADJ_MASK		AL_FIELD_MASK(11, 0)
#define AL_PLL_SETUP_1_BWADJ_SHIFT		0

/* Core reset extend */
#define AL_PLL_SETUP_1_CRE_MASK			AL_FIELD_MASK(14, 12)
#define AL_PLL_SETUP_1_CRE_SHIFT		12

/* Lock use NR */
#define AL_PLL_SETUP_1_LCK_USE_NR		AL_BIT(15)

/* Lock count max */
#define AL_PLL_SETUP_1_LCK_CNT_MAX_MASK		AL_FIELD_MASK(31, 16)
#define AL_PLL_SETUP_1_LCK_CNT_MAX_SHIFT	16

/**** flow_counters_0 register ****/
/* PLL reset and bandgap-power-down ref clk count period */
#define AL_PLL_FLOW_CTR_0_PLL_RST_CNT_MASK	AL_FIELD_MASK(15, 0)
#define AL_PLL_FLOW_CTR_0_PLL_RST_CNT_SHIFT	0

/* Dividers settle ref clk count period */
#define AL_PLL_FLOW_CTR_0_DIV_SETTLE_CNT_MASK	AL_FIELD_MASK(31, 16)
#define AL_PLL_FLOW_CTR_0_DIV_SETTLE_CNT_SHIFT	16

/**** flow_counters_1 register ****/
/* Dividers set/sample ref clk count period */
#define AL_PLL_FLOW_CTR_1_DIV_SET_CNT_MASK	AL_FIELD_MASK(15, 0)
#define AL_PLL_FLOW_CTR_1_DIV_SET_CNT_SHIFT	0

/* Dividers keep towards re-lock ref clk count */
#define AL_PLL_FLOW_CTR_1_DIV_KEEP_CNT_MASK	AL_FIELD_MASK(31, 16)
#define AL_PLL_FLOW_CTR_1_DIV_KEEP_CNT_SHIFT	16

/**** al_tst_mode_ctl register ****/
/* Test count period [ref clks] */
#define AL_PLL_AL_TST_CTL_PERIOD_MASK		AL_FIELD_MASK(23, 0)
#define AL_PLL_AL_TST_CTL_PERIOD_SHIFT		0

/* PLL clock test ratio [power of 2] */
#define AL_PLL_AL_TST_CTL_RATIO_MASK		AL_FIELD_MASK(26, 24)
#define AL_PLL_AL_TST_CTL_RATIO_SHIFT		24

/* Enable the test clock divider operation */
#define AL_PLL_AL_TST_CTL_CLK_DIV_EN		AL_BIT(27)

/* Enable the test (trigger) */
#define AL_PLL_AL_TST_CTL_TRIG			AL_BIT(31)

/**** al_tst_mode_res register ****/
/* Test count as measured in PLL-ratioed clock */
#define AL_PLL_AL_TST_RES_CNT_RES_MASK		AL_FIELD_MASK(31, 0)
#define AL_PLL_AL_TST_RES_CNT_RES_SHIFT		0

/**** tci_tst_mode_spcl_ctl register ****/
/* PLL bypass */
#define AL_PLL_TCI_TST_SPCL_CTL_PLL_BYPASS	AL_BIT(31)

/**** status register ****/
/* PLL ctrl state machine */
#define AL_PLL_STATUS_SM_MASK				AL_FIELD_MASK(11, 8)
#define AL_PLL_STATUS_SM_SHIFT				8

#define AL_PLL_STATUS_SM_VAL_DIV_STABLE		6
#define AL_PLL_STATUS_SM_VAL_DIV_ACTIVE		7

/* ANPA test mode - count busy */
#define AL_PLL_STATUS_ATM_BUSY			AL_BIT(29)

/* Dividers locked */
#define AL_PLL_STATUS_DIV_LOCK			AL_BIT(30)

/* PLL locked */
#define AL_PLL_STATUS_PLL_LOCK			AL_BIT(31)

/**** clk_div_setup register ****/
/* Even channel params */
#define AL_PLL_CLK_DIV_SETUP_PARAMS_EVEN_MASK	AL_FIELD_MASK(15, 0)
#define AL_PLL_CLK_DIV_SETUP_PARAMS_EVEN_SHIFT	0

/* Odd channel params */
#define AL_PLL_CLK_DIV_SETUP_PARAMS_ODD_MASK	AL_FIELD_MASK(31, 16)
#define AL_PLL_CLK_DIV_SETUP_PARAMS_ODD_SHIFT	16

/**** Channel parameters ****/
/* Division value - 0 for disabling the channel */
#define AL_PLL_CHNL_PARAM_DIV_VAL_MASK		AL_FIELD_MASK(9, 0)
#define AL_PLL_CHNL_PARAM_DIV_VAL_SHIFT		0

/*  Division value addition of 0.5 */
#define AL_PLL_CHNL_PARAM_DIV_VAL_ADD_HALF	AL_BIT(11)

/*  Relock request */
#define AL_PLL_CHNL_PARAM_RELOCK		AL_BIT(12)

/*  Last in series */
#define AL_PLL_CHNL_PARAM_LAST_IN_SERIES	AL_BIT(13)

/*  Reset mask */
#define AL_PLL_CHNL_PARAM_RESET_MASK		AL_BIT(14)

/*  Reference clock bypass */
#define AL_PLL_CHNL_PARAM_REF_CLK_BYPASS	AL_BIT(15)

#ifdef __cplusplus
}
#endif

#endif /* __AL_PLL_REG_H */

