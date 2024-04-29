/*
 **************************************************************************
 * Copyright (c) 2019-2020, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
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
	struct nss_pvxlan_msg pvxlanmsg;
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

	memset(&pvxlanmsg, 0, sizeof(struct nss_pvxlan_msg));

	/*
	 * Set MAC rule message
	 */
	ncfrm = &pvxlanmsg.msg.mac_del;
	ncfrm->tunnel_id = tunnel_id;
	memcpy(ncfrm->mac_addr, mac_addr, ETH_ALEN);

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
	struct nss_pvxlan_msg pvxlanmsg;
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

	memset(&pvxlanmsg, 0, sizeof(struct nss_pvxlan_msg));

	/*
	 * Set MAC rule message
	 */
	ncfrm = &pvxlanmsg.msg.mac_add;
	ncfrm->flags = pvxch->flags;
	ncfrm->vnet_id = pvxch->vnet_id;
	ncfrm->policy_id = pvxch->gpid;
	ncfrm->tunnel_id = tunnel_id;
	memcpy(ncfrm->mac_addr, mac_addr, ETH_ALEN);

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
	struct nss_pvxlan_msg pvxlanmsg;
	nss_tx_status_t status;

	nss_pvxlanmgr_info("%px: ctx: PVxLAN Rule src_port: 0x%d dest_port:0x%d\n", ctx,
	    msg->encap.src_port, msg->encap.dest_port);

	/*
	 * Prepare the tunnel configuration parameter to send to NSS FW
	 */
	memset(&pvxlanmsg, 0, sizeof(struct nss_pvxlan_msg));
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
	struct nss_pvxlan_msg pvxlanmsg;
	nss_tx_status_t status;

	nss_pvxlanmgr_info("%px: ctx: PVxLAN Rule src_port: 0x%d dest_port:0x%d\n", ctx,
	    msg->encap.src_port, msg->encap.dest_port);

	/*
	 * Prepare the tunnel configuration parameter to send to NSS FW
	 */
	memset(&pvxlanmsg, 0, sizeof(struct nss_pvxlan_msg));
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
 * nss_pvxlanmgr_tunnel_destroy_ipv4_rule()
 *	Destroy a given connection in the NSS
 */
static nss_tx_status_t nss_pvxlanmgr_tunnel_destroy_ipv4_rule(struct nss_ctx_instance *nss_ctx, struct nss_ipv4_5tuple *ni5t)
{
	struct nss_ipv4_msg nim;
	nss_tx_status_t status;

	nss_pvxlanmgr_info("%px: ctx: Destroy IPv4: %pI4h :%u, %pI4h :%u, p: %d\n", nss_ctx,
		&ni5t->flow_ip, ni5t->flow_ident, &ni5t->return_ip, ni5t->return_ident, ni5t->protocol);

	nss_ipv4_msg_init(&nim, NSS_IPV4_RX_INTERFACE, NSS_IPV4_TX_DESTROY_RULE_MSG,
			sizeof(struct nss_ipv4_rule_destroy_msg), NULL, NULL);

	nim.msg.rule_destroy.tuple = *ni5t;
	status = nss_ipv4_tx_sync(nss_ctx, &nim);
	if (status != NSS_TX_SUCCESS) {
		nss_pvxlanmgr_warn("%px: Destroy IPv4 message failed %d\n", nss_ctx, status);
	}

	return status;
}

/*
 * nss_pvxlanmgr_tunnel_destroy_ipv6_rule()
 *	Destroy a given connection in the NSS
 */
static nss_tx_status_t nss_pvxlanmgr_tunnel_destroy_ipv6_rule(struct nss_ctx_instance *nss_ctx, struct nss_ipv6_5tuple *ni5t)
{
	struct nss_ipv6_msg nim;
	nss_tx_status_t status;

	nss_pvxlanmgr_info("%px: ctx: Destroy IPv6: %pI6:%u, %pI6:%u, p: %d\n", nss_ctx,
		ni5t->flow_ip, ni5t->flow_ident, ni5t->return_ip, ni5t->return_ident, ni5t->protocol);

	nss_ipv6_msg_init(&nim, NSS_IPV6_RX_INTERFACE, NSS_IPV6_TX_DESTROY_RULE_MSG,
			sizeof(struct nss_ipv6_rule_destroy_msg), NULL, NULL);

	nim.msg.rule_destroy.tuple = *ni5t;
	status = nss_ipv6_tx_sync(nss_ctx, &nim);
	if (status != NSS_TX_SUCCESS) {
		nss_pvxlanmgr_warn("%px: Destroy IPv6 message failed %d\n", nss_ctx, status);
	}
	return status;
}

/*
 * nss_pvxlanmgr_tunnel_create_ipv4_rule()
 *	Create a given connection in the NSS
 */
static nss_tx_status_t nss_pvxlanmgr_tunnel_create_ipv4_rule(struct nss_ctx_instance *nss_ctx, struct nss_ipv4_rule_create_msg *nircm)
{
	struct nss_ipv4_msg nim;
	nss_tx_status_t status;

	nss_ipv4_msg_init(&nim, NSS_IPV4_RX_INTERFACE, NSS_IPV4_TX_CREATE_RULE_MSG,
			sizeof(struct nss_ipv4_rule_create_msg), NULL, NULL);

	nim.msg.rule_create = *nircm;
	status = nss_ipv4_tx_sync(nss_ctx, &nim);
	if (status != NSS_TX_SUCCESS) {
		nss_pvxlanmgr_warn("%px: Create IPv4 message failed %d\n", nss_ctx, status);
	}

	return status;
}

/*
 * nss_pvxlanmgr_tunnel_create_ipv6_rule()
 *	Create a given connection in the NSS
 */
static nss_tx_status_t nss_pvxlanmgr_tunnel_create_ipv6_rule(struct nss_ctx_instance *nss_ctx, struct nss_ipv6_rule_create_msg *nircm)
{
	struct nss_ipv6_msg nim;
	nss_tx_status_t status;

	memset(&nim, 0, sizeof (struct nss_ipv6_msg));
	nss_ipv6_msg_init(&nim, NSS_IPV6_RX_INTERFACE, NSS_IPV6_TX_CREATE_RULE_MSG,
			sizeof(struct nss_ipv6_rule_create_msg), NULL, NULL);

	nim.msg.rule_create = *nircm;
	status = nss_ipv6_tx_sync(nss_ctx, &nim);
	if (status != NSS_TX_SUCCESS) {
		nss_pvxlanmgr_warn("%px: Create IPv6 message failed %d\n", nss_ctx, status);
	}

	return status;
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
 * nss_pvxlanmgr_tunnel_tx_msg_disable()
 *	PVxLAN tunnel transmit disable messages.
 */
nss_tx_status_t nss_pvxlanmgr_tunnel_tx_msg_disable(struct nss_ctx_instance *ctx, uint32_t if_num)
{
	struct nss_pvxlan_msg pvxlanmsg;
	nss_tx_status_t status;

	/*
	 * Prepare the tunnel configuration parameter to send to NSS FW
	 */
	memset(&pvxlanmsg, 0, sizeof(struct nss_pvxlan_msg));

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
	struct nss_pvxlan_msg pvxlanmsg;
	nss_tx_status_t status;

	/*
	 * Prepare the tunnel configuration parameter to send to NSS FW
	 */
	memset(&pvxlanmsg, 0, sizeof(struct nss_pvxlan_msg));
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
	struct nss_ctx_instance *nss_ctx;
	struct nss_pvxlanmgr_priv *priv;
	struct nss_pvxlanmgr_tunnel *t;
	nss_tx_status_t nss_status = NSS_TX_SUCCESS;
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

	pvxlan_rule = &t->pvxlan_rule;
	status = nss_pvxlanmgr_tunnel_pvxlan_rule_destroy(priv->pvxlan_ctx, if_num_host_inner, pvxlan_rule);
	if (status != NSS_PVXLANMGR_SUCCESS) {
		nss_pvxlanmgr_warn("%px: %d: PVXLAN rule destroy failed with status: %d\n", dev, if_num_host_inner, status);
		dev_put(dev);
		return NSS_PVXLANMGR_FAILURE_PVXLAN_RULE;
	}

	if (t->pvxlan_rule.flags & NSS_PVXLAN_TUNNEL_IPV4) {
		nss_ctx = priv->ipv4_ctx;
		nss_status = nss_pvxlanmgr_tunnel_destroy_ipv4_rule(nss_ctx, &t->ip_rule.v4.tuple);
	} else {
		nss_ctx = priv->ipv6_ctx;
		nss_status = nss_pvxlanmgr_tunnel_destroy_ipv6_rule(nss_ctx, &t->ip_rule.v6.tuple);
	}

	if (nss_status != NSS_TX_SUCCESS) {
		nss_pvxlanmgr_tunnel_pvxlan_rule_create(priv->pvxlan_ctx, if_num_host_inner, pvxlan_rule);
		nss_pvxlanmgr_warn("%px: Unconfigure IP rule failed for tunnel: %d\n",
			dev, tunnel_id);
		dev_put(dev);
		return NSS_PVXLANMGR_FAILURE_IP_DESTROY_RULE;
	}

	t->tunnel_state &= ~NSS_PVXLANMGR_TUNNEL_STATE_CONFIGURED;

	nss_pvxlanmgr_info("%px: Tunnel %d is completely destroyed\n", dev, tunnel_id);
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
	struct nss_ctx_instance *nss_ctx;
	struct nss_pvxlanmgr_priv *priv;
	struct nss_pvxlanmgr_tunnel *t;
	struct nss_pvxlan_rule_msg pvxlan_rule;
	nss_pvxlanmgr_status_t status = NSS_PVXLANMGR_SUCCESS;
	uint32_t if_num_host_inner, if_num_outer;
	uint16_t flags = 0;
	nss_tx_status_t nss_status = NSS_TX_SUCCESS;

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

	pvxlan_rule.encap.dest_port = nircm->tuple.flow_ident;
	pvxlan_rule.encap.dest.ip.ipv4 = nircm->tuple.flow_ip;
	pvxlan_rule.tunnel_id = tunnel_id;
	pvxlan_rule.rps = -1;
	flags |= NSS_PVXLAN_TUNNEL_IPV4;
	flags |= NSS_PVXLAN_TUNNEL_UDP;
	pvxlan_rule.flags = flags;

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

	nss_ctx = priv->ipv4_ctx;
	nss_status = nss_pvxlanmgr_tunnel_create_ipv4_rule(nss_ctx, nircm);
	if (nss_status != NSS_TX_SUCCESS) {
		nss_pvxlanmgr_tunnel_pvxlan_rule_destroy(priv->pvxlan_ctx, if_num_host_inner, &pvxlan_rule);
		nss_pvxlanmgr_warn("%px: %d: IPv4 rule create failed with status: %d\n", dev, if_num_host_inner, nss_status);
		dev_put(dev);
		return NSS_PVXLANMGR_FAILURE_IP_RULE;
	}

	t = &priv->tunnel[tunnel_id];

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
	struct nss_ctx_instance *nss_ctx;
	struct nss_pvxlanmgr_priv *priv;
	struct nss_pvxlanmgr_tunnel *t;
	struct nss_pvxlan_rule_msg pvxlan_rule;
	nss_pvxlanmgr_status_t status = NSS_PVXLANMGR_SUCCESS;
	uint32_t if_num_host_inner, if_num_outer;
	uint16_t flags = 0;
	nss_tx_status_t nss_status = NSS_TX_SUCCESS;

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

	pvxlan_rule.encap.dest_port = nircm->tuple.flow_ident;
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

	nss_ctx = priv->ipv6_ctx;
	nss_status = nss_pvxlanmgr_tunnel_create_ipv6_rule(nss_ctx, nircm);
	if (nss_status != NSS_TX_SUCCESS) {
		nss_pvxlanmgr_tunnel_pvxlan_rule_destroy(priv->pvxlan_ctx, if_num_host_inner, &pvxlan_rule);
		nss_pvxlanmgr_warn("%px: %d: IPv6 rule create failed with status: %d\n", dev, if_num_host_inner, nss_status);
		dev_put(dev);
		return NSS_PVXLANMGR_FAILURE_IP_RULE;
	}

	t = &priv->tunnel[tunnel_id];

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
