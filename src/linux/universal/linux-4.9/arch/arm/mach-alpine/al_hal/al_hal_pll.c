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

/**
 *  @{
 * @file   al_hal_pll.c
 *
 * @brief  PLL HAL driver
 *
 */

#include "al_hal_pll.h"
#include "al_hal_pll_regs.h"
#include "al_hal_pll_map.h"

#define AL_PLL_DIV_LOCK_START_DELAY 3

/**
 * Determine whether a PLL is locked
 */
static inline int al_pll_is_locked_s(
	struct al_pll_obj	*obj);

/**
 * Determine whether a PLL is bypassed
 */
static inline int al_pll_is_bypassed_s(
	struct al_pll_obj	*obj);

/**
 * Get current PLL frequency, according to its configuration and lock state
 */
static unsigned int al_pll_freq_get_s(
	struct al_pll_obj	*obj);

/**
 * Determine whether all PLL's dividers are locked
 */
static inline int al_pll_channel_div_are_all_locked_s(
	struct al_pll_obj	*obj);

/**
 * Get PLL specific channel divider parameters
 */
static inline uint16_t al_pll_channel_params_get_s(
	struct al_pll_obj	*obj,
	unsigned int		chan_idx);

/**
 * Set PLL specific channel divider parameters
 */
static inline void al_pll_channel_params_set_s(
	struct al_pll_obj	*obj,
	unsigned int		chan_idx,
	uint16_t		params);

/**
 * Determine whether a PLL test is done
 */
static inline int al_pll_test_is_done_s(
	struct al_pll_obj	*obj);

/******************************************************************************/
/******************************************************************************/
int al_pll_init(
	void __iomem			*regs_base,
	char				*name,
	enum al_pll_ref_clk_freq	ref_clk_freq,
	struct al_pll_obj		*obj)
{
	int status = 0;

	al_dbg(
		"%s(%p, %s, %d, %p)\n",
		__func__,
		regs_base,
		name,
		ref_clk_freq,
		obj);


	al_assert(regs_base);
	al_assert(name);
	al_assert(obj);

	obj->regs_base = ioremap(regs_base, 0x40);

	obj->name = name;

	switch (ref_clk_freq) {
	case AL_PLL_REF_CLK_FREQ_125_MHZ:
		obj->ref_clk_freq_val = 125000;
		obj->freq_map = (const void *)al_pll_freq_map_125;
		obj->freq_map_size = sizeof(al_pll_freq_map_125) /
			sizeof(al_pll_freq_map_125[0]);
		break;

	case AL_PLL_REF_CLK_FREQ_100_MHZ:
		obj->ref_clk_freq_val = 100000;
		obj->freq_map = (const void *)al_pll_freq_map_100;
		obj->freq_map_size = sizeof(al_pll_freq_map_100) /
			sizeof(al_pll_freq_map_100[0]);
		break;

	case AL_PLL_REF_CLK_FREQ_25_MHZ:
		obj->ref_clk_freq_val = 25000;
		obj->freq_map = (const void *)al_pll_freq_map_25;
		obj->freq_map_size = sizeof(al_pll_freq_map_25) /
			sizeof(al_pll_freq_map_25[0]);
		break;

	default:
		al_err(
			"%s: invalid ref clk freq enum (%d)!\n",
			__func__,
			ref_clk_freq);
		status = -EINVAL;
	}

	return status;
}

/******************************************************************************/
/******************************************************************************/
int al_pll_terminate(
	struct al_pll_obj	*obj)
{
	int status = 0;

	al_dbg("%s(%p)\n", __func__, obj);

	return status;
}

/******************************************************************************/
/******************************************************************************/
int al_pll_freq_get(
	struct al_pll_obj	*obj,
	enum al_pll_freq	*freq,
	unsigned int		*freq_val)
{
	int status = 0;

	int i;

	struct al_pll_freq_map_ent *ent;

	al_dbg(
		"%s(%p)\n",
		__func__,
		obj);

	al_assert(obj);

	ent = (struct al_pll_freq_map_ent *)obj->freq_map;

	*freq = AL_PLL_FREQ_NA;

	*freq_val = al_pll_freq_get_s(obj);

	for (i = 0; i < obj->freq_map_size; i++, ent++) {
		if (*freq_val == ent->freq_val) {
			*freq = ent->freq;
			break;
		}
	}

	al_dbg(
		"%s: return (%d, %u)\n",
		__func__,
		*freq,
		*freq_val);

	return status;
}

/******************************************************************************/
/******************************************************************************/
int al_pll_freq_set(
	struct al_pll_obj	*obj,
	enum al_pll_freq	freq,
	unsigned int		timeout)
{
	int status = 0;
	int i;
	struct al_pll_freq_map_ent *ent;
	struct al_pll_regs *regs;
	uint32_t setup_0;
	uint32_t setup_1;
	uint32_t tci_tst_mode_spcl_ctl;

	al_dbg(
		"%s(%p, %d, %u)\n",
		__func__,
		obj,
		freq,
		timeout);

	al_assert(obj);

	ent = (struct al_pll_freq_map_ent *)obj->freq_map;
	regs = (struct al_pll_regs *)obj->regs_base;

	for (i = 0; i < obj->freq_map_size; i++, ent++) {
		if (freq == ent->freq)
			break;
	}

	if (i == obj->freq_map_size) {
		al_err("%s: invalid freq (%d)\n", __func__, freq);
		status = -EINVAL;
		goto done;
	}

	tci_tst_mode_spcl_ctl = al_reg_read32(&regs->tci_tst_mode_spcl_ctl);
	tci_tst_mode_spcl_ctl &= ~AL_PLL_TCI_TST_SPCL_CTL_PLL_BYPASS;
	al_reg_write32(&regs->tci_tst_mode_spcl_ctl, tci_tst_mode_spcl_ctl);

	setup_1 = al_reg_read32(&regs->setup_1);

	setup_0 =
		(ent->nf << AL_PLL_SETUP_0_NF_SHIFT) |
		(ent->nr << AL_PLL_SETUP_0_NR_SHIFT) |
		(ent->od << AL_PLL_SETUP_0_OD_SHIFT);

	setup_1 =
		(setup_1 & ~AL_PLL_SETUP_1_BWADJ_MASK) |
		(ent->bwadj << AL_PLL_SETUP_1_BWADJ_SHIFT);

	al_reg_write32(&regs->setup_0, setup_0);
	al_reg_write32(&regs->setup_1, setup_1);
	setup_0 |= AL_PLL_SETUP_0_RELOCK_EN;
	al_reg_write32(&regs->setup_0, setup_0);

	while (!(al_pll_is_locked_s(obj)) && (timeout)) {
		al_udelay(1);
		timeout--;
	}

	if (!al_pll_is_locked_s(obj)) {
		al_err("%s: timed out!\n", __func__);
		status = -ETIMEDOUT;
	}

done:
	return status;
}

/******************************************************************************/
/******************************************************************************/
int al_pll_freq_switch_to_bypass(
	struct al_pll_obj	*obj,
	unsigned int		timeout)
{
	int status = 0;
	struct al_pll_regs *regs;
	uint32_t setup_0;
	uint32_t tci_tst_mode_spcl_ctl;

	al_dbg(
		"%s(%p, %u)\n",
		__func__,
		obj,
		timeout);

	al_assert(obj);

	regs = (struct al_pll_regs *)obj->regs_base;

	tci_tst_mode_spcl_ctl = al_reg_read32(&regs->tci_tst_mode_spcl_ctl);
	tci_tst_mode_spcl_ctl |= AL_PLL_TCI_TST_SPCL_CTL_PLL_BYPASS;
	al_reg_write32(&regs->tci_tst_mode_spcl_ctl, tci_tst_mode_spcl_ctl);

	setup_0 = al_reg_read32(&regs->setup_0);
	setup_0 |= AL_PLL_SETUP_0_RELOCK_EN;
	al_reg_write32(&regs->setup_0, setup_0);

	while (!(al_pll_is_locked_s(obj)) && (timeout)) {
		al_udelay(1);
		timeout--;
	}

	if (!al_pll_is_locked_s(obj)) {
		al_err("%s: timed out!\n", __func__);
		status = -ETIMEDOUT;
	}

	return status;
}

/******************************************************************************/
/******************************************************************************/
int al_pll_is_locked(
	struct al_pll_obj	*obj,
	int			*is_locked)
{
	int status = 0;

	al_dbg("%s(%p)\n", __func__, obj);

	al_assert(obj);

	*is_locked = al_pll_is_locked_s(obj);

	al_dbg(
		"%s: return (%d)\n",
		__func__,
		*is_locked);

	return status;
}

/******************************************************************************/
/******************************************************************************/
int al_pll_channel_freq_get(
	struct al_pll_obj	*obj,
	unsigned int		chan_idx,
	unsigned int		*freq)
{
	int status = 0;

	al_dbg("%s(%p, %d)\n", __func__, obj, chan_idx);

	al_assert(obj);
	al_assert(freq);
	al_assert(chan_idx < AL_PLL_NUM_CHNLS);

	*freq = 0;

	if (al_pll_channel_div_are_all_locked_s(obj)) {
		unsigned int pll_freq = al_pll_freq_get_s(obj);
		uint16_t chan_params =
			al_pll_channel_params_get_s(obj, chan_idx);
		uint16_t div_val =
			((chan_params & AL_PLL_CHNL_PARAM_DIV_VAL_MASK) >>
			 AL_PLL_CHNL_PARAM_DIV_VAL_SHIFT);
		uint16_t ref_clk_bypass =
			!!(chan_params & AL_PLL_CHNL_PARAM_REF_CLK_BYPASS);

		if ((!div_val) && ref_clk_bypass)
			*freq = pll_freq;
		else if ((!div_val) && (!ref_clk_bypass))
			*freq = 0;
		else if (div_val && (!ref_clk_bypass))
			*freq = pll_freq / (unsigned int)div_val;
		else { /* div_val && ref_clk_bypass */
			al_err(
				"%s: div_val && ref_clk_bypass!\n",
				__func__);
			status = -EIO;
		}
	}

	al_dbg(
		"%s: return (%u)\n",
		__func__,
		*freq);

	return status;
}

/******************************************************************************/
/******************************************************************************/
int al_pll_channel_div_set(
	struct al_pll_obj	*obj,
	unsigned int		chan_idx,
	unsigned int		divider,
	al_bool			divider_add_half,
	int			reset,
	int			apply,
	unsigned int		timeout)
{
	int status = 0;
	uint16_t chan_params;

	al_dbg(
		"%s(%p, %u, %u, %d, %d, %u)\n",
		__func__,
		obj,
		chan_idx,
		divider,
		reset,
		apply,
		timeout);

	al_assert(obj);
	al_assert(chan_idx < AL_PLL_NUM_CHNLS);

	if (divider & ~AL_PLL_CHNL_PARAM_DIV_VAL_MASK) {
		al_err(
			"%s: requested divider too big (%u > %lu)!\n",
			__func__,
			divider,
			AL_PLL_CHNL_PARAM_DIV_VAL_MASK >>
			AL_PLL_CHNL_PARAM_DIV_VAL_SHIFT);
		status = -EINVAL;
		goto done;
	}

	chan_params = 0;

	chan_params |= (divider << AL_PLL_CHNL_PARAM_DIV_VAL_SHIFT);

	if (divider_add_half)
		chan_params |= AL_PLL_CHNL_PARAM_DIV_VAL_ADD_HALF;

	if (!reset)
		chan_params |= AL_PLL_CHNL_PARAM_RESET_MASK;

	chan_params |= AL_PLL_CHNL_PARAM_RELOCK;

	al_pll_channel_params_set_s(obj, chan_idx, chan_params);

	if (apply) {
		/* Last in series must be written separately */
		chan_params |= AL_PLL_CHNL_PARAM_LAST_IN_SERIES;
		al_pll_channel_params_set_s(obj, chan_idx, chan_params);

		/*
		* Addressing RMN: 5593
		*
		* RMN description:
		* PLL divider relock status does not cover pre-relock count
		*
		* Software flow:
		* Add delay before checking the lock status
		*/
		al_udelay(AL_PLL_DIV_LOCK_START_DELAY);

		while ((!al_pll_channel_div_are_all_locked_s(obj)) && timeout) {
			al_udelay(1);
			timeout--;
		}

		if (!al_pll_channel_div_are_all_locked_s(obj)) {
			al_err("%s: timed out!\n", __func__);
			status = -ETIMEDOUT;
		}
	}

done:
	return status;
}

/******************************************************************************/
/******************************************************************************/
int al_pll_channel_switch_to_bypass(
	struct al_pll_obj	*obj,
	unsigned int		chan_idx,
	int			reset,
	int			apply,
	unsigned int		timeout)
{
	int status = 0;
	uint16_t chan_params;

	al_dbg(
		"%s(%p, %u, %d, %d, %u)\n",
		__func__,
		obj,
		chan_idx,
		reset,
		apply,
		timeout);

	al_assert(obj);
	al_assert(chan_idx < AL_PLL_NUM_CHNLS);

	chan_params = 0;

	chan_params |= AL_PLL_CHNL_PARAM_REF_CLK_BYPASS;

	if (!reset)
		chan_params |= AL_PLL_CHNL_PARAM_RESET_MASK;

	chan_params |= AL_PLL_CHNL_PARAM_RELOCK;

	al_pll_channel_params_set_s(obj, chan_idx, chan_params);

	if (apply) {
		/* Last in series must be written separately */
		chan_params |= AL_PLL_CHNL_PARAM_LAST_IN_SERIES;
		al_pll_channel_params_set_s(obj, chan_idx, chan_params);

		while ((!al_pll_channel_div_are_all_locked_s(obj)) && timeout) {
			al_udelay(1);
			timeout--;
		}

		if (!al_pll_channel_div_are_all_locked_s(obj)) {
			al_err("%s: timed out!\n", __func__);
			status = -ETIMEDOUT;
		}
	}

	return status;
}

/******************************************************************************/
/******************************************************************************/
int al_pll_channel_div_are_all_locked(
	struct al_pll_obj	*obj,
	int			*are_locked)
{
	int status = 0;

	al_dbg(
		"%s(%p)\n",
		__func__,
		obj);

	al_assert(obj);

	*are_locked = al_pll_channel_div_are_all_locked_s(obj);

	al_dbg(
		"%s: return (%d)\n",
		__func__,
		*are_locked);

	return status;
}

/******************************************************************************/
/******************************************************************************/
int al_pll_test(
	struct al_pll_obj	*obj,
	unsigned int		ref_clk_num_cycles,
	unsigned int		ref_clk_ratio,
	int			*test_passed,
	unsigned int		*res_pll_num_cycles,
	unsigned int		timeout)
{
	int status = 0;

	struct al_pll_regs __iomem *regs;
	unsigned int pll_freq;
	unsigned int pll_num_cycles_expected;
	uint32_t tst_ctl;

	al_dbg(
		"%s(%p, %u, %u, %u)\n",
		__func__,
		obj,
		ref_clk_num_cycles,
		ref_clk_ratio,
		timeout);

	al_assert(obj);
	al_assert(!(ref_clk_num_cycles & ~(AL_PLL_AL_TST_CTL_PERIOD_MASK >>
					AL_PLL_AL_TST_CTL_PERIOD_SHIFT)));
	al_assert(!(ref_clk_ratio & ~(AL_PLL_AL_TST_CTL_RATIO_MASK >>
					AL_PLL_AL_TST_CTL_RATIO_SHIFT)));

	regs = (struct al_pll_regs __iomem *)obj->regs_base;

	tst_ctl =
		(ref_clk_num_cycles << AL_PLL_AL_TST_CTL_PERIOD_SHIFT) |
		(ref_clk_ratio << AL_PLL_AL_TST_CTL_RATIO_SHIFT) |
		AL_PLL_AL_TST_CTL_CLK_DIV_EN;

	al_reg_write32(&regs->al_tst_mode_ctl, tst_ctl);

	tst_ctl |= AL_PLL_AL_TST_CTL_TRIG;

	al_reg_write32(&regs->al_tst_mode_ctl, tst_ctl);

	while (!(al_pll_test_is_done_s(obj)) && (timeout)) {
		al_udelay(1);
		timeout--;
	}

	if (!al_pll_test_is_done_s(obj)) {
		al_err("%s: timed out!\n", __func__);
		status = -ETIMEDOUT;
		goto done;
	}

	*res_pll_num_cycles =
		(al_reg_read32(&regs->al_tst_mode_res) &
		 AL_PLL_AL_TST_RES_CNT_RES_MASK) >>
		AL_PLL_AL_TST_RES_CNT_RES_SHIFT;

	pll_freq = al_pll_freq_get_s(obj);

	pll_num_cycles_expected =
		(ref_clk_num_cycles * pll_freq) /
		((1 << ref_clk_ratio) * obj->ref_clk_freq_val);

	*test_passed =
		((*res_pll_num_cycles) == pll_num_cycles_expected);

	al_dbg(
		"%s: return (%d, %u)\n",
		__func__,
		*test_passed,
		*res_pll_num_cycles);

done:

	al_reg_write32(&regs->al_tst_mode_ctl, 0);

	return status;
}

/******************************************************************************/
/******************************************************************************/
static inline int al_pll_is_locked_s(
	struct al_pll_obj	*obj)
{
	struct al_pll_regs *regs =
		(struct al_pll_regs *)obj->regs_base;

	uint32_t status = al_reg_read32(&regs->status);

	return !!(status & AL_PLL_STATUS_PLL_LOCK);
}

/******************************************************************************/
/******************************************************************************/
static inline int al_pll_is_bypassed_s(
	struct al_pll_obj	*obj)
{
	struct al_pll_regs *regs =
		(struct al_pll_regs *)obj->regs_base;

	uint32_t reg_val = al_reg_read32(&regs->tci_tst_mode_spcl_ctl);

	return !!(reg_val & AL_PLL_TCI_TST_SPCL_CTL_PLL_BYPASS);
}

/******************************************************************************/
/******************************************************************************/
static unsigned int al_pll_freq_get_s(
	struct al_pll_obj	*obj)
{
	unsigned int freq_val = 0;

	if (al_pll_is_locked_s(obj)) {
		if (al_pll_is_bypassed_s(obj)) {
			freq_val = obj->ref_clk_freq_val;
		} else {
			struct al_pll_regs *regs =
				(struct al_pll_regs *)obj->regs_base;

			uint32_t setup_0 = al_reg_read32(&regs->setup_0);

			unsigned int nf =
				1 + ((setup_0 & AL_PLL_SETUP_0_NF_MASK) >>
				AL_PLL_SETUP_0_NF_SHIFT);
			unsigned int nr =
				1 + ((setup_0 & AL_PLL_SETUP_0_NR_MASK) >>
				AL_PLL_SETUP_0_NR_SHIFT);
			unsigned int od =
				1 + ((setup_0 & AL_PLL_SETUP_0_OD_MASK) >>
				AL_PLL_SETUP_0_OD_SHIFT);
			freq_val = (obj->ref_clk_freq_val * nf) / (nr * od);
		}
	}

	return freq_val;
}

/******************************************************************************/
/******************************************************************************/
static inline int al_pll_channel_div_are_all_locked_s(
	struct al_pll_obj	*obj)
{
	int pll_div_locked = 0;
	int pll_div_sm_current_state = 0;

	struct al_pll_regs *regs =
		(struct al_pll_regs *)obj->regs_base;

	uint32_t status = al_reg_read32(&regs->status);

	pll_div_sm_current_state = ((status & (AL_PLL_STATUS_SM_MASK)) >> AL_PLL_STATUS_SM_SHIFT);

	/*
	* Addressing RMN: 5633
	*
	* RMN description:
	* In the PLL status register there is a dedicated field for the divider state-machine
	* current state. Because of PLL/apb clock synchronization issues, the value in the status
	* register might not reflect the correct state-machine current state.
	*
	* Software flow:
	* For the divider relock completion checking, both state 6 (DIV_ACTIVE) and state 7
	* (DIV_STABLE) can serve as relock completion indication.
	*/
	pll_div_locked = (pll_div_sm_current_state == AL_PLL_STATUS_SM_VAL_DIV_STABLE) ||
				(pll_div_sm_current_state == AL_PLL_STATUS_SM_VAL_DIV_ACTIVE);

	return pll_div_locked;
}

/******************************************************************************/
/******************************************************************************/
static inline uint16_t al_pll_channel_params_get_s(
	struct al_pll_obj	*obj,
	unsigned int		chan_idx)
{
	unsigned int pair_idx = chan_idx / 2;
	struct al_pll_regs __iomem *regs =
		(struct al_pll_regs __iomem *)obj->regs_base;
	uint32_t pair = al_reg_read32(&regs->clk_div_setup[pair_idx]);
	uint16_t params;

	if (!(chan_idx & 1)) {
		params = (pair & AL_PLL_CLK_DIV_SETUP_PARAMS_EVEN_MASK) >>
			AL_PLL_CLK_DIV_SETUP_PARAMS_EVEN_SHIFT;
	} else {
		params = (pair & AL_PLL_CLK_DIV_SETUP_PARAMS_ODD_MASK) >>
			AL_PLL_CLK_DIV_SETUP_PARAMS_ODD_SHIFT;
	}

	return params;
}

/******************************************************************************/
/******************************************************************************/
static inline void al_pll_channel_params_set_s(
	struct al_pll_obj	*obj,
	unsigned int		chan_idx,
	uint16_t		params)
{
	unsigned int pair_idx = chan_idx / 2;
	struct al_pll_regs __iomem *regs =
		(struct al_pll_regs __iomem *)obj->regs_base;
	uint32_t pair = al_reg_read32(&regs->clk_div_setup[pair_idx]);

	if (!(chan_idx & 1)) {
		pair &= ~AL_PLL_CLK_DIV_SETUP_PARAMS_EVEN_MASK;
		pair |= ((uint32_t)params) <<
			AL_PLL_CLK_DIV_SETUP_PARAMS_EVEN_SHIFT;
	} else {
		pair &= ~AL_PLL_CLK_DIV_SETUP_PARAMS_ODD_MASK;
		pair |= ((uint32_t)params) <<
			AL_PLL_CLK_DIV_SETUP_PARAMS_ODD_SHIFT;
	}

	al_reg_write32(&regs->clk_div_setup[pair_idx], pair);
}

/******************************************************************************/
/******************************************************************************/
static inline int al_pll_test_is_done_s(
	struct al_pll_obj	*obj)
{
	struct al_pll_regs __iomem *regs =
		(struct al_pll_regs __iomem *)obj->regs_base;

	uint32_t status = al_reg_read32(&regs->status);

	return !(status & AL_PLL_STATUS_ATM_BUSY);
}

/** @} end of PLL group */
