#include <net80211/ieee80211_node.h>
#include "linux/wprobe.h"

atomic_t cleanup_tasks = ATOMIC_INIT(0);

enum wp_node_val {
	WP_NODE_RSSI,
	WP_NODE_SIGNAL,
	WP_NODE_RX_RATE,
	WP_NODE_TX_RATE,
	WP_NODE_RETRANSMIT_200,
	WP_NODE_RETRANSMIT_400,
	WP_NODE_RETRANSMIT_800,
	WP_NODE_RETRANSMIT_1600,
};

enum wp_global_val {
	WP_GLOBAL_NOISE,
	WP_GLOBAL_PHY_BUSY,
	WP_GLOBAL_PHY_RX,
	WP_GLOBAL_PHY_TX,
	WP_GLOBAL_FRAMES,
	WP_GLOBAL_PROBEREQ,
};

static struct wprobe_item ath_wprobe_globals[] = {
	[WP_GLOBAL_NOISE] = {
			     .name = "noise",
			     .type = WPROBE_VAL_S16,
			     .flags = WPROBE_F_KEEPSTAT },
	[WP_GLOBAL_PHY_BUSY] = {
				.name = "phy_busy",
				.type = WPROBE_VAL_U8,
				.flags = WPROBE_F_KEEPSTAT },
	[WP_GLOBAL_PHY_RX] = {
			      .name = "phy_rx",
			      .type = WPROBE_VAL_U8,
			      .flags = WPROBE_F_KEEPSTAT },
	[WP_GLOBAL_PHY_TX] = {
			      .name = "phy_tx",
			      .type = WPROBE_VAL_U8,
			      .flags = WPROBE_F_KEEPSTAT },
	[WP_GLOBAL_FRAMES] = {
			      .name = "frames",
			      .type = WPROBE_VAL_U32,
			       },
	[WP_GLOBAL_PROBEREQ] = {
				.name = "probereq",
				.type = WPROBE_VAL_U32,
				 },
};

static struct wprobe_item ath_wprobe_link[] = {
	[WP_NODE_RSSI] = {
			  .name = "rssi",
			  .type = WPROBE_VAL_U8,
			  .flags = WPROBE_F_KEEPSTAT },
	[WP_NODE_SIGNAL] = {
			    .name = "signal",
			    .type = WPROBE_VAL_S16,
			    .flags = WPROBE_F_KEEPSTAT },
	[WP_NODE_RX_RATE] = {
			     .name = "rx_rate",
			     .type = WPROBE_VAL_U16,
			     .flags = WPROBE_F_KEEPSTAT },
	[WP_NODE_TX_RATE] = {
			     .name = "tx_rate",
			     .type = WPROBE_VAL_U16,
			     .flags = WPROBE_F_KEEPSTAT },
	[WP_NODE_RETRANSMIT_200] = {
				    .name = "retransmit_200",
				    .type = WPROBE_VAL_U8,
				    .flags = WPROBE_F_KEEPSTAT },
	[WP_NODE_RETRANSMIT_400] = {
				    .name = "retransmit_400",
				    .type = WPROBE_VAL_U8,
				    .flags = WPROBE_F_KEEPSTAT },
	[WP_NODE_RETRANSMIT_800] = {
				    .name = "retransmit_800",
				    .type = WPROBE_VAL_U8,
				    .flags = WPROBE_F_KEEPSTAT },
	[WP_NODE_RETRANSMIT_1600] = {
				     .name = "retransmit_1600",
				     .type = WPROBE_VAL_U8,
				     .flags = WPROBE_F_KEEPSTAT },
};

#define AR5K_MIBC       0x0040
#define AR5K_MIBC_FREEZE	(1 << 1)
#define AR5K_TXFC       0x80ec
#define AR5K_RXFC       0x80f0
#define AR5K_RXCLEAR    0x80f4
#define AR5K_CYCLES     0x80f8

#define READ_CLR(_ah, _reg) \
	({ u32 __val = OS_REG_READ(_ah, _reg); OS_REG_WRITE(_ah, _reg, 0); __val; })

#if 1
#define wprobe_disabled() 0
#else
static bool wprobe_disabled(void)
{
	return (!wprobe_add_iface || IS_ERR(wprobe_add_iface));
}
#endif
static int ath_wprobe_sync(struct wprobe_iface *dev, struct wprobe_link *l, struct wprobe_value *val, bool measure)
{
	struct ath_vap *avp = container_of(dev, struct ath_vap, av_wpif);
	struct ieee80211vap *vap = &avp->av_vap;
	struct ieee80211com *ic = vap->iv_ic;
	struct ath_softc *sc = netdev_priv(ic->ic_dev);
	struct ath_hal *ah = sc->sc_ah;
	u32 cc, busy, rx, tx;
	s16 noise;

	if (l)
		goto out;

	OS_REG_WRITE(ah, AR5K_MIBC, AR5K_MIBC_FREEZE);
	cc = READ_CLR(ah, AR5K_CYCLES);
	busy = READ_CLR(ah, AR5K_RXCLEAR);
	rx = READ_CLR(ah, AR5K_RXFC);
	tx = READ_CLR(ah, AR5K_TXFC);
	OS_REG_WRITE(ah, AR5K_MIBC, 0);
	noise = ath_hw_read_nf(sc);

	WPROBE_FILL_BEGIN(val, ath_wprobe_globals);
	if (cc & 0xf0000000) {
		/* scale down if the counters are near max */
		cc >>= 8;
		busy >>= 8;
		rx >>= 8;
		tx >>= 8;
	}
	if (ah->ah_macType < 5212)
		goto phy_skip;
	if (!cc)
		goto phy_skip;
	if (busy > cc)
		goto phy_skip;
	if (rx > cc)
		goto phy_skip;
	if (tx > cc)
		goto phy_skip;
	busy = (busy * 100) / cc;
	rx = (rx * 100) / cc;
	tx = (tx * 100) / cc;
	WPROBE_SET(WP_GLOBAL_PHY_BUSY, U8, busy);
	WPROBE_SET(WP_GLOBAL_PHY_RX, U8, rx);
	WPROBE_SET(WP_GLOBAL_PHY_TX, U8, tx);
	WPROBE_SET(WP_GLOBAL_FRAMES, U32, avp->av_rxframes);
	WPROBE_SET(WP_GLOBAL_PROBEREQ, U32, avp->av_rxprobereq);

phy_skip:
	WPROBE_SET(WP_GLOBAL_NOISE, S16, noise);
	WPROBE_FILL_END();

out:
	return 0;
}

#undef AR5K_TXFC
#undef AR5K_RXFC
#undef AR5K_RXCLEAR
#undef AR5K_CYCLES
#undef AR5K_MIBC
#undef AR5K_MIBC_FREEZE
#undef READ_CLR

static const struct wprobe_iface ath_wprobe_dev = {
	.link_items = ath_wprobe_link,
	.n_link_items = ARRAY_SIZE(ath_wprobe_link),
	.global_items = ath_wprobe_globals,
	.n_global_items = ARRAY_SIZE(ath_wprobe_globals),
	.sync_data = ath_wprobe_sync,
};

static int ath_lookup_rateval(struct ieee80211_node *ni, int rate)
{
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211com *ic = vap->iv_ic;
	struct ath_softc *sc = netdev_priv(ic->ic_dev);
	const HAL_RATE_TABLE *rt = sc->sc_currates;

	if ((!rt) || (rate < 0) || (rate >= ARRAY_SIZE(sc->sc_hwmap)))
		return -1;

	rate = sc->sc_hwmap[rate].ieeerate;
	rate = sc->sc_rixmap[rate & IEEE80211_RATE_VAL];
	if ((rate < 0) || (rate >= rt->rateCount))
		return -1;

	return rt->info[rate].rateKbps;
}

static void ath_wprobe_report_rx(struct ieee80211vap *vap, struct ath_rx_status *rs, struct sk_buff *skb)
{
	const struct ieee80211_frame *wh = (struct ieee80211_frame *)skb->data;
	struct wprobe_wlan_hdr hdr;
	struct ath_vap *avp;
	int hdrsize;

	if (wprobe_disabled())
		return;

	avp = ATH_VAP(vap);
	avp->av_rxframes++;
	if (wh->i_fc[0] == (IEEE80211_FC0_TYPE_MGT | IEEE80211_FC0_SUBTYPE_PROBE_REQ))
		avp->av_rxprobereq++;

	memset(&hdr, 0, sizeof(hdr));
	hdr.len = skb->len;
	hdr.snr = rs->rs_rssi;
	hdr.type = WPROBE_PKT_RX;
	if ((wh->i_fc[0] & IEEE80211_FC0_TYPE_MASK) == IEEE80211_FC0_TYPE_CTL)
		hdrsize = sizeof(struct ieee80211_ctlframe_addr2);
	else
		hdrsize = ieee80211_hdrsize(skb->data);
	wprobe_add_frame(&avp->av_wpif, &hdr, skb->data, hdrsize + 0x42);
}

static void ath_node_sample_rx(struct ieee80211_node *ni, struct ath_rx_status *rs)
{
	struct ath_node *an = ATH_NODE(ni);
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211com *ic = vap->iv_ic;
	struct wprobe_link *l = &an->an_wplink;
	struct wprobe_value *v = l->val;
	unsigned long flags;
	int rate;

	if (wprobe_disabled() || !an->an_wplink_active || !l->val)
		return;

	rate = ath_lookup_rateval(ni, rs->rs_rate);

	spin_lock_irqsave(&l->iface->lock, flags);
	WPROBE_FILL_BEGIN(v, ath_wprobe_link);
	WPROBE_SET(WP_NODE_RSSI, U8, rs->rs_rssi);
	WPROBE_SET(WP_NODE_SIGNAL, S16, ic->ic_channoise + rs->rs_rssi);
	if ((rate > 0) && (rate <= 600000))
		WPROBE_SET(WP_NODE_RX_RATE, U16, rate);
	WPROBE_FILL_END();
	wprobe_update_stats(l->iface, l);
	spin_unlock_irqrestore(&l->iface->lock, flags);
}

static void ath_wprobe_report_tx(struct ieee80211vap *vap, struct ath_tx_status *ts, struct sk_buff *skb)
{
	const struct ieee80211_frame *wh = (struct ieee80211_frame *)skb->data;
	struct wprobe_wlan_hdr hdr;
	struct ath_vap *avp;
	int hdrsize;

	if (wprobe_disabled())
		return;

	avp = ATH_VAP(vap);

	memset(&hdr, 0, sizeof(hdr));
	hdr.len = skb->len;
	hdr.snr = ts->ts_rssi;
	hdr.type = WPROBE_PKT_TX;
	if ((wh->i_fc[0] & IEEE80211_FC0_TYPE_MASK) == IEEE80211_FC0_TYPE_CTL)
		hdrsize = sizeof(struct ieee80211_ctlframe_addr2);
	else
		hdrsize = ieee80211_hdrsize(skb->data);
	wprobe_add_frame(&avp->av_wpif, &hdr, skb->data, hdrsize + 0x42);
}

static void ath_node_sample_tx(struct ieee80211_node *ni, struct ath_tx_status *ts, struct sk_buff *skb)
{
	struct ath_node *an = ATH_NODE(ni);
	struct ieee80211vap *vap = ni->ni_vap;
	struct ieee80211com *ic = vap->iv_ic;
	struct wprobe_link *l = &an->an_wplink;
	struct wprobe_value *v = l->val;
	unsigned long flags;
	int rate, rexmit_counter;
	int len = skb->len;

	if (wprobe_disabled() || !an->an_wplink_active || !l->val)
		return;

	ath_wprobe_report_tx(vap, ts, skb);
	rate = ath_lookup_rateval(ni, ts->ts_rate);

	spin_lock_irqsave(&l->iface->lock, flags);
	WPROBE_FILL_BEGIN(v, ath_wprobe_link);
	WPROBE_SET(WP_NODE_RSSI, U8, ts->ts_rssi);
	WPROBE_SET(WP_NODE_SIGNAL, S16, ic->ic_channoise + ts->ts_rssi);

	if (len <= 200)
		rexmit_counter = WP_NODE_RETRANSMIT_200;
	else if (len <= 400)
		rexmit_counter = WP_NODE_RETRANSMIT_400;
	else if (len <= 800)
		rexmit_counter = WP_NODE_RETRANSMIT_800;
	else
		rexmit_counter = WP_NODE_RETRANSMIT_1600;
	WPROBE_SET(rexmit_counter, U8, ts->ts_longretry);

	if ((rate > 0) && (rate <= 600000))
		WPROBE_SET(WP_NODE_TX_RATE, U16, rate);
	WPROBE_FILL_END();
	wprobe_update_stats(l->iface, l);
	spin_unlock_irqrestore(&l->iface->lock, flags);
}

static void ath_wprobe_node_join(struct ieee80211vap *vap, struct ieee80211_node *ni)
{
	struct wprobe_iface *dev;
	struct wprobe_link *l;
	struct ath_vap *avp;
	struct ath_node *an = ATH_NODE(ni);

	if (wprobe_disabled() || an->an_wplink_active)
		return;

	avp = ATH_VAP(vap);
	dev = &avp->av_wpif;
	l = &an->an_wplink;

	ieee80211_ref_node(ni);
	wprobe_add_link(dev, l, ni->ni_macaddr);
	an->an_wplink_active = 1;
}

static void ath_wprobe_do_node_leave(struct work_struct *work)
{
	struct ath_node *an = container_of(work, struct ath_node, an_destroy);
	struct ieee80211_node *ni = &an->an_node;
	struct ieee80211vap *vap = ni->ni_vap;
	struct wprobe_iface *dev;
	struct wprobe_link *l;
	struct ath_vap *avp;

	avp = ATH_VAP(vap);
	dev = &avp->av_wpif;
	l = &an->an_wplink;

	wprobe_remove_link(dev, l);
	ieee80211_unref_node(&ni);
	atomic_dec(&cleanup_tasks);
}

static void ath_wprobe_node_leave(struct ieee80211vap *vap, struct ieee80211_node *ni)
{
	struct ath_node *an = ATH_NODE(ni);

	if (wprobe_disabled() || !an->an_wplink_active)
		return;

	atomic_inc(&cleanup_tasks);
	an->an_wplink_active = 0;
	IEEE80211_INIT_WORK(&an->an_destroy, ath_wprobe_do_node_leave);
	schedule_work(&an->an_destroy);
}

static void ath_init_wprobe_dev(struct ath_vap *avp)
{
	struct ieee80211vap *vap = &avp->av_vap;
	struct wprobe_iface *dev = &avp->av_wpif;

	if (wprobe_disabled() || (vap->iv_opmode == IEEE80211_M_WDS))
		return;

	memcpy(dev, &ath_wprobe_dev, sizeof(struct wprobe_iface));
	dev->addr = vap->iv_myaddr;
	dev->name = vap->iv_dev->name;
	wprobe_add_iface(dev);
}

static void ath_remove_wprobe_dev(struct ath_vap *avp)
{
	struct ieee80211vap *vap = &avp->av_vap;
	struct ieee80211com *ic = vap->iv_ic;
	struct ieee80211_node *ni;
	struct wprobe_iface *dev = &avp->av_wpif;
	struct wprobe_link *l;
	struct ath_node *an;
	unsigned long flags;

	if (wprobe_disabled() || (vap->iv_opmode == IEEE80211_M_WDS))
		return;

restart:
	rcu_read_lock();
	list_for_each_entry_rcu(l, &dev->links, list) {
		an = container_of(l, struct ath_node, an_wplink);

		if (!an->an_wplink_active)
			continue;

		ni = &an->an_node;
		ath_wprobe_node_leave(vap, ni);
		rcu_read_unlock();
		goto restart;
	}
	rcu_read_unlock();

	/* wait for the cleanup tasks to finish */
	while (atomic_read(&cleanup_tasks) != 0) {
		schedule();
	}

	wprobe_remove_iface(dev);
}
