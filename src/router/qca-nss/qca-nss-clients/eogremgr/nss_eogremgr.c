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

/*
 * nss_eogremgr.c
 *	NSS EOGRE manager
 */

#include <linux/of.h>
#include <nss_api_if.h>
#include <nss_cmn.h>
#include "nss_connmgr_gre_public.h"
#include "nss_eogremgr.h"
#include "nss_eogremgr_priv.h"

/*
 * Mapping table from tunnel-id to rule/GRE netdev
 */
struct nss_eogremgr_tunnel {
	union {
		struct nss_ipv4_rule_create_msg v4;	/* IPv4 rule structure. */
		struct nss_ipv6_rule_create_msg v6;	/* IPv6 rule structure. */
	} ip_rule;

	bool is_ipv4;			/* Set when tunnel is ipv4. */
	struct net_device *gre_dev;	/* Store the gre netdev associated with the tunnel. */
};

struct nss_eogremgr_tunnel *tunnels[NSS_EOGREMGR_MAX_TUNNELS] = {NULL};

/*
 * nss_eogremgr_get_if_num_inner()
 * 	Get the GRE inner interface number associated with the tunnel.
 */
int nss_eogremgr_get_if_num_inner(uint32_t tunnel_id)
{
	int inner_ifnum;

	/*
	 * Check if the tunnel exists.
	 */
	if (!tunnels[tunnel_id]) {
		nss_eogremgr_warn("Could not find tunnel data, for tunnel id : %d\n", tunnel_id);
		return -1;
	}

	/*
	 * Find inner ifnum for the GRE netdev.
	 */
	inner_ifnum = nss_cmn_get_interface_number_by_dev_and_type(
			tunnels[tunnel_id]->gre_dev, NSS_DYNAMIC_INTERFACE_TYPE_GRE_INNER);
	if (inner_ifnum < 0) {
		nss_eogremgr_warn("No GRE inner interface registered for tunnel id : %d\n", tunnel_id);
		return -1;
	}

	/*
	 * Append the core-id to the ifnum.
	 */
	return nss_gre_ifnum_with_core_id(inner_ifnum);
}
EXPORT_SYMBOL(nss_eogremgr_get_if_num_inner);

/*
 * nss_eogremgr_tunnel_disable()
 * 	Function to disable eogre tunnel.
 */
nss_eogremgr_status_t nss_eogremgr_tunnel_disable(uint32_t tunnel_id)
{
	int ret;

	/*
	 * Check if the tunnel exists.
	 */
	if (!tunnels[tunnel_id]) {
		nss_eogremgr_warn("Could not find tunnel data for tunnel id : %d\n", tunnel_id);
		return NSS_EOGRE_ERR_NETDEV_DISABLE_FAILED;
	}

	/*
	 * Notify GRE client to disable GRE tunnel.
	 */
	ret = nss_connmgr_gre_dev_close(tunnels[tunnel_id]->gre_dev);
	if (ret != 0) {
		nss_eogremgr_warn("%s: Unable to disable tunnel:%d\n\n",
				tunnels[tunnel_id]->gre_dev->name, tunnel_id);
		return NSS_EOGRE_ERR_NETDEV_DISABLE_FAILED;
	}

	return NSS_EOGRE_SUCCESS;
}
EXPORT_SYMBOL(nss_eogremgr_tunnel_disable);

/*
 * nss_eogremgr_tunnel_enable()
 * 	Function to enable eogre tunnel.
 */
nss_eogremgr_status_t nss_eogremgr_tunnel_enable(uint32_t tunnel_id)
{
	int ret;

	/*
	 * Check if the tunnel exists.
	 */
	if (!tunnels[tunnel_id]) {
		nss_eogremgr_warn("Could not find tunnel data for tunnel id : %d\n", tunnel_id);
		return NSS_EOGRE_ERR_NETDEV_ENABLE_FAILED;
	}

	/*
	 * Notify GRE client to enable GRE tunnel.
	 */
	ret = nss_connmgr_gre_dev_open(tunnels[tunnel_id]->gre_dev);
	if (ret != 0) {
		nss_eogremgr_warn("%s: Unable to enable tunnel:%d\n",
				tunnels[tunnel_id]->gre_dev->name, tunnel_id);
		return NSS_EOGRE_ERR_NETDEV_ENABLE_FAILED;
	}

	return NSS_EOGRE_SUCCESS;
}
EXPORT_SYMBOL(nss_eogremgr_tunnel_enable);

/*
 * nss_eogremgr_tunnel_destroy_ipv6_rule()
 * 	Destroy a given IPv6 connection in the NSS
 */
static nss_tx_status_t nss_eogremgr_tunnel_destroy_ipv6_rule(struct nss_ctx_instance *nss_ctx, struct nss_ipv6_5tuple *ni5t)
{
	struct nss_ipv6_msg nim;
	nss_tx_status_t status;

	nss_eogremgr_info("%px: ctx: Destroy IPv6: %pI6:%u, %pI6:%u, p: %d\n", nss_ctx,
			&ni5t->flow_ip, ni5t->flow_ident, &ni5t->return_ip, ni5t->return_ident, ni5t->protocol);

	nss_ipv6_msg_init(&nim, NSS_IPV6_RX_INTERFACE, NSS_IPV6_TX_DESTROY_RULE_MSG,
			sizeof(struct nss_ipv6_rule_destroy_msg), NULL, NULL);

	nim.msg.rule_destroy.tuple = *ni5t;
	status = nss_ipv6_tx_sync(nss_ctx, &nim);
	if (status != NSS_TX_SUCCESS) {
		nss_eogremgr_warn("%px: Destroy IPv6 message failed %d\n", nss_ctx, status);
	}

	return status;
}

/*
 * nss_eogremgr_tunnel_destroy_ipv4_rule()
 * 	Destroy a given IPv4 connection in the NSS
 */
static nss_tx_status_t nss_eogremgr_tunnel_destroy_ipv4_rule(struct nss_ctx_instance *nss_ctx, struct nss_ipv4_5tuple *ni5t)
{
	struct nss_ipv4_msg nim;
	nss_tx_status_t status;

	nss_eogremgr_info("%px: ctx: Destroy IPv4: %pI4h :%u, %pI4h :%u, p: %d\n", nss_ctx,
			&ni5t->flow_ip, ni5t->flow_ident, &ni5t->return_ip, ni5t->return_ident, ni5t->protocol);

	nss_ipv4_msg_init(&nim, NSS_IPV4_RX_INTERFACE, NSS_IPV4_TX_DESTROY_RULE_MSG,
			sizeof(struct nss_ipv4_rule_destroy_msg), NULL, NULL);

	nim.msg.rule_destroy.tuple = *ni5t;
	status = nss_ipv4_tx_sync(nss_ctx, &nim);
	if (status != NSS_TX_SUCCESS) {
		nss_eogremgr_warn("%px: Destroy IPv4 message failed %d\n", nss_ctx, status);
	}

	return status;
}

/*
 * nss_eogremgr_tunnel_create_ipv6_rule()
 * 	Create a given IPv6 connection in the NSS
 */
static nss_tx_status_t nss_eogremgr_tunnel_create_ipv6_rule(struct nss_ctx_instance *nss_ctx, struct nss_ipv6_rule_create_msg *nircm)
{
	struct nss_ipv6_msg nim;
	nss_tx_status_t status;

	nss_ipv6_msg_init(&nim, NSS_IPV6_RX_INTERFACE, NSS_IPV6_TX_CREATE_RULE_MSG,
			sizeof(struct nss_ipv6_rule_create_msg), NULL, NULL);

	nim.msg.rule_create = *nircm;
	status = nss_ipv6_tx_sync(nss_ctx, &nim);
	if (status != NSS_TX_SUCCESS) {
		nss_eogremgr_warn("%px: Create IPv6 message failed %d\n", nss_ctx, status);
	}

	return status;
}

/*
 * nss_eogremgr_tunnel_create_ipv4_rule()
 * 	Create a given IPv4 connection in the NSS
 */
static nss_tx_status_t nss_eogremgr_tunnel_create_ipv4_rule(struct nss_ctx_instance *nss_ctx, struct nss_ipv4_rule_create_msg *nircm)
{
	struct nss_ipv4_msg nim;
	nss_tx_status_t status;

	nss_ipv4_msg_init(&nim, NSS_IPV4_RX_INTERFACE, NSS_IPV4_TX_CREATE_RULE_MSG,
			sizeof(struct nss_ipv4_rule_create_msg), NULL, NULL);

	nim.msg.rule_create = *nircm;
	status = nss_ipv4_tx_sync(nss_ctx, &nim);
	if (status != NSS_TX_SUCCESS) {
		nss_eogremgr_warn("%px: Create IPv4 message failed %d\n", nss_ctx, status);
	}

	return status;
}

/*
 * nss_eogremgr_tunnel_destroy()
 * 	Destroy the EoGRE tunnel.
 */
nss_eogremgr_status_t nss_eogremgr_tunnel_destroy(uint32_t tunnel_id)
{
	struct nss_ctx_instance *nss_ctx_v4, *nss_ctx_v6;
	nss_tx_status_t status;
	enum nss_connmgr_gre_err_codes err_code_gre;

	/*
	 * Check if tunnel_id is in range.
	 */
	if (tunnel_id >= NSS_EOGREMGR_MAX_TUNNELS) {
		nss_eogremgr_warn("Invalid param tunnel_id: %d max: %d\n",
				tunnel_id, NSS_EOGREMGR_MAX_TUNNELS);
		return NSS_EOGRE_ERR_INVALID_TUNNEL_ID;
	}

	/*
	 * Check if the tunnel_id exists.
	 */
	if (!tunnels[tunnel_id]) {
		nss_eogremgr_warn("Could not find tunnel data for tunnel id : %d\n", tunnel_id);
		return NSS_EOGRE_ERR_INVALID_TUNNEL_ID;
	}

	/*
	 * Destroy IPv4/IPv6 rule
	 */
	if (tunnels[tunnel_id]->is_ipv4) {
		nss_ctx_v4 = nss_ipv4_get_mgr();
		if (!nss_ctx_v4) {
			nss_eogremgr_warn("Tunnel: %d: could not get IPv4 ctx\n", tunnel_id);
			return NSS_EOGRE_ERR_BAD_NSS_CTX;
		}
		status = nss_eogremgr_tunnel_destroy_ipv4_rule(nss_ctx_v4, &tunnels[tunnel_id]->ip_rule.v4.tuple);
	} else {
		nss_ctx_v6 = nss_ipv6_get_mgr();
		if (!nss_ctx_v6) {
			nss_eogremgr_warn("Tunnel: %d: could not get IPv6 ctx\n", tunnel_id);
			return NSS_EOGRE_ERR_BAD_NSS_CTX;
		}
		status = nss_eogremgr_tunnel_destroy_ipv6_rule(nss_ctx_v6, &tunnels[tunnel_id]->ip_rule.v6.tuple);
	}

	if (status != NSS_TX_SUCCESS) {
		nss_eogremgr_warn("Destroy IP rule failed for tunnel: %d\n", tunnel_id);
		return NSS_EOGRE_ERR_DESTROY_IP_RULE_FAILED;
	}

	/*
	 * Destroy GRE tunnel
	 */
	err_code_gre = nss_connmgr_gre_destroy_interface(tunnels[tunnel_id]->gre_dev);
	if (err_code_gre != GRE_SUCCESS) {
		nss_eogremgr_warn("Could not destroy GRE interface: %d, err=%d\n", tunnel_id, err_code_gre);

		/*
		 * Add the ipv4/upv6 rule back, since tunnel is not destroyed.
		 */
		if (tunnels[tunnel_id]->is_ipv4) {
			status = nss_eogremgr_tunnel_create_ipv4_rule(
					nss_ctx_v4, &tunnels[tunnel_id]->ip_rule.v4);
		} else {
			status = nss_eogremgr_tunnel_create_ipv6_rule(
					nss_ctx_v6, &tunnels[tunnel_id]->ip_rule.v6);
		}

		if (status != NSS_TX_SUCCESS) {
			nss_eogremgr_warn("IPv4/IPv6 rule create from nss_eogremgr_tunnel_destroy() failed with status: %d\n",
					status);
			return NSS_EOGRE_ERR_CREATE_IP_RULE_FAILED;
		}

		return NSS_EOGRE_TUNNEL_DESTROY_FAILED;
	}

	nss_eogremgr_info("Tunnel: %d is destroyed\n", tunnel_id);
	kfree(tunnels[tunnel_id]);
	tunnels[tunnel_id] = NULL;
	return NSS_EOGRE_SUCCESS;
}
EXPORT_SYMBOL(nss_eogremgr_tunnel_destroy);

/*
 * nss_eogremgr_tunnel_ipv6_create()
 * 	API to create EoGRE IPv6 tunnel and Push IPv6 rule to NSS.
 */
nss_eogremgr_status_t nss_eogremgr_tunnel_ipv6_create(struct nss_connmgr_gre_cfg *cfg,
	       	struct nss_ipv6_rule_create_msg *nircm, uint32_t tunnel_id)
{
	struct net_device *gre_dev;
	struct nss_ctx_instance *nss_ctx;
	enum nss_connmgr_gre_err_codes err_code_gre;
	nss_tx_status_t status;
	int gre_outer_ifnum;

	/*
	 * Check if tunnel_id is in range.
	 */
	if (tunnel_id >= NSS_EOGREMGR_MAX_TUNNELS) {
		nss_eogremgr_warn("Invalid tunnel_id: %d max: %d\n",
				tunnel_id, NSS_EOGREMGR_MAX_TUNNELS);
		return NSS_EOGRE_ERR_INVALID_TUNNEL_ID;
	}

	/*
	 * Check if tunnel_id is already in use.
	 */
	if (tunnels[tunnel_id]) {
		nss_eogremgr_warn("Tunnel already exist for this tunnel_id = %d, netdev = %s\n",
				tunnel_id, tunnels[tunnel_id]->gre_dev->name);
		return NSS_EOGRE_ERR_TUNNEL_ID_EXIST;
	}

	/*
	 * GRE API to create GRE standard interface
	 */
	gre_dev = nss_connmgr_gre_create_interface(cfg, &err_code_gre);
	if (!gre_dev) {
		nss_eogremgr_warn("Could not create GRE interface, err=%d\n", err_code_gre);
		return NSS_EOGRE_ERR_GRE_CREATE_FAILED;
	}

	nss_eogremgr_info("GRE interface %s created with tunnel_id: %u\n", gre_dev->name, tunnel_id);

	gre_outer_ifnum = nss_cmn_get_interface_number_by_dev_and_type(
			                        gre_dev, NSS_DYNAMIC_INTERFACE_TYPE_GRE_OUTER);
	if (gre_outer_ifnum < 0) {
		nss_eogremgr_warn("No GRE outer interface registered for gre netdev: %s\n", gre_dev->name);
		nss_connmgr_gre_destroy_interface(gre_dev);
		return NSS_EOGRE_ERR_INVALID_GRE_IFNUM;
	}

	nss_ctx = nss_ipv6_get_mgr();
	if (!nss_ctx) {
		nss_eogremgr_warn("Tunnel: %d could not get IPv6 ctx\n", tunnel_id);
		nss_connmgr_gre_destroy_interface(gre_dev);
		return NSS_EOGRE_ERR_BAD_NSS_CTX;
	}

	/*
	 * Configure ifnum for GRE decap node before adding IPv6 rule.
	 */
	nircm->conn_rule.flow_interface_num = gre_outer_ifnum;

	/*
	 * Fill the flow/return ident with 32 bit key, to create a 5-tuple match for GRE with key
	 */
	if (cfg->okey_valid) {
		nircm->tuple.flow_ident = cfg->okey;
		nircm->tuple.return_ident = cfg->okey;
	}

	/*
	 * Push IPv6 rule to NSS
	 */
	status = nss_eogremgr_tunnel_create_ipv6_rule(nss_ctx, nircm);
	if (status != NSS_TX_SUCCESS) {
		nss_eogremgr_warn("IPv6 rule create failed with status: %d\n", status);
		nss_connmgr_gre_destroy_interface(gre_dev);
		return NSS_EOGRE_ERR_CREATE_IP_RULE_FAILED;
	}

	nss_eogremgr_info("EoGRE Tunnel create done tunnel_id:%d IPv6: %pI6h :%u, %pI6h :%u, p: %d\n",
			tunnel_id, &nircm->tuple.flow_ip, nircm->tuple.flow_ident, &nircm->tuple.return_ip,
			nircm->tuple.return_ident, nircm->tuple.protocol);

	/*
	 * Add EoGRE tunnel entry
	 */
	tunnels[tunnel_id] = kzalloc(sizeof(struct nss_eogremgr_tunnel), GFP_ATOMIC);
	if(!tunnels[tunnel_id]) {
		nss_eogremgr_warn("Failed to allocate tunnel memory for tunnel_id : %d\n", tunnel_id);
		nss_eogremgr_tunnel_destroy_ipv6_rule(nss_ctx, &nircm->tuple);
		nss_connmgr_gre_destroy_interface(gre_dev);
		return NSS_EOGRE_ERR_ALLOC_TUNNEL_FAILED;
	}

	memcpy(&tunnels[tunnel_id]->ip_rule.v6, nircm, sizeof(struct nss_ipv6_rule_create_msg));
	tunnels[tunnel_id]->is_ipv4 = false;
	tunnels[tunnel_id]->gre_dev = gre_dev;

	return NSS_EOGRE_SUCCESS;
}
EXPORT_SYMBOL(nss_eogremgr_tunnel_ipv6_create);

/*
 * nss_eogremgr_tunnel_ipv4_create()
 * 	API to create EoGRE IPv4 tunnel and push IPv4 rule to NSS.
 */
nss_eogremgr_status_t nss_eogremgr_tunnel_ipv4_create(struct nss_connmgr_gre_cfg *cfg,
	       	struct nss_ipv4_rule_create_msg *nircm, uint32_t tunnel_id)
{
	struct net_device *gre_dev;
	struct nss_ctx_instance *nss_ctx;
	int gre_outer_ifnum;
	nss_tx_status_t status;
	enum nss_connmgr_gre_err_codes err_code_gre;

	/*
	 * Check if tunnel_id is in range.
	 */
	if (tunnel_id >= NSS_EOGREMGR_MAX_TUNNELS) {
		nss_eogremgr_warn("Invalid param tunnel_id: %d max: %d\n", tunnel_id, NSS_EOGREMGR_MAX_TUNNELS);
		return NSS_EOGRE_ERR_INVALID_TUNNEL_ID;
	}

	/*
	 * Check if tunnel_id is already in use.
	 */
	if (tunnels[tunnel_id]) {
		nss_eogremgr_warn("Tunnel already exist for this tunnel_id = %d, netdev = %s\n",
				tunnel_id, tunnels[tunnel_id]->gre_dev->name);
		return NSS_EOGRE_ERR_TUNNEL_ID_EXIST;
	}

	/*
	 * Create GRE standard interface
	 */
	gre_dev = nss_connmgr_gre_create_interface(cfg, &err_code_gre);
	if (!gre_dev) {
		nss_eogremgr_warn("Could not create GRE interface, err=%d\n", err_code_gre);
		return NSS_EOGRE_ERR_GRE_CREATE_FAILED;
	}

	nss_eogremgr_info("GRE interface %s created with tunnel_id: %u\n", gre_dev->name, tunnel_id);

	gre_outer_ifnum = nss_cmn_get_interface_number_by_dev_and_type(
			gre_dev, NSS_DYNAMIC_INTERFACE_TYPE_GRE_OUTER);
	if (gre_outer_ifnum < 0) {
		nss_eogremgr_warn("No GRE outer interface registered for gre netdev: %s\n", gre_dev->name);
		nss_connmgr_gre_destroy_interface(gre_dev);
		return NSS_EOGRE_ERR_INVALID_GRE_IFNUM;
	}

	nss_ctx = nss_ipv4_get_mgr();
	if (!nss_ctx) {
		nss_eogremgr_warn("EoGRE couldn't get IPv4 ctx\n");
		nss_connmgr_gre_destroy_interface(gre_dev);
		return NSS_EOGRE_ERR_BAD_NSS_CTX;
	}

	/*
	 * Configure ifnum for GRE decap node before adding IPv4 rule.
	 */
	nircm->conn_rule.flow_interface_num = gre_outer_ifnum;

	/*
	 * Fill the flow/return ident with 32 bit key, to create a 5-tuple match for GRE with key
	 */
	if (cfg->okey_valid) {
		nircm->tuple.flow_ident = cfg->okey;
		nircm->tuple.return_ident = cfg->okey;
		nircm->conn_rule.flow_ident_xlate = cfg->okey;
		nircm->conn_rule.return_ident_xlate = cfg->okey;
	}

	/*
	 * Push IPv4 rule to NSS.
	 */
	status = nss_eogremgr_tunnel_create_ipv4_rule(nss_ctx, nircm);
	if (status != NSS_TX_SUCCESS) {
		nss_eogremgr_warn("IPv4 rule create failed with status: %d\n", status);
		nss_connmgr_gre_destroy_interface(gre_dev);
		return NSS_EOGRE_ERR_CREATE_IP_RULE_FAILED;
	}

	nss_eogremgr_info("EoGRE Tunnel create done tunnel_id:%d IPv4: %pI4h :%u, %pI4h :%u, p: %d\n",
			tunnel_id, &nircm->tuple.flow_ip, nircm->tuple.flow_ident, &nircm->tuple.return_ip,
			nircm->tuple.return_ident, nircm->tuple.protocol);

	/*
	 * Add EoGRE tunnel entry
	 */
	tunnels[tunnel_id] = kzalloc(sizeof(struct nss_eogremgr_tunnel), GFP_ATOMIC);
	if (!tunnels[tunnel_id]) {
		nss_eogremgr_warn("Failed to allocate tunnel memory for tunnel_id : %d\n", tunnel_id);
		nss_eogremgr_tunnel_destroy_ipv4_rule(nss_ctx, &nircm->tuple);
		nss_connmgr_gre_destroy_interface(gre_dev);
		return NSS_EOGRE_ERR_ALLOC_TUNNEL_FAILED;
	}

	memcpy(&tunnels[tunnel_id]->ip_rule.v4, nircm, sizeof(struct nss_ipv4_rule_create_msg));
	tunnels[tunnel_id]->is_ipv4 = true;
	tunnels[tunnel_id]->gre_dev = gre_dev;

	return NSS_EOGRE_SUCCESS;
}
EXPORT_SYMBOL(nss_eogremgr_tunnel_ipv4_create);

/*
 * nss_eogremgr_destroy_tunnel_data()
 * 	Destroy EoGRE global tunnel structure
 */
static int nss_eogremgr_destroy_tunnel_data(void)
{
	int tunnel_id, status, fail_count = 0;

	for (tunnel_id = 0; tunnel_id < NSS_EOGREMGR_MAX_TUNNELS; tunnel_id++) {
		if (!tunnels[tunnel_id]) {
			continue;
		}

		status = nss_eogremgr_tunnel_destroy(tunnel_id);
		if (!status) {
			nss_eogremgr_warn("Unable to delete GRE netdev and IPv4/IPv6 rule for tunnel_id: %u\n", tunnel_id);
			fail_count++;
		}
	}

	return fail_count;
}

/*
 * nss_eogremgr_exit_module()
 * 	Tunnel EoGRE module exit function
 */
static void __exit nss_eogremgr_exit_module(void)
{
	int status;

	/*
	 * Destroy tunnel data
	 */
	status = nss_eogremgr_destroy_tunnel_data();
	if (status) {
		nss_eogremgr_info("Error in module unloading, Unable to delete %d tunnels \n", status);
		return;
	}

	nss_eogremgr_info("module unloaded\n");
}

/*
 * nss_eogremgr_init_module()
 * 	Tunnel EoGRE module init function
 */
static int __init nss_eogremgr_init_module(void)
{
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return 0;
	}

	nss_eogremgr_info("module %s loaded\n", NSS_CLIENT_BUILD_ID);

	return 0;

}
module_init(nss_eogremgr_init_module);
module_exit(nss_eogremgr_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS EOGRE manager");
