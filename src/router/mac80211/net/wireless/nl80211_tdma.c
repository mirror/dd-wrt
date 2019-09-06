/*
 * Some TDMA support code for cfg80211.
 *
 * Copyright 2011-2013	Stanislav V. Korsakov <sta@stasoft.net>
 */

#include <linux/if.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/if_ether.h>
#include <linux/ieee80211.h>
#include <linux/nl80211.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include <linux/etherdevice.h>
#include <net/net_namespace.h>
#include <net/genetlink.h>
#include <net/cfg80211.h>
#include <net/sock.h>
#include "core.h"
#include "nl80211.h"
#include "reg.h"

extern struct cfg80211_cached_keys *
nl80211_parse_connkeys(struct cfg80211_registered_device *rdev,
		       struct nlattr *keys);
extern int nl80211_parse_chandef(struct cfg80211_registered_device *rdev,
				 struct genl_info *info,
				 struct cfg80211_chan_def *chandef);

extern bool nl80211_parse_mcast_rate(struct cfg80211_registered_device *rdev,
			 int mcast_rate[NUM_NL80211_BANDS],
			 int rateval);

static const unsigned ht20_mcs_rates[] = { 65, 130, 195, 260, 390, 520, 585, 650, 130, 260, 390, 520, 780, 1040, 1170, 1300, 195, 390, 585, 780, 1170, 1560, 1755, 1950, 260, 520, 780, 1040, 1560, 2080, 2340, 2600 };
static const unsigned ht40_mcs_rates[] = { 135, 270, 405, 540, 810, 1080, 1215, 1350, 270, 540, 810, 1080, 1620, 2160, 2430, 2700 };

static const struct nla_policy nl80211_txattr_policy[NL80211_TXRATE_MAX + 1] = {
	[NL80211_TXRATE_LEGACY] = { .type = NLA_BINARY,
				    .len = NL80211_MAX_SUPP_RATES },
	[NL80211_TXRATE_MCS] = { .type = NLA_BINARY,
				 .len = NL80211_MAX_SUPP_HT_RATES },
};

int nl80211_join_tdma(struct sk_buff *skb, struct genl_info *info)
{
	struct cfg80211_registered_device *rdev = info->user_ptr[0];
	struct net_device *dev = info->user_ptr[1];
	struct cfg80211_tdma_params tdma;
	struct wiphy *wiphy;
	int err;
	struct nlattr *tb[NL80211_TXRATE_MAX + 1];
	int rem, i;
	struct nlattr *tx_rates;
	struct ieee80211_supported_band *sband;
	u8 * rates;
	struct cfg80211_cached_keys *connkeys = NULL;

	memset(&tdma, 0, sizeof(tdma));

	if (!info->attrs[NL80211_ATTR_WIPHY_FREQ]) {
		return -EINVAL;
	}

	if (!rdev->ops->join_tdma) {
		return -EOPNOTSUPP;
	}

	if (dev->ieee80211_ptr->iftype != NL80211_IFTYPE_TDMA) {
		return -EOPNOTSUPP;
	}

	if (info->attrs[NL80211_ATTR_TX_RATES] == NULL)
		return -EINVAL;

	wiphy = &rdev->wiphy;

	err = nl80211_parse_chandef(rdev, info, &tdma.chandef);
	if (err) {
		return err;
	}

	if (!cfg80211_reg_can_beacon(&rdev->wiphy, &tdma.chandef, NL80211_IFTYPE_TDMA)) {
		return -EINVAL;
	}

	if (info->attrs[NL80211_ATTR_TDMA_VERSION])
	    tdma.version = nla_get_u8(info->attrs[NL80211_ATTR_TDMA_VERSION]);

	switch (tdma.chandef.width) {
	case NL80211_CHAN_WIDTH_20_NOHT:
		break;
	case NL80211_CHAN_WIDTH_20:
	case NL80211_CHAN_WIDTH_40:
		if (rdev->wiphy.features & NL80211_FEATURE_HT_IBSS)
			break;
	default:
		break;
	}
	/*
	 * The nested attribute uses enum nl80211_band as the index. This maps
	 * directly to the enum nl80211_band values used in cfg80211.
	 */
	BUILD_BUG_ON(NL80211_MAX_SUPP_HT_RATES > IEEE80211_HT_MCS_MASK_LEN * 8);
	nla_for_each_nested(tx_rates, info->attrs[NL80211_ATTR_TX_RATES], rem)
	{
		enum nl80211_band band = nla_type(tx_rates);
		if (band < 0 || band >= NUM_NL80211_BANDS)
			return -EINVAL;
		sband = rdev->wiphy.bands[band];
		if (sband == NULL)
			return -EINVAL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0)
		nla_parse(tb, NL80211_TXRATE_MAX, nla_data(tx_rates),
			  nla_len(tx_rates), nl80211_txattr_policy, NULL);
#else
		nla_parse(tb, NL80211_TXRATE_MAX, nla_data(tx_rates),
			  nla_len(tx_rates), nl80211_txattr_policy);
#endif
		if (tb[NL80211_TXRATE_LEGACY]) {
		    rates = nla_data(tb[NL80211_TXRATE_LEGACY]);
		    for (i = 0; i < nla_len(tb[NL80211_TXRATE_LEGACY]); i++) {
			int rate = (rates[i] & 0x7f) * 5;
			int ridx;
			for (ridx = 0; ridx < sband->n_bitrates; ridx++) {
			    struct ieee80211_rate *srate = &sband->bitrates[ridx];
			    if (rate == srate->bitrate) {
				tdma.supp_rates |= BIT(ridx);
				tdma.mask.control[band].legacy = 1 << ridx;
				tdma.cur_rate = (u8)ridx;
				tdma.rate = rate;
			    }
			}
		    }
		} else {
		    if ( tdma.version) {
			tdma.supp_rates = 0xFFFFFFFF;
			tdma.mask.control[band].legacy = 0xFFFFFFFF;
		    }
		}
		if (tb[NL80211_TXRATE_MCS]) {
		    rates = nla_data(tb[NL80211_TXRATE_MCS]);
		    for (i = 0; i < nla_len(tb[NL80211_TXRATE_MCS]); i++) {
			int ridx, rbit;

			ridx = rates[i] / 8;
			rbit = BIT(rates[i] % 8);

			/* check validity */
			if ((ridx < 0) || (ridx >= IEEE80211_HT_MCS_MASK_LEN))
			    continue;
			/* check availability */
			if (sband->ht_cap.mcs.rx_mask[ridx] & rbit) {
			    tdma.mask.control[band].ht_mcs[ridx] |= rbit;
			    switch (tdma.chandef.width) {
				case NL80211_CHAN_WIDTH_40:
				    tdma.rate = ht40_mcs_rates[rates[i]];
				break;
				default:
				    tdma.rate = ht20_mcs_rates[rates[i]];
				break;
			    }
			}
		    }
		}
	}

	tdma.privacy = !!info->attrs[NL80211_ATTR_PRIVACY];
	if (tdma.privacy && info->attrs[NL80211_ATTR_KEYS]) {
		connkeys = nl80211_parse_connkeys(rdev,
					info->attrs[NL80211_ATTR_KEYS]);
		if (IS_ERR(connkeys))
			return PTR_ERR(connkeys);
	}

	if (info->attrs[NL80211_ATTR_TDMA_NODES])
	    tdma.node_num = nla_get_u8(info->attrs[NL80211_ATTR_TDMA_NODES]);

	if ( tdma.version) {
	    if (info->attrs[NL80211_ATTR_TDMA_SLOT_SIZE])
		tdma.cur_rate = nla_get_u8(info->attrs[NL80211_ATTR_TDMA_SLOT_SIZE]);
	    else
		tdma.cur_rate = 0;
	}

	if (info->attrs[NL80211_ATTR_SSID]) {
		tdma.ssid.ssid_len =
			nla_len(info->attrs[NL80211_ATTR_SSID]);
		if (tdma.ssid.ssid_len == 0 ||
		    tdma.ssid.ssid_len > IEEE80211_MAX_SSID_LEN)
			return -EINVAL;
		memcpy( tdma.ssid.ssid, nla_data(info->attrs[NL80211_ATTR_SSID]), tdma.ssid.ssid_len);
	}

	if (info->attrs[NL80211_ATTR_TDMA_AGGREGATION])
	    tdma.no_msdu = nla_get_u8(info->attrs[NL80211_ATTR_TDMA_AGGREGATION]);

	tdma.gack = nla_get_flag(info->attrs[NL80211_ATTR_TDMA_GACK]);
	tdma.no_reorder = nla_get_flag(info->attrs[NL80211_ATTR_TDMA_REORDER]);

	if (info->attrs[NL80211_ATTR_MCAST_RATE] &&
	    !nl80211_parse_mcast_rate(rdev, tdma.mcast_rate,
			nla_get_u32(info->attrs[NL80211_ATTR_MCAST_RATE])))
		return -EINVAL;

	if (info->attrs[NL80211_ATTR_MAC]) {
	    nla_memcpy(tdma.bs_mac, info->attrs[NL80211_ATTR_MAC], ETH_ALEN);
	    if (!is_valid_ether_addr(tdma.bs_mac))
		return -EADDRNOTAVAIL;
	}

	tdma.control_port = nla_get_flag(info->attrs[NL80211_ATTR_CONTROL_PORT]);
	tdma.enable_polling = nla_get_flag(info->attrs[NL80211_ATTR_TDMA_POLLING]);
	if (tdma.version == 2) {
	    if (info->attrs[NL80211_ATTR_TDMA_TX_RATIO])
		tdma.tx_ratio = nla_get_u8(info->attrs[NL80211_ATTR_TDMA_TX_RATIO]);
	    if (info->attrs[NL80211_ATTR_TDMA_RX_RATIO])
		tdma.rx_ratio = nla_get_u8(info->attrs[NL80211_ATTR_TDMA_RX_RATIO]);
	}
	if ((err = cfg80211_join_tdma(rdev, dev, &tdma, connkeys)))
	    kfree( connkeys);
	return err;
}

int nl80211_leave_tdma(struct sk_buff *skb, struct genl_info *info)
{
	struct cfg80211_registered_device *rdev = info->user_ptr[0];
	struct net_device *dev = info->user_ptr[1];

	if (!rdev->ops->leave_tdma)
		return -EOPNOTSUPP;

	if (dev->ieee80211_ptr->iftype != NL80211_IFTYPE_TDMA)
		return -EOPNOTSUPP;

	return cfg80211_leave_tdma(rdev, dev, false);
}
