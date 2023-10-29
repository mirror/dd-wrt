// SPDX-License-Identifier: GPL-2.0
/*  OpenVPN data channel accelerator
 *
 *  Copyright (C) 2020-2023 OpenVPN, Inc.
 *
 *  Author:	Antonio Quartulli <antonio@openvpn.net>
 *		James Yonan <james@openvpn.net>
 */

#include "main.h"

#include "ovpn.h"
#include "ovpnstruct.h"
#include "netlink.h"
#include "tcp.h"

#include <linux/ethtool.h>
#include <linux/genetlink.h>
#include <linux/inetdevice.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/lockdep.h>
#include <linux/rcupdate.h>
#include <linux/net.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/version.h>

#include <net/ip_tunnels.h>

/* Driver info */
#define DRV_NAME	"ovpn-dco"
#define DRV_VERSION	OVPN_DCO_VERSION
#define DRV_DESCRIPTION	"OpenVPN data channel offload (ovpn-dco)"
#define DRV_COPYRIGHT	"(C) 2020-2023 OpenVPN, Inc."

static void ovpn_struct_free(struct net_device *net)
{
	struct ovpn_struct *ovpn = netdev_priv(net);

	security_tun_dev_free_security(ovpn->security);
	free_percpu(net->tstats);
	flush_workqueue(ovpn->crypto_wq);
	flush_workqueue(ovpn->events_wq);
	destroy_workqueue(ovpn->crypto_wq);
	destroy_workqueue(ovpn->events_wq);
	rcu_barrier();
}

/* Net device open */
static int ovpn_net_open(struct net_device *dev)
{
	struct in_device *dev_v4 = __in_dev_get_rtnl(dev);

	if (dev_v4) {
		/* disable redirects as Linux gets confused by ovpn-dco handling same-LAN routing */
		IN_DEV_CONF_SET(dev_v4, SEND_REDIRECTS, false);
		IPV4_DEVCONF_ALL(dev_net(dev), SEND_REDIRECTS) = false;
	}

	netif_tx_start_all_queues(dev);
	return 0;
}

/* Net device stop -- called prior to device unload */
static int ovpn_net_stop(struct net_device *dev)
{
	netif_tx_stop_all_queues(dev);
	return 0;
}

/*******************************************
 * ovpn ethtool ops
 *******************************************/
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 5, 0)

static int ovpn_get_link_ksettings(struct net_device *dev,
				   struct ethtool_link_ksettings *cmd)
{
	ethtool_convert_legacy_u32_to_link_mode(cmd->link_modes.supported, 0);
	ethtool_convert_legacy_u32_to_link_mode(cmd->link_modes.advertising, 0);
	cmd->base.speed	= SPEED_1000;
	cmd->base.duplex = DUPLEX_FULL;
	cmd->base.port = PORT_TP;
	cmd->base.phy_address = 0;
	cmd->base.transceiver = XCVR_INTERNAL;
	cmd->base.autoneg = AUTONEG_DISABLE;

	return 0;
}
#else
static int ovpn_get_settings(struct net_device *netdev,
			      struct ethtool_cmd *cmd)
{
//	ethtool_convert_legacy_u32_to_link_mode(cmd->link_modes.supported, 0);
//	ethtool_convert_legacy_u32_to_link_mode(cmd->link_modes.advertising, 0);
	cmd->supported = 0;
	cmd->advertising = 0;
	cmd->speed	= SPEED_1000;
	cmd->duplex = DUPLEX_FULL;
	cmd->port = PORT_TP;
	cmd->phy_address = 0;
	cmd->transceiver = XCVR_INTERNAL;
	cmd->autoneg = AUTONEG_DISABLE;
	return 0;
}
#endif
static void ovpn_get_drvinfo(struct net_device *dev,
			     struct ethtool_drvinfo *info)
{
	strscpy(info->driver, DRV_NAME, sizeof(info->driver));
	strscpy(info->version, DRV_VERSION, sizeof(info->version));
	strscpy(info->bus_info, "ovpn", sizeof(info->bus_info));
}

bool ovpn_dev_is_valid(const struct net_device *dev)
{
	return dev->netdev_ops->ndo_start_xmit == ovpn_net_xmit;
}

/*******************************************
 * ovpn exported methods
 *******************************************/

static const struct net_device_ops ovpn_netdev_ops = {
	.ndo_open		= ovpn_net_open,
	.ndo_stop		= ovpn_net_stop,
	.ndo_start_xmit		= ovpn_net_xmit,
	.ndo_get_stats64        = dev_get_tstats64,
};

static const struct ethtool_ops ovpn_ethtool_ops = {
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 5, 0)
	.get_link_ksettings	= ovpn_get_link_ksettings,
#else
	.get_settings	= ovpn_get_settings,
#endif
	.get_drvinfo		= ovpn_get_drvinfo,
	.get_link		= ethtool_op_get_link,
	.get_ts_info		= ethtool_op_get_ts_info,
};

static void ovpn_setup(struct net_device *dev)
{
	/* compute the overhead considering AEAD encryption */
	const int overhead = sizeof(u32) + NONCE_WIRE_SIZE + 16 + sizeof(struct udphdr) +
			     max(sizeof(struct ipv6hdr), sizeof(struct iphdr));

	netdev_features_t feat = NETIF_F_SG | NETIF_F_LLTX |
				 NETIF_F_HW_CSUM | NETIF_F_RXCSUM | NETIF_F_GSO |
				 NETIF_F_GSO_SOFTWARE | NETIF_F_HIGHDMA;

	dev->ethtool_ops = &ovpn_ethtool_ops;

	dev->netdev_ops = &ovpn_netdev_ops;
	netdev_set_priv_destructor(dev, ovpn_struct_free);

	/* Point-to-Point TUN Device */
	dev->hard_header_len = 0;
	dev->addr_len = 0;
	dev->mtu = ETH_DATA_LEN - overhead;
	dev->min_mtu = IPV4_MIN_MTU;
	dev->max_mtu = IP_MAX_MTU - overhead;

	/* Zero header length */
	dev->type = ARPHRD_NONE;
	dev->flags = IFF_POINTOPOINT | IFF_NOARP | IFF_MULTICAST;

	dev->features |= feat;
	dev->hw_features |= feat;
	dev->hw_enc_features |= feat;

	dev->needed_headroom = OVPN_HEAD_ROOM;
	dev->needed_tailroom = OVPN_MAX_PADDING;
}

static const struct nla_policy ovpn_policy[IFLA_OVPN_MAX + 1] = {
	[IFLA_OVPN_MODE] = NLA_POLICY_RANGE(NLA_U8, __OVPN_MODE_FIRST,
					    __OVPN_MODE_AFTER_LAST - 1),
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
static int ovpn_newlink(struct net *src_net, struct net_device *dev, struct nlattr *tb[],
			struct nlattr *data[])
#else
static int ovpn_newlink(struct net *src_net, struct net_device *dev, struct nlattr *tb[],
			struct nlattr *data[], struct netlink_ext_ack *extack)
#endif
{
	struct ovpn_struct *ovpn = netdev_priv(dev);
	int ret;

	ret = security_tun_dev_create();
	if (ret < 0)
		return ret;

	ret = ovpn_struct_init(dev);
	if (ret < 0)
		return ret;

	ovpn->mode = OVPN_MODE_P2P;
	if (data && data[IFLA_OVPN_MODE]) {
		ovpn->mode = nla_get_u8(data[IFLA_OVPN_MODE]);
		netdev_dbg(dev, "%s: setting device (%s) mode: %u\n", __func__, dev->name,
			   ovpn->mode);
	}

	return register_netdevice(dev);
}

static void ovpn_dellink(struct net_device *dev, struct list_head *head)
{
	struct ovpn_struct *ovpn = netdev_priv(dev);

	switch (ovpn->mode) {
	case OVPN_MODE_P2P:
		ovpn_peer_release_p2p(ovpn);
		break;
	default:
		ovpn_peers_free(ovpn);
		break;
	}

	unregister_netdevice_queue(dev, head);
}

static struct rtnl_link_ops ovpn_link_ops __read_mostly = {
	.kind			= DRV_NAME,
	.priv_size		= sizeof(struct ovpn_struct),
	.setup			= ovpn_setup,
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 20, 0)
	.policy			= ovpn_policy,
#endif
	.maxtype		= IFLA_OVPN_MAX,
	.newlink		= ovpn_newlink,
	.dellink		= ovpn_dellink,
};

static int __init ovpn_init(void)
{
	int err = 0;

	pr_info("%s %s -- %s\n", DRV_DESCRIPTION, DRV_VERSION, DRV_COPYRIGHT);

	err = ovpn_tcp_init();
	if (err) {
		pr_err("ovpn: can't initialize TCP subsystem\n");
		goto err;
	}

	/* init RTNL link ops */
	err = rtnl_link_register(&ovpn_link_ops);
	if (err) {
		pr_err("ovpn: can't register RTNL link ops\n");
		goto err;
	}

	err = ovpn_netlink_register();
	if (err) {
		pr_err("ovpn: can't register netlink family\n");
		goto err_rtnl_unregister;
	}

	return 0;

err_rtnl_unregister:
	rtnl_link_unregister(&ovpn_link_ops);
err:
	pr_err("ovpn: initialization failed, error status=%d\n", err);
	return err;
}

static __exit void ovpn_cleanup(void)
{
	rtnl_link_unregister(&ovpn_link_ops);
	ovpn_netlink_unregister();
	rcu_barrier(); /* because we use call_rcu */
}

module_init(ovpn_init);
module_exit(ovpn_cleanup);

MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_AUTHOR(DRV_COPYRIGHT);
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
MODULE_ALIAS_RTNL_LINK(DRV_NAME);
MODULE_ALIAS_GENL_FAMILY(OVPN_NL_NAME);
