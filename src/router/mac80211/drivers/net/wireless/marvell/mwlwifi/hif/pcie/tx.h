/*
 * Copyright (C) 2006-2018, Marvell International Ltd.
 *
 * This software file (the "File") is distributed by Marvell International
 * Ltd. under the terms of the GNU General Public License Version 2, June 1991
 * (the "License").  You may use, redistribute and/or modify this File in
 * accordance with the terms and conditions of the License, a copy of which
 * is available by writing to the Free Software Foundation, Inc.
 *
 * THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
 * ARE EXPRESSLY DISCLAIMED.  The License provides additional details about
 * this warranty disclaimer.
 */

/* Description:  This file defines transmit related functions. */

#ifndef _TX_H_
#define _TX_H_

int pcie_tx_init(struct ieee80211_hw *hw);
void pcie_tx_deinit(struct ieee80211_hw *hw);
void pcie_tx_skbs(unsigned long data);
void pcie_tx_done(unsigned long data);
void pcie_tx_flush_amsdu(unsigned long data);
void pcie_tx_xmit(struct ieee80211_hw *hw,
		  struct ieee80211_tx_control *control,
		  struct sk_buff *skb);
void pcie_tx_del_pkts_via_vif(struct ieee80211_hw *hw,
			      struct ieee80211_vif *vif);
void pcie_tx_del_pkts_via_sta(struct ieee80211_hw *hw,
			      struct ieee80211_sta *sta);
void pcie_tx_del_ampdu_pkts(struct ieee80211_hw *hw,
			    struct ieee80211_sta *sta, u8 tid);
void pcie_tx_del_sta_amsdu_pkts(struct ieee80211_hw *hw,
				struct ieee80211_sta *sta);

#endif /* _TX_H_ */
