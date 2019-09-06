/*
 * Some TDMA support code for mac80211.
 *
 * Copyright 2011-2013	Stanislav V. Korsakov <sta@stasoft.net>
*/

#ifndef	TDMA_I_H
#define	TDMA_I_H

#include "pdwext.h"

extern void ieee80211_tdma_stop(struct ieee80211_sub_if_data *);
extern void ieee80211_tdma_rx_queued_mgmt(struct ieee80211_sub_if_data *, struct sk_buff *);
extern void ieee80211_tdma_setup_sdata(struct ieee80211_sub_if_data *);
extern struct sk_buff *tdma_create_beacon(struct ieee80211_sub_if_data *);
extern void tdma_reset_state(struct ieee80211_if_tdma *);
extern void ieee80211_tdma_setup_sdata(struct ieee80211_sub_if_data *);
extern void ieee80211_tdma_work(struct ieee80211_sub_if_data *);
extern void tdma_upd_rc(struct ieee80211_sub_if_data *, struct sk_buff *);
extern void tdma_sta_cleanup(struct sta_info *);

#if IS_ENABLED(CPTCFG_MAC80211_COMPRESS)
extern __weak ieee80211_rx_result ieee80211_rx_h_decompress(struct ieee80211_rx_data *);
extern __weak ieee80211_tx_result ieee80211_tx_h_compress(struct ieee80211_tx_data *);
#endif

#endif
