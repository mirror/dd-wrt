/*
 * Copyright 2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#ifndef _MORSE_RC_H_
#define _MORSE_RC_H_

#include "morse.h"
#include "mmrc-submodule/src/core/mmrc.h"
#include "linux/list.h"
#include <linux/workqueue.h>

/* This initial value is for MMRC, and there is another in minstrel_rc.h for Minstrel */
#define INIT_MAX_RATES_NUM 4

struct morse_rc {
	/* Serialise rate control queue manipulation and timer functions */
	spinlock_t lock;
	struct list_head stas;
	struct timer_list timer;
	struct work_struct work;
	struct morse *mors;
};

struct morse_rc_sta {
	struct mmrc_table *tb;
	struct list_head list;

	unsigned long last_update;
};

int morse_rc_init(struct morse *mors);

int morse_rc_deinit(struct morse *mors);

int morse_rc_sta_add(struct morse *mors, struct ieee80211_vif *vif, struct ieee80211_sta *sta);

#define morse_rc_set_fixed_rate(mors, sta, mcs, bw, ss, guard) \
	_morse_rc_set_fixed_rate(mors, sta, mcs, bw, ss, guard, __func__)
bool _morse_rc_set_fixed_rate(struct morse *mors,
			      struct ieee80211_sta *sta,
			      int mcs, int bw, int ss, int guard, const char *caller);

bool morse_rc_get_enable_fixed_rate(void);
int morse_rc_get_fixed_bandwidth(void);
int morse_rc_get_fixed_mcs(void);
int morse_rc_get_fixed_ss(void);
int morse_rc_get_fixed_guard(void);

void morse_rc_sta_remove(struct morse *mors, struct ieee80211_sta *sta);

void morse_rc_sta_fill_tx_rates(struct morse *mors,
				struct morse_skb_tx_info *tx_info,
				struct sk_buff *skb,
				struct ieee80211_sta *sta, int tx_bw, bool rts_allowed);

void morse_rc_sta_feedback_rates(struct morse *mors, struct sk_buff *skb,
				 struct ieee80211_sta *sta, struct morse_skb_tx_status *tx_sts,
				 int tx_attempts);

void morse_rc_sta_state_check(struct morse *mors,
			      struct ieee80211_vif *vif, struct ieee80211_sta *sta,
			      enum ieee80211_sta_state old_state,
			      enum ieee80211_sta_state new_state);

/*
 * Reinitialize the associated stations when there is a change in BW.
 * Must be called with mors->lock held
 */
void morse_rc_reinit_stas(struct morse *mors, struct ieee80211_vif *vif);

/**
 * morse_rc_vif_update_mcast_rate() - Goes through the list of STAs connected to
 * VIF to find and update MMRC rate for multicast traffic.
 *
 * @note: This function must be called with mutex mors->lock held.
 *
 * @mors: Global morse struct
 * @mors_vif: The morse VIF struct
 */
void morse_rc_vif_update_mcast_rate(struct morse *mors, struct morse_vif *mors_vif);

#endif /* !_MORSE_RC_H_ */
