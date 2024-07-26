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
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_net.h>
#include <linux/version.h>
#include <linux/sysfs.h>
#include <linux/skbuff.h>
#include <net/addrconf.h>
#include <linux/inetdevice.h>
#include <linux/vmalloc.h>
#include <linux/debugfs.h>
#include <ppe_drv.h>
#include <ppe_drv_v4.h>
#include <ppe_drv_v6.h>
#include <ppe_drv_port.h>
#include <ppe_rfs.h>
#include "ppe_rfs.h"

struct ppe_rfs gbl_ppe_rfs;

/*
 * ppe_rfs_ipv6_rule_destroy()
 * 	Destroy IPv6 PPE RFS rule
 */
enum ppe_rfs_ret ppe_rfs_ipv6_rule_destroy(struct ppe_rfs_ipv6_rule_destroy_msg *destroy_ipv6)
{
	struct ppe_rfs *p = &gbl_ppe_rfs;
	struct ppe_drv_v6_rule_destroy pd6rd = {0};

	ppe_rfs_stats_inc(&p->stats.v6_destroy_ppe_rule_rfs);

	if (ppe_drv_port_check_rfs_support(destroy_ipv6->reply_dev)) {
		pd6rd.tuple.flow_ip[0] = destroy_ipv6->tuple.flow_ip[3];
		pd6rd.tuple.flow_ip[1] = destroy_ipv6->tuple.flow_ip[2];
		pd6rd.tuple.flow_ip[2] = destroy_ipv6->tuple.flow_ip[1];
		pd6rd.tuple.flow_ip[3] = destroy_ipv6->tuple.flow_ip[0];
		pd6rd.tuple.flow_ident = destroy_ipv6->tuple.flow_ident;
		pd6rd.tuple.return_ip[0] = destroy_ipv6->tuple.return_ip[3];
		pd6rd.tuple.return_ip[1] = destroy_ipv6->tuple.return_ip[2];
		pd6rd.tuple.return_ip[2] = destroy_ipv6->tuple.return_ip[1];
		pd6rd.tuple.return_ip[3] = destroy_ipv6->tuple.return_ip[0];
		pd6rd.tuple.return_ident = destroy_ipv6->tuple.return_ident;
		pd6rd.tuple.protocol = destroy_ipv6->tuple.protocol;
	} else if (ppe_drv_port_check_rfs_support(destroy_ipv6->original_dev)) {
		pd6rd.tuple.flow_ip[0] = destroy_ipv6->tuple.return_ip[3];
		pd6rd.tuple.flow_ip[1] = destroy_ipv6->tuple.return_ip[2];
		pd6rd.tuple.flow_ip[2] = destroy_ipv6->tuple.return_ip[1];
		pd6rd.tuple.flow_ip[3] = destroy_ipv6->tuple.return_ip[0];
		pd6rd.tuple.flow_ident = destroy_ipv6->tuple.return_ident;
		pd6rd.tuple.return_ip[0] = destroy_ipv6->tuple.flow_ip[3];
		pd6rd.tuple.return_ip[1] = destroy_ipv6->tuple.flow_ip[2];
		pd6rd.tuple.return_ip[2] = destroy_ipv6->tuple.flow_ip[1];
		pd6rd.tuple.return_ip[3] = destroy_ipv6->tuple.flow_ip[0];
		pd6rd.tuple.return_ident = destroy_ipv6->tuple.flow_ident;
		pd6rd.tuple.protocol = destroy_ipv6->tuple.protocol;
	} else {
		ppe_rfs_warn("%p: RFS not enabled for both TX and RX\n", destroy_ipv6);
		ppe_rfs_stats_inc(&p->stats.v6_destroy_rfs_not_enabled);
		return PPE_RFS_RET_FAILURE;
	}

	if (ppe_drv_v6_assist_rule_destroy(&pd6rd) != PPE_DRV_RET_SUCCESS) {
		ppe_rfs_warn("%p: error in pushing Passive PPE RFS rules\n", destroy_ipv6);
		ppe_rfs_stats_inc(&p->stats.v6_destroy_ppe_rule_fail);
		return PPE_RFS_RET_FAILURE;
	}

	return PPE_RFS_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_rfs_ipv6_rule_destroy);

/*
 * ppe_rfs_ipv6_rule_create()
 * 	Create IPv6 PPE RFS rule
 */
enum ppe_rfs_ret ppe_rfs_ipv6_rule_create(struct ppe_rfs_ipv6_rule_create_msg *create_ipv6)
{
	struct ppe_drv_v6_rule_create pd6rc = {0};
	struct net_device *ppe_dev = NULL;
	struct ppe_rfs *p = &gbl_ppe_rfs;
	bool tx_rfs_enabled = false;
	bool rx_rfs_enabled = false;
	ppe_drv_ret_t ret;
	struct net_device *top_rule_rx_dev, *top_rule_tx_dev;
	ppe_drv_iface_t top_rx_if, rx_if;
	ppe_drv_iface_t top_tx_if, tx_if;

	ppe_rfs_stats_inc(&p->stats.v6_create_ppe_rule_rfs);

	ppe_dev = dev_get_by_index(&init_net, create_ipv6->conn_rule.flow_interface_num);
	if (!ppe_dev) {
		ppe_rfs_warn("dev not found during ppe dummy config for flow iface: %d\n", create_ipv6->conn_rule.flow_interface_num);
		ppe_rfs_stats_inc(&p->stats.v6_create_flow_interface_fail);
		return PPE_RFS_RET_FAILURE;
	};

	rx_if = ppe_drv_iface_idx_get_by_dev(ppe_dev);
	rx_rfs_enabled = ppe_drv_port_check_rfs_support(ppe_dev);
	dev_put(ppe_dev);

	ppe_dev = dev_get_by_index(&init_net, create_ipv6->conn_rule.return_interface_num);
	if (!ppe_dev) {
		ppe_rfs_warn("dev not found during ppe dummy config for return iface: %d\n", create_ipv6->conn_rule.return_interface_num);
		ppe_rfs_stats_inc(&p->stats.v6_create_return_interface_fail);
		return PPE_RFS_RET_FAILURE;
	};

	tx_if = ppe_drv_iface_idx_get_by_dev(ppe_dev);
	tx_rfs_enabled = ppe_drv_port_check_rfs_support(ppe_dev);
	dev_put(ppe_dev);

	top_rule_rx_dev = dev_get_by_index(&init_net, create_ipv6->conn_rule.flow_top_interface_num);
	if (!top_rule_rx_dev) {
		ppe_rfs_warn("Top rule dev not found during ppe dummy config for flow iface: %d\n", create_ipv6->conn_rule.flow_top_interface_num);
		ppe_rfs_stats_inc(&p->stats.v6_create_flow_top_interface_fail);
		return PPE_RFS_RET_FAILURE;
	};

	top_rx_if = ppe_drv_iface_idx_get_by_dev(top_rule_rx_dev);
	dev_put(top_rule_rx_dev);

	top_rule_tx_dev = dev_get_by_index(&init_net, create_ipv6->conn_rule.return_top_interface_num);
	if (!top_rule_tx_dev) {
		ppe_rfs_warn("Top rule return dev not found during ppe dummy config for flow iface: %d\n", create_ipv6->conn_rule.return_top_interface_num);
		ppe_rfs_stats_inc(&p->stats.v6_create_return_top_interface_fail);
		return PPE_RFS_RET_FAILURE;
	};

	top_tx_if = ppe_drv_iface_idx_get_by_dev(top_rule_tx_dev);
	dev_put(top_rule_tx_dev);

	if (tx_rfs_enabled && rx_rfs_enabled) {
		ppe_rfs_warn("RFS enabled on both tx(%d) and rx interface(%d)\n", create_ipv6->conn_rule.return_interface_num, create_ipv6->conn_rule.flow_interface_num);
		ppe_rfs_stats_inc(&p->stats.v6_create_rfs_direction_check_fail);
		return PPE_RFS_RET_FAILURE;
	}

	if (!tx_rfs_enabled && !rx_rfs_enabled) {
		ppe_rfs_warn("RFS disable on both tx(%d) and rx interface(%d)\n", create_ipv6->conn_rule.return_interface_num, create_ipv6->conn_rule.flow_interface_num);
		ppe_rfs_stats_inc(&p->stats.v6_create_rfs_not_enabled);
		return PPE_RFS_RET_FAILURE;
	}

	if (create_ipv6->rule_flags & PPE_RFS_V6_RULE_FLAG_BRIDGE_FLOW) {
		pd6rc.rule_flags |= PPE_DRV_V6_RULE_FLAG_BRIDGE_FLOW;
	}

	if (tx_rfs_enabled) {
		pd6rc.tuple.flow_ip[0] = create_ipv6->tuple.flow_ip[3];
		pd6rc.tuple.flow_ip[1] = create_ipv6->tuple.flow_ip[2];
		pd6rc.tuple.flow_ip[2] = create_ipv6->tuple.flow_ip[1];
		pd6rc.tuple.flow_ip[3] = create_ipv6->tuple.flow_ip[0];
		pd6rc.tuple.flow_ident = create_ipv6->tuple.flow_ident;
		pd6rc.tuple.return_ip[0] = create_ipv6->tuple.return_ip[3];
		pd6rc.tuple.return_ip[1] = create_ipv6->tuple.return_ip[2];
		pd6rc.tuple.return_ip[2] = create_ipv6->tuple.return_ip[1];
		pd6rc.tuple.return_ip[3] = create_ipv6->tuple.return_ip[0];
		pd6rc.tuple.return_ident = create_ipv6->tuple.return_ident;
		pd6rc.tuple.protocol = create_ipv6->tuple.protocol;
		pd6rc.conn_rule.flow_mtu = create_ipv6->conn_rule.return_mtu;

		/*
		 * Fill from and to interface for this direction.
		 */
		pd6rc.conn_rule.rx_if = rx_if;
		pd6rc.top_rule.rx_if = top_rx_if;
		pd6rc.conn_rule.tx_if = tx_if;
		pd6rc.top_rule.tx_if = top_tx_if;

		if (create_ipv6->valid_flags & PPE_DRV_V6_VALID_FLAG_QOS) {
			pd6rc.qos_rule.flow_qos_tag = create_ipv6->qos_rule.flow_qos_tag;
			pd6rc.qos_rule.return_qos_tag = create_ipv6->qos_rule.return_qos_tag;
			pd6rc.valid_flags |= PPE_DRV_V6_VALID_FLAG_QOS;
		}

	} else if (rx_rfs_enabled) {
		pd6rc.tuple.flow_ip[0] = create_ipv6->tuple.return_ip[3];
		pd6rc.tuple.flow_ip[1] = create_ipv6->tuple.return_ip[2];
		pd6rc.tuple.flow_ip[2] = create_ipv6->tuple.return_ip[1];
		pd6rc.tuple.flow_ip[3] = create_ipv6->tuple.return_ip[0];
		pd6rc.tuple.flow_ident = create_ipv6->tuple.return_ident;
		pd6rc.tuple.return_ip[0] = create_ipv6->tuple.flow_ip[3];
		pd6rc.tuple.return_ip[1] = create_ipv6->tuple.flow_ip[2];
		pd6rc.tuple.return_ip[2] = create_ipv6->tuple.flow_ip[1];
		pd6rc.tuple.return_ip[3] = create_ipv6->tuple.flow_ip[0];
		pd6rc.tuple.return_ident = create_ipv6->tuple.flow_ident;
		pd6rc.tuple.protocol = create_ipv6->tuple.protocol;
		pd6rc.conn_rule.flow_mtu = create_ipv6->conn_rule.flow_mtu;

		/*
		 * Fill from and to interface for this direction.
		 */
		pd6rc.conn_rule.rx_if = tx_if;
		pd6rc.top_rule.rx_if = top_tx_if;
		pd6rc.conn_rule.tx_if = rx_if;
		pd6rc.top_rule.tx_if = top_rx_if;

		if (create_ipv6->valid_flags & PPE_DRV_V6_VALID_FLAG_QOS) {
			pd6rc.qos_rule.flow_qos_tag = create_ipv6->qos_rule.return_qos_tag;
			pd6rc.qos_rule.return_qos_tag = create_ipv6->qos_rule.flow_qos_tag;
			pd6rc.valid_flags |= PPE_DRV_V6_VALID_FLAG_QOS;
		}
	}

	pd6rc.rule_flags |= PPE_DRV_V6_RULE_FLAG_FLOW_VALID;

	ret = ppe_drv_v6_assist_rule_create(&pd6rc, PPE_DRV_ASSIST_FEATURE_RFS);
	if (ret != PPE_DRV_RET_SUCCESS) {
		ppe_rfs_warn("%p: Error in pushing Passive PPE RFS rules\n", create_ipv6);
		ppe_rfs_stats_inc(&p->stats.v6_create_ppe_rule_fail);
		return PPE_RFS_RET_FAILURE;
	}

	return PPE_RFS_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_rfs_ipv6_rule_create);

/*
 * ppe_rfs_ipv4_rule_destroy()
 * 	Destroy IPv4 PPE RFS rule
 */
enum ppe_rfs_ret ppe_rfs_ipv4_rule_destroy(struct ppe_rfs_ipv4_rule_destroy_msg *destroy_ipv4)
{
	struct ppe_rfs *p = &gbl_ppe_rfs;
	struct ppe_drv_v4_rule_destroy pd4rd = {0};

	ppe_rfs_stats_inc(&p->stats.v4_destroy_ppe_rule_rfs);

	if (ppe_drv_port_check_rfs_support(destroy_ipv4->reply_dev)) {
		pd4rd.tuple.flow_ip = destroy_ipv4->tuple.flow_ip;
		pd4rd.tuple.flow_ident = destroy_ipv4->tuple.flow_ident;
		pd4rd.tuple.return_ip = destroy_ipv4->tuple.return_ip;
		pd4rd.tuple.return_ident = destroy_ipv4->tuple.return_ident;
		pd4rd.tuple.protocol = destroy_ipv4->tuple.protocol;
	} else if (ppe_drv_port_check_rfs_support(destroy_ipv4->original_dev)) {
		/*
		 * TODO: Fix this with proper xlate ip's later when we support NAT with RFS
		 */
		pd4rd.tuple.flow_ip = destroy_ipv4->tuple.return_ip;
		pd4rd.tuple.flow_ident = destroy_ipv4->tuple.return_ident;
		pd4rd.tuple.return_ip = destroy_ipv4->tuple.flow_ip;
		pd4rd.tuple.return_ident = destroy_ipv4->tuple.flow_ident;
		pd4rd.tuple.protocol = destroy_ipv4->tuple.protocol;
	} else {
		ppe_rfs_warn("%p: RFS not enabled for both TX and RX\n", destroy_ipv4);
		ppe_rfs_stats_inc(&p->stats.v4_destroy_rfs_not_enabled);
		return PPE_RFS_RET_FAILURE;
	}

	if (ppe_drv_v4_assist_rule_destroy(&pd4rd) != PPE_DRV_RET_SUCCESS) {
		ppe_rfs_stats_inc(&p->stats.v4_destroy_ppe_rule_fail);
		return PPE_RFS_RET_FAILURE;
	}

	return PPE_RFS_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_rfs_ipv4_rule_destroy);

/*
 * ppe_rfs_ipv4_rule_create()
 * 	Create IPv4 PPE RFS rule
 */
enum ppe_rfs_ret ppe_rfs_ipv4_rule_create(struct ppe_rfs_ipv4_rule_create_msg *create_ipv4)
{
	struct ppe_drv_v4_rule_create pd4rc = {0};
	struct net_device *ppe_dev = NULL;
	struct ppe_rfs *p = &gbl_ppe_rfs;
	bool tx_rfs_enabled = false;
	bool rx_rfs_enabled = false;
	ppe_drv_ret_t ret;
	struct net_device *top_rule_rx_dev, *top_rule_tx_dev;
	ppe_drv_iface_t top_rx_if, rx_if;
	ppe_drv_iface_t top_tx_if, tx_if;

	ppe_rfs_stats_inc(&p->stats.v4_create_ppe_rule_rfs);

	ppe_dev = dev_get_by_index(&init_net, create_ipv4->conn_rule.flow_interface_num);
	if (!ppe_dev) {
		ppe_rfs_warn("dev not found during ppe dummy config for flow iface: %d\n", create_ipv4->conn_rule.flow_interface_num);
		ppe_rfs_stats_inc(&p->stats.v4_create_flow_interface_fail);
		return PPE_RFS_RET_FAILURE;
	};

	rx_if = ppe_drv_iface_idx_get_by_dev(ppe_dev);
	rx_rfs_enabled = ppe_drv_port_check_rfs_support(ppe_dev);
	dev_put(ppe_dev);

	ppe_dev = dev_get_by_index(&init_net, create_ipv4->conn_rule.return_interface_num);
	if (!ppe_dev) {
		ppe_rfs_warn("dev not found during ppe dummy config for return iface: %d\n", create_ipv4->conn_rule.return_interface_num);
		ppe_rfs_stats_inc(&p->stats.v4_create_return_interface_fail);
		return PPE_RFS_RET_FAILURE;
	};

	tx_if = ppe_drv_iface_idx_get_by_dev(ppe_dev);
	tx_rfs_enabled = ppe_drv_port_check_rfs_support(ppe_dev);
	dev_put(ppe_dev);

	if (tx_rfs_enabled && rx_rfs_enabled) {
		ppe_rfs_warn("RFS enabled on both tx(%d) and rx interface(%d)\n", create_ipv4->conn_rule.return_interface_num, create_ipv4->conn_rule.flow_interface_num);
		ppe_rfs_stats_inc(&p->stats.v4_create_rfs_direction_check_fail);
		return PPE_RFS_RET_FAILURE;
	}

	top_rule_rx_dev = dev_get_by_index(&init_net, create_ipv4->conn_rule.flow_top_interface_num);
	if (!top_rule_rx_dev) {
		ppe_rfs_warn("Top rule dev not found during ppe dummy config for flow iface: %d\n", create_ipv4->conn_rule.flow_top_interface_num);
		ppe_rfs_stats_inc(&p->stats.v4_create_flow_top_interface_fail);
		return PPE_RFS_RET_FAILURE;
	};

	top_rx_if = ppe_drv_iface_idx_get_by_dev(top_rule_rx_dev);
	dev_put(top_rule_rx_dev);

	top_rule_tx_dev = dev_get_by_index(&init_net, create_ipv4->conn_rule.return_top_interface_num);
	if (!top_rule_tx_dev) {
		ppe_rfs_warn("Top rule return dev not found during ppe dummy config for flow iface: %d\n", create_ipv4->conn_rule.return_top_interface_num);
		ppe_rfs_stats_inc(&p->stats.v4_create_return_top_interface_fail);
		return PPE_RFS_RET_FAILURE;
	};

	top_tx_if = ppe_drv_iface_idx_get_by_dev(top_rule_tx_dev);
	dev_put(top_rule_tx_dev);

	if (!tx_rfs_enabled && !rx_rfs_enabled) {
		ppe_rfs_warn("RFS disable on both tx(%d) and rx interface(%d)\n", create_ipv4->conn_rule.return_interface_num, create_ipv4->conn_rule.flow_interface_num);
		ppe_rfs_stats_inc(&p->stats.v4_create_rfs_not_enabled);
		return PPE_RFS_RET_FAILURE;
	}

	if (tx_rfs_enabled) {
		pd4rc.tuple.flow_ip = create_ipv4->tuple.flow_ip;
		pd4rc.tuple.flow_ident = create_ipv4->tuple.flow_ident;
		pd4rc.tuple.return_ip = create_ipv4->tuple.return_ip;
		pd4rc.tuple.return_ident = create_ipv4->tuple.return_ident;
		pd4rc.tuple.protocol = create_ipv4->tuple.protocol;
		pd4rc.conn_rule.flow_mtu = create_ipv4->conn_rule.return_mtu;

		/*
		 * PPE RFS does not support NAT on egress interface; hence filling xlate apis
		 * to be same as that of original tupe
		 */
		pd4rc.conn_rule.flow_ip_xlate =  pd4rc.tuple.flow_ip;
		pd4rc.conn_rule.flow_ident_xlate =  pd4rc.tuple.flow_ident;
		pd4rc.conn_rule.return_ip_xlate =  pd4rc.tuple.return_ip;
		pd4rc.conn_rule.return_ident_xlate =  pd4rc.tuple.return_ident;

		/*
		 * Fill from and to interface for this direction.
		 */
		pd4rc.conn_rule.rx_if = rx_if;
		pd4rc.top_rule.rx_if = top_rx_if;
		pd4rc.conn_rule.tx_if = tx_if;
		pd4rc.top_rule.tx_if = top_tx_if;

		if (create_ipv4->valid_flags & PPE_DRV_V4_VALID_FLAG_QOS) {
			pd4rc.qos_rule.flow_qos_tag = create_ipv4->qos_rule.flow_qos_tag;
			pd4rc.qos_rule.return_qos_tag = create_ipv4->qos_rule.return_qos_tag;
			pd4rc.valid_flags |= PPE_DRV_V4_VALID_FLAG_QOS;
		}

	} else if (rx_rfs_enabled) {
		pd4rc.tuple.flow_ip = create_ipv4->conn_rule.return_ip_xlate;
		pd4rc.tuple.flow_ident = create_ipv4->conn_rule.return_ident_xlate;
		pd4rc.tuple.return_ip = create_ipv4->conn_rule.flow_ip_xlate;
		pd4rc.tuple.return_ident = create_ipv4->conn_rule.flow_ident_xlate;
		pd4rc.tuple.protocol = create_ipv4->tuple.protocol;
		pd4rc.conn_rule.flow_mtu = create_ipv4->conn_rule.flow_mtu;

		/*
		 * TODO: Fix this xlate ip's when RFS + NAT is supported
		 */
		pd4rc.conn_rule.flow_ip_xlate = pd4rc.tuple.flow_ip;
		pd4rc.conn_rule.flow_ident_xlate = pd4rc.tuple.flow_ident;
		pd4rc.conn_rule.return_ip_xlate = pd4rc.tuple.return_ip;
		pd4rc.conn_rule.return_ident_xlate = pd4rc.tuple.return_ident;

		/*
		 * Fill from and to interface for this direction.
		 */
		pd4rc.conn_rule.rx_if = tx_if;
		pd4rc.top_rule.rx_if = top_tx_if;
		pd4rc.conn_rule.tx_if = rx_if;
		pd4rc.top_rule.tx_if = top_rx_if;

		if (create_ipv4->valid_flags & PPE_DRV_V4_VALID_FLAG_QOS) {
			pd4rc.qos_rule.flow_qos_tag = create_ipv4->qos_rule.return_qos_tag;
			pd4rc.qos_rule.return_qos_tag = create_ipv4->qos_rule.flow_qos_tag;
			pd4rc.valid_flags |= PPE_DRV_V4_VALID_FLAG_QOS;
		}
	}

	if (create_ipv4->rule_flags & PPE_RFS_V4_RULE_FLAG_BRIDGE_FLOW) {
		pd4rc.rule_flags |= PPE_DRV_V4_RULE_FLAG_BRIDGE_FLOW;
	}

	pd4rc.rule_flags |= PPE_DRV_V4_RULE_FLAG_FLOW_VALID;

	ret = ppe_drv_v4_assist_rule_create(&pd4rc, PPE_DRV_ASSIST_FEATURE_RFS);
	if (ret != PPE_DRV_RET_SUCCESS) {
		ppe_rfs_warn("%p: Error in pushing Passive PPE RFS rules\n", create_ipv4);
		ppe_rfs_stats_inc(&p->stats.v4_create_ppe_rule_fail);
		return PPE_RFS_RET_FAILURE;
	}

	return PPE_RFS_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_rfs_ipv4_rule_create);

/*
 * ppe_rfs_deinit()
 *	RFS deinit API
 */
void ppe_rfs_deinit(void)
{
	ppe_rfs_stats_debugfs_exit();
}

/*
 * ppe_rfs_init()
 *	RFS init API
 */
void ppe_rfs_init(struct dentry *d_rule)
{
	struct ppe_rfs *g_rfs = &gbl_ppe_rfs;
	spin_lock_init(&g_rfs->lock);
	ppe_rfs_stats_debugfs_init(d_rule);
}
