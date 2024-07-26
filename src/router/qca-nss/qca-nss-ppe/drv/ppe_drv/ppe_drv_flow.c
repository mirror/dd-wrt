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

#include <linux/in.h>
#include <net/ipv6.h>
#include <linux/netdevice.h>
#include <linux/if_vlan.h>
#include <fal/fal_flow.h>
#include <fal/fal_qos.h>
#include "ppe_drv.h"

#if (PPE_DRV_DEBUG_LEVEL == 3)
/*
 * ppe_drv_flow_dump()
 *	Read and dump the flow table entry.
 */
void ppe_drv_flow_dump(struct ppe_drv_flow *pf)
{
	fal_flow_entry_t flow_cfg = {0};
	sw_error_t err;

	flow_cfg.entry_id = pf->index;
	err = fal_flow_entry_get(PPE_DRV_SWITCH_ID, FAL_FLOW_OP_MODE_INDEX, &flow_cfg);
	if (err != SW_OK){
		ppe_drv_warn("%p: failed to get flow entry", pf);
		return;
	}

	ppe_drv_trace("%p: index: %d", pf, flow_cfg.entry_id);
	ppe_drv_trace("%p: entry_type: %d", pf, flow_cfg.entry_type);
	ppe_drv_trace("%p: protocol: %d", pf, flow_cfg.protocol);;
	ppe_drv_trace("%p: age: %d", pf, flow_cfg.age);
	ppe_drv_trace("%p: src_intf_valid: %d", pf, flow_cfg.src_intf_valid);
	ppe_drv_trace("%p: src_intf_index: %d", pf, flow_cfg.src_intf_index);
	ppe_drv_trace("%p: fwd_type: %d", pf, flow_cfg.fwd_type);
	ppe_drv_trace("%p: snat_nexthop: %d", pf, flow_cfg.snat_nexthop);
	ppe_drv_trace("%p: snat_srcport: %d", pf, flow_cfg.snat_srcport);
	ppe_drv_trace("%p: dnat_nexthop: %d", pf, flow_cfg.dnat_nexthop);
	ppe_drv_trace("%p: dnat_dstport: %d", pf, flow_cfg.dnat_dstport);
	ppe_drv_trace("%p: route_nexthop: %d", pf, flow_cfg.route_nexthop);
	ppe_drv_trace("%p: port_valid: %d", pf, flow_cfg.port_valid);
	ppe_drv_trace("%p: route_port: %d", pf, flow_cfg.route_port);
	ppe_drv_trace("%p: bridge_port: %d", pf, flow_cfg.bridge_port);
	ppe_drv_trace("%p: deacclr_en: %d", pf, flow_cfg.deacclr_en);
	ppe_drv_trace("%p: copy_tocpu_en: %d", pf, flow_cfg.copy_tocpu_en);
	ppe_drv_trace("%p: syn_toggle: %d", pf, flow_cfg.syn_toggle);
	ppe_drv_trace("%p: pri_profile: %d", pf, flow_cfg.pri_profile);
	ppe_drv_trace("%p: sevice_code: %d", pf, flow_cfg.sevice_code);
	ppe_drv_trace("%p: src_port: %d", pf, flow_cfg.src_port);
	ppe_drv_trace("%p: dst_port: %d", pf, flow_cfg.dst_port);
	ppe_drv_trace("%p: tree_id: %d", pf, flow_cfg.flow_qos.tree_id);
	ppe_drv_trace("%p: pkt_counter: %d", pf, flow_cfg.pkt_counter);
	ppe_drv_trace("%p: byte_counter: %llu", pf, flow_cfg.byte_counter);
	ppe_drv_trace("%p: pmtu_check_l3: %d", pf, flow_cfg.pmtu_check_l3);
	ppe_drv_trace("%p: pmtu: %d", pf, flow_cfg.pmtu);
	ppe_drv_trace("%p: vpn_id: %d", pf, flow_cfg.vpn_id);
	ppe_drv_trace("%p: vlan_fmt_valid: %d", pf, flow_cfg.vlan_fmt_valid);
	ppe_drv_trace("%p: svlan_fmt: %d", pf, flow_cfg.svlan_fmt);
	ppe_drv_trace("%p: cvlan_fmt: %d", pf, flow_cfg.cvlan_fmt);
	ppe_drv_trace("%p: wifi_qos_en: %d", pf, flow_cfg.flow_qos.wifi_qos_en);
	ppe_drv_trace("%p: wifi_qos: %d", pf, flow_cfg.flow_qos.wifi_qos);
	ppe_drv_trace("%p: invalid: %d", pf, flow_cfg.invalid);

	if (pf->type == PPE_DRV_IP_TYPE_V4) {
		ppe_drv_trace("%p: ipv4 address: %pI4", pf, &flow_cfg.flow_ip.ipv4);
	} else {
		ppe_drv_trace("%p: ipv6 address: %pI6", pf, &flow_cfg.flow_ip.ipv6.ul[0]);
	}
}
#else
/*
 * ppe_drv_flow_dump()
 *	Read and dump the flow table entry.
 */
void ppe_drv_flow_dump(struct ppe_drv_flow *pf)
{
}
#endif

/*
 * ppe_drv_flow_valid_set()
 *	Enable ppe flow entry.
 */
bool ppe_drv_flow_valid_set(struct ppe_drv_flow *pf, bool enable)
{
	sw_error_t err;

	err = fal_flow_entry_en_set(PPE_DRV_SWITCH_ID, pf->index, enable);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to set valid bit: %d for flow index: %d", pf, enable, pf->index);
		return false;
	}

	pf->flags |= PPE_DRV_FLOW_VALID;
	ppe_drv_trace("%p: set valid bit: %d for flow index: %d", pf, enable, pf->index);
	return true;
}

/*
 * ppe_drv_flow_stats_clear()
 *	Clears the flow statistics for the given flow index.
 */
void ppe_drv_flow_stats_clear(struct ppe_drv_flow *pf)
{
	sw_error_t err;

	err = fal_flow_counter_cleanup(PPE_DRV_SWITCH_ID, pf->index);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to clear stats for flow at index: %u err: %d",
				pf, pf->index, err);
	}

	ppe_drv_trace("%p: clear stats for flow index: %d", pf, pf->index);
}

/*
 * ppe_drv_flow_sawf_sc_stats_add()
 *	Update stats for given service class.
 */
void ppe_drv_flow_sawf_sc_stats_add(uint8_t service_class, uint32_t delta_pkts, uint32_t delta_bytes)
{
	struct ppe_drv_stats_sawf_sc *sawf_sc_stats = &ppe_drv_gbl.stats.sawf_sc_stats[service_class];

	atomic64_add(delta_pkts, &sawf_sc_stats->rx_packets);
	atomic64_add(delta_bytes, &sawf_sc_stats->rx_bytes);

	ppe_drv_trace("Stats Updated: service class %u : packets %llu : bytes %llu\n",
			service_class, atomic64_read(&sawf_sc_stats->rx_packets), atomic64_read(&sawf_sc_stats->rx_bytes));
}

/*
 * ppe_drv_flow_v6_stats_update()
 *	Updates flow instance's stats counter from PPE flow hit counter.
 */
void ppe_drv_flow_v6_stats_update(struct ppe_drv_v6_conn_flow *pcf)
{
	sw_error_t err;
	uint32_t delta_pkts;
	uint32_t delta_bytes;
	struct ppe_drv_v6_conn_flow *pcr;
	struct ppe_drv_flow *pf = pcf->pf;
	struct ppe_drv_flow_tree_id_data *tree_id_data = &(pcf->flow_metadata.tree_id_data);
	fal_entry_counter_t flow_cntrs = {0};
	struct ppe_drv_v6_conn *cn = pcf->conn;

	err = fal_flow_counter_get(PPE_DRV_SWITCH_ID, pf->index, &flow_cntrs);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to get stats for flow at index: %u", pf, pf->index);
		return;
	}

	/*
	 * PPE stats are not clear on read, so we need to calculate the delta
	 * between the latest counters and previously read counters.
	 */
	delta_pkts = (flow_cntrs.matched_pkts - pf->pkts + FAL_FLOW_PKT_CNT_MASK + 1) & FAL_FLOW_PKT_CNT_MASK;
	delta_bytes = (flow_cntrs.matched_bytes - pf->bytes + FAL_FLOW_BYTE_CNT_MASK + 1)
		                                              & FAL_FLOW_BYTE_CNT_MASK;

	/*
	 * Update ppe_conn_flow packet and byte counters
	 */
	ppe_drv_v6_conn_flow_rx_stats_add(pcf, delta_pkts, delta_bytes);
	if (ppe_drv_v6_conn_flags_check(cn, PPE_DRV_V6_CONN_FLAG_RETURN_VALID)) {
		pcr = (pcf == &cn->pcf) ? &cn->pcr : &cn->pcf;
		ppe_drv_v6_conn_flow_tx_stats_add(pcr, delta_pkts, delta_bytes);
	} else {
		/*
		 * Multicast flows have no counter cme. For this type of flows
		 * ECM expects tx count to be incremented in the same cme.
		 */
		ppe_drv_v6_conn_flow_tx_stats_add(pcf, delta_pkts, delta_bytes);
	}

	pf->pkts = flow_cntrs.matched_pkts;
	pf->bytes = flow_cntrs.matched_bytes;

	/*
	 * Update the stats only if tree_id is configured with SAWF.
	 */
	if (ppe_drv_tree_id_type_get(&pcf->flow_metadata) == PPE_DRV_TREE_ID_TYPE_SAWF) {
		ppe_drv_flow_sawf_sc_stats_add(tree_id_data->info.sawf_metadata.service_class, delta_pkts, delta_bytes);
	}
}

/*
 * ppe_drv_flow_v4_stats_update()
 *	Updates flow instance's stats counter from PPE flow hit counter.
 */
void ppe_drv_flow_v4_stats_update(struct ppe_drv_v4_conn_flow *pcf)
{
	sw_error_t err;
	uint32_t delta_pkts;
	uint32_t delta_bytes;
	struct ppe_drv_v4_conn_flow *pcr;
	struct ppe_drv_flow *pf = pcf->pf;
	struct ppe_drv_flow_tree_id_data *tree_id_data = &(pcf->flow_metadata.tree_id_data);
	fal_entry_counter_t flow_cntrs = {0};
	struct ppe_drv_v4_conn *cn = pcf->conn;
	struct ppe_drv_v6_conn_flow *mapt_pcf_v6, *mapt_pcr_v6;
	struct ppe_drv_v6_conn *mapt_cn_v6;

	err = fal_flow_counter_get(PPE_DRV_SWITCH_ID, pf->index, &flow_cntrs);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to get stats for flow at index: %u", pf, pf->index);
		return;
	}

	/*
	 * PPE stats are not clear on read, so we need to calculate the delta
	 * between the latest counters and previously read counters.
	 */
	delta_pkts = (flow_cntrs.matched_pkts - pf->pkts + FAL_FLOW_PKT_CNT_MASK + 1) & FAL_FLOW_PKT_CNT_MASK;
	delta_bytes = (flow_cntrs.matched_bytes - pf->bytes + FAL_FLOW_BYTE_CNT_MASK + 1)
		                                              & FAL_FLOW_BYTE_CNT_MASK;

	/*
	 * Update ppe_conn_flow packet and byte counters
	 */
	ppe_drv_v4_conn_flow_rx_stats_add(pcf, delta_pkts, delta_bytes);
	if (ppe_drv_v4_conn_flags_check(cn, PPE_DRV_V4_CONN_FLAG_RETURN_VALID)) {
		pcr = (pcf == &cn->pcf) ? &cn->pcr : &cn->pcf;
		ppe_drv_v4_conn_flow_tx_stats_add(pcr, delta_pkts, delta_bytes);
	} else {
		/*
		 * Multicast flows have no counter cme. For this type of flows
		 * ECM expects tx count to be incremented in the same cme.
		 */
		ppe_drv_v4_conn_flow_tx_stats_add(pcf, delta_pkts, delta_bytes);
	}

	if (pf->flags & PPE_DRV_FLOW_MAPT) {
		mapt_pcf_v6 = pf->mapt_info.mapt_v6;
		mapt_cn_v6 = mapt_pcf_v6->conn;

		delta_bytes += delta_pkts * pf->mapt_info.len_adjust;
		ppe_drv_v6_conn_flow_rx_stats_add(mapt_pcf_v6, delta_pkts, delta_bytes);

		if (ppe_drv_v6_conn_flags_check(mapt_pcf_v6->conn, PPE_DRV_V6_CONN_FLAG_RETURN_VALID)) {
			mapt_pcr_v6 = (mapt_pcf_v6 == &mapt_cn_v6->pcf) ? &mapt_cn_v6->pcr : &mapt_cn_v6->pcf;
			ppe_drv_v6_conn_flow_tx_stats_add(mapt_pcr_v6, delta_pkts, delta_bytes);
		}
	}

	pf->pkts = flow_cntrs.matched_pkts;
	pf->bytes = flow_cntrs.matched_bytes;

	/*
	 * Update the stats only if tree_id has SAWF metadata.
	 */
	if (ppe_drv_tree_id_type_get(&pcf->flow_metadata) == PPE_DRV_TREE_ID_TYPE_SAWF) {
		ppe_drv_flow_sawf_sc_stats_add(tree_id_data->info.sawf_metadata.service_class, delta_pkts, delta_bytes);
	}
}

/*
 * ppe_drv_flow_v6_qos_set()
 *	Set QOS mapping for this flow.
 */
bool ppe_drv_flow_v6_qos_set(struct ppe_drv_v6_conn_flow *pcf, struct ppe_drv_flow *flow)
{
	sw_error_t err;
	fal_qos_cosmap_t qos_cfg = {0};

	/*
	 * Check if flow needs qos update, set priority profile in QoS config
	 */
	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_QOS_VALID)) {
		qos_cfg.internal_pri = ppe_drv_v6_conn_flow_int_pri_get(pcf);
		qos_cfg.pri_en = A_TRUE;
	}

	/*
	 * Check if flow needs DSCP marking, set DSCP fields in QoS config
	 */
	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_DSCP_MARKING)) {
		qos_cfg.internal_dscp = ppe_drv_v6_conn_flow_egress_dscp_get(pcf) << PPE_DRV_DSCP_SHIFT;
		qos_cfg.dscp_mask = PPE_DRV_DSCP_MASK;
		qos_cfg.dscp_en = A_TRUE;
	}

	/*
	 * Check if flow needs PCP marking, set PCP fields in QoS config
	 */
	if ((ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_VLAN_PRI_MARKING))
			&& (ppe_drv_v6_conn_flow_egress_vlan_cnt_get(pcf) == 1)) {
		/*
		 * Note: PPE does not have flow based support for double vlan tagging.
		 * We do not need to mask unsigned integer.
		 */
		qos_cfg.internal_pcp = ppe_drv_v6_conn_flow_egress_vlan_get(pcf, 0)->tci >> PPE_DRV_VLAN_PRIORITY_SHIFT;;
		qos_cfg.pcp_en = A_TRUE;
	}

	err = fal_qos_cosmap_flow_set(PPE_DRV_SWITCH_ID, 0, flow->index, &qos_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p qos(DSCP/PCP) mapping configuration failed for flow: %p", pcf, flow);
		return false;
	}

	ppe_drv_trace("%p qos(DSCP/PCP) mapping configuration done for flow: %p", pcf, flow);
	return true;
}

/*
 * ppe_drv_flow_v6_qos_clear()
 *	Return service code required for this flow.
 */
bool ppe_drv_flow_v6_qos_clear(struct ppe_drv_flow *pf)
{
	sw_error_t err;
	fal_qos_cosmap_t qos_cfg = {0};

	err = fal_qos_cosmap_flow_set(PPE_DRV_SWITCH_ID, 0, pf->index, &qos_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p qos mapping configuration failed for flow", pf);
		return false;
	}

	ppe_drv_trace("%p qos mapping configuration done for flow", pf);
	return true;
}

/*
 * ppe_drv_flow_v6_tree_id_get()
 *	Find the tree ID associated with a flow
 */
static bool ppe_drv_flow_v6_tree_id_get(struct ppe_drv_v6_conn_flow *pcf, uint32_t *tree_id)
{
	struct ppe_drv_flow_tree_id_data *tree_id_data = &(pcf->flow_metadata.tree_id_data);

	switch (tree_id_data->type) {
	case PPE_DRV_TREE_ID_TYPE_NONE:
		*tree_id = tree_id_data->info.value;
		return true;

	case PPE_DRV_TREE_ID_TYPE_SAWF:
		PPE_DRV_TREE_ID_TYPE_SET(tree_id, tree_id_data->type);
		PPE_DRV_TREE_ID_SERVICE_CLASS_SET(tree_id, tree_id_data->info.sawf_metadata.service_class);
		PPE_DRV_TREE_ID_PEER_ID_SET(tree_id, tree_id_data->info.sawf_metadata.peer_id);
		return true;

	case PPE_DRV_TREE_ID_TYPE_WIFI_TID:
	case PPE_DRV_TREE_ID_TYPE_SCS:
		PPE_DRV_TREE_ID_TYPE_SET(tree_id, tree_id_data->type);
		return true;

	case PPE_DRV_TREE_ID_TYPE_MLO_ASSIST:
		PPE_DRV_TREE_ID_TYPE_SET(tree_id, tree_id_data->type);
		PPE_DRV_TREE_ID_MLO_MARK_SET(tree_id, tree_id_data->info.value);
		return true;

	default:
		ppe_drv_warn("Invalid tree_id_type : (%u)", tree_id_data->type);
		return false;
	}
}

/*
 * ppe_drv_flow_v6_vpn_id_get()
 *	Find the tree ID associated with a flow
 */
static bool ppe_drv_flow_v6_vpn_id_get(struct ppe_drv_v6_conn_flow *pcf, uint32_t *vpn_id)
{
	/*
	 * TODO: to be added later.
	 */
	*vpn_id = 0;
	return true;
}

/*
 * ppe_drv_flow_ds_wifi_qos_set()
 *	Sets the WiFi QoS for DS mode
 */
static void ppe_drv_flow_ds_wifi_qos_set(uint32_t *wifi_qos, uint32_t *msduq_value, bool flow_override_mode)
{
	bool flow_override;
	uint8_t tid;

	/*
	 * In case of DS mode,
	 *
	 * For flow override mode, wifi qos is configured as below:
	 * --------------------------------------------------------------------------------------
	 * |	Who Classify (2 bits)	|	TID (3 bits)	|	Flow override (1 bit)	|
	 * --------------------------------------------------------------------------------------
	 *
	 * OR
	 *
	 * fill wifi_qos[7]=1 to support hlos_tid Override configuration interpretation
	 * ---------------------------------------------------------------------------------------
         * |  HLOS_TID override mode(1 bit)  |   (3 bits)   |       TID (3 bits)    |  (1 bit)   |
         * ---------------------------------------------------------------------------------------
	 */

	if (flow_override_mode) {
		/*
		 * In sawf, tid value is mapped from msduq
		 */
		tid = *msduq_value & PPE_DRV_FLOW_TID_MASK;
		flow_override = *msduq_value & PPE_DRV_FLOW_FO_MASK;
		*wifi_qos = (*msduq_value & PPE_DRV_FLOW_WC_MASK) | (tid << PPE_DRV_FLOW_TID_SHIFT) | flow_override;
		return;
	}

	/*
	 * For scs, msduq is passed as tid.
	 */
	*wifi_qos = 0;
	tid = *msduq_value;
	*wifi_qos = PPE_DRV_FLOW_DS_HLOS_TID_OVERRIDE_ENABLE | (tid << PPE_DRV_FLOW_TID_SHIFT);
}

/*
 * ppe_drv_flow_override_mode_get()
 * 	Return false for hlos override mode.
 */
static bool ppe_drv_flow_override_mode_get(uint32_t *msduq_value)
{
	if (*msduq_value <= PPE_DRV_FLOW_HLOS_OVERRIDE_MSDUQ_MAX)
		return false;

	return true;
}

/*
 * ppe_drv_flow_v6_wifi_qos_get()
 *	Find the WIFI QOS associated with a flow
 */
static bool ppe_drv_flow_v6_wifi_qos_get(struct ppe_drv_v6_conn_flow *pcf, uint32_t *wifi_qos, bool *wifi_qos_en)
{
	bool flow_override_mode = true;

	/*
	 * If SAWF metadata is valid, set 6 bit MSDUQ in wifi_qos field (bits 0-5).
	 */
	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_METADATA_TYPE_WIFI_INFO)) {
		/*
		 * MSDUQ representation in wifi_qos field is:
		 * --------------------------------------------------------------------------------------
		 * |	Who Classify (2 bits)	|	Flow override (1 bit)	|	TID (3 bits)	|
		 * --------------------------------------------------------------------------------------
		 */
		*wifi_qos = pcf->flow_metadata.wifi_qos;
		*wifi_qos_en = true;

		/*
		 * In case of DS mode, rearrange the MSDUQ representation in wifi qos field to be aligned with TCL descriptor.
		 */
		if (!ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_VP_VALID) &&
				(pcf->tx_port->user_type == PPE_DRV_PORT_USER_TYPE_DS)) {
			flow_override_mode = ppe_drv_flow_override_mode_get(&pcf->flow_metadata.wifi_qos);

			/*
			 * Disabling WIFI_QOS flag for hlos tid mode
			 */
			if (!flow_override_mode)
				*wifi_qos_en = false;

			ppe_drv_flow_ds_wifi_qos_set(wifi_qos, &pcf->flow_metadata.wifi_qos, flow_override_mode);
			ppe_drv_trace("WiFi_QoS configured in DS descriptor is: 0x%x\n", *wifi_qos);
		}

		ppe_drv_trace("For User type: %u, WiFi_QoS initially: 0x%x and WiFi_QoS configured: 0x%x", pcf->tx_port->user_type, pcf->flow_metadata.wifi_qos, *wifi_qos);
	}

	return true;
}

#ifdef NSS_PPE_IPQ53XX
/*
 * ppe_drv_flow_v6_policer_get()
 *	Find the Policer get associated with a flow
 */
static bool ppe_drv_flow_v6_policer_get(struct ppe_drv_v6_conn_flow *pcf, uint32_t *policer_index, a_bool_t *policer_valid)
{
	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_POLICER_VALID)) {
		*policer_index = pcf->policer_hw_id;
		*policer_valid = A_TRUE;
	}

	return true;
}
#endif

/*
 * ppe_drv_flow_v6_service_code_get()
 *	Return service code required for this flow.
 */
bool ppe_drv_flow_v6_service_code_get(struct ppe_drv_v6_conn_flow *pcf, struct ppe_drv_port *pp, uint8_t *scp)
{
	struct ppe_drv_port *port_rx = pcf->rx_port;
	ppe_drv_sc_t service_code = *scp;
	ppe_drv_sc_t sc = PPE_DRV_SC_NONE;
	int next_core;

	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_PRIORITY_PPE_ASSIST)) {
		/*
		 * Service code to set priority for PPE assisted flows.
		 * No other service code is supported if priority assist is active.
		 */
		if (!ppe_drv_sc_check_and_set(&service_code, PPE_DRV_SC_NOEDIT_PRIORITY_SET)) {
			ppe_drv_warn("%p: flow requires multiple service code, existing:%u new:%u",
						pcf, service_code, PPE_DRV_SC_NOEDIT_PRIORITY_SET);
			return false;
		}

		*scp = service_code;
		return true;
	}

	/*
	 * Service code to enable PPE to bypass packet header editing and forward them unmodified.
	 */
	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_NO_EDIT_RULE)) {
		/*
		 * Service code to set noedit rule.
		 */
		if (!ppe_drv_sc_check_and_set(&service_code, PPE_DRV_SC_NOEDIT_RULE)) {
			ppe_drv_warn("%p: flow requires multiple service code, existing:%u new:%u",
					pcf, service_code, PPE_DRV_SC_NOEDIT_RULE);
			return false;
		}

		*scp = service_code;
		return true;
	}

	/*
	 * Get the service code for the flow according to the flow type
	 * and precedence of these features (like DS flows, policer/ACL based service code, etc)
	 */
	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_WIFI_DS)) {
		if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_BRIDGE_FLOW)) {
			sc = PPE_DRV_SC_DS_MLO_LINK_BR_NODE0 + pcf->wifi_rule_ds_metadata;
		} else {
			sc = PPE_DRV_SC_DS_MLO_LINK_RO_NODE0 + pcf->wifi_rule_ds_metadata;
		}
	} else if((ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_POLICER_VALID) ||
			ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_ACL_VALID)) && (pcf->acl_sc != PPE_DRV_SC_NONE)) {
		sc = pcf->acl_sc;
	} else if (pp->user_type == PPE_DRV_PORT_USER_TYPE_DS) {
		if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_VP_VALID)) {
			if (pp->core_mask) {
				next_core = __builtin_ffs(pp->shadow_core_mask) - 1;
				pp->shadow_core_mask &= ~(1 << next_core);
				sc = PPE_DRV_CORE2SC_EDIT(next_core);
				if (!pp->shadow_core_mask) {
					pp->shadow_core_mask = pp->core_mask;
				}
			} else {
				sc = PPE_DRV_SC_VP_RPS;
			}
		} else if (ppe_drv_v6_conn_flow_flags_check(pcf,
					PPE_DRV_V6_CONN_FLAG_FLOW_OFFLOAD_DISABLED)) {
			if (pp->core_mask) {
				next_core = __builtin_ffs(pp->shadow_core_mask) - 1;
				pp->shadow_core_mask &= ~(1 << next_core);
				sc = PPE_DRV_CORE2SC_NOEDIT(next_core);
				if (!pp->shadow_core_mask) {
					pp->shadow_core_mask = pp->core_mask;
				}
			} else {
				ppe_drv_warn("%p: invalid core mask for ds user type", pcf);
				return false;
			}
		}
	} else if ((pp->user_type == PPE_DRV_PORT_USER_TYPE_ACTIVE_VP) && pp->core_mask) {
		if (ppe_drv_v6_conn_flow_flags_check(pcf,
					PPE_DRV_V6_CONN_FLAG_FLOW_OFFLOAD_DISABLED)) {
			next_core = __builtin_ffs(pp->shadow_core_mask) - 1;
			pp->shadow_core_mask &= ~(1 << next_core);
			sc = PPE_DRV_CORE2SC_NOEDIT(next_core);
			if (!pp->shadow_core_mask) {
				pp->shadow_core_mask = pp->core_mask;
			}
		} else {
			next_core = __builtin_ffs(pp->shadow_core_mask) - 1;
			pp->shadow_core_mask &= ~(1 << next_core);
			sc = PPE_DRV_CORE2SC_EDIT(next_core);
			if (!pp->shadow_core_mask) {
				pp->shadow_core_mask = pp->core_mask;
			}
		}
	} else if ((pp->user_type == PPE_DRV_PORT_USER_TYPE_PASSIVE_VP) && pp->core_mask) {
		next_core = __builtin_ffs(pp->shadow_core_mask) - 1;
		pp->shadow_core_mask &= ~(1 << next_core);
		sc = PPE_DRV_CORE2SC_NOEDIT(next_core);
		if (!pp->shadow_core_mask) {
			pp->shadow_core_mask = pp->core_mask;
		}
	}

	if (is_vlan_dev(port_rx->dev) && (port_rx->type == PPE_DRV_PORT_VIRTUAL) && (netif_is_bridge_port(port_rx->dev)) &&
	    !ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_BRIDGE_FLOW)) {
		sc = PPE_DRV_SC_SPF_BYPASS;
	}

	if (sc != PPE_DRV_SC_NONE) {
		if (!ppe_drv_sc_check_and_set(&service_code, sc)) {
			ppe_drv_warn("%p: flow requires multiple service code, existing:%u new:%u",
					pcf, service_code, sc);
			return false;
		}
	}

	/*
	 * SC required to accelerate inline EIP flows.
	 */
	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_INLINE_IPSEC)) {
		if (!ppe_drv_sc_check_and_set(&service_code, PPE_DRV_SC_IPSEC_PPE2EIP)) {
			ppe_drv_warn("%p: EIP flow requires multiple service codes existing:%u new:%u",
					pcf, service_code, PPE_DRV_SC_IPSEC_PPE2EIP);
			return false;
		}
	}

	/*
	 * SC required when its a bridge flow.
	 */
	if ((sc == PPE_DRV_SC_NONE) && ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_BRIDGE_FLOW)) {
		sc = PPE_DRV_SC_VLAN_FILTER_BYPASS;
		if (!ppe_drv_sc_check_and_set(&service_code, sc)) {
			ppe_drv_warn("%p: Bridge flow requires multiple service codes existing:%u new:%u",
					pcf, service_code, sc);
			return false;
		}
	}

	if ((sc == PPE_DRV_SC_NONE) &&
	    (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLAG_BRIDGE_VLAN_NETDEV))) {
		sc = PPE_DRV_SC_VLAN_FILTER_BYPASS;
		if (!ppe_drv_sc_check_and_set(&service_code, sc)) {
			ppe_drv_warn("%p: service code %d update failed in VLAN over bridge sc %d\n", pcf, service_code,
				     sc);
			return false;
		}
	}

	*scp = service_code;
	return true;
}

/*
 * ppe_drv_flow_v6_get()
 *	Find flow table entry.
 */
struct ppe_drv_flow *ppe_drv_flow_v6_get(struct ppe_drv_v6_5tuple *tuple)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_flow_host_entry_t flow_host = {0};
	fal_flow_entry_t *flow_cfg = &flow_host.flow_entry;
	fal_host_entry_t *host_cfg = &flow_host.host_entry;
	uint8_t protocol = tuple->protocol;
	struct ppe_drv_flow *flow;
	bool tuple_3 = false;
	sw_error_t err;

	/*
	 * Fill host entry.
	 */
	memcpy(host_cfg->ip6_addr.ul, tuple->flow_ip, sizeof(tuple->flow_ip));
	host_cfg->flags = FAL_IP_IP6_ADDR;

	/*
	 * Fill relevant details for 3-tuple and 5-tuple flows.
	 */
	if (protocol == IPPROTO_TCP) {
		flow_cfg->protocol = FAL_FLOW_PROTOCOL_TCP;
		ppe_drv_trace("%p: flow_tbl[protocol]: TCP-%u", tuple, FAL_FLOW_PROTOCOL_TCP);
	} else if (protocol == IPPROTO_UDP) {
		flow_cfg->protocol = FAL_FLOW_PROTOCOL_UDP;
		ppe_drv_trace("%p: flow_tbl[protocol]: UDP-%u", tuple, FAL_FLOW_PROTOCOL_UDP);
	} else if (protocol == IPPROTO_UDPLITE) {
		flow_cfg->protocol = FAL_FLOW_PROTOCOL_UDPLITE;
		ppe_drv_trace("%p: flow_tbl[protocol]: UDP-Lite-%u", tuple, FAL_FLOW_PROTOCOL_UDPLITE);
	} else if (protocol == IPPROTO_ESP) {
		tuple_3 = true;
		ppe_drv_trace("%p: flow_tbl[protocol]: Other-%u", tuple, FAL_FLOW_PROTOCOL_OTHER);
	} else if (protocol == IPPROTO_IPIP) {
		tuple_3 = true;
		ppe_drv_trace("%p: flow_tbl[protocol]: Other-%u", tuple, FAL_FLOW_PROTOCOL_OTHER);
	} else if (protocol == IPPROTO_GRE) {
		tuple_3 = true;
		ppe_drv_trace("%p: flow_tbl[protocol]: Other-%u", tuple, FAL_FLOW_PROTOCOL_OTHER);
	} else {
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_query_unknown_proto);
		ppe_drv_warn("%p: protocol: %u incorrect for PPE", tuple, protocol);
		return NULL;
	}

	memcpy(flow_cfg->flow_ip.ipv6.ul, tuple->return_ip, sizeof(tuple->return_ip));

	ppe_drv_trace("%p: flow_tbl[dest_ip]: %pI6", tuple, &tuple->return_ip);
	if (tuple_3) {
		flow_cfg->entry_type = FAL_FLOW_IP6_3TUPLE_ADDR;
		flow_cfg->protocol = FAL_FLOW_PROTOCOL_OTHER;
		flow_cfg->ip_type = protocol;
		ppe_drv_trace("%p: flow_tbl[ip_proto]: 0x%x", tuple, protocol);
	} else {
		flow_cfg->entry_type = FAL_FLOW_IP6_5TUPLE_ADDR;

		flow_cfg->src_port = tuple->flow_ident;
		flow_cfg->dst_port = tuple->return_ident;

		ppe_drv_trace("%p: flow_tbl[sport]: %u", tuple, flow_cfg->src_port);
		ppe_drv_trace("%p: flow_tbl[dport]: %u", tuple, flow_cfg->dst_port);

	}

	/*
	 * query hardware
	 */
	err = fal_flow_host_get(PPE_DRV_SWITCH_ID, FAL_FLOW_OP_MODE_KEY, &flow_host);
	if (err != SW_OK) {
		ppe_drv_trace("%p: flow get failed", tuple);
		return NULL;
	}

	/*
	 * Get the sw instance of flow entry.
	 */
	flow = &p->flow[flow_cfg->entry_id];
	ppe_drv_assert((flow->flags & PPE_DRV_FLOW_VALID), "%p: flow entry is already decelerated from PPE at index: %d",
			tuple, flow_cfg->entry_id);

	ppe_drv_assert((host_cfg->entry_id == flow->host->index),
			"%p flow entry and host entry mismatch flow-index: %d hw-host_index: %d sw-host_index: %d",
			tuple, flow->index, host_cfg->entry_id, flow->host->index);

	ppe_drv_trace("%p: flow_tbl entry found at index: %u", tuple, flow_cfg->entry_id);
	return flow;
}

/*
 * ppe_drv_flow_v6_add()
 *	Add flow table entry.
 */
struct ppe_drv_flow *ppe_drv_flow_v6_add(struct ppe_drv_v6_conn_flow *pcf, struct ppe_drv_nexthop *nh,
					struct ppe_drv_host *host, bool entry_valid)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_stats_sawf_sc *sawf_sc_stats;
	struct ppe_drv_flow_tree_id_data *tree_id_data = &(pcf->flow_metadata.tree_id_data);
	fal_flow_entry_t flow_cfg = {0};
	uint32_t match_dest_ip[4] = {0};
	struct in6_addr network_dest_ip = {0};
	uint32_t match_protocol = ppe_drv_v6_conn_flow_match_protocol_get(pcf);
	uint8_t vlan_hdr_cnt = ppe_drv_v6_conn_flow_egress_vlan_cnt_get(pcf);
	uint8_t service_class;
	struct ppe_drv_iface *port_if = ppe_drv_v6_conn_flow_eg_port_if_get(pcf);
	struct ppe_drv_port *pp = NULL;
	struct ppe_drv_flow *flow;
	bool tuple_3 = false;
	bool wifi_qos_en = false;
	uint16_t xmit_mtu;
	sw_error_t err;

	/*
	 * PPE port reference is not taken for priority assist in PPE. PPE is
	 * used only for priority queue selection. Hence Port would not be valid
	 */
	if (!ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_PRIORITY_PPE_ASSIST)) {
		if (!port_if) {
			ppe_drv_warn("%p: Invalid egress port_if", pcf);
			return NULL;
		}

		pp = ppe_drv_iface_port_get(port_if);
		if (!pp) {
			ppe_drv_warn("%p: Invalid egress port", pcf);
			return NULL;
		}
	}

	ppe_drv_v6_conn_flow_match_dest_ip_get(pcf, &match_dest_ip[0]);

	/*
	 * Change the destination ip to network byte order
	 */
	PPE_DRV_IPV6_TO_IN6(network_dest_ip, match_dest_ip)

	ppe_drv_trace("%p: flow_tbl[host_idx]: %u", pcf, host->index);
	flow_cfg.host_addr_type = PPE_DRV_HOST_LAN;
	flow_cfg.host_addr_index = host->index;
	flow_cfg.deacclr_en = A_FALSE;
	flow_cfg.invalid = !entry_valid;
	flow_cfg.sevice_code = PPE_DRV_SC_NONE;

	if (!pp) {
		ppe_drv_warn("%p: Invalid egress port", pcf);
		return NULL;
	}

	if (!ppe_drv_flow_v6_service_code_get(pcf, pp, &flow_cfg.sevice_code)) {
		ppe_drv_warn("%p: failed to obtain a valid service code", pcf);
		return NULL;
	}
	ppe_drv_trace("service_code: %d\n", flow_cfg.sevice_code);

	ppe_drv_trace("pcf %p: flow_tbl[host_idx]: %u sevice_code %d\n", pcf, host->index,
		      flow_cfg.sevice_code);

	/*
	 * Get the tree ID corresponding to flow.
	 */
	if (!ppe_drv_flow_v6_tree_id_get(pcf, &flow_cfg.flow_qos.tree_id)) {
		ppe_drv_warn("%p: failed to obtain a valid tree ID", pcf);
		return NULL;
	}

	/*
	 * Get the VPN ID corresponding to flow.
	 */
	if (!ppe_drv_flow_v6_vpn_id_get(pcf, &flow_cfg.vpn_id)) {
		ppe_drv_warn("%p: failed to obtain a valid vpn_id", pcf);
		return NULL;
	}

	/*
	 * Get the WIFI QOS corresponding to flow.
	 */
	if (!ppe_drv_flow_v6_wifi_qos_get(pcf, &flow_cfg.flow_qos.wifi_qos, &wifi_qos_en)) {
		ppe_drv_warn("%p: failed to obtain wifi qos", pcf);
		return NULL;
	}

	flow_cfg.flow_qos.wifi_qos_en = wifi_qos_en;

#ifdef NSS_PPE_IPQ53XX
	if (!ppe_drv_flow_v6_policer_get(pcf, &flow_cfg.policer_index, &flow_cfg.policer_valid)) {
		ppe_drv_warn("%p: failed to obtain policer_index", pcf);
		return NULL;
	}

	/*
	 * Get the Source interface index.
	 */
	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_SRC_INTERFACE_CHECK)) {
		/*
		 * Get the source interface index corresponding to flow.
		 * For bridge flow = Rx port no.
		 * For routed flow = ingress l3 if index.
		 */
		if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_BRIDGE_FLOW)) {
			struct ppe_drv_iface *in_port_if = ppe_drv_v6_conn_flow_in_port_if_get(pcf);
			struct ppe_drv_port *port = ppe_drv_iface_port_get(in_port_if);
			if (port) {
				flow_cfg.src_intf_index = port->port;
				flow_cfg.src_intf_valid = A_TRUE;
				ppe_drv_trace("%p: Bridged flow, src_intf_index: %u", pcf, flow_cfg.src_intf_index);
			}
		} else {
			struct ppe_drv_iface *in_l3_if = ppe_drv_v6_conn_flow_in_l3_if_get(pcf);
			struct ppe_drv_l3_if *l3_if = ppe_drv_iface_l3_if_get(in_l3_if);
			if (l3_if) {
				flow_cfg.src_intf_index = l3_if->l3_if_index;
				flow_cfg.src_intf_valid = A_TRUE;
				ppe_drv_trace("%p: Routed flow, src_intf_index: %u", pcf, flow_cfg.src_intf_index);
			}
		}
	}
#endif

	/*
	 * Set forwarding type
	 */
	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_PPE_POLICER_ASSIST)) {
		flow_cfg.fwd_type = ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_BRIDGE_FLOW) ?
					FAL_FLOW_BRIDGE: FAL_FLOW_ROUTE;
		ppe_drv_trace("%p: Policer enabled flow\n", pcf);
	} else if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_RFS_PPE_ASSIST)) {
		flow_cfg.fwd_type = ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_BRIDGE_FLOW) ?
				    FAL_FLOW_BRIDGE: FAL_FLOW_ROUTE;
		ppe_drv_trace("%p: RFS enabled flow\n", pcf);
	} else if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_PRIORITY_PPE_ASSIST)) {
		/*
		 * Case PPE is used only to Assist in priority marking of packets
		 */
		flow_cfg.fwd_type = FAL_FLOW_RDT_TO_CPU;
		ppe_drv_trace("%p: flow_tbl[fwd_type]: Priority Assist: %u", pcf, FAL_FLOW_FORWARD);
	} else if (ipv6_addr_is_multicast(&network_dest_ip)) {
		/*
		 * Multicast flow
		 */
		flow_cfg.fwd_type = FAL_FLOW_FORWARD;
		ppe_drv_trace("%p: flow_tbl[fwd_type]: L2-Multicast: %u", pcf, FAL_FLOW_FORWARD);
	} else if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_BRIDGE_FLOW)) {
		/*
		 * Case bridging
		 */
		flow_cfg.fwd_type = FAL_FLOW_BRIDGE;
		ppe_drv_trace("%p: flow_tbl[fwd_type]: L2: %u", pcf, FAL_FLOW_BRIDGE);

#ifdef NSS_PPE_IPQ53XX
		/*
		 * VLAN tag addition from NEXTHOP_TBL for flow based bridging.
		 * With VLAN tagging capability from flow table for bridge flows,
		 * we can avoid EG_VLAN_XLT_RULE based VLAN tagging. This would help
		 * doing VLANs between different ingress and egress VSI.
		 */
		if (nh) {
			flow_cfg.bridge_nexthop_valid = A_TRUE;
			flow_cfg.bridge_nexthop = nh->index;
			ppe_drv_trace("%p:nexthop index: %u", pcf, nh->index);
		}
#endif
		flow_cfg.port_valid = A_TRUE;
		flow_cfg.bridge_port = pp->port;
		ppe_drv_trace("%p: xmit interface port: %d", pcf, pp->port);

		if (ppe_drv_port_is_tunnel_vp(pp)) {
			switch (vlan_hdr_cnt) {
				case 2:
					flow_cfg.svlan_fmt = A_TRUE;
					flow_cfg.cvlan_fmt = A_TRUE;
					flow_cfg.vlan_fmt_valid = 1;
					break;
				case 1:
					flow_cfg.cvlan_fmt = A_TRUE;
					flow_cfg.vlan_fmt_valid = 1;
					break;
				case 0:
					break;
				default:
					ppe_drv_warn("%p: cannot acclerate bridge VLAN flows for more than 2 vlans: %u",
							pcf, vlan_hdr_cnt);
					return NULL;
			}
		}
	} else {
		/*
		 * Case simple routing (Non-NAT)
		 */
		flow_cfg.route_nexthop = nh->index;
		flow_cfg.fwd_type = FAL_FLOW_ROUTE;
		ppe_drv_trace("%p: flow_tbl[fwd_type]: L3: %u", pcf, FAL_FLOW_ROUTE);
	}

	/*
	 * Fill relevant details for 3-tuple and 5-tuple flows.
	 */
	switch (match_protocol) {
	case IPPROTO_TCP:
		flow_cfg.protocol = FAL_FLOW_PROTOCOL_TCP;
		ppe_drv_trace("%p: flow_tbl[protocol]: TCP-%u", pcf, FAL_FLOW_PROTOCOL_TCP);
		break;

	case IPPROTO_UDP:
		flow_cfg.protocol = FAL_FLOW_PROTOCOL_UDP;
		ppe_drv_trace("%p: flow_tbl[protocol]: UDP-%u", pcf, FAL_FLOW_PROTOCOL_UDP);
		break;

	case IPPROTO_UDPLITE:
		flow_cfg.protocol = FAL_FLOW_PROTOCOL_UDPLITE;
		ppe_drv_trace("%p: flow_tbl[protocol]: UDP-Lite-%u", pcf, FAL_FLOW_PROTOCOL_UDPLITE);
		break;

	case IPPROTO_ESP:
		tuple_3 = true;
		ppe_drv_trace("%p: flow_tbl[protocol]: Other-%u", pcf, FAL_FLOW_PROTOCOL_OTHER);
		break;

	case IPPROTO_IPIP:
		tuple_3 = true;
		ppe_drv_trace("%p: flow_tbl[protocol]: Other-%u", pcf, FAL_FLOW_PROTOCOL_OTHER);
		break;

	case IPPROTO_GRE:
		tuple_3 = true;
		ppe_drv_trace("%p: flow_tbl[protocol]: Other-%u", pcf, FAL_FLOW_PROTOCOL_OTHER);
		break;

	default:
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_unknown_proto);
		ppe_drv_warn("%p: protocol: %u cannot be offloaded to PPE", pcf, match_protocol);
		return NULL;
	}

	memcpy(flow_cfg.flow_ip.ipv6.ul, &match_dest_ip, sizeof(match_dest_ip));
	ppe_drv_trace("%p: flow_tbl[dest_ip]: %pI6", pcf, &match_dest_ip);
	if (tuple_3) {
		flow_cfg.entry_type = FAL_FLOW_IP6_3TUPLE_ADDR;
		flow_cfg.protocol = FAL_FLOW_PROTOCOL_OTHER;
		flow_cfg.ip_type = match_protocol;
		ppe_drv_trace("%p: flow_tbl[ip_proto]: 0x%x", pcf, match_protocol);
	} else {
		flow_cfg.entry_type = FAL_FLOW_IP6_5TUPLE_ADDR;

		flow_cfg.src_port = ppe_drv_v6_conn_flow_match_src_ident_get(pcf);
		flow_cfg.dst_port = ppe_drv_v6_conn_flow_match_dest_ident_get(pcf);

		ppe_drv_trace("%p: flow_tbl[sport]: %u", pcf, flow_cfg.src_port);
		ppe_drv_trace("%p: flow_tbl[dport]: %u", pcf, flow_cfg.dst_port);

	}

	/*
	 * Fill Path-MTU.
	 *
	 * Note: For multicast flows, it should be ok to program the minimum MTU value
	 * of all the interfaces, since PPE also check MTU for each destination interface
	 * and exception the packet (without cloning) if MTU check fail for any interface.
	 */
	xmit_mtu = ipv6_addr_is_multicast(&network_dest_ip) ? ppe_drv_v6_conn_flow_mc_min_mtu_get(pcf)
		: ppe_drv_v6_conn_flow_xmit_interface_mtu_get(pcf);
	if (xmit_mtu > PPE_DRV_PORT_JUMBO_MAX) {
		ppe_drv_trace("%p: xmit_mtu: %d is larger, restricting to max: %d", pcf, xmit_mtu, PPE_DRV_PORT_JUMBO_MAX);
		xmit_mtu = PPE_DRV_PORT_JUMBO_MAX;
	}

	flow_cfg.pmtu_check_l3 = (a_bool_t)PPE_DRV_FLOW_PMTU_TYPE_L3;
	flow_cfg.pmtu = xmit_mtu;

	ppe_drv_trace("%p: flow_tbl[PMTU]: %u", pcf, xmit_mtu);

	/*
	 * Add the flow
	 */
	err = fal_flow_entry_add(PPE_DRV_SWITCH_ID, FAL_FLOW_OP_MODE_KEY, &flow_cfg);
	if (err != SW_OK) {
		ppe_drv_trace("%p: flow entry add failed", pcf);
		return NULL;
	}

	/*
	 * Get the sw instance of flow entry.
	 */
	flow = &p->flow[flow_cfg.entry_id];
	ppe_drv_assert(!(flow->flags & PPE_DRV_FLOW_VALID), "%p: flow entry is already accelerated to PPE at index: %d",
			pcf, flow_cfg.entry_id);

	/*
	 * Increment flow count if SAWF service class is configured in tree_id.
	 */
	if (ppe_drv_tree_id_type_get(&pcf->flow_metadata) == PPE_DRV_TREE_ID_TYPE_SAWF) {
		service_class = tree_id_data->info.sawf_metadata.service_class;
		if (PPE_DRV_SERVICE_CLASS_IS_VALID(service_class)) {
			sawf_sc_stats = &p->stats.sawf_sc_stats[service_class];
			ppe_drv_stats_inc(&sawf_sc_stats->flow_count);
			ppe_drv_trace("Stats Updated: service class %u : flow  %llu\n", service_class, atomic64_read(&sawf_sc_stats->flow_count));
		}
	}

	/*
	 * Update the shadow copy
	 */
	flow->host = host;
	flow->nh = nh;
	flow->flags |= PPE_DRV_FLOW_V6;
	flow->type = PPE_DRV_IP_TYPE_V6;
	flow->entry_type = flow_cfg.entry_type;
	flow->pcf.v6 = pcf;
	ppe_drv_trace("%p: flow_tbl entry added at index: %u", pcf, flow_cfg.entry_id);
	return flow;
}

/*
 * ppe_drv_flow_v4_qos_set()
 *	Set QOS mapping for this flow.
 */
bool ppe_drv_flow_v4_qos_set(struct ppe_drv_v4_conn_flow *pcf, struct ppe_drv_flow *pf)
{
	sw_error_t err;
	fal_qos_cosmap_t qos_cfg = {0};

	/*
	 * Check if flow needs qos update, set priority profile in QoS config
	 */
	if (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_QOS_VALID)) {
		qos_cfg.internal_pri = ppe_drv_v4_conn_flow_int_pri_get(pcf);
		qos_cfg.pri_en = A_TRUE;
	}

	/*
	 * Check if flow needs DSCP marking, set DSCP fields in QoS config
	 */
	if (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_DSCP_MARKING)) {
		qos_cfg.internal_dscp = ppe_drv_v4_conn_flow_egress_dscp_get(pcf) << PPE_DRV_DSCP_SHIFT;
		qos_cfg.dscp_mask = PPE_DRV_DSCP_MASK;
		qos_cfg.dscp_en = A_TRUE;
	}

	/*
	 * Check if flow needs PCP marking, set PCP fields in QoS config
	 */
	if ((ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_VLAN_PRI_MARKING))
			&& (ppe_drv_v4_conn_flow_egress_vlan_cnt_get(pcf) == 1)) {
		/*
		 * Note: PPE does not have flow based support for double vlan tagging.
		 * We do not need to mask unsigned integer.
		 */
		qos_cfg.internal_pcp = ppe_drv_v4_conn_flow_egress_vlan_get(pcf, 0)->tci >> PPE_DRV_VLAN_PRIORITY_SHIFT;;
		qos_cfg.pcp_en = A_TRUE;
	}

	err = fal_qos_cosmap_flow_set(PPE_DRV_SWITCH_ID, 0, pf->index, &qos_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p qos(DSCP/PCP) mapping configuration failed for flow: %p", pcf, pf);
		return false;
	}

	ppe_drv_trace("%p qos(DSCP/PCP) mapping configuration done for flow: %p", pcf, pf);
	return true;
}

/*
 * ppe_drv_flow_v4_qos_clear()
 *	Return service code required for this flow.
 */
bool ppe_drv_flow_v4_qos_clear(struct ppe_drv_flow *pf)
{
	sw_error_t err;
	fal_qos_cosmap_t qos_cfg = {0};

	err = fal_qos_cosmap_flow_set(PPE_DRV_SWITCH_ID, 0, pf->index, &qos_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p qos clear configuration failed for flow", pf);
		return false;
	}

	ppe_drv_trace("%p qos clear configuration done for flow", pf);
	return true;
}

/*
 * ppe_drv_flow_v4_tree_id_get()
 *	Find the tree ID associated with a flow
 */
static bool ppe_drv_flow_v4_tree_id_get(struct ppe_drv_v4_conn_flow *pcf, uint32_t *tree_id)
{
	struct ppe_drv_flow_tree_id_data *tree_id_data = &(pcf->flow_metadata.tree_id_data);

	switch (tree_id_data->type) {
	case PPE_DRV_TREE_ID_TYPE_NONE:
		*tree_id = tree_id_data->info.value;
		return true;

	case PPE_DRV_TREE_ID_TYPE_SAWF:
		PPE_DRV_TREE_ID_TYPE_SET(tree_id, tree_id_data->type);
		PPE_DRV_TREE_ID_SERVICE_CLASS_SET(tree_id, tree_id_data->info.sawf_metadata.service_class);
		PPE_DRV_TREE_ID_PEER_ID_SET(tree_id, tree_id_data->info.sawf_metadata.peer_id);
		return true;

	case PPE_DRV_TREE_ID_TYPE_WIFI_TID:
        case PPE_DRV_TREE_ID_TYPE_SCS:
		PPE_DRV_TREE_ID_TYPE_SET(tree_id, tree_id_data->type);
		return true;

	case PPE_DRV_TREE_ID_TYPE_MLO_ASSIST:
		PPE_DRV_TREE_ID_TYPE_SET(tree_id, tree_id_data->type);
		PPE_DRV_TREE_ID_MLO_MARK_SET(tree_id, tree_id_data->info.value);
		return true;

	default:
		ppe_drv_warn("Invalid tree_id_type : (%u)", tree_id_data->type);
		return false;
	}
}

/*
 * ppe_drv_flow_v4_vpn_id_get()
 *	Find the tree ID associated with a flow
 */
static bool ppe_drv_flow_v4_vpn_id_get(struct ppe_drv_v4_conn_flow *pcf, uint32_t *vpn_id)
{
	/*
	 * TODO: to be added later.
	 */
	*vpn_id = 0;
	return true;
}

/*
 * ppe_drv_flow_v4_wifi_qos_get()
 *	Find the WIFI QOS associated with a flow
 */
static bool ppe_drv_flow_v4_wifi_qos_get(struct ppe_drv_v4_conn_flow *pcf, uint32_t *wifi_qos, bool *wifi_qos_en)
{
	bool flow_override_mode = true;

	/*
	 * If SAWF metadata is valid, set 6 bit MSDUQ in wifi_qos field (bits 0-5).
	 */
	if (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLOW_METADATA_TYPE_WIFI_INFO)) {
		/*
		 * MSDUQ representation in wifi_qos field is:
		 * --------------------------------------------------------------------------------------
		 * |	Who Classify (2 bits)	|	Flow override (1 bit)	|	TID (3 bits)	|
		 * --------------------------------------------------------------------------------------
		 */
		*wifi_qos = pcf->flow_metadata.wifi_qos;
		*wifi_qos_en = true;

		/*
		 * In case of DS mode, rearrange the MSDUQ representation in wifi qos field to be aligned with TCL descriptor.
		 */
		if (!ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLAG_FLOW_VP_VALID) &&
				(pcf->tx_port->user_type == PPE_DRV_PORT_USER_TYPE_DS)) {
			flow_override_mode = ppe_drv_flow_override_mode_get(&pcf->flow_metadata.wifi_qos);

			/*
			 * Disabling WIFI_QOS flag for hlos tid mode
			 */
			if (!flow_override_mode)
				*wifi_qos_en = false;

			ppe_drv_flow_ds_wifi_qos_set(wifi_qos, &pcf->flow_metadata.wifi_qos, flow_override_mode);
			ppe_drv_trace("WiFi_QoS configured in DS descriptor is: 0x%x\n", *wifi_qos);
		}

		ppe_drv_trace("For User type: %u, WiFi_QoS initially: 0x%x and WiFi_QoS configured: 0x%x", pcf->tx_port->user_type, pcf->flow_metadata.wifi_qos, *wifi_qos);
	}

	return true;
}

#ifdef NSS_PPE_IPQ53XX
/*
 * ppe_drv_flow_v4_policer_get()
 *	Find the Policer get associated with a flow
 */
static bool ppe_drv_flow_v4_policer_get(struct ppe_drv_v4_conn_flow *pcf, uint32_t *policer_index, a_bool_t *policer_valid)
{
	if (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLAG_FLOW_POLICER_VALID)) {
		*policer_index = pcf->policer_hw_id;
		*policer_valid = A_TRUE;
	}

	return true;
}
#endif

/*
 * ppe_drv_flow_v4_service_code_get()
 *	Return service code required for this flow.
 */
bool ppe_drv_flow_v4_service_code_get(struct ppe_drv_v4_conn_flow *pcf, struct ppe_drv_port *pp, uint8_t *scp)
{
	struct ppe_drv_port *port_rx = pcf->rx_port;
	ppe_drv_sc_t service_code = *scp;
	ppe_drv_sc_t sc = PPE_DRV_SC_NONE;
	int next_core;

	if (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLAG_FLOW_PRIORITY_PPE_ASSIST)) {
		/*
		 * Service code to set priority for PPE assisted flows.
		 * No other service code is supported if priority assist is active.
		 */
		if (!ppe_drv_sc_check_and_set(&service_code, PPE_DRV_SC_NOEDIT_PRIORITY_SET)) {
			ppe_drv_warn("%p: flow requires multiple service code, existing:%u new:%u",
						pcf, service_code, PPE_DRV_SC_NOEDIT_PRIORITY_SET);
			return false;
		}

		*scp = service_code;
		return true;
	}

	/*
	 * Service code to enable PPE to bypass packet header editing and forward them unmodified.
	 */
	if (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLAG_FLOW_NO_EDIT_RULE)) {
		/*
		 * Service code to set noedit rule.
		 */
		if (!ppe_drv_sc_check_and_set(&service_code, PPE_DRV_SC_NOEDIT_RULE)) {
			ppe_drv_warn("%p: flow requires multiple service code, existing:%u new:%u",
						pcf, service_code, PPE_DRV_SC_NOEDIT_RULE);
			return false;
		}

		*scp = service_code;
		return true;
	}

	/*
	 * Get the service code for the flow according to the flow type
	 * and precedence of these features (like DS flows, policer/ACL based service code, etc)
	 */
	if (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLAG_FLOW_WIFI_DS)) {
		if (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_BRIDGE_FLOW)) {
			sc = PPE_DRV_SC_DS_MLO_LINK_BR_NODE0 + pcf->wifi_rule_ds_metadata;
		} else {
			sc = PPE_DRV_SC_DS_MLO_LINK_RO_NODE0 + pcf->wifi_rule_ds_metadata;
		}
	} else if ((ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLAG_FLOW_POLICER_VALID) ||
		ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLAG_FLOW_ACL_VALID)) && (pcf->acl_sc != PPE_DRV_SC_NONE)) {
		sc = pcf->acl_sc;
	} else if (pp->user_type == PPE_DRV_PORT_USER_TYPE_DS) {
		if (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLAG_FLOW_VP_VALID)) {
			if (pp->core_mask) {
				next_core = __builtin_ffs(pp->shadow_core_mask) - 1;
				pp->shadow_core_mask &= ~(1 << next_core);
				sc = PPE_DRV_CORE2SC_EDIT(next_core);
				if (!pp->shadow_core_mask) {
					pp->shadow_core_mask = pp->core_mask;
				}
			} else {
				sc = PPE_DRV_SC_VP_RPS;
			}
		} else if (ppe_drv_v4_conn_flow_flags_check(pcf,
					PPE_DRV_V4_CONN_FLAG_FLOW_OFFLOAD_DISABLED)) {
			if (pp->core_mask) {
				next_core = __builtin_ffs(pp->shadow_core_mask) - 1;
				pp->shadow_core_mask &= ~(1 << next_core);
				sc = PPE_DRV_CORE2SC_NOEDIT(next_core);
				if (!pp->shadow_core_mask) {
					pp->shadow_core_mask = pp->core_mask;
				}
			} else {
				ppe_drv_warn("%p: invalid core mask for DS user type", pcf);
				return false;
			}
		}
	} else if ((pp->user_type == PPE_DRV_PORT_USER_TYPE_ACTIVE_VP) && pp->core_mask) {
		if (ppe_drv_v4_conn_flow_flags_check(pcf,
					PPE_DRV_V4_CONN_FLAG_FLOW_OFFLOAD_DISABLED)) {
			next_core = __builtin_ffs(pp->shadow_core_mask) - 1;
			pp->shadow_core_mask &= ~(1 << next_core);
			sc = PPE_DRV_CORE2SC_NOEDIT(next_core);
			if (!pp->shadow_core_mask) {
				pp->shadow_core_mask = pp->core_mask;
			}
		} else {
			next_core = __builtin_ffs(pp->shadow_core_mask) - 1;
			pp->shadow_core_mask &= ~(1 << next_core);
			sc = PPE_DRV_CORE2SC_EDIT(next_core);
			if (!pp->shadow_core_mask) {
				pp->shadow_core_mask = pp->core_mask;
			}
		}
	} else if ((pp->user_type == PPE_DRV_PORT_USER_TYPE_PASSIVE_VP) && pp->core_mask) {
		next_core = __builtin_ffs(pp->shadow_core_mask) - 1;
		pp->shadow_core_mask &= ~(1 << next_core);
		sc = PPE_DRV_CORE2SC_NOEDIT(next_core);
		if (!pp->shadow_core_mask) {
			pp->shadow_core_mask = pp->core_mask;
		}
	}

	if (is_vlan_dev(port_rx->dev) && (port_rx->type == PPE_DRV_PORT_VIRTUAL) && (netif_is_bridge_port(port_rx->dev)) &&
	    !ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_BRIDGE_FLOW)) {
		sc = PPE_DRV_SC_SPF_BYPASS;
	}

	if (sc != PPE_DRV_SC_NONE) {
		if (!ppe_drv_sc_check_and_set(&service_code, sc)) {
			ppe_drv_warn("%p: flow requires multiple service code, existing:%u new:%u",
					pcf, service_code, sc);
			return false;
		}
	}

	/*
	 * SC required to accelerate inline EIP flows.
	 */
	if (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_INLINE_IPSEC)) {
		if (!ppe_drv_sc_check_and_set(&service_code, PPE_DRV_SC_IPSEC_PPE2EIP)) {
			ppe_drv_warn("%p: EIP flow requires multiple service codes existing:%u new:%u",
					pcf, service_code, PPE_DRV_SC_IPSEC_PPE2EIP);
			return false;
		}
	}

	/*
	 * SC required when its a bridge flow.
	 */
	if ((sc == PPE_DRV_SC_NONE) && ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_BRIDGE_FLOW)) {
		sc = PPE_DRV_SC_VLAN_FILTER_BYPASS;
		if (!ppe_drv_sc_check_and_set(&service_code, sc)) {
			ppe_drv_warn("%p: Bridge flow requires multiple service codes existing:%u new:%u",
					pcf, service_code, sc);
			return false;
		}
	}

	if ((sc == PPE_DRV_SC_NONE) &&
	    (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLAG_BRIDGE_VLAN_NETDEV))) {
		sc = PPE_DRV_SC_VLAN_FILTER_BYPASS;
		if (!ppe_drv_sc_check_and_set(&service_code, sc)) {
			ppe_drv_warn("%p: service code %d update failed in VLAN over bridge sc %d\n", pcf, service_code,
				     sc);
			return false;
		}
	}

	*scp = service_code;
	return true;
}

/*
 * ppe_drv_flow_del()
 *	Delete a flow entry.
 */
bool ppe_drv_flow_del(struct ppe_drv_flow *pf)
{
	fal_flow_entry_t flow_cfg = {0};
	sw_error_t err;

	/*
	 * Delete the flow
	 */
	flow_cfg.entry_id = pf->index;
	flow_cfg.entry_type = pf->entry_type;
	err = fal_flow_entry_del(PPE_DRV_SWITCH_ID, FAL_FLOW_OP_MODE_INDEX, &flow_cfg);
	if (err != SW_OK) {
		ppe_drv_trace("%p: flow entry deletion failed", pf);
		return false;
	}

	/*
	 * Update the shadow copy
	 */
	pf->flags = 0;
	pf->type = 0;
	pf->entry_type = 0;
	ppe_drv_trace("%p: flow_tbl entry deleted at index: %u", pf, pf->index);
	return true;
}

/*
 * ppe_drv_flow_v4_get()
 *	Find flow table entry.
 */
struct ppe_drv_flow *ppe_drv_flow_v4_get(struct ppe_drv_v4_5tuple *tuple)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_flow_host_entry_t flow_host = {0};
	fal_flow_entry_t *flow_cfg = &flow_host.flow_entry;
	fal_host_entry_t *host_cfg = &flow_host.host_entry;
	uint8_t protocol = tuple->protocol;
	struct ppe_drv_flow *flow;
	bool tuple_3 = false;
	sw_error_t err;

	/*
	 * Fill host entry.
	 */
	host_cfg->ip4_addr = tuple->flow_ip;
	host_cfg->flags = FAL_IP_IP4_ADDR;

	/*
	 * Fill relevant details for 3-tuple and 5-tuple flows.
	 */
	if (protocol == IPPROTO_TCP) {
		flow_cfg->protocol = FAL_FLOW_PROTOCOL_TCP;
		ppe_drv_trace("%p: flow_tbl[protocol]: TCP-%u", tuple, FAL_FLOW_PROTOCOL_TCP);
	} else if (protocol == IPPROTO_UDP) {
		flow_cfg->protocol = FAL_FLOW_PROTOCOL_UDP;
		ppe_drv_trace("%p: flow_tbl[protocol]: UDP-%u", tuple, FAL_FLOW_PROTOCOL_UDP);
	} else if (protocol == IPPROTO_UDPLITE) {
		flow_cfg->protocol = FAL_FLOW_PROTOCOL_UDPLITE;
		ppe_drv_trace("%p: flow_tbl[protocol]: UDP-Lite-%u", tuple, FAL_FLOW_PROTOCOL_UDPLITE);
	} else if (protocol == IPPROTO_ESP) {
		tuple_3 = true;
		ppe_drv_trace("%p: flow_tbl[protocol]: Other-%u", tuple, FAL_FLOW_PROTOCOL_OTHER);
	} else if (protocol == IPPROTO_IPV6) {
		tuple_3 = true;
		ppe_drv_trace("%p: flow_tbl[protocol]: Other-%u", tuple, FAL_FLOW_PROTOCOL_OTHER);
	} else if (protocol == IPPROTO_GRE) {
		tuple_3 = true;
		ppe_drv_trace("%p: flow_tbl[protocol]: Other-%u", tuple, FAL_FLOW_PROTOCOL_OTHER);
	} else {
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_query_unknown_proto);
		ppe_drv_warn("%p: protocol: %u incorrect for PPE", tuple, protocol);
		return NULL;
	}

	flow_cfg->flow_ip.ipv4 = tuple->return_ip;
	ppe_drv_trace("%p: flow_tbl[dest_ip]: %pI4", tuple, &tuple->return_ip);
	if (tuple_3) {
		flow_cfg->entry_type = FAL_FLOW_IP4_3TUPLE_ADDR;
		flow_cfg->protocol = FAL_FLOW_PROTOCOL_OTHER;
		flow_cfg->ip_type = protocol;
		ppe_drv_trace("%p: flow_tbl[ip_proto]: 0x%x", tuple, protocol);
	} else {
		flow_cfg->entry_type = FAL_FLOW_IP4_5TUPLE_ADDR;

		flow_cfg->src_port = tuple->flow_ident;
		flow_cfg->dst_port = tuple->return_ident;

		ppe_drv_trace("%p: flow_tbl[sport]: %u", tuple, flow_cfg->src_port);
		ppe_drv_trace("%p: flow_tbl[dport]: %u", tuple, flow_cfg->dst_port);

	}

	/*
	 * query hardware
	 */
	err = fal_flow_host_get(PPE_DRV_SWITCH_ID, FAL_FLOW_OP_MODE_KEY, &flow_host);
	if (err != SW_OK) {
		ppe_drv_trace("%p: flow get failed", tuple);
		return NULL;
	}

	/*
	 * Get the sw instance of flow entry.
	 */
	flow = &p->flow[flow_cfg->entry_id];
	ppe_drv_assert((flow->flags & PPE_DRV_FLOW_VALID), "%p: flow entry is already decelerated from PPE at index: %d",
			tuple, flow_cfg->entry_id);

	ppe_drv_assert((host_cfg->entry_id == flow->host->index),
			"%p flow entry and host entry mismatch flow-index: %d hw-host_index: %d sw-host_index: %d",
			tuple, flow->index, host_cfg->entry_id, flow->host->index);

	ppe_drv_trace("%p: flow_tbl entry found at index: %u", tuple, flow_cfg->entry_id);
	return flow;
}

/*
 * ppe_drv_flow_v4_add()
 *	Add flow table entry.
 */
struct ppe_drv_flow *ppe_drv_flow_v4_add(struct ppe_drv_v4_conn_flow *pcf, struct ppe_drv_nexthop *nh,
					struct ppe_drv_host *host, bool entry_valid)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_stats_sawf_sc *sawf_sc_stats;
	struct ppe_drv_flow_tree_id_data *tree_id_data = &(pcf->flow_metadata.tree_id_data);
	fal_flow_entry_t flow_cfg = {0};
	uint32_t match_src_ip = ppe_drv_v4_conn_flow_match_src_ip_get(pcf);
	uint32_t match_dest_ip = ppe_drv_v4_conn_flow_match_dest_ip_get(pcf);
	uint32_t xlate_src_ip = ppe_drv_v4_conn_flow_xlate_src_ip_get(pcf);
	uint32_t xlate_dest_ip = ppe_drv_v4_conn_flow_xlate_dest_ip_get(pcf);
	uint32_t match_protocol = ppe_drv_v4_conn_flow_match_protocol_get(pcf);
	uint8_t vlan_hdr_cnt = ppe_drv_v4_conn_flow_egress_vlan_cnt_get(pcf);
	uint8_t service_class;
	struct ppe_drv_iface *port_if = ppe_drv_v4_conn_flow_eg_port_if_get(pcf);
	struct ppe_drv_port *pp = NULL;
	struct ppe_drv_flow *flow;
	bool tuple_3 = false;
	bool wifi_qos_en = false;
	uint16_t xmit_mtu;
	sw_error_t err;

	/*
	 * PPE port reference is not taken for priority assist in PPE as PPE is
	 * used only for priority queue selection. Hence port would not be valid
	 */
	if (!ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLAG_FLOW_PRIORITY_PPE_ASSIST)) {
		if (!port_if) {
			ppe_drv_warn("%p: Invalid egress port_if", pcf);
			return NULL;
		}

		pp = ppe_drv_iface_port_get(port_if);
		if (!pp) {
			ppe_drv_warn("%p: Invalid egress port", pcf);
			return NULL;
		}
	}

	/*
	 * Ensure it falls under SNAT, DNAT or simple L3 routing
	 */
	if ((match_src_ip != xlate_src_ip && match_dest_ip != xlate_dest_ip)
		&& (!ipv4_is_multicast(htonl(match_dest_ip)))) {
		ppe_drv_warn("%p: both source and dest IPs are getting translated", pcf);
		return NULL;
	}

	flow_cfg.host_addr_type = PPE_DRV_HOST_LAN;
	flow_cfg.host_addr_index = host->index;
	flow_cfg.deacclr_en = A_FALSE;
	flow_cfg.invalid = !entry_valid;
	flow_cfg.sevice_code = PPE_DRV_SC_NONE;

	if (!ppe_drv_flow_v4_service_code_get(pcf, pp, &flow_cfg.sevice_code)) {
		ppe_drv_warn("%p: failed to obtain a valid service code", pcf);
		return NULL;
	}
	ppe_drv_trace("service_code: %d\n", flow_cfg.sevice_code);

	ppe_drv_trace("pcf %p: flow_tbl[host_idx]: %u sevice_code %d\n", pcf, host->index, flow_cfg.sevice_code);
	/*
	 * Get the tree ID corresponding to flow.
	 */
	if (!ppe_drv_flow_v4_tree_id_get(pcf, &flow_cfg.flow_qos.tree_id)) {
		ppe_drv_warn("%p: failed to obtain a valid tree ID", pcf);
		return NULL;
	}

	/*
	 * Get the VPN ID corresponding to flow.
	 */
	if (!ppe_drv_flow_v4_vpn_id_get(pcf, &flow_cfg.vpn_id)) {
		ppe_drv_warn("%p: failed to obtain a valid vpn_id", pcf);
		return NULL;
	}

	/*
	 * Get the WIFI QOS corresponding to flow.
	 */
	if (!ppe_drv_flow_v4_wifi_qos_get(pcf, &flow_cfg.flow_qos.wifi_qos, &wifi_qos_en)) {
		ppe_drv_warn("%p: failed to obtain wifi qos", pcf);
		return NULL;
	}

	flow_cfg.flow_qos.wifi_qos_en = wifi_qos_en;

#ifdef NSS_PPE_IPQ53XX
	if (!ppe_drv_flow_v4_policer_get(pcf, &flow_cfg.policer_index, &flow_cfg.policer_valid)) {
		ppe_drv_warn("%p: failed to obtain policer_index", pcf);
		return NULL;
	}

	/*
	 * Get the Source interface index.
	 */
	if (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_SRC_INTERFACE_CHECK)) {
		/*
		 * Get the source interface index corresponding to flow.
		 * For bridge flow = Rx port no.
		 * For routed flow = ingress l3 if index.
		 */
		if (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_BRIDGE_FLOW)) {
			struct ppe_drv_iface *in_port_if = ppe_drv_v4_conn_flow_in_port_if_get(pcf);
			struct ppe_drv_port *port = ppe_drv_iface_port_get(in_port_if);
			if (port) {
				flow_cfg.src_intf_index = port->port;
				flow_cfg.src_intf_valid = A_TRUE;
				ppe_drv_trace("%p: Bridged flow, src_intf_index: %u", pcf, flow_cfg.src_intf_index);
			}
		} else {
			struct ppe_drv_iface *in_l3_if = ppe_drv_v4_conn_flow_in_l3_if_get(pcf);
			struct ppe_drv_l3_if *l3_if = ppe_drv_iface_l3_if_get(in_l3_if);
			if (l3_if) {
				flow_cfg.src_intf_index = l3_if->l3_if_index;
				flow_cfg.src_intf_valid = A_TRUE;
				ppe_drv_trace("%p: Routed flow, src_intf_index: %u", pcf, flow_cfg.src_intf_index);
			}
		}
	}
#endif

	/*
	 * Set forwarding type
	 */
	if (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLAG_FLOW_PPE_POLICER_ASSIST)) {
		flow_cfg.fwd_type = ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_BRIDGE_FLOW) ?
				    FAL_FLOW_BRIDGE: FAL_FLOW_ROUTE;
		ppe_drv_trace("%p: Policer enabled flow\n", pcf);
	} else if (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLAG_FLOW_RFS_PPE_ASSIST)) {
		flow_cfg.fwd_type = ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_BRIDGE_FLOW) ?
				    FAL_FLOW_BRIDGE: FAL_FLOW_ROUTE;
		ppe_drv_trace("%p: RFS enabled flow\n", pcf);
	} else if (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLAG_FLOW_PRIORITY_PPE_ASSIST)) {
		/*
		 * Case PPE is used only to Assist in priority marking of packets
		 */
		flow_cfg.fwd_type = FAL_FLOW_RDT_TO_CPU;
		ppe_drv_trace("%p: flow_tbl[fwd_type]: Priority Assist: %u", pcf, FAL_FLOW_FORWARD);
	} else if (ipv4_is_multicast(htonl(match_dest_ip))) {
		/*
		 * Multicast flow
		 */
		flow_cfg.fwd_type = FAL_FLOW_FORWARD;
		ppe_drv_trace("%p: flow_tbl[fwd_type]: L2-Multicast: %u", pcf, FAL_FLOW_FORWARD);
	} else if (match_src_ip != xlate_src_ip) {
		/*
		 * Case SNAT
		 */
		flow_cfg.fwd_type = FAL_FLOW_SNAT;
		flow_cfg.snat_nexthop = nh->index;
		flow_cfg.snat_srcport = ppe_drv_v4_conn_flow_xlate_src_ident_get(pcf);
		ppe_drv_trace("%p: flow_tbl[nat_type]: SNAT: %u snat_port: %u", pcf, FAL_FLOW_SNAT, flow_cfg.snat_srcport);
	} else if (match_dest_ip != xlate_dest_ip) {
		/*
		 * Case DNAT
		 */
		flow_cfg.fwd_type = FAL_FLOW_DNAT;
		flow_cfg.dnat_nexthop = nh->index;
		flow_cfg.dnat_dstport = ppe_drv_v4_conn_flow_xlate_dest_ident_get(pcf);
		ppe_drv_trace("%p: flow_tbl[nat_type]: DNAT: %u dnat_port: %u", pcf, FAL_FLOW_DNAT, flow_cfg.dnat_dstport);
	} else if (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_BRIDGE_FLOW)) {
		/*
		 * Case bridging
		 */
		flow_cfg.fwd_type = FAL_FLOW_BRIDGE;
		ppe_drv_trace("%p: flow_tbl[fwd_type]: L2: %u", pcf, FAL_FLOW_BRIDGE);

#ifdef NSS_PPE_IPQ53XX
		/*
		 * VLAN tag addition from NEXTHOP_TBL for flow based bridging.
		 * With VLAN tagging capability from flow table for bridge flows,
		 * we can avoid EG_VLAN_XLT_RULE based VLAN tagging. This would help
		 * doing VLANs between different ingress and egress VSI.
		 */
		if (nh) {
			flow_cfg.bridge_nexthop_valid = A_TRUE;
			flow_cfg.bridge_nexthop = nh->index;
			ppe_drv_trace("%p:nexthop index: %u", pcf, nh->index);
		}
#endif
		flow_cfg.port_valid = A_TRUE;
		flow_cfg.bridge_port = pp->port;
		ppe_drv_trace("%p: xmit interface port: %d", pcf, pp->port);

		if (ppe_drv_port_is_tunnel_vp(pp)) {
			switch (vlan_hdr_cnt) {
			case 2:
				flow_cfg.svlan_fmt = A_TRUE;
				flow_cfg.cvlan_fmt = A_TRUE;
				flow_cfg.vlan_fmt_valid = 1;
				break;
			case 1:
				flow_cfg.cvlan_fmt = A_TRUE;
				flow_cfg.vlan_fmt_valid = 1;
				break;
			case 0:
				break;
			default:
				ppe_drv_warn("%p: cannot accelerate bridge VLAN flows for more than 2 vlans: %u",
						pcf, vlan_hdr_cnt);
				return NULL;
			}
		}
	} else {
		/*
		 * Case simple routing (Non-NAT)
		 */
		flow_cfg.route_nexthop = nh->index;
		flow_cfg.fwd_type = FAL_FLOW_ROUTE;
		ppe_drv_trace("%p: flow_tbl[fwd_type]: L3: %u", pcf, FAL_FLOW_ROUTE);
	}

	/*
	 * Fill relevant details for 3-tuple and 5-tuple flows.
	 */
	switch (match_protocol) {
	case IPPROTO_TCP:
		flow_cfg.protocol = FAL_FLOW_PROTOCOL_TCP;
		ppe_drv_trace("%p: flow_tbl[protocol]: TCP-%u", pcf, FAL_FLOW_PROTOCOL_TCP);
		break;

	case IPPROTO_UDP:
		flow_cfg.protocol = FAL_FLOW_PROTOCOL_UDP;
		ppe_drv_trace("%p: flow_tbl[protocol]: UDP-%u", pcf, FAL_FLOW_PROTOCOL_UDP);
		break;

	case IPPROTO_UDPLITE:
		flow_cfg.protocol = FAL_FLOW_PROTOCOL_UDPLITE;
		ppe_drv_trace("%p: flow_tbl[protocol]: UDP-Lite-%u", pcf, FAL_FLOW_PROTOCOL_UDPLITE);
		break;

	case IPPROTO_ESP:
		tuple_3 = true;
		ppe_drv_trace("%p: flow_tbl[protocol]: Other-%u", pcf, FAL_FLOW_PROTOCOL_OTHER);
		break;

	case IPPROTO_IPIP:
		tuple_3 = true;
		ppe_drv_trace("%p: flow_tbl[protocol]: Other-%u", pcf, FAL_FLOW_PROTOCOL_OTHER);
		break;

	case IPPROTO_GRE:
		tuple_3 = true;
		ppe_drv_trace("%p: flow_tbl[protocol]: Other-%u", pcf, FAL_FLOW_PROTOCOL_OTHER);
		break;

	default:
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_unknown_proto);
		ppe_drv_warn("%p: protocol: %u cannot be offloaded to PPE", pcf, match_protocol);
		return NULL;
	}

	flow_cfg.flow_ip.ipv4 = match_dest_ip;
	ppe_drv_trace("%p: flow_tbl[dest_ip]: %pI4", pcf, &match_dest_ip);
	if (tuple_3) {
		flow_cfg.entry_type = FAL_FLOW_IP4_3TUPLE_ADDR;
		flow_cfg.protocol = FAL_FLOW_PROTOCOL_OTHER;
		flow_cfg.ip_type = match_protocol;
		ppe_drv_trace("%p: flow_tbl[ip_proto]: 0x%x", pcf, match_protocol);
	} else {
		flow_cfg.entry_type = FAL_FLOW_IP4_5TUPLE_ADDR;

		flow_cfg.src_port = ppe_drv_v4_conn_flow_match_src_ident_get(pcf);
		flow_cfg.dst_port = ppe_drv_v4_conn_flow_match_dest_ident_get(pcf);

		ppe_drv_trace("%p: flow_tbl[sport]: %u", pcf, flow_cfg.src_port);
		ppe_drv_trace("%p: flow_tbl[dport]: %u", pcf, flow_cfg.dst_port);

	}

	/*
	 * Fill Path-MTU.
	 *
	 * Note: For multicast flows, it should be ok to program the minimum MTU value
	 * of all the interfaces, since PPE also check MTU for each destination interface
	 * and exception the packet (without cloning) if MTU check fail for any interface.
	 */
	xmit_mtu = ipv4_is_multicast(htonl(match_dest_ip)) ? ppe_drv_v4_conn_flow_mc_min_mtu_get(pcf)
		: ppe_drv_v4_conn_flow_xmit_interface_mtu_get(pcf);
	if (xmit_mtu > PPE_DRV_PORT_JUMBO_MAX) {
		ppe_drv_trace("%p: xmit_mtu: %d is larger, restricting to max: %d", pcf, xmit_mtu, PPE_DRV_PORT_JUMBO_MAX);
		xmit_mtu = PPE_DRV_PORT_JUMBO_MAX;
	}

	flow_cfg.pmtu_check_l3 = (a_bool_t)PPE_DRV_FLOW_PMTU_TYPE_L3;
	flow_cfg.pmtu = xmit_mtu;

	ppe_drv_trace("%p: flow_tbl[PMTU]: %u", pcf, xmit_mtu);

	/*
	 * Add the flow
	 */
	err = fal_flow_entry_add(PPE_DRV_SWITCH_ID, FAL_FLOW_OP_MODE_KEY, &flow_cfg);
	if (err != SW_OK) {
		ppe_drv_trace("%p: flow entry add failed", pcf);
		return NULL;
	}

	/*
	 * Get the sw instance of flow entry.
	 */
	flow = &p->flow[flow_cfg.entry_id];
	ppe_drv_assert(!(flow->flags & PPE_DRV_FLOW_VALID), "%p: flow entry is already accelerated to PPE at index: %d",
			pcf, flow_cfg.entry_id);

	/*
	 * Increment flow count if SAWF service is configured in tree_id.
	 */
	if (ppe_drv_tree_id_type_get(&pcf->flow_metadata) == PPE_DRV_TREE_ID_TYPE_SAWF) {
		service_class = tree_id_data->info.sawf_metadata.service_class;
		if (PPE_DRV_SERVICE_CLASS_IS_VALID(service_class)) {
			sawf_sc_stats = &p->stats.sawf_sc_stats[service_class];
			ppe_drv_stats_inc(&sawf_sc_stats->flow_count);
			ppe_drv_trace("Stats Updated: service class %u : flow  %llu\n", service_class, atomic64_read(&sawf_sc_stats->flow_count));
		}
	}

	/*
	 * Update the shadow copy
	 */
	flow->host = host;
	flow->nh = nh;
	flow->flags |= PPE_DRV_FLOW_V4;
	flow->type = PPE_DRV_IP_TYPE_V4;
	flow->entry_type = flow_cfg.entry_type;
	flow->pcf.v4 = pcf;
	ppe_drv_trace("%p: flow_tbl entry added at index: %u", pcf, flow_cfg.entry_id);
	return flow;
}

/*
 * ppe_drv_flow_v4_attach_mapt_v6_conn
 *	Attach v6 flow entry to the corresponding v4 flow entry for mapt.
 */
bool ppe_drv_flow_v4_attach_mapt_v6_conn(struct ppe_drv_v4_conn_flow *pcf_v4, struct ppe_drv_v6_conn_flow *pcf_v6, uint8_t length_adjust)
{
	struct ppe_drv_flow *pf = pcf_v4->pf;
	struct ppe_drv_flow_mapt_info *mapt_info = &pf->mapt_info;

	if (!(pf->flags & PPE_DRV_FLOW_V4)) {
		ppe_drv_trace("%p: flow is invalid", pf);
		return false;
	}

	mapt_info->mapt_v6 = pcf_v6;
	mapt_info->len_adjust = length_adjust;
	pf->flags |= PPE_DRV_FLOW_MAPT;

	return true;
}

/*
 * ppe_drv_flow_v4_detach_attach_mapt_v6_conn
 *	detach v6 flow entry to the corresponding v4 flow entry for mapt.
 */
bool ppe_drv_flow_v4_detach_mapt_v6_conn(struct ppe_drv_v4_conn_flow *pcf_v4)
{
	struct ppe_drv_flow *pf = pcf_v4->pf;
	struct ppe_drv_flow_mapt_info *mapt_info;

	if (!pf || !(pf->flags & PPE_DRV_FLOW_MAPT)) {
		ppe_drv_warn("%p: flow is invalid", pcf_v4);
		return false;
	}

	mapt_info = &pf->mapt_info;
	pf->flags &= ~PPE_DRV_FLOW_MAPT;
	mapt_info->mapt_v6 = NULL;
	mapt_info->len_adjust = 0;

	return true;
}

/*
 * ppe_drv_flow_entries_free()
 *	Free flow table entries if it was allocated.
 */
void ppe_drv_flow_entries_free(struct ppe_drv_flow *flow)
{
	vfree(flow);
}

/*
 * ppe_drv_flow_entries_alloc()
 *	Allocated and initialize requested number of flow table entries.
 */
struct ppe_drv_flow *ppe_drv_flow_entries_alloc()
{
	uint16_t i;
	struct ppe_drv_flow *flow;
	struct ppe_drv *p = &ppe_drv_gbl;

	flow = vzalloc(sizeof(struct ppe_drv_flow) * p->flow_num);
	if (!flow) {
		ppe_drv_warn("%p: failed to allocate flow entries", p);
		return NULL;
	}

	/*
	 * Assign flow index values to the flow entries
	 */
	for (i = 0; i < p->flow_num; i++) {
		flow[i].index = i;
	}

	return flow;
}
