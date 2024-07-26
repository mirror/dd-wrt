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
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/vmalloc.h>
#include <linux/debugfs.h>
#include <fal/fal_rss_hash.h>
#include <fal/fal_ip.h>
#include <fal/fal_init.h>
#include <fal/fal_qm.h>
#include <fal/fal_fdb.h>
#include "ppe_drv.h"
#include "tun/ppe_drv_tun.h"

#define PPE_DRV_STATIC_DBG_LEVEL_STR_LEN 8

/*
 * Module parameter to enable/disable 2-tuple RSS hash for IP fragments.
 */
static bool ipfrag_2tuple_hash = true;
module_param(ipfrag_2tuple_hash, bool, 0644);
MODULE_PARM_DESC(ipfrag_2tuple_hash, "RSS hash for IP fragments based on SIP & DIP");

uint32_t if_bm_to_offload;
bool disable_port_mtu_check = true;
uint32_t static_dbg_level = 0;
static char static_dbg_level_str[PPE_DRV_STATIC_DBG_LEVEL_STR_LEN];
uint8_t ppe_drv_redir_prio_map[PPE_DRV_MAX_PRIORITY] = {0, 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7};
static bool eth2eth_offload_if_bitmap;

/*
 * Define the filename to be used for assertions.
 */
struct ppe_drv ppe_drv_gbl;

/*
 * ppe_drv_get_vxlan_dport()
 * Get the VXLAN destination port.
 */
int ppe_drv_get_vxlan_dport(void)
{
	return ppe_drv_gbl.vxlan_dport;
}

/*
 * ppe_drv_is_mht_dev()
 *	API to get MHT switch interface flag
 */
bool ppe_drv_is_mht_dev(struct net_device *dev)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_iface *iface = NULL;

	spin_lock_bh(&p->lock);
	iface = ppe_drv_iface_get_by_dev_internal(dev);
	if (!iface) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: No valid PPE interface for dev", dev);
		return false;
	}

	/*
	 * Get the MHT switch flag on the interface.
	 */
	if (!(iface->flags & PPE_DRV_IFACE_FLAG_MHT_SWITCH_VALID)) {
		spin_unlock_bh(&p->lock);
		return false;
	}
	spin_unlock_bh(&p->lock);
	return true;
}
EXPORT_SYMBOL(ppe_drv_is_mht_dev);

/*
 * ppe_drv_hw_stats_sync()
 *	Sync PPE HW stats
 */
static void ppe_drv_hw_stats_sync(struct timer_list *tm)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_v4_conn *cn_v4;
	struct ppe_drv_v6_conn *cn_v6;
#ifdef PPE_TUNNEL_ENABLE
	struct ppe_drv_v4_conn *cn_tun_v4;
	struct ppe_drv_v6_conn *cn_tun_v6;
#endif

	struct ppe_drv_v4_conn_flow *pcf_v4;
	struct ppe_drv_v4_conn_flow *pcr_v4;
	struct ppe_drv_v6_conn_flow *pcf_v6;
	struct ppe_drv_v6_conn_flow *pcr_v6;
	uint16_t id;

	/*
	 * Update hw stats for flow associated with active v4 connections
	 */
	spin_lock_bh(&p->lock);
	if (!list_empty(&p->conn_v4)) {
		list_for_each_entry(cn_v4, &p->conn_v4, list) {
			pcf_v4 = &cn_v4->pcf;
			pcr_v4 = &cn_v4->pcr;

			ppe_drv_flow_v4_stats_update(pcf_v4);
			if (pcr_v4) {
				ppe_drv_flow_v4_stats_update(pcr_v4);
			}
		}
	}

	/*
	 * Update hw stats for flow associated with active v6 connections
	 */
	if (!list_empty(&p->conn_v6)) {
		list_for_each_entry(cn_v6, &p->conn_v6, list) {
			pcf_v6 = &cn_v6->pcf;
			pcr_v6 = &cn_v6->pcr;

			ppe_drv_flow_v6_stats_update(pcf_v6);
			if (pcr_v6) {
				ppe_drv_flow_v6_stats_update(pcr_v6);
			}
		}
	}

#ifdef PPE_TUNNEL_ENABLE
	/*
	 * Update hw stats for tunnels associated with active v4 connections
	 */

	if (!list_empty(&p->conn_tun_v4)) {
		list_for_each_entry(cn_tun_v4, &p->conn_tun_v4, list) {
			ppe_drv_tun_v4_port_stats_update(cn_tun_v4);
		}
	}

	/*
	 * Update hw stats for tunnels associated with active v6 connections
	 */
	if (!list_empty(&p->conn_tun_v6)) {
		list_for_each_entry(cn_tun_v6, &p->conn_tun_v6, list) {
			/*
			 * Mapt outer rule for tunnel stats must not be updated as
			 * it would be mapped to the innner v4 flow stats
			 */
			if (ppe_drv_v6_conn_flags_check(cn_tun_v6, PPE_DRV_V6_CONN_FLAG_TYPE_MAPT)) {
				continue;
			}

			ppe_drv_tun_v6_port_stats_update(cn_tun_v6);
		}
	}
#endif
	for (id = 0; id < PPE_DRV_ACL_LIST_ID_MAX; id++) {
		if (p->acl->list_id[id].list_id_state == PPE_DRV_ACL_LIST_ID_USED) {
			ppe_drv_acl_stats_update(p->acl->list_id[id].ctx);
		}
	}

	spin_unlock_bh(&p->lock);

	/*
	 * Re arm the hardware stats timer
	 */
	mod_timer(&p->hw_flow_stats_timer, jiffies + p->hw_flow_stats_ticks);
}

/*
 * ppe_drv_nsm_queue_stats_update()
 *	Get queue stats from fal to nsm.
 */
bool ppe_drv_nsm_queue_stats_update(struct ppe_drv_nsm_stats *nsm_stats, uint32_t queue_id, uint8_t item_id)
{
	fal_queue_stats_t queue_info;
	sw_error_t err;

	if (item_id >= FAL_QM_DROP_ITEMS) {
		ppe_drv_warn("invalid drop item id: %u", item_id);
		return false;
	}

	err = fal_queue_counter_get(PPE_DRV_SWITCH_ID, queue_id, &queue_info);
	if (err != SW_OK) {
		ppe_drv_warn("failed to get stats for queue id: %u", queue_id);
		return false;
	}

	nsm_stats->queue_stats.drop_packets = queue_info.drop_packets[item_id];
	nsm_stats->queue_stats.drop_bytes = queue_info.drop_bytes[item_id];
	ppe_drv_trace("queue id: %u, item id: %u, drop_packet = %llu, drop_bytes = %llu", queue_id, item_id, nsm_stats->queue_stats.drop_packets, nsm_stats->queue_stats.drop_bytes);

	return true;
}
EXPORT_SYMBOL(ppe_drv_nsm_queue_stats_update);

/*
 * ppe_drv_nsm_sawf_sc_stats_read()
 *	Export service-class stats to nsm.
 */
bool ppe_drv_nsm_sawf_sc_stats_read(struct ppe_drv_nsm_stats *nsm_stats, uint8_t service_class)
{
	struct ppe_drv_stats_sawf_sc *sawf_sc_stats;
	struct ppe_drv *p = &ppe_drv_gbl;

	if (!PPE_DRV_SERVICE_CLASS_IS_VALID(service_class)) {
		ppe_drv_warn("%u Invalid SAWF service class", service_class);
		return false;
	}

	spin_lock_bh(&p->lock);

	sawf_sc_stats = &ppe_drv_gbl.stats.sawf_sc_stats[service_class];

	nsm_stats->sawf_sc_stats.rx_packets = atomic64_read(&sawf_sc_stats->rx_packets);
	nsm_stats->sawf_sc_stats.rx_bytes = atomic64_read(&sawf_sc_stats->rx_bytes);
	nsm_stats->sawf_sc_stats.flow_count = atomic64_read(&sawf_sc_stats->flow_count);

	spin_unlock_bh(&p->lock);

	ppe_drv_trace("Stats Updated: service class %u : packets %llu : bytes %llu : flows %u\n",
			service_class, nsm_stats->sawf_sc_stats.rx_packets, nsm_stats->sawf_sc_stats.rx_bytes, nsm_stats->sawf_sc_stats.flow_count);
	return true;
}
EXPORT_SYMBOL(ppe_drv_nsm_sawf_sc_stats_read);

/*
 * ppe_drv_hash_init()
 *	Initialize the PPE hash registers
 */
static bool ppe_drv_hash_init(void)
{
	fal_rss_hash_mode_t mode = { 0 };
	fal_rss_hash_config_t config = { 0 };

	mode = FAL_RSS_HASH_IPV4ONLY;
	config.hash_mask = PPE_DRV_HASH_MASK;
	config.hash_fragment_mode = ipfrag_2tuple_hash;
	config.hash_seed = PPE_DRV_HASH_SEED_DEFAULT;
	config.hash_sip_mix[0] = PPE_DRV_HASH_MIX_V4_SIP;
	config.hash_dip_mix[0] = PPE_DRV_HASH_MIX_V4_DIP;
	config.hash_protocol_mix = PPE_DRV_HASH_MIX_V4_PROTO;
	config.hash_sport_mix = PPE_DRV_HASH_MIX_V4_SPORT;
	config.hash_dport_mix = PPE_DRV_HASH_MIX_V4_DPORT;

	config.hash_fin_inner[0] = (PPE_DRV_HASH_FIN_INNER_OUTER_0 & PPE_DRV_HASH_FIN_MASK);
	config.hash_fin_outer[0] = ((PPE_DRV_HASH_FIN_INNER_OUTER_0 >> 5) & PPE_DRV_HASH_FIN_MASK);
	config.hash_fin_inner[1] = (PPE_DRV_HASH_FIN_INNER_OUTER_1 & PPE_DRV_HASH_FIN_MASK);
	config.hash_fin_outer[1] = ((PPE_DRV_HASH_FIN_INNER_OUTER_1 >> 5) & PPE_DRV_HASH_FIN_MASK);
	config.hash_fin_inner[2] = (PPE_DRV_HASH_FIN_INNER_OUTER_2 & PPE_DRV_HASH_FIN_MASK);
	config.hash_fin_outer[2] = ((PPE_DRV_HASH_FIN_INNER_OUTER_2 >> 5) & PPE_DRV_HASH_FIN_MASK);
	config.hash_fin_inner[3] = (PPE_DRV_HASH_FIN_INNER_OUTER_3 & PPE_DRV_HASH_FIN_MASK);
	config.hash_fin_outer[3] = ((PPE_DRV_HASH_FIN_INNER_OUTER_3 >> 5) & PPE_DRV_HASH_FIN_MASK);
	config.hash_fin_inner[4] = (PPE_DRV_HASH_FIN_INNER_OUTER_4 & PPE_DRV_HASH_FIN_MASK);
	config.hash_fin_outer[4] = ((PPE_DRV_HASH_FIN_INNER_OUTER_4 >> 5) & PPE_DRV_HASH_FIN_MASK);

	if (fal_rss_hash_config_set(PPE_DRV_SWITCH_ID, mode, &config) != SW_OK) {
		ppe_drv_warn("IPv4 hash register initialization failed\n");
		return false;
	}

	mode = FAL_RSS_HASH_IPV6ONLY;
	config.hash_sip_mix[0] = PPE_DRV_HASH_SIPV6_MIX_0;
	config.hash_dip_mix[0] = PPE_DRV_HASH_DIPV6_MIX_0;
	config.hash_sip_mix[1] = PPE_DRV_HASH_SIPV6_MIX_1;
	config.hash_dip_mix[1] = PPE_DRV_HASH_DIPV6_MIX_1;
	config.hash_sip_mix[2] = PPE_DRV_HASH_SIPV6_MIX_2;
	config.hash_dip_mix[2] = PPE_DRV_HASH_DIPV6_MIX_2;
	config.hash_sip_mix[3] = PPE_DRV_HASH_SIPV6_MIX_3;
	config.hash_dip_mix[3] = PPE_DRV_HASH_DIPV6_MIX_3;

	if (fal_rss_hash_config_set(PPE_DRV_SWITCH_ID, mode, &config) != SW_OK) {
		ppe_drv_warn("IPv6 hash register initialization failed\n");
		return false;
	}

	return true;
}

/*
 * ppe_drv phy_port_base_queue_init()
 *	Initialize PPE port 0 - 7 base queue
 */
static bool ppe_drv_phy_port_base_queue_init(struct ppe_drv *p)
{
	fal_ucast_queue_dest_t q_dst = {0};
	uint32_t queue_id = 0;
	sw_error_t err;
	uint8_t profile = 0;
	int i = 0;

	for (i = 0; i < PPE_DRV_PHYSICAL_MAX; i++) {
		q_dst.src_profile = 0;
		q_dst.dst_port = i;

		err = fal_ucast_queue_base_profile_get(PPE_DRV_SWITCH_ID, &q_dst, &queue_id, &profile);
		if (err != SW_OK) {
			ppe_drv_warn("%p unable to get port queue base ID: %d", p, err);
			return false;
		}

		ppe_drv_port_ucast_queue_update(&p->port[i], (uint8_t)queue_id);
	}

	return true;
}

/*
 * ppe_drv_fse_feature_enable()
 *	Enable PPE FSE feature
 */
void ppe_drv_fse_feature_enable()
{
	struct ppe_drv *p = &ppe_drv_gbl;

	spin_lock_bh(&p->lock);
	p->fse_enable = true;
	spin_unlock_bh(&p->lock);
}
EXPORT_SYMBOL(ppe_drv_fse_feature_enable);

/*
 * ppe_drv_fse_feature_disable()
 *	Disable PPE FSE feature
 */
void ppe_drv_fse_feature_disable()
{
	struct ppe_drv *p = &ppe_drv_gbl;

	spin_lock_bh(&p->lock);
	p->fse_enable = false;
	spin_unlock_bh(&p->lock);
}
EXPORT_SYMBOL(ppe_drv_fse_feature_disable);

/*
 * ppe_drv_core2queue_mapping()
 *	Core to queue mapping
 *
 * This API will be invoked by DP driver to provide core to queue
 * mapping. This internally will be used to configure service code
 * to queue mapping for PPE RFS feature.
 */
void ppe_drv_core2queue_mapping(uint8_t core, uint8_t queue_id)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	if (core >= NR_CPUS) {
		ppe_drv_warn("%p: invalid core-id: %d", p, core);
		return;
	}

	ppe_drv_trace("%d: queue mapping called for core(%d)\n", queue_id, core);

	switch(core) {
	case 0:
		ppe_drv_sc_ucast_queue_set(PPE_DRV_SC_NOEDIT_REDIR_CORE0, queue_id, PPE_DRV_REDIR_PROFILE_ID);
		ppe_drv_sc_ucast_queue_set(PPE_DRV_SC_EDIT_REDIR_CORE0, queue_id, PPE_DRV_REDIR_PROFILE_ID);
		break;
	case 1:
		ppe_drv_sc_ucast_queue_set(PPE_DRV_SC_NOEDIT_REDIR_CORE1, queue_id, PPE_DRV_REDIR_PROFILE_ID);
		ppe_drv_sc_ucast_queue_set(PPE_DRV_SC_EDIT_REDIR_CORE1, queue_id, PPE_DRV_REDIR_PROFILE_ID);
		break;
	case 2:
		ppe_drv_sc_ucast_queue_set(PPE_DRV_SC_NOEDIT_REDIR_CORE2, queue_id, PPE_DRV_REDIR_PROFILE_ID);
		ppe_drv_sc_ucast_queue_set(PPE_DRV_SC_EDIT_REDIR_CORE2, queue_id, PPE_DRV_REDIR_PROFILE_ID);
		break;
	case 3:
		ppe_drv_sc_ucast_queue_set(PPE_DRV_SC_NOEDIT_REDIR_CORE3, queue_id, PPE_DRV_REDIR_PROFILE_ID);
		ppe_drv_sc_ucast_queue_set(PPE_DRV_SC_EDIT_REDIR_CORE3, queue_id, PPE_DRV_REDIR_PROFILE_ID);
		break;
	default:
		ppe_drv_warn("%d Invalid core(%d)\n", queue_id, core);
		return;
	}

	p->core2queue[core] = queue_id;
}
EXPORT_SYMBOL(ppe_drv_core2queue_mapping);

/*
 * ppe_drv_queue_from_core()
 *	Get base queue for a specific core.
 */
int16_t ppe_drv_queue_from_core(uint8_t core)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	if (core >= NR_CPUS) {
		ppe_drv_warn("%p: invalid core-id: %d", p, core);
		return -1;
	}

	return p->core2queue[core];
}
EXPORT_SYMBOL(ppe_drv_queue_from_core);

/*
 * ppe_drv_ds_map_node_to_queue()
 *	node to queue mapping
 *
 * This API will be invoked by DP driver to provide node to queue mapping.
 */
void ppe_drv_ds_map_node_to_queue(uint8_t node_id, uint8_t queue_id)
{
	if (node_id > PPE_DRV_DS_MLO_LINK_NODE_ID_MAX) {
		ppe_drv_warn("Invalid node ID %d, for queue id %d\n", node_id, queue_id);
		return;
	}

	ppe_drv_sc_ucast_queue_set(PPE_DRV_SC_DS_MLO_LINK_RO_NODE0 + node_id, queue_id, PPE_DRV_REDIR_PROFILE_ID);
	ppe_drv_sc_ucast_queue_set(PPE_DRV_SC_DS_MLO_LINK_BR_NODE0 + node_id, queue_id, PPE_DRV_REDIR_PROFILE_ID);
}
EXPORT_SYMBOL(ppe_drv_ds_map_node_to_queue);

/*
 * ppe_drv_l3_route_ctrl_init()
 *	Initialize PPE global configuration
 */
static bool ppe_drv_l3_route_ctrl_init(struct ppe_drv *p)
{
	fal_ip_global_cfg_t cfg = { 0 };

	/*
	 * Global MTU and MRU cofiguration check; if flows MTU or MRU is not as
	 * expected by PPE then flow will be deaccelerated by PPE and packets will
	 * be redirected to CPU
	 */
	cfg.mru_fail_action = FAL_MAC_RDT_TO_CPU;
	cfg.mru_deacclr_en = A_TRUE;
	cfg.mtu_fail_action = FAL_MAC_RDT_TO_CPU;
	cfg.mtu_deacclr_en = A_TRUE;

	/*
	 * Don't deaccelerate flow based on DF bit.
	 */
	cfg.mtu_nonfrag_fail_action = FAL_MAC_RDT_TO_CPU;
	cfg.mtu_df_deacclr_en = A_FALSE;

	if (fal_ip_global_ctrl_set(PPE_DRV_SWITCH_ID, &cfg) != SW_OK) {
		ppe_drv_warn("%p: IP global control configuration failed\n", p);
		return false;
	}

	if (fal_ip_route_mismatch_action_set(PPE_DRV_SWITCH_ID, FAL_MAC_RDT_TO_CPU) != SW_OK) {
		ppe_drv_warn("%p: IP route mismatch action configuration failed\n", p);
		return false;
	}

	return true;
}

/*
 * ppe_drv_fse_ops_free()
 *	Function to release fse_ops related memory
 *
 * Should be called under ppe lock
 */
void ppe_drv_fse_ops_free(struct kref *kref)
{
	struct ppe_drv *p = container_of(kref, struct ppe_drv, fse_ops_ref);
	struct ppe_drv_fse_ops *ops_internal;

	if (!p->fse_ops) {
		ppe_drv_warn("%p: No FSE ops registered\n", p);
		return;
	}

	ops_internal = p->fse_ops;
	p->fse_ops = NULL;
	kfree(ops_internal);
}

/*
 * ppe_drv_fse_ops_unregister()
 *	Un-register FSE rule add/delete callbacks. This function will be called from Wi-Fi driver
 */
void ppe_drv_fse_ops_unregister(void)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	spin_lock_bh(&p->lock);
	if (!p->fse_ops || !p->is_wifi_fse_up) {
		spin_unlock_bh(&p->lock);
		return;
	}

	p->is_wifi_fse_up = false;
	kref_put(&p->fse_ops_ref, ppe_drv_fse_ops_free);
	spin_unlock_bh(&p->lock);
}
EXPORT_SYMBOL(ppe_drv_fse_ops_unregister);

/**
 * ppe_drv_fse_ops_register()
 *	Register FSE rule add/delete callbacks. This function will be called from Wi-Fi driver
 */
bool ppe_drv_fse_ops_register(struct ppe_drv_fse_ops *ops)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_fse_ops *ops_internal;

	if (!ops->create_fse_rule || !ops->destroy_fse_rule) {
		ppe_drv_warn("%p: Invalid FSE ops passed from Wi-FI driver\n", p);
		return false;
	}

	ops_internal = (struct ppe_drv_fse_ops *)kzalloc(sizeof(struct ppe_drv_fse_ops), GFP_ATOMIC);
	if (!ops_internal) {
		ppe_drv_warn("%p: FSE ops registration failed\n", p);
		return false;
	}

	spin_lock_bh(&p->lock);
	if (p->fse_ops) {
		ppe_drv_warn("%p: FSE ops registration already done\n", p);
		spin_unlock_bh(&p->lock);
		kfree(ops_internal);
		return false;
	}

	ops_internal->create_fse_rule = ops->create_fse_rule;
	ops_internal->destroy_fse_rule = ops->destroy_fse_rule;
	p->fse_ops = ops_internal;
	p->is_wifi_fse_up = true;
	kref_init(&p->fse_ops_ref);
	spin_unlock_bh(&p->lock);

	ppe_drv_trace("%p: FSE ops registration done successfully\n", p);
	return true;
}
EXPORT_SYMBOL(ppe_drv_fse_ops_register);

/*
 * ppe_drv_get_dentry()
 *	Get PPE driver debugfs dentry
 */
struct dentry *ppe_drv_get_dentry()
{
	struct ppe_drv *p = &ppe_drv_gbl;
	return p->dentry;
}
EXPORT_SYMBOL(ppe_drv_get_dentry);

static const struct of_device_id ppe_drv_dt_ids[] = {
	{ .compatible =  "qcom,nss-ppe" },
	{},
};
MODULE_DEVICE_TABLE(of, ppe_drv_dt_ids);

/*
 * ppe_drv_confgiure_ucast_prio_map_tbl
 *	Configure unicast priority map table for RFS/DS flows
 */
static bool ppe_drv_confgiure_ucast_prio_map_tbl(struct ppe_drv *p, uint8_t profile_id, uint8_t *prio_map)
{
	uint8_t pri_class;
	uint8_t int_pri;
	sw_error_t ret;

	/*
	 * Set the priority class value for every possible priority.
	 */
	for (int_pri = 0; int_pri < PPE_DRV_MAX_PRIORITY; int_pri++) {
		pri_class = prio_map[int_pri];

		/*
		 * Configure priority class for Profile 9 used by RFS and DS.
		 */
		ret = fal_ucast_priority_class_set(PPE_DRV_SWITCH_ID, profile_id, int_pri, pri_class);
		if (ret != SW_OK) {
			ppe_drv_warn("%p Failed to configure ucast priority class for profile_id %d, int_pri: %d with err: %d\n",
					p, profile_id, int_pri, ret);
			return false;
		}

		ppe_drv_info("profile_id: %d, int_priority: %d, pri_class: %d\n", profile_id, int_pri, pri_class);
	}

	return true;
}

/*
 * ppe_drv_probe()
 *	probe the PPE driver
 */
static int ppe_drv_probe(struct platform_device *pdev)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct device_node *np;
	fal_ppe_tbl_caps_t cap;

	np = of_node_get(pdev->dev.of_node);

	fal_ppe_capacity_get(PPE_DRV_SWITCH_ID, &cap);

	/*
	 * Fill number of table entries
	 */
	p->l3_if_num = cap.l3_if_caps;
	p->port_num = cap.port_caps;
	p->vsi_num = cap.vsi_caps;
	p->pub_ip_num = cap.pub_ip_caps;
	p->host_num = cap.host_caps;
	p->flow_num = cap.flow_caps;
	p->pppoe_session_max = cap.pppoe_session_caps;
	p->nexthop_num = cap.nexthop_caps;
	p->sc_num = cap.service_code_caps;
	p->iface_num = p->l3_if_num + p->port_num + p->pppoe_session_max;

	/*
	 * Initialize locks
	 * Initialize locks before its first usage in "ppe_drv_tun_global_init".
	 */
	spin_lock_init(&p->lock);
	spin_lock_init(&p->stats_lock);
	spin_lock_init(&p->notifier_lock);


	if (!ppe_drv_hash_init()) {
		return -1;
	}

	if (!ppe_drv_l3_route_ctrl_init(p)) {
		return -1;
	}

#ifdef PPE_TUNNEL_ENABLE
	if (!ppe_drv_tun_global_init(p)) {
		ppe_drv_warn("%p: failed to do global config init for tunnels", p);
		return -1;
	}
#endif

	p->pub_ip = ppe_drv_pub_ip_entries_alloc();
	if (!p->pub_ip) {
		ppe_drv_warn("%p: failed to allocate pub_ip entries", p);
		goto fail;
	}

	p->l3_if = ppe_drv_l3_if_entries_alloc();
	if (!p->l3_if) {
		ppe_drv_warn("%p: failed to allocate l3_if entries", p);
		goto fail;
	}

	p->vsi = ppe_drv_vsi_entries_alloc();
	if (!p->vsi) {
		ppe_drv_warn("%p: failed to allocate vsi entries", p);
		goto fail;
	}

	p->pppoe = ppe_drv_pppoe_entries_alloc();
	if (!p->pppoe) {
		ppe_drv_warn("%p: failed to allocate PPPoe entries", p);
		goto fail;
	}

	p->port = ppe_drv_port_entries_alloc();
	if (!p->port) {
		ppe_drv_warn("%p: failed to allocate Port entries", p);
		goto fail;
	}

	p->sc = ppe_drv_sc_entries_alloc();
	if (!p->sc) {
		ppe_drv_warn("%p: failed to allocate service code entries", p);
		goto fail;
	}

	p->iface = ppe_drv_iface_entries_alloc();
	if (!p->iface) {
		ppe_drv_warn("%p: failed to allocate iface entries", p);
		goto fail;
	}

	p->nexthop = ppe_drv_nexthop_entries_alloc();
	if (!p->nexthop) {
		ppe_drv_warn("%p: failed to allocate nexthop entries", p);
		goto fail;
	}

	p->host = ppe_drv_host_entries_alloc();
	if (!p->host) {
		ppe_drv_warn("%p: failed to allocate host entries", p);
		goto fail;
	}

	p->flow = ppe_drv_flow_entries_alloc();
	if (!p->flow) {
		ppe_drv_warn("%p: failed to allocate flow entries", p);
		goto fail;
	}

	p->cc = ppe_drv_cc_entries_alloc();
	if (!p->cc) {
		ppe_drv_warn("%p: failed to allocate cpu-code entries", p);
		goto fail;
	}

	ppe_drv_exception_init();

	if (!ppe_drv_phy_port_base_queue_init(p)) {
		ppe_drv_warn("%p: failed to initialize physical port base queue\n", p);
		goto fail;
	}

	if (!ppe_drv_confgiure_ucast_prio_map_tbl(p, PPE_DRV_REDIR_PROFILE_ID, ppe_drv_redir_prio_map)) {
		ppe_drv_warn("%p: failed to configure ucast priority class setting\n", p);
		goto fail;
	}

	/*
	 *
	 * TODO: Check if we need to modify timer flag - specially irqsafe
	 * or pinned to a specific core.
	 */
	p->hw_flow_stats_ticks = msecs_to_jiffies(PPE_DRV_HW_FLOW_STATS_MS);
	timer_setup(&p->hw_flow_stats_timer, ppe_drv_hw_stats_sync, 0);

	/* Initialize list */
	INIT_LIST_HEAD(&p->conn_v4);
	INIT_LIST_HEAD(&p->conn_v6);
	INIT_LIST_HEAD(&p->conn_tun_v4);
	INIT_LIST_HEAD(&p->conn_tun_v6);
	INIT_LIST_HEAD(&p->notifier_list_head);

	p->toggled_v4 = false;
	p->toggled_v6 = false;
	p->tun_toggled_v4 = false;
	p->tun_toggled_v6 = false;
	p->disable_port_mtu_check = true;
	p->fse_ops = NULL;
	p->fse_enable = false;
        p->is_wifi_fse_up = false;

	p->tun_gbl.tun_l2tp.l2tp_dport = PPE_DRV_L2TP_DEFAULT_UDP_PORT;
	p->tun_gbl.tun_l2tp.l2tp_sport = PPE_DRV_L2TP_DEFAULT_UDP_PORT;
	p->tun_gbl.tun_l2tp.l2tp_encap_rule = NULL;

#ifdef PPE_TUNNEL_ENABLE
	/*
	 * Allocate tunnel specific entries
	 */

	p->ptun_ec = ppe_drv_tun_encap_entries_alloc(p);
	if (!p->ptun_ec) {
		ppe_drv_warn("%p: failed to allocate tunnel encap entries", p);
		goto fail;
	}

	p->ptun_dc = ppe_drv_tun_decap_entries_alloc(p);
	if (!p->ptun_dc) {
		ppe_drv_warn("%p: failed to allocate tunnel decap entries", p);
		goto fail;
	}

	p->ptun_l3_if = ppe_drv_tun_l3_if_entries_alloc(p);
	if (!p->ptun_l3_if) {
		ppe_drv_warn("%p: failed to allocate TL L3 interface entries", p);
		goto fail;
	}

	p->decap_map_entries = ppe_drv_tun_decap_map_entries_alloc(p);
	if (!p->decap_map_entries) {
		ppe_drv_warn("%p: failed to allocate TL MAP LPM table interface entries", p);
		goto fail;
	}

	p->encap_xlate_rules = ppe_drv_tun_encap_xlate_rule_entries_alloc(p);
	if (!p->encap_xlate_rules) {
		ppe_drv_warn("%p: failed to allocate EG edit rule entries", p);
		goto fail;
	}

	p->decap_xlate_rules = ppe_drv_tun_decap_xlate_rule_entries_alloc(p);
	if (!p->decap_xlate_rules) {
		ppe_drv_warn("%p: failed to allocate TL MAP LPM action interface entries", p);
		goto fail;
	}
#endif

	p->acl = ppe_drv_acl_entries_alloc();
	if (!p->acl) {
		ppe_drv_warn("%p: failed to allocate ACL entries", p);
		goto fail;
	}

	p->pol_ctx = ppe_drv_policer_entries_alloc();
	if (!p->pol_ctx) {
		ppe_drv_warn("%p: failed to allocate global policer context", p);
		goto fail;
	}

#ifdef PPE_TUNNEL_ENABLE
	p->pgm = ppe_drv_tun_prgm_prsr_alloc(p);
	if (!p->pgm) {
		ppe_drv_warn("%p: failed to allocate program parser entries", p);
		goto fail;
	}

	p->pgm_udf = ppe_drv_tun_udf_alloc(p);
	if (!p->pgm_udf) {
		ppe_drv_warn("%p: failed to allocate tunnel udf entries", p);
		goto fail;
	}
#endif

	/*
	 * Take a reference
	 */
	kref_init(&p->ref);

	/*
	 * Start sync timer for hardware stats collection.
	 */
	mod_timer(&p->hw_flow_stats_timer, jiffies + p->hw_flow_stats_ticks);

	/*
	 * Non availability of debugfs directory is not a catastrophy
	 * We can still go ahead with other initialization
	 */
	ppe_drv_stats_debugfs_init();

	ppe_drv_flow_dump_init(p->dentry);
	ppe_drv_if_map_init(p->dentry);

	return of_platform_populate(np, NULL, NULL, &pdev->dev);

fail:

#ifdef PPE_TUNNEL_ENABLE
	if (p->decap_map_entries) {
		ppe_drv_tun_decap_entries_free(p->decap_map_entries);
		p->decap_map_entries = NULL;
	}

	if (p->encap_xlate_rules) {
		ppe_drv_tun_encap_xlate_rule_entries_free(p->encap_xlate_rules);
		p->encap_xlate_rules = NULL;
	}

	if (p->decap_xlate_rules) {
		ppe_drv_tun_decap_xlate_rule_entries_free(p->decap_xlate_rules);
		p->decap_xlate_rules = NULL;
	}

	if (p->ptun_ec) {
		ppe_drv_tun_encap_entries_free(p->ptun_ec);
		p->ptun_ec = NULL;
	}

	if (p->ptun_dc) {
		ppe_drv_tun_decap_entries_free(p->ptun_dc);
		p->ptun_dc = NULL;
	}

	if (p->ptun_l3_if) {
		ppe_drv_tun_l3_if_entries_free(p->ptun_l3_if);
		p->ptun_l3_if = NULL;
	}
#endif

	if (p->pol_ctx) {
		ppe_drv_policer_entries_free(p->pol_ctx);
		p->pol_ctx = NULL;
	}

	if (p->acl) {
		ppe_drv_acl_entries_free(p->acl);
		p->acl = NULL;
	}

	if (p->pub_ip) {
		ppe_drv_pub_ip_entries_free(p->pub_ip);
		p->pub_ip = NULL;
	}

	if (p->l3_if) {
		ppe_drv_l3_if_entries_free(p->l3_if);
		p->l3_if = NULL;
	}

	if (p->vsi) {
		ppe_drv_vsi_entries_free(p->vsi);
		p->vsi = NULL;
	}

	if (p->pppoe) {
		ppe_drv_pppoe_entries_free(p->pppoe);
		p->pppoe = NULL;
	}

	if (p->port) {
		ppe_drv_port_entries_free(p->port);
		p->port = NULL;
	}

	if (p->sc) {
		ppe_drv_sc_entries_free(p->sc);
		p->sc = NULL;
	}

	if (p->iface) {
		ppe_drv_iface_entries_free(p->iface);
		p->iface = NULL;
	}

	if (p->nexthop) {
		ppe_drv_nexthop_entries_free(p->nexthop);
		p->nexthop = NULL;
	}

	if (p->host) {
		ppe_drv_host_entries_free(p->host);
		p->host = NULL;
	}

	if (p->flow) {
		ppe_drv_flow_entries_free(p->flow);
		p->flow = NULL;
	}

	if (p->cc) {
		ppe_drv_cc_entries_free(p->cc);
		p->cc = NULL;
	}

#ifdef PPE_TUNNEL_ENABLE
	if (p->pgm) {
		ppe_drv_tun_prgm_prsr_free(p->pgm);
		p->pgm = NULL;
	}

	if (p->pgm_udf) {
		ppe_drv_tun_udf_free(p->pgm_udf);
		p->pgm_udf = NULL;
	}

	if (p->ecap_hdr_ctrl) {
		ppe_drv_tun_encap_hdr_ctrl_free(p->ecap_hdr_ctrl);
		p->ecap_hdr_ctrl = NULL;
	}
#endif

	ppe_drv_flow_dump_exit();
	ppe_drv_if_map_exit();

	return -1;
}

/*
 * ppe_drv_remove()
 *	remove the ppe driver
 */
static int ppe_drv_remove(struct platform_device *pdev)
{
	struct ppe_drv *p = platform_get_drvdata(pdev);

	ppe_drv_stats_debugfs_exit();

	if (p->pub_ip) {
		ppe_drv_pub_ip_entries_free(p->pub_ip);
		p->pub_ip = NULL;
	}

	if (p->l3_if) {
		ppe_drv_l3_if_entries_free(p->l3_if);
		p->l3_if = NULL;
	}

	if (p->vsi) {
		ppe_drv_vsi_entries_free(p->vsi);
		p->vsi = NULL;
	}

	if (p->pppoe) {
		ppe_drv_pppoe_entries_free(p->pppoe);
		p->pppoe = NULL;
	}

	if (p->port) {
		ppe_drv_port_entries_free(p->port);
		p->port = NULL;
	}

	if (p->sc) {
		ppe_drv_sc_entries_free(p->sc);
		p->sc = NULL;
	}

	if (p->iface) {
		ppe_drv_iface_entries_free(p->iface);
		p->iface = NULL;
	}

	if (p->nexthop) {
		ppe_drv_nexthop_entries_free(p->nexthop);
		p->nexthop = NULL;
	}

	if (p->host) {
		ppe_drv_host_entries_free(p->host);
		p->host = NULL;
	}

	if (p->flow) {
		ppe_drv_flow_entries_free(p->flow);
		p->flow = NULL;
	}

	if (p->cc) {
		ppe_drv_cc_entries_free(p->cc);
		p->cc = NULL;
	}

#ifdef PPE_TUNNEL_ENABLE
	if (p->ptun_ec) {
		ppe_drv_tun_encap_entries_free(p->ptun_ec);
		p->ptun_ec = NULL;
	}

	if (p->ptun_dc) {
		ppe_drv_tun_decap_entries_free(p->ptun_dc);
		p->ptun_dc = NULL;
	}

	if (p->ptun_l3_if) {
		ppe_drv_tun_l3_if_entries_free(p->ptun_l3_if);
		p->ptun_l3_if = NULL;
	}

	if (p->decap_map_entries) {
		ppe_drv_tun_decap_entries_free(p->decap_map_entries);
		p->decap_map_entries = NULL;
	}

	if (p->encap_xlate_rules) {
		ppe_drv_tun_encap_xlate_rule_entries_free(p->encap_xlate_rules);
		p->encap_xlate_rules = NULL;
	}

	if (p->decap_xlate_rules) {
		ppe_drv_tun_decap_xlate_rule_entries_free(p->decap_xlate_rules);
		p->decap_xlate_rules = NULL;
	}
#endif

	if (p->acl) {
		ppe_drv_acl_entries_free(p->acl);
		p->acl = NULL;
	}

#ifdef PPE_TUNNEL_ENABLE
	ppe_drv_tun_vxlan_deconfigure(p);
#endif

	if (p->pol_ctx) {
		ppe_drv_policer_entries_free(p->pol_ctx);
		p->pol_ctx = NULL;
	}

	if (p->fse_ops) {
		ppe_drv_warn("FSE ops still registered while ppe module getting removed\n");
	}

#ifdef PPE_TUNNEL_ENABLE
	if (p->pgm) {
		ppe_drv_tun_prgm_prsr_free(p->pgm);
		p->pgm = NULL;
	}

	if (p->pgm_udf) {
		ppe_drv_tun_udf_free(p->pgm_udf);
		p->pgm_udf = NULL;
	}

	if (p->ecap_hdr_ctrl) {
		ppe_drv_tun_encap_hdr_ctrl_free(p->ecap_hdr_ctrl);
		p->ecap_hdr_ctrl = NULL;
	}
#endif

	ppe_drv_flow_dump_exit();
	ppe_drv_if_map_exit();

	return 0;
}

/*
 * ppe_drv_notify_change_upper_handler()
 *	Call registered notifiers with ppe drv
 *	To-do: Introduce new file for ppe-drv notifier functions
 */
static void ppe_drv_notify_change_upper_handler(void *ptr, int event)
{
	struct ppe_drv_notifier_ops *iterator;
	struct ppe_drv *p = &ppe_drv_gbl;
	struct netdev_notifier_info *info = (struct netdev_notifier_info *)ptr;
	struct netdev_notifier_changeupper_info *cu_info = (struct netdev_notifier_changeupper_info *)info;

	spin_lock_bh(&p->notifier_lock);
	if (cu_info->linking) {
		ppe_drv_trace("%px: Linking ppe drv event: %d\n", ptr, event);
		list_for_each_entry(iterator, &p->notifier_list_head, entry) {
			if (iterator->notifier_call) {
				(iterator->notifier_call)(iterator, event, info);
			}
		}
		spin_unlock_bh(&p->notifier_lock);
		return;
	}

	/*
	 * Call notifier in reverse order during unlinking
	 * to clear the database in the correct order
	 */
	ppe_drv_trace("%px: Unlinking ppe drv event: %d\n", ptr, event);
	list_for_each_entry_reverse(iterator, &p->notifier_list_head, entry) {
		if (iterator->notifier_call) {
			(iterator->notifier_call)(iterator, event, ptr);
		}
	}
	spin_unlock_bh(&p->notifier_lock);
}

/*
 * ppe_drv_handle_netdev_event()
 *	Handle events received from network stack
 */
static int ppe_drv_handle_netdev_event(struct notifier_block *nb,
		unsigned long event, void *ptr)
{
	switch (event) {
	case NETDEV_CHANGEUPPER:
		ppe_drv_notify_change_upper_handler(ptr, PPE_DRV_EVENT_CHANGEUPPER);
		break;

	default:
		 ppe_drv_trace("Event not supported through ppe-drv: %ld\n", event);
	}

	return NOTIFY_DONE;
}

/* register netdev notifier callback */
static struct notifier_block nss_ppe_netdevice __read_mostly = {
	.notifier_call = ppe_drv_handle_netdev_event,
};

/*
 * ppe_drv_platform
 *	platform device instance
 */
static struct platform_driver ppe_drv_platform = {
	.probe		= ppe_drv_probe,
	.remove		= ppe_drv_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "nss-ppe",
		.of_match_table = of_match_ptr(ppe_drv_dt_ids),
	},
};

/*
 * ppe_drv_notifier_ops_register()
 *	Register notifier operations
 */
void ppe_drv_notifier_ops_register(struct ppe_drv_notifier_ops *notifier_ops)
{
	struct list_head *ptr;
	struct ppe_drv_notifier_ops *entry_pnb;
	struct ppe_drv *p = &ppe_drv_gbl;

	ppe_drv_trace("%px: Add notifier callback with priority: %d\n", notifier_ops, notifier_ops->priority);

	spin_lock_bh(&p->notifier_lock);
	list_for_each(ptr, &p->notifier_list_head) {
		entry_pnb = list_entry(ptr, struct ppe_drv_notifier_ops, entry);
		if (entry_pnb->priority < notifier_ops->priority) {
			list_add(&notifier_ops->entry, &entry_pnb->entry);
			spin_unlock_bh(&p->notifier_lock);
			return;
		}
	}

	list_add(&notifier_ops->entry, &p->notifier_list_head);
	spin_unlock_bh(&p->notifier_lock);
}
EXPORT_SYMBOL(ppe_drv_notifier_ops_register);

/*
 * ppe_drv_notifier_ops_unregister()
 *	Unregister notifier operations
 */
void ppe_drv_notifier_ops_unregister(struct ppe_drv_notifier_ops *notifier_ops)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	spin_lock_bh(&p->notifier_lock);
	list_del(&notifier_ops->entry);
	spin_unlock_bh(&p->notifier_lock);
}
EXPORT_SYMBOL(ppe_drv_notifier_ops_unregister);

/*
 * ppe_drv_set_ppe_if_bm_to_port()
 *	API to set the OFFLOAD flag in the port from the interface bitmask
 */
static void ppe_drv_set_ppe_if_bm_to_port(void)
{
	struct ppe_drv_port *drv_port = NULL;
	struct ppe_drv *p = &ppe_drv_gbl;
	unsigned int if_bit_count = __builtin_popcount(PPE_DRV_PORT_OFFLOAD_MAX_VAL);
	uint32_t i;

	spin_lock_bh(&p->lock);
	for (i = 1; i <= if_bit_count; i++) {
		drv_port = ppe_drv_port_from_port_num(i);
		if (!drv_port) {
			ppe_drv_warn("failed in getting drv port from %d port\n", i);
			continue;
		}

		if (if_bm_to_offload & (1 << (i - 1))) {
			drv_port->flags |= PPE_DRV_PORT_FLAG_OFFLOAD_ENABLED;
		} else {
			drv_port->flags &= ~PPE_DRV_PORT_FLAG_OFFLOAD_ENABLED;
		}
	}
	spin_unlock_bh(&p->lock);
}

/*
 * ppe_drv_if_bm_to_offload_handler()
 *	API to configure the interface bitmask where the offload is enabled
 */
static int ppe_drv_if_bm_to_offload_handler(struct ctl_table *table, int write,
		void __user *buffer, size_t *lenp, loff_t *ppos)
{
	int ret;
	uint32_t current_value;

	/*
	 * Take the current value
	 */
	current_value = if_bm_to_offload;

	ret = proc_dointvec(table, write, buffer, lenp, ppos);

	if (!write) {
		return ret;
	}

	if (if_bm_to_offload > PPE_DRV_PORT_OFFLOAD_MAX_VAL) {
		ppe_drv_warn("Incorrect value of offload bitmask. Setting it to"
				" default value. Value: %0x, if_bm_to_offload: %0x\n",
				if_bm_to_offload, PPE_DRV_PORT_OFFLOAD_MAX_VAL);
		if_bm_to_offload = current_value;
	}

	ppe_drv_set_ppe_if_bm_to_port();

	ppe_drv_warn("PPE DRV interface bitmask to offload is %0x\n", if_bm_to_offload);
	return ret;
}

/*
 * ppe_drv_disable_port_mtu_check_handler()
 * 	Set disable port mtu config
 */
static int ppe_drv_disable_port_mtu_check_handler(struct ctl_table *table,
						int write, void __user *buffer,
						size_t *lenp, loff_t *ppos)
{
	int ret;
	struct ppe_drv *p = &ppe_drv_gbl;

	ret = proc_dointvec(table, write, buffer, lenp, ppos);

	if (!write) {
		return ret;
	}

	p->disable_port_mtu_check = disable_port_mtu_check;

	ppe_drv_info("Updating disable_port_mtu_check flag as %d\n", p->disable_port_mtu_check);
	return ret;
}

/*
 * ppe_drv_eth2eth_offload_if_bitmap_handler()
 * 	Set eth to eth offload with if bitmap config
 */
static int ppe_drv_eth2eth_offload_if_bitmap_handler(struct ctl_table *table,
						int write, void __user *buffer,
						size_t *lenp, loff_t *ppos)
{
	int ret;
	struct ppe_drv *p = &ppe_drv_gbl;

	ret = proc_dointvec(table, write, buffer, lenp, ppos);

	if (!write) {
		return ret;
	}

	p->eth2eth_offload_if_bitmap = eth2eth_offload_if_bitmap;

	ppe_drv_info("Updating eth2eth_offload_if_bitmap flag as %d\n", p->eth2eth_offload_if_bitmap);
	return ret;
}

/*
 * ppe_drv_static_dbg_level_handler()
 *	Set static debug level for ppe-driver.
 */
static int ppe_drv_static_dbg_level_handler(struct ctl_table *table,
					int write, void __user *buffer,
					size_t *lenp, loff_t *ppos)
{
	int ret;
	char *level_str;
	enum ppe_drv_static_dbg_level dbg_level;

	/*
	 * Find the string, return an error if not found
	 */
	ret = proc_dostring(table, write, buffer, lenp, ppos);
	if (ret || !write) {
		return ret;
	}

	level_str = static_dbg_level_str;
	printk("dbg_level: %s", level_str);

	if (!strcmp(level_str, "warn")) {
		dbg_level = PPE_DRV_STATIC_DBG_LEVEL_WARN;
	} else if (!strcmp(level_str, "info")) {
		dbg_level = PPE_DRV_STATIC_DBG_LEVEL_INFO;
	} else if (!strcmp(level_str, "trace")) {
		dbg_level = PPE_DRV_STATIC_DBG_LEVEL_TRACE;
	} else if (!strcmp(level_str, "none")) {
		dbg_level = PPE_DRV_STATIC_DBG_LEVEL_NONE;
	} else {
		printk("Usage: echo '[warn|info|trace|none]' > /proc/sys/ppe/ppe_drv/static_dbg_level\n");
		return -EINVAL;
	}

	if (dbg_level >= PPE_DRV_DEBUG_LEVEL) {
		printk("debug level: %d not compiled in: %d\n", dbg_level, PPE_DRV_DEBUG_LEVEL);
		return -EINVAL;
	}

	static_dbg_level = dbg_level;
	return ret;
}

/*
 * ppe_drv_sub
 *	PPE DRV sub directory
 */
static struct ctl_table ppe_drv_sub[] = {
	{
		.procname	=	"if_bm_to_offload",
		.data		=	&if_bm_to_offload,
		.maxlen		=	sizeof(int),
		.mode		=	0644,
		.proc_handler	=	ppe_drv_if_bm_to_offload_handler
	},
	{
		.procname	=	"static_dbg_level",
		.data		=	&static_dbg_level_str,
		.maxlen		=	sizeof(char) * PPE_DRV_STATIC_DBG_LEVEL_STR_LEN,
		.mode		=	0644,
		.proc_handler	=	ppe_drv_static_dbg_level_handler
	},
	{
		.procname       =       "disable_port_mtu_check",
		.data           =       &disable_port_mtu_check,
		.maxlen         =       sizeof(int),
		.mode           =       0644,
		.proc_handler   =       ppe_drv_disable_port_mtu_check_handler
	},
	{
		.procname       =       "eth2eth_offload_if_bitmap",
		.data           =       &eth2eth_offload_if_bitmap,
		.maxlen         =       sizeof(int),
		.mode           =       0644,
		.proc_handler   =       ppe_drv_eth2eth_offload_if_bitmap_handler
	},
	{}
};

/*
 * ppe_drv_mht_port_from_fdb()
 *	Get the port id corresponding to the destination
 *	mac address and vid
 */
int32_t ppe_drv_mht_port_from_fdb(uint8_t *dmac, uint16_t vid)
{
	fal_fdb_entry_t entry = {0};
	sw_error_t err = SW_OK;
	entry.fid = vid;
	entry.port.id = 0;
	entry.type = SW_ENTRY;

	memcpy(&entry.addr, dmac, ETH_ALEN);

	err = fal_fdb_entry_search(PPE_DRV_MHT_SWITCH_ID, &entry);
	if (err != SW_OK) {
		return -1;
	}

	if (entry.port.id > 0) {
		return ffs(entry.port.id) - 1;
	}

	return -1;
}
EXPORT_SYMBOL(ppe_drv_mht_port_from_fdb);

/*
 * ppe_drv_module_init()
 *	module init for ppe driver
 */
static int __init ppe_drv_module_init(void)
{
	int ret;

	if (!of_find_compatible_node(NULL, NULL, "qcom,nss-ppe")) {
		ppe_drv_info("PPE device tree node not found\n");
		return -EINVAL;
	}

	if (platform_driver_register(&ppe_drv_platform)) {
		ppe_drv_warn("unable to register the driver\n");
		return -EIO;
	}

	ret = register_netdevice_notifier(&nss_ppe_netdevice);
	if (ret) {
		ppe_drv_warn("Failed to register NETDEV notifier, error=%d\n", ret);
		platform_driver_unregister(&ppe_drv_platform);
		return -EINVAL;
	}

	/*
	 * Register sysctl framework for PPE DRV
	 */
	ppe_drv_gbl.ppe_drv_header = register_sysctl("ppe/ppe_drv", ppe_drv_sub);
	if (!ppe_drv_gbl.ppe_drv_header) {
		ppe_drv_warn("sysctl table configuration failed");
		unregister_netdevice_notifier(&nss_ppe_netdevice);
		platform_driver_unregister(&ppe_drv_platform);
		return -EINVAL;
	}

	return 0;
}
module_init(ppe_drv_module_init);

/*
 * ppe_drv_module_exit()
 *	module exit for ppe driver
 */
static void __exit ppe_drv_module_exit(void)
{
	unregister_sysctl_table(ppe_drv_gbl.ppe_drv_header);
	ppe_drv_gbl.ppe_drv_header = NULL;
	unregister_netdevice_notifier(&nss_ppe_netdevice);
	platform_driver_unregister(&ppe_drv_platform);
}
module_exit(ppe_drv_module_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS PPE driver");
