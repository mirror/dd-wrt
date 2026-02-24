/*
 * Copyright 2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

/* This initial value is for Minstrel, and there is another in rc.h for MMRC */
#define INIT_MAX_RATES_NUM	3

void morse_rc_sta_fill_tx_rates(struct morse *mors,
				struct morse_skb_tx_info *tx_info,
				struct sk_buff *skb,
				struct ieee80211_sta *sta, int tx_bw_mhz, bool rts_allowed);
