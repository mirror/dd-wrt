/**********************************************************************
 * Author: Cavium, Inc.
 *
 * Contact: support@cavium.com
 * This file is part of the OCTEON SDK
 *
 * Copyright (c) 2014 Cavium, Inc.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 *
 * This file is distributed in the hope that it will be useful, but
 * AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or
 * NONINFRINGEMENT.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 * or visit http://www.gnu.org/licenses/.
 *
 * This file may also be available under a different license from Cavium.
 * Contact Cavium, Inc. for more information
 **********************************************************************/
#ifndef _OCTEON_BGX_H_
#define _OCTEON_BGX_H_

#include <linux/ethtool.h>

struct device;
struct net_device;
struct bgx_port_priv;

struct bgx_port_netdev_priv {
	struct bgx_port_priv *bgx_priv;
};

void bgx_nexus_load(void);

void bgx_port_set_netdev(struct device *dev, struct net_device *netdev);
int bgx_port_enable(struct net_device *netdev);
int bgx_port_disable(struct net_device *netdev);
const u8 *bgx_port_get_mac(struct net_device *netdev);
void bgx_port_set_rx_filtering(struct net_device *netdev);
int bgx_port_change_mtu(struct net_device *netdev, int new_mtu);
int bgx_port_ethtool_get_settings(struct net_device *netdev,
				  struct ethtool_link_ksettings *cmd);
int bgx_port_ethtool_set_settings(struct net_device *netdev,
				  const struct ethtool_link_ksettings *cmd);
int bgx_port_ethtool_nway_reset(struct net_device *netdev);
int bgx_port_do_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd);

void bgx_port_mix_assert_reset(struct net_device *netdev, int mix, bool v);

#endif /* _OCTEON_BGX_H_ */
