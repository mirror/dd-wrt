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
 * @defgroup grouppll PLL control
 * @ingroup group_ring
 *  @{
 *
 * The chip uses three separate PLLs (refered to as south bridge PLL,
 * the north bridge PLL, and CPU PLL)
 * The SouthBridge PLL derives the frequencies to I/O Fabric, and
 * all the slow I/O interfaces.
 * The NorthBridge PLL drives the DRAM and the System Fabric
 * The CPU PLL drives the CPU cores, caches and local interrupts controllers.
 * These PLL’s are independent and asynchronous to
 * each other, and support up to 16 clock channels derived from each PLL
 * frequency.
 * The PLL HAL is used for controlling each PLL separately.
 *
 * Common operation exmaple:
 * @code
 * void sb_pll_set_ch14_ch15()
 * {
 *	int status = 0;
 *
 *	struct al_pll_obj obj;
 *
 *	enum al_pll_freq pll_freq;
 *	unsigned int pll_freq_val;
 *
 *	// Init SB PLL object
 *	// TODO: Obtain ref clock freq using the bootstrap HAL
 *	status = al_pll_init(
 *		(void __iomem *)0xFD860B00,
 *		"SB PLL",
 *		AL_PLL_REF_CLK_FREQ_125_MHZ,
 *		&obj);
 *	if (status)
 *		return status;
 *
 *	// Obtain PLL current frequency
 *	status = al_pll_freq_get(
 *		&obj,
 *		&pll_freq,
 *		&pll_freq_val);
 *
 *	// Set 100 MHz freq to channel 14 and 125 MHz to channel 15
 *	if (pll_freq_val % 100000) {
 *		al_err("PLL freq not suitable for 100MHz channel!\n");
 *		return -EINVAL;
 *	}
 *
 *	if (pll_freq_val % 125000) {
 *		al_err("PLL freq not suitable for 125MHz channel!\n");
 *		return -EINVAL;
 *	}
 *
 *	status = al_pll_channel_div_set(
 *		&obj,
 *		14,
 *		pll_freq_val / 100000,
 *		0,
 *		0,	// No reset
 *		0,	// Don't yet apply
 *		0);	// Not applying so no need timeout
 *	if (status)
 *		return status;
 *
 *	status = al_pll_channel_div_set(
 *		&obj,
 *		15,
 *		pll_freq_val / 125000,
 *		0,	// No reset
 *		1,	// Apply (both 14 and 15)
 *		1000);	// Let it 1ms to settle
 *	if (status)
 *		return status;
 *
 *	return status;
 * }
 * @endcode
 *
 * @file   al_hal_pll.h
 *
 * @brief Header file for the PLL HAL driver
 */

#ifndef __AL_HAL_PLL_H__
#define __AL_HAL_PLL_H__

#include "al_hal_common.h"

/* *INDENT-OFF* */
#ifdef __cplusplus
extern "C" {
#endif
/* *INDENT-ON* */

#define AL_PLL_NUM_CHANNELS	16

/**
  * Reference clock frequencies
  */
enum al_pll_ref_clk_freq {
	AL_PLL_REF_CLK_FREQ_25_MHZ,
	AL_PLL_REF_CLK_FREQ_100_MHZ,
	AL_PLL_REF_CLK_FREQ_125_MHZ,
};

/**
  * PLL frequencies
  * AL_PLL_FREQ_XXX_YYY means XXXYYY KHz
  */
enum al_pll_freq {
	AL_PLL_FREQ_NA,
	AL_PLL_FREQ_1400_000, //default
	AL_PLL_FREQ_533_333,
	AL_PLL_FREQ_666_666,
	AL_PLL_FREQ_800_000,
	AL_PLL_FREQ_916_666,
	AL_PLL_FREQ_933_333,
	AL_PLL_FREQ_1000_000,
	AL_PLL_FREQ_1062_500,
	AL_PLL_FREQ_1200_000,
	AL_PLL_FREQ_1250_000,
	AL_PLL_FREQ_1500_000,
	AL_PLL_FREQ_1600_000,
	AL_PLL_FREQ_1625_000,
	AL_PLL_FREQ_1700_000,
	AL_PLL_FREQ_1750_000,
	AL_PLL_FREQ_1800_000,
	AL_PLL_FREQ_1900_000,
	AL_PLL_FREQ_2000_000,
	AL_PLL_FREQ_2100_000,
	AL_PLL_FREQ_2125_000,
	AL_PLL_FREQ_2200_000,
	AL_PLL_FREQ_2250_000,
	AL_PLL_FREQ_2300_000,
	AL_PLL_FREQ_2375_000,
	AL_PLL_FREQ_2400_000,
	AL_PLL_FREQ_2500_000,
	AL_PLL_FREQ_2600_000,
	AL_PLL_FREQ_2625_000,
	AL_PLL_FREQ_2700_000,
	AL_PLL_FREQ_2750_000,
	AL_PLL_FREQ_2800_000,
	AL_PLL_FREQ_2875_000,
	AL_PLL_FREQ_3000_000,
	AL_PLL_FREQ_3200_000,
};

struct al_pll_obj {
	void __iomem	*regs_base;
	char		*name;
	const void	*freq_map;
	int		freq_map_size;
	unsigned int	ref_clk_freq_val;
};

/**
 * Initialization
 *
 * Initializes a PLL object
 *
 * @param  regs_base
 *             The PBS register file base pointer
 *
 * @param  name
 *             A name for the object - should remain allocated until a call to
 *             'al_pll_terminate'
 *
 * @param  ref_clk_freq
 *             Reference clock frequency
 *
 * @param obj
 *             An allocated, non initialized object context
 *
 * @return 0 upon success
 *
 */
int al_pll_init(
	void __iomem			*regs_base,
	char				*name,
	enum al_pll_ref_clk_freq	ref_clk_freq,
	struct al_pll_obj		*obj);

/**
 * Termination
 *
 * Initializes a PLL object
 *
 * @param  obj
 *             The object context
 *
 * No function besides 'al_pll_init' can be called after calling this
 * function.
 *
 * @return 0 upon success
 *
 */
int al_pll_terminate(
	struct al_pll_obj	*obj);

/**
 * Get current PLL frequency
 *
 * Gets current set PLL frequency
 *
 * @param  obj
 *             The object context
 *
 * @param freq
 *             Current PLL frequency enumeration
 *             If no such exists - AL_PLL_FREQ_NA
 *
 * @param freq_val
 *             Current PLL frequency [KHz]
 *             0 freq means that the PLL is not locked
 *
 * @return 0 upon success
 *
 */
int al_pll_freq_get(
	struct al_pll_obj	*obj,
	enum al_pll_freq	*freq,
	unsigned int		*freq_val);

/**
 * PLL frequency setting
 *
 * Adjust the PLL frequency
 * by calling this function, a glitch-free clock frequency change will
 * take place
 *
 * @param  obj
 *             The object context
 *
 * @param  freq
 *             The required frequency
 *
 * @param  timeout
 *             Timeout for the operation to finish [micro seconds]
 *
 * @return 0 upon success
 *
 */
int al_pll_freq_set(
	struct al_pll_obj	*obj,
	enum al_pll_freq	freq,
	unsigned int		timeout);

/**
 * PLL switching to bypass
 *
 * Switches the PLL to bypass frequency
 *
 * @param  obj
 *             The object context
 *
 * @param  timeout
 *             Timeout for the operation to finish [micro seconds]
 *
 * @return 0 upon success
 *
 */
int al_pll_freq_switch_to_bypass(
	struct al_pll_obj	*obj,
	unsigned int		timeout);

/**
 * PLL lock state polling
 *
 * Polls the PLL lock state
 *
 * @param  obj
 *             The object context
 *
 * @param is_locked
 *             An indication whether locked (1) or not (0)
 *
 * @return 0 upon success
 *
 */
int al_pll_is_locked(
	struct al_pll_obj	*obj,
	int			*is_locked);

/**
 * PLL channel current frequency getting
 *
 * Gets the current frequency of a specific PLL channel
 *
 * @param  obj
 *             The object context
 *
 * @param  chan_idx
 *             The channel index
 *
 * @param freq
 *             Current PLL channel frequency [KHz]
 *             0 represents a disabled channel
 *
 * @return 0 upon success
 *
 */
int al_pll_channel_freq_get(
	struct al_pll_obj	*obj,
	unsigned int		chan_idx,
	unsigned int		*freq);

/**
 * PLL channel divider settings
 *
 * Sets a specific PLL channel divider and enables the channel (if was disabled)
 * A zero divider can be provided for disabling a channel
 *
 * @param  obj
 *             The object context
 *
 * @param  chan_idx
 *             The channel index
 *
 * @param  divider
 *             The requested divider value
 *
 * @param  divider_add_half
 *             Indicates whether to add 0.5 to the divider value
 *
 * @param  reset
 *             An indication of whether or not reset is requested for this
 *             channel (usually this is not required)
 *
 * @param  apply
 *             An indication of whether or not the request should take effect
 *             immediatelly or not. When wishing to change the settings of more
 *             than one channel, keep apply=0 for all channels, but the last,
 *             and set apply=1 for the last.
 *
 * @param  timeout
 *             Timeout for the operation to finish [micro seconds]
 *
 * @return 0 upon success
 *
 */
int al_pll_channel_div_set(
	struct al_pll_obj	*obj,
	unsigned int		chan_idx,
	unsigned int		divider,
	al_bool			divider_add_half,
	int			reset,
	int			apply,
	unsigned int		timeout);

/**
 * PLL channel switching to bypass
 *
 * Switches a PLL channel to bypass, with optional channel reset, and optional
 * delayed operation for multiple channel settings.
 *
 * @param  obj
 *             The object context
 *
 * @param  chan_idx
 *             The channel index
 *
 * @param  reset
 *             An indication of whether or not reset is requested for this
 *             channel (usually this is not required)
 *
 * @param  apply
 *             An indication of whether or not the request should take effect
 *             immediatelly or not. When wishing to change the settings of more
 *             than one channel, keep apply=0 for all channels, but the last,
 *             and set apply=1 for the last.
 *
 * @param  timeout
 *             Timeout for the operation to finish [micro seconds]
 *
 * @return 0 upon success
 *
 */
int al_pll_channel_switch_to_bypass(
	struct al_pll_obj	*obj,
	unsigned int		chan_idx,
	int			reset,
	int			apply,
	unsigned int		timeout);

/**
 * PLL channel dividors lock status polling
 *
 * Polls the current PLL dividers lock status
 *
 * @param  obj
 *             The object context
 *
 * @param are_locked
 *             An indication of whether all dividers are locked
 *
 * @return 0 upon success
 *
 */
int al_pll_channel_div_are_all_locked(
	struct al_pll_obj	*obj,
	int			*are_locked);

/**
 * PLL self test
 *
 * Self tests the PLL frequency by counting how many PLL clocks are generated
 * per a specific period of time.
 *
 * @param  obj
 *             The object context
 *
 * @param  ref_clk_num_cycles
 *             The object context
 *
 * @param  ref_clk_ratio
 *             The ratio for counting PLL clocks [2^]
 *             E.g. if 'clk_ratio' is 3, each count will represent 8 PLL clocks
 *
 * @param test_passed
 *             An indication of whether the test passed (the number of sampled
 *             PLL ratioed clocks is as expected)
 *
 * @param res_pll_num_cycles
 *             The number of sampled PLL ratioed clocks
 *
 * @param  timeout
 *             Timeout for the operation to finish [micro seconds]
 *
 * @return 0 upon success
 *
 */
int al_pll_test(
	struct al_pll_obj	*obj,
	unsigned int		ref_clk_num_cycles,
	unsigned int		ref_clk_ratio,
	int					*test_passed,
	unsigned int		*res_pll_num_cycles,
	unsigned int		timeout);

/* *INDENT-OFF* */
#ifdef __cplusplus
}
#endif
/* *INDENT-ON* */
#endif		/* __AL_PLL__ */

/** @} end of PLL group */

