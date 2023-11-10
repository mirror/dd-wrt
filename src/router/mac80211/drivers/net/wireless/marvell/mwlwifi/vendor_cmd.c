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

/* Description:  This file implements vendor spcific functions. */

#include <net/mac80211.h>
#include <net/netlink.h>

#include "sysadpt.h"
#include "core.h"
#include "utils.h"
#include "hif/fwcmd.h"
#include "vendor_cmd.h"

static const struct nla_policy mwl_vendor_attr_policy[NUM_MWL_VENDOR_ATTR] = {
	[MWL_VENDOR_ATTR_BF_TYPE] = { .type = NLA_U8 },
};

static int mwl_vendor_cmd_set_bf_type(struct wiphy *wiphy,
				      struct wireless_dev *wdev,
				      const void *data, int data_len)
{
	struct ieee80211_hw *hw = wiphy_to_ieee80211_hw(wiphy);
	struct mwl_priv *priv = hw->priv;
	struct nlattr *tb[NUM_MWL_VENDOR_ATTR];
	int rc;
	u8 val;

	if (priv->chip_type != MWL8964)
		return -EPERM;

	rc = nla_parse(tb, MWL_VENDOR_ATTR_MAX, data, data_len,
		       mwl_vendor_attr_policy, NULL);
	if (rc)
		return rc;

	if (!tb[MWL_VENDOR_ATTR_BF_TYPE])
		return -EINVAL;

	val = nla_get_u8(tb[MWL_VENDOR_ATTR_BF_TYPE]);
	if ((val < TXBF_MODE_OFF) || (val > TXBF_MODE_BFMER_AUTO))
		return -EINVAL;
	wiphy_dbg(wiphy, "set bf_type: 0x%x\n", val);

	rc = mwl_fwcmd_set_bftype(hw, val);
	if (!rc)
		priv->bf_type = val;

	return rc;
}

static int mwl_vendor_cmd_get_bf_type(struct wiphy *wiphy,
				      struct wireless_dev *wdev,
				      const void *data, int data_len)
{
	struct ieee80211_hw *hw = wiphy_to_ieee80211_hw(wiphy);
	struct mwl_priv *priv = hw->priv;
	struct sk_buff *skb;

	if (priv->chip_type != MWL8964)
		return -EPERM;

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, 8);
	if (!skb)
		return -ENOMEM;

	nla_put_u8(skb, MWL_VENDOR_ATTR_BF_TYPE, priv->bf_type);

	return cfg80211_vendor_cmd_reply(skb);
}

static const struct wiphy_vendor_command mwl_vendor_commands[] = {
	{
		.info = { .vendor_id = MRVL_OUI,
			  .subcmd = MWL_VENDOR_CMD_SET_BF_TYPE},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mwl_vendor_cmd_set_bf_type,
		.policy = mwl_vendor_attr_policy,
	},
	{
		.info = { .vendor_id = MRVL_OUI,
			  .subcmd = MWL_VENDOR_CMD_GET_BF_TYPE},
		.flags = WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mwl_vendor_cmd_get_bf_type,
		.policy = mwl_vendor_attr_policy,
	}
};

static const struct nl80211_vendor_cmd_info mwl_vendor_events[] = {
	{
		.vendor_id = MRVL_OUI,
		.subcmd =  MWL_VENDOR_EVENT_DRIVER_READY,
	},
	{
		.vendor_id = MRVL_OUI,
		.subcmd =  MWL_VENDOR_EVENT_DRIVER_START_REMOVE,
	},
	{
		.vendor_id = MRVL_OUI,
		.subcmd =  MWL_VENDOR_EVENT_CMD_TIMEOUT,
	}
};

void vendor_cmd_register(struct wiphy *wiphy)
{
	wiphy->vendor_commands = mwl_vendor_commands;
	wiphy->n_vendor_commands = ARRAY_SIZE(mwl_vendor_commands);
	wiphy->vendor_events = mwl_vendor_events;
	wiphy->n_vendor_events = ARRAY_SIZE(mwl_vendor_events);
}

void vendor_cmd_basic_event(struct wiphy *wiphy, int event_idx)
{
	struct sk_buff *skb;

	skb = cfg80211_vendor_event_alloc(wiphy, NULL, 0,
					  event_idx, GFP_KERNEL);

	if (skb)
		cfg80211_vendor_event(skb, GFP_KERNEL);
}
