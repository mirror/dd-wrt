// SPDX-License-Identifier: GPL-2.0 OR ISC
/* Copyright (c) 2015 - 2016, The Linux Foundation. All rights reserved.
 * Copyright (c) 2017 - 2018, John Crispin <john@phrozen.org>
 * Copyright (c) 2021 - 2022, Maxime Chevallier <maxime.chevallier@bootlin.com>
 *
 */

#include <linux/ethtool.h>
#include <linux/netdevice.h>
#include <linux/string.h>
#include <linux/phylink.h>

#include "ipqess.h"

struct ipqess_ethtool_stats {
	u8 string[ETH_GSTRING_LEN];
	u32 offset;
};

#define IPQESS_STAT(m)    offsetof(struct ipqess_statistics, m)
#define DRVINFO_LEN	32

static const struct ipqess_ethtool_stats ipqess_stats[] = {
	{"tx_q0_pkt", IPQESS_STAT(tx_q0_pkt)},
	{"tx_q1_pkt", IPQESS_STAT(tx_q1_pkt)},
	{"tx_q2_pkt", IPQESS_STAT(tx_q2_pkt)},
	{"tx_q3_pkt", IPQESS_STAT(tx_q3_pkt)},
	{"tx_q4_pkt", IPQESS_STAT(tx_q4_pkt)},
	{"tx_q5_pkt", IPQESS_STAT(tx_q5_pkt)},
	{"tx_q6_pkt", IPQESS_STAT(tx_q6_pkt)},
	{"tx_q7_pkt", IPQESS_STAT(tx_q7_pkt)},
	{"tx_q8_pkt", IPQESS_STAT(tx_q8_pkt)},
	{"tx_q9_pkt", IPQESS_STAT(tx_q9_pkt)},
	{"tx_q10_pkt", IPQESS_STAT(tx_q10_pkt)},
	{"tx_q11_pkt", IPQESS_STAT(tx_q11_pkt)},
	{"tx_q12_pkt", IPQESS_STAT(tx_q12_pkt)},
	{"tx_q13_pkt", IPQESS_STAT(tx_q13_pkt)},
	{"tx_q14_pkt", IPQESS_STAT(tx_q14_pkt)},
	{"tx_q15_pkt", IPQESS_STAT(tx_q15_pkt)},
	{"tx_q0_byte", IPQESS_STAT(tx_q0_byte)},
	{"tx_q1_byte", IPQESS_STAT(tx_q1_byte)},
	{"tx_q2_byte", IPQESS_STAT(tx_q2_byte)},
	{"tx_q3_byte", IPQESS_STAT(tx_q3_byte)},
	{"tx_q4_byte", IPQESS_STAT(tx_q4_byte)},
	{"tx_q5_byte", IPQESS_STAT(tx_q5_byte)},
	{"tx_q6_byte", IPQESS_STAT(tx_q6_byte)},
	{"tx_q7_byte", IPQESS_STAT(tx_q7_byte)},
	{"tx_q8_byte", IPQESS_STAT(tx_q8_byte)},
	{"tx_q9_byte", IPQESS_STAT(tx_q9_byte)},
	{"tx_q10_byte", IPQESS_STAT(tx_q10_byte)},
	{"tx_q11_byte", IPQESS_STAT(tx_q11_byte)},
	{"tx_q12_byte", IPQESS_STAT(tx_q12_byte)},
	{"tx_q13_byte", IPQESS_STAT(tx_q13_byte)},
	{"tx_q14_byte", IPQESS_STAT(tx_q14_byte)},
	{"tx_q15_byte", IPQESS_STAT(tx_q15_byte)},
	{"rx_q0_pkt", IPQESS_STAT(rx_q0_pkt)},
	{"rx_q1_pkt", IPQESS_STAT(rx_q1_pkt)},
	{"rx_q2_pkt", IPQESS_STAT(rx_q2_pkt)},
	{"rx_q3_pkt", IPQESS_STAT(rx_q3_pkt)},
	{"rx_q4_pkt", IPQESS_STAT(rx_q4_pkt)},
	{"rx_q5_pkt", IPQESS_STAT(rx_q5_pkt)},
	{"rx_q6_pkt", IPQESS_STAT(rx_q6_pkt)},
	{"rx_q7_pkt", IPQESS_STAT(rx_q7_pkt)},
	{"rx_q0_byte", IPQESS_STAT(rx_q0_byte)},
	{"rx_q1_byte", IPQESS_STAT(rx_q1_byte)},
	{"rx_q2_byte", IPQESS_STAT(rx_q2_byte)},
	{"rx_q3_byte", IPQESS_STAT(rx_q3_byte)},
	{"rx_q4_byte", IPQESS_STAT(rx_q4_byte)},
	{"rx_q5_byte", IPQESS_STAT(rx_q5_byte)},
	{"rx_q6_byte", IPQESS_STAT(rx_q6_byte)},
	{"rx_q7_byte", IPQESS_STAT(rx_q7_byte)},
	{"tx_desc_error", IPQESS_STAT(tx_desc_error)},
};

static int ipqess_get_strset_count(struct net_device *netdev, int sset)
{
	switch (sset) {
	case ETH_SS_STATS:
		return ARRAY_SIZE(ipqess_stats);
	default:
		netdev_dbg(netdev, "%s: Unsupported string set", __func__);
		return -EOPNOTSUPP;
	}
}

static void ipqess_get_strings(struct net_device *netdev, u32 stringset,
			       u8 *data)
{
	u8 *p = data;
	u32 i;

	switch (stringset) {
	case ETH_SS_STATS:
		for (i = 0; i < ARRAY_SIZE(ipqess_stats); i++)
			ethtool_puts(&p, ipqess_stats[i].string);
		break;
	}
}

static void ipqess_get_ethtool_stats(struct net_device *netdev,
				     struct ethtool_stats *stats,
				     uint64_t *data)
{
	struct ipqess *ess = netdev_priv(netdev);
	u32 *essstats = (u32 *)&ess->ipqess_stats;
	int i;

	spin_lock(&ess->stats_lock);

	ipqess_update_hw_stats(ess);

	for (i = 0; i < ARRAY_SIZE(ipqess_stats); i++)
		data[i] = *(u32 *)(essstats + (ipqess_stats[i].offset / sizeof(u32)));

	spin_unlock(&ess->stats_lock);
}

static void ipqess_get_drvinfo(struct net_device *dev,
			       struct ethtool_drvinfo *info)
{
	strscpy(info->driver, "qca_ipqess", DRVINFO_LEN);
	strscpy(info->bus_info, "axi", ETHTOOL_BUSINFO_LEN);
}

static int ipqess_get_link_ksettings(struct net_device *netdev,
				     struct ethtool_link_ksettings *cmd)
{
	struct ipqess *ess = netdev_priv(netdev);

	return phylink_ethtool_ksettings_get(ess->phylink, cmd);
}

static int ipqess_set_link_ksettings(struct net_device *netdev,
				     const struct ethtool_link_ksettings *cmd)
{
	struct ipqess *ess = netdev_priv(netdev);

	return phylink_ethtool_ksettings_set(ess->phylink, cmd);
}

static void ipqess_get_ringparam(struct net_device *netdev,
				 struct ethtool_ringparam *ring,
				 struct kernel_ethtool_ringparam *kernel_ering,
				 struct netlink_ext_ack *extack)
{
	ring->tx_max_pending = IPQESS_TX_RING_SIZE;
	ring->rx_max_pending = IPQESS_RX_RING_SIZE;
}

static const struct ethtool_ops ipqesstool_ops = {
	.get_drvinfo = &ipqess_get_drvinfo,
	.get_link = &ethtool_op_get_link,
	.get_link_ksettings = &ipqess_get_link_ksettings,
	.set_link_ksettings = &ipqess_set_link_ksettings,
	.get_strings = &ipqess_get_strings,
	.get_sset_count = &ipqess_get_strset_count,
	.get_ethtool_stats = &ipqess_get_ethtool_stats,
	.get_ringparam = ipqess_get_ringparam,
};

void ipqess_set_ethtool_ops(struct net_device *netdev)
{
	netdev->ethtool_ops = &ipqesstool_ops;
}
