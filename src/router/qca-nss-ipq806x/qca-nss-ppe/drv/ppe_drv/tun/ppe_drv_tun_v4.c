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

#include <ppe_drv/ppe_drv.h>
#include <ppe_drv_iface.h>
#include <exports/ppe_drv_tun_public.h>
#include <fal/fal_tunnel.h>
#include <fal/fal_port_ctrl.h>
#include "ppe_drv_tun.h"
#include <net/vxlan.h>

/*
 * ppe_drv_tun_v4_port_stats_update()
 *	Updates flow instance's stats counter from PPE port Tx and Rx counters.
 */
void ppe_drv_tun_v4_port_stats_update(struct ppe_drv_v4_conn *cn)
{
	sw_error_t err;
	uint32_t delta_pkts;
	uint32_t delta_bytes;
	fal_port_t v_port;
	struct ppe_drv_v4_conn_flow *pcf = &cn->pcf;
	struct ppe_drv_v4_conn_flow *pcr = &cn->pcr;
	struct ppe_drv_port *pp = NULL;
	fal_port_cnt_t port_cnt;

	/*
	 * Check if its tx/rx port
	 */
	if (ppe_drv_port_tun_get(pcf->tx_port)) {
		pp = pcf->tx_port;
	} else {
		pp = pcf->rx_port;
	}

	v_port = FAL_PORT_ID(FAL_PORT_TYPE_VPORT, pp->port);
	err = fal_port_cnt_get(PPE_DRV_SWITCH_ID, v_port, &port_cnt);
	if (err != SW_OK) {
		printk("%p: failed to get port stats at index: %u", pp, pp->port);
		return;
	}

	/*
	 * Update PORT RX/TX  packet and byte counters
	 */
	delta_pkts = (port_cnt.rx_pkt_cnt - pp->stats.rx_pkt_cnt + FAL_TUNNEL_DECAP_PKT_CNT_MASK + 1) & FAL_TUNNEL_DECAP_PKT_CNT_MASK;
	delta_bytes = (port_cnt.rx_byte_cnt - pp->stats.rx_byte_cnt + FAL_TUNNEL_DECAP_BYTE_CNT_MASK + 1)
		                                              & FAL_TUNNEL_DECAP_BYTE_CNT_MASK;

	/*
	 * Rx packets from the VP port would be Tx count for WAN port
	 * hence updating from VP tx stats
	 */
	ppe_drv_v4_conn_flow_rx_stats_add(pcr, delta_pkts, delta_bytes);
	ppe_drv_v4_conn_flow_tx_stats_add(pcf, delta_pkts, delta_bytes);

	delta_pkts = (port_cnt.tx_pkt_cnt - pp->stats.tx_pkt_cnt + FAL_TUNNEL_DECAP_PKT_CNT_MASK + 1) & FAL_TUNNEL_DECAP_PKT_CNT_MASK;
	delta_bytes = (port_cnt.tx_byte_cnt - pp->stats.tx_byte_cnt + FAL_TUNNEL_DECAP_BYTE_CNT_MASK + 1)
		                                              & FAL_TUNNEL_DECAP_BYTE_CNT_MASK;

	/*
	 * Tx packets from the VP port would be RX count for WAN port
	 *  hence updating the VP tx stats to FLOW Rx
	 */
	ppe_drv_v4_conn_flow_tx_stats_add(pcr, delta_pkts, delta_bytes);
	ppe_drv_v4_conn_flow_rx_stats_add(pcf, delta_pkts, delta_bytes);

	pp->stats.tx_pkt_cnt = port_cnt.tx_pkt_cnt;
	pp->stats.tx_byte_cnt = port_cnt.tx_byte_cnt;
	pp->stats.rx_pkt_cnt = port_cnt.rx_pkt_cnt;
	pp->stats.rx_byte_cnt = port_cnt.rx_byte_cnt;
}

/*
 * ppe_drv_v4_conn_tun_conn_get()
 *	Get the connection entry from list for matching tuple
 *
 * Requires caller to hold lock on ppe_drv_gbl.
 */
struct ppe_drv_v4_conn *ppe_drv_v4_conn_tun_conn_get(struct ppe_drv_v4_5tuple *tuple)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_v4_conn *cn;

	list_for_each_entry(cn, &p->conn_tun_v4, list) {
		if (cn->pcf.match_protocol != tuple->protocol) {
			continue;
		}

		if (!(ppe_drv_v4_addr_equal(cn->pcf.match_src_ip, tuple->flow_ip) &&
					ppe_drv_v4_addr_equal(cn->pcf.match_dest_ip, tuple->return_ip)) &&
				!(ppe_drv_v4_addr_equal(cn->pcr.match_src_ip, tuple->flow_ip) &&
					ppe_drv_v4_addr_equal(cn->pcr.match_dest_ip, tuple->return_ip))) {
			continue;
		}

		if (((cn->pcf.match_src_ident == tuple->flow_ident) &&
					(cn->pcf.match_dest_ident == tuple->return_ident)) ||
				((cn->pcr.match_src_ident == tuple->flow_ident) &&
				 (cn->pcr.match_dest_ident == tuple->return_ident))) {
			return cn;
		}
	}

	return NULL;
}

/*
 * ppe_drv_v4_tun_conn_fill()
 *	Populate each direction flow object.
 */
static ppe_drv_ret_t ppe_drv_v4_tun_conn_fill(struct ppe_drv_v4_rule_create *create, struct ppe_drv_v4_conn *cn)
{
	return ppe_drv_v4_conn_fill(create, cn, PPE_DRV_CONN_TYPE_TUNNEL);
}

/*
 * ppe_drv_v4_tun_get_tun_from_create_rule
 *	Get tunnel drv for tunnel device
 *
 * Requires caller to hold lock on ppe_drv_gbl.
 */
static struct ppe_drv_tun *ppe_drv_v4_tun_get_tun_from_create_rule(struct ppe_drv_v4_connection_rule *conn)
{
	struct ppe_drv_port *pp_port;
	struct ppe_drv_iface *iface;
	struct ppe_drv_tun *port_tun;

	iface = ppe_drv_iface_get_by_idx(conn->rx_if);
	pp_port = iface ? ppe_drv_iface_port_get(iface) : NULL;

	port_tun = (pp_port) ? (ppe_drv_port_tun_get(pp_port)) : (NULL);
	if (!port_tun) {
		iface = ppe_drv_iface_get_by_idx(conn->tx_if);
		pp_port = (iface) ? (ppe_drv_iface_port_get(iface)) : (NULL);
		port_tun = (pp_port) ? (ppe_drv_port_tun_get(pp_port)) : (NULL);
	}

	return port_tun;
}

/*
 * ppe_drv_v4_vxlan_tunnel()
 *	Check if create request for Vxlan tunnel.
 */
static bool ppe_drv_v4_vxlan_tunnel(struct ppe_drv_v4_rule_create *create, struct net_device *dev)
{
	int vxlan_dport = ppe_drv_get_vxlan_dport();

	if (netif_is_vxlan(dev) || (!strncmp(dev->name, "ppe_vxlan_tun", 13))) {
		/*
		 * Check if it is an outer rule.
		 */
		if (((create->tuple.flow_ident == vxlan_dport) && (create->tuple.return_ident == vxlan_dport))) {
			ppe_drv_info("%p: Creating VXLAN tunnel dev: %s", dev, dev->name);
			return true;
		}
	}

	return false;
}

/*
 * ppe_drv_v4_l2tp_tunnel()
 *	Check if create request for l2tp tunnel.
 */
bool ppe_drv_v4_l2tp_tunnel(uint8_t protocol, uint32_t flow_ident, uint32_t return_ident)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	if ((protocol == IPPROTO_UDP) &&
		(((flow_ident == p->tun_gbl.tun_l2tp.l2tp_sport) && (return_ident == p->tun_gbl.tun_l2tp.l2tp_dport)) ||
		 ((flow_ident == p->tun_gbl.tun_l2tp.l2tp_dport) && (return_ident == p->tun_gbl.tun_l2tp.l2tp_sport)))) {
		return true;
	}

	return false;
}

/*
 * ppe_drv_v4_tun_allow_tunnel_destroy()
 *	Check if the destroy rule is received for tunnel
 *
 * Requires caller to hold lock on ppe_drv_gbl.
 */
bool ppe_drv_v4_tun_allow_tunnel_destroy(struct ppe_drv_v4_rule_destroy *destroy)
{
	int vxlan_dport = ppe_drv_get_vxlan_dport();

	/*
	 * PPE accelearation is only supported for default port currently.
	 */
	if (ppe_drv_tun_check_support(destroy->tuple.protocol)) {
		return true;
	}

       if ((destroy->tuple.flow_ident == vxlan_dport && destroy->tuple.return_ident == vxlan_dport)) {
		return true;
       }

       if (ppe_drv_v4_l2tp_tunnel(destroy->tuple.protocol,
			       destroy->tuple.flow_ident, destroy->tuple.return_ident)) {
		return true;
       }

	return false;
}

/*
 * ppe_drv_v4_tun_allow_tunnel_create()
 *	Check if the create rule is received for tunnel activation
 *
 * Requires caller to hold lock on ppe_drv_gbl.
 */
bool ppe_drv_v4_tun_allow_tunnel_create(struct ppe_drv_v4_rule_create *create)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_tun *port_tun;
	struct net_device *dev;

	/*
	 * Check if the rule is for GRE or IPIP6
	 */
	if (ppe_drv_tun_check_support(create->tuple.protocol)) {
		return true;
	}

	/*
	 * Check if rule is for a L2TP tunnel.
	 */
	if (ppe_drv_v4_l2tp_tunnel(create->tuple.protocol, create->tuple.flow_ident,
					create->tuple.return_ident)) {
		return true;
	}

	spin_lock_bh(&p->lock);
	port_tun = ppe_drv_v4_tun_get_tun_from_create_rule(&create->conn_rule);
	if (!port_tun) {
		spin_unlock_bh(&p->lock);
		return false;
	}

	dev = ppe_drv_port_to_dev(port_tun->pp);
	if (ppe_drv_v4_vxlan_tunnel(create, dev)) {
		spin_unlock_bh(&p->lock);
		return true;
	}

	spin_unlock_bh(&p->lock);
	return false;
}

/*
 * ppe_drv_tun_v4_parse_l2_hdr()
 *	Collect the layer 2 parameters from rule create
 *
 * Requires caller to hold lock on ppe_drv_gbl.
 */
void ppe_drv_tun_v4_parse_l2_hdr(struct ppe_drv_v4_rule_create *create, struct ppe_drv_v4_conn *cn,
				 struct ppe_drv_tun_cmn_ctx_l2 *l2)
{
	struct ppe_drv_v4_connection_rule *rule = &create->conn_rule;
	struct ppe_drv_v4_conn_flow *pcf = &cn->pcf;
	uint16_t xmit_port = PPE_DRV_PORTS_MAX;
	struct ppe_drv_vlan *vlan;
	uint8_t *pppoe_server_mac;
	struct ppe_drv_port *pp;
	uint8_t egress_vlan_cnt;
	uint8_t *src_mac_addr;

	if (!ppe_drv_port_tun_get(pcf->tx_port)) {
		pp = pcf->tx_port;
	} else {
		pp = pcf->rx_port;
	}

	ppe_drv_assert(pp, "%p: physical xmit port not found", create);

	xmit_port = pp->port;

	ppe_drv_assert(pp->mac_valid, "%p: MAC address is not set", pp);

	src_mac_addr = pp->mac_addr;

	memset(l2, 0, sizeof(*l2));

	l2->xmit_port = xmit_port;
	memcpy(l2->smac, src_mac_addr, sizeof(l2->smac));
	memcpy(l2->dmac, rule->return_mac, sizeof(l2->dmac));

	l2->eth_type = ETH_P_IP;

	egress_vlan_cnt = ppe_drv_v4_conn_flow_egress_vlan_cnt_get(pcf);

	if (egress_vlan_cnt == 2) {
		vlan = ppe_drv_v4_conn_flow_egress_vlan_get(pcf, 0);
		l2->vlan[0].tpid = vlan->tpid;
		l2->vlan[0].tci = vlan->tci;
		l2->flags |= PPE_DRV_TUN_CMN_CTX_L2_SVLAN_VALID;

		vlan = ppe_drv_v4_conn_flow_egress_vlan_get(pcf, 1);
		l2->vlan[1].tpid = vlan->tpid;
		l2->vlan[1].tci = vlan->tci;
		l2->flags |= PPE_DRV_TUN_CMN_CTX_L2_CVLAN_VALID;
	} else if (egress_vlan_cnt == 1) {
		vlan = ppe_drv_v4_conn_flow_egress_vlan_get(pcf, 0);
		l2->vlan[0].tpid = vlan->tpid;
		l2->vlan[0].tci = vlan->tci;
		l2->flags |= PPE_DRV_TUN_CMN_CTX_L2_CVLAN_VALID;
	}

	if (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_PPPOE_FLOW)) {
		l2->pppoe.ph.type = 1;
		l2->pppoe.ph.ver = 1;
		l2->pppoe.ph.code = 0;
		l2->pppoe.ph.sid = htons(ppe_drv_v4_conn_flow_pppoe_session_id_get(pcf));
		l2->pppoe.ppp_proto = htons(PPP_IP);
		pppoe_server_mac = ppe_drv_v4_conn_flow_pppoe_server_mac_get(pcf);
		memcpy(&l2->pppoe.server_mac, pppoe_server_mac, ETH_ALEN);

		l2->flags |= PPE_DRV_TUN_CMN_CTX_L2_PPPOE_VALID;

		/*
		 * Set the ethernet header type to PPPoE session
		 */
		l2->eth_type = ETH_P_PPP_SES;

	}
}

/*
 * ppe_drv_v4_tun_del_ce_validate()
 *	Delete a tunnel connection entry from list.
 *
 * Requires caller to hold lock on ppe_drv_gbl.
 */
ppe_drv_ret_t ppe_drv_v4_tun_del_ce_validate(void *vdestroy_rule, struct ppe_drv_v4_conn_sync **cns_v4, struct ppe_drv_v4_conn **cn_v4)
{
	struct ppe_drv_v4_rule_destroy *destroy = (struct ppe_drv_v4_rule_destroy *)vdestroy_rule;
	ppe_drv_ret_t ret = PPE_DRV_RET_SUCCESS;
	struct ppe_drv_v4_conn_flow *pcf, *pcr;
	struct ppe_drv_comm_stats *comm_stats;
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_v4_conn *cn;
	struct ppe_drv_v4_conn_sync *cns;

	cns = ppe_drv_v4_conn_stats_alloc();
	if (!cns) {
		ppe_drv_warn("%p: Could not allocate connection stats", destroy);
		return PPE_DRV_RET_FAILURE_CREATE_OOM;
	}

	comm_stats = &p->stats.comm_stats[PPE_DRV_CONN_TYPE_TUNNEL];

	cn = ppe_drv_v4_conn_tun_conn_get(&destroy->tuple);
	if (!cn) {
		ppe_drv_stats_inc(&comm_stats->v4_destroy_conn_not_found);
		ppe_drv_warn("%p: Could not find tunnel connection entry", destroy);
		ppe_drv_v4_conn_stats_free(cns);
		return PPE_DRV_RET_FAILURE_DESTROY_NO_CONN;
	}

	pcf = &cn->pcf;
	pcr = &cn->pcr;

	/*
	 * Release references on interfaces.
	 */
	ppe_drv_v4_if_walk_release(pcf);

	/*
	 * Release references on interfaces.
	 */
	ppe_drv_v4_if_walk_release(pcr);

	*cns_v4 = cns;
	*cn_v4 = cn;

	return ret;
}

/*
 * ppe_drv_v4_tun_del_ce_notify()
 *	Notify the PPE tunnel driver to delete connection entry
 */
ppe_drv_ret_t ppe_drv_v4_tun_del_ce_notify(struct ppe_drv_v4_rule_destroy *destroy)
{
	struct ppe_drv_v4_conn_flow *pcf, *pcr;
	struct ppe_drv_comm_stats *comm_stats;
	struct ppe_drv *p = &ppe_drv_gbl;
	ppe_drv_tun_del_ce_callback_t del_cb;
	struct ppe_drv_v4_conn *cn;
	struct ppe_drv_tun *tun;
	uint8_t vp_num;
	uint8_t status;

	comm_stats = &p->stats.comm_stats[PPE_DRV_CONN_TYPE_TUNNEL];

	spin_lock_bh(&p->lock);
	cn = ppe_drv_v4_conn_tun_conn_get(&destroy->tuple);
	if (!cn) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&comm_stats->v4_destroy_conn_not_found);
		ppe_drv_warn("%p: Could not find tunnel connection entry", destroy);
		return PPE_DRV_RET_FAILURE_DESTROY_NO_CONN;
	}

	pcf = &cn->pcf;
	pcr = &cn->pcr;

	tun = (ppe_drv_port_tun_get(pcf->tx_port)) ? \
			(ppe_drv_port_tun_get(pcf->tx_port)) : (ppe_drv_port_tun_get(pcf->rx_port));

	vp_num = tun->vp_num;
	del_cb = tun->del_cb;

	spin_unlock_bh(&p->lock);

	ppe_drv_stats_inc(&comm_stats->v4_destroy_req);
	status = del_cb(vp_num, destroy);
	if (status != true) {
		ppe_drv_stats_inc(&comm_stats->v4_destroy_fail);
		return PPE_DRV_RET_FAILURE_TUN_CE_DEL_FAILURE;
	}

	return PPE_DRV_RET_SUCCESS;
}

/*
 * ppe_drv_v4_tun_add_ce_notify()
 *	Notify the PPE tunnel driver to add connection.
 */
ppe_drv_ret_t ppe_drv_v4_tun_add_ce_notify(struct ppe_drv_v4_rule_create *create)
{
	struct ppe_drv_v4_connection_rule *conn = &create->conn_rule;
	struct ppe_drv_iface *iface;
	struct ppe_drv_port *pp_port;
	struct ppe_drv_tun *port_tun;
	ppe_drv_tun_add_ce_callback_t add_cb;
	struct ppe_drv *p = &ppe_drv_gbl;
	uint8_t vp_num;
	uint8_t status;

	spin_lock_bh(&p->lock);
	iface = ppe_drv_iface_get_by_idx(conn->rx_if);
	pp_port = (iface) ? (ppe_drv_iface_port_get(iface)) : (NULL);
	port_tun = (pp_port) ? (ppe_drv_port_tun_get(pp_port)) : (NULL);

	if (!port_tun) {
		iface = ppe_drv_iface_get_by_idx(conn->tx_if);
		pp_port = (iface) ? (ppe_drv_iface_port_get(iface)) : (NULL);
		port_tun = (pp_port) ? (ppe_drv_port_tun_get(pp_port)) : (NULL);
	}

	/*
	 * If the outer rule is pushed before the PPE tunnel client creates the tunnel
	 * instance, then the tunnel will be NULL.
	 */
	vp_num = (port_tun) ? (port_tun->vp_num) : 0;
	add_cb = (port_tun) ? (port_tun->add_cb) : (NULL);

	spin_unlock_bh(&p->lock);

	if ((vp_num < PPE_DRV_VIRTUAL_START) || !add_cb) {
		return (!add_cb) ? (PPE_DRV_RET_TUN_ADD_CE_NULL) : (PPE_DRV_RET_INVALID_VP_NUM);
	}

	status = add_cb(vp_num, create);
	if (status != true) {
		return PPE_DRV_RET_FAILURE_TUN_CE_ADD_FAILURE;
	}

	return PPE_DRV_RET_SUCCESS;
}

/*
 * ppe_drv_v4_tun_add_ce_validate()
 *	Validate rule create parameters.
 *
 * Requires caller to hold lock on ppe_drv_gbl.
 */
ppe_drv_ret_t ppe_drv_v4_tun_add_ce_validate(void *vcreate_rule, struct ppe_drv_v4_conn *cn)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_v4_conn_flow *pcf = NULL;
	struct ppe_drv_v4_conn_flow *pcr = NULL;
	struct ppe_drv_top_if_rule top_if;
	struct ppe_drv_comm_stats *comm_stats;
	struct ppe_drv_v4_rule_create *create = (struct ppe_drv_v4_rule_create *)vcreate_rule;
	ppe_drv_ret_t ret;

	comm_stats = &p->stats.comm_stats[PPE_DRV_CONN_TYPE_TUNNEL];
	/*
	 * Fill the connection entry.
	 */
	ret = ppe_drv_v4_tun_conn_fill(create, cn);
	if (ret != PPE_DRV_RET_SUCCESS) {
		ppe_drv_stats_inc(&comm_stats->v4_create_fail_conn);
		ppe_drv_warn("%p: failed to fill connection object: %p", p, create);
		goto fail;
	}

	/*
	 * Ensure either direction flow is not already offloaded by us.
	 */
	if (ppe_drv_v4_conn_tun_conn_get(&create->tuple)) {
		ppe_drv_stats_inc(&comm_stats->v4_create_fail_collision);
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
	if (!ppe_drv_v4_if_walk(&cn->pcf, &top_if, create->conn_rule.tx_if, create->conn_rule.rx_if)) {
		ppe_drv_stats_inc(&comm_stats->v4_create_fail_if_hierarchy);
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
	if (!ppe_drv_v4_if_walk(&cn->pcr, &top_if, create->conn_rule.rx_if, create->conn_rule.tx_if)) {
		ppe_drv_stats_inc(&comm_stats->v4_create_fail_if_hierarchy);
		ppe_drv_warn("%p: create failed invalid interface hierarchy: %p", p, create);
		ret = PPE_DRV_RET_FAILURE_INVALID_HIERARCHY;
		goto fail;
	}

	pcr = &cn->pcr;

	/*
	 * Add connection entry to the active connection list.
	 */

	pcf->conn = cn;
	pcr->conn = cn;

	/*
	 * Set the toggle bit to mark this connection as due for stats update in next sync.
	 */
	cn->toggle = !p->tun_toggled_v4;

	return PPE_DRV_RET_SUCCESS;

fail:
	/*
	 * Free flow direction references.
	 */
	if (pcf) {
		ppe_drv_v4_if_walk_release(pcf);
	}

	/*
	 * Free return direction references.
	 */
	if (pcr) {
		ppe_drv_v4_if_walk_release(pcr);
	}

	return ret;
}

/*
 * ppe_drv_tun_v4_fse_entry_del
 *	Delete tunnel FSE rule entry
 */
bool ppe_drv_tun_v4_fse_entry_del(struct ppe_drv_v4_conn_flow *pcf,  struct ppe_drv_v4_conn_flow *pcr)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_comm_stats *comm_stats;
        struct ppe_drv_fse_rule_info fse_info = {0};
        struct ppe_drv_v4_conn_flow *fse_cn = NULL;
	struct ppe_drv_port *rx_port = ppe_drv_v4_conn_flow_rx_port_get(pcf);
	struct ppe_drv_port *tx_port = ppe_drv_v4_conn_flow_tx_port_get(pcf);
	bool is_tx_ds = (tx_port->user_type == PPE_DRV_PORT_USER_TYPE_DS);
	bool is_rx_ds = (rx_port->user_type == PPE_DRV_PORT_USER_TYPE_DS);
	bool is_tx_active_vp = (tx_port->user_type == PPE_DRV_PORT_USER_TYPE_ACTIVE_VP);
	bool is_rx_active_vp = (rx_port->user_type == PPE_DRV_PORT_USER_TYPE_ACTIVE_VP);

	comm_stats = &p->stats.comm_stats[PPE_DRV_CONN_TYPE_TUNNEL];
	if ((is_rx_ds && !is_tx_ds) || (is_rx_active_vp && !is_tx_active_vp)) {
		ppe_drv_fill_fse_v4_tuple_info(pcf, &fse_info, true);
		fse_cn = pcf;
	} else if ((is_tx_ds && !is_rx_ds) || (is_tx_active_vp && !is_rx_active_vp)) {
		ppe_drv_fill_fse_v4_tuple_info(pcr, &fse_info, true);
		fse_cn = pcr;
	} else {
		ppe_drv_warn("Tx/Rx either port must be active/DS VP\n");
		return false;
	}

	if (p->fse_ops->destroy_fse_rule(&fse_info)) {
		ppe_drv_stats_inc(&p->stats.comm_stats->v4_destroy_fse_fail);
		ppe_drv_warn("%p: FSE v4 rule deletion failed\n", pcf);
		return false;
	}

	ppe_drv_stats_inc(&p->stats.comm_stats->v4_destroy_fse_success);
	ppe_drv_v4_conn_flow_flags_clear(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_FSE);
	ppe_drv_v4_conn_flow_flags_clear(pcr, PPE_DRV_V4_CONN_FLOW_FLAG_FSE);
	kref_put(&p->fse_ops_ref, ppe_drv_fse_ops_free);
	ppe_drv_trace("%p: FSE v4 rule deletion successfull\n", pcf);

	return true;
}

/*
 * ppe_drv_tun_v6_fse_entry_add
 *	Add tunnel FSE rule entry
 */
bool ppe_drv_tun_v4_fse_entry_add(void *vcreate_rule, struct ppe_drv_v4_conn_flow *pcf, struct ppe_drv_v4_conn_flow *pcr)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_comm_stats *comm_stats;
	struct ppe_drv_v4_rule_create *create = (struct ppe_drv_v4_rule_create *)vcreate_rule;

	comm_stats = &p->stats.comm_stats[PPE_DRV_CONN_TYPE_TUNNEL];

	if (ppe_drv_v4_fse_interface_check(pcf)) {
		if (!ppe_drv_v4_fse_flow_configure(create, pcf, pcr)) {
			ppe_drv_stats_inc(&comm_stats->v4_create_fse_fail);
			ppe_drv_warn("%p: FSE flow table programming failed\n", p);
			return false;
		}

		ppe_drv_stats_inc(&comm_stats->v4_create_fse_success);
	}

	return true;
}
