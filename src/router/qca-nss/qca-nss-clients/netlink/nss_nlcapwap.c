/*
 **************************************************************************
 * Copyright (c) 2015-2016,2018-2020 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022,2024 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#include <linux/debugfs.h>
#include <linux/if.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/netlink.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/workqueue.h>

#include <net/genetlink.h>
#include <net/sock.h>

#include <nss_api_if.h>
#include <nss_capwap.h>
#include <nss_capwapmgr.h>
#include <nss_capwap_user.h>
#include <nss_cmn.h>
#include <nss_dtlsmgr.h>
#include <nss_dtls_cmn.h>
#include <nss_nlcmn_if.h>
#include <nss_nl_if.h>
#include "nss_crypto_defines.h"
#include "nss_nl.h"
#include "nss_nlcapwap_if.h"
#include "nss_nlcapwap.h"
#include "nss_nlcmn_if.h"

/*
 * nss_nlcapwap_tunnel_stats
 *	Keeps track of the netlink capwap stats
 */
struct nss_nlcapwap_tunnel_stats {
	uint32_t ka_seq_fail;
	uint32_t tx_data_pkts;
	uint32_t rx_data_pkts;
	uint32_t tx_ka_pkts;
	uint32_t rx_ka_pkts;
};

/*
 * nss_nlcapwap_tunnel_keepalive
 *	Parameters needed for keepalive execution
 */
struct nss_nlcapwap_tunnel_keepalive {
	struct delayed_work work;
	uint32_t tx_seq;
	uint32_t rx_seq;
	uint16_t tun_id;
	atomic_t status;
};

/*
 * nss_nlcapwap_tunnel
 *	Stores the per tunnel data
 */
struct nss_nlcapwap_tunnel {
	struct nss_nlcapwap_tunnel_keepalive kalive;	/* Keepalive parameters */
	struct nss_nlcapwap_tunnel_stats stats;		/* Per tunnel netlink level stats */
	struct nss_nlcapwap_meta_header mh;		/* Stores meta header of tunnel */
};

/*
 * nss_nlcapwap_global_ctx
 *	Global context for capwap
 */
struct nss_nlcapwap_global_ctx {
	struct nss_nlcapwap_tunnel tun[NSS_CAPWAPMGR_MAX_TUNNELS];	/* Keepalive params per tunnel */
	unsigned long tun_bitmap[NSS_NLCAPWAP_MAX_TUNNEL_LONGS];	/* Bitmap to keep track of tunnel status */
	struct net_device *capwap_dev;	/* CAPWAP global device */
	struct dentry *dentry;		/* Debug entry to maintain netlink stats */
	atomic_t enable_perf;		/* Atomic variable to enable and disable perf */
	rwlock_t lock;			/* Lock variable for synchronization */
};

/*
 * nss_nlcapwap_app_hdr
 *	Custom header needed by sender and receiver
 */
struct nss_nlcapwap_app_hdr {
	uint32_t seq_num;		/* Seq number associated with packet */
	uint16_t tun_id;		/* Tunnel used for transmission */
	uint8_t res[2];			/* Reserved for padding */
};

/*
 * nss_nlcapwap_hdr
 *	capwap header used for dtls_keepalive packets
 */
struct nss_nlcapwap_hdr {
	uint8_t preamble;	/* 0=CAPWAP header, 1=CAPWAP DTLS header */

	/*
	 * 1-byte
	 */
	uint8_t rid1:3;

	/*
	 * rid1: 3 bits of a 5-bit field that contains the Radio ID number for
	 * this packet, whose value is between one (1) and 31. Given
	 * that MAC Addresses are not necessarily unique across physical
	 * radios in a WTP, the Radio Identifier (RID) field is used to
	 * indicate with which physical radio the message is associated.
	 */
	uint8_t hlen:5;
	/*
	 * hlen: A 5-bit field containing the length of the CAPWAP transport
	 * header in 4-byte words (similar to IP header length). This
	 * length includes the optional headers.
	 */

	/*
	 * 1-byte
	 */
	uint8_t T:1;		/* Type (1=802.11, 0=Other) */
	uint8_t wbid:5;
	/*
	 * wbid: A 5-bit field that is the wireless binding identifier. The
	 * identifier will indicate the type of wireless packet associated
	 * with the radio. The following values are defined:
	 * 0 - Reserved 1 - IEEE 802.11 2 - Reserved 3 - EPCGlobal [EPCGlobal]
	 */

	uint8_t rid2:2;		/* 2-bits of radio id -- look at rid1 */

	/*
	 * 1-byte
	 */
	uint8_t flags:3;	/* Not Used */
	uint8_t K:1;		/* 1=KeepAlive packet 0=Not keepalive packet */
	uint8_t M:1;		/* 1=MAC address is present, 0=not present */
	uint8_t W:1;		/* 1=wireless info present, 0=not present */
	uint8_t L:1;		/* 1=Last fragment, 0=Not the last fragment */
	uint8_t F:1;		/* 1=Fragmented, 0=Not fragmented */

	uint16_t frag_id;	/* Fragment ID */
	uint16_t frag_off;	/* 13-bit Offset of the fragment in 8 byte words */
				/* lower 3 bits are reserved & must be set to 0 */
} __attribute__((packed));

/*
 * Global capwap context variable
 */
static struct nss_nlcapwap_global_ctx global_ctx;

static int nss_nlcapwap_ops_create_tun(struct sk_buff *skb, struct genl_info *info);
static int nss_nlcapwap_ops_destroy_tun(struct sk_buff *skb, struct genl_info *info);
static int nss_nlcapwap_ops_update_mtu(struct sk_buff *skb, struct genl_info *info);
static int nss_nlcapwap_ops_dtls(struct sk_buff *skb, struct genl_info *info);
static int nss_nlcapwap_ops_perf(struct sk_buff *skb, struct genl_info *info);
static int nss_nlcapwap_ops_tx_packets(struct sk_buff *skb, struct genl_info *info);
static int nss_nlcapwap_ops_meta_header(struct sk_buff *skb, struct genl_info *info);
static int nss_nlcapwap_ops_ip_flow(struct sk_buff *skb, struct genl_info *info);
static int nss_nlcapwap_ops_keepalive(struct sk_buff *skb, struct genl_info *info);
static int nss_nlcapwap_ops_get_stats(struct sk_buff *skb, struct genl_info *info);

/*
 * nss_nlcapwap_family_mcgrp
 *	Multicast group for sending message status & events
 */
static const struct genl_multicast_group nss_nlcapwap_family_mcgrp[] = {
	{.name = NSS_NLCAPWAP_MCAST_GRP},
};

/*
 * nss_nlcapwap_cmd_ops
 *	Operation table called by the generic netlink layer based on the command
 */
struct genl_ops nss_nlcapwap_cmd_ops[] = {
	{.cmd = NSS_NLCAPWAP_CMD_TYPE_CREATE_TUN, .doit = nss_nlcapwap_ops_create_tun,},
	{.cmd = NSS_NLCAPWAP_CMD_TYPE_DESTROY_TUN, .doit = nss_nlcapwap_ops_destroy_tun,},
	{.cmd = NSS_NLCAPWAP_CMD_TYPE_UPDATE_MTU, .doit = nss_nlcapwap_ops_update_mtu,},
	{.cmd = NSS_NLCAPWAP_CMD_TYPE_DTLS, .doit = nss_nlcapwap_ops_dtls,},
	{.cmd = NSS_NLCAPWAP_CMD_TYPE_PERF, .doit = nss_nlcapwap_ops_perf,},
	{.cmd = NSS_NLCAPWAP_CMD_TYPE_TX_PACKETS, .doit = nss_nlcapwap_ops_tx_packets,},
	{.cmd = NSS_NLCAPWAP_CMD_TYPE_META_HEADER, .doit = nss_nlcapwap_ops_meta_header,},
	{.cmd = NSS_NLCAPWAP_CMD_TYPE_IP_FLOW, .doit = nss_nlcapwap_ops_ip_flow,},
	{.cmd = NSS_NLCAPWAP_CMD_TYPE_KEEPALIVE, .doit = nss_nlcapwap_ops_keepalive,},
	{.cmd = NSS_STATS_EVENT_NOTIFY, .doit = nss_nlcapwap_ops_get_stats,},
};

/*
 * nss_nlcapwap_family
 *	Capwap family definition
 */
struct genl_family nss_nlcapwap_family = {
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 9, 0))
	.id = GENL_ID_GENERATE,				/* Auto generate ID */
#endif
	.name = NSS_NLCAPWAP_FAMILY,			/* family name string */
	.hdrsize = sizeof(struct nss_nlcapwap_rule),	/* NSS NETLINK capwap rule */
	.version = NSS_NL_VER,				/* Set it to NSS_NL_VER version */
	.maxattr = NSS_NLCAPWAP_CMD_TYPE_MAX,		/* maximum commands supported */
	.netnsok = true,
	.pre_doit = NULL,
	.post_doit = NULL,
	.ops = nss_nlcapwap_cmd_ops,
	.n_ops = ARRAY_SIZE(nss_nlcapwap_cmd_ops),
	.mcgrps = nss_nlcapwap_family_mcgrp,
	.n_mcgrps = ARRAY_SIZE(nss_nlcapwap_family_mcgrp)
};

/*
 * nss_nlcapwap_process_notify()
 *	process notification messages from NSS
 */
static int nss_nlcapwap_process_notify(struct notifier_block *nb, unsigned long val, void *data)
{
	struct sk_buff *skb;
	struct nss_capwap_stats_notification *nss_stats, *nl_stats;

	nss_stats = (struct nss_capwap_stats_notification *)data;
	skb = nss_nl_new_msg(&nss_nlcapwap_family, NSS_NLCMN_SUBSYS_CAPWAP);
	if (!skb) {
		nss_nl_error("unable to allocate NSS_NLCAPWAP event\n");
		return NOTIFY_DONE;
	}

	nl_stats = nss_nl_get_data(skb);
	memcpy(nl_stats, nss_stats, sizeof(struct nss_capwap_stats_notification));
	nss_nl_mcast_event(&nss_nlcapwap_family, skb);

	return NOTIFY_DONE;
}

/*
 * device call back handler for capwap from NSS
 */
static struct notifier_block nss_capwap_stats_notifier_nb = {
	.notifier_call = nss_nlcapwap_process_notify,
};

/*
 * nss_nlcapwap_get_tun_by_index()
 *	Returns the tunnel context based on index passed
 */
static struct nss_nlcapwap_tunnel *nss_nlcapwap_get_tun_by_index(int idx)
{
	if (idx >= NSS_CAPWAPMGR_MAX_TUNNELS) {
		nss_nl_error("Index value out of bound: %d\n", idx);
		return NULL;
	}

	return test_bit(idx, global_ctx.tun_bitmap) ? &global_ctx.tun[idx] : NULL;
}

/*
 * nss_nlcapwap_rx_handler()
 *	Capwap netdev rx handler
 */
static rx_handler_result_t nss_nlcapwapmgr_rx_handler(struct sk_buff **pskb)
{
	struct nss_capwap_metaheader *mh;
	struct nss_nlcapwap_tunnel *tun;
	struct sk_buff *skb = *pskb;
	uint32_t matched = 0;
	uint8_t *data;
	int len;
	int i;

	len = skb->len;
	mh = (struct nss_capwap_metaheader *)skb->data;

	/*
	 * We need to pull the meta header for the payload start
	 */
	if (!atomic_read(&global_ctx.enable_perf)) {
		data = skb_pull(skb, sizeof(*mh) + sizeof(struct ethhdr));
		/*
		 * Test bytes with known pattern
		 */
		for (i = 0; i < skb->len; i++, data++) {
			matched += (*data == NSS_NLCAPWAP_DATA);
		}
	}

	nss_nl_info("RX packet for tun(%d), len(%d) matched(%d)\n", mh->tunnel_id, len, matched);

	write_lock_bh(&global_ctx.lock);
	tun = nss_nlcapwap_get_tun_by_index(mh->tunnel_id);
	if (!tun) {
		nss_nl_error("%px: Could not find tunnel associated with index: %d\n", skb, mh->tunnel_id);
		write_unlock_bh(&global_ctx.lock);
		goto free;
	}

	tun->stats.rx_data_pkts++;
	write_unlock_bh(&global_ctx.lock);

free:
	dev_kfree_skb_any(skb);
	return RX_HANDLER_CONSUMED;
}

/*
 * nss_nlcapwap_tx_keepalive()
 *	Handler for sending capwap keepalive frames
 */
static int nss_nlcapwap_tx_keepalive(struct nss_nlcapwap_tunnel_keepalive *kp)
{
	struct nss_nlcapwap_app_hdr *apph;
	struct nss_nlcapwap_tunnel *tun;
	struct net_device *capwap_dev;
	struct net_device *dtls_dev;
	struct nss_nlcapwap_hdr *ch;
	struct sk_buff *skb;
	size_t align_offset;
	size_t skb_sz;

	/*
	 * Check if dtls is enabled for the tunnel
	 */
	dtls_dev = nss_capwapmgr_get_dtls_netdev(global_ctx.capwap_dev, kp->tun_id);
	if (!dtls_dev) {
		nss_nl_error("%px: DTLS net_device not found for capwap_dev(%s)\n", &kp, capwap_dev->name);
		return -ENODEV;
	}

	/*
	 * Allocate a new skb
	 */
	skb_sz = NSS_NLCAPWAP_MAX_HEADROOM + NSS_NLCAPWAP_KALIVE_PAYLOAD_SZ + NSS_NLCAPWAP_MAX_TAILROOM;
	skb_sz += SMP_CACHE_BYTES;

	skb = dev_alloc_skb(skb_sz);
	if (!skb) {
		nss_nl_error("%px: Could not allocate a skb of size(%zu)\n", kp, skb_sz);
		dev_put(dtls_dev);
		return -ENOMEM;
	}

	align_offset = PTR_ALIGN(skb->data, SMP_CACHE_BYTES) - skb->data;
	skb_reserve(skb, NSS_NLCAPWAP_MAX_HEADROOM + align_offset);

	/*
	 * Set the queue mapping to highest priority queue
	 */
	skb_set_queue_mapping(skb, 1);

	/*
	 * Initialize the capwap header with zero
	 */
	ch = (struct nss_nlcapwap_hdr *)skb_put(skb, sizeof(*ch));
	memset(ch, 0, sizeof(*ch));

	/*
	 * Set the keepalive timer bit
	 */
	ch->K = 1;

	/*
	 * Fill the packet data with msg_type and tun_id
	 */
	apph = (struct nss_nlcapwap_app_hdr *)skb_put(skb, sizeof(*apph));

	write_lock_bh(&global_ctx.lock);
	tun = nss_nlcapwap_get_tun_by_index(kp->tun_id);
	if (!tun) {
		write_unlock_bh(&global_ctx.lock);
		nss_nl_error("%px: Could not find tunnel associated with index: %d\n", kp, kp->tun_id);
		return -ENODEV;
	}

	apph->tun_id = kp->tun_id;
	apph->seq_num = kp->tx_seq;

	kp->tx_seq++;
	tun->stats.tx_ka_pkts++;
	write_unlock_bh(&global_ctx.lock);

	BUG_ON(!IS_ALIGNED((unsigned long)skb->data, sizeof(uint32_t)));

	if (dtls_dev->netdev_ops->ndo_start_xmit(skb, dtls_dev) != NETDEV_TX_OK) {
		dev_kfree_skb_any(skb);
		return -EBUSY;
	}

	nss_nl_info("%px: Keepalive packet sent\n", dtls_dev);
	dev_put(dtls_dev);
	return 0;
}

/*
 * nss_nlcapwap_keepalive()
 *	Sends keepalive packet at regular interval
 */
static void nss_nlcapwap_keepalive(struct work_struct *work)
{
	struct delayed_work *dwork = (struct delayed_work *)work;
	struct nss_nlcapwap_tunnel_keepalive *kp = container_of(dwork, struct nss_nlcapwap_tunnel_keepalive, work);

	nss_nlcapwap_tx_keepalive(kp);
	if (atomic_read(&kp->status)) {
		   schedule_delayed_work(dwork, NSS_NLCAPWAP_KALIVE_TIMER_MSECS);
	}
}

/*
 * nss_nlcapwap_tun_init()
 *	Initialize the tunnel
 */
static void nss_nlcapwap_tun_init(struct nss_nlcapwap_tunnel *tun, uint16_t tun_id)
{
	memset(tun, 0, sizeof(*tun));
	tun->kalive.tun_id = tun_id;
	INIT_DELAYED_WORK(&tun->kalive.work, nss_nlcapwap_keepalive);
}

/*
 * nss_nlcapwap_tun_deinit()
 *	Deinitializes the tunnel
 */
static void nss_nlcapwap_tun_deinit(struct nss_nlcapwap_tunnel *tun)
{
	/*
	 * Flush the delayed work if its enabled for the tunnel
	 */
	if (!atomic_read(&tun->kalive.status)) {
		return;
	}

	/*
	 * De-initialize the keepalive context for tun_id
	 */
	atomic_set(&tun->kalive.status, 0);
	cancel_delayed_work(&tun->kalive.work);
}

/*
 * nss_nlcapwap_destroy_tun()
 *	Destroys tunnel based on tun_id
 */
static int nss_nlcapwap_destroy_tun(uint16_t tun_id)
{
	struct net_device *capwap_dev;
	nss_capwapmgr_status_t status;

	/*
	 * Get the capwap netdev reference
	 */
	capwap_dev = global_ctx.capwap_dev;
	if (!capwap_dev) {
		nss_nl_error("CAPWAP net_device not found\n");
		return -ENODEV;
	}

	status = nss_capwapmgr_disable_tunnel(capwap_dev, tun_id);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_nl_error("Unable to disable the tunnel: %d\n", tun_id);
		return -EAGAIN;
	}

	status = nss_capwapmgr_tunnel_destroy(capwap_dev, tun_id);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_nl_error("Unable to destroy the tunnel: %d\n", tun_id);
		nss_capwapmgr_enable_tunnel(capwap_dev, tun_id);
		return -ENODEV;
	}

	return 0;
}

/*
 * nss_nlcapwap_data_cb()
 *	Data callback for capwap
 */
static void nss_nlcapwap_data_cb(void *app_data, struct sk_buff *skb)
{
	struct nss_nlcapwap_app_hdr *apph;
	struct nss_dtlsmgr_metadata *mh;
	struct nss_nlcapwap_tunnel *tun;
	uint32_t exp_seq;
	uint32_t tun_id;

	/*
	 * Move the pointer to start of custom header
	 */
	apph = (struct nss_nlcapwap_app_hdr *)skb_pull(skb, sizeof(*mh) + 8);
	tun_id = apph->tun_id;

	/*
	 * Aquire lock and check if the seq num in packet is what we expected as rx_seq
	 */
	write_lock_bh(&global_ctx.lock);
	tun = nss_nlcapwap_get_tun_by_index(tun_id);
	if (!tun) {
		write_unlock_bh(&global_ctx.lock);
		nss_nl_error("%px: Could not find tunnel associated with index: %d\n", skb, tun_id);
		return;
	}

	/*
	 * Check if the received sequence number matches with the expected sequence number
	 */
	exp_seq = tun->kalive.rx_seq;
	tun->stats.rx_ka_pkts++;
	tun->kalive.rx_seq = apph->seq_num + 1;
	tun->stats.ka_seq_fail += (apph->seq_num != exp_seq);
	write_unlock_bh(&global_ctx.lock);

	/*
	 * TODO: If, we have exceeded ka_seq_fail threshold then destroy tunnel
	 */
	nss_nl_info("%px: RX DTLS pkt len:%d, tun_id:%d, seq_num:%u\n", skb, skb->len, apph->tun_id, apph->seq_num);
	dev_kfree_skb_any(skb);
}

/*
 * nss_nlcapwap_dtls_configure()
 *	Common handler for v4 and v6 capwap-dtls configuration
 */
static void nss_nlcapwap_dtls_configure(struct nss_dtlsmgr_config *dcfg, struct nss_nlcapwap_rule *nl_rule)
{
	uint8_t algo = nl_rule->msg.dtls.encap.crypto.algo;

	dcfg->flags = nl_rule->msg.dtls.flags;
	dcfg->flags |= NSS_DTLSMGR_HDR_CAPWAP;

	if (algo == NSS_DTLSMGR_ALGO_AES_GCM) {
		dcfg->flags |= NSS_DTLSMGR_CIPHER_MODE_GCM;
	}

	/*
	 * Encap configuration
	 */
	dcfg->app_data = NULL;
	dcfg->notify = NULL;
	dcfg->data = nss_nlcapwap_data_cb;

	memcpy(&dcfg->encap, &nl_rule->msg.dtls.encap, sizeof(struct nss_dtlsmgr_encap_config));
	memcpy(&dcfg->decap, &nl_rule->msg.dtls.decap, sizeof(struct nss_dtlsmgr_decap_config));
}

/*
 * nss_nlcapwap_create_tun_ipv4_config()
 *	Configures the common rule for rx and tx.
 */
static void nss_nlcapwap_create_tun_ipv4_config(struct nss_nlcapwap_rule *nl_rule,
		struct nss_capwap_rule_msg *capwap_rule, struct nss_dtlsmgr_config *dtls_ipv4,
		struct nss_ipv4_create *ipv4)
{
	struct net_device *wan_ndev;
	uint32_t features = 0;
	uint32_t if_num;

	/*
	 * Initialize the msgs
	 */
	memset(ipv4, 0, sizeof(*ipv4));
	memset(capwap_rule, 0, sizeof(*capwap_rule));

	/*
	 * Configure CAPWAP rule
	 */
	capwap_rule->encap.path_mtu = htonl(nl_rule->msg.create.rule.encap.path_mtu);
	capwap_rule->decap.reassembly_timeout = htonl(nl_rule->msg.create.rule.decap.reassembly_timeout);
	capwap_rule->decap.max_fragments = htonl(nl_rule->msg.create.rule.decap.max_fragments);
	capwap_rule->decap.max_buffer_size = htonl(nl_rule->msg.create.rule.decap.max_buffer_size);
	capwap_rule->stats_timer = htonl(nl_rule->msg.create.rule.stats_timer);
	capwap_rule->l3_proto = NSS_CAPWAP_TUNNEL_IPV4;
	if (nl_rule->msg.create.rule.which_udp == IPPROTO_UDP) {
		capwap_rule->which_udp = NSS_CAPWAP_TUNNEL_UDP;
		ipv4->protocol = IPPROTO_UDP;
	} else {
		capwap_rule->which_udp = NSS_CAPWAP_TUNNEL_UDPLite;
		ipv4->protocol = IPPROTO_UDPLITE;
	}

	if (nl_rule->msg.create.inner_trustsec_en) {
		nss_nl_info("Enabling INNER TRUSTSEC in rule\n");
		features |= NSS_CAPWAPMGR_FEATURE_INNER_TRUSTSEC_ENABLED;
	}

	if (nl_rule->msg.create.outer_trustsec_en) {
		nss_nl_info("Enabling OUTER TRUSTSEC in rule\n");
		features |= NSS_CAPWAPMGR_FEATURE_OUTER_TRUSTSEC_ENABLED;
	}

	if (nl_rule->msg.create.wireless_qos_en) {
		nss_nl_info("Enabling WIRELESS QOS\n");
		features |= NSS_CAPWAPMGR_FEATURE_WIRELESS_QOS_ENABLED;
	}

	capwap_rule->enabled_features = features;
	capwap_rule->outer_sgt_value = nl_rule->msg.create.rule.outer_sgt_value;

	/*
	 * Configure IPv4 rule
	 */
	wan_ndev = dev_get_by_name(&init_net, &nl_rule->msg.create.gmac_ifname[0]);
	if (!wan_ndev) {
		nss_nl_info("Can't find %s netdev\n", nl_rule->msg.create.gmac_ifname);
		return;
	}

	if_num = nss_cmn_get_interface_number_by_dev(wan_ndev);
	nss_nl_info("CAPWAP on %s, if_num is %d\n", nl_rule->msg.create.gmac_ifname, if_num);
	ipv4->src_interface_num = if_num;
	ipv4->dest_interface_num = NSS_NLCAPWAP_WAN_IFNUM;
	ipv4->from_mtu = wan_ndev->mtu;
	ipv4->to_mtu = wan_ndev->mtu;

	ipv4->src_ip = nl_rule->msg.create.rule.encap.src_ip.ip.ipv4;
	ipv4->src_port = nl_rule->msg.create.rule.encap.src_port;
	ipv4->src_ip_xlate = nl_rule->msg.create.rule.encap.src_ip.ip.ipv4;
	ipv4->src_port_xlate = nl_rule->msg.create.rule.encap.src_port;

	ipv4->dest_ip = nl_rule->msg.create.rule.encap.dest_ip.ip.ipv4;
	ipv4->dest_port = nl_rule->msg.create.rule.encap.dest_port;
	ipv4->dest_ip_xlate = nl_rule->msg.create.rule.encap.dest_ip.ip.ipv4;
	ipv4->dest_port_xlate = nl_rule->msg.create.rule.encap.dest_port;

	memcpy(ipv4->src_mac, nl_rule->msg.create.gmac_ifmac, sizeof(ipv4->src_mac));

	ipv4->in_vlan_tag[0] = NSS_NLCAPWAP_VLAN_TAG_INVALID;
	ipv4->in_vlan_tag[1] = NSS_NLCAPWAP_VLAN_TAG_INVALID;
	ipv4->out_vlan_tag[0] = NSS_NLCAPWAP_VLAN_TAG_INVALID;
	ipv4->out_vlan_tag[1] = NSS_NLCAPWAP_VLAN_TAG_INVALID;
	dev_put(wan_ndev);
}

/*
 * nss_nlcapwap_create_tun_ipv6_config()
 *	Configures the common rule for rx and tx.
 */
static void nss_nlcapwap_create_tun_ipv6_config(struct nss_nlcapwap_rule *nl_rule,
		struct nss_capwap_rule_msg *capwap_rule, struct nss_dtlsmgr_config *dtls_ipv6,
		struct nss_ipv6_create *ipv6)
{
	struct net_device *wan_ndev;
	uint32_t features = 0;
	uint32_t if_num;

	/*
	 * Initialize the msgs
	 */
	memset(ipv6, 0, sizeof (struct nss_ipv6_create));
	memset(capwap_rule, 0, sizeof(struct nss_capwap_rule_msg));

	/*
	 * Configuring capwap rule
	 */
	capwap_rule->encap.path_mtu = htonl(nl_rule->msg.create.rule.encap.path_mtu);
	capwap_rule->decap.reassembly_timeout = htonl(nl_rule->msg.create.rule.decap.reassembly_timeout);
	capwap_rule->decap.max_fragments = htonl(nl_rule->msg.create.rule.decap.max_fragments);
	capwap_rule->decap.max_buffer_size = htonl(nl_rule->msg.create.rule.decap.max_buffer_size);
	capwap_rule->stats_timer = htonl(nl_rule->msg.create.rule.stats_timer);
	capwap_rule->l3_proto = NSS_CAPWAP_TUNNEL_IPV6;

	if (nl_rule->msg.create.rule.which_udp == IPPROTO_UDP) {
		capwap_rule->which_udp = NSS_CAPWAP_TUNNEL_UDP;
		ipv6->protocol = IPPROTO_UDP;
	} else {
		capwap_rule->which_udp = NSS_CAPWAP_TUNNEL_UDPLite;
		ipv6->protocol = IPPROTO_UDPLITE;
	}

	if (nl_rule->msg.create.inner_trustsec_en) {
		nss_nl_info("Enabling INNER TRUSTSEC in rule\n");
		features |= NSS_CAPWAPMGR_FEATURE_INNER_TRUSTSEC_ENABLED;
	}

	if (nl_rule->msg.create.outer_trustsec_en) {
		nss_nl_info("Enabling OUTER TRUSTSEC in rule\n");
		features |= NSS_CAPWAPMGR_FEATURE_OUTER_TRUSTSEC_ENABLED;
	}

	if (nl_rule->msg.create.wireless_qos_en) {
		nss_nl_info("Enabling WIRELESS QOS\n");
		features |= NSS_CAPWAPMGR_FEATURE_WIRELESS_QOS_ENABLED;
	}

	capwap_rule->enabled_features = features;
	capwap_rule->outer_sgt_value = nl_rule->msg.create.rule.outer_sgt_value;

	/*
	 * Configure IPv6 rule
	 */
	wan_ndev = dev_get_by_name(&init_net, &nl_rule->msg.create.gmac_ifname[0]);
	if (!wan_ndev) {
		nss_nl_info("Can't find %s netdev\n", nl_rule->msg.create.gmac_ifname);
		return;
	}

	if_num = nss_cmn_get_interface_number_by_dev(wan_ndev);
	nss_nl_info("CAPWAP on %s, if_num is %d\n", nl_rule->msg.create.gmac_ifname, if_num);
	ipv6->src_interface_num = if_num;
	ipv6->dest_interface_num = NSS_NLCAPWAP_WAN_IFNUM;
	ipv6->from_mtu = wan_ndev->mtu;
	ipv6->to_mtu = wan_ndev->mtu;
	memcpy(ipv6->src_ip, nl_rule->msg.create.rule.encap.src_ip.ip.ipv6, sizeof(ipv6->src_ip));
	memcpy(ipv6->dest_ip, nl_rule->msg.create.rule.encap.dest_ip.ip.ipv6, sizeof(ipv6->dest_ip));
	memcpy(&ipv6->src_mac[0], &nl_rule->msg.create.gmac_ifmac[0], sizeof(ipv6->src_mac));
	ipv6->src_port = nl_rule->msg.create.rule.encap.src_port;
	ipv6->dest_port = nl_rule->msg.create.rule.encap.dest_port;
	ipv6->in_vlan_tag[0] = NSS_NLCAPWAP_VLAN_TAG_INVALID;
	ipv6->in_vlan_tag[1] = NSS_NLCAPWAP_VLAN_TAG_INVALID;
	ipv6->out_vlan_tag[0] = NSS_NLCAPWAP_VLAN_TAG_INVALID;
	ipv6->out_vlan_tag[1] = NSS_NLCAPWAP_VLAN_TAG_INVALID;
	dev_put(wan_ndev);
}

/*
 * nss_nlcapwap_ops_create_tun()
 *	Handler for creating tunnel
 */
static int nss_nlcapwap_ops_create_tun(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_capwap_rule_msg capwap_rule;
	struct nss_dtlsmgr_config dtls_ipv4;
	struct nss_dtlsmgr_config dtls_ipv6;
	struct nss_nlcapwap_rule *nl_rule;
	struct nss_ipv4_create ipv4_rule;
	struct nss_ipv6_create ipv6_rule;
	struct nss_nlcapwap_tunnel *tun;
	struct net_device *capwap_dev;
	struct nss_nlcmn *nl_cm;
	uint16_t tun_id;
	int ret = 0;

	/*
	 * Get the tunnel_id. We only create a new tunnel if max limit is not exceeded
	 */
	write_lock_bh(&global_ctx.lock);
	tun_id = find_first_zero_bit(global_ctx.tun_bitmap, NSS_CAPWAPMGR_MAX_TUNNELS);
	if (tun_id >= NSS_CAPWAPMGR_MAX_TUNNELS) {
		nss_nl_error("All tunnels exhausted(%d), no free tunnel found.\n", tun_id);
		return -ENOSPC;
	}

	tun = &global_ctx.tun[tun_id];
	nss_nlcapwap_tun_init(tun, tun_id);

	set_bit(tun_id, global_ctx.tun_bitmap);
	write_unlock_bh(&global_ctx.lock);

	/*
	 * Get the capwap netdev reference
	 */
	capwap_dev = global_ctx.capwap_dev;
	if (!capwap_dev) {
		nss_nl_error("Failed to find CAPWAP netdevice\n");
		return -ENODEV;
	}

	/*
	 * Extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlcapwap_family, info, NSS_NLCAPWAP_CMD_TYPE_CREATE_TUN);
	if (!nl_cm) {
		nss_nl_error("Unable to extract create tunnel data\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlcapwap_rule, cm);

	/*
	 * Needed for 802.3 to 802.11 conversion
	 */
	memcpy(capwap_rule.bssid, nl_rule->msg.create.rule.bssid, sizeof(capwap_rule.bssid));
	capwap_rule.outer_sgt_value = nl_rule->msg.create.rule.outer_sgt_value;

	/*
	 * Create tunnel based on ip version
	 */
	if (nl_rule->msg.create.rule.l3_proto == NSS_NLCAPWAP_IP_VERS_4) {
		nss_nlcapwap_create_tun_ipv4_config(nl_rule, &capwap_rule, &dtls_ipv4, &ipv4_rule);
		/*
		 * Create CAPWAP IPv4 tunnel
		 */
		ret = nss_capwapmgr_ipv4_tunnel_create(capwap_dev, tun_id, &ipv4_rule, &capwap_rule, &dtls_ipv4);
		if (ret != 0) {
			nss_nl_error("Unable to create tunnel(%d), status(%d)\n", tun_id, ret);
			return -EAGAIN;
		}

		nss_nl_info("Created IPv4 tunnel: src:%pI4h(%d) dst:%pI4h(%d) p:%d\n\n",
				&ipv4_rule.src_ip, ipv4_rule.src_port, &ipv4_rule.dest_ip,
				ipv4_rule.dest_port, ipv4_rule.protocol);
	} else {
		nss_nlcapwap_create_tun_ipv6_config(nl_rule, &capwap_rule, &dtls_ipv6, &ipv6_rule);
		/*
		 * Create CAPWAP IPv6 tunnel
		 */
		ret = nss_capwapmgr_ipv6_tunnel_create(capwap_dev, tun_id, &ipv6_rule, &capwap_rule, &dtls_ipv6);
		if (ret != 0) {
			nss_nl_error("Unable to create tunnel: %d\n", tun_id);
			return -EAGAIN;
		}

		nss_nl_info("Created IPv6 tunnel: src:%pI6(%d), dst:%pI6(%d), proto:%d\n\n",
				ipv6_rule.src_ip, ipv6_rule.src_port, ipv6_rule.dest_ip, ipv6_rule.dest_port,
				ipv6_rule.protocol);
	}

	nss_capwapmgr_change_version(capwap_dev, tun_id, NSS_CAPWAP_VERSION_V2);
	nss_capwapmgr_enable_tunnel(capwap_dev, tun_id);

	nss_nl_info("Successfully created tunnel %d\n", tun_id);
	return 0;
}

/*
 * nss_nlcapwap_ops_destroy_tun()
 *	Handler to destroy tunnel
 */
static int nss_nlcapwap_ops_destroy_tun(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nlcapwap_rule *nl_rule;
	struct nss_nlcapwap_tunnel *tun;
	struct nss_nlcmn *nl_cm;
	uint16_t tun_id;
	int ret;

	/*
	 * Extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlcapwap_family, info, NSS_NLCAPWAP_CMD_TYPE_DESTROY_TUN);
	if (!nl_cm) {
		nss_nl_error("Unable to extract destroy tunnel data\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlcapwap_rule, cm);

	/*
	 * Get the tunnel id
	 */
	tun_id = nl_rule->msg.destroy.tun_id;
	if (tun_id >= NSS_CAPWAPMGR_MAX_TUNNELS) {
		nss_nl_error("Not a valid tunnel_id: %d\n", tun_id);
		return -ENODEV;
	}

	write_lock_bh(&global_ctx.lock);
	tun = nss_nlcapwap_get_tun_by_index(tun_id);
	if (!tun) {
		write_unlock_bh(&global_ctx.lock);
		nss_nl_error("%px: Could not find tunnel associated with index: %d\n", nl_rule, tun_id);
		return -ENODEV;
	}

	nss_nlcapwap_tun_deinit(tun);
	clear_bit(tun_id, global_ctx.tun_bitmap);
	write_unlock_bh(&global_ctx.lock);

	/*
	 * Destroy the corresponding tunnel
	 */
	ret = nss_nlcapwap_destroy_tun(tun_id);
	if (ret) {
		nss_nl_error("Unable to set the tunnel status to 0 for tun: %d\n", tun_id);
		return -EINVAL;
	}

	nss_nl_info("Successfully destroyed %d tunnel\n", nl_rule->msg.destroy.tun_id);
	return 0;
}

/*
 * nss_nlcapwap_ops_update_mtu()
 *	Handler for updating mtu
 */
static int nss_nlcapwap_ops_update_mtu(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nlcapwap_rule *nl_rule;
	struct net_device *capwap_dev;
	nss_capwapmgr_status_t status;
	struct nss_nlcmn *nl_cm;
	uint16_t tun_id;
	uint32_t mtu;

	/*
	 * Get the capwap netdev reference
	 */
	capwap_dev = global_ctx.capwap_dev;
	if (!capwap_dev) {
		nss_nl_error("Failed to find CAPWAP netdevice\n");
		return -ENODEV;
	}

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlcapwap_family, info, NSS_NLCAPWAP_CMD_TYPE_UPDATE_MTU);
	if (!nl_cm) {
		nss_nl_error("Unable to extract update mtu data\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlcapwap_rule, cm);

	/*
	 * Update the path_mtu of the corresponding tunnel
	 */
	tun_id = nl_rule->msg.update_mtu.tun_id;
	if (tun_id >= NSS_CAPWAPMGR_MAX_TUNNELS) {
		nss_nl_error("Not a valid tunnel_id: %d\n", tun_id);
		return -ENODEV;
	}

	mtu = nl_rule->msg.update_mtu.mtu.path_mtu;

	status = nss_capwapmgr_update_path_mtu(capwap_dev, tun_id, mtu);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_nl_error("Unable to update the mtu of the %d tunnel: %d\n", tun_id, mtu);
		return -EINVAL;
	}

	nss_nl_info("Successfully updated the mtu of the %d tunnel.\n", tun_id);
	return 0;
}

/*
 * nss_nlcapwap_ops_dtls()
 *	Handler for dtls enable or disable command
 */
static int nss_nlcapwap_ops_dtls(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_dtlsmgr_config dtls_config;
	struct nss_nlcapwap_rule *nl_rule;
	struct net_device *capwap_dev;
	nss_capwapmgr_status_t status;
	struct nss_nlcmn *nl_cm;
	uint16_t tun_id;

	/*
	 * Get the capwap netdev reference
	 */
	capwap_dev = global_ctx.capwap_dev;
	if (!capwap_dev) {
		nss_nl_error("Failed to find CAPWAP netdevice\n");
		return -ENODEV;
	}

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlcapwap_family, info, NSS_NLCAPWAP_CMD_TYPE_DTLS);
	if (!nl_cm) {
		nss_nl_error("Unable to extract dtls data\n");
		return -EINVAL;
	}

	/*
	 * Disabling dtls for capwap
	 */
	nl_rule = container_of(nl_cm, struct nss_nlcapwap_rule, cm);

	tun_id = nl_rule->msg.dtls.tun_id;
	if (tun_id >= NSS_CAPWAPMGR_MAX_TUNNELS) {
		nss_nl_error("Not a valid tunnel_id: %d\n", tun_id);
		return -ENODEV;
	}

	if (!nl_rule->msg.dtls.enable_dtls) {
		status = nss_capwapmgr_disable_tunnel(capwap_dev, tun_id);
		if (status != NSS_CAPWAPMGR_SUCCESS) {
			nss_nl_error("Not able to disable tunnel %d\n", tun_id);
			return -EBUSY;
		}

		status = nss_capwapmgr_configure_dtls(capwap_dev, tun_id, 0, NULL);
		if (status != NSS_CAPWAPMGR_SUCCESS) {
			nss_nl_error("Not able to disable dtls for tunnel(%d)\n", tun_id);
			nss_capwapmgr_enable_tunnel(capwap_dev, tun_id);
			return -EINVAL;
		}

		status = nss_capwapmgr_enable_tunnel(capwap_dev, tun_id);
		if (status != NSS_CAPWAPMGR_SUCCESS) {
			nss_nl_error("Not able to enable tunnel %d\n", tun_id);
			return -EINVAL;
		}

		nss_nl_info("Succesfully disabled dtls for capwap tunnel %d\n", tun_id);
		return 0;
	}

	nss_nl_info("Enabling DTLS for tunnel %d\n", tun_id);

	/*
	 * Fill DTLS configuration data
	 */
	nss_nlcapwap_dtls_configure(&dtls_config, nl_rule);

	/*
	 * Enabling dtls for capwap tunnel
	 */
	status = nss_capwapmgr_disable_tunnel(capwap_dev, tun_id);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_nl_error("Not able to disable tunnel %d\n", tun_id);
		return -EBUSY;
	}

	status = nss_capwapmgr_configure_dtls(capwap_dev, tun_id, 1, &dtls_config);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_nl_error("Not able to enable dtls for tunnel %d\n", tun_id);
		nss_capwapmgr_enable_tunnel(capwap_dev, tun_id);
		return -EINVAL;
	}

	status = nss_capwapmgr_enable_tunnel(capwap_dev, tun_id);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_nl_error("Not able to enable tunnel %d\n", tun_id);
		return -EINVAL;
	}

	nss_nl_info("Successfully enabled dtls for the capwap tunnel: %d\n", tun_id);
	return 0;
}

/*
 * nss_nlcapwap_ops_perf()
 *	Handler for enabling or disabling perf
 */
static int nss_nlcapwap_ops_perf(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nlcapwap_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	bool perf_en;

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlcapwap_family, info, NSS_NLCAPWAP_CMD_TYPE_PERF);
	if (!nl_cm) {
		nss_nl_error("Unable to extract perf parameter.\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlcapwap_rule, cm);
	perf_en = !!nl_rule->msg.perf.perf_en;

	atomic_set(&global_ctx.enable_perf, perf_en);
	nss_nl_info("Successfully %s performance\n", perf_en ? "enabled" : "disabled");

	return 0;
}

/*
 * nss_nlcapwap_ops_ip_flow()
 *	Handler for adding or deleting ip flow rule
 */
static int nss_nlcapwap_ops_ip_flow(struct sk_buff *skb, struct genl_info *info)
{
	nss_capwapmgr_status_t status;
	struct nss_nlcapwap_rule *nl_rule;
	struct net_device *capwap_dev;
	struct nss_nlcmn *nl_cm;

	/*
	 * Get the capwap netdev reference
	 */
	capwap_dev = global_ctx.capwap_dev;
	if (!capwap_dev) {
		nss_nl_error("Failed to find CAPWAP netdevice\n");
		return -ENODEV;
	}

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlcapwap_family, info, NSS_NLCAPWAP_CMD_TYPE_IP_FLOW);
	if (!nl_cm) {
		nss_nl_error("Unable to extract ip flow rule.\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlcapwap_rule, cm);

	/*
	 * Add flow rule
	 */
	if (nl_rule->msg.ip_flow.ip_flow_mode == NSS_NLCAPWAP_IP_FLOW_MODE_ADD) {
		status = nss_capwapmgr_add_flow_rule(capwap_dev, nl_rule->msg.ip_flow.tun_id,
				nl_rule->msg.ip_flow.flow.ip_version, nl_rule->msg.ip_flow.flow.protocol,
				nl_rule->msg.ip_flow.flow.src_ip, nl_rule->msg.ip_flow.flow.dst_ip,
				nl_rule->msg.ip_flow.flow.src_port, nl_rule->msg.ip_flow.flow.dst_port,
				nl_rule->msg.ip_flow.flow.flow_attr.flow_id);
		if (status != NSS_CAPWAPMGR_SUCCESS) {
			nss_nl_error("Unable to add flow rule\n");
			return -EAGAIN;
		}

		nss_nl_info("Succesfully added flow rule for tunnel %d\n",
				nl_rule->msg.ip_flow.tun_id);
		return 0;
	}

	/*
	 * Delete existing flow rule
	 */
	status = nss_capwapmgr_del_flow_rule(capwap_dev, nl_rule->msg.ip_flow.tun_id,
			nl_rule->msg.ip_flow.flow.ip_version, nl_rule->msg.ip_flow.flow.protocol,
			nl_rule->msg.ip_flow.flow.src_ip, nl_rule->msg.ip_flow.flow.dst_ip,
			nl_rule->msg.ip_flow.flow.src_port, nl_rule->msg.ip_flow.flow.dst_port);
	if (status != NSS_CAPWAPMGR_SUCCESS) {
		nss_nl_error("Unable to del flow rule\n");
		return -EAGAIN;
	}

	nss_nl_info("Succesfully deleted flow rule for tunnel %d\n", nl_rule->msg.ip_flow.tun_id);
	return 0;
}

/*
 * nss_nlcapwap_tx_packets()
 *	Handler for sending traffic from one DUT to other
 */
static int nss_nlcapwap_tx_packets(struct nss_nlcapwap_rule *nl_rule)
{
	struct nss_capwap_metaheader *mh;
	struct nss_nlcapwap_tunnel *tun;
	struct net_device *capwap_dev;
	struct sk_buff *skb;
	size_t align_offset;
	uint16_t tun_id;
	uint8_t mh_type;
	size_t pkt_sz;
	size_t skb_sz;

	tun_id = nl_rule->msg.tx_packets.tun_id;
	pkt_sz = nl_rule->msg.tx_packets.pkt_size;

	/*
	 * Get the capwap netdev reference;
	 * TODO: We need to add the DTLS needed headroom/tailroom to it
	 */
	capwap_dev = global_ctx.capwap_dev;
	if (!capwap_dev) {
		nss_nl_error("Failed to find CAPWAP netdevice\n");
		return -ENODEV;
	}

	skb_sz = NSS_NLCAPWAP_MAX_HEADROOM + pkt_sz + NSS_NLCAPWAP_MAX_TAILROOM + SMP_CACHE_BYTES;

	skb = dev_alloc_skb(skb_sz);
	if (!skb) {
		nss_nl_error("%px: Could not allocate a sk_buff of size(%zu).\n", capwap_dev, skb_sz);
		return -ENOMEM;
	}

	/*
	 * Reserve headroom for tunnel headers CAPWAP/DTLS
	 */
	align_offset = PTR_ALIGN(skb->data, SMP_CACHE_BYTES) - skb->data;
	skb_reserve(skb, NSS_NLCAPWAP_MAX_HEADROOM + align_offset + sizeof(uint16_t));

	write_lock_bh(&global_ctx.lock);
	tun = nss_nlcapwap_get_tun_by_index(tun_id);
	if (!tun) {
		write_unlock_bh(&global_ctx.lock);
		dev_kfree_skb_any(skb);
		nss_nl_error("%px: Could not find tunnel associated with index: %d\n", nl_rule, tun_id);
		return -ENODEV;
	}

	mh = (struct nss_capwap_metaheader *)skb_put(skb, sizeof(*mh));
	memcpy(mh, tun->mh.meta_header_blob, NSS_NLCAPWAP_META_HEADER_SZ);
	mh_type = tun->mh.type;
	tun->stats.tx_data_pkts++;
	write_unlock_bh(&global_ctx.lock);
	pkt_sz -= sizeof(*mh);

	/*
	 * Set the appropriate ether_type
	 */
	if (mh_type == NSS_NLCAPWAP_META_HEADER_TYPE_IPV4_DATA) {
		/*
		 * For normal ipv4 data frames.
		 */
		struct ethhdr *eh = (struct ethhdr *)skb_put(skb, sizeof(*eh));
		eh->h_proto = htons(ETH_P_IP);
		pkt_sz -= sizeof(*eh);
	} else if (mh_type == NSS_NLCAPWAP_META_HEADER_TYPE_EAPOL) {
		/*
		 * EAPOL type frames.
		 */
		struct ethhdr *eh = (struct ethhdr *)skb_put(skb, sizeof(*eh));
		eh->h_proto = htons(ETH_P_PAE);
		pkt_sz -= sizeof(*eh);
	} else {
		/*
		 * DTLS management type frames.
		 */
		uint16_t *data = (uint16_t *)skb_put(skb, sizeof(*data));
		*data = htons(NSS_CAPWAP_PKT_TYPE_DTLS_ENABLED);
		pkt_sz -= sizeof(*data);
	}

	if (!atomic_read(&global_ctx.enable_perf)) {
		memset(skb_put(skb, pkt_sz), NSS_NLCAPWAP_DATA, pkt_sz);
	}

	BUG_ON(!IS_ALIGNED((unsigned long)skb->data, sizeof(uint16_t)));

	if (capwap_dev->netdev_ops->ndo_start_xmit(skb, capwap_dev) != NETDEV_TX_OK) {
		dev_kfree_skb_any(skb);
		return -EBUSY;
	}

	nss_nl_info("Tx packet for tun(%d), skb_size(%zu) matched(%zu)\n", mh->tunnel_id, skb_sz, pkt_sz);
	return 0;
}

/*
 * nss_nlcapwap_ops_tx_packets()
 *	Handler for sending traffic
 */
static int nss_nlcapwap_ops_tx_packets(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nlcapwap_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	uint16_t num_pkts;
	int ret;
	int i;

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlcapwap_family, info, NSS_NLCAPWAP_CMD_TYPE_TX_PACKETS);
	if (!nl_cm) {
		nss_nl_error("Unable to extract tx_packets data\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlcapwap_rule, cm);
	num_pkts = nl_rule->msg.tx_packets.num_of_packets;

	for (i = 0; i < num_pkts; i++) {
		ret = nss_nlcapwap_tx_packets(nl_rule);
		if (ret < 0) {
			nss_nl_error("Error in transmission of skb\n");
			return ret;
		}
	}

	nss_nl_info("Traffic transmission successful\n");
	return 0;
}

/*
 * nss_nlcapwap_ops_meta_header()
 *	Handler for creating meta header
 */
static int nss_nlcapwap_ops_meta_header(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nlcapwap_rule *nl_rule;
	struct nss_nlcapwap_tunnel *tun;
	struct nss_nlcmn *nl_cm;
	uint16_t tun_id;

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlcapwap_family, info, NSS_NLCAPWAP_CMD_TYPE_META_HEADER);
	if (!nl_cm) {
		nss_nl_error("Unable to extract meta header values.\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlcapwap_rule, cm);
	tun_id = nl_rule->msg.meta_header.tun_id;

	/*
	 * Set meta header values
	 */
	write_lock_bh(&global_ctx.lock);
	tun = nss_nlcapwap_get_tun_by_index(tun_id);
	if (!tun) {
		write_unlock_bh(&global_ctx.lock);
		nss_nl_error("%px: Could not find tunnel associated with index: %d\n", nl_rule, tun_id);
		return -EAGAIN;
	}

	tun->mh = nl_rule->msg.meta_header;
	write_unlock_bh(&global_ctx.lock);
	nss_nl_info("Successfully created meta header.\n");
	return 0;
}

/*
 * nss_nlcapwap_ops_get_stats()
 *	get stats handler
 */
static int nss_nlcapwap_ops_get_stats(struct sk_buff *skb, struct genl_info *info)
{
	return 0;
}

/*
 * nss_nlcapwap_ops_keepalive()
 *	Handler for enabling and disabling keepalive flag
 */
static int nss_nlcapwap_ops_keepalive(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nlcapwap_rule *nl_rule;
	struct nss_nlcapwap_tunnel *tun;
	struct net_device *dtls_dev;
	struct delayed_work *dwork;
	struct nss_nlcmn *nl_cm;
	uint32_t tun_id;
	bool kalive;

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlcapwap_family, info, NSS_NLCAPWAP_CMD_TYPE_KEEPALIVE);
	if (!nl_cm) {
		nss_nl_error("Unable to extract meta header values.\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlcapwap_rule, cm);

	/*
	 * Extract the tun_id
	 */
	tun_id = nl_rule->msg.kalive.tun_id;
	dtls_dev = nss_capwapmgr_get_dtls_netdev(global_ctx.capwap_dev, tun_id);
	if (!dtls_dev) {
		nss_nl_error("%px: Failed to find DTLS dev for (%s)\n", &nl_rule, global_ctx.capwap_dev->name);
		return -ENODEV;
	}

	/*
	 * Get the local tunnel object
	 */
	write_lock_bh(&global_ctx.lock);
	tun = nss_nlcapwap_get_tun_by_index(tun_id);
	if (!tun) {
		write_unlock_bh(&global_ctx.lock);
		nss_nl_error("%px: Could not find tunnel associated with index: %d\n", nl_rule, tun_id);
		dev_put(dtls_dev);
		return -EAGAIN;
	}

	dwork = &tun->kalive.work;
	kalive = nl_rule->msg.kalive.tx_keepalive;
	atomic_set(&tun->kalive.status, kalive);

	write_unlock_bh(&global_ctx.lock);

	/*
	 * Check if dtls keepalive packets needs to be sent
	 */
	if (kalive) {
		schedule_delayed_work(dwork, NSS_NLCAPWAP_KALIVE_TIMER_MSECS);
	} else {
		flush_delayed_work(dwork);
	}

	nss_nl_info("%px: keepalive %s for tun(%d)\n", tun, kalive ? "enabled" : "disabled", tun_id);
	dev_put(dtls_dev);
	return 0;
}

/*
 * nss_nlcapwap_tunnel_stats_read()
 *	Reads the netlink capwap stats
 */
static ssize_t nss_nlcapwap_tunnel_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *f_ppos)
{
	struct nss_nlcapwap_tunnel_stats stats = {0};
	struct nss_nlcapwap_tunnel *tun;
	uint32_t max_output_lines;
	ssize_t bytes_read = 0;
	ssize_t size_wr = 0;
	ssize_t size_al;
	char *lbuf;
	int index;

	/*
	 * Header and footer for instance stats
	 */
	max_output_lines = 4 + (NSS_CAPWAPMGR_MAX_TUNNELS * NSS_NLCAPWAP_STATS_MAX);
	size_al = NSS_NLCAPWAP_MAX_STR_LEN * max_output_lines;

	lbuf = vzalloc(size_al);
	if (!lbuf) {
		nss_nl_error("%px: Could not allocate space for debug entry\n", f_ppos);
		return 0;
	}

	/*
	 * Session stats
	 */
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nCapwap netlinks stats start:\n\n");
	for (index = 0; index < NSS_CAPWAPMGR_MAX_TUNNELS; index++) {
		/*
		 * Copy the tunnels stats
		 */
		read_lock_bh(&global_ctx.lock);
		tun = nss_nlcapwap_get_tun_by_index(index);
		if (!tun) {
			read_unlock_bh(&global_ctx.lock);
			continue;
		}

		memcpy(&stats, &tun->stats, sizeof(stats));
		read_unlock_bh(&global_ctx.lock);

		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "----------------------------");
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n%s\t: %d", "Tunnel ID", index);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n%s\t: %d", "tx_data_pkts", stats.tx_data_pkts);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n%s\t: %d", "rx_data_pkts", stats.rx_data_pkts);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n%s\t: %d", "tx_ka_pkts", stats.tx_ka_pkts);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n%s\t: %d", "rx_ka_pkts", stats.rx_ka_pkts);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n%s\t: %d", "ka_seq_fail", stats.ka_seq_fail);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n----------------------------");
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n");
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nCapwap netlinks stats end.\n\n");
	bytes_read = simple_read_from_buffer(ubuf, sz, f_ppos, lbuf, size_wr);

	vfree(lbuf);
	return bytes_read;
}

/*
 * nss_nlcapwap_stats_ops
 *	Defines the file operations for nlcapwap dentry
 */
static const struct file_operations nss_nlcapwap_stats_ops = {
	.read = nss_nlcapwap_tunnel_stats_read,
};

/*
 * nss_nlcapwap_get_ifnum()
 *	Get the interface number corresponding to netdev
 */
int nss_nlcapwap_get_ifnum(struct net_device *dev, enum nss_dynamic_interface_type type)
{
	int ifnum;

	/*
	 * Get the interface number depending upon the dev and type
	 */
	ifnum = nss_cmn_get_interface_number_by_dev_and_type(dev, type);
	if (ifnum < 0) {
		nss_nl_error("%px: Failed to find interface number (dev:%s, type:%d)\n",
				dev, dev->name, type);
		return -ENODEV;
	}

	return ifnum;
}

/*
 * nss_nlcapwap_init()
 *	Init handler for capwap
 */
bool nss_nlcapwap_init(void)
{
	int err;

	nss_nl_info_always("Init NSS netlink capwap handler\n");

	/*
	 * Initialize atomic variable
	 */
	atomic_set(&global_ctx.enable_perf, 0);

	/*
	 * Register the capwap netdev rx handler
	 */
	global_ctx.capwap_dev = nss_capwapmgr_get_netdev();
	if (!global_ctx.capwap_dev) {
		nss_nl_info_always("Failed to find the CAPWAP device\n");
	}

	/*
	 * Create a debugfs entry for netlink capwap
	 */
	global_ctx.dentry = debugfs_create_dir("nlcapwap", NULL);
	if (!global_ctx.dentry) {
		nss_nl_info_always("Cannot create nlcapwap directory\n");
		return false;
	}

	if (!debugfs_create_file("stats", 0400, global_ctx.dentry, NULL, &nss_nlcapwap_stats_ops)) {
		nss_nl_info_always("Cannot create nlcapwap dentry file\n");
		return false;
	}

	/*
	 * register NETLINK ops with the family
	 */
	err = genl_register_family(&nss_nlcapwap_family);
	if (err) {
		nss_nl_info_always("Error: %d unable to register capwap family\n", err);
		goto free;
	}

	/*
	 * register device call back handler for capwap from NSS
	 */
	err = nss_capwap_stats_register_notifier(&nss_capwap_stats_notifier_nb);
	if (err) {
		nss_nl_info_always("Error: %d unable to register capwap stats notifier\n", err);
		goto free_family;
	}

	/*
	 * Register a netdevice rx handler
	 */
	rtnl_lock();
	err = netdev_rx_handler_register(global_ctx.capwap_dev, nss_nlcapwapmgr_rx_handler, NULL);
	rtnl_unlock();

	if (err < 0) {
		nss_nl_error("Couldn't register CAPWAP RX handler\n");
		goto free_notify;
	}

	/*
	 * Initialize the global lock
	 */
	rwlock_init(&global_ctx.lock);
	return true;

free_notify:
	nss_capwap_stats_unregister_notifier(&nss_capwap_stats_notifier_nb);
free_family:
	genl_unregister_family(&nss_nlcapwap_family);
free:
	debugfs_remove_recursive(global_ctx.dentry);
	return false;
}

/*
 * nss_nlcapwap_exit()
 *	Exit handler for capwap
 */
bool nss_nlcapwap_exit(void)
{
	struct nss_nlcapwap_tunnel *tun;
	int err;
	int i;

	nss_nl_info_always("Exit NSS netlink capwap handler\n");

	/*
	 * unregister the ops family so that we don't receive any new requests
	 */
	err = genl_unregister_family(&nss_nlcapwap_family);
	if (err) {
		nss_nl_info_always("Error: %d unable to unregister capwap NETLINK family\n", err);
		return false;
	}

	/*
	 * Unregister the capwap netdev rx handler
	 */
	if (global_ctx.capwap_dev) {
		rtnl_lock();
		netdev_rx_handler_unregister(global_ctx.capwap_dev);
		rtnl_unlock();
	}

	/*
	 * Remove the debugfs entry
	 */
	debugfs_remove_recursive(global_ctx.dentry);

	/*
	 * Destroy all the active tunnels
	 */
	for (i = 0; i < NSS_CAPWAPMGR_MAX_TUNNELS; i++) {
		write_lock_bh(&global_ctx.lock);
		tun = nss_nlcapwap_get_tun_by_index(i);
		if (!tun) {
			write_unlock_bh(&global_ctx.lock);
			continue;
		}

		nss_nlcapwap_tun_deinit(tun);
		clear_bit(i, global_ctx.tun_bitmap);
		write_unlock_bh(&global_ctx.lock);

		/*
		 * Destroy the corresponding tunnel
		 */
		if (!nss_nlcapwap_destroy_tun(i)) {
			nss_nl_error("Unable to set the tunnel status to 0 for tun: %d\n", i);
			continue;
		}
	}

	/*
	 * Unregister the device callback handler for capwap
	 */
	nss_capwap_stats_unregister_notifier(&nss_capwap_stats_notifier_nb);
	return true;
}
