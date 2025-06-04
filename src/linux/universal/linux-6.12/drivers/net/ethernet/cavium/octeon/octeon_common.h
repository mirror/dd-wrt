/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2013 Cavium, Inc.
 */
#ifndef _ETHERNET_OCTEON_OCTEON_COMMON_H
#define _ETHERNET_OCTEON_OCTEON_COMMON_H

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/if_vlan.h>

/* Allow 8 bytes for vlan and FCS. */
#define OCTEON_FRAME_HEADER_LEN	(ETH_HLEN + ETH_FCS_LEN + VLAN_HLEN)

#define GMX_PRT_CFG                 0x10

#define GMX_RX_FRM_MAX              0x30
#define GMX_RX_JABBER               0x38

#define GMX_RX_ADR_CTL              0x100
#define GMX_RX_ADR_CAM_EN           0x108
#define GMX_RX_ADR_CAM0             0x180
#define GMX_RX_ADR_CAM1             0x188
#define GMX_RX_ADR_CAM2             0x190
#define GMX_RX_ADR_CAM3             0x198
#define GMX_RX_ADR_CAM4             0x1a0
#define GMX_RX_ADR_CAM5             0x1a8

extern void cvm_oct_common_set_rx_filtering(struct net_device *dev, u64 base_reg,
					spinlock_t *lock);

extern int cvm_oct_common_set_mac_address(struct net_device *dev, void *addr,
					u64 base_reg, spinlock_t *lock);

extern int cvm_oct_common_change_mtu(struct net_device *dev, int mtu, u64 base_reg,
		u64 pip_reg, int mtu_limit);
#endif
