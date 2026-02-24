#ifndef _MORSE_MONITOR_H_
#define _MORSE_MONITOR_H_

/*
 * Copyright 2017-2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include <net/mac80211.h>

int morse_mon_init(struct morse *mors);

void morse_mon_free(struct morse *mors);

void morse_mon_rx(struct morse *mors, struct sk_buff *rx_skb,
		  struct morse_skb_rx_status *hdr_rx_status);

void morse_mon_sig_field_error(const struct morse_cmd_evt_sig_field_error *sig_field_error_evt);

#endif /* !_MORSE_MONITOR_H_ */
