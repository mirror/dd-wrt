/* SPDX-License-Identifier: GPL-2.0-only */
/*
################################################################################
#
# r8127 is the Linux device driver released for Realtek 10 Gigabit Ethernet
# controllers with PCI-Express interface.
#
# Copyright(c) 2025 Realtek Semiconductor Corp. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, see <http://www.gnu.org/licenses/>.
#
# Author:
# Realtek NIC software team <nicfae@realtek.com>
# No. 2, Innovation Road II, Hsinchu Science Park, Hsinchu 300, Taiwan
#
################################################################################
*/

/************************************************************************************
 *  This product is covered by one or more of the following patents:
 *  US6,570,884, US6,115,776, and US6,327,625.
 ***********************************************************************************/

#ifndef _LINUX_R8127_RSS_H
#define _LINUX_R8127_RSS_H

#include <linux/netdevice.h>
#include <linux/types.h>

#define RTL8127_RSS_KEY_SIZE     40  /* size of RSS Hash Key in bytes */
#define RTL8127_MAX_INDIRECTION_TABLE_ENTRIES 128

enum rtl8127_rss_flag {
        RTL_8125_RSS_FLAG_HASH_UDP_IPV4  = (1 << 0),
        RTL_8125_RSS_FLAG_HASH_UDP_IPV6  = (1 << 1),
};

struct rtl8127_private;
struct RxDesc;

int rtl8127_get_rxnfc(struct net_device *dev, struct ethtool_rxnfc *cmd,
                      u32 *rule_locs);
int rtl8127_set_rxnfc(struct net_device *dev, struct ethtool_rxnfc *cmd);
u32 rtl8127_get_rxfh_key_size(struct net_device *netdev);
u32 rtl8127_rss_indir_size(struct net_device *netdev);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,8,0)
int rtl8127_get_rxfh(struct net_device *dev, struct ethtool_rxfh_param *rxfh);
int rtl8127_set_rxfh(struct net_device *dev, struct ethtool_rxfh_param *rxfh,
                     struct netlink_ext_ack *extack);
#else
int rtl8127_get_rxfh(struct net_device *netdev, u32 *indir, u8 *key,
                     u8 *hfunc);
int rtl8127_set_rxfh(struct net_device *netdev, const u32 *indir,
                     const u8 *key, const u8 hfunc);
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(6,8,0) */
void rtl8127_rx_hash(struct rtl8127_private *tp,
                     struct RxDesc *desc,
                     struct sk_buff *skb);
void _rtl8127_config_rss(struct rtl8127_private *tp);
void rtl8127_config_rss(struct rtl8127_private *tp);
void rtl8127_init_rss(struct rtl8127_private *tp);
u32 rtl8127_rss_indir_tbl_entries(struct rtl8127_private *tp);
void rtl8127_disable_rss(struct rtl8127_private *tp);

#endif /* _LINUX_R8127_RSS_H */
