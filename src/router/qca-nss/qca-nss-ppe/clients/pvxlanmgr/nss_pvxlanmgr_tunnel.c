/*
 **************************************************************************
 * Copyright (c) 2019-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "nss_pvxlanmgr_priv.h"

/*
 * nss_pvxlanmgr_tunnel_tx_msg()
 *	Transmit PVXLAN tunnel operation messages.
 */
static nss_tx_status_t nss_pvxlanmgr_tunnel_tx_msg(struct nss_ctx_instance *ctx, struct nss_pvxlan_msg * msg,
		uint32_t if_num, nss_pvxlan_msg_type_t cmd, uint32_t len)
{

	/*
	 * Send PVXLAN data tunnel command to NSS
	 */
	nss_pvxlan_msg_init(msg, if_num, cmd, len, NULL, NULL);
	return nss_pvxlan_tx_msg_sync(ctx, msg);
}

/*
 * nss_pvxlanmgr_tunnel_tx_msg_mac_del()
 *	PvxLAN tunnel transmit mac delete messages.
 */
static inline nss_pvxlanmgr_status_t nss_pvxlanmgr_tunnel_tx_msg_mac_del(struct net_device *dev, uint32_t tunnel_id,
						uint8_t *mac_addr)
{
	struct nss_pvxlanmgr_priv *priv;
	struct nss_pvxlan_msg pvxlanmsg = {0};
	struct nss_pvxlan_mac_msg *ncfrm;
	struct nss_pvxlanmgr_tunnel *t;
	nss_tx_status_t status;

	t = nss_pvxlanmgr_tunnel_get(dev, tunnel_id);
	if (!t) {
		nss_pvxlanmgr_warn("%px: can't find tunnel: %d\n", dev, tunnel_id);
		return NSS_PVXLANMGR_FAILURE_BAD_PARAM;
	}

	dev_hold(dev);
	priv = netdev_priv(dev);

	/*
	 * Set MAC rule message
	 */
	ncfrm = &pvxlanmsg.msg.mac_del;
	ncfrm->tunnel_id = tunnel_id;
	ether_addr_copy((u8 *)ncfrm->mac_addr, mac_addr);

	status = nss_pvxlanmgr_tunnel_tx_msg(priv->pvxlan_ctx, &pvxlanmsg, priv->if_num_host_inner, NSS_PVXLAN_MSG_TYPE_MAC_DEL, sizeof(struct nss_pvxlan_mac_msg));
	if (status != NSS_TX_SUCCESS) {
		nss_pvxlanmgr_warn("%px: ctx: MAC delete failed %d\n", dev, status);
	}

	dev_put(dev);
	return status;
}

/*
 * nss_pvxlanmgr_tunnel_tx_msg_mac_add()
 *	PvxLAN tunnel transmit mac add messages.
 */
static inline nss_pvxlanmgr_status_t nss_pvxlanmgr_tunnel_tx_msg_mac_add(struct net_device *dev, uint32_t tunnel_id,
						uint8_t *mac_addr, struct nss_pvxlanmgr_vxlan_hdr *pvxch)
{
	struct nss_pvxlanmgr_priv *priv;
	struct nss_pvxlan_msg pvxlanmsg = {0};
	struct nss_pvxlan_mac_msg *ncfrm;
	struct nss_pvxlanmgr_tunnel *t;
	nss_tx_status_t status;

	t = nss_pvxlanmgr_tunnel_get(dev, tunnel_id);
	if (!t) {
		nss_pvxlanmgr_warn("%px: can't find tunnel: %d\n", dev, tunnel_id);
		return NSS_PVXLANMGR_FAILURE_BAD_PARAM;
	}

	dev_hold(dev);
	priv = netdev_priv(dev);

	/*
	 * Set MAC rule message
	 */
	ncfrm = &pvxlanmsg.msg.mac_add;
	ncfrm->flags = pvxch->flags;
	ncfrm->vnet_id = pvxch->vnet_id;
	ncfrm->policy_id = pvxch->gpid;
	ncfrm->tunnel_id = tunnel_id;
	ether_addr_copy((u8 *)ncfrm->mac_addr, mac_addr);

	status = nss_pvxlanmgr_tunnel_tx_msg(priv->pvxlan_ctx, &pvxlanmsg, priv->if_num_host_inner, NSS_PVXLAN_MSG_TYPE_MAC_ADD, sizeof(struct nss_pvxlan_mac_msg));
	if (status != NSS_TX_SUCCESS) {
		nss_pvxlanmgr_warn("%px: ctx: MAC addition failed %d\n", dev, status);
	}

	dev_put(dev);
	return status;
}

/*
 * nss_pvxlanmgr_pvxlan_rule_destroy()
 *	Internal function to destroy a PVxLAN rule
 */
static nss_pvxlanmgr_status_t nss_pvxlanmgr_tunnel_pvxlan_rule_destroy(struct nss_ctx_instance *ctx, uint32_t if_num,
			 struct nss_pvxlan_rule_msg *msg)
{
	struct nss_pvxlan_msg pvxlanmsg = {0};
	nss_tx_status_t status;

	nss_pvxlanmgr_info("%px: ctx: PVxLAN Rule src_port: 0x%d dest_port:0x%d\n", ctx,
	    msg->encap.src_port, msg->encap.dest_port);

	/*
	 * Prepare the tunnel configuration parameter to send to NSS FW
	 */
	pvxlanmsg.msg.rule_uncfg = *msg;

	/*
	 * Send PVxLAN tunnel create command to NSS
	 */
	status = nss_pvxlanmgr_tunnel_tx_msg(ctx, &pvxlanmsg, if_num, NSS_PVXLAN_MSG_TYPE_TUNNEL_DESTROY_RULE, sizeof(struct nss_pvxlan_rule_msg));
	if (status != NSS_TX_SUCCESS) {
		nss_pvxlanmgr_warn("%px: ctx: create encap data tunnel error %d\n", ctx, status);
		return status;
	}

	return NSS_PVXLANMGR_SUCCESS;
}

/*
 * nss_pvxlanmgr_pvxlan_rule_create()
 *	Internal function to create a PVxLAN rule
 */
static nss_pvxlanmgr_status_t nss_pvxlanmgr_tunnel_pvxlan_rule_create(struct nss_ctx_instance *ctx, uint32_t if_num,
			 struct nss_pvxlan_rule_msg *msg)
{
	struct nss_pvxlan_msg pvxlanmsg = {0};
	nss_tx_status_t status;

	nss_pvxlanmgr_info("%px: ctx: PVxLAN Rule src_port: 0x%d dest_port:0x%d\n", ctx,
	    msg->encap.src_port, msg->encap.dest_port);

	/*
	 * Prepare the tunnel configuration parameter to send to NSS FW
	 */
	pvxlanmsg.msg.rule_cfg = *msg;

	/*
	 * Send PVxLAN tunnel create command to NSS
	 */
	status = nss_pvxlanmgr_tunnel_tx_msg(ctx, &pvxlanmsg, if_num, NSS_PVXLAN_MSG_TYPE_TUNNEL_CREATE_RULE, sizeof(struct nss_pvxlan_rule_msg));
	if (status != NSS_TX_SUCCESS) {
		nss_pvxlanmgr_warn("%px: ctx: create encap data tunnel error %d\n", ctx, status);
		return status;
	}

	return NSS_PVXLANMGR_SUCCESS;
}

/*
 * nss_pvxlanmgr_tunnel_ppe_destroy_ipv4_rule()
 *	Destroy a given connection in the PPE
 */
static nss_pvxlanmgr_status_t nss_pvxlanmgr_tunnel_ppe_destroy_ipv4_rule(struct nss_pvxlanmgr_tunnel *t)
{
	ppe_drv_ret_t ppe_status;
	ppe_acl_ret_t ppe_acl_status;
	struct ppe_drv_v4_rule_destroy pd4rd = {0};
	struct nss_ipv4_5tuple *ni5t = &t->ip_rule.v4.tuple;

	nss_pvxlanmgr_info("Destroy IPv4: %pI4h :%u, %pI4h :%u, p: %d\n",
		&ni5t->flow_ip, ni5t->flow_ident, &ni5t->return_ip, ni5t->return_ident, ni5t->protocol);

	memcpy(&pd4rd, ni5t, sizeof(struct ppe_drv_v4_rule_destroy));

	if (t->tunnel_state & NSS_PVXLANMGR_TUNNEL_STATE_PPE_FLOW_RULE_CONFIGURED) {
		ppe_status = ppe_drv_v4_destroy(&pd4rd);
		if (ppe_status != PPE_DRV_RET_SUCCESS) {
			nss_pvxlanmgr_warn("Unconfigure ipv4 flow rule failed : %d\n", ppe_status);
			return NSS_PVXLANMGR_FAILURE_UNCONFIG_PPE_FLOW_RULE;
		}
		t->tunnel_state &= ~NSS_PVXLANMGR_TUNNEL_STATE_PPE_FLOW_RULE_CONFIGURED;
	}

	if (t->tunnel_state & NSS_PVXLANMGR_TUNNEL_STATE_PPE_ACL_RULE_CONFIGURED) {
		ppe_acl_status = ppe_acl_rule_destroy(t->acl_rule_id);
		if (ppe_acl_status != PPE_ACL_RET_SUCCESS) {
			nss_pvxlanmgr_warn("unconfigure ipv4 acl rule failed : %d\n", ppe_acl_status);
			return NSS_PVXLANMGR_FAILURE_UNCONFIG_PPE_ACL_RULE;
		}
		t->tunnel_state &= ~NSS_PVXLANMGR_TUNNEL_STATE_PPE_ACL_RULE_CONFIGURED;
	}

	return 	NSS_PVXLANMGR_SUCCESS;
}

/*
 * nss_pvxlanmgr_tunnel_ppe_destroy_ipv6_rule()
 *	Destroy a given connection in the PPE
 */
static nss_pvxlanmgr_status_t nss_pvxlanmgr_tunnel_ppe_destroy_ipv6_rule(struct nss_pvxlanmgr_tunnel *t)
{
	ppe_drv_ret_t ppe_status;
	ppe_acl_ret_t ppe_acl_status;
	struct ppe_drv_v6_rule_destroy pd6rd;
	struct nss_ipv6_5tuple *ni5t = &t->ip_rule.v6.tuple;

	nss_pvxlanmgr_info("Destroy IPv6: %pI6:%u, %pI6:%u, p: %d\n",
		ni5t->flow_ip, ni5t->flow_ident, ni5t->return_ip, ni5t->return_ident, ni5t->protocol);


	memcpy(&pd6rd, ni5t, sizeof(struct ppe_drv_v6_rule_destroy));

	if (t->tunnel_state & NSS_PVXLANMGR_TUNNEL_STATE_PPE_FLOW_RULE_CONFIGURED) {
		ppe_status = ppe_drv_v6_destroy(&pd6rd);
		if (ppe_status != PPE_DRV_RET_SUCCESS) {
			nss_pvxlanmgr_warn("Unconfigure ipv4 flow rule failed : %d\n", ppe_status);
			return NSS_PVXLANMGR_FAILURE_UNCONFIG_PPE_FLOW_RULE;
		}
		t->tunnel_state &= ~NSS_PVXLANMGR_TUNNEL_STATE_PPE_FLOW_RULE_CONFIGURED;
	}

	if (t->tunnel_state & NSS_PVXLANMGR_TUNNEL_STATE_PPE_ACL_RULE_CONFIGURED) {
		ppe_acl_status = ppe_acl_rule_destroy(t->acl_rule_id);
		if (ppe_acl_status != PPE_ACL_RET_SUCCESS) {
			nss_pvxlanmgr_warn("Unconfigure ipv4 acl rule failed : %d\n", ppe_acl_status);
			return NSS_PVXLANMGR_FAILURE_UNCONFIG_PPE_ACL_RULE;
		}
		t->tunnel_state &= ~NSS_PVXLANMGR_TUNNEL_STATE_PPE_ACL_RULE_CONFIGURED;
	}

	return NSS_PVXLANMGR_SUCCESS;
}

/*
 * nss_pvxlanmgr_tunnel_ppe_create_ipv4_rule()
 *	Create a given connection in the PPE.
 *
 * For the flow in uplink direction we use PPE's 5 tuple flow lookup.
 * For the flow in downlink direction we use ACL rule to match 4 tuple and redirect to VP.
 */
static nss_pvxlanmgr_status_t nss_pvxlanmgr_tunnel_ppe_create_ipv4_rule(struct net_device *dev, struct nss_ipv4_rule_create_msg *nircm, struct nss_pvxlanmgr_tunnel *t)
{
	struct ppe_drv_v4_rule_create pd4rc = {0};
	struct ppe_drv_v4_rule_destroy pd4rd = {0};
	struct ppe_acl_rule acl_rule = {0};
	struct net_device *wan_dev = NULL;
	ppe_drv_ret_t ppe_status = PPE_DRV_RET_SUCCESS;
	ppe_acl_ret_t ppe_acl_status = PPE_ACL_RET_SUCCESS;

	/*
	 * Copy over the 5 tuple details.
	 */
	memcpy(&pd4rc.tuple, &nircm->tuple, sizeof(struct ppe_drv_v4_5tuple));

	/*
	 * Copy over the connection rules and set the CONN_VALID flag
	 */
	pd4rc.conn_rule.rx_if = nircm->conn_rule.flow_interface_num;
	pd4rc.conn_rule.flow_mtu = nircm->conn_rule.flow_mtu;
	pd4rc.conn_rule.flow_ip_xlate = nircm->conn_rule.flow_ip_xlate;
	pd4rc.conn_rule.flow_ident_xlate = nircm->conn_rule.flow_ident_xlate;
	ether_addr_copy(pd4rc.conn_rule.flow_mac, (u8 *)nircm->conn_rule.flow_mac);

	pd4rc.conn_rule.tx_if = nircm->conn_rule.return_interface_num;
	pd4rc.conn_rule.return_mtu = nircm->conn_rule.return_mtu;
	pd4rc.conn_rule.return_ip_xlate = nircm->conn_rule.return_ip_xlate;
	pd4rc.conn_rule.return_ident_xlate = nircm->conn_rule.return_ident_xlate;
	ether_addr_copy(pd4rc.conn_rule.return_mac, (u8 *)nircm->conn_rule.return_mac);

	pd4rc.rule_flags |= (PPE_DRV_V4_RULE_FLAG_RETURN_VALID | PPE_DRV_V4_RULE_FLAG_FLOW_VALID);

	/*
	 * Copy over the qos rules and set the QOS_VALID flag.
	 */
        if (nircm->valid_flags & NSS_IPV4_RULE_CREATE_QOS_VALID) {
		memcpy(&pd4rc.qos_rule, &nircm->qos_rule, sizeof(struct ppe_drv_qos_rule));
		pd4rc.valid_flags |= PPE_DRV_V4_VALID_FLAG_QOS;
	}

	/*
	 * Copy over the DSCP rule parameters.
	 */
	if (nircm->valid_flags & NSS_IPV4_RULE_CREATE_DSCP_MARKING_VALID) {
                memcpy(&pd4rc.dscp_rule, &nircm->dscp_rule, sizeof(struct ppe_drv_dscp_rule));
		pd4rc.rule_flags |= PPE_DRV_V4_RULE_FLAG_DSCP_MARKING;
		pd4rc.valid_flags |= PPE_DRV_V4_VALID_FLAG_DSCP_MARKING;
	}

	pd4rc.top_rule.rx_if = nircm->conn_rule.flow_interface_num;
	pd4rc.top_rule.tx_if = nircm->conn_rule.return_interface_num;
	ppe_status = ppe_drv_v4_create(&pd4rc);
	if (ppe_status != PPE_DRV_RET_SUCCESS) {
		nss_pvxlanmgr_warn("%px:PPE flow rule create failed with ppe_status %d\n", dev, ppe_status);
		return NSS_PVXLANMGR_FAILURE_IP_RULE_PPE_FLOW_CREATE;
	}

	t->tunnel_state |= NSS_PVXLANMGR_TUNNEL_STATE_PPE_FLOW_RULE_CONFIGURED;

	/*
	 * Create the acl rule to match 4-tuple for the downlink flow.
	 */
	acl_rule.cmn.cmn_flags |= PPE_ACL_RULE_CMN_FLAG_IPV4;

	acl_rule.valid_flags |= PPE_ACL_RULE_MATCH_TYPE_SIP_VALID;
	acl_rule.rules[PPE_ACL_RULE_MATCH_TYPE_SIP].rule.sip.ip_type = PPE_ACL_IP_TYPE_V4;
	acl_rule.rules[PPE_ACL_RULE_MATCH_TYPE_SIP].rule.sip.ip[0] = htonl(nircm->tuple.flow_ip);


	acl_rule.valid_flags |= PPE_ACL_RULE_MATCH_TYPE_DIP_VALID;
	acl_rule.rules[PPE_ACL_RULE_MATCH_TYPE_DIP].rule.dip.ip_type = PPE_ACL_IP_TYPE_V4;
	acl_rule.rules[PPE_ACL_RULE_MATCH_TYPE_DIP].rule.dip.ip[0] = htonl(nircm->tuple.return_ip);

	acl_rule.valid_flags |= PPE_ACL_RULE_MATCH_TYPE_PROTO_NEXTHDR_VALID;
	acl_rule.rules[PPE_ACL_RULE_MATCH_TYPE_PROTO_NEXTHDR].rule.proto_nexthdr.l3_v4proto_v6nexthdr =
				nircm->tuple.protocol;

	acl_rule.valid_flags |= PPE_ACL_RULE_MATCH_TYPE_DPORT_VALID;
	acl_rule.rules[PPE_ACL_RULE_MATCH_TYPE_DPORT].rule_flags = PPE_ACL_RULE_FLAG_DPORT_MASK;
	acl_rule.rules[PPE_ACL_RULE_MATCH_TYPE_DPORT].rule.dport.l4_port_min = htons(nircm->tuple.flow_ident);
	acl_rule.rules[PPE_ACL_RULE_MATCH_TYPE_DPORT].rule.dport.l4_port_max_mask = NSS_PVXLAN_ACL_PORT_MASK;

	/*
	 * Bind the ACL rule to the WAN port
	 */
	acl_rule.stype = PPE_ACL_RULE_SRC_TYPE_DEV;
	wan_dev = ppe_drv_dev_get_by_iface_idx(nircm->conn_rule.flow_interface_num);
	if (!wan_dev) {
		nss_pvxlanmgr_warn("%px: Failed to lookup wan net device associated with flow interface number %d\n",
					dev, nircm->conn_rule.flow_interface_num);
		memcpy(&pd4rd, &nircm->tuple, sizeof(struct ppe_drv_v4_rule_destroy));
		ppe_drv_v4_destroy(&pd4rd);
		return NSS_PVXLANMGR_FAILURE_IP_RULE_WAN_DEV_LOOKUP;
	}

	strlcpy(acl_rule.src.dev_name, wan_dev->name, IFNAMSIZ);

	/*
	 * Redirect the packets matching the 4 tuple to the pvxlan VP.
	 */
	acl_rule.action.flags = PPE_ACL_RULE_ACTION_FLAG_DEST_INFO_CHANGE_EN;
	strlcpy(acl_rule.action.dst.dev_name, dev->name, IFNAMSIZ);
	acl_rule.cmn.cmn_flags |=  PPE_ACL_RULE_CMN_FLAG_NO_RULEID;

	ppe_acl_status = ppe_acl_rule_create(&acl_rule);
	if(!(ppe_acl_status == PPE_ACL_RET_SUCCESS)) {
		nss_pvxlanmgr_warn("%px:PPE ACL rule create failed with ppe_acl_status = %d\n", dev, ppe_acl_status);
		memcpy(&pd4rd, &nircm->tuple, sizeof(struct ppe_drv_v4_rule_destroy));
		ppe_drv_v4_destroy(&pd4rd);
		return NSS_PVXLANMGR_FAILURE_IP_RULE_PPE_ACL_CREATE;
	}

	t->tunnel_state |= NSS_PVXLANMGR_TUNNEL_STATE_PPE_ACL_RULE_CONFIGURED;
	t->acl_rule_id = acl_rule.rule_id;

	return NSS_PVXLANMGR_SUCCESS;
}

/*
 * nss_pvxlanmgr_tunnel_ppe_create_ipv6_rule()
 *	Create a given connection in the PPE
 */
static nss_pvxlanmgr_status_t nss_pvxlanmgr_tunnel_ppe_create_ipv6_rule(struct net_device *dev, struct nss_ipv6_rule_create_msg *nircm, struct nss_pvxlanmgr_tunnel *t)
{
	struct ppe_drv_v6_rule_create pd6rc = {0};
	struct ppe_drv_v6_rule_destroy pd6rd = {0};
	struct ppe_acl_rule acl_rule = {0};
	struct net_device *wan_dev = NULL;
	ppe_drv_ret_t ppe_status = PPE_DRV_RET_SUCCESS;
	ppe_acl_ret_t ppe_acl_status = PPE_ACL_RET_SUCCESS;

	/*
	 * Copy over the 5 tuple information.
	 */
	memcpy(&pd6rc.tuple, &nircm->tuple, sizeof(struct ppe_drv_v6_5tuple));

	/*
	 * Copy over the connection rules and set the CONN_VALID flag
	 */
	pd6rc.conn_rule.rx_if = nircm->conn_rule.flow_interface_num;
	pd6rc.conn_rule.flow_mtu = (uint16_t)nircm->conn_rule.flow_mtu;
	ether_addr_copy(pd6rc.conn_rule.flow_mac, (u8 *)nircm->conn_rule.flow_mac);

	pd6rc.conn_rule.tx_if = nircm->conn_rule.return_interface_num;
	pd6rc.conn_rule.return_mtu = (uint16_t)nircm->conn_rule.return_mtu;
	ether_addr_copy(pd6rc.conn_rule.return_mac, (u8 *)nircm->conn_rule.return_mac);

	pd6rc.rule_flags |= PPE_DRV_V6_RULE_FLAG_RETURN_VALID | PPE_DRV_V6_RULE_FLAG_FLOW_VALID;

	/*
	 * Copy over the qos rules and set the QOS_VALID flag.
	 */
        if (nircm->valid_flags & NSS_IPV6_RULE_CREATE_QOS_VALID) {
		memcpy(&pd6rc.qos_rule, &nircm->qos_rule, sizeof(struct ppe_drv_qos_rule));
		pd6rc.valid_flags |= PPE_DRV_V6_VALID_FLAG_QOS;
	}

	/*
	 * Copy over the DSCP rule parameters.
	 */
	if (nircm->valid_flags & NSS_IPV6_RULE_CREATE_DSCP_MARKING_VALID) {
                memcpy(&pd6rc.dscp_rule, &nircm->dscp_rule, sizeof(struct ppe_drv_dscp_rule));
		pd6rc.rule_flags |= PPE_DRV_V6_RULE_FLAG_DSCP_MARKING;
		pd6rc.valid_flags |= PPE_DRV_V6_VALID_FLAG_DSCP_MARKING;
	}

	pd6rc.top_rule.rx_if = nircm->conn_rule.flow_interface_num;
	pd6rc.top_rule.tx_if = nircm->conn_rule.return_interface_num;
	ppe_status = ppe_drv_v6_create(&pd6rc);
	if (ppe_status != PPE_DRV_RET_SUCCESS) {
		nss_pvxlanmgr_warn("%px:PPE rule create failed\n", nircm);
		return NSS_PVXLANMGR_FAILURE_IP_RULE_PPE_FLOW_CREATE;
	}

	t->tunnel_state |= NSS_PVXLANMGR_TUNNEL_STATE_PPE_FLOW_RULE_CONFIGURED;

	/*
	 * Create the acl rule to match 4-tuple for the downlink flow.
	 */
	acl_rule.cmn.cmn_flags |= PPE_ACL_RULE_CMN_FLAG_IPV6;

	acl_rule.valid_flags |= PPE_ACL_RULE_MATCH_TYPE_SIP_VALID;
	acl_rule.rules[PPE_ACL_RULE_MATCH_TYPE_SIP].rule.sip.ip_type = PPE_ACL_IP_TYPE_V6;
	acl_rule.rules[PPE_ACL_RULE_MATCH_TYPE_SIP].rule.sip.ip[0] = htonl(nircm->tuple.flow_ip[0]);
	acl_rule.rules[PPE_ACL_RULE_MATCH_TYPE_SIP].rule.sip.ip[1] = htonl(nircm->tuple.flow_ip[1]);
	acl_rule.rules[PPE_ACL_RULE_MATCH_TYPE_SIP].rule.sip.ip[2] = htonl(nircm->tuple.flow_ip[2]);
	acl_rule.rules[PPE_ACL_RULE_MATCH_TYPE_SIP].rule.sip.ip[3] = htonl(nircm->tuple.flow_ip[3]);


	acl_rule.valid_flags |= PPE_ACL_RULE_MATCH_TYPE_DIP_VALID;
	acl_rule.rules[PPE_ACL_RULE_MATCH_TYPE_DIP].rule.dip.ip_type = PPE_ACL_IP_TYPE_V6;
	acl_rule.rules[PPE_ACL_RULE_MATCH_TYPE_DIP].rule.dip.ip[0] = htonl(nircm->tuple.return_ip[0]);
	acl_rule.rules[PPE_ACL_RULE_MATCH_TYPE_DIP].rule.dip.ip[1] = htonl(nircm->tuple.return_ip[1]);
	acl_rule.rules[PPE_ACL_RULE_MATCH_TYPE_DIP].rule.dip.ip[2] = htonl(nircm->tuple.return_ip[2]);
	acl_rule.rules[PPE_ACL_RULE_MATCH_TYPE_DIP].rule.dip.ip[3] = htonl(nircm->tuple.return_ip[3]);

	acl_rule.valid_flags |= PPE_ACL_RULE_MATCH_TYPE_PROTO_NEXTHDR_VALID;
	acl_rule.rules[PPE_ACL_RULE_MATCH_TYPE_PROTO_NEXTHDR].rule.proto_nexthdr.l3_v4proto_v6nexthdr =
				nircm->tuple.protocol;

	acl_rule.valid_flags |= PPE_ACL_RULE_MATCH_TYPE_DPORT_VALID;
	acl_rule.rules[PPE_ACL_RULE_MATCH_TYPE_DPORT].rule_flags = PPE_ACL_RULE_FLAG_DPORT_MASK;
	acl_rule.rules[PPE_ACL_RULE_MATCH_TYPE_DPORT].rule.dport.l4_port_min = htons(nircm->tuple.flow_ident);
	acl_rule.rules[PPE_ACL_RULE_MATCH_TYPE_DPORT].rule.dport.l4_port_max_mask = NSS_PVXLAN_ACL_PORT_MASK;

	/*
	 * Bind the ACL rule to the WAN port
	 */
	acl_rule.stype = PPE_ACL_RULE_SRC_TYPE_DEV;
	wan_dev = ppe_drv_dev_get_by_iface_idx(nircm->conn_rule.flow_interface_num);
	if (!wan_dev) {
		nss_pvxlanmgr_warn("%px: Failed to lookup wan net device associated with flow interface number %d\n",
					dev, nircm->conn_rule.flow_interface_num);
		memcpy(&pd6rd, &nircm->tuple, sizeof(struct ppe_drv_v6_rule_destroy));
		ppe_drv_v6_destroy(&pd6rd);
		return NSS_PVXLANMGR_FAILURE_IP_RULE_WAN_DEV_LOOKUP;
	}

	strlcpy(acl_rule.src.dev_name, wan_dev->name, IFNAMSIZ);

	/*
	 * Redirect the packets matching the 4 tuple to the pvxlan VP.
	 */
	acl_rule.action.flags = PPE_ACL_RULE_ACTION_FLAG_DEST_INFO_CHANGE_EN;
	strlcpy(acl_rule.action.dst.dev_name, dev->name, IFNAMSIZ);
	acl_rule.cmn.cmn_flags |=  PPE_ACL_RULE_CMN_FLAG_NO_RULEID;

	ppe_acl_status = ppe_acl_rule_create(&acl_rule);
	if(!(ppe_acl_status == PPE_ACL_RET_SUCCESS)) {
		nss_pvxlanmgr_warn("%px:PPE ACL rule create failed with ppe_acl_status = %d\n", dev, ppe_acl_status);
		memcpy(&pd6rd, &nircm->tuple, sizeof(struct ppe_drv_v6_rule_destroy));
		ppe_drv_v6_destroy(&pd6rd);
		return NSS_PVXLANMGR_FAILURE_IP_RULE_PPE_ACL_CREATE;
	}

	t->tunnel_state |= NSS_PVXLANMGR_TUNNEL_STATE_PPE_ACL_RULE_CONFIGURED;
	t->acl_rule_id = acl_rule.rule_id;

	return NSS_PVXLANMGR_SUCCESS;
}

/*
 * nss_pvxlanmgr_tunnel_get()
 *	Common function to verify tunnel_id and returns pointer to tunnel.
 */
struct nss_pvxlanmgr_tunnel *nss_pvxlanmgr_tunnel_get(struct net_device *dev, uint32_t tunnel_id)
{
	struct nss_pvxlanmgr_priv *priv;
	struct nss_pvxlanmgr_tunnel *t;

	if (!dev) {
		nss_pvxlanmgr_warn("Invalid net_device\n");
		return NULL;
	}

	if (tunnel_id >= NSS_PVXLANMGR_MAX_TUNNELS) {
		nss_pvxlanmgr_warn("%px: tunnel_id: %d out of range (%d)\n", dev, tunnel_id, NSS_PVXLANMGR_MAX_TUNNELS);
		return NULL;
	}

	dev_hold(dev);
	priv = netdev_priv(dev);
	t = &priv->tunnel[tunnel_id];
	if (!(t->tunnel_state & NSS_PVXLANMGR_TUNNEL_STATE_CONFIGURED)) {
		dev_put(dev);
		return NULL;
	}

	dev_put(dev);
	return t;
}

/*
 * nss_pvxlanmgr_tunnel_mac_remove()
 *	Send a pvxlan remove mac message to NSS core.
 */
nss_pvxlanmgr_status_t nss_pvxlanmgr_tunnel_mac_remove(struct net_device *dev, uint32_t tunnel_id, uint8_t *mac_addr)
{
	return nss_pvxlanmgr_tunnel_tx_msg_mac_del(dev, tunnel_id, mac_addr);
}
EXPORT_SYMBOL(nss_pvxlanmgr_tunnel_mac_remove);

/*
 * nss_pvxlanmgr_tunnel_mac_add()
 *	Send a pvxlan add mac message to NSS core.
 */
nss_pvxlanmgr_status_t nss_pvxlanmgr_tunnel_mac_add(struct net_device *dev, uint32_t tunnel_id, uint8_t *mac_addr, struct nss_pvxlanmgr_vxlan_hdr *pvxch)
{
	return nss_pvxlanmgr_tunnel_tx_msg_mac_add(dev, tunnel_id, mac_addr, pvxch);
}
EXPORT_SYMBOL(nss_pvxlanmgr_tunnel_mac_add);

/*
 * nss_pvxlanmgr_tunnel_unconfig_vp()
 *	PVxLAN tunnel vp unconfiguration message.
 */
nss_tx_status_t nss_pvxlanmgr_tunnel_unconfig_vp(struct nss_ctx_instance *ctx, uint32_t if_num, ppe_vp_num_t vp_num)
{
	struct nss_pvxlan_msg pvxlanmsg = {0};
	nss_tx_status_t status;

	/*
	 * Prepare the tunnel configuration parameter to send to NSS FW
	 */
	status = nss_pvxlanmgr_tunnel_tx_msg(ctx, &pvxlanmsg, if_num, NSS_PVXLAN_MSG_TYPE_UNCONFIG_VP, 0);
	if (status != NSS_TX_SUCCESS) {
		nss_pvxlanmgr_warn("%px: ctx: Tunnel Unconfig VP num %d error : %d\n", ctx, vp_num, status);
	}

	return status;
}

/*
 * nss_pvxlanmgr_tunnel_config_vp()
 *	PVxLAN tunnel vp configuration message.
 */
nss_tx_status_t nss_pvxlanmgr_tunnel_config_vp(struct nss_ctx_instance *ctx, uint32_t if_num, ppe_vp_num_t vp_num, bool ppe_to_host)
{
	struct nss_pvxlan_msg pvxlanmsg = {0};
	struct nss_pvxlan_vp_msg *cvpm;
	nss_tx_status_t status;

	/*
	 * Prepare the tunnel configuration parameter to send to NSS FW
	 */
	cvpm = &pvxlanmsg.msg.vp_config;
	cvpm->vp_num = vp_num;
	cvpm->ppe_to_host = ppe_to_host;
	status = nss_pvxlanmgr_tunnel_tx_msg(ctx, &pvxlanmsg, if_num, NSS_PVXLAN_MSG_TYPE_CONFIG_VP, sizeof(struct nss_pvxlan_vp_msg));
	if (status != NSS_TX_SUCCESS) {
		nss_pvxlanmgr_warn("%px: ctx: Tunnel Config VP num %d error : %d\n", ctx, vp_num, status);
	}

	return status;
}

/*
 * nss_pvxlanmgr_tunnel_tx_msg_disable()
 *	PVxLAN tunnel transmit disable messages.
 */
nss_tx_status_t nss_pvxlanmgr_tunnel_tx_msg_disable(struct nss_ctx_instance *ctx, uint32_t if_num)
{
	struct nss_pvxlan_msg pvxlanmsg = {0};
	nss_tx_status_t status;

	/*
	 * Prepare the tunnel configuration parameter to send to NSS FW
	 */

	status = nss_pvxlanmgr_tunnel_tx_msg(ctx, &pvxlanmsg, if_num, NSS_PVXLAN_MSG_TYPE_TUNNEL_DISABLE, 0);
	if (status != NSS_TX_SUCCESS) {
		nss_pvxlanmgr_warn("%px: ctx: Tunnel disable error : %d\n", ctx, status);
	}

	return status;
}

/*
 * nss_pvxlanmgr_tunnel_tx_msg_enable()
 *	PVxLAN tunnel transmit enable messages.
 */
nss_tx_status_t nss_pvxlanmgr_tunnel_tx_msg_enable(struct nss_ctx_instance *ctx,
					uint32_t if_num, uint32_t sibling_if_num)
{
	struct nss_pvxlan_msg pvxlanmsg = {0};
	nss_tx_status_t status;

	/*
	 * Prepare the tunnel configuration parameter to send to NSS FW
	 */
	pvxlanmsg.msg.enable.sibling_if_num = sibling_if_num;

	status = nss_pvxlanmgr_tunnel_tx_msg(ctx, &pvxlanmsg, if_num, NSS_PVXLAN_MSG_TYPE_TUNNEL_ENABLE, sizeof(struct nss_pvxlan_tunnel_state_msg));

	if (status != NSS_TX_SUCCESS) {
		nss_pvxlanmgr_warn("%px: ctx: Tunnel enable error : %d\n", ctx, status);
	}

	return status;
}

/*
 * nss_pvxlanmgr_tunnel_destroy()
 *	API for destroying a tunnel. All mac rules associated with this tunnel must be removed first.
 */
nss_pvxlanmgr_status_t nss_pvxlanmgr_tunnel_destroy(struct net_device *dev, uint32_t tunnel_id)
{
	struct nss_pvxlanmgr_priv *priv;
	struct nss_pvxlanmgr_tunnel *t;
	uint32_t if_num_host_inner, if_num_outer;
	struct nss_pvxlan_rule_msg *pvxlan_rule;
	nss_pvxlanmgr_status_t status;

	t = nss_pvxlanmgr_tunnel_get(dev, tunnel_id);
	if (!t) {
		nss_pvxlanmgr_warn("%px: tunnel %d: wrong argument for tunnel destroy\n", dev, tunnel_id);
		return NSS_PVXLANMGR_FAILURE_BAD_PARAM;
	}

	if (!(t->tunnel_state & NSS_PVXLANMGR_TUNNEL_STATE_CONFIGURED)) {
		nss_pvxlanmgr_warn("%px: tunnel %d is not configured yet\n", dev, tunnel_id);
		return NSS_PVXLANMGR_FAILURE_TUNNEL_NOT_CREATED;
	}

	dev_hold(dev);
	priv = netdev_priv(dev);
	if_num_host_inner = priv->if_num_host_inner;
	if_num_outer = priv->if_num_outer;
	nss_pvxlanmgr_info("%px: tunnel destroy is being called for tunnel %d and tunnel %d\n", dev, if_num_host_inner, if_num_outer);

	/*
	 * Destroy the IP rule first.
	 */
	if (t->pvxlan_rule.flags & NSS_PVXLAN_TUNNEL_IPV4) {
		status = nss_pvxlanmgr_tunnel_ppe_destroy_ipv4_rule(t);
	} else {
		status = nss_pvxlanmgr_tunnel_ppe_destroy_ipv6_rule(t);
	}

	if (status != NSS_PVXLANMGR_SUCCESS) {
		nss_pvxlanmgr_warn("%px: Unconfigure IP rule failed for tunnel: %d\n", dev, tunnel_id);
		dev_put(dev);
		return status;
	}

	/*
	 * Destroy the PVxLAN rule.
	 */
	pvxlan_rule = &t->pvxlan_rule;
	status = nss_pvxlanmgr_tunnel_pvxlan_rule_destroy(priv->pvxlan_ctx, if_num_host_inner, pvxlan_rule);
	if (status != NSS_PVXLANMGR_SUCCESS) {
		nss_pvxlanmgr_warn("%px: %d: PVXLAN rule destroy failed with status: %d\n", dev, if_num_host_inner, status);
		dev_put(dev);
		return NSS_PVXLANMGR_FAILURE_PVXLAN_RULE;
	}

	t->tunnel_state &= ~NSS_PVXLANMGR_TUNNEL_STATE_CONFIGURED;

	nss_pvxlanmgr_info("%px: Tunnel %d is destroyed\n", dev, tunnel_id);
	dev_put(dev);
	return NSS_PVXLANMGR_SUCCESS;
}
EXPORT_SYMBOL(nss_pvxlanmgr_tunnel_destroy);

/*
 * nss_pvxlanmgr_tunnel_ipv4_create()
 *	Common handling for creating IPv4 tunnel
 */
nss_pvxlanmgr_status_t nss_pvxlanmgr_tunnel_ipv4_create(struct net_device *dev, uint32_t tunnel_id,
							struct nss_ipv4_rule_create_msg *nircm, uint32_t pvxlan_src_port)
{
	struct nss_pvxlanmgr_priv *priv;
	struct nss_pvxlanmgr_tunnel *t;
	struct nss_pvxlan_rule_msg pvxlan_rule;
	nss_pvxlanmgr_status_t status = NSS_PVXLANMGR_SUCCESS;
	uint32_t if_num_host_inner, if_num_outer;
	uint16_t flags = 0;

	if (tunnel_id >= NSS_PVXLANMGR_MAX_TUNNELS) {
		nss_pvxlanmgr_warn("%px: invalid tunnel_id: %d max: NSS_PVXLANMGR_MAX_TUNNELS\n", dev, tunnel_id);
		return NSS_PVXLANMGR_FAILURE_BAD_PARAM;
	}

	if (!(nircm->tuple.protocol == IPPROTO_UDP)) {
		nss_pvxlanmgr_warn("%px: tunnel %d: wrong argument for protocol: %u\n", dev, tunnel_id, nircm->tuple.protocol);
		return NSS_PVXLANMGR_FAILURE_BAD_PARAM;
	}

	t = nss_pvxlanmgr_tunnel_get(dev, tunnel_id);
	if (t) {
		nss_pvxlanmgr_warn("%px: tunnel: %d already created\n", dev, tunnel_id);
		return NSS_PVXLANMGR_FAILURE_TUNNEL_EXISTS;
	}

	/*
	 * Copy over the IP rule information to PVxLAN rule for encap side.
	 * This will avoid any confusions because IP rule direction is ingress from WAN and
	 * PVxLAN encap rule direction is egress to the WAN port.
	 * pvxlan_src port for encapsulation is provided separately.
	 * So, use that one rather than the one in the IP rule.
	 */
	pvxlan_rule.encap.src_port = pvxlan_src_port;
	pvxlan_rule.encap.src.ip.ipv4 = nircm->tuple.return_ip;

	pvxlan_rule.encap.dest_port = nircm->tuple.return_ident;
	pvxlan_rule.encap.dest.ip.ipv4 = nircm->tuple.flow_ip;
	pvxlan_rule.tunnel_id = tunnel_id;
	pvxlan_rule.rps = -1;
	flags |= NSS_PVXLAN_TUNNEL_IPV4;
	flags |= NSS_PVXLAN_TUNNEL_UDP;
	pvxlan_rule.flags = flags;

	/*
	 * For the flow rule in uplink direction, the source port should be the pvxlan_src_port.
	 */
	nircm->tuple.flow_ident = nircm->tuple.return_ident;
	nircm->tuple.return_ident = pvxlan_src_port;

	dev_hold(dev);
	priv = netdev_priv(dev);
	if_num_host_inner = priv->if_num_host_inner;
	if_num_outer = priv->if_num_outer;

	status = nss_pvxlanmgr_tunnel_pvxlan_rule_create(priv->pvxlan_ctx, if_num_host_inner, &pvxlan_rule);
	nss_pvxlanmgr_info("%px: dynamic interface if_num is :%d and pvxlan tunnel status:%d\n", dev, if_num_host_inner, status);
	if (status != NSS_PVXLANMGR_SUCCESS) {
		nss_pvxlanmgr_warn("%px: %d: PVXLAN rule create failed with status: %d\n", dev, if_num_host_inner, status);
		dev_put(dev);
		return NSS_PVXLANMGR_FAILURE_PVXLAN_RULE;
	}

	t = &priv->tunnel[tunnel_id];
	status = nss_pvxlanmgr_tunnel_ppe_create_ipv4_rule(dev, nircm, t);
	if (status != NSS_PVXLANMGR_SUCCESS) {
		nss_pvxlanmgr_tunnel_pvxlan_rule_destroy(priv->pvxlan_ctx, if_num_host_inner, &pvxlan_rule);
		nss_pvxlanmgr_warn("%px: %d: IPv4 rule create failed with status: %d\n", dev, if_num_host_inner, status);
		dev_put(dev);
		return NSS_PVXLANMGR_FAILURE_IP_RULE;
	}


	nss_pvxlanmgr_info("%px: Pvxlan Tunnel create done tunnel_id:%d (%px) IPv4: %pI4h :%u, %pI4h :%u, p: %d\n", dev,
			tunnel_id, t, &nircm->tuple.flow_ip, nircm->tuple.flow_ident, &nircm->tuple.return_ip,
			nircm->tuple.return_ident, nircm->tuple.protocol);

	memcpy(&t->ip_rule.v4, nircm, sizeof(struct nss_ipv4_rule_create_msg));
	memcpy(&t->pvxlan_rule, &pvxlan_rule, sizeof(struct nss_pvxlan_rule_msg));

	/*
	 * Make it globally visible inside the netdev.
	 */
	t->tunnel_state |= NSS_PVXLANMGR_TUNNEL_STATE_CONFIGURED;

	dev_put(dev);
	return status;
}
EXPORT_SYMBOL(nss_pvxlanmgr_tunnel_ipv4_create);

/*
 * nss_pvxlanmgr_tunnel_ipv6_create()
 *	Common handling for creating IPv6 tunnel
 */
nss_pvxlanmgr_status_t nss_pvxlanmgr_tunnel_ipv6_create(struct net_device *dev, uint32_t tunnel_id,
							struct nss_ipv6_rule_create_msg *nircm, uint32_t pvxlan_src_port)
{
	struct nss_pvxlanmgr_priv *priv;
	struct nss_pvxlanmgr_tunnel *t;
	struct nss_pvxlan_rule_msg pvxlan_rule;
	nss_pvxlanmgr_status_t status = NSS_PVXLANMGR_SUCCESS;
	uint32_t if_num_host_inner, if_num_outer;
	uint16_t flags = 0;

	if (tunnel_id >= NSS_PVXLANMGR_MAX_TUNNELS) {
		nss_pvxlanmgr_warn("%px: invalid tunnel_id: %d max: NSS_PVXLANMGR_MAX_TUNNELS\n", dev, tunnel_id);
		return NSS_PVXLANMGR_FAILURE_BAD_PARAM;
	}

	if (!(nircm->tuple.protocol == IPPROTO_UDP ||
		nircm->tuple.protocol == IPPROTO_UDPLITE)) {
		nss_pvxlanmgr_warn("%px: tunnel %d: wrong argument for protocol %u\n", dev, tunnel_id, nircm->tuple.protocol);
		return NSS_PVXLANMGR_FAILURE_BAD_PARAM;
	}

	t = nss_pvxlanmgr_tunnel_get(dev, tunnel_id);
	if (t) {
		nss_pvxlanmgr_warn("%px: tunnel: %d already created\n", dev, tunnel_id);
		return NSS_PVXLANMGR_FAILURE_TUNNEL_EXISTS;
	}

	/*
	 * Copy over the IP rule information to PVxLAN rule for encap side.
	 * This will avoid any confusions because IP rule direction is ingress from WAN and
	 * PVxLAN encap rule direction is egress to the WAN port.
	 *
	 * pvxlan_src port is provided separately. So, use that one rather than the one in the IP rule.
	 */
	pvxlan_rule.encap.src_port = pvxlan_src_port;
	pvxlan_rule.encap.src.ip.ipv6[0] = nircm->tuple.return_ip[0];
	pvxlan_rule.encap.src.ip.ipv6[1] = nircm->tuple.return_ip[1];
	pvxlan_rule.encap.src.ip.ipv6[2] = nircm->tuple.return_ip[2];
	pvxlan_rule.encap.src.ip.ipv6[3] = nircm->tuple.return_ip[3];

	pvxlan_rule.encap.dest_port = nircm->tuple.return_ident;
	pvxlan_rule.encap.dest.ip.ipv6[0] = nircm->tuple.flow_ip[0];
	pvxlan_rule.encap.dest.ip.ipv6[1] = nircm->tuple.flow_ip[1];
	pvxlan_rule.encap.dest.ip.ipv6[2] = nircm->tuple.flow_ip[2];
	pvxlan_rule.encap.dest.ip.ipv6[3] = nircm->tuple.flow_ip[3];
	pvxlan_rule.tunnel_id = tunnel_id;
	pvxlan_rule.rps = -1;
	flags |= NSS_PVXLAN_TUNNEL_IPV6;

	if (nircm->tuple.protocol == IPPROTO_UDP) {
		flags |= NSS_PVXLAN_TUNNEL_UDP;
	} else {
		flags |= NSS_PVXLAN_TUNNEL_UDPLite;
		flags |= NSS_PVXLAN_TUNNEL_ENCAP_UDPLITE_HDR_CSUM;
	}
	pvxlan_rule.flags = flags;

	/*
	 * For the flow rule in uplink direction, the source port should be the pvxlan_src_port.
	 */
	nircm->tuple.flow_ident = nircm->tuple.return_ident;
	nircm->tuple.return_ident = pvxlan_src_port;

	dev_hold(dev);
	priv = netdev_priv(dev);
	if_num_host_inner = priv->if_num_host_inner;
	if_num_outer = priv->if_num_outer;

	status = nss_pvxlanmgr_tunnel_pvxlan_rule_create(priv->pvxlan_ctx, if_num_host_inner, &pvxlan_rule);
	nss_pvxlanmgr_info("%px: dynamic interface if_num is :%d and pvxlan tunnel status:%d\n", dev, if_num_host_inner, status);
	if (status != NSS_PVXLANMGR_SUCCESS) {
		nss_pvxlanmgr_warn("%px: %d: PVXLAN rule create failed with status: %d\n", dev, if_num_host_inner, status);
		dev_put(dev);
		return NSS_PVXLANMGR_FAILURE_PVXLAN_RULE;
	}

	t = &priv->tunnel[tunnel_id];
	status = nss_pvxlanmgr_tunnel_ppe_create_ipv6_rule(dev, nircm, t);
	if (status != NSS_PVXLANMGR_SUCCESS) {
		nss_pvxlanmgr_tunnel_pvxlan_rule_destroy(priv->pvxlan_ctx, if_num_host_inner, &pvxlan_rule);
		nss_pvxlanmgr_warn("%px: %d: IPv6 rule create failed with status: %d\n", dev, if_num_host_inner, status);
		dev_put(dev);
		return NSS_PVXLANMGR_FAILURE_IP_RULE;
	}

	nss_pvxlanmgr_info("%px: Pvxlan Tunnel create done tunnel_id:%d (%px)\n", dev, tunnel_id, t);

	memcpy(&t->ip_rule.v6, nircm, sizeof(struct nss_ipv6_rule_create_msg));
	memcpy(&t->pvxlan_rule, &pvxlan_rule, sizeof(struct nss_pvxlan_rule_msg));

	/*
	 * Make it globally visible inside the netdev.
	 */
	t->tunnel_state |= NSS_PVXLANMGR_TUNNEL_STATE_CONFIGURED;

	dev_put(dev);
	return status;
}
EXPORT_SYMBOL(nss_pvxlanmgr_tunnel_ipv6_create);
