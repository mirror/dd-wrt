/*
 * Copyright 2024 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _REG_RULES_H_
#define _REG_RULES_H_

#include "dot11ah.h"

#define AUTO_BW	NL80211_RRF_AUTO_BW

#define REG_RULE_KHZ(start, end, bw, gain, eirp, reg_flags)	\
{								\
	.freq_range.start_freq_khz = start,			\
	.freq_range.end_freq_khz = end,				\
	.freq_range.max_bandwidth_khz = bw,			\
	.power_rule.max_antenna_gain = DBI_TO_MBI(gain),	\
	.power_rule.max_eirp = DBM_TO_MBM(eirp),		\
	.flags = reg_flags,					\
	.dfs_cac_ms = 0,					\
}

/**
 * The duty cycle for AP and STA is provided in 100ths of a percent. E.g. 10000 = 100%
 */
#define MORSE_REG_RULE_KHZ(start, end, bw, gain, eirp, reg_flags, duty_cycle_ap,	\
			   duty_cycle_sta, duty_cycle_omit_ctrl_resp, mpsw_min_us,	\
			   mpsw_max_us, mpsw_win_length_us)				\
{											\
	.dot11_reg = REG_RULE_KHZ(start, end, bw, gain, eirp, reg_flags),		\
	.duty_cycle.ap = duty_cycle_ap,							\
	.duty_cycle.sta = duty_cycle_sta,						\
	.duty_cycle.omit_ctrl_resp = duty_cycle_omit_ctrl_resp,				\
	.mpsw.airtime_min_us = mpsw_min_us,						\
	.mpsw.airtime_max_us = mpsw_max_us,						\
	.mpsw.window_length_us = mpsw_win_length_us					\
}

#define MORSE_REG_RULE(start, end, bw, gain, eirp, reg_flags, duty_cycle_ap,			\
		       duty_cycle_sta, duty_cycle_omit_ctrl_resp, mpsw_min_us,			\
		       mpsw_max_us, mpsw_win_length_us)						\
	MORSE_REG_RULE_KHZ(MHZ_TO_KHZ(start), MHZ_TO_KHZ(end), MHZ_TO_KHZ(bw), gain, eirp,	\
			   reg_flags, duty_cycle_ap, duty_cycle_sta, duty_cycle_omit_ctrl_resp, \
			   mpsw_min_us, mpsw_max_us, mpsw_win_length_us)

/**
 * Take a channel alpha and return the corresponding regulatory domain
 *
 * @param alpha A pointer to a country alpha
 * @returns struct morse_regdomain of the domain with the matching channel alpha
 *			else NULL if no match is found
 */
const struct morse_regdomain *morse_reg_alpha_lookup(const char *alpha);

#endif /*_REG_RULES_H_ */
