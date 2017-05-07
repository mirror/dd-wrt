/*
 * phylink models the MAC to optional PHY connection, supporting
 * technologies such as SFP cages where the PHY is hot-pluggable.
 *
 * Copyright (C) 2015 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/ethtool.h>
#include <linux/export.h>
#include <linux/gpio/consumer.h>
#include <linux/list.h>
#include <linux/netdevice.h>
#include <linux/of.h>
#include <linux/of_mdio.h>
#include <linux/phy.h>
#include <linux/phy_fixed.h>
#include <linux/phylink.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>

#include "swphy.h"

#define SUPPORTED_INTERFACES \
	(SUPPORTED_TP | SUPPORTED_MII | SUPPORTED_FIBRE | \
	 SUPPORTED_BNC | SUPPORTED_AUI | SUPPORTED_Backplane)
#define ADVERTISED_INTERFACES \
	(ADVERTISED_TP | ADVERTISED_MII | ADVERTISED_FIBRE | \
	 ADVERTISED_BNC | ADVERTISED_AUI | ADVERTISED_Backplane)

static LIST_HEAD(phylinks);
static DEFINE_MUTEX(phylink_mutex);

enum {
	PHYLINK_DISABLE_STOPPED,
	PHYLINK_DISABLE_LINK,
};

struct phylink {
	struct list_head node;
	struct net_device *netdev;
	const struct phylink_mac_ops *ops;
	struct mutex config_mutex;

	unsigned long phylink_disable_state; /* bitmask of disables */
	struct phy_device *phydev;
	phy_interface_t link_interface;	/* PHY_INTERFACE_xxx */
	u8 link_an_mode;		/* MLO_AN_xxx */
	u8 link_port;			/* The current non-phy ethtool port */
	/* ethtool supported mask for ports */
	__ETHTOOL_DECLARE_LINK_MODE_MASK(supported);

	/* The link configuration settings */
	struct phylink_link_state link_config;
	struct gpio_desc *link_gpio;

	struct mutex state_mutex;	/* may be taken within config_mutex */
	struct phylink_link_state phy_state;
	struct work_struct resolve;

	bool mac_link_dropped;

	const struct phylink_module_ops *module_ops;
	void *module_data;
};

static inline void linkmode_zero(unsigned long *dst)
{
	bitmap_zero(dst, __ETHTOOL_LINK_MODE_MASK_NBITS);
}

static inline void linkmode_copy(unsigned long *dst, const unsigned long *src)
{
	bitmap_copy(dst, src, __ETHTOOL_LINK_MODE_MASK_NBITS);
}

static inline void linkmode_and(unsigned long *dst, const unsigned long *a,
				const unsigned long *b)
{
	bitmap_and(dst, a, b, __ETHTOOL_LINK_MODE_MASK_NBITS);
}

static inline void linkmode_or(unsigned long *dst, const unsigned long *a,
				const unsigned long *b)
{
	bitmap_or(dst, a, b, __ETHTOOL_LINK_MODE_MASK_NBITS);
}

static inline bool linkmode_empty(const unsigned long *src)
{
	return bitmap_empty(src, __ETHTOOL_LINK_MODE_MASK_NBITS);
}

static void phylink_set_port_bits(unsigned long *bits)
{
	__set_bit(ETHTOOL_LINK_MODE_TP_BIT, bits);
	__set_bit(ETHTOOL_LINK_MODE_AUI_BIT, bits);
	__set_bit(ETHTOOL_LINK_MODE_MII_BIT, bits);
	__set_bit(ETHTOOL_LINK_MODE_FIBRE_BIT, bits);
	__set_bit(ETHTOOL_LINK_MODE_BNC_BIT, bits);
	__set_bit(ETHTOOL_LINK_MODE_Backplane_BIT, bits);
}

static const char *phylink_an_mode_str(unsigned int mode)
{
	static const char *modestr[] = {
		[MLO_AN_PHY] = "phy",
		[MLO_AN_FIXED] = "fixed",
		[MLO_AN_SGMII] = "SGMII",
		[MLO_AN_8023Z] = "802.3z",
	};

	return mode < ARRAY_SIZE(modestr) ? modestr[mode] : "unknown";
}

static int phylink_parse_fixedlink(struct phylink *pl, struct device_node *np)
{
	__ETHTOOL_DECLARE_LINK_MODE_MASK(mask) = { 0, };
	struct device_node *fixed_node;
	const struct phy_setting *s;
	struct gpio_desc *desc;
	const __be32 *fixed_prop;
	u32 speed;
	int ret, len;

	fixed_node = of_get_child_by_name(np, "fixed-link");
	if (fixed_node) {
		ret = of_property_read_u32(fixed_node, "speed", &speed);

		pl->link_config.speed = speed;
		pl->link_config.duplex = DUPLEX_HALF;

		if (of_property_read_bool(fixed_node, "full-duplex"))
			pl->link_config.duplex = DUPLEX_FULL;

		/* We treat the "pause" and "asym-pause" terminology as
		 * defining the link partner's ability. */
		if (of_property_read_bool(fixed_node, "pause"))
			pl->link_config.pause |= MLO_PAUSE_SYM;
		if (of_property_read_bool(fixed_node, "asym-pause"))
			pl->link_config.pause |= MLO_PAUSE_ASYM;

		if (ret == 0) {
			desc = fwnode_get_named_gpiod(&fixed_node->fwnode,
						      "link-gpios");

			if (!IS_ERR(desc))
				pl->link_gpio = desc;
			else if (desc == ERR_PTR(-EPROBE_DEFER))
				ret = -EPROBE_DEFER;
		}
		of_node_put(fixed_node);

		if (ret)
			return ret;
	} else {
		fixed_prop = of_get_property(np, "fixed-link", &len);
		if (!fixed_prop) {
			netdev_err(pl->netdev, "broken fixed-link?\n");
			return -EINVAL;
		}
		if (len == 5 * sizeof(*fixed_prop)) {
			pl->link_config.duplex = be32_to_cpu(fixed_prop[1]) ?
						DUPLEX_FULL : DUPLEX_HALF;
			pl->link_config.speed = be32_to_cpu(fixed_prop[2]);
			if (be32_to_cpu(fixed_prop[3]))
				pl->link_config.pause |= MLO_PAUSE_SYM;
			if (be32_to_cpu(fixed_prop[4]))
				pl->link_config.pause |= MLO_PAUSE_ASYM;
		}
	}

	bitmap_fill(mask, __ETHTOOL_LINK_MODE_MASK_NBITS);
	pl->ops->validate_support(pl->netdev, MLO_AN_FIXED, mask);

	pl->link_config.link = 1;
	pl->link_config.an_complete = 1;

	if (pl->link_config.speed > SPEED_1000 &&
	    pl->link_config.duplex != DUPLEX_FULL)
		netdev_warn(pl->netdev, "fixed link specifies half duplex for %dMbps link?\n",
			    pl->link_config.speed);

	s = phy_lookup_setting(pl->link_config.speed, pl->link_config.duplex,
				mask, __ETHTOOL_LINK_MODE_MASK_NBITS, true);
	if (s) {
		__set_bit(s->bit, pl->supported);
	} else {
		netdev_warn(pl->netdev, "fixed link %s duplex %dMbps not recognised\n",
			    pl->link_config.duplex == DUPLEX_FULL ? "full" : "half",
			    pl->link_config.speed);
	}
	return 0;
}

static int phylink_parse_mode(struct phylink *pl, struct device_node *np)
{
	struct device_node *dn;
	const char *managed;

	dn = of_get_child_by_name(np, "fixed-link");
	if (dn || of_find_property(np, "fixed-link", NULL))
		pl->link_an_mode = MLO_AN_FIXED;
	of_node_put(dn);

	if (of_property_read_string(np, "managed", &managed) == 0 &&
	    strcmp(managed, "in-band-status") == 0) {
		if (pl->link_an_mode == MLO_AN_FIXED) {
			netdev_err(pl->netdev,
				   "can't use both fixed-link and in-band-status\n");
			return -EINVAL;
		}
		phylink_set(pl->supported, 10baseT_Half);
		phylink_set(pl->supported, 10baseT_Full);
		phylink_set(pl->supported, 100baseT_Half);
		phylink_set(pl->supported, 100baseT_Full);
		phylink_set(pl->supported, 1000baseT_Half);
		phylink_set(pl->supported, 1000baseT_Full);
		phylink_set(pl->supported, Asym_Pause);
		phylink_set(pl->supported, Pause);
		pl->link_an_mode = MLO_AN_SGMII;
		pl->link_config.an_enabled = true;
		pl->ops->validate_support(pl->netdev, pl->link_an_mode,
					  pl->supported);
	}

	return 0;
}


static void phylink_init_advert(struct phylink *pl, unsigned int mode,
				const unsigned long *supported,
				unsigned long *advertising)
{
	linkmode_copy(advertising, supported);
	if (pl->ops->validate_advert)
		pl->ops->validate_advert(pl->netdev, mode, supported,
					 advertising);
}

static void phylink_mac_config(struct phylink *pl,
			       const struct phylink_link_state *state)
{
	netdev_dbg(pl->netdev,
		   "%s: mode=%s/%s/%s/%s adv=%*pb pause=%02x link=%u an=%u\n",
		   __func__, phylink_an_mode_str(pl->link_an_mode),
		   phy_modes(state->interface),
		   phy_speed_to_str(state->speed),
		   phy_duplex_to_str(state->duplex),
		   __ETHTOOL_LINK_MODE_MASK_NBITS, state->advertising,
		   state->pause, state->link, state->an_enabled);

	pl->ops->mac_config(pl->netdev, pl->link_an_mode, state);
}

static void phylink_mac_an_restart(struct phylink *pl)
{
	if (pl->link_config.an_enabled)
		pl->ops->mac_an_restart(pl->netdev, pl->link_an_mode);
}

static int phylink_get_mac_state(struct phylink *pl, struct phylink_link_state *state)
{
	struct net_device *ndev = pl->netdev;

	linkmode_copy(state->advertising, pl->link_config.advertising);
	linkmode_zero(state->lp_advertising);
	state->interface = pl->link_config.interface;
	state->an_enabled = pl->link_config.an_enabled;
	state->link = 1;

	return pl->ops->mac_link_state(ndev, state);
}

/* The fixed state is... fixed except for the link state,
 * which may be determined by a GPIO.
 */
static void phylink_get_fixed_state(struct phylink *pl, struct phylink_link_state *state)
{
	*state = pl->link_config;
	if (pl->link_gpio)
		state->link = !!gpiod_get_value(pl->link_gpio);
}

/* Flow control is resolved according to our and the link partners
 * advertisments using the following drawn from the 802.3 specs:
 *  Local device  Link partner
 *  Pause AsymDir Pause AsymDir Result
 *    1     X       1     X     TX+RX
 *    0     1       1     1     RX
 *    1     1       0     1     TX
 */
static void phylink_resolve_flow(struct phylink *pl,
	struct phylink_link_state *state)
{
	int new_pause = 0;

	if (pl->link_config.pause & MLO_PAUSE_AN) {
		int pause = 0;

		if (phylink_test(pl->link_config.advertising, Pause))
			pause |= MLO_PAUSE_SYM;
		if (phylink_test(pl->link_config.advertising, Asym_Pause))
			pause |= MLO_PAUSE_ASYM;

		pause &= state->pause;

		if (pause & MLO_PAUSE_SYM)
			new_pause = MLO_PAUSE_TX | MLO_PAUSE_RX;
		else if (pause & MLO_PAUSE_ASYM)
			new_pause = state->pause & MLO_PAUSE_SYM ?
				 MLO_PAUSE_RX : MLO_PAUSE_TX;
	} else {
		new_pause = pl->link_config.pause & MLO_PAUSE_TXRX_MASK;
	}

	state->pause &= ~MLO_PAUSE_TXRX_MASK;
	state->pause |= new_pause;
}

static const char *phylink_pause_to_str(int pause)
{
	switch (pause & MLO_PAUSE_TXRX_MASK) {
	case MLO_PAUSE_TX | MLO_PAUSE_RX:
		return "rx/tx";
	case MLO_PAUSE_TX:
		return "tx";
	case MLO_PAUSE_RX:
		return "rx";
	default:
		return "off";
	}
}

static void phylink_resolve(struct work_struct *w)
{
	struct phylink *pl = container_of(w, struct phylink, resolve);
	struct phylink_link_state link_state;
	struct net_device *ndev = pl->netdev;

	mutex_lock(&pl->state_mutex);
	if (pl->phylink_disable_state) {
		pl->mac_link_dropped = false;
		link_state.link = false;
	} else if (pl->mac_link_dropped) {
		link_state.link = false;
	} else {
		switch (pl->link_an_mode) {
		case MLO_AN_PHY:
			link_state = pl->phy_state;
			phylink_resolve_flow(pl, &link_state);
			phylink_mac_config(pl, &link_state);
			break;

		case MLO_AN_FIXED:
			phylink_get_fixed_state(pl, &link_state);
			phylink_mac_config(pl, &link_state);
			break;

		case MLO_AN_SGMII:
			phylink_get_mac_state(pl, &link_state);
			if (pl->phydev) {
				bool changed = false;

				link_state.link = link_state.link &&
						  pl->phy_state.link;

				if (pl->phy_state.interface !=
				    link_state.interface) {
					link_state.interface = pl->phy_state.interface;
					changed = true;
				}

				/* Propagate the flow control from the PHY
				 * to the MAC. Also propagate the interface
				 * if changed.
				 */
				if (pl->phy_state.link || changed) {
					link_state.pause |= pl->phy_state.pause;
					phylink_resolve_flow(pl, &link_state);

					phylink_mac_config(pl, &link_state);
				}
			}
			break;

		case MLO_AN_8023Z:
			phylink_get_mac_state(pl, &link_state);
			break;
		}
	}

	if (link_state.link != netif_carrier_ok(ndev)) {
		if (!link_state.link) {
			netif_carrier_off(ndev);
			pl->ops->mac_link_down(ndev, pl->link_an_mode);
			netdev_info(ndev, "Link is Down\n");
		} else {
			pl->ops->mac_link_up(ndev, pl->link_an_mode,
					     pl->phydev);

			netif_carrier_on(ndev);

			netdev_info(ndev,
				    "Link is Up - %s/%s - flow control %s\n",
				    phy_speed_to_str(link_state.speed),
				    phy_duplex_to_str(link_state.duplex),
				    phylink_pause_to_str(link_state.pause));
		}
	}
	if (!link_state.link && pl->mac_link_dropped) {
		pl->mac_link_dropped = false;
		queue_work(system_power_efficient_wq, &pl->resolve);
	}
	mutex_unlock(&pl->state_mutex);
}

static void phylink_run_resolve(struct phylink *pl)
{
	if (!pl->phylink_disable_state)
		queue_work(system_power_efficient_wq, &pl->resolve);
}

struct phylink *phylink_create(struct net_device *ndev, struct device_node *np,
	phy_interface_t iface, const struct phylink_mac_ops *ops)
{
	struct phylink *pl;
	int ret;

	pl = kzalloc(sizeof(*pl), GFP_KERNEL);
	if (!pl)
		return ERR_PTR(-ENOMEM);

	mutex_init(&pl->state_mutex);
	mutex_init(&pl->config_mutex);
	INIT_WORK(&pl->resolve, phylink_resolve);
	pl->netdev = ndev;
	pl->phy_state.interface = iface;
	pl->link_interface = iface;
	pl->link_port = PORT_MII;
	pl->link_config.interface = iface;
	pl->link_config.pause = MLO_PAUSE_AN;
	pl->link_config.speed = SPEED_UNKNOWN;
	pl->link_config.duplex = DUPLEX_UNKNOWN;
	pl->ops = ops;
	__set_bit(PHYLINK_DISABLE_STOPPED, &pl->phylink_disable_state);

	ret = phylink_parse_mode(pl, np);
	if (ret < 0) {
		kfree(pl);
		return ERR_PTR(ret);
	}

	if (pl->link_an_mode == MLO_AN_FIXED) {
		ret = phylink_parse_fixedlink(pl, np);
		if (ret < 0) {
			kfree(pl);
			return ERR_PTR(ret);
		}
	}

	phylink_set(pl->supported, MII);
	phylink_init_advert(pl, pl->link_an_mode, pl->supported,
			    pl->link_config.advertising);

	mutex_lock(&phylink_mutex);
	list_add_tail(&pl->node, &phylinks);
	mutex_unlock(&phylink_mutex);

	return pl;
}
EXPORT_SYMBOL_GPL(phylink_create);

void phylink_destroy(struct phylink *pl)
{
	mutex_lock(&phylink_mutex);
	list_del(&pl->node);
	mutex_unlock(&phylink_mutex);

	cancel_work_sync(&pl->resolve);
	kfree(pl);
}
EXPORT_SYMBOL_GPL(phylink_destroy);

void phylink_phy_change(struct phy_device *phydev, bool up, bool do_carrier)
{
	struct phylink *pl = phydev->phylink;

	mutex_lock(&pl->state_mutex);
	pl->phy_state.speed = phydev->speed;
	pl->phy_state.duplex = phydev->duplex;
	pl->phy_state.pause = MLO_PAUSE_NONE;
	if (phydev->pause)
		pl->phy_state.pause |= MLO_PAUSE_SYM;
	if (phydev->asym_pause)
		pl->phy_state.pause |= MLO_PAUSE_ASYM;
	pl->phy_state.interface = phydev->interface;
	pl->phy_state.link = up;
	mutex_unlock(&pl->state_mutex);

	phylink_run_resolve(pl);

	netdev_dbg(pl->netdev, "phy link %s %s/%s/%s\n", up ? "up" : "down",
	           phy_modes(phydev->interface),
		   phy_speed_to_str(phydev->speed),
		   phy_duplex_to_str(phydev->duplex));
}

static int phylink_empty_linkmode(const unsigned long *linkmode)
{
	__ETHTOOL_DECLARE_LINK_MODE_MASK(tmp) = { 0, };

	phylink_set_port_bits(tmp);
	phylink_set(tmp, Autoneg);
	phylink_set(tmp, Pause);
	phylink_set(tmp, Asym_Pause);

	bitmap_andnot(tmp, linkmode, tmp, __ETHTOOL_LINK_MODE_MASK_NBITS);

	return linkmode_empty(tmp);
}

static int phylink_validate_support(struct phylink *pl, int mode,
				    unsigned long *mask)
{
	pl->ops->validate_support(pl->netdev, mode, mask);

	return phylink_empty_linkmode(mask) ? -EINVAL : 0;
}

static int phylink_bringup_phy(struct phylink *pl, struct phy_device *phy)
{
	__ETHTOOL_DECLARE_LINK_MODE_MASK(mask);
	u32 advertising;
	int ret;

	ethtool_convert_legacy_u32_to_link_mode(mask, phy->supported);
	ret = phylink_validate_support(pl, pl->link_an_mode, mask);
	if (ret)
		return ret;

	mutex_lock(&pl->config_mutex);
	phy->phylink = pl;
	phy->phy_link_change = phylink_phy_change;

	netdev_info(pl->netdev,
		    "PHY [%s] driver [%s]\n", dev_name(&phy->mdio.dev),
		    phy->drv->name);

	mutex_lock(&pl->state_mutex);
	pl->phydev = phy;
	linkmode_copy(pl->supported, mask);

	/* Restrict the phy advertisment according to the MAC support. */
	ethtool_convert_link_mode_to_legacy_u32(&advertising, mask);
	phy->advertising &= ADVERTISED_INTERFACES | advertising;
	ethtool_convert_legacy_u32_to_link_mode(pl->link_config.advertising,
						phy->advertising);
	mutex_unlock(&pl->state_mutex);

	netdev_dbg(pl->netdev,
		   "phy: setting supported %*pb advertising 0x%08x\n",
		   __ETHTOOL_LINK_MODE_MASK_NBITS, pl->supported,
		   phy->advertising);

	phy_start_machine(phy);
	if (phy->irq > 0)
		phy_start_interrupts(phy);

	mutex_unlock(&pl->config_mutex);

	return 0;
}

int phylink_connect_phy(struct phylink *pl, struct phy_device *phy)
{
	int ret;

	ret = phy_attach_direct(pl->netdev, phy, 0, pl->link_interface);
	if (ret)
		return ret;

	ret = phylink_bringup_phy(pl, phy);
	if (ret)
		phy_detach(phy);

	return ret;
}
EXPORT_SYMBOL_GPL(phylink_connect_phy);

int phylink_of_phy_connect(struct phylink *pl, struct device_node *dn)
{
	struct device_node *phy_node;
	struct phy_device *phy_dev;
	int ret;

	/* Fixed links are handled without needing a PHY */
	if (pl->link_an_mode == MLO_AN_FIXED)
		return 0;

	phy_node = of_parse_phandle(dn, "phy-handle", 0);
	if (!phy_node)
		phy_node = of_parse_phandle(dn, "phy", 0);
	if (!phy_node)
		phy_node = of_parse_phandle(dn, "phy-device", 0);

	if (!phy_node) {
		if (pl->link_an_mode == MLO_AN_PHY) {
			netdev_err(pl->netdev, "unable to find PHY node\n");
			return -ENODEV;
		}
		return 0;
	}

	phy_dev = of_phy_attach(pl->netdev, phy_node, 0, pl->link_interface);
	/* We're done with the phy_node handle */
	of_node_put(phy_node);

	if (!phy_dev)
		return -ENODEV;

	ret = phylink_bringup_phy(pl, phy_dev);
	if (ret)
		phy_detach(phy_dev);

	return ret;
}
EXPORT_SYMBOL_GPL(phylink_of_phy_connect);

void phylink_disconnect_phy(struct phylink *pl)
{
	struct phy_device *phy;

	mutex_lock(&pl->config_mutex);
	phy = pl->phydev;

	mutex_lock(&pl->state_mutex);
	pl->phydev = NULL;
	mutex_unlock(&pl->state_mutex);
	flush_work(&pl->resolve);

	if (phy)
		phy_disconnect(phy);

	mutex_unlock(&pl->config_mutex);
}
EXPORT_SYMBOL_GPL(phylink_disconnect_phy);

void phylink_mac_change(struct phylink *pl, bool up)
{
	if (!up)
		pl->mac_link_dropped = true;
	phylink_run_resolve(pl);
	netdev_dbg(pl->netdev, "mac link %s\n", up ? "up" : "down");
}
EXPORT_SYMBOL_GPL(phylink_mac_change);

void phylink_start(struct phylink *pl)
{
	mutex_lock(&pl->config_mutex);

	netdev_info(pl->netdev, "configuring for %s link mode\n",
		    phylink_an_mode_str(pl->link_an_mode));

	/* Apply the link configuration to the MAC when starting. This allows
	 * a fixed-link to start with the correct parameters, and also
	 * ensures that we set the appropriate advertisment for Serdes links.
	 */
	phylink_resolve_flow(pl, &pl->link_config);
	phylink_mac_config(pl, &pl->link_config);

	clear_bit(PHYLINK_DISABLE_STOPPED, &pl->phylink_disable_state);
	phylink_run_resolve(pl);

	if (pl->phydev)
		phy_start(pl->phydev);

	mutex_unlock(&pl->config_mutex);
}
EXPORT_SYMBOL_GPL(phylink_start);

void phylink_stop(struct phylink *pl)
{
	mutex_lock(&pl->config_mutex);

	if (pl->phydev)
		phy_stop(pl->phydev);

	set_bit(PHYLINK_DISABLE_STOPPED, &pl->phylink_disable_state);
	flush_work(&pl->resolve);

	mutex_unlock(&pl->config_mutex);
}
EXPORT_SYMBOL_GPL(phylink_stop);

static void phylink_merge_link_mode(unsigned long *dst, const unsigned long *b)
{
	__ETHTOOL_DECLARE_LINK_MODE_MASK(mask);

	linkmode_zero(mask);
	phylink_set_port_bits(mask);

	linkmode_and(dst, dst, mask);
	linkmode_or(dst, dst, b);
}

static void phylink_get_ksettings(const struct phylink_link_state *state,
				  struct ethtool_link_ksettings *kset)
{
	phylink_merge_link_mode(kset->link_modes.advertising, state->advertising);
	linkmode_copy(kset->link_modes.lp_advertising, state->lp_advertising);
	kset->base.speed = state->speed;
	kset->base.duplex = state->duplex;
	kset->base.autoneg = state->an_enabled ? AUTONEG_ENABLE :
				AUTONEG_DISABLE;
}

static int __phylink_ethtool_ksettings_get(struct phylink *pl,
					   struct ethtool_link_ksettings *kset)
{
	struct phylink_link_state link_state;
	int ret;

	if (pl->phydev) {
		ret = phy_ethtool_ksettings_get(pl->phydev, kset);
		if (ret)
			return ret;
	} else {
		kset->base.port = pl->link_port;
	}

	linkmode_copy(kset->link_modes.supported, pl->supported);

	switch (pl->link_an_mode) {
	case MLO_AN_FIXED:
		/* We are using fixed settings. Report these as the
		 * current link settings - and note that these also
		 * represent the supported speeds/duplex/pause modes.
		 */
		phylink_get_fixed_state(pl, &link_state);
		phylink_get_ksettings(&link_state, kset);
		break;

	case MLO_AN_SGMII:
		/* If there is a phy attached, then use the reported
		 * settings from the phy with no modification.
		 */
		if (pl->phydev)
			break;

	case MLO_AN_8023Z:
		phylink_get_mac_state(pl, &link_state);

		/* The MAC is reporting the link results from its own PCS
		 * layer via in-band status. Report these as the current
		 * link settings.
		 */
		phylink_get_ksettings(&link_state, kset);
		break;
	}

	return 0;
}

int phylink_ethtool_ksettings_get(struct phylink *pl,
	struct ethtool_link_ksettings *kset)
{
	int ret;

	mutex_lock(&pl->config_mutex);
	ret = __phylink_ethtool_ksettings_get(pl, kset);
	mutex_unlock(&pl->config_mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(phylink_ethtool_ksettings_get);

static int __phylink_ethtool_ksettings_set(struct phylink *pl,
					   const struct ethtool_link_ksettings *kset)
{
	struct ethtool_link_ksettings our_kset = *kset;
	int ret;

	/* Mask out unsupported advertisments */
	linkmode_and(our_kset.link_modes.advertising,
		     kset->link_modes.advertising, pl->supported);

	if (pl->ops->validate_advert)
		pl->ops->validate_advert(pl->netdev, pl->link_an_mode,
					 pl->supported,
					 our_kset.link_modes.advertising);

	/* FIXME: should we reject autoneg if phy/mac does not support it? */

	if (kset->base.autoneg == AUTONEG_DISABLE) {
		const struct phy_setting *s;

		/* Autonegotiation disabled, select a suitable speed and
		 * duplex.
		 */
		s = phy_lookup_setting(kset->base.speed, kset->base.duplex,
				       pl->supported,
				       __ETHTOOL_LINK_MODE_MASK_NBITS, false);
		if (!s)
			return -EINVAL;

		/* If we have a fixed link (as specified by firmware), refuse
		 * to change link parameters.
		 */
		if (pl->link_an_mode == MLO_AN_FIXED &&
		    (s->speed != pl->link_config.speed ||
		     s->duplex != pl->link_config.duplex))
			return -EINVAL;

		our_kset.base.speed = s->speed;
		our_kset.base.duplex = s->duplex;

		__clear_bit(ETHTOOL_LINK_MODE_Autoneg_BIT,
			    our_kset.link_modes.advertising);
	} else {
		/* If we have a fixed link, refuse to enable autonegotiation */
		if (pl->link_an_mode == MLO_AN_FIXED)
			return -EINVAL;

		/* Autonegotiation enabled, validate advertisment */
		if (phylink_empty_linkmode(our_kset.link_modes.advertising))
			return -EINVAL;

		__set_bit(ETHTOOL_LINK_MODE_Autoneg_BIT,
			  our_kset.link_modes.advertising);
	}

	/* If we have a PHY, configure the phy */
	if (pl->phydev) {
		ret = phy_ethtool_ksettings_set(pl->phydev, &our_kset);
		if (ret)
			return ret;
	}

	mutex_lock(&pl->state_mutex);
	/* Configure the MAC to match the new settings */
	linkmode_copy(pl->link_config.advertising, our_kset.link_modes.advertising);
	pl->link_config.speed = our_kset.base.speed;
	pl->link_config.duplex = our_kset.base.duplex;
	pl->link_config.an_enabled = our_kset.base.autoneg != AUTONEG_DISABLE;

	if (!test_bit(PHYLINK_DISABLE_STOPPED, &pl->phylink_disable_state)) {
		phylink_mac_config(pl, &pl->link_config);
		phylink_mac_an_restart(pl);
	}
	mutex_unlock(&pl->state_mutex);

	return ret;
}

int phylink_ethtool_ksettings_set(struct phylink *pl,
	const struct ethtool_link_ksettings *kset)
{
	int ret;

	if (kset->base.autoneg != AUTONEG_DISABLE &&
	    kset->base.autoneg != AUTONEG_ENABLE)
		return -EINVAL;

	mutex_lock(&pl->config_mutex);
	ret = __phylink_ethtool_ksettings_set(pl, kset);
	mutex_unlock(&pl->config_mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(phylink_ethtool_ksettings_set);

int phylink_ethtool_nway_reset(struct phylink *pl)
{
	int ret = 0;

	mutex_lock(&pl->config_mutex);
	if (pl->phydev)
		ret = genphy_restart_aneg(pl->phydev);
	phylink_mac_an_restart(pl);
	mutex_unlock(&pl->config_mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(phylink_ethtool_nway_reset);

void phylink_ethtool_get_pauseparam(struct phylink *pl,
				    struct ethtool_pauseparam *pause)
{
	mutex_lock(&pl->config_mutex);

	pause->autoneg = !!(pl->link_config.pause & MLO_PAUSE_AN);
	pause->rx_pause = !!(pl->link_config.pause & MLO_PAUSE_RX);
	pause->tx_pause = !!(pl->link_config.pause & MLO_PAUSE_TX);

	mutex_unlock(&pl->config_mutex);
}
EXPORT_SYMBOL_GPL(phylink_ethtool_get_pauseparam);

static int __phylink_ethtool_set_pauseparam(struct phylink *pl,
					    struct ethtool_pauseparam *pause)
{
	struct phylink_link_state *config = &pl->link_config;

	if (!phylink_test(pl->supported, Pause) &&
	    !phylink_test(pl->supported, Asym_Pause))
		return -EOPNOTSUPP;

	if (!phylink_test(pl->supported, Asym_Pause) &&
	    !pause->autoneg && pause->rx_pause != pause->tx_pause)
		return -EINVAL;

	config->pause &= ~(MLO_PAUSE_AN | MLO_PAUSE_TXRX_MASK);

	if (pause->autoneg)
		config->pause |= MLO_PAUSE_AN;
	if (pause->rx_pause)
		config->pause |= MLO_PAUSE_RX;
	if (pause->tx_pause)
		config->pause |= MLO_PAUSE_TX;

	if (!test_bit(PHYLINK_DISABLE_STOPPED, &pl->phylink_disable_state)) {
		switch (pl->link_an_mode) {
		case MLO_AN_PHY:
			/* Silently mark the carrier down, and then trigger a resolve */
			netif_carrier_off(pl->netdev);
			phylink_run_resolve(pl);
			break;

		case MLO_AN_FIXED:
			/* Should we allow fixed links to change against the config? */
			phylink_resolve_flow(pl, config);
			phylink_mac_config(pl, config);
			break;

		case MLO_AN_SGMII:
		case MLO_AN_8023Z:
			phylink_mac_config(pl, config);
			phylink_mac_an_restart(pl);
			break;
		}
	}

	return 0;
}

int phylink_ethtool_set_pauseparam(struct phylink *pl,
				   struct ethtool_pauseparam *pause)
{
	int ret;

	mutex_lock(&pl->config_mutex);
	ret = __phylink_ethtool_set_pauseparam(pl, pause);
	mutex_unlock(&pl->config_mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(phylink_ethtool_set_pauseparam);

int phylink_ethtool_get_module_info(struct phylink *pl,
				    struct ethtool_modinfo *modinfo)
{
	int ret = -EOPNOTSUPP;

	mutex_lock(&pl->config_mutex);
	if (pl->module_ops)
		ret = pl->module_ops->get_module_info(pl->module_data,
						      modinfo);
	mutex_unlock(&pl->config_mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(phylink_ethtool_get_module_info);

int phylink_ethtool_get_module_eeprom(struct phylink *pl,
				      struct ethtool_eeprom *ee, u8 *buf)
{
	int ret = -EOPNOTSUPP;

	mutex_lock(&pl->config_mutex);
	if (pl->module_ops)
		ret = pl->module_ops->get_module_eeprom(pl->module_data, ee,
							buf);
	mutex_unlock(&pl->config_mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(phylink_ethtool_get_module_eeprom);

int phylink_init_eee(struct phylink *pl, bool clk_stop_enable)
{
	int ret = -EPROTONOSUPPORT;

	mutex_lock(&pl->config_mutex);
	if (pl->phydev)
		ret = phy_init_eee(pl->phydev, clk_stop_enable);
	mutex_unlock(&pl->config_mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(phylink_init_eee);

int phylink_get_eee_err(struct phylink *pl)
{
	int ret = 0;

	mutex_lock(&pl->config_mutex);
	if (pl->phydev)
		ret = phy_get_eee_err(pl->phydev);
	mutex_unlock(&pl->config_mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(phylink_get_eee_err);

int phylink_ethtool_get_eee(struct phylink *pl, struct ethtool_eee *eee)
{
	int ret = -EOPNOTSUPP;

	mutex_lock(&pl->config_mutex);
	if (pl->phydev)
		ret = phy_ethtool_get_eee(pl->phydev, eee);
	mutex_unlock(&pl->config_mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(phylink_ethtool_get_eee);

int phylink_ethtool_set_eee(struct phylink *pl, struct ethtool_eee *eee)
{
	int ret = -EOPNOTSUPP;

	mutex_lock(&pl->config_mutex);
	if (pl->phydev)
		ret = phy_ethtool_set_eee(pl->phydev, eee);
	mutex_unlock(&pl->config_mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(phylink_ethtool_set_eee);

/* This emulates MII registers for a fixed-mode phy operating as per the
 * passed in state. "aneg" defines if we report negotiation is possible.
 *
 * FIXME: should deal with negotiation state too.
 */
static int phylink_mii_emul_read(struct net_device *ndev, unsigned int reg,
				 struct phylink_link_state *state, bool aneg)
{
	struct fixed_phy_status fs;
	int val;

	fs.link = state->link;
	fs.speed = state->speed;
	fs.duplex = state->duplex;
	fs.pause = state->pause & MLO_PAUSE_SYM;
	fs.asym_pause = state->pause & MLO_PAUSE_ASYM;

	val = swphy_read_reg(reg, &fs);
	if (reg == MII_BMSR) {
		if (!state->an_complete)
			val &= ~BMSR_ANEGCOMPLETE;
		if (!aneg)
			val &= ~BMSR_ANEGCAPABLE;
	}
	return val;
}

static int phylink_mii_read(struct phylink *pl, unsigned int phy_id,
			    unsigned int reg)
{
	struct phylink_link_state state;
	int val = 0xffff;

	/* PHYs only exist for MLO_AN_PHY and MLO_AN_SGMII */
	if (pl->phydev)
		return mdiobus_read(pl->phydev->mdio.bus, phy_id, reg);

	switch (pl->link_an_mode) {
	case MLO_AN_FIXED:
		if (phy_id == 0) {
			phylink_get_fixed_state(pl, &state);
			val = phylink_mii_emul_read(pl->netdev, reg, &state,
						    true);
		}
		break;

	case MLO_AN_PHY:
		return -EOPNOTSUPP;

	case MLO_AN_SGMII:
		/* No phy, fall through to 8023z method */
	case MLO_AN_8023Z:
		if (phy_id == 0) {
			val = phylink_get_mac_state(pl, &state);
			if (val < 0)
				return val;

			val = phylink_mii_emul_read(pl->netdev, reg, &state,
						    true);
		}
		break;
	}

	return val & 0xffff;
}

static int phylink_mii_write(struct phylink *pl, unsigned int phy_id,
			     unsigned int reg, unsigned int val)
{
	/* PHYs only exist for MLO_AN_PHY and MLO_AN_SGMII */
	if (pl->phydev) {
		mdiobus_write(pl->phydev->mdio.bus, phy_id, reg, val);
		return 0;
	}

	switch (pl->link_an_mode) {
	case MLO_AN_FIXED:
		break;

	case MLO_AN_PHY:
		return -EOPNOTSUPP;

	case MLO_AN_SGMII:
		/* No phy, fall through to 8023z method */
	case MLO_AN_8023Z:
		break;
	}

	return 0;
}

int phylink_mii_ioctl(struct phylink *pl, struct ifreq *ifr, int cmd)
{
	struct mii_ioctl_data *mii_data = if_mii(ifr);
	int val, ret;

	mutex_lock(&pl->config_mutex);

	switch (cmd) {
	case SIOCGMIIPHY:
		mii_data->phy_id = pl->phydev ? pl->phydev->mdio.addr : 0;
		/* fallthrough */

	case SIOCGMIIREG:
		val = phylink_mii_read(pl, mii_data->phy_id, mii_data->reg_num);
		if (val < 0) {
			ret = val;
		} else {
			mii_data->val_out = val;
			ret = 0;
		}
		break;

	case SIOCSMIIREG:
		ret = phylink_mii_write(pl, mii_data->phy_id, mii_data->reg_num,
					mii_data->val_in);
		break;

	default:
		ret = -EOPNOTSUPP;
		if (pl->phydev)
			ret = phy_mii_ioctl(pl->phydev, ifr, cmd);
		break;
	}

	mutex_unlock(&pl->config_mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(phylink_mii_ioctl);



int phylink_register_module(struct phylink *pl, void *data,
			    const struct phylink_module_ops *ops)
{
	int ret = -EBUSY;

	mutex_lock(&pl->config_mutex);
	if (!pl->module_ops) {
		pl->module_ops = ops;
		pl->module_data = data;
		ret = 0;
	}
	mutex_unlock(&pl->config_mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(phylink_register_module);

int phylink_unregister_module(struct phylink *pl, void *data)
{
	int ret = -EINVAL;

	mutex_lock(&pl->config_mutex);
	if (pl->module_data == data) {
		pl->module_ops = NULL;
		pl->module_data = NULL;
		ret = 0;
	}
	mutex_unlock(&pl->config_mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(phylink_unregister_module);

void phylink_disable(struct phylink *pl)
{
	set_bit(PHYLINK_DISABLE_LINK, &pl->phylink_disable_state);
	flush_work(&pl->resolve);

	netif_carrier_off(pl->netdev);
}
EXPORT_SYMBOL_GPL(phylink_disable);

void phylink_enable(struct phylink *pl)
{
	clear_bit(PHYLINK_DISABLE_LINK, &pl->phylink_disable_state);
	phylink_run_resolve(pl);
}
EXPORT_SYMBOL_GPL(phylink_enable);

int phylink_set_link(struct phylink *pl, unsigned int mode, u8 port,
		     const unsigned long *support)
{
	__ETHTOOL_DECLARE_LINK_MODE_MASK(mask);
	int ret = 0;

	netdev_dbg(pl->netdev, "requesting link mode %s with support %*pb\n",
		   phylink_an_mode_str(mode),
		   __ETHTOOL_LINK_MODE_MASK_NBITS, support);

	if (mode == MLO_AN_FIXED)
		return -EINVAL;

	linkmode_copy(mask, support);

	/* Ignore errors if we're expecting a PHY to attach later */
	ret = phylink_validate_support(pl, mode, mask);
	if (ret && mode != MLO_AN_PHY)
		return ret;

	mutex_lock(&pl->config_mutex);
	if (mode == MLO_AN_8023Z && pl->phydev) {
		ret = -EINVAL;
	} else {
		bool changed = !bitmap_equal(pl->supported, mask,
					     __ETHTOOL_LINK_MODE_MASK_NBITS);
		if (changed) {
			linkmode_copy(pl->supported, mask);

			phylink_init_advert(pl, mode, mask,
					    pl->link_config.advertising);
		}

		if (pl->link_an_mode != mode) {
			pl->link_an_mode = mode;

			changed = true;

			netdev_info(pl->netdev, "switched to %s link mode\n",
				    phylink_an_mode_str(mode));
		}

		pl->link_port = port;

		if (changed && !test_bit(PHYLINK_DISABLE_STOPPED,
					 &pl->phylink_disable_state))
			phylink_mac_config(pl, &pl->link_config);
	}
	mutex_unlock(&pl->config_mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(phylink_set_link);

struct phylink *phylink_lookup_by_netdev(struct net_device *ndev)
{
	struct phylink *pl, *found = NULL;

	mutex_lock(&phylink_mutex);
	list_for_each_entry(pl, &phylinks, node)
		if (pl->netdev == ndev) {
			found = pl;
			break;
		}

	mutex_unlock(&phylink_mutex);

	return found;
}
EXPORT_SYMBOL_GPL(phylink_lookup_by_netdev);

MODULE_LICENSE("GPL");
