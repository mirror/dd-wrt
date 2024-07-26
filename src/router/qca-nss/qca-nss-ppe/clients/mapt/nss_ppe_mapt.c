/*
 * Copyright (c) 2016-2021, The Linux Foundation. All rights reserved.
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
#include <linux/debugfs.h>
#include <net/ip6_tunnel.h>
#include <linux/netdevice.h>

#include <nat46-core.h>
#include <nat46-netdev.h>
#include <nss_ppe_tun_drv.h>
#include <ppe_drv_port.h>
#include "nss_ppe_mapt.h"

/*
 * (IPv6 hdr - IPv4 hdr) + PPPoE hdr + CVLAN + SVLAN
 */
#define NSS_PPE_MAPT_MTU_OVERHEAD ((40 - 20) + 8 + 4 + 4)
#define NSS_PPE_MAPT_MAX_MTU (PPE_DRV_PORT_JUMBO_MAX - NSS_PPE_MAPT_MTU_OVERHEAD)

static struct dentry *mapt_dentry;

static bool nss_mapt_stats_dentry_create(struct net_device *dev);
static bool nss_mapt_stats_dentry_free(struct net_device *dev);

static uint8_t encap_ecn_mode = PPE_DRV_TUN_CMN_CTX_ENCAP_ECN_NO_UPDATE;
module_param(encap_ecn_mode, byte, 0644);
MODULE_PARM_DESC(encap_ecn_mode, "Encap ECN mode 0:NO_UPDATE, 1:RFC3168_LIMIT_RFC6040_CMPAT, 2:RFC3168_FULL, 3:RFC4301_RFC6040_NORMAL");

static uint8_t decap_ecn_mode = PPE_DRV_TUN_CMN_CTX_DECAP_ECN_RFC3168_MODE;
module_param(decap_ecn_mode, byte, 0644);
MODULE_PARM_DESC(decap_ecn_mode, "Decap ECN mode 0:RFC3168, 1:RFC4301, 2:RFC6040");

static bool inherit_dscp = false;
module_param(inherit_dscp, bool, 0644);
MODULE_PARM_DESC(inherit_dscp, "DSCP 0:Dont Inherit inner, 1:Inherit inner");

static bool inherit_ttl = true;
module_param(inherit_ttl, bool, 0644);
MODULE_PARM_DESC(inherit_ttl, "TTL 0:Dont Inherit inner, 1:Inherit inner");

/*
 * nss_ppe_mapt_dev_stats_update()
 *	Update MAPT dev statistics
 */
static bool nss_ppe_mapt_dev_stats_update(struct net_device *dev, ppe_tun_hw_stats *stats, ppe_tun_data *tun_cb_data)
{
	struct pcpu_sw_netstats *tstats;

	if (!(dev->priv_flags_ext & IFF_EXT_MAPT)) {
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
	/*
	 * For Map-t device we need to update the rx and tx stats separately.
	 */
	if (unlikely(dev->priv_flags_ext & IFF_EXT_MAPT)) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
		tstats->rx_bytes += stats->tx_byte_cnt;
		tstats->rx_packets += stats->tx_pkt_cnt;
		tstats->tx_bytes += stats->rx_byte_cnt;
		tstats->tx_packets += stats->rx_pkt_cnt;
#else
		u64_stats_add(&tstats->rx_bytes, stats->tx_byte_cnt);
		u64_stats_add(&tstats->rx_packets,  stats->tx_pkt_cnt);
		u64_stats_add(&tstats->tx_bytes, stats->rx_byte_cnt);
		u64_stats_add(&tstats->tx_packets,  stats->rx_pkt_cnt);
#endif
	}

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
 * nss_ppe_mapt_src_exception()
 *	handle the source VP exception.
 */
static bool nss_ppe_mapt_src_exception(struct ppe_vp_cb_info *info, ppe_tun_data *tun_data)
{
	struct sk_buff *skb = info->skb;
	struct net_device *dev = skb->dev;
	int ret;

	skb->protocol = eth_type_trans(skb, dev);
	skb_reset_network_header(skb);

	ret = netif_receive_skb(skb);
	if (ret != NET_RX_SUCCESS) {
		nss_ppe_mapt_warning("%p: excpetion packet dropped\n", dev);
	}

	return true;
}

/*
 * nss_ppe_mapt_validate_rule_style_mapt()
 *	Validate map-t style rule
 */
static bool nss_ppe_mapt_validate_rule_style_mapt(struct net_device *dev, struct nat46_xlate_rule *rule,
						       bool is_local_rule)
{
	int psid_len;

	/*
	 * Validate rule parameters
	 */
	if (rule->ea_len < 0 || rule->ea_len > 48) {
		nss_ppe_mapt_warning("%p: mapt rule is invalid as ea_len < 0 or ea_len > 48", dev);
		return false;
	}

	if (rule->v4_pref_len + rule->ea_len > 32) {
		psid_len = rule->ea_len - (32 - rule->v4_pref_len);
	} else {
		psid_len = 0;
	}

	if (psid_len + rule->psid_offset > 16) {
		nss_ppe_mapt_warning("%p: mapt rule is invalid as psid offset + psid len > 16", dev);
		return false;
	}

	return true;
}

/*
 * nss_ppe_mapt_validate_rule_style_rfc6052()
 *	Validate map-t style rule
 */
static bool nss_ppe_mapt_validate_rule_style_rfc6052(struct net_device *dev, struct nat46_xlate_rule *rule,
						     bool is_local_rule)
{
	if (!(rule->v6_pref_len == 32 || rule->v6_pref_len == 40 ||
	      rule->v6_pref_len == 48 || rule->v6_pref_len == 56 ||
	      rule->v6_pref_len == 64 || rule->v6_pref_len == 96)) {
		nss_ppe_mapt_warning("%p: mapt rule is invalid as rfc6052 end user prefix is invalid", dev);
		return false;
	}

	return true;
}

/*
 * nss_ppe_mapt_validate_rule()
 *	Check each mapt rule params and validate each field.
 *
 * Returns true if all parameters are correct.
 * As per RFC7599 (map-t rfc), local style should be map-t. Remote style
 * of FMR must be map-t, but remote syle of DMR is RFC6052.
 * map-t user space process doesnot really mandates this restriction and
 * allows style of rule can be anything irrespective of FMR or DMR. So
 * this check also won't fails on style mismatch.
 */
bool nss_ppe_mapt_validate_rule(struct net_device *dev, struct nat46_xlate_rulepair *rule_pair)
{
	/*
	 * Validate local rule parameters
	 */
	switch (rule_pair->local.style) {
	case NAT46_XLATE_NONE:
		return false;

	case NAT46_XLATE_MAP:
		if (!nss_ppe_mapt_validate_rule_style_mapt(dev, &rule_pair->local, true)) {
			return false;
		}
		break;

	case NAT46_XLATE_RFC6052:
		if (!nss_ppe_mapt_validate_rule_style_rfc6052(dev, &rule_pair->local, true)) {
			return false;
		}
		break;

	default:
		return false;
	}

	/*
	 * Validate remote rule parameters
	 */
	switch (rule_pair->remote.style) {
	case NAT46_XLATE_MAP:
		if (!nss_ppe_mapt_validate_rule_style_mapt(dev, &rule_pair->remote, false)) {
			return false;
		}
		break;

	case NAT46_XLATE_RFC6052:
		if (!nss_ppe_mapt_validate_rule_style_rfc6052(dev, &rule_pair->remote, false)) {
			return false;
		}
		break;

	default:
		return false;
	}

	return true;
}

/*
 * nss_ppe_mapt_dev_parse_param()
 *	parse tunnel parameter
 */
static bool nss_ppe_mapt_dev_parse_param(struct net_device *dev, struct ppe_drv_tun_cmn_ctx *tun_hdr)
{
	struct ppe_drv_tun_cmn_ctx_mapt *mapt = &tun_hdr->tun.mapt;
	struct ppe_drv_tun_cmn_ctx_l3 *l3 = &tun_hdr->l3;
	struct nat46_xlate_rulepair *rule_pairs;
	int rule_pair_count = 0;
	uint8_t map_t_flags = 0;
	struct ip6_tnl *tunnel;
	int j;

	tunnel = (struct ip6_tnl *)netdev_priv(dev);

	/*
	 * Get MAP-T interface's information.
	 */
	if (!nat46_get_info(dev, &rule_pairs, &rule_pair_count, &map_t_flags)) {
		nss_ppe_mapt_warning("%p: Failed to get rulepair on %s", dev, dev->name);
		return false;
	}

	/*
	 * Return, if number of  rules configured for the map-t
	 * interface is not equal to 1, as PPE needs exactly one rule pair for offload.
	 */

	if (rule_pair_count != 1) {
		nss_ppe_mapt_warning("%p: Rule pair count %d is not supported", dev, rule_pair_count);
		return false;
	}

	/*
	 * check correctness of a map-t rule
	 */
	if (!nss_ppe_mapt_validate_rule(dev, rule_pairs)) {
		nss_ppe_mapt_warning("%p: MAP-T rule correctness validation failed", dev);
		return false;
	}

	/*
	 * Set local rule params
	 */
	for (j = 0; j < 4; j++) {
		*((uint32_t *)(mapt->local.ipv6_prefix) + j) = ntohl(*((uint32_t *)(rule_pairs->local.v6_pref.s6_addr) + j));
	}

	mapt->local.ipv6_prefix_len = rule_pairs->local.v6_pref_len;
	mapt->local.ipv4_prefix = ntohl(rule_pairs->local.v4_pref);
	mapt->local.ipv4_prefix_len = rule_pairs->local.v4_pref_len;
	mapt->local.ea_len = rule_pairs->local.ea_len;
	mapt->local.psid_offset = rule_pairs->local.psid_offset;

	/*
	 * Set remote rule params
	 */
	for (j = 0; j < 4; j++) {
		*((uint32_t *)(mapt->remote.ipv6_prefix) + j) = ntohl(*((uint32_t *)(rule_pairs->remote.v6_pref.s6_addr) + j));
	}

	mapt->remote.ipv6_prefix_len = rule_pairs->remote.v6_pref_len;
	mapt->remote.ipv4_prefix = ntohl(rule_pairs->remote.v4_pref);
	mapt->remote.ipv4_prefix_len = rule_pairs->remote.v4_pref_len;
	mapt->remote.ea_len = rule_pairs->remote.ea_len;
	mapt->remote.psid_offset = rule_pairs->remote.psid_offset;

	l3->ttl = tunnel->parms.hop_limit;
	if (inherit_ttl) {
		l3->flags |= PPE_DRV_TUN_CMN_CTX_L3_INHERIT_TTL;
	}

	l3->dscp = ip6_tclass(tunnel->parms.flowinfo) & 0xfc;
	if (inherit_dscp) {
		l3->flags |=  PPE_DRV_TUN_CMN_CTX_L3_INHERIT_DSCP;
	}

	if (encap_ecn_mode <= PPE_DRV_TUN_CMN_CTX_ENCAP_ECN_RFC4301_RFC6040_NORMAL_MODE) {
		l3->encap_ecn_mode = encap_ecn_mode;
	}

	if (decap_ecn_mode <= PPE_DRV_TUN_CMN_CTX_DECAP_ECN_RFC6040_MODE) {
		l3->decap_ecn_mode = decap_ecn_mode;
	}
	l3->proto = tunnel->parms.proto;
	l3->flags |= PPE_DRV_TUN_CMN_CTX_L3_IPV6;
	tun_hdr->type = PPE_DRV_TUN_CMN_CTX_TYPE_MAPT;

	return true;
}

/*
 * nss_ppe_mapt_dev_event()
 *	Net device notifier for mapt module
 */
static int nss_ppe_mapt_dev_event(struct notifier_block  *nb,
		unsigned long event, void  *info)
{
	struct net_device *dev = netdev_notifier_info_to_dev(info);
	struct ppe_drv_tun_cmn_ctx *tun_hdr;
	struct ppe_tun_excp *tun_cb = NULL;
	int status;

	if (!(dev->priv_flags_ext & IFF_EXT_MAPT)) {
		return NOTIFY_DONE;
	}

	nss_ppe_mapt_trace("%p: NETDEV event %lu for dev %s", dev, event, dev->name);

	switch (event) {
	case NETDEV_REGISTER:
		if (dev->mtu > NSS_PPE_MAPT_MAX_MTU) {
			status = dev_set_mtu(dev, NSS_PPE_MAPT_MAX_MTU);
			if (status) {
				nss_ppe_mapt_warning("%p: Error %d limiting the mtu %d for %s",
						     dev, status, PPE_DRV_PORT_JUMBO_MAX, dev->name);
				break;
			}
		}

		status = ppe_tun_alloc(dev, PPE_DRV_TUN_CMN_CTX_TYPE_MAPT);
		if (status) {
			nss_mapt_stats_dentry_create(dev);
		}

		break;

	case NETDEV_UP:
		tun_hdr = kzalloc(sizeof(struct ppe_drv_tun_cmn_ctx), GFP_ATOMIC);
		if (!tun_hdr) {
			nss_ppe_mapt_warning("%p: memory allocation for tunnel failed", dev);
			ppe_tun_free(dev);
			nss_mapt_stats_dentry_free(dev);
			break;
		}

		if (!nss_ppe_mapt_dev_parse_param(dev, tun_hdr)) {
			nss_ppe_mapt_warning("%p: Unable to parse param for PPE tunnel for dev: %s", dev, dev->name);
			ppe_tun_free(dev);
			nss_mapt_stats_dentry_free(dev);
			kfree(tun_hdr);
			break;
		}

		tun_cb = kzalloc(sizeof(struct ppe_tun_excp), GFP_ATOMIC);

		if (!tun_cb) {
			nss_ppe_mapt_warning("%px: memory allocation for tunnel callback failed for device %s\n", dev, dev->name);

			kfree(tun_hdr);
			break;
		}

		tun_cb->src_excp_method = nss_ppe_mapt_src_exception;
		tun_cb->stats_update_method = nss_ppe_mapt_dev_stats_update;

		if (!(ppe_tun_configure(dev, tun_hdr, tun_cb))) {
			nss_ppe_mapt_warning("%p: Unable to configure PPE tunnel for dev: %s", dev, dev->name);
			ppe_tun_free(dev);
			nss_mapt_stats_dentry_free(dev);
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
		nss_mapt_stats_dentry_free(dev);
		break;

	default:
		nss_ppe_mapt_trace("%p: Unhandled notifier dev %s event %x", dev, dev->name, (int)event);
		break;
	}

	return NOTIFY_DONE;
}

/*
 * nss_ppe_mapt_stats_show()
 *	Read ppe tunnel statistics.
 */
static int nss_ppe_mapt_stats_show(struct seq_file *m, void __attribute__((unused))*ptr)
{
	struct net_device *dev = (struct net_device *)m->private;
	uint64_t exception_packet = 0;
	uint64_t exception_bytes = 0;

	ppe_tun_exception_packet_get(dev, &exception_packet, &exception_bytes);

	seq_puts(m, "\n################ PPE Client mapt Statistics Start ################\n");
	seq_printf(m, "dev: %s\n", dev->name);
	seq_puts(m, "  Exception:\n");
	seq_printf(m, "\t exception packet: %llu\n", exception_packet);
	seq_printf(m, "\t exception bytes: %llu\n", exception_bytes);
	seq_puts(m, "\n################ PPE Client mapt Statistics End ################\n");

	return 0;
}

/*
 * nss_ppe_mapt_stats_open()
 *	Mapt stats open handler.
 */
static int nss_ppe_mapt_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, nss_ppe_mapt_stats_show, inode->i_private);
}

/*
 * nss_ppe_mapt_stats_ops
 *	File operations for mapt tunnel stats
 */
static const struct file_operations nss_ppe_mapt_stats_ops = {
	.open = nss_ppe_mapt_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

/*
 * nss_mapt_stats_dentry_create()
 *	Create dentry for a given netdevice.
 */
static bool nss_mapt_stats_dentry_create(struct net_device *dev)
{
	char dentry_name[IFNAMSIZ];
	struct dentry *dentry;

	scnprintf(dentry_name, sizeof(dentry_name), "%s", dev->name);

	dentry = debugfs_create_file(dentry_name, S_IRUGO,
			mapt_dentry, dev, &nss_ppe_mapt_stats_ops);
	if (!dentry) {
		nss_ppe_mapt_warning("Debugfs file creation failed for device %s", dev->name);
		return false;
	}

	return true;
}

/*
 * nss_mapt_stats_dentry_free()
 *	Remove dentry for a given netdevice.
 */
static bool nss_mapt_stats_dentry_free(struct net_device *dev)
{
	char dentry_name[IFNAMSIZ];
	struct dentry *dentry;

	scnprintf(dentry_name, sizeof(dentry_name), "%s", dev->name);

	dentry = debugfs_lookup(dentry_name, mapt_dentry);
	if (dentry) {
		debugfs_remove(dentry);
		nss_ppe_mapt_trace("removed stats debugfs entry for dev %s", dentry_name);
		return true;
	}

	nss_ppe_mapt_trace("Could not find stats debugfs entry for dev %s", dentry_name);
	return false;
}

/*
 * nss_mapt_stats_dentry_deinit()
 *	Cleanup the debugfs tree.
 */
static void nss_ppe_mapt_dentry_deinit(void)
{
	debugfs_remove_recursive(mapt_dentry);
	mapt_dentry = NULL;
}

/*
 * nss_ppe_mapt_dentry_init()
 *	Create mapt tunnel statistics debugfs entry.
 */
static bool nss_ppe_mapt_dentry_init(void)
{
	/*
	 * Initialize debugfs directory.
	 */
	struct dentry *parent;
	struct dentry *clients;

	parent = debugfs_lookup("qca-nss-ppe", NULL);
	if (!parent) {
		nss_ppe_mapt_warning("parent debugfs entry for qca-nss-ppe not present");
		return false;
	}

	clients = debugfs_lookup("clients", parent);
	if (!clients) {
		nss_ppe_mapt_warning("clients debugfs entry inside qca-nss-ppe not present");
		return false;
	}

	mapt_dentry = debugfs_create_dir("mapt", clients);
	if (!mapt_dentry) {
		nss_ppe_mapt_warning("mapt debugfs entry inside qca-nss-ppe/clients could not be created");
		return false;
	}

	return true;
}

/*
 * Linux Net device Notifier
 */
struct notifier_block nss_ppe_mapt_notifier = {
	.notifier_call = nss_ppe_mapt_dev_event,
};

/*
 * nss_ppe_mapt_init_module()
 *	Tunnel mapt module init function
 */
int __init nss_ppe_mapt_init_module(void)
{
	/*
	 * Create the debugfs directory for statistics.
	 */
	if (!nss_ppe_mapt_dentry_init()) {
		nss_ppe_mapt_trace("Failed to initialize debugfs");
		return -1;
	}

	if (encap_ecn_mode > PPE_DRV_TUN_CMN_CTX_ENCAP_ECN_RFC4301_RFC6040_NORMAL_MODE) {
		nss_ppe_mapt_dentry_deinit();
		nss_ppe_mapt_warning("Invalid Encap ECN mode %u\n", encap_ecn_mode);
		return -1;
	}

	if (decap_ecn_mode > PPE_DRV_TUN_CMN_CTX_DECAP_ECN_RFC6040_MODE) {
		nss_ppe_mapt_dentry_deinit();
		nss_ppe_mapt_warning("Invalid Decap ECN mode %u\n", decap_ecn_mode);
		return -1;
	}

	/*
	 * Register net device notification for standard tunnel.
	 */
	register_netdevice_notifier(&nss_ppe_mapt_notifier);

	nss_ppe_mapt_trace("mapt PPE driver registered");
	return 0;
}

/*
 * nss_ppe_mapt_exit_module()
 *	Tunnel mapt module exit function
 */
void __exit nss_ppe_mapt_exit_module(void)
{
	/*
	 * Disable all the mapt connection in PPE tunnel driver
	 */
	ppe_tun_conf_accel(PPE_DRV_TUN_CMN_CTX_TYPE_MAPT, false);

	/*
	 * De-initialize debugfs.
	 */
	nss_ppe_mapt_dentry_deinit();

	/*
	 * Unregister net device notification for standard tunnel.
	 */
	unregister_netdevice_notifier(&nss_ppe_mapt_notifier);

	nss_ppe_mapt_info("mapt PPE module unloaded");
}

module_init(nss_ppe_mapt_init_module);
module_exit(nss_ppe_mapt_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS PPE mapt client driver");
