/*
 **************************************************************************
 * Copyright (c) 2015-2016,2018-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <crypto/internal/skcipher.h>
#include <linux/debugfs.h>
#include <linux/etherdevice.h>
#include <linux/icmp.h>
#include <linux/inet.h>
#include <linux/udp.h>
#include <linux/version.h>
#include <net/genetlink.h>
#include <net/ip6_checksum.h>
#include <net/udp.h>
#include <nss_api_if.h>
#include <nss_dtls.h>
#include <nss_dtlsmgr.h>
#include <nss_dtls_cmn.h>
#include <nss_nlcmn_if.h>
#include <nss_nl_if.h>
#include "nss_nl.h"
#include "nss_nldtls.h"
#include "nss_nldtls_if.h"

/*
 * prototypes
 */
static int nss_nldtls_ops_create_tun(struct sk_buff *skb, struct genl_info *info);
static int nss_nldtls_ops_destroy_tun(struct sk_buff *skb, struct genl_info *info);
static int nss_nldtls_ops_update_config(struct sk_buff *skb, struct genl_info *info);
static int nss_nldtls_ops_tx_pkts(struct sk_buff *skb, struct genl_info *info);

/*
 * Initializing the global variables
 */
static struct nss_nldtls_gbl_ctx gbl_ctx = {
	.lock = __SPIN_LOCK_UNLOCKED(lock),
	.num_tun = ATOMIC_INIT(0),
	.dtls_list_head = LIST_HEAD_INIT(gbl_ctx.dtls_list_head),
	.log_en = false
};

/*
 * nss_nldtls_family_mcgrp
 *	Multicast group for sending message status & events
 */
static const struct genl_multicast_group nss_nldtls_family_mcgrp[] = {
	{.name = NSS_NLDTLS_MCAST_GRP},
};

/*
 * nss_nldtls_ops
 *	Operation table called by the generic netlink layer based on the command
 */
static struct genl_ops nss_nldtls_ops[] = {
	{.cmd = NSS_NLDTLS_CMD_TYPE_CREATE_TUN, .doit = nss_nldtls_ops_create_tun,},
	{.cmd = NSS_NLDTLS_CMD_TYPE_DESTROY_TUN, .doit = nss_nldtls_ops_destroy_tun,},
	{.cmd = NSS_NLDTLS_CMD_TYPE_UPDATE_CONFIG, .doit = nss_nldtls_ops_update_config,},
	{.cmd = NSS_NLDTLS_CMD_TYPE_TX_PKTS, .doit = nss_nldtls_ops_tx_pkts,},
};

/*
 * nss_nldtls_family
 *	Dtls family definition
 */
struct genl_family nss_nldtls_family = {
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 9, 0))
	.id = GENL_ID_GENERATE,				/* Auto generate ID */
#endif
	.name = NSS_NLDTLS_FAMILY,			/* family name string */
	.hdrsize = sizeof(struct nss_nldtls_rule),	/* NSS NETLINK dtls rule */
	.version = NSS_NL_VER,				/* Set it to NSS_NLDTLS version */
	.maxattr = NSS_NLDTLS_CMD_TYPE_MAX,		/* maximum commands supported */
	.netnsok = true,
	.pre_doit = NULL,
	.post_doit = NULL,
	.ops = nss_nldtls_ops,
	.n_ops = ARRAY_SIZE(nss_nldtls_ops),
	.mcgrps = nss_nldtls_family_mcgrp,
	.n_mcgrps = ARRAY_SIZE(nss_nldtls_family_mcgrp)
};

/*
 * nss_nldtls_find_tun_ctx()
 *	Returns the global context object of a tunnel
 */
static struct nss_nldtls_tun_ctx *nss_nldtls_find_tun_ctx(struct net_device *dev)
{
	struct nss_nldtls_tun_ctx *entry;

	spin_lock(&gbl_ctx.lock);
	list_for_each_entry(entry, &gbl_ctx.dtls_list_head, list) {
		if (!strncmp(entry->dev_name, dev->name, IFNAMSIZ)) {
			spin_unlock(&gbl_ctx.lock);
			return entry;
		}
	}

	spin_unlock(&gbl_ctx.lock);
	return NULL;
}

/*
 * nss_nldtls_dev_rx_handler()
 *	Common rx handler for all dtls dev
 */
static rx_handler_result_t nss_nldtls_dev_rx_handler(struct sk_buff **pskb)
{
	static bool first_pkt;
	unsigned long long duration;
	struct sk_buff *skb = *pskb;
	ktime_t delta;

	if (unlikely(!first_pkt)) {
		gbl_ctx.first_rx_pkt_time = ktime_get();
		first_pkt = true;
	}

	gbl_ctx.last_rx_pkt_time = ktime_get();
	if (unlikely(gbl_ctx.log_en)) {
		struct net_device *dev;

		delta = ktime_sub(gbl_ctx.last_rx_pkt_time, gbl_ctx.first_rx_pkt_time);
		duration = (unsigned long long) ktime_to_ns(delta) >> 10;
		print_hex_dump_bytes("", DUMP_PREFIX_OFFSET, skb->data, 64);
		dev = dev_get_by_index(&init_net, skb->skb_iif);
		if (dev) {
			nss_nl_info("%px: in_dev = %s, out_dev = %s\n", skb, dev->name, skb->dev->name);
			dev_put(dev);
		}

		nss_nl_info("%px: DTLS RX (%s) pkt len = %d udp_csum = %s rx_time: %llu\n", skb,
				skb->dev->name, skb->len, udp_lib_checksum_complete(skb) ?
				"invalid" : "valid", duration);
	}

	dev_kfree_skb_any(skb);
	return RX_HANDLER_CONSUMED;
}

/*
 * nss_nldtls_create_ipv4_rule()
 *	Create a nss entry to accelerate the given IPv4 connection
 */
static int nss_nldtls_create_ipv4_rule(struct nss_ipv4_create *unic, uint16_t rule_flags)
{
	struct nss_ipv4_rule_create_msg *nircm;
	struct nss_ctx_instance *nss_ctx;
	struct nss_ipv4_msg nim;
	nss_tx_status_t status;

	nss_ctx = nss_ipv4_get_mgr();
	if (!nss_ctx) {
		nss_nl_info("%px: Couldn't get IPv4 ctx\n", unic);
		return -1;
	}

	nss_nl_info("%px: IPv4 rule: src:%pI4h:%d dst:%pI4h:%d p:%d\n",
	      unic, &unic->src_ip, unic->src_port,
	      &unic->dest_ip, unic->dest_port, unic->protocol);

	memset(&nim, 0, sizeof (struct nss_ipv4_msg));
	nss_ipv4_msg_init(&nim, NSS_IPV4_RX_INTERFACE,
			  NSS_IPV4_TX_CREATE_RULE_MSG,
			  sizeof(struct nss_ipv4_rule_create_msg), NULL, NULL);

	nircm = &nim.msg.rule_create;
	nircm->valid_flags = 0;
	nircm->rule_flags = 0;

	/*
	 * Copy over the 5 tuple details.
	 */
	nircm->tuple.protocol = (uint8_t)unic->protocol;
	nircm->tuple.flow_ip = unic->src_ip;
	nircm->tuple.flow_ident = (uint32_t)unic->src_port;
	nircm->tuple.return_ip = unic->dest_ip;
	nircm->tuple.return_ident = (uint32_t)unic->dest_port;

	/*
	 * Copy over the connection rules and set the CONN_VALID flag
	 */
	nircm->conn_rule.flow_interface_num = unic->src_interface_num;
	nircm->conn_rule.flow_mtu = unic->from_mtu;
	nircm->conn_rule.flow_ip_xlate = unic->src_ip_xlate;
	nircm->conn_rule.flow_ident_xlate = (uint32_t)unic->src_port_xlate;
	memcpy(nircm->conn_rule.flow_mac, unic->src_mac, 6);
	nircm->conn_rule.return_interface_num = unic->dest_interface_num;
	nircm->conn_rule.return_mtu = unic->to_mtu;
	nircm->conn_rule.return_ip_xlate = unic->dest_ip_xlate;
	nircm->conn_rule.return_ident_xlate = (uint32_t)unic->dest_port_xlate;
	if ((nircm->tuple.return_ip != nircm->conn_rule.return_ip_xlate) ||
	    (nircm->tuple.return_ident != nircm->conn_rule.return_ident_xlate))
		memcpy(nircm->conn_rule.return_mac, unic->dest_mac_xlate, 6);
	else
		memcpy(nircm->conn_rule.return_mac, unic->dest_mac, 6);

	/*
	 * Copy over the DSCP rule parameters
	 */
	if (unic->flags & NSS_IPV4_CREATE_FLAG_DSCP_MARKING) {
		nircm->dscp_rule.flow_dscp = unic->flow_dscp;
		nircm->dscp_rule.return_dscp = unic->return_dscp;
		nircm->rule_flags |= NSS_IPV4_RULE_CREATE_FLAG_DSCP_MARKING;
		nircm->valid_flags |= NSS_IPV4_RULE_CREATE_DSCP_MARKING_VALID;
	}

	nircm->valid_flags |= NSS_IPV4_RULE_CREATE_CONN_VALID;

	/*
	 * Copy over the pppoe rules and set the PPPOE_VALID flag.
	 */
	nircm->pppoe_rule.flow_if_exist = unic->flow_pppoe_if_exist;
	nircm->pppoe_rule.flow_if_num = unic->flow_pppoe_if_num;
	nircm->pppoe_rule.return_if_exist = unic->return_pppoe_if_exist;
	nircm->pppoe_rule.return_if_num = unic->return_pppoe_if_num;
	nircm->valid_flags |= NSS_IPV4_RULE_CREATE_PPPOE_VALID;

	/*
	 * Copy over the vlan rules and set the VLAN_VALID flag
	 */
	nircm->vlan_primary_rule.ingress_vlan_tag = unic->in_vlan_tag[0];
	nircm->vlan_primary_rule.egress_vlan_tag = unic->out_vlan_tag[0];
	nircm->vlan_secondary_rule.ingress_vlan_tag = unic->in_vlan_tag[1];
	nircm->vlan_secondary_rule.egress_vlan_tag = unic->out_vlan_tag[1];
	nircm->valid_flags |= NSS_IPV4_RULE_CREATE_VLAN_VALID;

	/*
	 * Copy over the qos rules and set the QOS_VALID flag
	 */
	nircm->qos_rule.flow_qos_tag = unic->flow_qos_tag;
	nircm->qos_rule.return_qos_tag = unic->return_qos_tag;
	nircm->valid_flags |= NSS_IPV4_RULE_CREATE_QOS_VALID;

	if (unic->flags & NSS_IPV4_CREATE_FLAG_NO_SEQ_CHECK)
		nircm->rule_flags |= NSS_IPV4_RULE_CREATE_FLAG_NO_SEQ_CHECK;

	if (unic->flags & NSS_IPV4_CREATE_FLAG_BRIDGE_FLOW)
		nircm->rule_flags |= NSS_IPV4_RULE_CREATE_FLAG_BRIDGE_FLOW;

	if (unic->flags & NSS_IPV4_CREATE_FLAG_ROUTED)
		nircm->rule_flags |= NSS_IPV4_RULE_CREATE_FLAG_ROUTED;

	/*
	 * Set the flag NSS_IPV4_RULE_CREATE_FLAG_ICMP_NO_CME_FLUSH so that
	 * rule is not flushed when NSS FW receives ICMP errors/packets.
	 */
	nircm->rule_flags |= NSS_IPV4_RULE_CREATE_FLAG_ICMP_NO_CME_FLUSH;

	/*
	 * Add any other additional flags which caller has requested.
	 * For example: update MTU
	 */
	nircm->rule_flags |= rule_flags;

	status = nss_ipv4_tx(nss_ctx, &nim);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_info("%px: Create IPv4 message failed %d\n", nss_ctx, status);
	}

	return 0;
}

/*
 * nss_nldtls_create_ipv6_rule()
 *	Create a nss entry to accelerate the given IPV6 connection
 */
static int nss_nldtls_create_ipv6_rule(struct nss_ipv6_create *unic, uint16_t rule_flags)
{
	struct nss_ipv6_rule_create_msg *nircm;
	struct nss_ctx_instance *nss_ctx;
	struct nss_ipv6_msg nim;
	nss_tx_status_t status;

	nss_ctx = nss_ipv6_get_mgr();
	if (!nss_ctx) {
		nss_nl_info("%px: Couldn't get IPv6 ctx\n", unic);
		return -1;
	}

	nss_nl_info("%px: Create IPv6 rule: %pI6:%d %pI6:%d p:%d\n",
	      unic, unic->src_ip, unic->src_port, unic->dest_ip,
	      unic->dest_port, unic->protocol);

	memset(&nim, 0, sizeof (struct nss_ipv6_msg));
	nss_ipv6_msg_init(&nim, NSS_IPV6_RX_INTERFACE,
			  NSS_IPV6_TX_CREATE_RULE_MSG,
			  sizeof(struct nss_ipv6_rule_create_msg), NULL, NULL);

	nircm = &nim.msg.rule_create;
	nircm->rule_flags = 0;
	nircm->valid_flags = 0;

	/*
	 * Copy over the 5 tuple information.
	 */
	nircm->tuple.protocol = (uint8_t)unic->protocol;
	memcpy(nircm->tuple.flow_ip, unic->src_ip, sizeof(nircm->tuple.flow_ip));
	memcpy(nircm->tuple.return_ip, unic->dest_ip, sizeof(nircm->tuple.return_ip));
	nircm->tuple.flow_ident = (uint32_t)unic->src_port;
	nircm->tuple.return_ident = (uint32_t)unic->dest_port;

	/*
	 * Copy over the connection rules and set CONN_VALID flag
	 */
	nircm->conn_rule.flow_interface_num = unic->src_interface_num;
	nircm->conn_rule.flow_mtu = unic->from_mtu;
	nircm->conn_rule.return_interface_num = unic->dest_interface_num;
	nircm->conn_rule.return_mtu = unic->to_mtu;
	memcpy(nircm->conn_rule.flow_mac, unic->src_mac, 6);
	memcpy(nircm->conn_rule.return_mac, unic->dest_mac, 6);
	nircm->valid_flags |= NSS_IPV6_RULE_CREATE_CONN_VALID;

	/*
	 * Copy over the DSCP rule parameters
	 */
	if (unic->flags & NSS_IPV6_CREATE_FLAG_DSCP_MARKING) {
		nircm->dscp_rule.flow_dscp = unic->flow_dscp;
		nircm->dscp_rule.return_dscp = unic->return_dscp;
		nircm->rule_flags |= NSS_IPV6_RULE_CREATE_FLAG_DSCP_MARKING;
		nircm->valid_flags |= NSS_IPV6_RULE_CREATE_DSCP_MARKING_VALID;
	}

	/*
	 * Copy over the pppoe rules and set PPPOE_VALID flag
	 */
	nircm->pppoe_rule.flow_if_exist = unic->flow_pppoe_if_exist;
	nircm->pppoe_rule.flow_if_num = unic->flow_pppoe_if_num;
	nircm->pppoe_rule.return_if_exist = unic->return_pppoe_if_exist;
	nircm->pppoe_rule.return_if_num = unic->return_pppoe_if_num;
	nircm->valid_flags |= NSS_IPV6_RULE_CREATE_PPPOE_VALID;

	/*
	 * Copy over the tcp rules and set TCP_VALID flag
	 */
	nircm->tcp_rule.flow_window_scale = unic->flow_window_scale;
	nircm->tcp_rule.flow_max_window = unic->flow_max_window;
	nircm->tcp_rule.flow_end = unic->flow_end;
	nircm->tcp_rule.flow_max_end = unic->flow_max_end;
	nircm->tcp_rule.return_window_scale = unic->return_window_scale;
	nircm->tcp_rule.return_max_window = unic->return_max_window;
	nircm->tcp_rule.return_end = unic->return_end;
	nircm->tcp_rule.return_max_end = unic->return_max_end;
	nircm->valid_flags |= NSS_IPV6_RULE_CREATE_TCP_VALID;

	/*
	 * Copy over the vlan rules and set the VLAN_VALID flag
	 */
	nircm->vlan_primary_rule.egress_vlan_tag = unic->out_vlan_tag[0];
	nircm->vlan_primary_rule.ingress_vlan_tag = unic->in_vlan_tag[0];
	nircm->vlan_secondary_rule.egress_vlan_tag = unic->out_vlan_tag[1];
	nircm->vlan_secondary_rule.ingress_vlan_tag = unic->in_vlan_tag[1];
	nircm->valid_flags |= NSS_IPV6_RULE_CREATE_VLAN_VALID;

	/*
	 * Copy over the qos rules and set the QOS_VALID flag
	 */
	nircm->qos_rule.flow_qos_tag = unic->flow_qos_tag;
	nircm->qos_rule.return_qos_tag = unic->return_qos_tag;
	nircm->valid_flags |= NSS_IPV6_RULE_CREATE_QOS_VALID;

	if (unic->flags & NSS_IPV6_CREATE_FLAG_NO_SEQ_CHECK)
		nircm->rule_flags |= NSS_IPV6_RULE_CREATE_FLAG_NO_SEQ_CHECK;

	if (unic->flags & NSS_IPV6_CREATE_FLAG_BRIDGE_FLOW)
		nircm->rule_flags |= NSS_IPV6_RULE_CREATE_FLAG_BRIDGE_FLOW;

	if (unic->flags & NSS_IPV6_CREATE_FLAG_ROUTED)
		nircm->rule_flags |= NSS_IPV6_RULE_CREATE_FLAG_ROUTED;

	/*
	 * Set the flag NSS_IPV4_RULE_CREATE_FLAG_ICMP_NO_CME_FLUSH so that
	 * rule is not flushed when NSS FW receives ICMP errors/packets.
	 */
	nircm->rule_flags |= NSS_IPV4_RULE_CREATE_FLAG_ICMP_NO_CME_FLUSH;

	/*
	 * Add any other additional flags which caller has requested.
	 * For example: update MTU
	 */
	nircm->rule_flags |= rule_flags;

	status = nss_ipv6_tx(nss_ctx, &nim);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_info("%px: Create IPv4 message failed %d\n", nss_ctx, status);
	}

	return 0;
}

/*
 * nss_nldtls_data_callback()
 *	Data callback function for dtls
 */
static void nss_nldtls_data_callback(void *app_data, struct sk_buff *skb)
{
	struct nss_dtlsmgr_metadata *ndm;
	struct nss_nldtls_tun_ctx *tun;
	struct nss_nldtls_stats *stats;
	struct net_device *dev;

	dev = dev_get_by_index(&init_net, skb->skb_iif);
	if (!dev) {
		nss_nl_error("Unable to get net dev for skb_iif %d\n", skb->skb_iif);
		dev_kfree_skb_any(skb);
		return;
	}

	ndm = (struct nss_dtlsmgr_metadata *)skb->data;
	tun = nss_nldtls_find_tun_ctx(dev);
	if (!tun) {
		nss_nl_error("Unable find tunnel ctx for %s\n", dev->name);
		dev_put(dev);
		dev_kfree_skb_any(skb);
		return;
	}

	stats = &tun->stats[NSS_NLDTLS_CTYPE_TO_IDX(ndm->ctype)];
	spin_lock(&gbl_ctx.lock);
	stats->rx_pkts++;
	stats->rx_bytes += skb->len - sizeof(*ndm);
	spin_unlock(&gbl_ctx.lock);

	if (unlikely(gbl_ctx.log_en)) {
		nss_nl_hex_dump_bytes("", DUMP_PREFIX_OFFSET, skb->data, (skb->len > 64) ? 64 : skb->len);
	}

	nss_nl_trace("%px Received DTLS packet\n", skb);
	dev_put(dev);
	dev_kfree_skb_any(skb);
}

/*
 * nss_nldtls_create_session()
 *	Create a DTLS session through dtlsmgr driver API.
 */
static struct net_device *nss_nldtls_create_session(struct nss_nldtls_rule *nl_rule)
{
	struct nss_nldtls_tun_ctx *dtls_tun_data;
	struct nss_dtlsmgr_config dcfg;
	struct nss_dtlsmgr_ctx *ctx;
	struct net_device *ndev;
	uint16_t key_len;
	uint8_t algo;
	int err;

	if (atomic_read(&gbl_ctx.num_tun) >= NSS_NLDTLS_MAX_TUNNELS) {
		nss_nl_error("Max number of tunnels exhausted: 32\n");
		return NULL;
	}

	memset(&dcfg, 0, sizeof(struct nss_dtlsmgr_config));
	algo = nl_rule->msg.create.encap.cfg.crypto.algo;
	dcfg.flags = nl_rule->msg.create.flags | NSS_DTLSMGR_ENCAP_METADATA;
	if (algo == NSS_DTLSMGR_ALGO_AES_GCM)
		dcfg.flags |= NSS_DTLSMGR_CIPHER_MODE_GCM;

	dcfg.app_data = NULL;
	dcfg.notify = NULL;
	dcfg.data = nss_nldtls_data_callback;

	/*
	 * Encap configuration
	 */
	key_len = nl_rule->msg.create.encap.cfg.crypto.cipher_key.len;
	if (key_len > NSS_NLDTLS_CIPHER_KEY_MAX) {
		nss_nl_error("Invalid cipher length: %u\n", key_len);
		return NULL;
	}

	key_len = nl_rule->msg.create.encap.cfg.crypto.auth_key.len;
	if (key_len > NSS_NLDTLS_AUTH_KEY_MAX) {
		nss_nl_error("Invalid authentication length: %u\n", key_len);
		return NULL;
	}

	key_len = nl_rule->msg.create.encap.cfg.crypto.nonce.len;
	if (key_len > NSS_NLDTLS_NONCE_SIZE_MAX) {
		nss_nl_error("Invalid nonce length: %u\n", key_len);
		return NULL;
	}

	nl_rule->msg.create.encap.cfg.crypto.cipher_key.data = nl_rule->msg.create.encap.keys.cipher;
	nl_rule->msg.create.encap.cfg.crypto.auth_key.data = nl_rule->msg.create.encap.keys.auth;
	nl_rule->msg.create.encap.cfg.crypto.nonce.data = nl_rule->msg.create.encap.keys.nonce;
	memcpy((void *)&dcfg.encap, (void *)&nl_rule->msg.create.encap.cfg, sizeof(struct nss_dtlsmgr_encap_config));

	/*
	 * Decap configuration
	 */
	key_len = nl_rule->msg.create.decap.cfg.crypto.cipher_key.len;
	if (key_len > NSS_NLDTLS_CIPHER_KEY_MAX) {
		nss_nl_error("Invalid cipher length: %u\n", key_len);
		return NULL;
	}

	key_len = nl_rule->msg.create.decap.cfg.crypto.auth_key.len;
	if (key_len > NSS_NLDTLS_AUTH_KEY_MAX) {
		nss_nl_error("Invalid authentication length: %u\n", key_len);
		return NULL;
	}

	key_len = nl_rule->msg.create.decap.cfg.crypto.nonce.len;
	if (key_len > NSS_NLDTLS_NONCE_SIZE_MAX) {
		nss_nl_error("Invalid nonce length: %u\n", key_len);
		return NULL;
	}

	nl_rule->msg.create.decap.cfg.crypto.cipher_key.data = nl_rule->msg.create.decap.keys.cipher;
	nl_rule->msg.create.decap.cfg.crypto.auth_key.data = nl_rule->msg.create.decap.keys.auth;
	nl_rule->msg.create.decap.cfg.crypto.nonce.data = nl_rule->msg.create.decap.keys.nonce;
	memcpy((void *)&dcfg.decap, (void *)&nl_rule->msg.create.decap.cfg, sizeof(struct nss_dtlsmgr_decap_config));
	dcfg.decap.nexthop_ifnum = NSS_N2H_INTERFACE;

	/*
	 * Create a dtls session
	 */
	ndev = nss_dtlsmgr_session_create(&dcfg);
	if (!ndev) {
		nss_nl_error("Failed to create DTLS session\n");
		return NULL;
	}

	ctx = netdev_priv(ndev);
	/*
	 * Register rx handler for dtls netdev
	 */
	rtnl_lock();
	err = netdev_rx_handler_register(ndev, nss_nldtls_dev_rx_handler, 0);
	if (err) {
		rtnl_unlock();
		nss_dtlsmgr_session_destroy(ndev);
		nss_nl_error("%px: Failed to register rx handler\n", ctx);
		return NULL;
	}

	rtnl_unlock();

	/*
	 * Prepare data for current tunnel
	 */
	dtls_tun_data = (struct nss_nldtls_tun_ctx *)kmalloc(sizeof(*dtls_tun_data), GFP_KERNEL);
	dtls_tun_data->nl_rule = nl_rule;
	memcpy(dtls_tun_data->dev_name, ndev->name, IFNAMSIZ);
	memset(&dtls_tun_data->stats, 0, sizeof(dtls_tun_data->stats));

	/*
	 * Adding tunnel to global list of tunnels
	 */
	spin_lock(&gbl_ctx.lock);
	list_add_tail(&dtls_tun_data->list, &gbl_ctx.dtls_list_head);
	spin_unlock(&gbl_ctx.lock);

	nss_nl_info("%px: Succesfully created dtls session.\n", ctx);
	return ndev;
}

/*
 * nss_nldtls_create_ipv4_rule_entry()
 *	Handler for adding ipv4 rule entry for dtls session
 */
static int nss_nldtls_create_ipv4_rule_entry(struct net_device *dtls_dev, struct nss_nldtls_rule *nl_rule)
{
	struct nss_ipv4_create ipv4;
	struct net_device *ndev;
	uint32_t if_num;

	/*
	 * Configure IPv4 rule
	 */
	memset(&ipv4, 0, sizeof(struct nss_ipv4_create));

	ipv4.from_mtu = nl_rule->msg.create.from_mtu;
	ipv4.to_mtu = nl_rule->msg.create.to_mtu;

	ndev = dev_get_by_name(&init_net, &nl_rule->msg.create.gmac_ifname[0]);
	if (ndev == NULL) {
		nss_nl_error("Can't find %s netdev\n", nl_rule->msg.create.gmac_ifname);
		return -EINVAL;
	}

	if_num = nss_cmn_get_interface_number_by_dev(ndev);
	ipv4.src_interface_num = if_num;
	ipv4.dest_interface_num = nss_dtlsmgr_get_interface(dtls_dev, NSS_DTLSMGR_INTERFACE_TYPE_OUTER);

	ipv4.src_port = nl_rule->msg.create.encap.cfg.dport;
	ipv4.src_port_xlate = nl_rule->msg.create.encap.cfg.dport;
	ipv4.src_ip = nl_rule->msg.create.encap.cfg.dip[0];
	ipv4.src_ip_xlate = nl_rule->msg.create.encap.cfg.dip[0];

	ipv4.dest_ip = nl_rule->msg.create.encap.cfg.sip[0];
	ipv4.dest_ip_xlate = nl_rule->msg.create.encap.cfg.sip[0];
	ipv4.dest_port = nl_rule->msg.create.encap.cfg.sport;
	ipv4.dest_port_xlate = nl_rule->msg.create.encap.cfg.sport;

	if (nl_rule->msg.create.flags & NSS_DTLSMGR_HDR_UDPLITE)
		ipv4.protocol = IPPROTO_UDPLITE;
	else
		ipv4.protocol = IPPROTO_UDP;

	ipv4.in_vlan_tag[0] = NSS_NLDTLS_VLAN_INVALID;
	ipv4.out_vlan_tag[0] = NSS_NLDTLS_VLAN_INVALID;
	ipv4.in_vlan_tag[1] = NSS_NLDTLS_VLAN_INVALID;
	ipv4.out_vlan_tag[1] = NSS_NLDTLS_VLAN_INVALID;

	memcpy(&ipv4.src_mac[0], &nl_rule->msg.create.gmac_ifmac[0], sizeof(ipv4.src_mac));

	dev_put(ndev);

	/*
	 * Create an ipv4 rule entry
	 */
	return nss_nldtls_create_ipv4_rule(&ipv4, 0);
}

/*
 * nss_nldtls_create_ipv6_rule_entry()
 *	Handler to add an ipv6 rule entry for dtls session
 */
static int nss_nldtls_create_ipv6_rule_entry(struct net_device *dtls_dev, struct nss_nldtls_rule *nl_rule)
{
	struct nss_ipv6_create ipv6;
	struct net_device *ndev;
	uint32_t if_num;

	/*
	 * Configure IPv6 rule
	 */
	memset(&ipv6, 0, sizeof(struct nss_ipv6_create));

	ipv6.from_mtu = nl_rule->msg.create.from_mtu;
	ipv6.to_mtu = nl_rule->msg.create.to_mtu;

	ndev = dev_get_by_name(&init_net, &nl_rule->msg.create.gmac_ifname[0]);
	if (ndev == NULL) {
		nss_nl_error("Can't find %s netdev\n", nl_rule->msg.create.gmac_ifname);
		return -EINVAL;
	}

	if_num = nss_cmn_get_interface_number_by_dev(ndev);
	ipv6.src_interface_num = if_num;
	ipv6.dest_interface_num = nss_dtlsmgr_get_interface(dtls_dev, NSS_DTLSMGR_INTERFACE_TYPE_OUTER);
	ipv6.src_port = nl_rule->msg.create.encap.cfg.dport;
	ipv6.dest_port = nl_rule->msg.create.encap.cfg.sport;

	/*
	 * Configure IPv6 rule
	 */
	memcpy(ipv6.src_ip, nl_rule->msg.create.encap.cfg.dip, sizeof(ipv6.src_ip));
	memcpy(ipv6.dest_ip, nl_rule->msg.create.encap.cfg.sip, sizeof(ipv6.dest_ip));

	if (nl_rule->msg.create.flags & NSS_DTLSMGR_HDR_UDPLITE)
		ipv6.protocol = IPPROTO_UDPLITE;
	else
		ipv6.protocol = IPPROTO_UDP;

	ipv6.in_vlan_tag[0] = NSS_NLDTLS_VLAN_INVALID;
	ipv6.in_vlan_tag[1] = NSS_NLDTLS_VLAN_INVALID;
	ipv6.out_vlan_tag[1] = NSS_NLDTLS_VLAN_INVALID;
	ipv6.out_vlan_tag[0] = NSS_NLDTLS_VLAN_INVALID;

	memcpy(&ipv6.src_mac[0], &nl_rule->msg.create.gmac_ifmac[0], sizeof(ipv6.src_mac));

	dev_put(ndev);

	/*
	 * Create an ipv6 rule entry
	 */
	return nss_nldtls_create_ipv6_rule(&ipv6, 0);
}

/*
 * nss_nldtls_destroy_tun()
 *	Common handler for tunnel destroy
 */
static int nss_nldtls_destroy_tun(struct net_device *dev)
{
	struct nss_nldtls_tun_ctx *tun;

	tun = nss_nldtls_find_tun_ctx(dev);
	if (!tun) {
		nss_nl_error("Unable to find context of the tunnel: %s\n", dev->name);
		return -EAGAIN;
	}

	/*
	 * Delete tunnel node from the list
	 */
	list_del_init(&tun->list);
	kfree(tun);

	/*
	 * Destroy the dtls session
	 */
	if (nss_dtlsmgr_session_destroy(dev)) {
		nss_nl_error("Unable to destroy the tunnel: %s\n", dev->name);
		return -EAGAIN;
	}

	return 0;
}

/*
 * nss_nldtls_ops_create_tun()
 *	Handler for creating tunnel
 */
static int nss_nldtls_ops_create_tun(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nldtls_rule *nl_rule;
	struct net_device *dtls_dev;
	struct nss_nlcmn *nl_cm;
	int ret = 0;

	/*
	 * Extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nldtls_family, info, NSS_NLDTLS_CMD_TYPE_CREATE_TUN);
	if (!nl_cm) {
		nss_nl_error("%px: Unable to extract create tunnel data\n", skb);
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nldtls_rule, cm);

	/*
	 * Create tunnel based on ip version
	 */
	if (nl_rule->msg.create.ip_version == NSS_NLDTLS_IP_VERS_4) {
		dtls_dev = nss_nldtls_create_session(nl_rule);
		if (!dtls_dev) {
			nss_nl_error("%px: Unable to create dtls session for v4\n", skb);
			return -EINVAL;
		}

		/*
		 * Create IPv4 rule entry
		 */
		ret = nss_nldtls_create_ipv4_rule_entry(dtls_dev, nl_rule);
		if (ret < 0) {
			nss_nldtls_destroy_tun(dtls_dev);
			nss_nl_error("%px Unable to add a rule entry for ipv4.\n", skb);
			return -EAGAIN;
		}

		atomic_inc(&gbl_ctx.num_tun);
		nss_nl_info("%px: Successfully created ipv4 dtls tunnel\n", skb);
	} else {
		dtls_dev = nss_nldtls_create_session(nl_rule);
		if (!dtls_dev) {
			nss_nl_error("%px: Unable to create dtls session for v6\n", skb);
			return -EINVAL;
		}

		/*
		 * Create IPv6 rule entry
		 */
		ret = nss_nldtls_create_ipv6_rule_entry(dtls_dev, nl_rule);
		if (ret < 0) {
			nss_nldtls_destroy_tun(dtls_dev);
			nss_nl_error("%px: Unable to add a rule entry for ipv6.\n", skb);
			return -EAGAIN;
		}

		atomic_inc(&gbl_ctx.num_tun);
		nss_nl_info("%px: Successfully created ipv6 dtls tunnel\n", skb);
	}

	return 0;
}

/*
 * nss_nldtls_ops_destroy_tun()
 *	Handler to destroy tunnel
 */
static int nss_nldtls_ops_destroy_tun(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nldtls_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	struct net_device *dev;
	int ret;

	/*
	 * Extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nldtls_family, info, NSS_NLDTLS_CMD_TYPE_DESTROY_TUN);
	if (!nl_cm) {
		nss_nl_error("%px: Unable to extract destroy tunnel data\n", skb);
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nldtls_rule, cm);

	dev = dev_get_by_name(&init_net, nl_rule->msg.destroy.dev_name);
	if (!dev) {
		nss_nl_error("%px Unable to find dev: %s\n", skb, nl_rule->msg.destroy.dev_name);
		return -EINVAL;
	}

	dev_put(dev);

	/*
	 * Common dtls handler for tunnel destroy
	 */
	ret = nss_nldtls_destroy_tun(dev);
	if (ret < 0) {
		nss_nl_error("%px Unable to destroy tunnel: %s\n", skb, dev->name);
		return -EAGAIN;
	}

	atomic_dec(&gbl_ctx.num_tun);
	nss_nl_info("%px: Successfully destroyed dtls session: %s\n", skb, nl_rule->msg.destroy.dev_name);
	return 0;
}

/*
 * nss_nldtls_ops_update_config()
 *	Handler for updating configuration command
 */
static int nss_nldtls_ops_update_config(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_dtlsmgr_config_update dcfg;
	struct nss_nldtls_rule *nl_rule;
	struct nss_nldtls_tun_ctx *tun;
	struct nss_dtlsmgr_ctx *ctx;
	nss_dtlsmgr_status_t status;
	struct nss_nlcmn *nl_cm;
	struct net_device *dev;
	uint16_t key_len;

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nldtls_family, info, NSS_NLDTLS_CMD_TYPE_UPDATE_CONFIG);
	if (!nl_cm) {
		nss_nl_error("%px: Unable to extract update_config data.\n", skb);
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nldtls_rule, cm);

	dev = dev_get_by_name(&init_net, nl_rule->msg.update_config.dev_name);
	if (!dev) {
		nss_nl_error("%px Unable to find dev: %s\n", skb, nl_rule->msg.update_config.dev_name);
		return -EINVAL;
	}

	ctx = netdev_priv(dev);
	tun = nss_nldtls_find_tun_ctx(dev);
	if (!tun) {
		nss_nl_error("%px Unable to find context of the tunnel: %s\n", ctx, dev->name);
		dev_put(dev);
		return -EAGAIN;
	}

	/*
	 * Configure the dtls configuration
	 */
	dcfg.crypto.algo = nl_rule->msg.update_config.config_update.crypto.algo;
	dcfg.crypto.cipher_key.data = nl_rule->msg.update_config.keys.cipher;
	dcfg.crypto.auth_key.data = nl_rule->msg.update_config.keys.auth;
	dcfg.crypto.nonce.data = nl_rule->msg.update_config.keys.nonce;
	key_len = nl_rule->msg.update_config.config_update.crypto.cipher_key.len;
	if (key_len > NSS_NLDTLS_CIPHER_KEY_MAX) {
		nss_nl_error("Invalid cipher length: %u\n", key_len);
		dev_put(dev);
		return -EINVAL;
	}

	dcfg.crypto.cipher_key.len = key_len;
	key_len = nl_rule->msg.update_config.config_update.crypto.auth_key.len;
	if (key_len > NSS_NLDTLS_AUTH_KEY_MAX) {
		nss_nl_error("Invalid authentication length: %u\n", key_len);
		dev_put(dev);
		return -EINVAL;
	}

	dcfg.crypto.auth_key.len = key_len;
	key_len = nl_rule->msg.update_config.config_update.crypto.nonce.len;
	if (key_len > NSS_NLDTLS_NONCE_SIZE_MAX) {
		nss_nl_error("Invalid nonce length: %u\n", key_len);
		dev_put(dev);
		return -EINVAL;
	}

	dcfg.crypto.nonce.len = key_len;
	dcfg.epoch = nl_rule->msg.update_config.config_update.epoch;
	dcfg.window_size = nl_rule->msg.update_config.config_update.window_size;
	if (!nl_rule->msg.update_config.dir) {
		status = nss_dtlsmgr_session_update_encap(dev, &dcfg);
		if (status != NSS_DTLSMGR_OK) {
			nss_nl_error("%px Unable to update encap configuration\n", ctx);
			dev_put(dev);
			return -EINVAL;
		}

		nss_nl_info("%px: Successfully update the encap configuration\n", ctx);
	} else {
		status = nss_dtlsmgr_session_update_decap(dev, &dcfg);
		if (status != NSS_DTLSMGR_OK) {
			nss_nl_error("%px Unable to update decap configuration\n", ctx);
			dev_put(dev);
			return -EINVAL;
		}

		nss_nl_info("%px: Successfully update the decap configuration\n", ctx);
	}

	/*
	 * Update the tun data configuration
	 */
	tun->nl_rule = nl_rule;
	dev_put(dev);
	return 0;
}

/*
 * nss_nldtls_alloc_pkt()
 *	Handler for forming ctype packet
 */
static struct sk_buff *nss_nldtls_alloc_pkt(struct nss_nldtls_rule *nl_rule,
			struct net_device *dev, uint32_t pkt_sz, uint8_t ctype)
{
	struct nss_dtlsmgr_metadata *ndm;
	uint16_t hdr_len, payload_len;
	struct sk_buff *skb;

	hdr_len = dev->needed_headroom + sizeof(*ndm);
	payload_len = hdr_len + dev->needed_tailroom + pkt_sz;
	skb = netdev_alloc_skb(dev, payload_len);
	if (!skb) {
		return NULL;
	}

	skb_reserve(skb, hdr_len);

	/*
	 * Fill the packet with dummy data
	 */
	memset(skb_put(skb, pkt_sz), NSS_NLDTLS_DUMMY_DATA, skb->len);

	ndm = nss_dtlsmgr_metadata_init(skb);
	nss_dtlsmgr_metadata_set_seq(ndm, nl_rule->msg.tx_pkts.seq_num);
	nss_dtlsmgr_metadata_set_ctype(ndm, ctype);

	if (unlikely(nl_rule->msg.tx_pkts.log_en)) {
		nss_nl_info("%px DTLS TX pkt len:%u\n", skb, skb->len);
	}

	return skb;
}

/*
 * nss_nldtls_ops_tx_pkts()
 *	Handler for sending traffic
 */
static int nss_nldtls_ops_tx_pkts(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nldtls_rule *nl_rule;
	struct nss_nldtls_tun_ctx *tun;
	struct nss_nldtls_stats *stats;
	unsigned long long duration;
	struct nss_nlcmn *nl_cm;
	struct net_device *dev;
	struct sk_buff *tx_skb;
	uint32_t num_pkts;
	uint32_t pkt_sz;
	uint32_t count;
	ktime_t delta;
	uint8_t ctype;

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nldtls_family, info, NSS_NLDTLS_CMD_TYPE_TX_PKTS);
	if (!nl_cm) {
		nss_nl_error("%px: Unable to extract tx_pkts data\n", skb);
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nldtls_rule, cm);

	ctype = nl_rule->msg.tx_pkts.ctype;
	num_pkts = nl_rule->msg.tx_pkts.num_pkts;

	switch (ctype) {
	case NSS_DTLSMGR_METADATA_CTYPE_CCS:
		pkt_sz = NSS_NLDTLS_CCS_PKT_SZ;
		break;

	case NSS_DTLSMGR_METADATA_CTYPE_ALERT:
		pkt_sz = NSS_NLDTLS_ALERT_PKT_SZ;
		break;

	case NSS_DTLSMGR_METADATA_CTYPE_HANDSHAKE:
		pkt_sz = NSS_NLDTLS_HANDSHAKE_PKT_SZ;
		break;

	case NSS_DTLSMGR_METADATA_CTYPE_APP:
		pkt_sz = nl_rule->msg.tx_pkts.pkt_sz;
		break;

	default:
		return -EINVAL;
	}

	dev = dev_get_by_name(&init_net, nl_rule->msg.tx_pkts.dev_name);
	if (!dev) {
		nss_nl_error("%px Unable to find dev: %s\n", skb, nl_rule->msg.tx_pkts.dev_name);
		return -EINVAL;
	}

	tun = nss_nldtls_find_tun_ctx(dev);
	if (!tun) {
		nss_nl_error("%px Unable to find context of the tunnel: %s\n", skb, dev->name);
		dev_put(dev);
		return -EINVAL;
	}

	spin_lock(&gbl_ctx.lock);
	gbl_ctx.log_en = nl_rule->msg.tx_pkts.log_en;
	spin_unlock(&gbl_ctx.lock);

	gbl_ctx.first_tx_pkt_time = ktime_get();

	for (count = 0; count < num_pkts; count++) {
		tx_skb = nss_nldtls_alloc_pkt(nl_rule, dev, pkt_sz, ctype);
		if (!tx_skb) {
			nss_nl_error("%px Failed to allocate skb\n", dev);
			break;
		}

		dev->netdev_ops->ndo_start_xmit(tx_skb, dev);
	}

	stats = &tun->stats[NSS_NLDTLS_CTYPE_TO_IDX(ctype)];
	spin_lock(&gbl_ctx.lock);
	stats->tx_pkts += count;
	stats->tx_bytes += (count * pkt_sz);
	spin_unlock(&gbl_ctx.lock);
	dev_put(dev);

	if (count != num_pkts) {
		nss_nl_error("%px Error in transmission\n", skb);
		return -EAGAIN;
	}

	gbl_ctx.last_tx_pkt_time = ktime_get();
	delta = ktime_sub(gbl_ctx.last_tx_pkt_time, gbl_ctx.first_tx_pkt_time);
	duration = (unsigned long long) ktime_to_ns(delta) >> 10;
	nss_nl_info("%px Packets sent in %llu usecs", dev, duration);
	nss_nl_info("%px Traffic transmission successful\n", skb);
	return 0;
}

/*
 * nss_nldtls_tunnel_stats_read()
 *	reads the per tunnel tx and rx packets stats for every ctypes
 */
static ssize_t nss_nldtls_tunnel_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *f_ppos)
{
	struct nss_nldtls_stats stats[NSS_NLDTLS_CTYPE_MAX];
	struct nss_nldtls_tun_ctx *entry;
	uint32_t max_output_lines;
	char dev_name[IFNAMSIZ];
	ssize_t bytes_read = 0;
	ssize_t size_wr = 0;
	ssize_t size_al;
	char *lbuf;

	max_output_lines = 2 + (NSS_NLDTLS_MAX_TUNNELS * NSS_NLDTLS_STATS_MAX_ROW);
	size_al = NSS_NLDTLS_STATS_MAX_STR_LEN * max_output_lines;

	lbuf = vzalloc(size_al);

	if (!lbuf) {
		nss_nl_error("%px Could not allocate buffer for debug entry\n", f_ppos);
		return 0;
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nDTLS netlink ctype stats:\n");
	list_for_each_entry(entry, &gbl_ctx.dtls_list_head, list) {
		spin_lock_bh(&gbl_ctx.lock);
		memcpy(&stats, &entry->stats, sizeof(stats));
		strlcpy(dev_name, entry->dev_name, IFNAMSIZ);
		spin_unlock_bh(&gbl_ctx.lock);

		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n--------------------------------");
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n%s:\t %s", "dev", dev_name);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n change_cipher_spec");
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n\t%s:\t %llu", "tx_pkts", stats[0].tx_pkts);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n\t%s:\t %llu", "tx_bytes", stats[0].tx_bytes);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n\t%s:\t %llu", "rx_pkts", stats[0].rx_pkts);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n\t%s:\t %llu", "rx_bytes", stats[0].rx_bytes);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n alert");
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n\t%s:\t %llu", "tx_pkts", stats[1].tx_pkts);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n\t%s:\t %llu", "tx_bytes", stats[1].tx_bytes);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n\t%s:\t %llu", "rx_pkts", stats[1].rx_pkts);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n\t%s:\t %llu", "rx_bytes", stats[1].rx_bytes);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n handshake");
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n\t%s:\t %llu", "tx_pkts", stats[2].tx_pkts);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n\t%s:\t %llu", "tx_bytes", stats[2].tx_bytes);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n\t%s:\t %llu", "rx_pkts", stats[2].rx_pkts);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n\t%s:\t %llu", "rx_bytes", stats[2].rx_bytes);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n app_data");
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n\t%s:\t %llu", "tx_pkts", stats[3].tx_pkts);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n\t%s:\t %llu", "tx_bytes", stats[3].tx_bytes);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n\t%s:\t %llu", "rx_pkts", stats[3].rx_pkts);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n\t%s:\t %llu", "rx_bytes", stats[3].rx_bytes);
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n--------------------------------\n");
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nDTLS netlink ctype stats end\n\n");
	bytes_read = simple_read_from_buffer(ubuf, sz, f_ppos, lbuf, size_wr);

	vfree(lbuf);
	return bytes_read;
}

/*
 * nss_nldtls_stats_ops()
 *	file operation handler for dentry
 */
static const struct file_operations nss_nldtls_stats_ops = {
	.read = nss_nldtls_tunnel_stats_read,
};

/*
 * nss_nldtls_init()
 *	Init handler for dtls
 */
bool nss_nldtls_init(void)
{
	int err;

	nss_nl_info_always("Init NSS netlink dtls handler\n");

	/*
	 * register NETLINK ops with the family
	 */
	err = genl_register_family(&nss_nldtls_family);
	if (err) {
		nss_nl_info_always("Error: %d unable to register gre_redir family\n", err);
		genl_unregister_family(&nss_nldtls_family);
		return false;
	}

	/*
	 * Create a debugfs entry for netlink dtls
	 */
	gbl_ctx.dentry = debugfs_create_dir("nldtls", NULL);
	if (!gbl_ctx.dentry) {
		nss_nl_info_always("Cannot create nldtls directory\n");
		goto free_family;
	}

	if (!debugfs_create_file("stats", 0400, gbl_ctx.dentry, NULL, &nss_nldtls_stats_ops)) {
		nss_nl_info_always("Cannot create nldtls dentry file\n");
		goto free_debugfs;
	}

	return true;

free_debugfs:
	debugfs_remove_recursive(gbl_ctx.dentry);
free_family:
	genl_unregister_family(&nss_nldtls_family);

	return false;
}

/*
 * nss_nldtls_exit()
 *	Exit handler for dtls
 */
bool nss_nldtls_exit(void)
{
	struct nss_nldtls_tun_ctx *entry, *tmp;
	struct net_device *dev;
	int err;

	nss_nl_info_always("Exit NSS netlink dtls handler\n");

	/*
	 * Destroy all active tunnel before exiting
	 */
	list_for_each_entry_safe(entry, tmp, &gbl_ctx.dtls_list_head, list) {
		dev = dev_get_by_name(&init_net, entry->dev_name);
		if (dev) {
			dev_put(dev);
			nss_nldtls_destroy_tun(dev);
		}
	}

	nss_nl_info_always("All active tunnels destroyed\n");

	/*
	 * unregister the ops family
	 */
	err = genl_unregister_family(&nss_nldtls_family);
	if (err) {
		nss_nl_info_always("Error: %d unable to unregister dtls NETLINK family\n", err);
		return false;
	}

	return true;
}
