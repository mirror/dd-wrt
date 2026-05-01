/* SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef __PPE_PORT_H__
#define __PPE_PORT_H__

#include <linux/phylink.h>

struct ethtool_keee;
struct rtnl_link_stats64;

/**
 * enum ppe_port_clk_rst_type - PPE port clock and reset ID type
 * @PPE_PORT_CLK_RST_MAC: The clock and reset ID for port MAC
 * @PPE_PORT_CLK_RST_RX: The clock and reset ID for port receive path
 * @PPE_PORT_CLK_RST_TX: The clock and reset for port transmit path
 * @PPE_PORT_CLK_RST_MAX: The maximum of port clock and reset
 */
enum ppe_port_clk_rst_type {
	PPE_PORT_CLK_RST_MAC,
	PPE_PORT_CLK_RST_RX,
	PPE_PORT_CLK_RST_TX,
	PPE_PORT_CLK_RST_MAX,
};

/**
 * enum ppe_mac_type - PPE MAC type
 * @PPE_MAC_TYPE_GMAC: GMAC type
 * @PPE_MAC_TYPE_XGMAC: XGMAC type
 */
enum ppe_mac_type {
	PPE_MAC_TYPE_GMAC,
	PPE_MAC_TYPE_XGMAC,
};

/**
 * struct ppe_port - Private data for each PPE port
 * @phylink: Linux phylink instance
 * @phylink_config: Linux phylink configurations
 * @pcs: Linux phylink PCS instance
 * @np: Port device tree node
 * @ppe_dev: Back pointer to PPE device private data
 * @interface: Port interface mode
 * @mac_type: Port MAC type, GMAC or XGMAC
 * @port_id: Port ID
 * @clks: Port clocks
 * @rstcs: Port resets
 * @gmib_read: Delay work task for GMAC MIB statistics polling function
 * @gmib_stats: GMAC MIB statistics array
 * @gmib_stats_lock: Lock to protect GMAC MIB statistics
 */
struct ppe_port {
	struct phylink *phylink;
	struct phylink_config phylink_config;
	struct phylink_pcs *pcs;
	struct device_node *np;
	struct ppe_device *ppe_dev;
	phy_interface_t interface;
	enum ppe_mac_type mac_type;
	int port_id;
	struct clk *clks[PPE_PORT_CLK_RST_MAX];
	struct reset_control *rstcs[PPE_PORT_CLK_RST_MAX];
	struct delayed_work gmib_read;
	u64 *gmib_stats;
	spinlock_t gmib_stats_lock; /* Protects GMIB stats */
};

/**
 * struct ppe_ports - Array of PPE ports
 * @num: Number of PPE ports
 * @port: Each PPE port private data
 */
struct ppe_ports {
	unsigned int num;
	struct ppe_port port[] __counted_by(num);
};

int ppe_port_mac_init(struct ppe_device *ppe_dev);
void ppe_port_mac_deinit(struct ppe_device *ppe_dev);
int ppe_port_phylink_setup(struct ppe_port *ppe_port,
			   struct net_device *netdev);
void ppe_port_phylink_destroy(struct ppe_port *ppe_port);
int ppe_port_get_sset_count(struct ppe_port *ppe_port, int sset);
void ppe_port_get_strings(struct ppe_port *ppe_port, u32 stringset, u8 *data);
void ppe_port_get_ethtool_stats(struct ppe_port *ppe_port, u64 *data);
void ppe_port_get_stats64(struct ppe_port *ppe_port,
			  struct rtnl_link_stats64 *s);
int ppe_port_set_mac_address(struct ppe_port *ppe_port, const u8 *addr);
int ppe_port_set_mac_eee(struct ppe_port *ppe_port, struct ethtool_keee *eee);
int ppe_port_set_maxframe(struct ppe_port *ppe_port, int maxframe_size);
#endif
