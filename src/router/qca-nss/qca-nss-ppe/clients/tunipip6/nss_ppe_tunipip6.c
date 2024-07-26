/*
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/rwlock_types.h>
#include <linux/hashtable.h>
#include <linux/inetdevice.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <net/ipv6.h>
#include <linux/if_arp.h>
#include <net/route.h>
#include <net/ip.h>
#include <linux/if_bridge.h>
#include <net/bonding.h>
#ifdef CONFIG_OF
#include <linux/of.h>
#endif
#include <linux/debugfs.h>

#include <net/ip6_tunnel.h>
#include <linux/netdevice.h>

#include <nss_ppe_tun_drv.h>
#include "nss_ppe_tunipip6.h"

static struct dentry *tunipip6_dentry;

static bool nss_tunipip6_stats_dentry_create(struct net_device *dev);
static bool nss_tunipip6_stats_dentry_free(struct net_device *dev);

static uint8_t encap_ecn_mode = PPE_DRV_TUN_CMN_CTX_ENCAP_ECN_NO_UPDATE;
module_param(encap_ecn_mode, byte, 0644);
MODULE_PARM_DESC(encap_ecn_mode, "Encap ECN mode 0:NO_UPDATE, 1:RFC3168_LIMIT_RFC6040_CMPAT, 2:RFC3168_FULL, 3:RFC4301_RFC6040_NORMAL");

static uint8_t decap_ecn_mode = PPE_DRV_TUN_CMN_CTX_DECAP_ECN_RFC3168_MODE;
module_param(decap_ecn_mode, byte, 0644);
MODULE_PARM_DESC(decap_ecn_mode, "Decap ECN mode 0:RFC3168, 1:RFC4301, 2:RFC6040");

static bool inherit_ttl = false;
module_param(inherit_ttl, bool, 0644);
MODULE_PARM_DESC(inherit_ttl, "TTL 0:Dont Inherit inner, 1:Inherit inner");

static bool inherit_dscp = false;
module_param(inherit_dscp, bool, 0644);
MODULE_PARM_DESC(inherit_dscp, "DSCP 0:Dont Inherit inner, 1:Inherit inner");

/*
 * nss_ppe_tunipip6_dev_stats_update()
 *	Update tunipip6 dev statistics
 */
static bool nss_ppe_tunipip6_dev_stats_update(struct net_device *dev, ppe_tun_hw_stats *stats, ppe_tun_data *tun_data)
{
	struct pcpu_sw_netstats *tstats;

	if (!dev) {
		return false;
	}

	tstats = this_cpu_ptr(dev->tstats);
	u64_stats_update_begin(&tstats->syncp);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	tstats->tx_bytes += stats->tx_byte_cnt;
	tstats->tx_packets += stats->tx_pkt_cnt;
	tstats->rx_bytes += stats->rx_byte_cnt;
	tstats->rx_packets += stats->rx_pkt_cnt;
#else
        u64_stats_add(&tstats->tx_bytes, stats->tx_byte_cnt);
	u64_stats_add(&tstats->tx_packets,  stats->tx_pkt_cnt);
	u64_stats_add(&tstats->rx_bytes, stats->rx_byte_cnt);
	u64_stats_add(&tstats->rx_packets,  stats->rx_pkt_cnt);
#endif
	u64_stats_update_end(&tstats->syncp);
/*
 * TODO: Remove the following check when net_device support for
 * drop counters is added from Kernel for PPE Tunnel stats.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	atomic_long_add(stats->tx_drop_pkt_cnt, &dev->tx_dropped);
	atomic_long_add(stats->rx_drop_pkt_cnt, &dev->rx_dropped);
#endif

	return true;
}

/*
 * nss_ppe_tunipip6_src_exception()
 *	handle the source VP exception.
 */
static bool nss_ppe_tunipip6_src_exception(struct ppe_vp_cb_info *info, ppe_tun_data *tun_data)
{
	struct sk_buff *skb = info->skb;

	skb_reset_network_header(skb);
	skb->protocol = htons(ETH_P_IP);
	netif_receive_skb(skb);
	return true;
}

/*
 * nss_ppe_tunipip6_dev_parse_param()
 *	parse tunnel parameter
 */
static bool nss_ppe_tunipip6_dev_parse_param(struct net_device *dev, struct ppe_drv_tun_cmn_ctx *tun_hdr)
{
	struct ip6_tnl *tunnel;
	struct flowi6 *fl6;
	struct ppe_drv_tun_cmn_ctx_l3 *l3 = &tun_hdr->l3;

	tunnel = (struct ip6_tnl *)netdev_priv(dev);

	if (!(tunnel->parms.flags & IP6_TNL_F_IGN_ENCAP_LIMIT)) {
		nss_ppe_tunipip6_warning("%p: Encap limit should be none", dev);
		return false;
	}

	/*
	 * Find the Tunnel device flow information
	 */
	fl6 = &tunnel->fl.u.ip6;
	nss_ppe_tunipip6_trace("%p: Tunnel param saddr: %pI6 daddr: %pI6", dev, fl6->saddr.s6_addr32,
				fl6->daddr.s6_addr32);
	nss_ppe_tunipip6_trace("%p: Hop limit %d", dev, tunnel->parms.hop_limit);
	nss_ppe_tunipip6_trace("%p: Tunnel param flag %x  fl6.flowlabel %x", dev,  tunnel->parms.flags, fl6->flowlabel);

	/*
	 * Prepare The Tunnel configuration parameter to send to nss
	 */
	l3->saddr[0] = (fl6->saddr.s6_addr32[0]);
	l3->saddr[1] = (fl6->saddr.s6_addr32[1]);
	l3->saddr[2] = (fl6->saddr.s6_addr32[2]);
	l3->saddr[3] = (fl6->saddr.s6_addr32[3]);
	l3->daddr[0] = (fl6->daddr.s6_addr32[0]);
	l3->daddr[1] = (fl6->daddr.s6_addr32[1]);
	l3->daddr[2] = (fl6->daddr.s6_addr32[2]);
	l3->daddr[3] = (fl6->daddr.s6_addr32[3]);

	if (inherit_ttl) {
		l3->flags |= PPE_DRV_TUN_CMN_CTX_L3_INHERIT_TTL;
	} else {
		l3->ttl = tunnel->parms.hop_limit;
	}

	if (inherit_dscp) {
		l3->flags |= PPE_DRV_TUN_CMN_CTX_L3_INHERIT_DSCP;
	} else {
		l3->dscp = ip6_tclass(tunnel->parms.flowinfo) & 0xfc;
	}

	if (encap_ecn_mode <= PPE_DRV_TUN_CMN_CTX_ENCAP_ECN_RFC4301_RFC6040_NORMAL_MODE) {
		l3->encap_ecn_mode = encap_ecn_mode;
	}

	if (decap_ecn_mode <= PPE_DRV_TUN_CMN_CTX_DECAP_ECN_RFC6040_MODE) {
		l3->decap_ecn_mode = decap_ecn_mode;
	}

	l3->proto = tunnel->parms.proto;
	l3->flags |= PPE_DRV_TUN_CMN_CTX_L3_IPV6;
	tun_hdr->type = PPE_DRV_TUN_CMN_CTX_TYPE_IPIP6;

	return true;
}

/*
 * nss_ppe_tunipip6_dev_event()
 *	Net device notifier for ipip6 module
 */
static int nss_ppe_tunipip6_dev_event(struct notifier_block  *nb,
		unsigned long event, void  *info)
{
	struct net_device *dev = netdev_notifier_info_to_dev(info);
	struct ppe_drv_tun_cmn_ctx *tun_hdr;
	struct ip6_tnl *tunnel = (struct ip6_tnl *)netdev_priv(dev);
	struct ppe_tun_excp *tun_cb = NULL;
	bool status;

	if (dev->type != ARPHRD_TUNNEL6) {
		return NOTIFY_DONE;
	}

	if (tunnel->parms.fmrs) {
		nss_ppe_tunipip6_trace("%p: IPIP6 does not support fmr, skip PPE.\n", dev);
		return NOTIFY_DONE;
	}

	nss_ppe_tunipip6_trace("%p: NETDEV event %lu for dev %s", dev, event, dev->name);

	switch (event) {
	case NETDEV_REGISTER:
		if (ip6_tunnel_is_fallback_dev(dev)) {
			nss_ppe_tunipip6_warning("%p: IPIP6 tunnel creation skipped for fb dev %s\n", dev, dev->name);
			break;
		}

		status = ppe_tun_alloc(dev, PPE_DRV_TUN_CMN_CTX_TYPE_IPIP6);
		if (status) {
			nss_tunipip6_stats_dentry_create(dev);
		}
		break;

	case NETDEV_UP:
		tun_hdr = kzalloc(sizeof(struct ppe_drv_tun_cmn_ctx), GFP_ATOMIC);
		if (!tun_hdr) {
			nss_ppe_tunipip6_warning("%p: memory allocation for tunnel failed", dev);
			break;
		}

		if (!nss_ppe_tunipip6_dev_parse_param(dev, tun_hdr)) {
			kfree(tun_hdr);
			break;
		}

		tun_cb = kzalloc(sizeof(struct ppe_tun_excp), GFP_ATOMIC);

		if (!tun_cb) {
			nss_ppe_tunipip6_warning("%px: memory allocation for tunnel callback failed for device %s\n", dev, dev->name);

			kfree(tun_hdr);
			break;
		}

		tun_cb->src_excp_method = nss_ppe_tunipip6_src_exception;
		tun_cb->stats_update_method = nss_ppe_tunipip6_dev_stats_update;

		if (!(ppe_tun_configure(dev, tun_hdr, tun_cb))) {
			nss_ppe_tunipip6_trace("%p: Unable to configure PPE tunnel for dev: %s", dev, dev->name);
		}

		kfree(tun_hdr);
		kfree(tun_cb);
		break;

	case NETDEV_CHANGEMTU:
		ppe_tun_mtu_set(dev, dev->mtu);
		break;

	case NETDEV_DOWN:
		ppe_tun_deconfigure(dev);
		break;

	case NETDEV_UNREGISTER:
		ppe_tun_free(dev);
		nss_tunipip6_stats_dentry_free(dev);
		break;

	default:
		nss_ppe_tunipip6_trace("%p: Unhandled notifier dev %s event %x", dev, dev->name, (int)event);
		break;
	}

	return NOTIFY_DONE;
}

/*
 * nss_ppe_tunipip6_stats_show()
 *	Read ppe tunnel statistics.
 */
static int nss_ppe_tunipip6_stats_show(struct seq_file *m, void __attribute__((unused))*ptr)
{
	struct net_device *dev = (struct net_device *)m->private;
	uint64_t exception_packet = 0;
	uint64_t exception_bytes = 0;

	ppe_tun_exception_packet_get(dev, &exception_packet, &exception_bytes);

	seq_puts(m, "\n################ PPE Client tunipip6 Statistics Start ################\n");
	seq_printf(m, "dev: %s\n", dev->name);
	seq_puts(m, "  Exception:\n");
	seq_printf(m, "\t exception packet: %llu\n", exception_packet);
	seq_printf(m, "\t exception bytes: %llu\n", exception_bytes);
	seq_puts(m, "\n################ PPE Client tunipip6 Statistics End ################\n");

	return 0;
}

/*
 * nss_ppe_tunipip6_stats_open()
 */
static int nss_ppe_tunipip6_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, nss_ppe_tunipip6_stats_show, inode->i_private);
}

/*
 * nss_ppe_tunipip6_stats_ops
 *	File operations for tunipip6 tunnel stats
 */
static const struct file_operations nss_ppe_tunipip6_stats_ops = {
	.open = nss_ppe_tunipip6_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

/*
 * nss_tunipip6_stats_dentry_create()
 *	Create dentry for a given netdevice.
 */
static bool nss_tunipip6_stats_dentry_create(struct net_device *dev)
{
	char dentry_name[IFNAMSIZ];
	struct dentry *dentry;

	scnprintf(dentry_name, sizeof(dentry_name), "%s", dev->name);

	dentry = debugfs_create_file(dentry_name, S_IRUGO,
			tunipip6_dentry, dev, &nss_ppe_tunipip6_stats_ops);
	if (!dentry) {
		nss_ppe_tunipip6_warning("Debugfs file creation failed for device %s", dev->name);
		return false;
	}

	return true;
}

/*
 * nss_tunipip6_stats_dentry_free()
 *	Remove dentry for a given netdevice.
 */
static bool nss_tunipip6_stats_dentry_free(struct net_device *dev)
{
	char dentry_name[IFNAMSIZ];
	struct dentry *dentry;

	scnprintf(dentry_name, sizeof(dentry_name), "%s", dev->name);

	dentry = debugfs_lookup(dentry_name, tunipip6_dentry);
	if (dentry) {
		debugfs_remove(dentry);
		nss_ppe_tunipip6_trace("removed stats debugfs entry for dev %s", dentry_name);
		return true;
	}

	nss_ppe_tunipip6_trace("Could not find stats debugfs entry for dev %s", dentry_name);
	return false;
}

/*
 * nss_tunipip6_stats_dentry_deinit()
 *	Cleanup the debugfs tree.
 */
static void nss_ppe_tunipip6_dentry_deinit(void)
{
	debugfs_remove_recursive(tunipip6_dentry);
	tunipip6_dentry = NULL;
}

/*
 * nss_ppe_tunipip6_dentry_init()
 *	Create tunipip6 tunnel statistics debugfs entry.
 */
static bool nss_ppe_tunipip6_dentry_init(void)
{
	/*
	 * Initialize debugfs directory.
	 */
	struct dentry *parent;
	struct dentry *clients;

	parent = debugfs_lookup("qca-nss-ppe", NULL);
	if (!parent) {
		nss_ppe_tunipip6_warning("parent debugfs entry for qca-nss-ppe not present");
		return false;
	}

	clients = debugfs_lookup("clients", parent);
	if (!clients) {
		nss_ppe_tunipip6_warning("clients debugfs entry inside qca-nss-ppe not present");
		return false;
	}

	tunipip6_dentry = debugfs_create_dir("tunipip6", clients);
	if (!tunipip6_dentry) {
		nss_ppe_tunipip6_warning("tunipip6 debugfs entry inside qca-nss-ppe/clients could not be created");
		return false;
	}

	return true;
}

/*
 * Linux Net device Notifier
 */
struct notifier_block nss_ppe_tunipip6_notifier = {
	.notifier_call = nss_ppe_tunipip6_dev_event,
};

/*
 * nss_ppe_tunipip6_init_module()
 *	Tunnel ipip6 module init function
 */
int __init nss_ppe_tunipip6_init_module(void)
{
	/*
	 * Create the debugfs directory for statistics.
	 */
	if (!nss_ppe_tunipip6_dentry_init()) {
		nss_ppe_tunipip6_warning("Failed to initialize debugfs");
		return -1;
	}

	if (encap_ecn_mode > PPE_DRV_TUN_CMN_CTX_ENCAP_ECN_RFC4301_RFC6040_NORMAL_MODE) {
		nss_ppe_tunipip6_dentry_deinit();
		nss_ppe_tunipip6_warning("Invalid Encap ECN mode %u\n", encap_ecn_mode);
		return -1;
	}

	if (decap_ecn_mode > PPE_DRV_TUN_CMN_CTX_DECAP_ECN_RFC6040_MODE) {
		nss_ppe_tunipip6_dentry_deinit();
		nss_ppe_tunipip6_warning("Invalid Decap ECN mode %u\n", decap_ecn_mode);
		return -1;
	}

	/*
	 * Register net device notification for standard tunnel.
	 */
	register_netdevice_notifier(&nss_ppe_tunipip6_notifier);

	nss_ppe_tunipip6_trace("tunipip6 PPE driver registered");
	return 0;
}

/*
 * nss_ppe_tunipip6_exit_module()
 *	Tunnel ipip6 module exit function
 */
void __exit nss_ppe_tunipip6_exit_module(void)
{
	/*
	 * Disable all the ipip6 connection in PPE tunnel driver
	 */
	ppe_tun_conf_accel(PPE_DRV_TUN_CMN_CTX_TYPE_IPIP6, false);

	/*
	 * De-initialize debugfs.
	 */
	nss_ppe_tunipip6_dentry_deinit();

	/*
	 * Unregister net device notification for standard tunnel.
	 */
	unregister_netdevice_notifier(&nss_ppe_tunipip6_notifier);

	nss_ppe_tunipip6_info("tunipip6 PPE module unloaded");
}

module_init(nss_ppe_tunipip6_init_module);
module_exit(nss_ppe_tunipip6_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS PPE tunipip6 client driver");
