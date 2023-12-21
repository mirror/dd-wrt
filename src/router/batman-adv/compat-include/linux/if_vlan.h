/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_IF_VLAN_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_IF_VLAN_H_

#include <linux/version.h>
#include_next <linux/if_vlan.h>

#if LINUX_VERSION_IS_LESS(5, 10, 0)

/* Prefer this version in TX path, instead of
 * skb_reset_mac_header() + vlan_eth_hdr()
 */
static inline struct vlan_ethhdr *skb_vlan_eth_hdr(const struct sk_buff *skb)
{
	return (struct vlan_ethhdr *)skb->data;
}

#endif /* LINUX_VERSION_IS_LESS(6, 4, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_IF_VLAN_H_ */
