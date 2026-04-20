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

#include <linux/if_vlan.h>
#include "ppe_drv.h"
#include "tun/ppe_drv_tun.h"
#include "tun/ppe_drv_tun_v6.h"

/*
 * ppe_drv_v6_bind_acl_policer()
 *	Map ACL/POLICER ID to service code.
 */
static bool ppe_drv_v6_bind_acl_policer(struct ppe_drv_v6_rule_create *create, struct ppe_drv_v6_conn *cn)
{
	struct ppe_drv_acl_policer_rule *ap_rule = &create->ap_rule;
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_acl *acl = p->acl;
	struct ppe_drv_acl_flow_bind info = {0};
	struct ppe_drv_comm_stats *comm_stats = &p->stats.comm_stats[PPE_DRV_CONN_TYPE_FLOW];
	struct ppe_drv_policer_ctx *ctx = p->pol_ctx;
	struct ppe_drv_policer_flow policer_info = {0};

	if (!(create->valid_flags & PPE_DRV_V6_VALID_FLAG_ACL_POLICER)) {
		return true;
	}

	switch (ap_rule->type) {
		case PPE_DRV_RULE_TYPE_FLOW_ACL:
			if (!acl->flow_add_cb) {
				ppe_drv_trace("%p: No callback registered for acl:%p\n", p, acl);
				return true;
			}

			if (ap_rule->rule_id.acl.flags & PPE_DRV_VALID_FLAG_FLOW_ACL) {
				info.id = ap_rule->rule_id.acl.flow_acl_id;
				if (!acl->flow_add_cb(acl->flow_app_data, &info)) {
					ppe_drv_warn("%p: invalid rule_id or no valid sc found for rule_id: %d",
							p, info.id);
					ppe_drv_stats_inc(&comm_stats->v6_create_fail_acl);
					return false;
				}

				cn->pcf.acl_sc = info.sc;
				cn->pcf.acl_id = info.id;
				ppe_drv_v6_conn_flow_flags_set(&cn->pcf, PPE_DRV_V6_CONN_FLAG_FLOW_ACL_VALID);
				ppe_drv_info("%p: using sc: %d for rule_id: %d", p, info.sc, info.id);
			}

			if (ap_rule->rule_id.acl.flags & PPE_DRV_VALID_FLAG_RETURN_ACL) {
				info.id = ap_rule->rule_id.acl.return_acl_id;
				if (!acl->flow_add_cb(acl->flow_app_data, &info)) {
					ppe_drv_warn("%p: invalid rule_id or no valid sc found for rule_id: %d",
							p, info.id);
					ppe_drv_stats_inc(&comm_stats->v6_create_fail_acl);
					return false;
				}

				cn->pcr.acl_sc = info.sc;
				cn->pcr.acl_id = info.id;
				ppe_drv_v6_conn_flow_flags_set(&cn->pcr, PPE_DRV_V6_CONN_FLAG_FLOW_ACL_VALID);
				ppe_drv_info("%p: using sc: %d for rule_id: %d", p, info.sc, info.id);
			}

			break;

		case PPE_DRV_RULE_TYPE_FLOW_POLICER:
			if (!ctx->flow_add_cb) {
				ppe_drv_trace("%p: No callback registered for policer:%p\n", p, ctx);
				return true;
			}

			if (ap_rule->rule_id.policer.flags & PPE_DRV_VALID_FLAG_FLOW_POLICER) {
				policer_info.id = ap_rule->rule_id.policer.flow_policer_id;
				if (ap_rule->pkt_noedit) {
					policer_info.pkt_noedit = true;
				}

				if (!ctx->flow_add_cb(ctx->flow_app_data, &policer_info)) {
					ppe_drv_trace("%p: no valid sc found for rule_id: %d", p, policer_info.id);
					return false;
				}

				cn->pcr.acl_sc = PPE_DRV_SC_NONE;
				if (policer_info.sc_valid) {
					cn->pcf.acl_sc = policer_info.sc;
				} else if (ap_rule->pkt_noedit) {
					cn->pcf.acl_sc = PPE_DRV_SC_NOEDIT_ACL_POLICER;
				}

				cn->pcf.policer_id = ap_rule->rule_id.policer.flow_policer_id;
				cn->pcf.policer_hw_id = ppe_drv_policer_user2hw_id(policer_info.id);
				ppe_drv_v6_conn_flow_flags_set(&cn->pcf, PPE_DRV_V6_CONN_FLAG_FLOW_POLICER_VALID);
			}

			if (ap_rule->rule_id.policer.flags & PPE_DRV_VALID_FLAG_RETURN_POLICER) {
				policer_info.id = ap_rule->rule_id.policer.return_policer_id;
				if (ap_rule->pkt_noedit) {
					policer_info.pkt_noedit = true;
				}

				if (!ctx->flow_add_cb(ctx->flow_app_data, &policer_info)) {
					ppe_drv_trace("%p: no valid sc found for rule_id: %d", p, policer_info.id);
					return false;
				}

				cn->pcr.acl_sc = PPE_DRV_SC_NONE;
				if (policer_info.sc_valid) {
					cn->pcr.acl_sc = policer_info.sc;
				} else if (ap_rule->pkt_noedit) {
					cn->pcr.acl_sc = PPE_DRV_SC_NOEDIT_ACL_POLICER;
				}

				cn->pcr.policer_id = ap_rule->rule_id.policer.return_policer_id;
				cn->pcr.policer_hw_id = ppe_drv_policer_user2hw_id(policer_info.id);
				ppe_drv_v6_conn_flow_flags_set(&cn->pcr, PPE_DRV_V6_CONN_FLAG_FLOW_POLICER_VALID);
				ppe_drv_trace("%p: using sc: %d for rule_id: %d", p, policer_info.sc, policer_info.id);
			}

			break;
	}

	return true;
}

/*
 * ppe_drv_v6_unbind_acl_policer()
 *	Unbind ACL ID from service code.
 */
static bool ppe_drv_v6_unbind_acl_policer(struct ppe_drv_v6_conn *cn)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_acl *acl = p->acl;
	struct ppe_drv_acl_flow_bind info = {0};
	struct ppe_drv_comm_stats *comm_stats = &p->stats.comm_stats[PPE_DRV_CONN_TYPE_FLOW];
	struct ppe_drv_policer_ctx *ctx = p->pol_ctx;
	struct ppe_drv_policer_flow policer_info = {0};

	if (ppe_drv_v6_conn_flow_flags_check(&cn->pcf, PPE_DRV_V6_CONN_FLAG_FLOW_ACL_VALID)) {
		info.id = cn->pcf.acl_id;
		if (!acl->flow_del_cb(acl->flow_app_data, &info)) {
			ppe_drv_warn("%p: no valid rule found for rule_id: %d", p, info.id);
			ppe_drv_stats_inc(&comm_stats->v6_destroy_fail_acl);
			return false;
		}

		cn->pcf.acl_sc = 0;
		ppe_drv_v6_conn_flow_flags_clear(&cn->pcf, PPE_DRV_V6_CONN_FLAG_FLOW_ACL_VALID);
		ppe_drv_info("%p: unlinking flow from ACL rule_id: %d", p, info.id);
	}

	if (ppe_drv_v6_conn_flow_flags_check(&cn->pcr, PPE_DRV_V6_CONN_FLAG_FLOW_ACL_VALID)) {
		info.id = cn->pcr.acl_id;
		if (!acl->flow_del_cb(acl->flow_app_data, &info)) {
			ppe_drv_warn("%p: no valid rule found for rule_id: %d", p, info.id);
			ppe_drv_stats_inc(&comm_stats->v6_destroy_fail_acl);
			return false;
		}

		cn->pcr.acl_sc = 0;
		ppe_drv_v6_conn_flow_flags_clear(&cn->pcr, PPE_DRV_V6_CONN_FLAG_FLOW_ACL_VALID);
		ppe_drv_info("%p: Unlinking flow from ACL rule_id: %d", p, info.id);
	}

	if (ppe_drv_v6_conn_flow_flags_check(&cn->pcf, PPE_DRV_V6_CONN_FLAG_FLOW_POLICER_VALID)) {
		policer_info.id = cn->pcf.policer_id;
		if (!ctx->flow_del_cb(ctx->flow_app_data, &policer_info)) {
			ppe_drv_warn("%p: no valid rule found for rule_id: %d", p, policer_info.id);
			return false;
		}

		cn->pcf.acl_sc = 0;
		ppe_drv_v6_conn_flow_flags_clear(&cn->pcf, PPE_DRV_V6_CONN_FLAG_FLOW_POLICER_VALID);
		ppe_drv_info("%p: unlinking flow from POLICER rule_id: %d", p, policer_info.id);
	}

	if (ppe_drv_v6_conn_flow_flags_check(&cn->pcr, PPE_DRV_V6_CONN_FLAG_FLOW_POLICER_VALID)) {
		policer_info.id = cn->pcr.policer_id;
		if (!ctx->flow_del_cb(ctx->flow_app_data, &policer_info)) {
			ppe_drv_warn("%p: no valid rule found for rule_id: %d", p, policer_info.id);
			return false;
		}

		cn->pcr.acl_sc = 0;
		ppe_drv_v6_conn_flow_flags_clear(&cn->pcr, PPE_DRV_V6_CONN_FLAG_FLOW_POLICER_VALID);
		ppe_drv_info("%p: Unlinking flow from POLICER rule_id: %d", p, policer_info.id);
	}

	return true;
}

/*
 * ppe_drv_fill_fse_v6_tuple_info()
 *	Fill FSE v6 tuple information
 */
void ppe_drv_fill_fse_v6_tuple_info(struct ppe_drv_v6_conn_flow *conn, struct ppe_drv_fse_rule_info *fse_info, bool is_ds)
{
	struct ppe_drv_port *pp;

	ppe_drv_v6_conn_flow_match_src_ip_get(conn, &fse_info->tuple.src_ip[0]);
	fse_info->tuple.src_port = ppe_drv_v6_conn_flow_match_src_ident_get(conn);
	ppe_drv_v6_conn_flow_match_dest_ip_get(conn, &fse_info->tuple.dest_ip[0]);
	fse_info->tuple.dest_port = ppe_drv_v6_conn_flow_match_dest_ident_get(conn);
	fse_info->tuple.protocol = ppe_drv_v6_conn_flow_match_protocol_get(conn);
	fse_info->flags |= PPE_DRV_FSE_IPV6;
	if (is_ds) {
		fse_info->flags |= PPE_DRV_FSE_DS;
	}

	pp = ppe_drv_v6_conn_flow_rx_port_get(conn);

	fse_info->dev = ppe_drv_port_to_dev(pp);
	fse_info->vp_num = pp->port;
}

/*
 * ppe_drv_v6_fse_interface_check()
 *	check if interface is FSE capable
 */
bool ppe_drv_v6_fse_interface_check(struct ppe_drv_v6_conn_flow *pcf)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_port *rx_port = ppe_drv_v6_conn_flow_rx_port_get(pcf);
	struct ppe_drv_port *tx_port = ppe_drv_v6_conn_flow_tx_port_get(pcf);
	bool is_tx_ds = (tx_port->user_type == PPE_DRV_PORT_USER_TYPE_DS);
	bool is_rx_ds = (rx_port->user_type == PPE_DRV_PORT_USER_TYPE_DS);
	bool is_tx_active_vp = (tx_port->user_type == PPE_DRV_PORT_USER_TYPE_ACTIVE_VP);
	bool is_rx_active_vp = (rx_port->user_type == PPE_DRV_PORT_USER_TYPE_ACTIVE_VP);

	/*
	 * If FSE operation is not enabled; return true and continue with a successfull
	 * PPE rule push
	 */
	if (!p->is_wifi_fse_up || !p->fse_enable || !p->fse_ops) {
		ppe_drv_trace("FSE operation not enabled: enable: %d ops: %p\n", p->fse_enable, p->fse_ops);
		return false;
	}

	/*
	 * If interfaces are not Wi-FI VPs then return true and continue with a successfull
	 * PPE rule push
	 */
	if (!is_tx_ds && !is_rx_ds && !is_tx_active_vp && !is_rx_active_vp) {
		ppe_drv_trace("no active or ds vp\n");
		return false;
	}

	return true;
}

void ppe_drv_v6_flow_vlan_set(struct ppe_drv_v6_conn_flow *pcf,
			      uint32_t primary_ingress_vlan_tag, uint32_t primary_egress_vlan_tag,
			      uint32_t secondary_ingress_vlan_tag, uint32_t secondary_egress_vlan_tag)
{
	if ((primary_ingress_vlan_tag & PPE_DRV_VLAN_ID_MASK) != PPE_DRV_VLAN_NOT_CONFIGURED) {
		pcf->ingress_vlan[0].tpid = primary_ingress_vlan_tag >> 16;
		pcf->ingress_vlan[0].tci = (uint16_t) primary_ingress_vlan_tag;
		pcf->ingress_vlan_cnt++;

		/*
		 * Check if flow needs 802.1p marking.
		 */
		if (pcf->ingress_vlan[0].tci & PPE_DRV_VLAN_PRIORITY_MASK) {
			ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_VLAN_PRI_MARKING);
		}
	}

	if ((secondary_ingress_vlan_tag & PPE_DRV_VLAN_ID_MASK) != PPE_DRV_VLAN_NOT_CONFIGURED) {
		pcf->ingress_vlan[1].tpid = secondary_ingress_vlan_tag >> 16;
		pcf->ingress_vlan[1].tci = (uint16_t) secondary_ingress_vlan_tag;
		pcf->ingress_vlan_cnt++;

		/*
		 * Check if flow needs 802.1p marking.
		 */
		if (pcf->ingress_vlan[1].tci & PPE_DRV_VLAN_PRIORITY_MASK) {
			ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_VLAN_PRI_MARKING);
		}
	}

	if ((primary_egress_vlan_tag & PPE_DRV_VLAN_ID_MASK) != PPE_DRV_VLAN_NOT_CONFIGURED) {
		pcf->egress_vlan[0].tpid = primary_egress_vlan_tag >> 16;
		pcf->egress_vlan[0].tci = (uint16_t) primary_egress_vlan_tag;
		pcf->egress_vlan_cnt++;

		/*
		 * Check if flow needs 802.1p marking.
		 */
		if (pcf->egress_vlan[0].tci & PPE_DRV_VLAN_PRIORITY_MASK) {
			ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_VLAN_PRI_MARKING);
		}
	}

	if ((secondary_egress_vlan_tag & PPE_DRV_VLAN_ID_MASK) != PPE_DRV_VLAN_NOT_CONFIGURED) {
		pcf->egress_vlan[1].tpid = ((secondary_egress_vlan_tag & PPE_DRV_VLAN_TPID_MASK) >> 16);
		pcf->egress_vlan[1].tci = (secondary_egress_vlan_tag & PPE_DRV_VLAN_TCI_MASK);
		pcf->egress_vlan_cnt++;

		/*
		 * Check if flow needs 802.1p marking.
		 */
		if (pcf->egress_vlan[1].tci & PPE_DRV_VLAN_PRIORITY_MASK) {
			ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_VLAN_PRI_MARKING);
		}
	}
}

/*
 * ppe_drv_v6_rfs_conn_fill()
 *	Populate single direction flow object rule.
 */
ppe_drv_ret_t ppe_drv_v6_rfs_conn_fill(struct ppe_drv_v6_rule_create *create,  struct ppe_drv_top_if_rule *top_if,
				       struct ppe_drv_v6_conn *cn, enum ppe_drv_conn_type flow_type)
{
	struct ppe_drv_v6_connection_rule *conn = &create->conn_rule;
	struct ppe_drv_v6_5tuple *tuple = &create->tuple;
	struct ppe_drv_iface *if_rx, *if_tx, *top_rx_iface;
	struct ppe_drv_v6_conn_flow *pcf = &cn->pcf;
	uint16_t rule_flags = create->rule_flags;
	struct ppe_drv_comm_stats *comm_stats;
	struct ppe_drv_port *pp_rx, *pp_tx;
	struct ppe_drv *p = &ppe_drv_gbl;

	comm_stats = &p->stats.comm_stats[flow_type];

	/*
	 * Make sure both Rx and Tx inteface are mapped to PPE ports properly.
	 */
	if_rx = ppe_drv_iface_get_by_idx(conn->rx_if);
	if (!if_rx) {
		ppe_drv_stats_inc(&comm_stats->v6_create_rfs_fail_invalid_rx_if);
		ppe_drv_warn("%p: No PPE interface corresponding to rx_if: %d", create, conn->rx_if);
		return PPE_DRV_RET_FAILURE_INVALID_PARAM;
	}

	pp_rx = ppe_drv_iface_port_get(if_rx);
	if (!pp_rx) {
		ppe_drv_stats_inc(&comm_stats->v6_create_rfs_fail_invalid_rx_port);
		ppe_drv_warn("%p: Invalid Rx IF: %d", create, conn->rx_if);
		return PPE_DRV_RET_FAILURE_IFACE_PORT_MAP;
	}

	if_tx = ppe_drv_iface_get_by_idx(conn->tx_if);
	if (!if_tx) {
		ppe_drv_stats_inc(&comm_stats->v6_create_rfs_fail_invalid_tx_if);
		ppe_drv_warn("%p: No PPE interface corresponding to tx_if: %d", create, conn->tx_if);
		return PPE_DRV_RET_FAILURE_INVALID_PARAM;
	}

	pp_tx = ppe_drv_iface_port_get(if_tx);
	if (!pp_tx) {
		ppe_drv_stats_inc(&comm_stats->v6_create_rfs_fail_invalid_tx_port);
		ppe_drv_warn("%p: Invalid Tx IF: %d", create, conn->tx_if);
		return PPE_DRV_RET_FAILURE_IFACE_PORT_MAP;
	}

	/*
	 * Check if the PPE offload is disabled on the the physical Rx port
	 */
	if ((pp_tx->user_type == PPE_DRV_PORT_USER_TYPE_ACTIVE_VP) ||
			(pp_tx->user_type == PPE_DRV_PORT_USER_TYPE_DS)) {
		if (!ppe_drv_port_check_flow_offload_enabled(pp_rx)) {
			ppe_drv_stats_inc(&comm_stats->v6_create_offload_disabled);
			ppe_drv_v6_conn_flow_flags_set(pcf,
					PPE_DRV_V6_CONN_FLAG_FLOW_OFFLOAD_DISABLED);
		}
	}

	top_rx_iface = ppe_drv_iface_get_by_idx(top_if->rx_if);
	if (!top_rx_iface) {
		ppe_drv_warn("%p: No PPE interface corresponding to top rx interface\n", p);
		return PPE_DRV_RET_NO_TOP_RX_IF;
	}

	if (ppe_drv_iface_l3_if_get(top_rx_iface)) {
		ppe_drv_trace("%p: Using top rx iface's l3 if for dev: %s\n", top_rx_iface, top_rx_iface->dev->name);
		ppe_drv_v6_conn_flow_in_l3_if_set_and_ref(pcf, top_rx_iface);
	} else {
		ppe_drv_trace("%p: Using port's l3 if for dev: %s\n", top_rx_iface, top_rx_iface->dev->name);
		ppe_drv_v6_conn_flow_in_l3_if_set_and_ref(pcf, if_rx);
	}

	/*
	 * Set the egress point based on direction of the flow
	 * TODO: Handle the else case and add error counter for it
	 */
	if ((pp_tx->flags & PPE_DRV_PORT_RFS_ENABLED) && ppe_drv_is_wlan_vp_port_type(pp_tx->user_type)) {
		pcf->eg_port_if = ppe_drv_iface_ref(if_tx);
	} else if ((pp_rx->flags & PPE_DRV_PORT_RFS_ENABLED) && ppe_drv_is_wlan_vp_port_type(pp_rx->user_type)) {
		pcf->eg_port_if = ppe_drv_iface_ref(if_rx);
	}

	/*
	 * Bridge flow
	 */
	if (rule_flags & PPE_DRV_V6_RULE_FLAG_BRIDGE_FLOW) {
		ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_BRIDGE_FLOW);
	}

	ppe_drv_v6_conn_flow_conn_set(pcf, cn);

	/*
	 * Set Rx and Tx port.
	 */
	ppe_drv_v6_conn_flow_rx_port_set(pcf, pp_rx);
	ppe_drv_v6_conn_flow_tx_port_set(pcf, pp_tx);

	/*
	 * Set 5-tuple.
	 */
	ppe_drv_v6_conn_flow_match_protocol_set(pcf, tuple->protocol);
	ppe_drv_v6_conn_flow_match_src_ip_set(pcf, tuple->flow_ip);
	ppe_drv_v6_conn_flow_match_src_ident_set(pcf, tuple->flow_ident);
	ppe_drv_v6_conn_flow_match_dest_ip_set(pcf, tuple->return_ip);
	ppe_drv_v6_conn_flow_match_dest_ident_set(pcf, tuple->return_ident);

	/*
	 * Host order IP addr.
	 */
	ppe_drv_v6_conn_flow_dump_match_src_ip_set(pcf, pcf->match_src_ip);
	ppe_drv_v6_conn_flow_dump_match_dest_ip_set(pcf, pcf->match_dest_ip);

	/*
	 * Flow MTU and transmit MAC address.
	 */
	ppe_drv_v6_conn_flow_xmit_interface_mtu_set(pcf, conn->flow_mtu);

	return PPE_DRV_RET_SUCCESS;
}

/*
 * ppe_drv_v6_priority_conn_fill()
 *	Populate single direction flow object rule.
 */
ppe_drv_ret_t ppe_drv_v6_priority_conn_fill(struct ppe_drv_v6_rule_create *create, struct ppe_drv_v6_conn *cn,
				   enum ppe_drv_conn_type flow_type)
{
	struct ppe_drv_v6_connection_rule *conn = &create->conn_rule;
	struct ppe_drv_v6_5tuple *tuple = &create->tuple;
	struct ppe_drv_v6_conn_flow *pcf = &cn->pcf;
	struct ppe_drv_comm_stats *comm_stats;
	struct ppe_drv *p = &ppe_drv_gbl;

	comm_stats = &p->stats.comm_stats[flow_type];

	ppe_drv_v6_conn_flow_conn_set(pcf, cn);

	/*
	 * Set 5-tuple.
	 */
	ppe_drv_v6_conn_flow_match_protocol_set(pcf, tuple->protocol);
	ppe_drv_v6_conn_flow_match_src_ip_set(pcf, tuple->flow_ip);
	ppe_drv_v6_conn_flow_match_src_ident_set(pcf, tuple->flow_ident);
	ppe_drv_v6_conn_flow_match_dest_ip_set(pcf, tuple->return_ip);
	ppe_drv_v6_conn_flow_match_dest_ident_set(pcf, tuple->return_ident);

	/*
	 * Host order IP addr.
	 */
	ppe_drv_v6_conn_flow_dump_match_src_ip_set(pcf, pcf->match_src_ip);
	ppe_drv_v6_conn_flow_dump_match_dest_ip_set(pcf, pcf->match_dest_ip);

	/*
	 * Set flow MTU.
	 */
	ppe_drv_v6_conn_flow_xmit_interface_mtu_set(pcf, conn->flow_mtu);

	ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_PRIORITY_PPE_ASSIST);
	ppe_drv_v6_conn_flow_int_pri_set(pcf, create->qos_rule.flow_qos_tag);
	ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_QOS_VALID);

	return PPE_DRV_RET_SUCCESS;
}


/*
 * ppe_drv_v6_conn_flow_metadata_set()
 *	Sets metadata associated with flow.
 */
static inline void ppe_drv_v6_conn_flow_metadata_set(struct ppe_drv_v6_conn_flow *pcf, struct ppe_drv_flow_cookie_metadata *fc_metadata, ppe_drv_tree_id_type_t tree_id_type)
{
	switch (tree_id_type) {
	case PPE_DRV_TREE_ID_TYPE_NONE:
		pcf->flow_metadata.tree_id_data.type = PPE_DRV_TREE_ID_TYPE_NONE;
		pcf->flow_metadata.tree_id_data.info.value = fc_metadata->type.mark;
		return;

	case PPE_DRV_TREE_ID_TYPE_SAWF:
		pcf->flow_metadata.wifi_qos = PPE_DRV_SAWF_MSDUQ_GET(fc_metadata->type.sawf.sawf_mark);
		pcf->flow_metadata.tree_id_data.type = PPE_DRV_TREE_ID_TYPE_SAWF;
		pcf->flow_metadata.tree_id_data.info.sawf_metadata.service_class = fc_metadata->type.sawf.service_class;
		pcf->flow_metadata.tree_id_data.info.sawf_metadata.peer_id = PPE_DRV_SAWF_PEER_ID_GET(fc_metadata->type.sawf.sawf_mark);
		return;

	case PPE_DRV_TREE_ID_TYPE_SCS:
		pcf->flow_metadata.wifi_qos = PPE_DRV_SAWF_MSDUQ_GET(fc_metadata->type.scs.scs_mark);
                pcf->flow_metadata.tree_id_data.type = PPE_DRV_TREE_ID_TYPE_SCS;
		return;

	case PPE_DRV_TREE_ID_TYPE_WIFI_TID:
                pcf->flow_metadata.tree_id_data.type = PPE_DRV_TREE_ID_TYPE_WIFI_TID;
		pcf->flow_metadata.wifi_qos = fc_metadata->type.mark;
		return;

	case PPE_DRV_TREE_ID_TYPE_MLO_ASSIST:
		pcf->flow_metadata.tree_id_data.type = PPE_DRV_TREE_ID_TYPE_MLO_ASSIST;
		pcf->flow_metadata.wifi_qos = PPE_DRV_MLO_MSDUQ_GET(fc_metadata->type.mark);
		pcf->flow_metadata.tree_id_data.info.value = PPE_DRV_MLO_MARK_GET(fc_metadata->type.mark);
		return;

	default:
		ppe_drv_trace("Invalid tree_id type : (%u)", tree_id_type);
		return;
	}
}

#ifdef PPE_DRV_FLOW_IG_MAC_WAR
/*
 * ppe_drv_v6_conn_flow_igmac_add()
 *	Ingress mac address add API
 */
static bool ppe_drv_v6_conn_flow_igmac_add(struct ppe_drv_v6_conn_flow *pcf)
{
	struct ppe_drv_iface *in_l3_if;

	/*
	 * ingress mac configuration needed only for routed flows.
	 */
	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_BRIDGE_FLOW)) {
		return true;
	}

	/*
	 * If no ingress L3_IF, no mac address configuration needed.
	 */
	in_l3_if = ppe_drv_v6_conn_flow_in_l3_if_get(pcf);
	if (!in_l3_if) {
		return true;
	}

	/*
	 * MY_MAC address is needed only if the ingress l3_if mac address
	 * is different than rx port mac address.
	 */
	if (in_l3_if->l3 && in_l3_if->l3->is_eg_mac_set
		&& pcf->rx_port && pcf->rx_port->mac_valid) {
		if (!memcmp(in_l3_if->l3->eg_mac_addr, pcf->rx_port->mac_addr, ETH_ALEN)) {
			ppe_drv_info("%p: same mac address for ingress l3 and port mac: %pM",
					pcf, pcf->rx_port->mac_addr);
			return true;
		}
	}

	if (in_l3_if->l3 && !ppe_drv_l3_if_ig_mac_add_and_ref(in_l3_if->l3)) {
		ppe_drv_warn("%p: failed to allocate MY_MAC entries for l3_if: %p",
				pcf, in_l3_if->l3);
		return false;
	}

	ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_IGMAC_VALID);
	return true;
}

/*
 * ppe_drv_v6_conn_flow_igmac_del()
 *	Ingress mac address delete API.
 */
static bool ppe_drv_v6_conn_flow_igmac_del(struct ppe_drv_v6_conn_flow *pcf)
{
	struct ppe_drv_iface *in_l3_if;

	/*
	 * ingress mac configuration needed only for routed flows.
	 */
	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_BRIDGE_FLOW)) {
		return true;
	}

	/*
	 * If no ingress L3_IF, no mac address deletion needed.
	 */
	in_l3_if = ppe_drv_v6_conn_flow_in_l3_if_get(pcf);
	if (!in_l3_if || !in_l3_if->l3) {
		return true;
	}

	if (!ppe_drv_l3_if_ig_mac_deref(in_l3_if->l3)) {
		ppe_drv_warn("%p: failed to deref MY_MAC entries for l3_if: %p",
				pcf, in_l3_if->l3);
		return false;
	}

	ppe_drv_v6_conn_flow_flags_clear(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_IGMAC_VALID);
	return true;
}
#endif

/*
 * ppe_drv_v6_policer_conn_fill()
 *	Populate single direction flow object rule.
 */
ppe_drv_ret_t ppe_drv_v6_policer_conn_fill(struct ppe_drv_v6_rule_create *create, struct ppe_drv_top_if_rule *top_if,
				       struct ppe_drv_v6_conn *cn, enum ppe_drv_conn_type flow_type)
{
	struct ppe_drv_v6_connection_rule *conn = &create->conn_rule;
	struct ppe_drv_v6_5tuple *tuple = &create->tuple;
	struct ppe_drv_iface *if_rx, *if_tx, *top_rx_iface;
	struct ppe_drv_v6_conn_flow *pcf = &cn->pcf;
	struct ppe_drv_v6_conn_flow *pcr = &cn->pcr;
	uint16_t rule_flags = create->rule_flags;
	struct ppe_drv_comm_stats *comm_stats;
	struct ppe_drv_port *pp_rx, *pp_tx;
	struct ppe_drv *p = &ppe_drv_gbl;

	comm_stats = &p->stats.comm_stats[flow_type];

	/*
	 * Make sure both Rx and Tx inteface are mapped to PPE ports properly.
	 */
	if_rx = ppe_drv_iface_get_by_idx(conn->rx_if);
	if (!if_rx) {
		ppe_drv_stats_inc(&comm_stats->v6_create_policer_fail_invalid_rx_if);
		ppe_drv_warn("%p: No PPE interface corresponding to rx_if: %d", create, conn->rx_if);
		return PPE_DRV_RET_FAILURE_INVALID_PARAM;
	}

	pp_rx = ppe_drv_iface_port_get(if_rx);
	if (!pp_rx) {
		ppe_drv_stats_inc(&comm_stats->v6_create_policer_fail_invalid_rx_port);
		ppe_drv_warn("%p: Invalid Rx IF: %d", create, conn->rx_if);
		return PPE_DRV_RET_FAILURE_IFACE_PORT_MAP;
	}

	if_tx = ppe_drv_iface_get_by_idx(conn->tx_if);
	if (!if_tx) {
		ppe_drv_stats_inc(&comm_stats->v6_create_policer_fail_invalid_tx_if);
		ppe_drv_warn("%p: No PPE interface corresponding to tx_if: %d", create, conn->tx_if);
		return PPE_DRV_RET_FAILURE_INVALID_PARAM;
	}

	pp_tx = ppe_drv_iface_port_get(if_tx);
	if (!pp_tx) {
		ppe_drv_stats_inc(&comm_stats->v6_create_policer_fail_invalid_tx_port);
		ppe_drv_warn("%p: Invalid Tx IF: %d", create, conn->tx_if);
		return PPE_DRV_RET_FAILURE_IFACE_PORT_MAP;
	}

	top_rx_iface = ppe_drv_iface_get_by_idx(top_if->rx_if);
	if (!top_rx_iface) {
		ppe_drv_warn("%p: No PPE interface corresponding to top rx interface\n", p);
		return PPE_DRV_RET_NO_TOP_RX_IF;
	}

	if (ppe_drv_iface_l3_if_get(top_rx_iface)) {
		ppe_drv_trace("%p: Using top rx iface's l3 if for dev: %s\n", top_rx_iface, top_rx_iface->dev->name);
		ppe_drv_v6_conn_flow_in_l3_if_set_and_ref(pcf, top_rx_iface);
	} else {
		ppe_drv_trace("%p: Using port's l3 if for dev: %s\n", top_rx_iface, top_rx_iface->dev->name);
		ppe_drv_v6_conn_flow_in_l3_if_set_and_ref(pcf, if_rx);
	}

	/*
	 * Bridge flow
	 */
	if (rule_flags & PPE_DRV_V6_RULE_FLAG_BRIDGE_FLOW) {
		ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_BRIDGE_FLOW);
		ppe_drv_v6_conn_flow_flags_set(pcr, PPE_DRV_V6_CONN_FLOW_FLAG_BRIDGE_FLOW);
	}

	/*
	 * Prepare flow direction rule
	 */
	if (rule_flags & PPE_DRV_V6_RULE_FLAG_FLOW_VALID) {
		ppe_drv_v6_conn_flow_conn_set(pcf, cn);

		/*
		 * Set Rx and Tx port.
		 */
		ppe_drv_v6_conn_flow_rx_port_set(pcf, pp_rx);
		ppe_drv_v6_conn_flow_tx_port_set(pcf, pp_tx);

		/*
		 * Set 5-tuple along with SNAT/DNAT requirement.
		 */
		ppe_drv_v6_conn_flow_match_protocol_set(pcf, tuple->protocol);
		ppe_drv_v6_conn_flow_match_src_ip_set(pcf, tuple->flow_ip);
		ppe_drv_v6_conn_flow_match_src_ident_set(pcf, tuple->flow_ident);
		ppe_drv_v6_conn_flow_match_dest_ip_set(pcf, tuple->return_ip);
		ppe_drv_v6_conn_flow_match_dest_ident_set(pcf, tuple->return_ident);

		/*
		 * Host order IP addr.
		 */
		ppe_drv_v6_conn_flow_dump_match_src_ip_set(pcf, pcf->match_src_ip);
		ppe_drv_v6_conn_flow_dump_match_dest_ip_set(pcf, pcf->match_dest_ip);

		/*
		 * Flow MTU and transmit MAC address.
		 */
		ppe_drv_v6_conn_flow_xmit_interface_mtu_set(pcf, conn->return_mtu);
		ppe_drv_v6_conn_flow_xmit_dest_mac_addr_set(pcf, conn->return_mac);
		pcf->eg_port_if = ppe_drv_iface_ref(if_tx);
	}

	/*
	 * Prepare return direction rule
	 */
	if (rule_flags & PPE_DRV_V6_RULE_FLAG_RETURN_VALID) {
		ppe_drv_v6_conn_flow_conn_set(pcr, cn);

		/*
		 * Set Rx and Tx port.
		 */
		ppe_drv_v6_conn_flow_rx_port_set(pcr, pp_tx);
		ppe_drv_v6_conn_flow_tx_port_set(pcr, pp_rx);

		/*
		 * Set 5-tuple along with SNAT/DNAT requirement.
		 */
		ppe_drv_v6_conn_flow_match_protocol_set(pcr, tuple->protocol);
		ppe_drv_v6_conn_flow_match_src_ip_set(pcr, tuple->return_ip);
		ppe_drv_v6_conn_flow_match_src_ident_set(pcr, tuple->return_ident);
		ppe_drv_v6_conn_flow_match_dest_ip_set(pcr, tuple->flow_ip);
		ppe_drv_v6_conn_flow_match_dest_ident_set(pcr, tuple->flow_ident);

		ppe_drv_v6_conn_flow_dump_match_src_ip_set(pcr, pcr->match_src_ip);
		ppe_drv_v6_conn_flow_dump_match_dest_ip_set(pcr, pcr->match_dest_ip);
		/*
		 * Flow MTU and transmit MAC address.
		 */
		ppe_drv_v6_conn_flow_xmit_interface_mtu_set(pcr, conn->flow_mtu);
		ppe_drv_v6_conn_flow_xmit_dest_mac_addr_set(pcr, conn->flow_mac);

		ppe_drv_v6_conn_flags_set(cn, PPE_DRV_V6_CONN_FLAG_RETURN_VALID);
		pcr->eg_port_if = ppe_drv_iface_ref(if_rx);
	}

	return PPE_DRV_RET_SUCCESS;
}

/*
 * ppe_drv_v6_conn_fill()
 *	Populate each direction flow object.
 */
ppe_drv_ret_t ppe_drv_v6_conn_fill(struct ppe_drv_v6_rule_create *create, struct ppe_drv_v6_conn *cn,
				   enum ppe_drv_conn_type flow_type)
{
	struct ppe_drv_v6_connection_rule *conn = &create->conn_rule;
	struct ppe_drv_v6_5tuple *tuple = &create->tuple;
	struct ppe_drv_pppoe_session *flow_pppoe_rule = &create->pppoe_rule.flow_session;
	struct ppe_drv_pppoe_session *return_pppoe_rule = &create->pppoe_rule.return_session;
	struct ppe_drv_dscp_rule *dscp_rule = &create->dscp_rule;
	struct ppe_drv_vlan_info *vlan_primary_rule = &create->vlan_rule.primary_vlan;
	struct ppe_drv_vlan_info *vlan_secondary_rule = &create->vlan_rule.secondary_vlan;
	struct ppe_drv_iface *if_rx, *if_tx, *top_if_rx, *top_if_tx;
	struct ppe_drv_top_if_rule *top_rule = &create->top_rule;
	struct ppe_drv_service_class_rule *sawf_rule = &create->sawf_rule;
	struct ppe_drv_qos_rule *qos_rule = &create->qos_rule;
	struct ppe_drv_v6_conn_flow *pcf = &cn->pcf;
	struct ppe_drv_v6_conn_flow *pcr = &cn->pcr;
	uint16_t valid_flags = create->valid_flags;
	uint16_t rule_flags = create->rule_flags;
	uint32_t sawf_tag;
	bool is_wanif;
	struct ppe_drv_flow_cookie_metadata fc_metadata = {0};
	struct ppe_drv_comm_stats *comm_stats;
	struct ppe_drv_port *pp_rx, *pp_tx;
	struct ppe_drv *p = &ppe_drv_gbl;

	comm_stats = &p->stats.comm_stats[flow_type];
	/*
	 * Make sure both Rx and Tx inteface are mapped to PPE ports properly.
	 */
	if_rx = ppe_drv_iface_get_by_idx(conn->rx_if);
	if (!if_rx) {
		ppe_drv_stats_inc(&comm_stats->v6_create_fail_invalid_rx_if);
		ppe_drv_warn("%p: No PPE interface corresponding to rx_if: %d", create, conn->rx_if);
		return PPE_DRV_RET_FAILURE_INVALID_PARAM;
	}

	pp_rx = ppe_drv_iface_port_get(if_rx);
	if (!pp_rx) {
		ppe_drv_stats_inc(&comm_stats->v6_create_fail_invalid_rx_port);
		ppe_drv_warn("%p: Invalid Rx IF: %d", create, conn->rx_if);
		return PPE_DRV_RET_FAILURE_IFACE_PORT_MAP;
	}

	if_tx = ppe_drv_iface_get_by_idx(conn->tx_if);
	if (!if_tx) {
		ppe_drv_stats_inc(&comm_stats->v6_create_fail_invalid_tx_if);
		ppe_drv_warn("%p: No PPE interface corresponding to tx_if: %d", create, conn->tx_if);
		return PPE_DRV_RET_FAILURE_INVALID_PARAM;
	}

	pp_tx = ppe_drv_iface_port_get(if_tx);
	if (!pp_tx) {
		ppe_drv_stats_inc(&comm_stats->v6_create_fail_invalid_tx_port);
		ppe_drv_warn("%p: Invalid Tx IF: %d", create, conn->tx_if);
		return PPE_DRV_RET_FAILURE_IFACE_PORT_MAP;
	}

	top_if_rx = ppe_drv_iface_get_by_idx(top_rule->rx_if);
	if (!top_if_rx) {
		ppe_drv_stats_inc(&comm_stats->v6_create_fail_invalid_rx_if);
		ppe_drv_warn("%p: No PPE interface corresponding to rx_if: %d", create, top_rule->rx_if);
		return PPE_DRV_RET_FAILURE_INVALID_PARAM;
	}

	top_if_tx = ppe_drv_iface_get_by_idx(top_rule->tx_if);
	if (!top_if_tx) {
		ppe_drv_stats_inc(&comm_stats->v6_create_fail_invalid_tx_if);
		ppe_drv_warn("%p: No PPE interface corresponding to tx_if: %d", create, top_rule->tx_if);
		return PPE_DRV_RET_FAILURE_INVALID_PARAM;
	}

	/*
	 * For DS flow
	 */
	if (rule_flags & PPE_DRV_V6_RULE_FLAG_DS_FLOW) {
		if ((pp_tx->user_type != PPE_DRV_PORT_USER_TYPE_DS) && (pp_rx->user_type != PPE_DRV_PORT_USER_TYPE_DS)) {
			ppe_drv_warn("%p: Invalid user type: %d", create, pp_tx->user_type);
			return PPE_DRV_RET_INVALID_USER_TYPE;
		}
	}

	/*
	 * Bridge flow
	 */
	if (rule_flags & PPE_DRV_V6_RULE_FLAG_BRIDGE_FLOW) {
		ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_BRIDGE_FLOW);
		ppe_drv_v6_conn_flow_flags_set(pcr, PPE_DRV_V6_CONN_FLOW_FLAG_BRIDGE_FLOW);
	}

	/*
	 * Prepare flow direction rule
	 */
	if (rule_flags & PPE_DRV_V6_RULE_FLAG_FLOW_VALID) {
		ppe_drv_v6_conn_flow_conn_set(pcf, cn);

		/*
		 * Set Rx and Tx port.
		 */
		ppe_drv_v6_conn_flow_rx_port_set(pcf, pp_rx);
		ppe_drv_v6_conn_flow_tx_port_set(pcf, pp_tx);

		/*
		 * Set 5-tuple.
		 */
		ppe_drv_v6_conn_flow_match_protocol_set(pcf, tuple->protocol);
		ppe_drv_v6_conn_flow_match_src_ip_set(pcf, tuple->flow_ip);
		ppe_drv_v6_conn_flow_match_src_ident_set(pcf, tuple->flow_ident);
		ppe_drv_v6_conn_flow_match_dest_ip_set(pcf, tuple->return_ip);
		ppe_drv_v6_conn_flow_match_dest_ident_set(pcf, tuple->return_ident);

		/*
		 * Host order IP addr.
		 */
		ppe_drv_v6_conn_flow_dump_match_src_ip_set(pcf, pcf->match_src_ip);
		ppe_drv_v6_conn_flow_dump_match_dest_ip_set(pcf, pcf->match_dest_ip);

		/*
		 * Flow MTU and transmit MAC address.
		 */
		ppe_drv_v6_conn_flow_xmit_interface_mtu_set(pcf, conn->return_mtu);
		ppe_drv_v6_conn_flow_xmit_dest_mac_addr_set(pcf, conn->return_mac);

		if (valid_flags & PPE_DRV_V6_VALID_FLAG_DSCP_MARKING) {
			ppe_drv_v6_conn_flow_egress_dscp_set(pcf, dscp_rule->flow_dscp);
			ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_DSCP_MARKING);
		}

		/*
		 * VLAN over brige case
		 */
		if ((rule_flags & PPE_DRV_V6_RULE_TO_BRIDGE_VLAN_NETDEV) == PPE_DRV_V6_RULE_TO_BRIDGE_VLAN_NETDEV) {
			ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLAG_BRIDGE_VLAN_NETDEV);
			ppe_drv_trace("%p: VLAN over bridge case from\n", pcf);
		}

		/*
		 * For VP flow if user type is DS, set conn rule VP valid.
		 */
		if ((rule_flags & PPE_DRV_V6_RULE_FLAG_VP_FLOW) && (pp_tx->user_type == PPE_DRV_PORT_USER_TYPE_DS)) {
			ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_VP_VALID);
		}

		if (valid_flags & PPE_DRV_V6_VALID_FLAG_QOS) {
			qos_rule->flow_qos_tag = (qos_rule->flow_qos_tag > PPE_DRV_INT_PRI_MAX) ? PPE_DRV_INT_PRI_MAX : qos_rule->flow_qos_tag;

			if (qos_rule->qos_valid_flags & PPE_DRV_VALID_FLAG_FLOW_PPE_QOS) {
				ppe_drv_v6_conn_flow_int_pri_set(pcf, qos_rule->flow_int_pri);
			} else {
				ppe_drv_v6_conn_flow_int_pri_set(pcf, qos_rule->flow_qos_tag);
			}

			ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_QOS_VALID);
		}

		/*
                 * Check if HLOS TID info is valid in this direction and if the
                 * interface is a wifi VP.
                 */
		if ((valid_flags & PPE_DRV_V6_VALID_FLAG_WIFI_TID) &&
				(ppe_drv_port_flags_check(pp_tx, PPE_DRV_PORT_FLAG_WIFI_DEV))) {
			fc_metadata.type.mark = qos_rule->flow_qos_tag;
			ppe_drv_v6_conn_flow_metadata_set(pcf, &fc_metadata, PPE_DRV_TREE_ID_TYPE_WIFI_TID);
			ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLOW_METADATA_TYPE_WIFI_INFO);
		}

		/*
		 * Check if SAWF info is valid in this direction and if the
		 * interface is a wifi VP.
		 */
		if ((valid_flags & PPE_DRV_V6_VALID_FLAG_SAWF) &&
					(ppe_drv_port_flags_check(pp_tx, PPE_DRV_PORT_FLAG_WIFI_DEV))) {
			sawf_tag = PPE_DRV_SAWF_TAG_GET(sawf_rule->flow_mark);
			if (sawf_tag == PPE_DRV_SAWF_VALID_TAG) {
				memset(&fc_metadata, 0, sizeof(fc_metadata));
				fc_metadata.type.sawf.sawf_mark = sawf_rule->flow_mark;
				fc_metadata.type.sawf.service_class = sawf_rule->flow_service_class;
				ppe_drv_v6_conn_flow_metadata_set(pcf, &fc_metadata, PPE_DRV_TREE_ID_TYPE_SAWF);
				ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLOW_METADATA_TYPE_WIFI_INFO);
			}
		}

		/*
		 * Check if SCS info is valid in this direction and if the
		 * interface is a wifi VP.
		 */
		if ((valid_flags & PPE_DRV_V6_VALID_FLAG_SCS) &&
					(ppe_drv_port_flags_check(pp_tx, PPE_DRV_PORT_FLAG_WIFI_DEV))) {
			memset(&fc_metadata, 0, sizeof(fc_metadata));
			fc_metadata.type.scs.scs_mark = sawf_rule->flow_mark;
			ppe_drv_v6_conn_flow_metadata_set(pcf, &fc_metadata, PPE_DRV_TREE_ID_TYPE_SCS);
			ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLOW_METADATA_TYPE_WIFI_INFO);
		}

		/*
		 * Check if DS metadata info is valid in this direction for MLO assist and if the
		 * interface is a Wi-Fi VP.
		 */
		if ((valid_flags & PPE_DRV_V6_VALID_FLAG_FLOW_WIFI_DS) &&
					(ppe_drv_port_flags_check(pp_tx, PPE_DRV_PORT_FLAG_WIFI_DEV))) {
			pcf->wifi_rule_ds_metadata = create->wifi_rule.flow_ds_node_mdata;
			ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_WIFI_DS);
		}

		/*
		 * Check if Wi-Fi metadata info is valid in this direction for MLO assist and if the
		 * interface is a wifi VP.
		 */
		if ((valid_flags & PPE_DRV_V6_VALID_FLAG_FLOW_WIFI_MDATA) &&
					(ppe_drv_port_flags_check(pp_tx, PPE_DRV_PORT_FLAG_WIFI_DEV))) {
				memset(&fc_metadata, 0, sizeof(fc_metadata));
				fc_metadata.type.mark = create->wifi_rule.flow_mark;
				ppe_drv_v6_conn_flow_metadata_set(pcf, &fc_metadata, PPE_DRV_TREE_ID_TYPE_MLO_ASSIST);
		}

		if (valid_flags & PPE_DRV_V6_VALID_FLAG_VLAN) {
			pcf->ingress_vlan[0].tci = PPE_DRV_VLAN_NOT_CONFIGURED;
			pcf->ingress_vlan[1].tci = PPE_DRV_VLAN_NOT_CONFIGURED;
			pcf->egress_vlan[0].tci = PPE_DRV_VLAN_NOT_CONFIGURED;
			pcf->egress_vlan[1].tci = PPE_DRV_VLAN_NOT_CONFIGURED;
			ppe_drv_v6_flow_vlan_set(pcf, vlan_primary_rule->ingress_vlan_tag,
					vlan_primary_rule->egress_vlan_tag,
					vlan_secondary_rule->ingress_vlan_tag,
					vlan_secondary_rule->egress_vlan_tag);

		}

		if (valid_flags & PPE_DRV_V6_VALID_FLAG_PPPOE_RETURN) {
			ppe_drv_v6_conn_flow_pppoe_session_id_set(pcf, return_pppoe_rule->session_id);
			ppe_drv_v6_conn_flow_pppoe_server_mac_set(pcf, return_pppoe_rule->server_mac);
			ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_PPPOE_FLOW);
		}

		/*
		 * Bridge + VLAN? Make sure both top interfaces are attached to same parent.
		 * If Interface is set as wanif using "echo eth# > /proc/sys/ppe/bridge_mgr/add_wanif",
		 * bridge interface is not set as parent of interface.
		 * So if either of top_if_rx or top_if_tx is wanif, then avoide this check.
		 */
		is_wanif = ((top_if_rx->flags & PPE_DRV_IFACE_FLAG_WAN_IF_VALID) || (top_if_tx->flags & PPE_DRV_IFACE_FLAG_WAN_IF_VALID));
		if ((rule_flags & PPE_DRV_V6_RULE_FLAG_BRIDGE_FLOW) && !is_wanif) {
			if (!ppe_drv_iface_parent_get(top_if_rx) || !ppe_drv_iface_parent_get(top_if_tx)) {
				ppe_drv_stats_inc(&comm_stats->v6_create_fail_bridge_noexist);
				ppe_drv_warn("%p: one of top's parent interface is null: top_rx_if : %d tx_if: %d",
						create, top_rule->rx_if, top_rule->tx_if);
				return PPE_DRV_RET_FAILURE_NOT_BRIDGE_SLAVES;
			}

			if (ppe_drv_v6_conn_flow_ingress_vlan_cnt_get(pcf) || ppe_drv_v6_conn_flow_egress_vlan_cnt_get(pcf)) {
				if (ppe_drv_iface_parent_get(top_if_rx) != ppe_drv_iface_parent_get(top_if_tx)) {
					ppe_drv_stats_inc(&comm_stats->v6_create_fail_vlan_filter);
					ppe_drv_warn("%p: IF not part of same bridge rx_if: %d tx_if: %d",
							create, top_rule->rx_if, top_rule->tx_if);
					return PPE_DRV_RET_FAILURE_NOT_BRIDGE_SLAVES;
				}
		       }
		}

		/*
		 * Check if destination vp is inline EIP virtual port.
		 */
		if (ppe_drv_port_flags_check(ppe_drv_v6_conn_flow_tx_port_get(pcf), PPE_DRV_PORT_FLAG_IIPSEC)) {
			ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_INLINE_IPSEC);
		}

#ifdef NSS_PPE_IPQ53XX
		/*
		 * Check source interface based on rule flags.
		 */
		if (rule_flags & PPE_DRV_V6_RULE_FLAG_SRC_INTERFACE_CHECK) {
			ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_SRC_INTERFACE_CHECK);
		}
#endif
		/*
		 * Set noedit rule
		 */
		if (rule_flags & PPE_DRV_V6_RULE_NOEDIT_FLOW_RULE) {
			ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_NO_EDIT_RULE);
		}
	}

	/*
	 * Prepare return direction rule
	 */
	if (rule_flags & PPE_DRV_V6_RULE_FLAG_RETURN_VALID) {
		ppe_drv_v6_conn_flow_conn_set(pcr, cn);

		/*
		 * Set Rx and Tx port.
		 */
		ppe_drv_v6_conn_flow_rx_port_set(pcr, pp_tx);
		ppe_drv_v6_conn_flow_tx_port_set(pcr, pp_rx);

		/*
		 * Set 5-tuple.
		 */
		ppe_drv_v6_conn_flow_match_protocol_set(pcr, tuple->protocol);
		ppe_drv_v6_conn_flow_match_src_ip_set(pcr, tuple->return_ip);
		ppe_drv_v6_conn_flow_match_src_ident_set(pcr, tuple->return_ident);
		ppe_drv_v6_conn_flow_match_dest_ip_set(pcr, tuple->flow_ip);
		ppe_drv_v6_conn_flow_match_dest_ident_set(pcr, tuple->flow_ident);

		ppe_drv_v6_conn_flow_dump_match_src_ip_set(pcr, pcr->match_src_ip);
		ppe_drv_v6_conn_flow_dump_match_dest_ip_set(pcr, pcr->match_dest_ip);
		/*
		 * Flow MTU and transmit MAC address.
		 */
		ppe_drv_v6_conn_flow_xmit_interface_mtu_set(pcr, conn->flow_mtu);
		ppe_drv_v6_conn_flow_xmit_dest_mac_addr_set(pcr, conn->flow_mac);

		if (valid_flags & PPE_DRV_V6_VALID_FLAG_DSCP_MARKING) {
			ppe_drv_v6_conn_flow_egress_dscp_set(pcr, dscp_rule->return_dscp);
			ppe_drv_v6_conn_flow_flags_set(pcr, PPE_DRV_V6_CONN_FLOW_FLAG_DSCP_MARKING);
		}

		/*
		 * VLAN over bridge case
		 */
		if ((rule_flags & PPE_DRV_V6_RULE_FROM_BRIDGE_VLAN_NETDEV) == PPE_DRV_V6_RULE_FROM_BRIDGE_VLAN_NETDEV) {
			ppe_drv_v6_conn_flow_flags_set(pcr, PPE_DRV_V6_CONN_FLAG_BRIDGE_VLAN_NETDEV);
			ppe_drv_trace("%p: VLAN over bridge case to\n", pcr);
		}

		/*
		 * For VP flow if user type is DS, set conn rule VP valid.
		 */
		if ((rule_flags & PPE_DRV_V6_RULE_FLAG_VP_FLOW) && (pp_rx->user_type == PPE_DRV_PORT_USER_TYPE_DS)) {
			ppe_drv_v6_conn_flow_flags_set(pcr, PPE_DRV_V6_CONN_FLAG_FLOW_VP_VALID);
		}

		if (valid_flags & PPE_DRV_V6_VALID_FLAG_QOS) {
			qos_rule->return_qos_tag = (qos_rule->return_qos_tag > PPE_DRV_INT_PRI_MAX) ? PPE_DRV_INT_PRI_MAX : qos_rule->return_qos_tag;

			if (qos_rule->qos_valid_flags & PPE_DRV_VALID_FLAG_RETURN_PPE_QOS) {
				ppe_drv_v6_conn_flow_int_pri_set(pcr, qos_rule->return_int_pri);
			} else {
				ppe_drv_v6_conn_flow_int_pri_set(pcr, qos_rule->return_qos_tag);
			}

			ppe_drv_v6_conn_flow_flags_set(pcr, PPE_DRV_V6_CONN_FLOW_FLAG_QOS_VALID);
		}

		/*
		 * Check if HLOS TID info is valid in this direction and if the
		 * interface is a wifi VP.
		 */
		if ((valid_flags & PPE_DRV_V6_VALID_FLAG_WIFI_TID) &&
				(ppe_drv_port_flags_check(pp_rx, PPE_DRV_PORT_FLAG_WIFI_DEV))) {
			fc_metadata.type.mark = qos_rule->return_qos_tag;
			ppe_drv_v6_conn_flow_metadata_set(pcr, &fc_metadata, PPE_DRV_TREE_ID_TYPE_WIFI_TID);
			ppe_drv_v6_conn_flow_flags_set(pcr, PPE_DRV_V6_CONN_FLOW_METADATA_TYPE_WIFI_INFO);
		}

		/*
		 * Check if SAWF info is valid in this direction and if the
		 * interface is a wifi VP.
		 */
		if ((valid_flags & PPE_DRV_V6_VALID_FLAG_SAWF) &&
				(ppe_drv_port_flags_check(pp_rx, PPE_DRV_PORT_FLAG_WIFI_DEV))) {
			sawf_tag = PPE_DRV_SAWF_TAG_GET(sawf_rule->return_mark);
			if (sawf_tag == PPE_DRV_SAWF_VALID_TAG) {
				memset(&fc_metadata, 0, sizeof(fc_metadata));
				fc_metadata.type.sawf.sawf_mark = sawf_rule->return_mark;
				fc_metadata.type.sawf.service_class = sawf_rule->return_service_class;
				ppe_drv_v6_conn_flow_metadata_set(pcr, &fc_metadata, PPE_DRV_TREE_ID_TYPE_SAWF);
				ppe_drv_v6_conn_flow_flags_set(pcr, PPE_DRV_V6_CONN_FLOW_METADATA_TYPE_WIFI_INFO);
			}
		}

		/*
		 * Check if SCS info is valid in this direction and if the
		 * interface is a wifi VP.
		 */
		if ((valid_flags & PPE_DRV_V6_VALID_FLAG_SCS) &&
					(ppe_drv_port_flags_check(pp_rx, PPE_DRV_PORT_FLAG_WIFI_DEV))) {
			memset(&fc_metadata, 0, sizeof(fc_metadata));
			fc_metadata.type.scs.scs_mark = sawf_rule->return_mark;
			ppe_drv_v6_conn_flow_metadata_set(pcr, &fc_metadata, PPE_DRV_TREE_ID_TYPE_SCS);
			ppe_drv_v6_conn_flow_flags_set(pcr, PPE_DRV_V6_CONN_FLOW_METADATA_TYPE_WIFI_INFO);
		}

		/*
		 * Check if DS metadata info is valid in this direction for MLO assist and if the
		 * interface is a Wi-Fi VP.
		 */
		if ((valid_flags & PPE_DRV_V6_VALID_FLAG_RETURN_WIFI_DS) &&
					(ppe_drv_port_flags_check(pp_rx, PPE_DRV_PORT_FLAG_WIFI_DEV))) {
			pcr->wifi_rule_ds_metadata = create->wifi_rule.return_ds_node_mdata;
			ppe_drv_v6_conn_flow_flags_set(pcr, PPE_DRV_V6_CONN_FLAG_FLOW_WIFI_DS);
		}

		/*
		 * Check if Wi-Fi metadata info is valid in this direction for MLO assist and if the
		 * interface is a wifi VP.
		 */
		if ((valid_flags & PPE_DRV_V6_VALID_FLAG_RETURN_WIFI_MDATA) &&
					(ppe_drv_port_flags_check(pp_rx, PPE_DRV_PORT_FLAG_WIFI_DEV))) {
				memset(&fc_metadata, 0, sizeof(fc_metadata));
				fc_metadata.type.mark = create->wifi_rule.return_mark;
				ppe_drv_v6_conn_flow_metadata_set(pcr, &fc_metadata, PPE_DRV_TREE_ID_TYPE_MLO_ASSIST);
		}

		if (valid_flags & PPE_DRV_V6_VALID_FLAG_VLAN) {
			pcr->ingress_vlan[0].tci = PPE_DRV_VLAN_NOT_CONFIGURED;
			pcr->ingress_vlan[1].tci = PPE_DRV_VLAN_NOT_CONFIGURED;
			pcr->egress_vlan[0].tci = PPE_DRV_VLAN_NOT_CONFIGURED;
			pcr->egress_vlan[1].tci = PPE_DRV_VLAN_NOT_CONFIGURED;
			ppe_drv_v6_flow_vlan_set(pcr, vlan_primary_rule->egress_vlan_tag,
					vlan_primary_rule->ingress_vlan_tag,
					vlan_secondary_rule->egress_vlan_tag,
					vlan_secondary_rule->ingress_vlan_tag);
		}

		if (valid_flags & PPE_DRV_V6_VALID_FLAG_PPPOE_FLOW) {
			ppe_drv_v6_conn_flow_pppoe_session_id_set(pcr, flow_pppoe_rule->session_id);
			ppe_drv_v6_conn_flow_pppoe_server_mac_set(pcr, flow_pppoe_rule->server_mac);
			ppe_drv_v6_conn_flow_flags_set(pcr, PPE_DRV_V6_CONN_FLOW_FLAG_PPPOE_FLOW);
		}

		/*
		 * Bridge + VLAN? Make sure both top interfaces are attached to same parent.
		 * If Interface is set as wanif using "echo eth# > /proc/sys/ppe/bridge_mgr/add_wanif",
		 * bridge interface is not set as parent of interface.
		 * So if either of top_if_rx or top_if_tx is wanif, then avoide this check.
		 */
		is_wanif = ((top_if_rx->flags & PPE_DRV_IFACE_FLAG_WAN_IF_VALID) || (top_if_tx->flags & PPE_DRV_IFACE_FLAG_WAN_IF_VALID));
		if ((rule_flags & PPE_DRV_V6_RULE_FLAG_BRIDGE_FLOW) && !is_wanif) {

			if (!ppe_drv_iface_parent_get(top_if_rx) || !ppe_drv_iface_parent_get(top_if_tx)) {
				ppe_drv_stats_inc(&comm_stats->v6_create_fail_bridge_noexist);
				ppe_drv_warn("%p: one of top's parent interface is null: top_rx_if : %d tx_if: %d",
						create, top_rule->rx_if, top_rule->tx_if);
				return PPE_DRV_RET_FAILURE_NOT_BRIDGE_SLAVES;
			}

			if (ppe_drv_v6_conn_flow_ingress_vlan_cnt_get(pcr) || ppe_drv_v6_conn_flow_egress_vlan_cnt_get(pcr)) {
				if (ppe_drv_iface_parent_get(top_if_rx) != ppe_drv_iface_parent_get(top_if_tx)) {
					ppe_drv_stats_inc(&comm_stats->v6_create_fail_vlan_filter);
					ppe_drv_warn("%p: IF not part of same bridge rx_if: %d tx_if: %d",
							create, top_rule->rx_if, top_rule->tx_if);
					return PPE_DRV_RET_FAILURE_NOT_BRIDGE_SLAVES;
				}
			}
		}

		/*
		 * Check if destination vp is inline EIP virtual port.
		 */
		if (ppe_drv_port_flags_check(ppe_drv_v6_conn_flow_tx_port_get(pcr), PPE_DRV_PORT_FLAG_IIPSEC)) {
			ppe_drv_v6_conn_flow_flags_set(pcr, PPE_DRV_V6_CONN_FLOW_FLAG_INLINE_IPSEC);
		}

#ifdef NSS_PPE_IPQ53XX
		/*
		 * Check source interface based on rule flags.
		 */
		if (rule_flags & PPE_DRV_V6_RULE_FLAG_SRC_INTERFACE_CHECK) {
			ppe_drv_v6_conn_flow_flags_set(pcr, PPE_DRV_V6_CONN_FLOW_FLAG_SRC_INTERFACE_CHECK);
		}
#endif
		/*
		 * Set noedit rule
		 */
		if (rule_flags & PPE_DRV_V6_RULE_NOEDIT_RETURN_RULE) {
			ppe_drv_v6_conn_flow_flags_set(pcr, PPE_DRV_V6_CONN_FLAG_FLOW_NO_EDIT_RULE);
		}

		ppe_drv_v6_conn_flags_set(cn, PPE_DRV_V6_CONN_FLAG_RETURN_VALID);
	}

	return PPE_DRV_RET_SUCCESS;
}

/*
 * ppe_drv_v6_if_walk_release()
 *	Release references taken during interface hierarchy walk.
 */
void ppe_drv_v6_if_walk_release(struct ppe_drv_v6_conn_flow *pcf)
{
	if (pcf->eg_vsi_if) {
		ppe_drv_iface_deref_internal(pcf->eg_vsi_if);
		pcf->eg_vsi_if = NULL;
	}

	if (pcf->eg_l3_if) {
		ppe_drv_iface_deref_internal(pcf->eg_l3_if);
		pcf->eg_l3_if = NULL;
	}

	if (pcf->eg_port_if) {
		ppe_drv_iface_deref_internal(pcf->eg_port_if);
		pcf->eg_port_if = NULL;
	}

	if (pcf->in_port_if) {
		ppe_drv_iface_deref_internal(pcf->in_port_if);
		pcf->in_port_if = NULL;
	}

	 if (pcf->in_l3_if) {
		ppe_drv_iface_deref_internal(pcf->in_l3_if);
		pcf->in_l3_if = NULL;
	}
}

/*
 * ppe_drv_v6_if_walk()
 *	Walk iface heirarchy to obtain egress L3_IF and VSI
 */
bool ppe_drv_v6_if_walk(struct ppe_drv_v6_conn_flow *pcf, struct ppe_drv_top_if_rule *top_if, ppe_drv_iface_t tx_if, ppe_drv_iface_t rx_if)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_iface *eg_vsi_if = NULL;
	struct ppe_drv_iface *eg_l3_if = NULL;
	struct ppe_drv_iface *iface, *top_iface = NULL, *top_rx_iface = NULL;
	struct ppe_drv_iface *tx_port_if = NULL, *rx_port_if = NULL;
	struct ppe_drv_iface *top_iface_parent = NULL;
	struct ppe_drv_vsi *vlan_vsi;
	struct ppe_drv_l3_if *pppoe_l3_if;
	uint32_t egress_vlan_inner = PPE_DRV_VLAN_NOT_CONFIGURED, egress_vlan_outer = PPE_DRV_VLAN_NOT_CONFIGURED;
	uint8_t vlan_cnt = ppe_drv_v6_conn_flow_egress_vlan_cnt_get(pcf);
	bool is_vlan_as_vp;

	switch (vlan_cnt) {
	case 2:
		egress_vlan_inner = ppe_drv_v6_conn_flow_egress_vlan_get(pcf, 1)->tci & PPE_DRV_VLAN_ID_MASK;
		egress_vlan_outer = ppe_drv_v6_conn_flow_egress_vlan_get(pcf, 0)->tci & PPE_DRV_VLAN_ID_MASK;
		break;
	case 1:
		egress_vlan_inner = ppe_drv_v6_conn_flow_egress_vlan_get(pcf, 0)->tci & PPE_DRV_VLAN_ID_MASK;
		break;
	case 0:
		break;
	default:
		return false;
	}

	/*
	 * Should have a valid bottom tx interface.
	 */
	tx_port_if = ppe_drv_iface_get_by_idx(tx_if);
	if (!tx_port_if) {
		ppe_drv_warn("%p: No PPE interface corresponding to tx_if: %d", p, tx_if);
		return false;
	}

	/*
	 * Should have a valid bottom rx interface.
	 */
	rx_port_if = ppe_drv_iface_get_by_idx(rx_if);
	if (!rx_port_if) {
		ppe_drv_warn("%p: No PPE interface corresponding to tx_if: %d", p, rx_if);
		return false;
	}

	/*
	 * Should have a valid top tx interface.
	 */
	iface = top_iface = ppe_drv_iface_get_by_idx(top_if->tx_if);
	if (!iface) {
		ppe_drv_warn("%p: No PPE interface corresponding to top tx interface\n", p);
		return false;
	}

	/*
	 * Should have a valid top rx interface.
	 */
	top_rx_iface = ppe_drv_iface_get_by_idx(top_if->rx_if);
	if (!top_rx_iface) {
		ppe_drv_warn("%p: No PPE interface corresponding to top rx interface\n", p);
		return false;
	}

	/*
	 * If it's a bridge flow, hierarchy walk not needed.
	 * Set ingress and egress port information in pcf.
	 * Set egress top VSI interface in pcf.
	 */
	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_BRIDGE_FLOW)) {
		ppe_drv_v6_conn_flow_eg_port_if_set(pcf, ppe_drv_iface_ref(tx_port_if));
		ppe_drv_v6_conn_flow_in_port_if_set(pcf, ppe_drv_iface_ref(rx_port_if));

		top_iface_parent = ppe_drv_iface_parent_get(top_iface);
		if (top_iface_parent)
			ppe_drv_v6_conn_flow_eg_top_vsi_set(pcf, ppe_drv_iface_vsi_get(top_iface_parent));

		ppe_drv_info("%p: Return for v6 bridge flow", p);
		return true;
	}

	/*
	 * Walk through tx if hierarchy.
	 */
	while (iface) {
		/*
		 * Routing to bridge device?
		 * Bridge's VSI and L3_if is the final egress VSI and egress L3_IF.
		 */
		if (iface->type == PPE_DRV_IFACE_TYPE_BRIDGE) {
			eg_vsi_if = iface;
			break;
		}

		if ((iface->type == PPE_DRV_IFACE_TYPE_VLAN)) {
			vlan_vsi = ppe_drv_iface_vsi_get(iface);
			if (vlan_vsi && ppe_drv_vsi_match_vlan(vlan_vsi, egress_vlan_inner, egress_vlan_outer)) {
				eg_vsi_if = iface;

				/*
				 * If desired eg_l3_if is also available, no need to continue the walk.
				 */
				if (eg_l3_if) {
					break;
				}
			}
		}

		if (iface->type == PPE_DRV_IFACE_TYPE_PPPOE) {
			pppoe_l3_if = ppe_drv_iface_l3_if_get(iface);
			if (pppoe_l3_if && ppe_drv_l3_if_pppoe_match(pppoe_l3_if, pcf->pppoe_session_id, pcf->pppoe_server_mac)) {
				eg_l3_if = iface;

				/*
				 * If desired eg_vsi_if is also available, no need to continue the walk.
				 */
				if (eg_vsi_if) {
					break;
				}
			}
		}

		iface = ppe_drv_iface_base_get(iface);
	}

	/*
	 * For create request with egress-VLAN, there must be a corresponding egress-VSI / VLAN as VP IF.
	 */
	is_vlan_as_vp = is_vlan_dev(tx_port_if->dev) && (tx_port_if->type == PPE_DRV_IFACE_TYPE_VIRTUAL);

	if (vlan_cnt && !is_vlan_as_vp && !eg_vsi_if) {
		ppe_drv_warn("%p: not able to find a matching vlan-if", pcf);
		return false;
	}

	/*
	 * For create request with egress-pppoe, there must be a corresponding egress-L3 IF.
	 */
	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_PPPOE_FLOW) && !eg_l3_if) {
		ppe_drv_warn("%p: not able to find a matching pppoe-if", pcf);
		return false;
	}

	/*
	 * Take the reference on egress interfaces used for this flow.
	 * Dereference: connection destroy.
	 */
	pcf->eg_vsi_if = eg_vsi_if ? ppe_drv_iface_ref(eg_vsi_if) : NULL;
	pcf->eg_l3_if = eg_l3_if ? ppe_drv_iface_ref(eg_l3_if) : NULL;
	pcf->eg_port_if = ppe_drv_iface_ref(tx_port_if);

	/*
	 * If top rx interface is valid, use ingress l3 if of that iface.
	 * else use port's l3
	 */
	if ((top_rx_iface) && (ppe_drv_iface_l3_if_get(top_rx_iface))) {
		ppe_drv_trace("Using top rx iface's l3 if");
		ppe_drv_v6_conn_flow_in_l3_if_set_and_ref(pcf, top_rx_iface);
	} else {
		ppe_drv_trace("Using port's l3 if");
		ppe_drv_v6_conn_flow_in_l3_if_set_and_ref(pcf, rx_port_if);
	}

	return true;
}

/*
 * ppe_drv_v6_flow_check()
 *	Search an entry into the flow table and returns the flow object.
 */
static bool ppe_drv_v6_flow_check(struct ppe_drv_v6_conn_flow *pcf)
{
	struct ppe_drv_v6_5tuple tuple;
	struct ppe_drv_flow *flow = NULL;

	ppe_drv_v6_conn_flow_match_src_ip_get(pcf, tuple.flow_ip);
	ppe_drv_v6_conn_flow_match_dest_ip_get(pcf, tuple.return_ip);
	tuple.flow_ident = ppe_drv_v6_conn_flow_match_src_ident_get(pcf);
	tuple.return_ident = ppe_drv_v6_conn_flow_match_dest_ident_get(pcf);
	tuple.protocol = ppe_drv_v6_conn_flow_match_protocol_get(pcf);

	/*
	 * Get flow table entry.
	 */
	flow = ppe_drv_flow_v6_get(&tuple);
	if (!flow) {
		ppe_drv_info("%p: flow entry not found", pcf);
		return false;
	}

	ppe_drv_info("%p: flow found: index=%d host_idx=%d", pcf, flow->index, flow->host->index);

	return true;
}

/*
 * ppe_drv_v6_flow_del()
 *	Delete an entry from the flow table.
 */
static bool ppe_drv_v6_flow_del(struct ppe_drv_v6_conn_flow *pcf)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_flow *flow = pcf->pf;
	struct ppe_drv_flow_tree_id_data *tree_id_data = &(pcf->flow_metadata.tree_id_data);
	struct ppe_drv_fse_rule_info fse_info = {0};
	uint8_t service_class;
	struct ppe_drv_port *tx_port = NULL;
	struct ppe_drv_port *rx_port = NULL;

	ppe_drv_trace("%p, flow deletion request for flow-idx: %d host-idx: %u", pcf, flow->index, flow->host->index);

	/*
	 * As opposed to 'add', flow 'delete' needs host and flow entry to be handled separately.
	 * Note: a host entry or nexthop entry could be referenced by multiple flow entries.
	 */
	if (!ppe_drv_flow_del(flow)) {
		ppe_drv_warn("%p: flow entry deletion failed for flow_index: %d", flow, flow->index);
		return false;
	}

	/*
	 * Release host entry.
	 */
	ppe_drv_host_deref(flow->host);
	flow->host = NULL;

	/*
	 * Release nexthop entry.
	 */
	if (flow->nh) {
		ppe_drv_nexthop_deref(flow->nh);
		flow->nh = NULL;
	}

#ifdef PPE_DRV_FLOW_IG_MAC_WAR
	/*
	 * Release reference on ingress MAC
	 */
	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_IGMAC_VALID)) {
		ppe_drv_v6_conn_flow_igmac_del(pcf);
	}
#endif

	/*
	 * Clear the QOS map information.
	 */
	ppe_drv_flow_v6_qos_clear(flow);

	/*
	 * Read stats and clear counters for deleted flow.
	 * Note: it is assured that no new flow can take the same index since all of this
	 * is lock protected. Unless this operation is complete, a new flow cannot be offloaded.
	 */
	ppe_drv_flow_v6_stats_update(pcf);

	ppe_drv_flow_stats_clear(flow);

	/*
	 * Update stats
	 */
	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_BRIDGE_FLOW)) {
		ppe_drv_stats_dec(&p->stats.gen_stats.v6_l2_flows);
	} else {
		ppe_drv_stats_dec(&p->stats.gen_stats.v6_l3_flows);
	}

	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_PRIORITY_PPE_ASSIST)) {
		/*
		 * For flows which are added to PPE for only priority queue selection.
		 * Tx/Rx ports would be invalid and stats for the same is not saved.
		 * Hence exit from here after deleting flow entry.
		 */
		return true;
	}


	tx_port = ppe_drv_v6_conn_flow_tx_port_get(pcf);
	rx_port = ppe_drv_v6_conn_flow_rx_port_get(pcf);

	/*
	 * Update the number of VP and DS flows.
	 */
	if (ppe_drv_port_flags_check(tx_port, PPE_DRV_PORT_FLAG_WIFI_DEV) ||
			ppe_drv_port_flags_check(rx_port, PPE_DRV_PORT_FLAG_WIFI_DEV)) {
		ppe_drv_stats_dec(&p->stats.gen_stats.v6_vp_wifi_flows);
	}

	if ((tx_port->user_type == PPE_DRV_PORT_USER_TYPE_DS) ||
			(rx_port->user_type == PPE_DRV_PORT_USER_TYPE_DS)) {
		ppe_drv_stats_dec(&p->stats.gen_stats.v6_ds_flows);
	}

	/*
	 * Decrement flow count if SAWF service class is configured in tree_id.
	 */
	if (ppe_drv_tree_id_type_get(&pcf->flow_metadata) == PPE_DRV_TREE_ID_TYPE_SAWF) {
		service_class = tree_id_data->info.sawf_metadata.service_class;
		ppe_drv_stats_dec(&p->stats.sawf_sc_stats[service_class].flow_count);
		ppe_drv_trace("Stats Updated: service class %u : flow  %llu\n",service_class, atomic64_read(&p->stats.sawf_sc_stats[service_class].flow_count));
	}

	/*
	 * Clear the flow entry in SW so that it can be reused.
	 */
	flow->pkts = 0;
	flow->bytes = 0;
	flow->flags = 0;
	flow->service_code = 0;
	flow->type = 0;

	/*
	 * Delete corresponding FSE rule for a Wi-Fi flow.
	 */
	if (p->is_wifi_fse_up && ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_FSE)) {
		ppe_drv_fill_fse_v6_tuple_info(pcf, &fse_info, false);

		if (p->fse_ops->destroy_fse_rule(&fse_info)) {
			ppe_drv_stats_inc(&p->stats.comm_stats->v6_destroy_fse_fail);
			ppe_drv_warn("%p: FSE v6 rule deletion failed\n", pcf);
			return true;
		}

		ppe_drv_v6_conn_flow_flags_clear(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_FSE);
		kref_put(&p->fse_ops_ref, ppe_drv_fse_ops_free);
		ppe_drv_stats_inc(&p->stats.comm_stats->v6_destroy_fse_success);
		ppe_drv_trace("%p: FSE v6 rule deletion successfull\n", pcf);
	}

	return true;
}

/*
 * ppe_drv_v6_flow_add()
 *	Adds an entry into the flow table and returns the flow index.
 */
static struct ppe_drv_flow *ppe_drv_v6_flow_add(struct ppe_drv_v6_conn_flow *pcf)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_nexthop *nh = NULL;
	struct ppe_drv_flow *flow = NULL;
	struct ppe_drv_host *host = NULL;
	struct ppe_drv_port *tx_port = NULL;
	struct ppe_drv_port *rx_port = NULL;
#ifdef NSS_PPE_IPQ53XX
	uint8_t vlan_cnt = 0;
	struct ppe_drv_vsi *eg_top_vsi = NULL;
	struct ppe_drv_port *pp_tx = NULL;
	struct ppe_drv_port *pp_rx = NULL;
#endif

	/*
	 * Fetch a new nexthop entry.
	 * Note: NEXTHOP entry is not required for bridged flows.
	 * Nexthop is not valid for priority assist as the packet is not required
	 * to be forwarded in PPE and is expected to be exceptioned to host.
	 */
	if (!(ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_BRIDGE_FLOW) ||
		ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_RFS_PPE_ASSIST) ||
		ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_PRIORITY_PPE_ASSIST))) {
		nh = ppe_drv_nexthop_v6_get_and_ref(pcf);
		if (!nh) {
			ppe_drv_warn("%p: unable to allocate nexthop", pcf);
			return NULL;
		}
	}

#if defined(NSS_PPE_IPQ53XX)
	/*
	 * Fetch a new nexthop entry for bridged flows.
	 */
	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_BRIDGE_FLOW)) {

		vlan_cnt = ppe_drv_v6_conn_flow_egress_vlan_cnt_get(pcf);
		if (vlan_cnt) {
			pp_tx = ppe_drv_v6_conn_flow_tx_port_get(pcf);
			if (!pp_tx) {
				ppe_drv_warn("%p: egress port invalid", pcf);
				return NULL;
			}

			pp_rx = ppe_drv_v6_conn_flow_rx_port_get(pcf);
			if (!pp_rx) {
				ppe_drv_warn("%p: ingress port invalid", pcf);
				return NULL;
			}

			eg_top_vsi = ppe_drv_v6_conn_flow_eg_top_vsi_get(pcf);
			if (!eg_top_vsi) {
				ppe_drv_warn("%p: no vsi configured on top iface", pcf);
				return NULL;
			}

			ppe_drv_info("%p: ppe tx port: %d, rx port: %d", pcf, pp_tx->port, pp_rx->port);

			if (!(eg_top_vsi->is_fdb_learn_enabled && pp_tx->is_fdb_learn_enabled && pp_rx->is_fdb_learn_enabled)) {
				nh = ppe_drv_nexthop_v6_bridge_flow_get_and_ref(pcf);
				if (!nh) {
					ppe_drv_warn("%p: unable to allocate nexthop for bridge flow", pcf);
					return NULL;
				}
			}
		}
	}
#endif

	/*
	 * Add host table entry.
	 */
	host = ppe_drv_host_v6_add(pcf);
	if (!host) {
		ppe_drv_warn("%p: host entry add failed for conn flow", pcf);
		goto flow_add_fail;
	}

	/*
	 * Add flow table entry.
	 */
	flow = ppe_drv_flow_v6_add(pcf, nh, host, false);
	if (!flow) {
		ppe_drv_warn("%p: flow entry add failed for conn flow", pcf);
		goto flow_add_fail;
	}

	ppe_drv_info("%p: flow acclerated: index=%d host_idx=%d", pcf, flow->index, host->index);

	/*
	 * qos mapping can updated only after flow entry is added. It is added at the same index
	 * as that of flow entry.
	 */
	if (!ppe_drv_flow_v6_qos_set(pcf, flow)) {
		ppe_drv_warn("%p: qos mapping failed for flow: %p", pcf, flow);
		goto flow_add_fail;
	}

#ifdef PPE_DRV_FLOW_IG_MAC_WAR
	/*
	 * Configure the MY_MAC during flow creation.
	 */
	if (!ppe_drv_v6_conn_flow_igmac_add(pcf)) {
		ppe_drv_warn("%p: flow entry valid set failed for flow: %p", pcf, flow);
		goto flow_add_fail;
	}
#endif

	/*
	 * Now the QOS mapping is set, mark the flow entry as valid.
	 */
	if (!ppe_drv_flow_valid_set(flow, true)) {
		ppe_drv_warn("%p: flow entry valid set failed for flow: %p", pcf, flow);
		goto flow_add_fail;
	}

	/*
	 * PPE priority assist flow requires only flow entry, QOS and flow valid to be set.
	 */
	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_PRIORITY_PPE_ASSIST)) {
		ppe_drv_host_dump(flow->host);
		ppe_drv_flow_dump(flow);
		return flow;
	}

	/*
	 * Update stats
	 */
	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_BRIDGE_FLOW)) {
		ppe_drv_stats_inc(&p->stats.gen_stats.v6_l2_flows);
	} else {
		ppe_drv_stats_inc(&p->stats.gen_stats.v6_l3_flows);
	}

	tx_port = ppe_drv_v6_conn_flow_tx_port_get(pcf);
	rx_port = ppe_drv_v6_conn_flow_rx_port_get(pcf);

	/*
	 * Update the number of VP and DS flows.
	 */
	if (ppe_drv_port_flags_check(tx_port, PPE_DRV_PORT_FLAG_WIFI_DEV) ||
			ppe_drv_port_flags_check(rx_port, PPE_DRV_PORT_FLAG_WIFI_DEV)) {
		ppe_drv_stats_inc(&p->stats.gen_stats.v6_vp_wifi_flows);
	}

	if ((tx_port->user_type == PPE_DRV_PORT_USER_TYPE_DS) ||
			(rx_port->user_type == PPE_DRV_PORT_USER_TYPE_DS)) {
		ppe_drv_stats_inc(&p->stats.gen_stats.v6_ds_flows);
	}

	ppe_drv_host_dump(flow->host);
	ppe_drv_flow_dump(flow);
	if (flow->nh) {
		ppe_drv_nexthop_dump(flow->nh);
	}

	return flow;

flow_add_fail:
#ifdef PPE_DRV_FLOW_IG_MAC_WAR
	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_IGMAC_VALID)) {
		ppe_drv_v6_conn_flow_igmac_del(pcf);
	}
#endif

	if (flow) {
		ppe_drv_flow_del(flow);
	}

	if (host) {
		ppe_drv_host_deref(host);
	}

	if (nh) {
		ppe_drv_nexthop_deref(nh);
	}

	return NULL;
}

/*
 * ppe_drv_v6_passive_vp_flow()
 *	check if the flow is for a Passive VP
 */
static bool ppe_drv_v6_passive_vp_flow(struct ppe_drv_v6_rule_create *create) {
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_port *tx_pp = NULL;
	struct ppe_drv_port *rx_pp = NULL;
	struct ppe_drv_iface *if_rx, *if_tx;
	bool is_tx_vp = false;
	bool is_rx_vp = false;

	if_rx = ppe_drv_iface_get_by_idx(create->conn_rule.rx_if);
	if (!if_rx) {
		ppe_drv_warn("%p: No PPE interface corresponding to rx_if: %d", create, create->conn_rule.rx_if);
		return false;
	}

	if_tx = ppe_drv_iface_get_by_idx(create->conn_rule.tx_if);
	if (!if_tx) {
		ppe_drv_warn("%p: No PPE interface corresponding to tx_if: %d", create, create->conn_rule.tx_if);
		return false;
	}

	tx_pp = ppe_drv_iface_port_get(if_tx);
	if (!tx_pp) {
		ppe_drv_warn("%p: create failed invalid TX interface hierarchy: %p", p, create);
		return false;
	}

	rx_pp = ppe_drv_iface_port_get(if_rx);
	if (!rx_pp) {
		ppe_drv_warn("%p: create failed invalid RX interface hierarchy: %p", p, create);
		return false;
	}

	is_tx_vp = PPE_DRV_VIRTUAL_PORT_CHK(tx_pp->port);
	is_rx_vp = PPE_DRV_VIRTUAL_PORT_CHK(rx_pp->port);

	if (is_tx_vp || is_rx_vp) {
		if (is_tx_vp) {
			if (tx_pp->user_type == PPE_DRV_PORT_USER_TYPE_PASSIVE_VP)
				return true;
		}

		if (is_rx_vp) {
			if (rx_pp->user_type == PPE_DRV_PORT_USER_TYPE_PASSIVE_VP)
				return true;
		}
	}

	return false;
}

/*
 * ppe_drv_v6_conn_sync_one()
 *	Sync stats for a single connection.
 */
void ppe_drv_v6_conn_sync_one(struct ppe_drv_v6_conn *cn, struct ppe_drv_v6_conn_sync *cns,
		enum ppe_drv_stats_sync_reason reason)
{
	struct ppe_drv_v6_conn_flow *pcf = &cn->pcf;
	struct ppe_drv_v6_conn_flow *pcr = &cn->pcr;

	/*
	 * Fill 5-tuple connection rule from ppe_drv_v6_conn_flow to cns.
	 */
	cns->protocol = ppe_drv_v6_conn_flow_match_protocol_get(pcf);
	cns->flow_ident = ppe_drv_v6_conn_flow_match_src_ident_get(pcf);
	cns->return_ident = ppe_drv_v6_conn_flow_match_dest_ident_get(pcf);
	ppe_drv_v6_conn_flow_match_src_ip_get(pcf, cns->flow_ip);
	ppe_drv_v6_conn_flow_match_dest_ip_get(pcf, cns->return_ip);

	/*
	 * Fill reason for sync
	 */
	cns->reason = reason;

	/*
	 * Update stats for each direction
	 */
	ppe_drv_v6_conn_flow_rx_stats_get(pcf, &cns->flow_rx_packet_count, &cns->flow_rx_byte_count);
	ppe_drv_v6_conn_flow_tx_stats_get(pcf, &cns->flow_tx_packet_count, &cns->flow_tx_byte_count);
	ppe_drv_v6_conn_flow_rx_stats_sub(pcf, cns->flow_rx_packet_count, cns->flow_rx_byte_count);
	ppe_drv_v6_conn_flow_tx_stats_sub(pcf, cns->flow_tx_packet_count, cns->flow_tx_byte_count);

	/*
	 * Update the status for return flow, if it exist.
	 */
	if (ppe_drv_v6_conn_flags_check(cn, PPE_DRV_V6_CONN_FLAG_RETURN_VALID)) {
		ppe_drv_v6_conn_flow_rx_stats_get(pcr, &cns->return_rx_packet_count, &cns->return_rx_byte_count);
		ppe_drv_v6_conn_flow_tx_stats_get(pcr, &cns->return_tx_packet_count, &cns->return_tx_byte_count);
		ppe_drv_v6_conn_flow_rx_stats_sub(pcr, cns->return_rx_packet_count, cns->return_rx_byte_count);
		ppe_drv_v6_conn_flow_tx_stats_sub(pcr, cns->return_tx_packet_count, cns->return_tx_byte_count);
	} else {
		cns->return_rx_packet_count = 0;
		cns->return_rx_byte_count = 0;
		cns->return_tx_packet_count = 0;
		cns->return_tx_byte_count = 0;
	}
}

/*
 * ppe_drv_v6_get_conn_stats()
 *	Sync stats for a given five tuple connection.
 */
ppe_drv_ret_t ppe_drv_v6_get_conn_stats(struct ppe_drv_v6_flow_conn_stats *conn_stats)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_flow *flow = NULL;
	struct ppe_drv_v6_conn *cn;
	struct ppe_drv_v6_conn_flow *pcf;
	struct ppe_drv_comm_stats *stats;

	stats = &p->stats.comm_stats[PPE_DRV_CONN_TYPE_FLOW];

	/*
	 * Get flow table entry.
	 */
	spin_lock_bh(&p->lock);
	flow = ppe_drv_flow_v6_get(&conn_stats->tuple);
	if (!flow) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&stats->v6_stats_conn_not_found);
		ppe_drv_warn("%p: flow entry not found", p);
		return PPE_DRV_RET_FAILURE_NO_MATCHING_CONN;
	}

	pcf = flow->pcf.v6;

	/*
	 * Get connection.
	 */
	cn = ppe_drv_v6_conn_flow_conn_get(pcf);

	/*
	 * Get stats of this connection.
	 */
	ppe_drv_v6_conn_sync_one(cn, &conn_stats->conn_sync, PPE_DRV_STATS_SYNC_REASON_STATS);

	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_v6_get_conn_stats);

/*
 * ppe_drv_v6_conn_sync_many()
 *	API to sync a specific number of connection stats
 */
void ppe_drv_v6_conn_sync_many(struct ppe_drv_v6_conn_sync_many *cn_syn, uint8_t num_conn)
{
	uint8_t count = 0;
	bool return_flow_valid;
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_v6_conn *cn;
	enum ppe_drv_stats_sync_reason reason = PPE_DRV_STATS_SYNC_REASON_STATS;
	uint16_t max_flow_conn_count = num_conn - PPE_DRV_TUN_PORT_STATS_RESERVED_COUNT;

	spin_lock_bh(&p->lock);
	if (list_empty(&p->conn_v6) && list_empty(&p->conn_tun_v6)) {
		spin_unlock_bh(&p->lock);
		return;
	}

	/*
	 * Traverse through active list of connection.
	 */
	list_for_each_entry(cn, &p->conn_v6, list) {
		/*
		 * Skip if
		 *	- Stats are already synced for this connection in previous iteration.
		 *	- Or there is no change in the stats from previous read.
		 */
		return_flow_valid = ppe_drv_v6_conn_flags_check(cn, PPE_DRV_V6_CONN_FLAG_RETURN_VALID);
		if ((cn->toggle == p->toggled_v6) || !(atomic_read(&cn->pcf.rx_packets)
				|| (return_flow_valid && atomic_read(&cn->pcr.rx_packets)))){
			if (list_is_last(&cn->list, &p->conn_v6)) {
				p->toggled_v6 = !p->toggled_v6;
				break;
			}

			continue;
		}

		/*
		 * sync stats for this connection.
		 */
		ppe_drv_v6_conn_sync_one(cn, &cn_syn->conn_sync[count], reason);
		count++;

		/*
		 * Flip the toggle bit to avoid syncing the stats for this connection until
		 * one full iteration of active list is done
		 */
		cn->toggle = !cn->toggle;

		/*
		 * If budget reached, break
		 */
		if (count == max_flow_conn_count) {
			break;
		}

		/*
		 * If we reached to the end of the list, flip the toggled bit
		 * for the next interation
		 */
		if (list_is_last(&cn->list, &p->conn_v6)) {
			p->toggled_v6 = !p->toggled_v6;
		}
	}

	/*
	 * Traverse through active list of tunnel connections.
	 */
	list_for_each_entry(cn, &p->conn_tun_v6, list) {
		/*
		 * Skip if stats are already synced for this connection in previous iteration.
		 */
		return_flow_valid = ppe_drv_v6_conn_flags_check(cn, PPE_DRV_V6_CONN_FLAG_RETURN_VALID);

		/*
		 * check if there are connections that need stats update; if yes then
		 * invoke stats_sync api for all those connections
		 */
		if (cn->toggle == p->tun_toggled_v6 || !(atomic_read(&cn->pcf.rx_packets) || atomic_read(&cn->pcf.tx_packets)
					|| (return_flow_valid && atomic_read(&cn->pcr.rx_packets))
					|| (return_flow_valid && atomic_read(&cn->pcr.tx_packets)))) {
			if (list_is_last(&cn->list, &p->conn_tun_v6)) {
				p->tun_toggled_v6 = !p->tun_toggled_v6;
				break;
			}

			continue;
		}

		/*
		 * sync stats for this connection.
		 */
		ppe_drv_v6_conn_sync_one(cn, &cn_syn->conn_sync[count], reason);
		count++;

		/*
		 * Flip the toggle bit to avoid syncing the stats for this connection until
		 * one full iteration of active list is done
		 */
		cn->toggle = !cn->toggle;

		/*
		 * If budget reached, break
		 */
		if (count == num_conn)
			break;

		/*
		 * If we reached to the end of the list, flip the toggled bit
		 * for the next interation
		 */
		if (list_is_last(&cn->list, &p->conn_tun_v6)) {
			p->tun_toggled_v6 = !p->tun_toggled_v6;
		}
	}

	spin_unlock_bh(&p->lock);
	cn_syn->count = count;
}
EXPORT_SYMBOL(ppe_drv_v6_conn_sync_many);

/*
 * ppe_drv_v6_conn_stats_sync_invoke_cb()
 *	Invoke cb
 */
void ppe_drv_v6_conn_stats_sync_invoke_cb(struct ppe_drv_v6_conn_sync *cns)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	ppe_drv_v6_sync_callback_t sync_cb;
	void *sync_data;

	spin_lock_bh(&p->lock);
	sync_cb = p->ipv6_stats_sync_cb;
	sync_data = p->ipv6_stats_sync_data;
	spin_unlock_bh(&p->lock);

	if (sync_cb) {
		sync_cb(sync_data, cns);
		return;
	}

	ppe_drv_trace("%p: No callback registered for stats sync for cns: %p", p, cns);
}

/*
 * ppe_drv_v6_stats_callback_unregister()
 * 	Un-Register a notifier callback for IPv6 stats from PPE
 */
void ppe_drv_v6_stats_callback_unregister(void)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	/*
	 * Unregister our sync callback.
	 */
	spin_lock_bh(&p->lock);
	if (p->ipv6_stats_sync_cb) {
		p->ipv6_stats_sync_cb = NULL;
		p->ipv6_stats_sync_data = NULL;
	}

	spin_unlock_bh(&p->lock);
	ppe_drv_info("%p: stats callback unregistered, cb:", p);
}
EXPORT_SYMBOL(ppe_drv_v6_stats_callback_unregister);

/*
 * ppe_drv_v6_stats_callback_register()
 * 	Register a notifier callback for IPv6 stats from PPE
 */
bool ppe_drv_v6_stats_callback_register(ppe_drv_v6_sync_callback_t cb, void *app_data)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	spin_lock_bh(&p->lock);
	p->ipv6_stats_sync_cb = cb;
	p->ipv6_stats_sync_data = app_data;
	spin_unlock_bh(&p->lock);

	ppe_drv_info("%p: stats callback registered, cb: %p", p, cb);
	return true;
}
EXPORT_SYMBOL(ppe_drv_v6_stats_callback_register);

/*
 * ppe_drv_v6_mc_destroy()
 *	Destroy a multicast connection entry in PPE.
 */
ppe_drv_ret_t ppe_drv_v6_mc_destroy(struct ppe_drv_v6_rule_destroy *destroy)
{
	return PPE_DRV_RET_FAILURE_NOT_SUPPORTED;
}

/*
 * ppe_drv_v6_mc_update()
 *	Update member list of ports in l2 multicast group.
 */
ppe_drv_ret_t ppe_drv_v6_mc_update(struct ppe_drv_v6_rule_create *update)
{
	return PPE_DRV_RET_FAILURE_NOT_SUPPORTED;
}

/*
 * ppe_drv_v6_mc_create()
 *	Adds a l2 multicast flow and host entry in PPE.
 */
ppe_drv_ret_t ppe_drv_v6_mc_create(struct ppe_drv_v6_rule_create *create)
{
	return PPE_DRV_RET_FAILURE_NOT_SUPPORTED;
}

/*
 * ppe_drv_v6_flush()
 *	Flush a connection entry in PPE.
 */
ppe_drv_ret_t ppe_drv_v6_flush(struct ppe_drv_v6_conn *cn)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_v6_conn_flow *pcf = &cn->pcf;
	struct ppe_drv_v6_conn_flow *pcr = &cn->pcr;

	/*
	 * Update stats
	 */
	ppe_drv_stats_inc(&p->stats.gen_stats.v6_flush_req);

	/*
	 * Get flow table entry.
	 */
	if (pcf && !ppe_drv_v6_flow_del(pcf)) {
		ppe_drv_stats_inc(&p->stats.gen_stats.v6_flush_fail);
		ppe_drv_warn("%p: deletion of flow failed: %p", p, pcf);
		return PPE_DRV_RET_FAILURE_FLUSH_FAIL;
	}

	/*
	 * Release references on interfaces.
	 */
	if (pcf) {
		ppe_drv_v6_if_walk_release(pcf);
	}

	/*
	 * Find the other flow associated with this connection.
	 */
	cn = ppe_drv_v6_conn_flow_conn_get(pcf);
	pcr = (pcf == &cn->pcf) ? &cn->pcr : &cn->pcf;

	if (pcr && !ppe_drv_v6_flow_del(pcr)) {
		ppe_drv_stats_inc(&p->stats.gen_stats.v6_flush_fail);
		ppe_drv_warn("%p: deletion of return flow failed: %p", p, pcf);
		return PPE_DRV_RET_FAILURE_FLUSH_FAIL;
	}

	/*
	 * Release references on interfaces.
	 */
	if (pcr) {
		ppe_drv_v6_if_walk_release(pcr);
	}

	/*
	 * Delete connection entry to the active connection list.
	 */
	list_del(&cn->list);

	return PPE_DRV_RET_SUCCESS;
}

/*
 * ppe_drv_v6_rfs_destroy()
 * 	Destroy a rfs connection entry in PPE.
 *
 * This function is deprecated, Please use "ppe_drv_v6_assist_rule_destroy"
 * API to destroy RFS flows
 */
ppe_drv_ret_t ppe_drv_v6_rfs_destroy(struct ppe_drv_v6_rule_destroy *destroy)
{
	struct ppe_drv_comm_stats *comm_stats;
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_flow *flow = NULL;
	struct ppe_drv_v6_conn_flow *pcf;
	struct ppe_drv_v6_conn *cn;

	comm_stats = &p->stats.comm_stats[PPE_DRV_CONN_TYPE_FLOW];

	/*
	 * Update stats
	 */
	ppe_drv_stats_inc(&comm_stats->v6_destroy_rfs_req);

	/*
	 * Get flow table entry.
	 */
	spin_lock_bh(&p->lock);
	flow = ppe_drv_flow_v6_get(&destroy->tuple);
	if (!flow) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&comm_stats->v6_destroy_rfs_conn_not_found);
		ppe_drv_warn("%p: flow entry not found", p);
		return PPE_DRV_RET_FAILURE_DESTROY_NO_CONN;
	}

	pcf = flow->pcf.v6;
	cn = ppe_drv_v6_conn_flow_conn_get(pcf);
	if (!ppe_drv_v6_flow_del(pcf)) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&comm_stats->v6_destroy_rfs_fail);
		ppe_drv_warn("%p: deletion of flow failed: %p", p, pcf);
		return PPE_DRV_RET_FAILURE_DESTROY_FAIL;
	}

	if (pcf->eg_port_if) {
		ppe_drv_iface_deref_internal(pcf->eg_port_if);
		pcf->eg_port_if = NULL;
	}

	if (pcf->in_l3_if) {
		ppe_drv_iface_deref_internal(pcf->in_l3_if);
		pcf->in_l3_if = NULL;
	}

	spin_unlock_bh(&p->lock);

	/*
	 * Free the connection entry memory.
	 */
	ppe_drv_v6_conn_free(cn);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_v6_rfs_destroy);

/*
 * ppe_drv_v6_assist_rule_destroy()
 *	Destroy a connection entry in PPE.
 */
ppe_drv_ret_t ppe_drv_v6_assist_rule_destroy(struct ppe_drv_v6_rule_destroy *destroy)
{
	struct ppe_drv_comm_stats *comm_stats;
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_flow *flow = NULL;
	struct ppe_drv_v6_conn_flow *pcf;
	struct ppe_drv_v6_conn *cn;

	comm_stats = &p->stats.comm_stats[PPE_DRV_CONN_TYPE_FLOW];

	/*
	 * Update stats
	 */
	ppe_drv_stats_inc(&comm_stats->v6_assist_rule_destroy_req);

	/*
	 * Get flow table entry.
	 */
	spin_lock_bh(&p->lock);
	flow = ppe_drv_flow_v6_get(&destroy->tuple);
	if (!flow) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&comm_stats->v6_assist_rule_destroy_conn_not_found);
		ppe_drv_warn("%p: flow entry not found", p);
		return PPE_DRV_RET_FAILURE_DESTROY_NO_CONN;
	}

	pcf = flow->pcf.v6;
	cn = ppe_drv_v6_conn_flow_conn_get(pcf);
	if (!ppe_drv_v6_flow_del(pcf)) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&comm_stats->v6_assist_rule_destroy_fail);
		ppe_drv_warn("%p: deletion of flow failed: %p", p, pcf);
		return PPE_DRV_RET_FAILURE_DESTROY_FAIL;
	}

	if (pcf->eg_port_if) {
		ppe_drv_iface_deref_internal(pcf->eg_port_if);
		pcf->eg_port_if = NULL;
	}

	if (pcf->in_l3_if) {
		ppe_drv_iface_deref_internal(pcf->in_l3_if);
		pcf->in_l3_if = NULL;
	}
	spin_unlock_bh(&p->lock);

	/*
	 * Free the connection entry memory.
	 */
	ppe_drv_v6_conn_free(cn);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_v6_assist_rule_destroy);

/*
 * ppe_drv_v6_policer_flow_destroy()
 *	Destroy a policer connection entry in PPE.
 */
ppe_drv_ret_t ppe_drv_v6_policer_flow_destroy(struct ppe_drv_v6_rule_destroy *destroy)
{
	struct ppe_drv_comm_stats *comm_stats;
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_flow *flow = NULL;
	struct ppe_drv_v6_conn_flow *pcf;
	struct ppe_drv_v6_conn_flow *pcr;
	struct ppe_drv_v6_conn *cn;

	comm_stats = &p->stats.comm_stats[PPE_DRV_CONN_TYPE_FLOW];

	/*
	 * Update stats
	 */
	ppe_drv_stats_inc(&comm_stats->v6_destroy_policer_req);

	/*
	 * Get flow table entry.
	 */
	spin_lock_bh(&p->lock);
	flow = ppe_drv_flow_v6_get(&destroy->tuple);
	if (!flow) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&comm_stats->v6_destroy_policer_conn_not_found);
		ppe_drv_warn("%p: flow entry not found", p);
		return PPE_DRV_RET_FAILURE_DESTROY_NO_CONN;
	}

	pcf = flow->pcf.v6;
	cn = ppe_drv_v6_conn_flow_conn_get(pcf);

	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_PPE_POLICER_ASSIST)) {
		if (!ppe_drv_v6_flow_del(pcf)) {
			spin_unlock_bh(&p->lock);
			ppe_drv_stats_inc(&comm_stats->v6_destroy_policer_fail);
			ppe_drv_warn("%p: deletion of flow failed: %p", p, pcf);
			return PPE_DRV_RET_FAILURE_DESTROY_FAIL;
		}

		if (pcf->eg_port_if) {
			ppe_drv_iface_deref_internal(pcf->eg_port_if);
			pcf->eg_port_if = NULL;
		}

		if (pcf->in_l3_if) {
			ppe_drv_iface_deref_internal(pcf->in_l3_if);
			pcf->in_l3_if = NULL;
		}
	}

	pcr = (pcf == &cn->pcf) ? &cn->pcr : &cn->pcf;

	/*
	 * Find the other flow associated with this connection.
	 */
	if (pcr && ppe_drv_v6_conn_flow_flags_check(pcr, PPE_DRV_V6_CONN_FLAG_FLOW_PPE_POLICER_ASSIST)) {
		if (!ppe_drv_v6_flow_del(pcr)) {
			spin_unlock_bh(&p->lock);
			ppe_drv_stats_inc(&comm_stats->v6_destroy_policer_fail);
			ppe_drv_warn("%p: deletion of return flow failed: %p", p, pcr);
			return PPE_DRV_RET_FAILURE_DESTROY_FAIL;
		}

		if (pcr->eg_port_if) {
			ppe_drv_iface_deref_internal(pcr->eg_port_if);
			pcr->eg_port_if = NULL;
		}

		if (pcr->in_l3_if) {
			ppe_drv_iface_deref_internal(pcr->in_l3_if);
			pcr->in_l3_if = NULL;
		}
	}

	spin_unlock_bh(&p->lock);

	/*
	 * Check if this flow is combined with ACL for n-tuple lookup.
	 */
	if (!ppe_drv_v6_unbind_acl_policer(cn)) {
		ppe_drv_stats_inc(&comm_stats->v6_destroy_policer_fail_acl);
		ppe_drv_warn("%p: failed to unlink with ACL, destroy object: %p", p, destroy);
	}

	/*
	 * Free the connection entry memory.
	 */
	ppe_drv_v6_conn_free(cn);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_v6_policer_flow_destroy);

/*
 * ppe_drv_v6_policer_flow_create()
 *	Adds a connection entry in PPE.
 */
ppe_drv_ret_t ppe_drv_v6_policer_flow_create(struct ppe_drv_v6_rule_create *create)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_v6_conn_flow *pcf = NULL;
	struct ppe_drv_v6_conn_flow *pcr = NULL;
	struct ppe_drv_comm_stats *comm_stats;
	struct ppe_drv_v6_conn *cn = NULL;
	ppe_drv_ret_t ret;
	struct ppe_drv_top_if_rule top_if = {0};

	comm_stats = &p->stats.comm_stats[PPE_DRV_CONN_TYPE_FLOW];

	/*
	 * Update stats
	 */
	ppe_drv_stats_inc(&comm_stats->v6_create_policer_req);

	/*
	 * Allocate a new connection entry
	 */
	cn = ppe_drv_v6_conn_alloc();
	if (!cn) {
		ppe_drv_stats_inc(&comm_stats->v6_create_policer_fail_mem);
		ppe_drv_warn("%p: failed to allocate connection memory: %p", p, create);
		return PPE_DRV_RET_FAILURE_CREATE_OOM;
	}

	/*
	 * Check if this flow is combined with ACL for n-tuple lookup.
	 */
	if (!ppe_drv_v6_bind_acl_policer(create, cn)) {
		ppe_drv_stats_inc(&comm_stats->v6_create_policer_fail_acl);
		ppe_drv_warn("%p: failed to combine with ACL, connection object: %p", p, create);
		kfree(cn);
		return PPE_DRV_RET_POLICER_RULE_BIND_FAIL;
	}

	/*
	 * Fill the connection entry.
	 */
	spin_lock_bh(&p->lock);

	top_if.rx_if = create->top_rule.rx_if;
	top_if.tx_if = create->top_rule.tx_if;
	ret = ppe_drv_v6_policer_conn_fill(create, &top_if, cn, PPE_DRV_CONN_TYPE_FLOW);
	if (ret != PPE_DRV_RET_SUCCESS) {
		ppe_drv_stats_inc(&comm_stats->v6_create_policer_fail_conn);
		ppe_drv_warn("%p: failed to fill connection object: %p", p, create);
		spin_unlock_bh(&p->lock);
		kfree(cn);
		return ret;
	}

	/*
	 * Ensure either direction flow is not already offloaded by us.
	 */
	if (ppe_drv_v6_flow_check(&cn->pcf) || ppe_drv_v6_flow_check(&cn->pcr)) {
		ppe_drv_stats_inc(&comm_stats->v6_create_policer_fail_collision);
		ppe_drv_warn("%p: create collision detected: %p", p, create);
		ret = PPE_DRV_RET_FAILURE_CREATE_COLLISSION;
		ppe_drv_iface_deref_internal(cn->pcf.eg_port_if);
		ppe_drv_iface_deref_internal(cn->pcf.in_l3_if);
		spin_unlock_bh(&p->lock);
		kfree(cn);
		return ret;
	}

	/*
	 * Add flow direction flow entry
	 */
	if (create->ap_rule.rule_id.policer.flags & PPE_DRV_VALID_FLAG_FLOW_POLICER) {
		ppe_drv_trace("calling flow add for flow\n");
		pcf = &cn->pcf;
		ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_PPE_POLICER_ASSIST);
		pcf->pf = ppe_drv_v6_flow_add(pcf);
		if (!pcf->pf) {
			ppe_drv_stats_inc(&comm_stats->v6_create_policer_fail);
			ppe_drv_warn("%p: acceleration of flow failed: %p", p, pcf);
			ret = PPE_DRV_RET_FAILURE_FLOW_ADD_FAIL;
			ppe_drv_iface_deref_internal(pcf->eg_port_if);
			ppe_drv_iface_deref_internal(cn->pcf.in_l3_if);
			spin_unlock_bh(&p->lock);
			kfree(cn);
			ppe_drv_v6_conn_flow_flags_clear(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_PPE_POLICER_ASSIST);
			return ret;
		}

		pcf->conn = cn;
	}

	if (create->ap_rule.rule_id.policer.flags & PPE_DRV_VALID_FLAG_FLOW_POLICER) {
		ppe_drv_trace("calling return add for flow\n");
		pcr = &cn->pcr;
		ppe_drv_v6_conn_flow_flags_set(pcr, PPE_DRV_V6_CONN_FLAG_FLOW_PPE_POLICER_ASSIST);
		pcr->pf = ppe_drv_v6_flow_add(pcr);
		if (!pcr->pf) {
			/*
			 * Destroy the offloaded flow entry
			 */
			ppe_drv_v6_flow_del(pcf);
			pcf->pf = NULL;

			ppe_drv_stats_inc(&comm_stats->v6_create_policer_fail);
			ppe_drv_warn("%p: acceleration of return direction failed: %p", p, pcr);
			ret = PPE_DRV_RET_FAILURE_FLOW_ADD_FAIL;
			ppe_drv_v6_conn_flow_flags_clear(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_PPE_POLICER_ASSIST);
			return ret;
		}

		/*
		 * Add connection entry to the active connection list.
		 */
		pcr->conn = cn;
		ppe_drv_trace("setting pcr ppe assist patch\n");
	}

	cn->pcr.pf = NULL;
	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_v6_policer_flow_create);

/*
 * ppe_drv_v6_destroy()
 *	Destroy a connection entry in PPE.
 */
ppe_drv_ret_t ppe_drv_v6_destroy(struct ppe_drv_v6_rule_destroy *destroy)
{
	struct ppe_drv_comm_stats *comm_stats;
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_flow *flow = NULL;
	struct ppe_drv_v6_conn_flow *pcf;
	struct ppe_drv_v6_conn_flow *pcr;
	struct ppe_drv_v6_conn_sync *cns;
	struct ppe_drv_v6_conn *cn;

#ifdef PPE_TUNNEL_ENABLE
	ppe_drv_ret_t ret;

	/*
	 * Check if the destroy rule is for tunnel (outer rule)
	 */
	ret = ppe_drv_v6_tun_del_ce_notify(destroy);
	if (ret != PPE_DRV_RET_FAILURE_DESTROY_NO_CONN) {
		if (ret != PPE_DRV_RET_SUCCESS) {
			ppe_drv_warn("%p: Tunnel destroy failed with error %d", destroy, ret);
		}

		return ret;
	}
#endif
	comm_stats = &p->stats.comm_stats[PPE_DRV_CONN_TYPE_FLOW];

	/*
	 * Update stats
	 */
	ppe_drv_stats_inc(&comm_stats->v6_destroy_req);

	/*
	 * Get flow table entry.
	 */
	spin_lock_bh(&p->lock);

	flow = ppe_drv_flow_v6_get(&destroy->tuple);
	if (!flow) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&comm_stats->v6_destroy_conn_not_found);
		ppe_drv_warn("%p: flow entry not found", p);
		return PPE_DRV_RET_FAILURE_DESTROY_NO_CONN;
	}

	pcf = flow->pcf.v6;

	if (!ppe_drv_v6_flow_del(pcf)) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&comm_stats->v6_destroy_fail);
		ppe_drv_warn("%p: deletion of flow failed: %p", p, pcf);
		return PPE_DRV_RET_FAILURE_DESTROY_FAIL;
	}

	/*
	 * Release references on interfaces.
	 */
	ppe_drv_v6_if_walk_release(pcf);

	/*
	 * Find the other flow associated with this connection.
	 */
	cn = ppe_drv_v6_conn_flow_conn_get(pcf);
	pcr = (pcf == &cn->pcf) ? &cn->pcr : &cn->pcf;

	if (!ppe_drv_v6_flow_del(pcr)) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&comm_stats->v6_destroy_fail);
		ppe_drv_warn("%p: deletion of return flow failed: %p", p, pcf);
		return PPE_DRV_RET_FAILURE_DESTROY_FAIL;
	}

	/*
	 * Release references on interfaces.
	 */
	ppe_drv_v6_if_walk_release(pcr);

	/*
	 * Delete connection entry to the active connection list.
	 */
	list_del(&cn->list);

	/*
	 * Capture remaining stats.
	 */
	cns = ppe_drv_v6_conn_stats_alloc();
	if (cns) {
		ppe_drv_v6_conn_sync_one(cn, cns, PPE_DRV_STATS_SYNC_REASON_DESTROY);
	}

	spin_unlock_bh(&p->lock);

	/*
	 * Check if this flow is combined with ACL for n-tuple lookup.
	 */
	if (!ppe_drv_v6_unbind_acl_policer(cn)) {
		ppe_drv_warn("%p: failed to unlink with ACL, destroy object: %p", p, destroy);
	}

	if (cns) {
		ppe_drv_v6_conn_stats_sync_invoke_cb(cns);
		ppe_drv_v6_conn_stats_free(cns);
	}

	/*
	 * We maintain reference per connection on main ppe context.
	 * Dereference: connection destroy.
	 *
	 * TODO: check if this is needed
	 */
	/* ppe_drv_deref(p); */
	ppe_drv_v6_conn_free(cn);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_v6_destroy);

/*
 * ppe_drv_v6_rfs_create()
 *      Adds a connection entry in PPE.
 *
 * This function is deprecated, Please use "ppe_drv_v6_assist_rule_create"
 * API for configuring RFS flows
 */
ppe_drv_ret_t ppe_drv_v6_rfs_create(struct ppe_drv_v6_rule_create *create)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_v6_conn_flow *pcf = NULL;
	struct ppe_drv_comm_stats *comm_stats;
	struct ppe_drv_v6_conn *cn = NULL;
	ppe_drv_ret_t ret;
	struct ppe_drv_top_if_rule top_if = {0};

	comm_stats = &p->stats.comm_stats[PPE_DRV_CONN_TYPE_FLOW];

	/*
	 * Update stats
	 */
	ppe_drv_stats_inc(&comm_stats->v6_create_rfs_req);

	/*
	 * Allocate a new connection entry
	 */
	cn = ppe_drv_v6_conn_alloc();
	if (!cn) {
		ppe_drv_stats_inc(&comm_stats->v6_create_rfs_fail_mem);
		ppe_drv_warn("%p: failed to allocate connection memory: %p", p, create);
		return PPE_DRV_RET_FAILURE_CREATE_OOM;
	}

	/*
	 * Fill the connection entry.
	 */
	spin_lock_bh(&p->lock);

	top_if.rx_if = create->top_rule.rx_if;
	top_if.tx_if = create->top_rule.tx_if;
	ret = ppe_drv_v6_rfs_conn_fill(create, &top_if, cn, PPE_DRV_CONN_TYPE_FLOW);
	if (ret != PPE_DRV_RET_SUCCESS) {
		ppe_drv_stats_inc(&comm_stats->v6_create_rfs_fail_conn);
		ppe_drv_warn("%p: failed to fill connection object: %p", p, create);
		goto fail;
	}

	/*
	 * Ensure either direction flow is not already offloaded by us.
	 */
	if (ppe_drv_v6_flow_check(&cn->pcf)) {
		ppe_drv_stats_inc(&comm_stats->v6_create_rfs_fail_collision);
		ppe_drv_warn("%p: create collision detected: %p", p, create);
		ppe_drv_iface_deref_internal(cn->pcf.eg_port_if);
		ppe_drv_iface_deref_internal(cn->pcf.in_l3_if);
		ret = PPE_DRV_RET_FAILURE_CREATE_COLLISSION;
		goto fail;
	}

	pcf = &cn->pcf;
	ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_RFS_PPE_ASSIST);

	/*
	 * Add flow direction flow entry
	 */
	pcf->pf = ppe_drv_v6_flow_add(pcf);
	if (!pcf->pf) {
		ppe_drv_stats_inc(&comm_stats->v6_create_rfs_fail);
		ppe_drv_warn("%p: acceleration of flow failed: %p", p, pcf);
		if (pcf->eg_port_if) {
			ppe_drv_iface_deref_internal(pcf->eg_port_if);
		}

		if (cn->pcf.in_l3_if) {
			ppe_drv_iface_deref_internal(cn->pcf.in_l3_if);
		}

		ret = PPE_DRV_RET_FAILURE_FLOW_ADD_FAIL;
		ppe_drv_v6_conn_flow_flags_clear(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_RFS_PPE_ASSIST);
		goto fail;
	}

	pcf->conn = cn;
	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;
fail:
	spin_unlock_bh(&p->lock);
	kfree(cn);
	return ret;
}
EXPORT_SYMBOL(ppe_drv_v6_rfs_create);

/*
 * ppe_drv_v6_assist_rule_create()
 *	Adds a connection entry in PPE.
 */
ppe_drv_ret_t ppe_drv_v6_assist_rule_create(struct ppe_drv_v6_rule_create *create, uint32_t feature)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_v6_conn_flow *pcf = NULL;
	struct ppe_drv_comm_stats *comm_stats;
	struct ppe_drv_v6_conn *cn = NULL;
	ppe_drv_ret_t ret;
	struct ppe_drv_top_if_rule top_if = {0};

	comm_stats = &p->stats.comm_stats[PPE_DRV_CONN_TYPE_FLOW];

	/*
	 * Update stats
	 */
	ppe_drv_stats_inc(&comm_stats->v6_assist_rule_create_req);

	/*
	 * PPE_DRV_ASSIST_FEATURE_PRIORITY flag must be set for flows which only require priority assist.
	 * To configure priority for RFS flows qos_tag information must be updated for RFS rule.
	 */
	if (!(ppe_drv_assist_feature_type_check(feature, PPE_DRV_ASSIST_FEATURE_RFS) ||
			ppe_drv_assist_feature_type_check(feature, PPE_DRV_ASSIST_FEATURE_PRIORITY))) {
		ppe_drv_warn("%p:Invalid assist type configuration %d\n", p, feature);
		return PPE_DRV_RET_FAILURE_INVALID_PARAM;
	}

	/*
	 * Allocate a new connection entry
	 */
	cn = ppe_drv_v6_conn_alloc();
	if (!cn) {
		ppe_drv_stats_inc(&comm_stats->v6_assist_rule_create_fail_mem);
		ppe_drv_warn("%p: failed to allocate connection memory: %p", p, create);
		return PPE_DRV_RET_FAILURE_CREATE_OOM;
	}

	/*
	 * Fill the connection entry.
	 */
	spin_lock_bh(&p->lock);

	if (ppe_drv_assist_feature_type_check(feature, PPE_DRV_ASSIST_FEATURE_RFS)) {
		ppe_drv_stats_inc(&comm_stats->v6_create_rfs_req);
		top_if.rx_if = create->top_rule.rx_if;
		top_if.tx_if = create->top_rule.tx_if;
		ret = ppe_drv_v6_rfs_conn_fill(create, &top_if, cn, PPE_DRV_CONN_TYPE_FLOW);
		if (ret != PPE_DRV_RET_SUCCESS) {
			ppe_drv_stats_inc(&comm_stats->v6_assist_rule_create_rfs_fail_conn);
			ppe_drv_warn("%p: failed to fill connection object: %p", p, create);
			goto fail;
		}
	} else if (ppe_drv_assist_feature_type_check(feature, PPE_DRV_ASSIST_FEATURE_PRIORITY)) {
		ppe_drv_stats_inc(&comm_stats->v6_create_priority_req);
		ret = ppe_drv_v6_priority_conn_fill(create, cn, PPE_DRV_CONN_TYPE_FLOW);
		if (ret != PPE_DRV_RET_SUCCESS) {
			ppe_drv_stats_inc(&comm_stats->v6_assist_rule_create_priority_fail_conn);
			ppe_drv_warn("%p: failed to fill connection object: %p", p, create);
			goto fail;
		}
	}

	/*
	 * Ensure either direction flow is not already offloaded by us.
	 */
	if (ppe_drv_v6_flow_check(&cn->pcf)) {
		ppe_drv_stats_inc(&comm_stats->v6_assist_rule_create_fail_collision);
		ppe_drv_warn("%p: create collision detected: %p", p, create);
		ret = PPE_DRV_RET_FAILURE_CREATE_COLLISSION;
		goto fail;
	}

	pcf = &cn->pcf;
	if (ppe_drv_assist_feature_type_check(feature, PPE_DRV_ASSIST_FEATURE_RFS)) {
		ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_RFS_PPE_ASSIST);
	}

	/*
	 * Add flow direction flow entry
	 */
	pcf->pf = ppe_drv_v6_flow_add(pcf);
	if (!pcf->pf) {
		ppe_drv_stats_inc(&comm_stats->v6_assist_rule_create_fail);
		ppe_drv_warn("%p: acceleration of flow failed: %p", p, pcf);
		ret = PPE_DRV_RET_FAILURE_FLOW_ADD_FAIL;
		goto fail;
	}

	pcf->conn = cn;
	cn->pcr.pf = NULL;
	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;
fail:
	if (cn->pcf.eg_port_if) {
		ppe_drv_iface_deref_internal(cn->pcf.eg_port_if);
	}

	if (cn->pcf.in_l3_if) {
		ppe_drv_iface_deref_internal(cn->pcf.in_l3_if);
	}

	if (ppe_drv_assist_feature_type_check(feature, PPE_DRV_ASSIST_FEATURE_RFS)) {
		ppe_drv_v6_conn_flow_flags_clear(&cn->pcf, PPE_DRV_V6_CONN_FLAG_FLOW_RFS_PPE_ASSIST);
	} else if (ppe_drv_assist_feature_type_check(feature, PPE_DRV_ASSIST_FEATURE_PRIORITY)) {
		ppe_drv_v6_conn_flow_flags_clear(&cn->pcf, PPE_DRV_V6_CONN_FLAG_FLOW_PRIORITY_PPE_ASSIST);
	}

	spin_unlock_bh(&p->lock);
	kfree(cn);
	return ret;
}
EXPORT_SYMBOL(ppe_drv_v6_assist_rule_create);

/*
 * ppe_drv_v6_fse_flow_configure()
 *	FSE v6 flow programming
 */
bool ppe_drv_v6_fse_flow_configure(struct ppe_drv_v6_rule_create *create, struct ppe_drv_v6_conn_flow *pcf,
					struct ppe_drv_v6_conn_flow *pcr)
{
	struct ppe_drv *p = &ppe_drv_gbl;
        struct ppe_drv_fse_rule_info fse_info = {0};
	struct ppe_drv_fse_rule_info fse_info_return = {0};
        struct ppe_drv_v6_conn_flow *fse_cn = NULL;
	struct ppe_drv_port *rx_port = ppe_drv_v6_conn_flow_rx_port_get(pcf);
	struct ppe_drv_port *tx_port = ppe_drv_v6_conn_flow_tx_port_get(pcf);
	bool is_tx_ds = (tx_port->user_type == PPE_DRV_PORT_USER_TYPE_DS);
	bool is_rx_ds = (rx_port->user_type == PPE_DRV_PORT_USER_TYPE_DS);
	bool is_tx_active_vp = (tx_port->user_type == PPE_DRV_PORT_USER_TYPE_ACTIVE_VP);
	bool is_rx_active_vp = (rx_port->user_type == PPE_DRV_PORT_USER_TYPE_ACTIVE_VP);
	bool inter_vap = false;

	/*
	 * Check if Connection manager is setting DS flag in the rule; if yes then decision
	 * space to choose pcf or pcr needs to be done only for DS VP else flow could also
	 * be for active VP so decision logic need to be executed for both active and DS VP.
	 */
	if ((create->rule_flags & PPE_DRV_V6_RULE_FLAG_DS_FLOW)  == PPE_DRV_V6_RULE_FLAG_DS_FLOW) {
		if (is_rx_ds && !is_tx_ds) {
			ppe_drv_fill_fse_v6_tuple_info(pcf, &fse_info, true);
			fse_cn = pcf;
		} else if (is_tx_ds && !is_rx_ds) {
			ppe_drv_fill_fse_v6_tuple_info(pcr, &fse_info, true);
			fse_cn = pcr;
		} else if (is_tx_ds && is_rx_ds) {
			ppe_drv_fill_fse_v6_tuple_info(pcf, &fse_info, true);
			ppe_drv_fill_fse_v6_tuple_info(pcr, &fse_info_return, true);
			inter_vap = true;
		} else {
			ppe_drv_trace("%p: Inter VAP configuration not valid when DS flags is set for V6 flows\n", p);
			return false;
		}
	} else if ((create->rule_flags & PPE_DRV_V6_RULE_FLAG_VP_FLOW)  == PPE_DRV_V6_RULE_FLAG_VP_FLOW) {
		if ((is_rx_ds && !is_tx_ds) || (is_rx_active_vp && !is_tx_active_vp)) {
			ppe_drv_fill_fse_v6_tuple_info(pcf, &fse_info, false);
			fse_cn = pcf;
		} else if ((is_tx_ds && !is_rx_ds) || (is_tx_active_vp && !is_rx_active_vp)) {
			ppe_drv_fill_fse_v6_tuple_info(pcr, &fse_info, false);
			fse_cn = pcr;
		} else if (is_tx_ds && is_rx_ds) {
			ppe_drv_fill_fse_v6_tuple_info(pcf, &fse_info, false);
			ppe_drv_fill_fse_v6_tuple_info(pcr, &fse_info_return, false);
			inter_vap = true;
		}
	} else {
		/*
		 * Check if both tx and rx interfaces are active vp and prepare fse rules
		 * for each radio
		 */
		if (is_rx_active_vp && is_tx_active_vp) {
			ppe_drv_fill_fse_v6_tuple_info(pcf, &fse_info, false);
			ppe_drv_fill_fse_v6_tuple_info(pcr, &fse_info_return, false);
			inter_vap = true;
		} else if (is_tx_ds && is_rx_ds) {
			/*
			 * Check if both tx and rx interfaces are DS and prepare fse rules
			 * for each radio
			 */
			ppe_drv_fill_fse_v6_tuple_info(pcf, &fse_info, true);
			ppe_drv_fill_fse_v6_tuple_info(pcr, &fse_info_return, true);
			inter_vap = true;
		} else if (is_rx_active_vp && is_tx_ds) {
			/*
			 * Check if tx and rx interfaces are DS and active_vp respectively and
			 * prepare two fse rules one for each radio.
			 */
			ppe_drv_fill_fse_v6_tuple_info(pcf, &fse_info, false);
			ppe_drv_fill_fse_v6_tuple_info(pcr, &fse_info_return, true);
			inter_vap = true;
		} else if (is_tx_active_vp && is_rx_ds) {
			/*
			 * Check if rx and tx interfaces are DS and active_vp respectively and
			 * prepare two fse rules one for each radio.
			 */
			ppe_drv_fill_fse_v6_tuple_info(pcr, &fse_info, false);
			ppe_drv_fill_fse_v6_tuple_info(pcf, &fse_info_return, true);
			inter_vap = true;
		} else if (is_rx_active_vp && !is_tx_active_vp) {
			ppe_drv_fill_fse_v6_tuple_info(pcf, &fse_info, false);
			fse_cn = pcf;
		} else if (is_tx_active_vp && !is_rx_active_vp) {
			ppe_drv_fill_fse_v6_tuple_info(pcr, &fse_info, false);
			fse_cn = pcr;
		} else if (is_rx_ds && !is_tx_ds) {
			ppe_drv_fill_fse_v6_tuple_info(pcf, &fse_info, true);
			fse_cn = pcf;
		} else if (is_tx_ds && !is_rx_ds) {
			ppe_drv_fill_fse_v6_tuple_info(pcr, &fse_info, true);
			fse_cn = pcr;
		}
	}

	if (!inter_vap) {
		if (p->fse_ops->create_fse_rule(&fse_info)) {
			ppe_drv_trace("%p: FSE v6 rule configuration failed\n", p);
			return false;
		}

		if (!fse_cn) {
			ppe_drv_trace("Failed to get v6 FSE connection\n");
			return false;
		}

		ppe_drv_v6_conn_flow_flags_set(fse_cn, PPE_DRV_V6_CONN_FLOW_FLAG_FSE);
		kref_get(&p->fse_ops_ref);
	} else {
		if (p->fse_ops->create_fse_rule(&fse_info)) {
			ppe_drv_trace("%p: Inter VAP v6 FSE rule configuration failed\n", p);
			return false;
		}

		if (p->fse_ops->create_fse_rule(&fse_info_return)) {
			p->fse_ops->destroy_fse_rule(&fse_info);
			ppe_drv_trace("%p: Inter VAP v6 FSE rule configuration failed for return\n", p);
			return false;
		}

		ppe_drv_v6_conn_flow_flags_set(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_FSE);
		ppe_drv_v6_conn_flow_flags_set(pcr, PPE_DRV_V6_CONN_FLOW_FLAG_FSE);
		kref_get(&p->fse_ops_ref);
		kref_get(&p->fse_ops_ref);
	}

	ppe_drv_trace("%p: FSE v6 rule configuration successful\n", p);
	return true;
}

/*
 * ppe_drv_v6_create()
 *	Adds a connection entry in PPE.
 */
ppe_drv_ret_t ppe_drv_v6_create(struct ppe_drv_v6_rule_create *create)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_v6_conn_flow *pcf = NULL;
	struct ppe_drv_v6_conn_flow *pcr = NULL;
	struct ppe_drv_comm_stats *comm_stats;
	struct ppe_drv_top_if_rule top_if;
	struct ppe_drv_v6_conn *cn = NULL;
	ppe_drv_ret_t ret;

	comm_stats = &p->stats.comm_stats[PPE_DRV_CONN_TYPE_FLOW];

	/*
	 * Check if PPE assist is required for the given interface; if yes
	 * then we should avoid accelerating these flows in PPE and should
	 * fallback to RFS specific API calls.
	 */
	spin_lock_bh(&p->lock);
	if (ppe_drv_v6_passive_vp_flow(create)) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&comm_stats->v6_create_rfs_noedit_flow);
		ppe_drv_warn("%p: v6 Flow needs to be pushed through RFS API(s): %p", p, create);
		return PPE_DRV_RET_FAILURE_DUMMY_RULE;
	}
	spin_unlock_bh(&p->lock);

	/*
	 * Check if the PPE offload is enabled on the rule's Tx/Rx ports or not
	 */
	if (!ppe_drv_iface_check_flow_offload_enabled(create->conn_rule.rx_if,
				create->conn_rule.tx_if)) {
		ppe_drv_stats_inc(&comm_stats->v6_create_fail_offload_disabled);
		ppe_drv_warn("%p: v6 Flow is configured to not offload: %p", p, create);
		return PPE_DRV_RET_PORT_NO_OFFLOAD;
	}

#ifdef PPE_TUNNEL_ENABLE
	if (ppe_drv_v6_tun_allow_tunnel_create(create)) {
		comm_stats = &p->stats.comm_stats[PPE_DRV_CONN_TYPE_TUNNEL];
		ppe_drv_stats_inc(&comm_stats->v6_create_req);
		ret = ppe_drv_v6_tun_add_ce_notify(create);
		if (ret != PPE_DRV_RET_SUCCESS) {
			ppe_drv_stats_inc(&comm_stats->v6_create_fail);
			ppe_drv_warn("%p: failed to create tunnel CE with error %d", create, ret);
			return PPE_DRV_RET_FAILURE_TUN_CE_ADD_FAILURE;
		}

		return PPE_DRV_RET_SUCCESS;
	}
#endif
	/*
	 * Update stats
	 */
	ppe_drv_stats_inc(&comm_stats->v6_create_req);

	/*
	 * Allocate a new connection entry
	 *
	 * TODO: kzalloc with GFP_ATOMIC is used while considering sync method, in
	 * that case this API would be called from softirq.
	 *
	 * Revisit if we later handle this in a workqueue in async model.
	 */
	cn = ppe_drv_v6_conn_alloc();
	if (!cn) {
		ppe_drv_stats_inc(&comm_stats->v6_create_fail_mem);
		ppe_drv_warn("%p: failed to allocate connection memory: %p", p, create);
		return PPE_DRV_RET_FAILURE_CREATE_OOM;
	}

	/*
	 * Check if this flow is combined with ACL for n-tuple lookup.
	 */
	if (!ppe_drv_v6_bind_acl_policer(create, cn)) {
		ppe_drv_warn("%p: failed to combine with ACL, connection object: %p", p, create);
		kfree(cn);
		return PPE_DRV_RET_ACL_RULE_BIND_FAIL;
	}

	/*
	 * Fill the connection entry.
	 */
	spin_lock_bh(&p->lock);
	ret = ppe_drv_v6_conn_fill(create, cn, PPE_DRV_CONN_TYPE_FLOW);
	if (ret != PPE_DRV_RET_SUCCESS) {
		ppe_drv_stats_inc(&comm_stats->v6_create_fail_conn);
		ppe_drv_warn("%p: failed to fill connection object: %p", p, create);
		goto fail;
	}

	/*
	 * Ensure either direction flow is not already offloaded by us.
	 */
	if (ppe_drv_v6_flow_check(&cn->pcf) || ppe_drv_v6_flow_check(&cn->pcr)) {
		ppe_drv_stats_inc(&comm_stats->v6_create_fail_collision);
		ppe_drv_warn("%p: create collision detected: %p", p, create);
		ret = PPE_DRV_RET_FAILURE_CREATE_COLLISSION;
		goto fail;
	}

	/*
	 * Perform interface hierarchy walk and obtain egress L3_If and egress VSI
	 * for each direction.
	 */
	top_if.rx_if = create->top_rule.rx_if;
	top_if.tx_if = create->top_rule.tx_if;
	if (!ppe_drv_v6_if_walk(&cn->pcf, &top_if, create->conn_rule.tx_if, create->conn_rule.rx_if)) {
		ppe_drv_stats_inc(&comm_stats->v6_create_fail_if_hierarchy);
		ppe_drv_warn("%p: create failed invalid interface hierarchy: %p", p, create);
		ret = PPE_DRV_RET_FAILURE_INVALID_HIERARCHY;
		goto fail;
	}

	pcf = &cn->pcf;

	/*
	 * Reverse the top interfaces for return direction.
	 */
	top_if.rx_if = create->top_rule.tx_if;
	top_if.tx_if = create->top_rule.rx_if;
	if (!ppe_drv_v6_if_walk(&cn->pcr, &top_if, create->conn_rule.rx_if, create->conn_rule.tx_if)) {
		ppe_drv_stats_inc(&comm_stats->v6_create_fail_if_hierarchy);
		ppe_drv_warn("%p: create failed invalid interface hierarchy: %p", p, create);
		ret = PPE_DRV_RET_FAILURE_INVALID_HIERARCHY;
		goto fail;
	}

	pcr = &cn->pcr;

	/*
	 * Add flow direction flow entry
	 */
	pcf->pf = ppe_drv_v6_flow_add(pcf);
	if (!pcf->pf) {
		ppe_drv_stats_inc(&comm_stats->v6_create_fail);
		ppe_drv_warn("%p: acceleration of flow failed: %p", p, pcf);
		ret = PPE_DRV_RET_FAILURE_FLOW_ADD_FAIL;
		goto fail;
	}

	pcr->pf = ppe_drv_v6_flow_add(pcr);
	if (!pcr->pf) {
		/*
		 * Destroy the offloaded flow entry
		 */
		ppe_drv_v6_flow_del(pcf);
		pcf->pf = NULL;

		ppe_drv_stats_inc(&comm_stats->v6_create_fail);
		ppe_drv_warn("%p: acceleration of return direction failed: %p", p, pcr);
		ret = PPE_DRV_RET_FAILURE_FLOW_ADD_FAIL;
		goto fail;
	}

	pcf->conn = cn;
	pcr->conn = cn;

	/*
	 * Set the toggle bit to mark this connection as due for stats update in next sync.
	 */
	cn->toggle = !p->toggled_v6;


	/*
	 * We maintain reference per connection on main ppe context.
	 * Dereference: connection destroy.
	 *
	 * TODO: check if this needed
	 */
	/* ppe_drv_ref(p); */

	/*
	 * Add corresponding FSE rule for a Wi-Fi flow.
	 */
	if (ppe_drv_v6_fse_interface_check(pcf)) {
		if (!ppe_drv_v6_fse_flow_configure(create, pcf, pcr)) {
			ppe_drv_stats_inc(&comm_stats->v6_create_fse_fail);
			ret = PPE_DRV_RET_FAILURE_FLOW_CONFIGURE_FAIL;
			ppe_drv_trace("%p: FSE V6 flow table programming failed\n", p);
			goto fail;
		}

		ppe_drv_stats_inc(&comm_stats->v6_create_fse_success);
	}

	/*
	 * Add connection entry to the active connection list.
	 */
	list_add(&cn->list, &p->conn_v6);

	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;

fail:
	if (pcf && pcf->pf) {
		ppe_drv_v6_flow_del(pcf);
	}

	if (pcr && pcr->pf) {
		ppe_drv_v6_flow_del(pcr);
	}

	/*
	 * Free flow direction references.
	 */
	if (pcf) {
		ppe_drv_v6_if_walk_release(pcf);
	}

	/*
	 * Free return direction references.
	 */
	if (pcr) {
		ppe_drv_v6_if_walk_release(pcr);
	}

	spin_unlock_bh(&p->lock);
	kfree(cn);
	return ret;
}
EXPORT_SYMBOL(ppe_drv_v6_create);

/*
 * ppe_drv_v6_nsm_stats_update()
 *	Update nsm stats for the given 5 tuple flow.
 */
bool ppe_drv_v6_nsm_stats_update(struct ppe_drv_nsm_stats *nsm_stats, struct ppe_drv_v6_5tuple *tuple)
{
	struct ppe_drv_flow *flow;
	struct ppe_drv *p = &ppe_drv_gbl;

	spin_lock_bh(&p->lock);
	flow = ppe_drv_flow_v6_get(tuple);

	if (!flow) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p : Flow not found for give tuple information", tuple);
		return false;
	}

	nsm_stats->flow_stats.rx_bytes = flow->bytes;
	nsm_stats->flow_stats.rx_packets = flow->pkts;
	spin_unlock_bh(&p->lock);

	ppe_drv_trace("nsm stats : packets = %llu bytes = %llu", nsm_stats->flow_stats.rx_packets, nsm_stats->flow_stats.rx_bytes);
	return true;
}
EXPORT_SYMBOL(ppe_drv_v6_nsm_stats_update);
