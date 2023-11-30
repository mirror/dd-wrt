/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_ETHERDEVICE_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_ETHERDEVICE_H_

#include <linux/version.h>
#include_next <linux/etherdevice.h>

#if LINUX_VERSION_IS_LESS(5, 15, 0)

static inline void batadv_eth_hw_addr_set(struct net_device *dev,
					  const u8 *addr)
{
	ether_addr_copy(dev->dev_addr, addr);
}
#define eth_hw_addr_set batadv_eth_hw_addr_set

#endif /* LINUX_VERSION_IS_LESS(5, 15, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_ETHERDEVICE_H_ */
