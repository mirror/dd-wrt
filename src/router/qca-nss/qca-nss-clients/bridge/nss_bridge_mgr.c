/*
 **************************************************************************
 * Copyright (c) 2016-2021, The Linux Foundation. All rights reserved.
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

/*
 * nss_bridge_mgr.c
 *	NSS to HLOS Bridge Interface manager
 */
#include <linux/sysctl.h>
#include <linux/etherdevice.h>
#include <linux/if_vlan.h>
#include <linux/of.h>
#include <linux/if_bridge.h>
#include <net/bonding.h>
#include <nss_vlan_mgr.h>
#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
#include <ref/ref_vsi.h>
#include <fal/fal_fdb.h>
#include <fal/fal_stp.h>
#include <fal/fal_acl.h>
#include <fal/fal_api.h>
#include <fal/fal_port_ctrl.h>
#endif
#include <nss_api_if.h>

#if defined(NSS_BRIDGE_MGR_OVS_ENABLE)
#include <ovsmgr.h>
#endif

#include "nss_bridge_mgr_priv.h"

/*
 * Module parameter to enable/disable OVS bridge.
 */
static bool ovs_enabled = false;

static struct nss_bridge_mgr_context br_mgr_ctx;

/*
 * Module parameter to enable/disable FDB learning.
 */
static bool fdb_disabled = false;

/*
 * nss_bridge_mgr_create_instance()
 *	Create a bridge instance.
 */
static struct nss_bridge_pvt *nss_bridge_mgr_create_instance(struct net_device *dev)
{
	struct nss_bridge_pvt *br;

#if !defined(NSS_BRIDGE_MGR_OVS_ENABLE)
	if (!netif_is_bridge_master(dev))
		return NULL;
#else
	/*
	 * When OVS is enabled, we have to check for both bridge master
	 * and OVS master.
	 */
	if (!netif_is_bridge_master(dev) && !ovsmgr_is_ovs_master(dev))
		return NULL;
#endif

	br = kzalloc(sizeof(*br), GFP_KERNEL);
	if (!br)
		return NULL;

	INIT_LIST_HEAD(&br->list);

	return br;
}

/*
 * nss_bridge_mgr_delete_instance()
 *	Delete a bridge instance from bridge list and free the bridge instance.
 */
static void nss_bridge_mgr_delete_instance(struct nss_bridge_pvt *br)
{
	spin_lock(&br_mgr_ctx.lock);
	if (!list_empty(&br->list))
		list_del(&br->list);

	spin_unlock(&br_mgr_ctx.lock);

	kfree(br);
}

/*
 * nss_bridge_mgr_find_instance()
 *	Find a bridge instance from bridge list.
 */
struct nss_bridge_pvt *nss_bridge_mgr_find_instance(struct net_device *dev)
{
	struct nss_bridge_pvt *br;

#if !defined(NSS_BRIDGE_MGR_OVS_ENABLE)
	if (!netif_is_bridge_master(dev))
		return NULL;
#else
	/*
	 * When OVS is enabled, we have to check for both bridge master
	 * and OVS master.
	 */
	if (!netif_is_bridge_master(dev) && !ovsmgr_is_ovs_master(dev))
		return NULL;
#endif
	/*
	 * Do we have it on record?
	 */
	spin_lock(&br_mgr_ctx.lock);
	list_for_each_entry(br, &br_mgr_ctx.list, list) {
		if (br->dev == dev) {
			spin_unlock(&br_mgr_ctx.lock);
			return br;
		}
	}

	spin_unlock(&br_mgr_ctx.lock);
	return NULL;
}

#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
/*
 * nss_bridge_mgr_enable_fdb_learning()
 *	Enable fdb learning in PPE.
 */
static int nss_bridge_mgr_enable_fdb_learning(struct nss_bridge_pvt *br)
{
	fal_vsi_newaddr_lrn_t newaddr_lrn;
	fal_vsi_stamove_t sta_move;

	/*
	 * Enable station move
	 */
	sta_move.stamove_en = 1;
	sta_move.action = FAL_MAC_FRWRD;
	if (fal_vsi_stamove_set(NSS_BRIDGE_MGR_SWITCH_ID, br->vsi, &sta_move)) {
		nss_bridge_mgr_warn("%px: Failed to enable station move for Bridge vsi\n", br);
		return -1;
	}

	/*
	 * Enable FDB learning in PPE
	 */
	newaddr_lrn.lrn_en = 1;
	newaddr_lrn.action = FAL_MAC_FRWRD;
	if (fal_vsi_newaddr_lrn_set(NSS_BRIDGE_MGR_SWITCH_ID, br->vsi, &newaddr_lrn)) {
		nss_bridge_mgr_warn("%px: Failed to enable FDB learning for Bridge vsi\n", br);
		goto disable_sta_move;
	}

	/*
	 * Send a notification to NSS for FDB learning enable.
	 */
	if (nss_bridge_tx_set_fdb_learn_msg(br->ifnum, NSS_BRIDGE_FDB_LEARN_ENABLE) != NSS_TX_SUCCESS) {
		nss_bridge_mgr_warn("%px: Tx message failed for FDB learning status\n", br);
		goto disable_fdb_learning;
	}

	return 0;

disable_fdb_learning:
	newaddr_lrn.lrn_en = 0;
	newaddr_lrn.action = FAL_MAC_FRWRD;
	if (fal_vsi_newaddr_lrn_set(NSS_BRIDGE_MGR_SWITCH_ID, br->vsi, &newaddr_lrn))
		nss_bridge_mgr_warn("%px: Failed to disable FDB learning for Bridge vsi\n", br);

disable_sta_move:
	sta_move.stamove_en = 0;
	sta_move.action = FAL_MAC_FRWRD;
	if (fal_vsi_stamove_set(NSS_BRIDGE_MGR_SWITCH_ID, br->vsi, &sta_move))
		nss_bridge_mgr_warn("%px: Failed to disable station move for Bridge vsi\n", br);

	return -1;
}

/*
 * nss_bridge_mgr_disable_fdb_learning()
 *	Disable fdb learning in PPE
 *
 * For the first time a bond interface join bridge, we need to use flow based rule.
 * FDB learing/station move need to be disabled.
 */
static int nss_bridge_mgr_disable_fdb_learning(struct nss_bridge_pvt *br)
{
	fal_vsi_newaddr_lrn_t newaddr_lrn;
	fal_vsi_stamove_t sta_move;

	/*
	 * Disable station move
	 */
	sta_move.stamove_en = 0;
	sta_move.action = FAL_MAC_FRWRD;
	if (fal_vsi_stamove_set(NSS_BRIDGE_MGR_SWITCH_ID, br->vsi, &sta_move)) {
		nss_bridge_mgr_warn("%px: Failed to disable station move for Bridge vsi\n", br);
		return -1;
	}

	/*
	 * Disable FDB learning in PPE
	 */
	newaddr_lrn.lrn_en = 0;
	newaddr_lrn.action = FAL_MAC_FRWRD;
	if (fal_vsi_newaddr_lrn_set(NSS_BRIDGE_MGR_SWITCH_ID, br->vsi, &newaddr_lrn)) {
		nss_bridge_mgr_warn("%px: Failed to disable FDB learning for Bridge vsi\n", br);
		goto enable_sta_move;
	}

	/*
	 * Flush FDB table for the bridge vsi
	 */
	if (fal_fdb_entry_del_byfid(NSS_BRIDGE_MGR_SWITCH_ID, br->vsi, FAL_FDB_DEL_STATIC)) {
		nss_bridge_mgr_warn("%px: Failed to flush FDB table for vsi:%d in PPE\n", br, br->vsi);
		goto enable_fdb_learning;
	}

	/*
	 * Send a notification to NSS for FDB learning disable.
	 */
	if (nss_bridge_tx_set_fdb_learn_msg(br->ifnum, NSS_BRIDGE_FDB_LEARN_DISABLE) != NSS_TX_SUCCESS) {
		nss_bridge_mgr_warn("%px: Tx message failed for FDB learning status\n", br);
		goto enable_fdb_learning;
	}

	return 0;

enable_fdb_learning:
	newaddr_lrn.lrn_en = 1;
	newaddr_lrn.action = FAL_MAC_FRWRD;
	if (fal_vsi_newaddr_lrn_set(NSS_BRIDGE_MGR_SWITCH_ID, br->vsi, &newaddr_lrn))
		nss_bridge_mgr_warn("%px: Failed to enable FDB learning for Bridge vsi\n", br);

enable_sta_move:
	sta_move.stamove_en = 1;
	sta_move.action = FAL_MAC_FRWRD;
	if (fal_vsi_stamove_set(NSS_BRIDGE_MGR_SWITCH_ID, br->vsi, &sta_move))
		nss_bridge_mgr_warn("%px: Failed to enable station move for Bridge vsi\n", br);

	return -1;
}

/*
 * nss_bridge_mgr_add_bond_slave()
 *	A slave interface being added to a bond master that belongs to a bridge.
 */
static int nss_bridge_mgr_add_bond_slave(struct net_device *bond_master,
		struct net_device *slave, struct nss_bridge_pvt *b_pvt)
{
	uint32_t *port_vsi;
	int32_t port_id;
	int32_t ifnum;
	int32_t lagid = 0;
	int32_t bondid = -1;

	/*
	 * Figure out the aggregation id of this slave
	 */
#if defined(BONDING_SUPPORT)
	bondid = bond_get_id(bond_master);
#endif
	if (bondid < 0) {
		nss_bridge_mgr_warn("%px: Invalid LAG group id 0x%x\n",
				b_pvt, bondid);
		return -1;
	}

	lagid = bondid + NSS_LAG0_INTERFACE_NUM;

	nss_bridge_mgr_trace("%px: Bond Slave %s is added bridge\n",
			b_pvt, slave->name);

	ifnum = nss_cmn_get_interface_number_by_dev(slave);

	/*
	 * Hardware supports only PHYSICAL Ports as trunk ports
	 */
	if (!NSS_BRIDGE_MGR_IF_IS_TYPE_PHYSICAL(ifnum)) {
		nss_bridge_mgr_warn("%px: Interface %s is not Physical Interface\n",
				b_pvt, slave->name);
		return -1;
	}

	nss_bridge_mgr_trace("%px: Interface %s adding into bridge\n",
			b_pvt, slave->name);
	port_id = ifnum;

	/*
	 * Take bridge lock as we are updating vsi and port forwarding
	 * details in PPE Hardware
	 */
	spin_lock(&br_mgr_ctx.lock);
	port_vsi = &b_pvt->port_vsi[port_id - 1];

	if (ppe_port_vsi_get(NSS_BRIDGE_MGR_SWITCH_ID, port_id, port_vsi)) {
		spin_unlock(&br_mgr_ctx.lock);
		nss_bridge_mgr_warn("%px: Couldn't get VSI for port %d\n",
				b_pvt, port_id);
		return -1;
	}

	if (ppe_port_vsi_set(NSS_BRIDGE_MGR_SWITCH_ID, port_id, b_pvt->vsi)) {
		spin_unlock(&br_mgr_ctx.lock);
		nss_bridge_mgr_warn("%px: Couldn't set bridge VSI for port %d\n",
				b_pvt, port_id);
		return -1;
	}
	spin_unlock(&br_mgr_ctx.lock);

	if (nss_bridge_tx_join_msg(b_pvt->ifnum,
				slave) != NSS_TX_SUCCESS) {
		if (ppe_port_vsi_set(NSS_BRIDGE_MGR_SWITCH_ID, port_id, *port_vsi))
			nss_bridge_mgr_warn("%px: Couldn't set bridge VSI for port %d\n", b_pvt, port_id);
		nss_bridge_mgr_warn("%px: Couldn't add port %d in bridge",
				b_pvt, port_id);
		return -1;
	}

	spin_lock(&br_mgr_ctx.lock);
	b_pvt->lag_ports[port_id - 1] = lagid;
	spin_unlock(&br_mgr_ctx.lock);

	return 0;
}

/*
 * nss_bridge_mgr_del_bond_slave()
 *	A slave interface being removed from a bond master that belongs to a bridge.
 */
static int nss_bridge_mgr_del_bond_slave(struct net_device *bond_master,
		struct net_device *slave, struct nss_bridge_pvt *b_pvt)
{
	uint32_t *port_vsi;
	int32_t port_id;
	int32_t ifnum;
	int32_t lagid = 0;
	int32_t bondid = -1;

	/*
	 * Figure out the aggregation id of this slave
	 */
#if defined(BONDING_SUPPORT)
	bondid = bond_get_id(bond_master);
#endif
	if (bondid < 0) {
		nss_bridge_mgr_warn("%px: Invalid LAG group id 0x%x\n",
				b_pvt, bondid);
		return -1;
	}

	lagid = bondid + NSS_LAG0_INTERFACE_NUM;

	nss_bridge_mgr_trace("%px: Bond Slave %s leaving bridge\n",
			b_pvt, slave->name);

	ifnum = nss_cmn_get_interface_number_by_dev(slave);

	/*
	 * Hardware supports only PHYSICAL Ports as trunk ports
	 */
	if (!NSS_BRIDGE_MGR_IF_IS_TYPE_PHYSICAL(ifnum)) {
		nss_bridge_mgr_warn("%px: Interface %s is not Physical Interface\n",
				b_pvt, slave->name);
		return -1;
	}

	nss_bridge_mgr_trace("%px: Interface %s leaving from bridge\n",
			b_pvt, slave->name);

	port_id = (fal_port_t)ifnum;

	/*
	 * Take bridge lock as we are updating vsi and port forwarding
	 * details in PPE Hardware
	 */
	spin_lock(&br_mgr_ctx.lock);
	port_vsi = &b_pvt->port_vsi[port_id - 1];

	if (b_pvt->lag_ports[port_id - 1] != lagid) {
		spin_unlock(&br_mgr_ctx.lock);
		return -1;
	}

	if (ppe_port_vsi_set(NSS_BRIDGE_MGR_SWITCH_ID, port_id, *port_vsi)) {
		spin_unlock(&br_mgr_ctx.lock);
		nss_bridge_mgr_warn("%px: failed to restore port VSI for port %d\n", b_pvt, port_id);
		return -1;
	}
	spin_unlock(&br_mgr_ctx.lock);

	if (nss_bridge_tx_leave_msg(b_pvt->ifnum,
				slave) != NSS_TX_SUCCESS) {
		ppe_port_vsi_set(NSS_BRIDGE_MGR_SWITCH_ID, port_id, b_pvt->vsi);
		nss_bridge_mgr_trace("%px: Failed to remove port %d from bridge\n",
				b_pvt, port_id);
		return -1;
	}

	spin_lock(&br_mgr_ctx.lock);
	b_pvt->lag_ports[port_id - 1] = 0;
	spin_unlock(&br_mgr_ctx.lock);

	/*
	 * Set STP state to forwarding after bond physical port leaves bridge
	 */
	fal_stp_port_state_set(NSS_BRIDGE_MGR_SWITCH_ID, NSS_BRIDGE_MGR_SPANNING_TREE_ID,
					port_id, FAL_STP_FORWARDING);
	return 0;
}

/*
 * nss_bridge_mgr_bond_fdb_join()
 *	Update FDB state when a bond interface joining bridge.
 */
static int nss_bridge_mgr_bond_fdb_join(struct nss_bridge_pvt *b_pvt)
{
	/*
	 * If already other bond devices are attached to bridge,
	 * only increment bond_slave_num,
	 */
	spin_lock(&br_mgr_ctx.lock);
	if (b_pvt->bond_slave_num) {
		b_pvt->bond_slave_num++;
		spin_unlock(&br_mgr_ctx.lock);
		return NOTIFY_DONE;
	}
	b_pvt->bond_slave_num = 1;
	spin_unlock(&br_mgr_ctx.lock);

	/*
	 * This is the first bond device being attached to bridge. In order to enforce Linux
	 * bond slave selection in bridge flows involving bond interfaces, we need to disable
	 * fdb learning on this bridge master to allow flow based bridging.
	 */
	if (nss_bridge_mgr_disable_fdb_learning(b_pvt) < 0) {
		return NOTIFY_BAD;
	}

	return NOTIFY_DONE;
}

/*
 * nss_bridge_mgr_bond_master_join()
 *	Add a bond interface to bridge
 */
static int nss_bridge_mgr_bond_master_join(struct net_device *bond_master,
		struct nss_bridge_pvt *b_pvt)
{
	struct net_device *slave;

	/*
	 * bond enslave/release path is protected by rtnl lock
	 */
	ASSERT_RTNL();

	/*
	 * Wait for RCU QS
	 */
	synchronize_rcu();

	/*
	 * Join each of the bonded slaves to the VSI group
	 */
	for_each_netdev(&init_net, slave) {
		if (netdev_master_upper_dev_get(slave) != bond_master) {
			continue;
		}

		if (nss_bridge_mgr_add_bond_slave(bond_master, slave, b_pvt)) {
			nss_bridge_mgr_warn("%px: Failed to add slave (%s) state in Bridge\n", b_pvt, slave->name);
			goto cleanup;
		}
	}

	if (nss_bridge_mgr_bond_fdb_join(b_pvt) == NOTIFY_DONE) {
		return NOTIFY_DONE;
	}

cleanup:

	for_each_netdev(&init_net, slave) {
		if (netdev_master_upper_dev_get(slave) != bond_master) {
			continue;
		}

		if (nss_bridge_mgr_del_bond_slave(bond_master, slave, b_pvt)) {
			nss_bridge_mgr_warn("%px: Failed to remove slave (%s) from Bridge\n", b_pvt, slave->name);
		}
	}

	return NOTIFY_BAD;
}

/*
 * nss_bridge_mgr_bond_fdb_leave()
 *	Update FDB state when a bond interface leaving bridge.
 */
static int nss_bridge_mgr_bond_fdb_leave(struct nss_bridge_pvt *b_pvt)
{

	nss_bridge_mgr_assert(b_pvt->bond_slave_num == 0);

	/*
	 * If more than one bond devices are attached to bridge,
	 * only decrement the bond_slave_num
	 */
	spin_lock(&br_mgr_ctx.lock);
	if (b_pvt->bond_slave_num > 1) {
		b_pvt->bond_slave_num--;
		spin_unlock(&br_mgr_ctx.lock);
		return NOTIFY_DONE;
	}
	b_pvt->bond_slave_num = 0;
	spin_unlock(&br_mgr_ctx.lock);

	/*
	 * The last bond interface is removed from bridge, we can switch back to FDB
	 * learning mode.
	 */
	if (!fdb_disabled && (nss_bridge_mgr_enable_fdb_learning(b_pvt) < 0)) {
		nss_bridge_mgr_warn("%px: Failed to enable fdb learning. fdb_disabled: %d\n", b_pvt, fdb_disabled);
		return NOTIFY_BAD;
	}

	return NOTIFY_DONE;
}


/*
 * nss_bridge_mgr_bond_master_leave()
 *	Remove a bond interface from bridge
 */
static int nss_bridge_mgr_bond_master_leave(struct net_device *bond_master,
		struct nss_bridge_pvt *b_pvt)
{
	struct net_device *slave;

	nss_bridge_mgr_assert(b_pvt->bond_slave_num == 0);

	ASSERT_RTNL();

	synchronize_rcu();

	/*
	 * Remove each of the bonded slaves from the VSI group
	 */
	for_each_netdev(&init_net, slave) {
		if (netdev_master_upper_dev_get(slave) != bond_master) {
			continue;
		}

		if (nss_bridge_mgr_del_bond_slave(bond_master, slave, b_pvt)) {
			nss_bridge_mgr_warn("%px: Failed to remove slave (%s) from Bridge\n", b_pvt, slave->name);
			goto cleanup;
		}
	}

	if (nss_bridge_mgr_bond_fdb_leave(b_pvt) == NOTIFY_DONE) {
		return NOTIFY_DONE;
	}

cleanup:
	for_each_netdev(&init_net, slave) {
		if (netdev_master_upper_dev_get(slave) != bond_master) {
			continue;
		}

		if (nss_bridge_mgr_add_bond_slave(bond_master, slave, b_pvt)) {
			nss_bridge_mgr_warn("%px: Failed to add slave (%s) state in Bridge\n", b_pvt, slave->name);
		}
	}

	return NOTIFY_BAD;
}

/*
 * nss_bridge_mgr_l2_exception_acl_enable()
 *	Create ACL rule to enable L2 exception.
 */
static bool nss_bridge_mgr_l2_exception_acl_enable(void)
{
	sw_error_t error;
	fal_acl_rule_t rule;

	memset(&rule, 0, sizeof(rule));
	error = fal_acl_list_creat(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID,
				NSS_BRIDGE_MGR_ACL_LIST_PRIORITY);
	if (error != SW_OK) {
		nss_bridge_mgr_warn("List creation failed with error = %d\n", error);
		return false;
	}

	/*
	 * Enable excpetion for packets with fragments.
	 */
	rule.rule_type = FAL_ACL_RULE_IP4;
	rule.is_fragement_mask = 1;
	rule.is_fragement_val = A_TRUE;
	FAL_FIELD_FLG_SET(rule.field_flg, FAL_ACL_FIELD_L3_FRAGMENT);
	FAL_ACTION_FLG_SET(rule.action_flg, FAL_ACL_ACTION_RDTCPU);

	error = fal_acl_rule_add(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID,
				NSS_BRIDGE_MGR_ACL_FRAG_RULE_ID, NSS_BRIDGE_MGR_ACL_RULE_NR, &rule);
	if (error != SW_OK) {
		nss_bridge_mgr_warn("Could not add fragment acl rule, error = %d\n", error);
		goto frag_fail;
	}

	/*
	 * Enable excpetion for TCP FIN.
	 */
	memset(&rule, 0, sizeof(rule));

	rule.rule_type = FAL_ACL_RULE_IP4;
	rule.tcp_flag_val = 0x1 & 0x3f;
	rule.tcp_flag_mask = 0x1 & 0x3f;
	FAL_FIELD_FLG_SET(rule.field_flg, FAL_ACL_FIELD_TCP_FLAG);
	FAL_ACTION_FLG_SET(rule.action_flg, FAL_ACL_ACTION_RDTCPU);

	error = fal_acl_rule_add(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID,
				NSS_BRIDGE_MGR_ACL_FIN_RULE_ID, NSS_BRIDGE_MGR_ACL_RULE_NR, &rule);
	if (error != SW_OK) {
		nss_bridge_mgr_warn("Could not add TCP FIN rule, error = %d\n", error);
		goto fin_fail;
	}

	/*
	 * Enable excpetion for TCP SYN.
	 */
	memset(&rule, 0, sizeof(rule));

	rule.rule_type = FAL_ACL_RULE_IP4;
	rule.tcp_flag_val = 0x2 & 0x3f;
	rule.tcp_flag_mask = 0x2 & 0x3f;
	FAL_FIELD_FLG_SET(rule.field_flg, FAL_ACL_FIELD_TCP_FLAG);
	FAL_ACTION_FLG_SET(rule.action_flg, FAL_ACL_ACTION_RDTCPU);

	error = fal_acl_rule_add(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID,
				NSS_BRIDGE_MGR_ACL_SYN_RULE_ID, NSS_BRIDGE_MGR_ACL_RULE_NR, &rule);
	if (error != SW_OK) {
		nss_bridge_mgr_warn("Could not add TCP SYN rule, error = %d\n", error);
		goto syn_fail;
	}

	/*
	 * Enable excpetion for TCP RST.
	 */
	memset(&rule, 0, sizeof(rule));

	rule.rule_type = FAL_ACL_RULE_IP4;
	rule.tcp_flag_val = 0x4 & 0x3f;
	rule.tcp_flag_mask = 0x4 & 0x3f;
	FAL_FIELD_FLG_SET(rule.field_flg, FAL_ACL_FIELD_TCP_FLAG);
	FAL_ACTION_FLG_SET(rule.action_flg, FAL_ACL_ACTION_RDTCPU);

	error = fal_acl_rule_add(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID,
				NSS_BRIDGE_MGR_ACL_RST_RULE_ID, NSS_BRIDGE_MGR_ACL_RULE_NR, &rule);
	if (error != SW_OK) {
		nss_bridge_mgr_warn("Could not add TCP RST rule, error = %d\n", error);
		goto rst_fail;
	}

	/*
	 * Bind ACL list with service code
	 */
	error = fal_acl_list_bind(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID,
				FAL_ACL_DIREC_IN, FAL_ACL_BIND_SERVICE_CODE, NSS_PPE_SC_VLAN_FILTER_BYPASS);
	if (error != SW_OK) {
		nss_bridge_mgr_warn("Could not bind ACL list, error = %d\n", error);
		goto bind_fail;
	}

	nss_bridge_mgr_info("Created ACL rule\n");
	return true;

bind_fail:
	error = fal_acl_rule_delete(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID,
				NSS_BRIDGE_MGR_ACL_RST_RULE_ID, NSS_BRIDGE_MGR_ACL_RULE_NR);
	if (error != SW_OK) {
		nss_bridge_mgr_warn("TCP RST rule deletion failed, error %d\n", error);
	}

rst_fail:
	error = fal_acl_rule_delete(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID,
				NSS_BRIDGE_MGR_ACL_SYN_RULE_ID, NSS_BRIDGE_MGR_ACL_RULE_NR);
	if (error != SW_OK) {
		nss_bridge_mgr_warn("TCP SYN rule deletion failed, error %d\n", error);
	}

syn_fail:
	error = fal_acl_rule_delete(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID,
				NSS_BRIDGE_MGR_ACL_FIN_RULE_ID, NSS_BRIDGE_MGR_ACL_RULE_NR);
	if (error != SW_OK) {
		nss_bridge_mgr_warn("TCP FIN rule deletion failed, error %d\n", error);
	}

fin_fail:
	error = fal_acl_rule_delete(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID,
				NSS_BRIDGE_MGR_ACL_FRAG_RULE_ID, NSS_BRIDGE_MGR_ACL_RULE_NR);
	if (error != SW_OK) {
		nss_bridge_mgr_warn("IP fragmentation rule deletion failed, error %d\n", error);
	}

frag_fail:
	error = fal_acl_list_destroy(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID);
	if (error != SW_OK) {
		nss_bridge_mgr_warn("ACL list destroy failed, error %d\n", error);
	}

	return false;
}

/*
 * nss_bridge_mgr_l2_exception_acl_disable()
 *	Destroy ACL list and rule created by the driver.
 */
static void nss_bridge_mgr_l2_exception_acl_disable(void)
{
	sw_error_t error;

	error = fal_acl_rule_delete(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID,
				NSS_BRIDGE_MGR_ACL_SYN_RULE_ID, NSS_BRIDGE_MGR_ACL_RULE_NR);
	if (error != SW_OK) {
		nss_bridge_mgr_warn("TCP SYN rule deletion failed, error %d\n", error);
	}

	error = fal_acl_rule_delete(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID,
				NSS_BRIDGE_MGR_ACL_FIN_RULE_ID, NSS_BRIDGE_MGR_ACL_RULE_NR);
	if (error != SW_OK) {
		nss_bridge_mgr_warn("TCP FIN rule deletion failed, error %d\n", error);
	}

	error = fal_acl_rule_delete(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID,
				NSS_BRIDGE_MGR_ACL_RST_RULE_ID, NSS_BRIDGE_MGR_ACL_RULE_NR);
	if (error != SW_OK) {
		nss_bridge_mgr_warn("TCP RST rule deletion failed, error %d\n", error);
	}

	error = fal_acl_rule_delete(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID,
				NSS_BRIDGE_MGR_ACL_FRAG_RULE_ID, NSS_BRIDGE_MGR_ACL_RULE_NR);
	if (error != SW_OK) {
		nss_bridge_mgr_warn("IP fragmentation rule deletion failed, error %d\n", error);
	}

	error = fal_acl_list_destroy(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID);
	if (error != SW_OK) {
		nss_bridge_mgr_warn("ACL list destroy failed, error %d\n", error);
	}
}

#endif

/*
 * nss_bridge_mgr_join_bridge()
 *	Netdevice join bridge and send netdevice joining bridge message to NSS FW.
 */
int nss_bridge_mgr_join_bridge(struct net_device *dev, struct nss_bridge_pvt *br)
{
	int32_t ifnum;

	ifnum = nss_cmn_get_interface_number_by_dev(dev);
	if (ifnum < 0) {
		nss_bridge_mgr_warn("%s: failed to find interface number\n", dev->name);
		return -EINVAL;
	}

#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
	if (NSS_BRIDGE_MGR_IF_IS_TYPE_PHYSICAL(ifnum)) {
		fal_port_t port_num = (fal_port_t)ifnum;

		/*
		 * If there is a wan interface added in bridge, create a
		 * separate VSI for it, hence avoiding FDB based forwarding.
		 * This is done by not sending join message to the bridge in NSS.
		 */
		if (br_mgr_ctx.wan_if_num == ifnum) {
			br->wan_if_enabled = true;
			br->wan_if_num = ifnum;
			nss_bridge_mgr_info("if_num %d is added as WAN interface \n", ifnum);
			return 0;
		}

		if (ppe_port_vsi_get(NSS_BRIDGE_MGR_SWITCH_ID, port_num, &br->port_vsi[port_num - 1])) {
			nss_bridge_mgr_warn("%px: failed to save port VSI of physical interface\n", br);
			return -EIO;
		}

		if (ppe_port_vsi_set(NSS_BRIDGE_MGR_SWITCH_ID, port_num, br->vsi)) {
			nss_bridge_mgr_warn("%px: failed to set bridge VSI for physical interface\n", br);
			return -EIO;
		}
	} else if (is_vlan_dev(dev)) {
		struct net_device *real_dev;

		/*
		 * Find real_dev associated with the VLAN
		 */
		real_dev = nss_vlan_mgr_get_real_dev(dev);
		if (real_dev && is_vlan_dev(real_dev))
			real_dev = nss_vlan_mgr_get_real_dev(real_dev);
		if (real_dev == NULL) {
			nss_bridge_mgr_warn("%px: real dev for the vlan: %s in NULL\n", br, dev->name);
			return -EINVAL;
		}

		/*
		 * This is a valid vlan dev, add the vlan dev to bridge
		 */
		if (nss_vlan_mgr_join_bridge(dev, br->vsi)) {
			nss_bridge_mgr_warn("%px: vlan device failed to join bridge\n", br);
			return -ENODEV;
		}

		/*
		 * dev is a bond with VLAN and VLAN is added to bridge
		 */
		if (netif_is_bond_master(real_dev)) {
			if (nss_bridge_tx_join_msg(br->ifnum, dev) != NSS_TX_SUCCESS) {
				nss_bridge_mgr_warn("%px: Interface %s join bridge failed\n", br, dev->name);
				nss_vlan_mgr_leave_bridge(dev, br->vsi);
				return -ENOENT;
			}

			/*
			 * Update FDB state of the bridge. No need to add individual interfaces of bond to the bridge.
			 * VLAN interface verifies that all interfaces are physical so, no need to verify again.
			 */
			if (nss_bridge_mgr_bond_fdb_join(br) != NOTIFY_DONE) {
				nss_bridge_mgr_warn("%px: Slaves of bond interface %s join bridge failed\n", br, real_dev->name);
				nss_bridge_tx_leave_msg(br->ifnum, dev);
				nss_vlan_mgr_leave_bridge(dev, br->vsi);
				return -EINVAL;
			}

			return 0;
		}
	}
#endif

	if (nss_bridge_tx_join_msg(br->ifnum, dev) != NSS_TX_SUCCESS) {
		nss_bridge_mgr_warn("%px: Interface %s join bridge failed\n", br, dev->name);
#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
		if (NSS_BRIDGE_MGR_IF_IS_TYPE_PHYSICAL(ifnum)) {
			fal_port_t port_num = (fal_port_t)ifnum;
			ppe_port_vsi_set(NSS_BRIDGE_MGR_SWITCH_ID, port_num, br->port_vsi[port_num - 1]);
		}
#endif
		return -EIO;
	}

	return 0;
}

/*
 * nss_bridge_mgr_leave_bridge()
 *	Netdevice leave bridge and send netdevice leaving bridge message to NSS FW.
 */
int nss_bridge_mgr_leave_bridge(struct net_device *dev, struct nss_bridge_pvt *br)
{
	int32_t ifnum;

	ifnum = nss_cmn_get_interface_number_by_dev(dev);
	if (ifnum < 0) {
		nss_bridge_mgr_warn("%s: failed to find interface number\n", dev->name);
		return -1;
	}

#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
	if (NSS_BRIDGE_MGR_IF_IS_TYPE_PHYSICAL(ifnum)) {
		fal_port_t port_num = (fal_port_t)ifnum;

		if (fal_stp_port_state_set(NSS_BRIDGE_MGR_SWITCH_ID, NSS_BRIDGE_MGR_SPANNING_TREE_ID, port_num, FAL_STP_FORWARDING)) {
			nss_bridge_mgr_warn("%px: faied to set the STP state to forwarding\n", br);
			return -1;
		}

		/*
		 * If there is a wan interface added in bridge, a separate
		 * VSI is created for it by not sending join message to NSS.
		 * Hence a leave message should also be avaoided.
		 */
		if ((br->wan_if_enabled) && (br->wan_if_num == ifnum)) {
			br->wan_if_enabled = false;
			br->wan_if_num = -1;
			nss_bridge_mgr_info("if_num %d is added as WAN interface\n", ifnum);
			return 0;
		}

		if (ppe_port_vsi_set(NSS_BRIDGE_MGR_SWITCH_ID, port_num, br->port_vsi[port_num - 1])) {
			nss_bridge_mgr_warn("%px: failed to restore port VSI of physical interface\n", br);
			fal_stp_port_state_set(NSS_BRIDGE_MGR_SWITCH_ID, NSS_BRIDGE_MGR_SPANNING_TREE_ID, port_num, FAL_STP_DISABLED);
			return -1;
		}
	} else if (is_vlan_dev(dev)) {
		struct net_device *real_dev;

		/*
		 * Find real_dev associated with the VLAN.
		 */
		real_dev = nss_vlan_mgr_get_real_dev(dev);
		if (real_dev && is_vlan_dev(real_dev))
			real_dev = nss_vlan_mgr_get_real_dev(real_dev);
		if (real_dev == NULL) {
			nss_bridge_mgr_warn("%px: real dev for the vlan: %s in NULL\n", br, dev->name);
			return -1;
		}

		/*
		 * This is a valid vlan dev, remove the vlan dev from bridge.
		 */
		if (nss_vlan_mgr_leave_bridge(dev, br->vsi)) {
			nss_bridge_mgr_warn("%px: vlan device failed to leave bridge\n", br);
			return -1;
		}

		/*
		 * dev is a bond with VLAN and VLAN is removed from bridge
		 */
		if (netif_is_bond_master(real_dev)) {
			if (nss_bridge_tx_leave_msg(br->ifnum, dev) != NSS_TX_SUCCESS) {
				nss_bridge_mgr_warn("%px: Interface %s leave bridge failed\n", br, dev->name);
				nss_vlan_mgr_join_bridge(dev, br->vsi);
				nss_bridge_tx_join_msg(br->ifnum, dev);
				return -1;
			}

			/*
			 * Update FDB state of the bridge. No need to add individual interfaces of bond to the bridge.
			 * VLAN interface verifies that all interfaces are physical so, no need to verify again.
			 */
			if (nss_bridge_mgr_bond_fdb_leave(br) != NOTIFY_DONE) {
				nss_bridge_mgr_warn("%px: Slaves of bond interface %s leave bridge failed\n", br, real_dev->name);
				nss_vlan_mgr_join_bridge(dev, br->vsi);
				nss_bridge_tx_join_msg(br->ifnum, dev);
				return -1;
			}

			return 0;
		}
	}
#endif

	if (nss_bridge_tx_leave_msg(br->ifnum, dev) != NSS_TX_SUCCESS) {
		nss_bridge_mgr_warn("%px: Interface %s leave bridge failed\n", br, dev->name);
#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
		if (is_vlan_dev(dev)) {
			nss_vlan_mgr_join_bridge(dev, br->vsi);
			nss_bridge_tx_join_msg(br->ifnum, dev);
		} else if (NSS_BRIDGE_MGR_IF_IS_TYPE_PHYSICAL(ifnum)) {
			fal_port_t port_num = (fal_port_t)ifnum;

			fal_stp_port_state_set(NSS_BRIDGE_MGR_SWITCH_ID, NSS_BRIDGE_MGR_SPANNING_TREE_ID, port_num, FAL_STP_DISABLED);
			ppe_port_vsi_set(NSS_BRIDGE_MGR_SWITCH_ID, port_num, br->vsi);
		}
#endif
		return -1;
	}

	return 0;
}

/*
 * nss_bridge_mgr_unregister_br()
 *	Unregister bridge device, dev, from bridge manager database.
 */
int nss_bridge_mgr_unregister_br(struct net_device *dev)
{
	struct nss_bridge_pvt *b_pvt;

	/*
	 * Do we have it on record?
	 */
	b_pvt = nss_bridge_mgr_find_instance(dev);
	if (!b_pvt)
		return -1;

	/*
	 * sequence of free:
	 * 1. issue VSI unassign to NSS
	 * 2. free VSI
	 * 3. flush bridge FDB table
	 * 4. unregister bridge netdevice from data plane
	 * 5. deallocate dynamic interface associated with bridge netdevice
	 * 6. free bridge netdevice
	 */
#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
	/*
	 * VSI unassign function in NSS firmware only returns
	 * CNODE_SEND_NACK in the beginning of the function when it
	 * detects that bridge VSI is not assigned for the bridge.
	 * Please refer to the function bridge_configure_vsi_unassign
	 * in NSS firmware for detailed operation.
	 */
	if (nss_bridge_tx_vsi_unassign_msg(b_pvt->ifnum, b_pvt->vsi) != NSS_TX_SUCCESS)
		nss_bridge_mgr_warn("%px: failed to unassign vsi\n", b_pvt);

	ppe_vsi_free(NSS_BRIDGE_MGR_SWITCH_ID, b_pvt->vsi);

	/*
	 * It may happen that the same VSI is allocated again,
	 * so there is a need to flush bridge FDB table.
	 */
	if (fal_fdb_entry_del_byfid(NSS_BRIDGE_MGR_SWITCH_ID, b_pvt->vsi, FAL_FDB_DEL_STATIC)) {
		nss_bridge_mgr_warn("%px: Failed to flush FDB table for vsi:%d in PPE\n", b_pvt, b_pvt->vsi);
	}
#endif

	nss_bridge_mgr_trace("%px: Bridge %s unregsitered. Freeing bridge di %d\n", b_pvt, dev->name, b_pvt->ifnum);

	nss_bridge_unregister(b_pvt->ifnum);

	if (nss_dynamic_interface_dealloc_node(b_pvt->ifnum, NSS_DYNAMIC_INTERFACE_TYPE_BRIDGE) != NSS_TX_SUCCESS) {
		nss_bridge_mgr_warn("%px: dealloc bridge di failed\n", b_pvt);
	}

	nss_bridge_mgr_delete_instance(b_pvt);
	return 0;
}

/*
 * nss_bridge_mgr_register_br()
 *	Register new bridge instance in bridge manager database.
 */
int nss_bridge_mgr_register_br(struct net_device *dev)
{
	struct nss_bridge_pvt *b_pvt;
	int ifnum;
	int err;
#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
	uint32_t vsi_id = 0;
#endif

	nss_bridge_mgr_info("%px: Bridge register: %s\n", dev, dev->name);

	b_pvt = nss_bridge_mgr_create_instance(dev);
	if (!b_pvt)
		return -EINVAL;

	b_pvt->dev = dev;

#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
	err = ppe_vsi_alloc(NSS_BRIDGE_MGR_SWITCH_ID, &vsi_id);
	if (err) {
		nss_bridge_mgr_warn("%px: failed to alloc bridge vsi, error = %d\n", b_pvt, err);
		goto fail;
	}

	b_pvt->vsi = vsi_id;
#endif

	ifnum = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_BRIDGE);
	if (ifnum < 0) {
		nss_bridge_mgr_warn("%px: failed to alloc bridge di\n", b_pvt);
		goto fail_1;
	}

	if (!nss_bridge_register(ifnum, dev, NULL, NULL, 0, b_pvt)) {
		nss_bridge_mgr_warn("%px: failed to register bridge di to NSS\n", b_pvt);
		goto fail_2;
	}

#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
	err = nss_bridge_tx_vsi_assign_msg(ifnum, vsi_id);
	if (err != NSS_TX_SUCCESS) {
		nss_bridge_mgr_warn("%px: failed to assign vsi msg, error = %d\n", b_pvt, err);
		goto fail_3;
	}
#endif

	err = nss_bridge_tx_set_mac_addr_msg(ifnum, (uint8_t *) dev->dev_addr);
	if (err != NSS_TX_SUCCESS) {
		nss_bridge_mgr_warn("%px: failed to set mac_addr msg, error = %d\n", b_pvt, err);
		goto fail_4;
	}

	err = nss_bridge_tx_set_mtu_msg(ifnum, dev->mtu);
	if (err != NSS_TX_SUCCESS) {
		nss_bridge_mgr_warn("%px: failed to set mtu msg, error = %d\n", b_pvt, err);
		goto fail_4;
	}

	/*
	 * All done, take a snapshot of the current mtu and mac addrees
	 */
	b_pvt->ifnum = ifnum;
	b_pvt->mtu = dev->mtu;
#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
	b_pvt->wan_if_num = -1;
	b_pvt->wan_if_enabled = false;
#endif
	ether_addr_copy(b_pvt->dev_addr, dev->dev_addr);
	spin_lock(&br_mgr_ctx.lock);
	list_add(&b_pvt->list, &br_mgr_ctx.list);
	spin_unlock(&br_mgr_ctx.lock);

#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
	/*
	 * Disable FDB learning if OVS is enabled for
	 * all bridges (including Linux bridge).
	 */
	if (ovs_enabled || fdb_disabled) {
		nss_bridge_mgr_disable_fdb_learning(b_pvt);
	}
#endif
	return 0;

fail_4:
#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
	if (nss_bridge_tx_vsi_unassign_msg(ifnum, vsi_id) != NSS_TX_SUCCESS) {
		nss_bridge_mgr_warn("%px: failed to unassign vsi\n", b_pvt);
	}
fail_3:
#endif

	nss_bridge_unregister(ifnum);

fail_2:
	if (nss_dynamic_interface_dealloc_node(ifnum, NSS_DYNAMIC_INTERFACE_TYPE_BRIDGE) != NSS_TX_SUCCESS) {
		nss_bridge_mgr_warn("%px: failed to dealloc bridge di\n", b_pvt);
	}

fail_1:
#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
	ppe_vsi_free(NSS_BRIDGE_MGR_SWITCH_ID, vsi_id);
fail:
#endif

	nss_bridge_mgr_delete_instance(b_pvt);

	return -EFAULT;
}

/*
 * nss_bridge_mgr_bond_slave_changeupper()
 *	Add bond slave to bridge VSI
 */
static int nss_bridge_mgr_bond_slave_changeupper(struct netdev_notifier_changeupper_info *cu_info,
		struct net_device *bond_slave)
{
	struct net_device *master;
	struct nss_bridge_pvt *b_pvt;

	/*
	 * Checking if our bond master is part of a bridge
	 */
	master = netdev_master_upper_dev_get(cu_info->upper_dev);
	if (!master)
		return NOTIFY_DONE;

	b_pvt = nss_bridge_mgr_find_instance(master);
	if (!b_pvt) {
		nss_bridge_mgr_warn("The bond master is not part of Bridge dev:%s\n", master->name);
		return NOTIFY_DONE;
	}

#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
	/*
	 * Add or remove the slave based based on linking event
	 */
	if (cu_info->linking) {
		if (nss_bridge_mgr_add_bond_slave(cu_info->upper_dev, bond_slave, b_pvt)) {
			nss_bridge_mgr_warn("%px: Failed to add slave (%s) state in Bridge %s\n", b_pvt,
					cu_info->upper_dev->name, master->name);
		}
	} else {
		if (nss_bridge_mgr_del_bond_slave(cu_info->upper_dev, bond_slave, b_pvt)) {
			nss_bridge_mgr_warn("%px: Failed to remove slave (%s) state in Bridge %s\n", b_pvt,
					cu_info->upper_dev->name, master->name);
		}
	}
#endif

	return NOTIFY_DONE;
}

/*
 * nss_bridge_mgr_changemtu_event()
 *	Change bridge MTU and send change bridge MTU message to NSS FW.
 */
static int nss_bridge_mgr_changemtu_event(struct netdev_notifier_info *info)
{
	struct net_device *dev = netdev_notifier_info_to_dev(info);
	struct nss_bridge_pvt *b_pvt = nss_bridge_mgr_find_instance(dev);

	if (!b_pvt)
		return NOTIFY_DONE;

	spin_lock(&br_mgr_ctx.lock);
	if (b_pvt->mtu == dev->mtu) {
		spin_unlock(&br_mgr_ctx.lock);
		return NOTIFY_DONE;
	}
	spin_unlock(&br_mgr_ctx.lock);

	nss_bridge_mgr_trace("%px: MTU changed to %d, send message to NSS\n", b_pvt, dev->mtu);

	if (nss_bridge_tx_set_mtu_msg(b_pvt->ifnum, dev->mtu) != NSS_TX_SUCCESS) {
		nss_bridge_mgr_warn("%px: Failed to send change MTU message to NSS\n", b_pvt);
		return NOTIFY_DONE;
	}

	spin_lock(&br_mgr_ctx.lock);
	b_pvt->mtu = dev->mtu;
	spin_unlock(&br_mgr_ctx.lock);

	return NOTIFY_DONE;
}

/*
 * nss_bridge_mgr_changeaddr_event()
 *	Change bridge MAC address and send change bridge address message to NSS FW.
 */
static int nss_bridge_mgr_changeaddr_event(struct netdev_notifier_info *info)
{
	struct net_device *dev = netdev_notifier_info_to_dev(info);
	struct nss_bridge_pvt *b_pvt = nss_bridge_mgr_find_instance(dev);

	if (!b_pvt)
		return NOTIFY_DONE;

	spin_lock(&br_mgr_ctx.lock);
	if (!memcmp(b_pvt->dev_addr, dev->dev_addr, ETH_ALEN)) {
		spin_unlock(&br_mgr_ctx.lock);
		nss_bridge_mgr_trace("%px: MAC are the same..skip processing it\n", b_pvt);
		return NOTIFY_DONE;
	}
	spin_unlock(&br_mgr_ctx.lock);

	nss_bridge_mgr_trace("%px: MAC changed to %pM, update NSS\n", b_pvt, dev->dev_addr);

	if (nss_bridge_tx_set_mac_addr_msg(b_pvt->ifnum, (uint8_t *) dev->dev_addr) != NSS_TX_SUCCESS) {
		nss_bridge_mgr_warn("%px: Failed to send change MAC address message to NSS\n", b_pvt);
		return NOTIFY_DONE;
	}

	spin_lock(&br_mgr_ctx.lock);
	ether_addr_copy(b_pvt->dev_addr, dev->dev_addr);
	spin_unlock(&br_mgr_ctx.lock);

	return NOTIFY_DONE;
}

/*
 * nss_bridge_mgr_changeupper_event()
 *	Bridge manager handles netdevice joining or leaving bridge notification.
 */
static int nss_bridge_mgr_changeupper_event(struct netdev_notifier_info *info)
{
	struct net_device *dev = netdev_notifier_info_to_dev(info);
	struct net_device *master_dev;
	struct netdev_notifier_changeupper_info *cu_info;
	struct nss_bridge_pvt *b_pvt;

	cu_info = (struct netdev_notifier_changeupper_info *)info;

	/*
	 * Check if the master pointer is valid
	 */
	if (!cu_info->master)
		return NOTIFY_DONE;

	/*
	 * The master is a bond that we don't need to process, but the bond might be part of a bridge.
	 */
	if (netif_is_bond_slave(dev))
		return nss_bridge_mgr_bond_slave_changeupper(cu_info, dev);

	master_dev = cu_info->upper_dev;

	/*
	 * Check if upper_dev is a known bridge.
	 */
	b_pvt = nss_bridge_mgr_find_instance(master_dev);
	if (!b_pvt)
		return NOTIFY_DONE;

	/*
	 * Slave device is bond master and it is added/removed to/from bridge
	 */
	if (netif_is_bond_master(dev)) {
#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
		if (cu_info->linking)
			return nss_bridge_mgr_bond_master_join(dev, b_pvt);
		else
			return nss_bridge_mgr_bond_master_leave(dev, b_pvt);
#endif
	}

	if (cu_info->linking) {
		nss_bridge_mgr_trace("%px: Interface %s joining bridge %s\n", b_pvt, dev->name, master_dev->name);
		if (nss_bridge_mgr_join_bridge(dev, b_pvt)) {
			nss_bridge_mgr_warn("%px: Interface %s failed to join bridge %s\n", b_pvt, dev->name, master_dev->name);
		}

		return NOTIFY_DONE;
	}

	nss_bridge_mgr_trace("%px: Interface %s leaving bridge %s\n", b_pvt, dev->name, master_dev->name);
	if (nss_bridge_mgr_leave_bridge(dev, b_pvt)) {
		nss_bridge_mgr_warn("%px: Interface %s failed to leave bridge %s\n", b_pvt, dev->name, master_dev->name);
	}

	return NOTIFY_DONE;
}

/*
 * nss_bridge_mgr_register_event()
 *	Bridge manager handles bridge registration notification.
 */
static int nss_bridge_mgr_register_event(struct netdev_notifier_info *info)
{
	nss_bridge_mgr_register_br(netdev_notifier_info_to_dev(info));
	return NOTIFY_DONE;
}

/*
 * nss_bridge_mgr_unregister_event()
 *	Bridge manager handles bridge unregistration notification.
 */
static int nss_bridge_mgr_unregister_event(struct netdev_notifier_info *info)
{
	nss_bridge_mgr_unregister_br(netdev_notifier_info_to_dev(info));
	return NOTIFY_DONE;
}

/*
 * nss_bridge_mgr_netdevice_event()
 *	Bridge manager handles bridge operation notifications.
 */
static int nss_bridge_mgr_netdevice_event(struct notifier_block *unused,
				unsigned long event, void *ptr)
{
	struct netdev_notifier_info *info = (struct netdev_notifier_info *)ptr;

	switch (event) {
	case NETDEV_CHANGEUPPER:
		return nss_bridge_mgr_changeupper_event(info);
	case NETDEV_CHANGEADDR:
		return nss_bridge_mgr_changeaddr_event(info);
	case NETDEV_CHANGEMTU:
		return nss_bridge_mgr_changemtu_event(info);
	case NETDEV_REGISTER:
		return nss_bridge_mgr_register_event(info);
	case NETDEV_UNREGISTER:
		return nss_bridge_mgr_unregister_event(info);
	}

	/*
	 * Notify done for all the events we don't care
	 */
	return NOTIFY_DONE;
}

static struct notifier_block nss_bridge_mgr_netdevice_nb __read_mostly = {
	.notifier_call = nss_bridge_mgr_netdevice_event,
};
#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)

/*
 * nss_bridge_mgr_is_physical_dev()
 *	Check if the device is on physical device.
 */
static bool nss_bridge_mgr_is_physical_dev(struct net_device *dev)
{
	struct net_device *root_dev = dev;
	uint32_t ifnum;

	if (!dev)
		return false;

	/*
	 * Check if it is VLAN first because VLAN can be over bond interface.
	 * However, the bond over VLAN is not supported in our driver.
	 */
	if (is_vlan_dev(dev)) {
		root_dev = nss_vlan_mgr_get_real_dev(dev);
		if (!root_dev)
			goto error;

		if (is_vlan_dev(root_dev))
			root_dev = nss_vlan_mgr_get_real_dev(root_dev);

		if (!root_dev)
			goto error;
	}

	/*
	 * Don't consider bond interface because FDB learning is disabled.
	 */
	if (netif_is_bond_master(root_dev))
		return false;

	ifnum = nss_cmn_get_interface_number_by_dev(root_dev);
	if (!NSS_BRIDGE_MGR_IF_IS_TYPE_PHYSICAL(ifnum)) {
		nss_bridge_mgr_warn("%px: interface %s is not physical interface\n",
				root_dev, root_dev->name);
		return false;
	}

	return true;

error:
	nss_bridge_mgr_warn("%px: cannot find the real device for VLAN %s\n", dev, dev->name);
	return false;
}

/*
 * nss_bridge_mgr_fdb_update_callback()
 *	Get invoked when there is a FDB update.
 */
static int nss_bridge_mgr_fdb_update_callback(struct notifier_block *notifier,
					      unsigned long val, void *ctx)
{
	struct br_fdb_event *event = (struct br_fdb_event *)ctx;
	struct nss_bridge_pvt *b_pvt = NULL;
	struct net_device *br_dev = NULL;
	fal_fdb_entry_t entry;

	if (!event->br)
		return NOTIFY_DONE;

	br_dev = br_fdb_bridge_dev_get_and_hold(event->br);
	if (!br_dev) {
		nss_bridge_mgr_warn("%px: bridge device not found\n", event->br);
		return NOTIFY_DONE;
	}

	nss_bridge_mgr_trace("%px: MAC: %pM, original source: %s, new source: %s, bridge: %s\n",
			event, event->addr, event->orig_dev->name, event->dev->name, br_dev->name);

	/*
	 * When a MAC address move from a physical interface to a non-physical
	 * interface, the FDB entry in the PPE needs to be flushed.
	 */
	if (!nss_bridge_mgr_is_physical_dev(event->orig_dev)) {
		nss_bridge_mgr_trace("%px: original source is not a physical interface\n", event->orig_dev);
		dev_put(br_dev);
		return NOTIFY_DONE;
	}

	if (nss_bridge_mgr_is_physical_dev(event->dev)) {
		nss_bridge_mgr_trace("%px: new source is not a non-physical interface\n", event->dev);
		dev_put(br_dev);
		return NOTIFY_DONE;
	}

	b_pvt = nss_bridge_mgr_find_instance(br_dev);
	dev_put(br_dev);
	if (!b_pvt) {
		nss_bridge_mgr_warn("%px: bridge instance not found\n", event->br);
		return NOTIFY_DONE;
	}

	memset(&entry, 0, sizeof(entry));
	memcpy(&entry.addr, event->addr, ETH_ALEN);
	entry.fid = b_pvt->vsi;
	if (SW_OK != fal_fdb_entry_del_bymac(NSS_BRIDGE_MGR_SWITCH_ID, &entry)) {
		nss_bridge_mgr_warn("%px: FDB entry delete failed with MAC %pM and fid %d\n",
				    b_pvt, &entry.addr, entry.fid);
		return NOTIFY_DONE;
	}
	return NOTIFY_OK;
}

/*
 * Notifier block for FDB update
 */
static struct notifier_block nss_bridge_mgr_fdb_update_notifier = {
	.notifier_call = nss_bridge_mgr_fdb_update_callback,
};
/*
 * nss_bridge_mgr_wan_inf_add_handler
 *	Marks an interface as a WAN interface for special handling by bridge.
 */
static int nss_bridge_mgr_wan_intf_add_handler(struct ctl_table *table,
						int write, void __user *buffer,
						size_t *lenp, loff_t *ppos)
{
	struct net_device *dev;
	char *dev_name;
	char *if_name;
	int32_t if_num;
	int ret;

	/*
	 * Find the string, return an error if not found
	 */
	ret = proc_dostring(table, write, buffer, lenp, ppos);
	if (ret || !write) {
		return ret;
	}

	if_name = br_mgr_ctx.wan_ifname;
	dev_name = strsep(&if_name, " ");
	dev = dev_get_by_name(&init_net, dev_name);
	if (!dev) {
		nss_bridge_mgr_warn("Cannot find the net device associated with %s\n", dev_name);
		return -ENODEV;
	}

	if_num = nss_cmn_get_interface_number_by_dev(dev);
	if (!NSS_BRIDGE_MGR_IF_IS_TYPE_PHYSICAL(if_num)) {
		dev_put(dev);
		nss_bridge_mgr_warn("Only physical interfaces can be marked as WAN interface: if_num %d\n", if_num);
		return -ENOMSG;
	}

	if (br_mgr_ctx.wan_if_num != -1) {
		dev_put(dev);
		nss_bridge_mgr_warn("Cannot overwrite a pre-existing wan interface\n");
		return -ENOMSG;
	}

	br_mgr_ctx.wan_if_num = if_num;
	dev_put(dev);
	nss_bridge_mgr_always("For adding if_num: %d as WAN interface, do a network restart\n", if_num);
	return ret;
}

/*
 * nss_bridge_mgr_wan_inf_del_handler
 *	Un-marks an interface as a WAN interface.
 */
static int nss_bridge_mgr_wan_intf_del_handler(struct ctl_table *table,
						int write, void __user *buffer,
						size_t *lenp, loff_t *ppos)
{
	struct net_device *dev;
	char *dev_name;
	char *if_name;
	int32_t if_num;
	int ret;

	ret = proc_dostring(table, write, buffer, lenp, ppos);
	if (ret)
		return ret;

	if (!write)
		return ret;

	if_name = br_mgr_ctx.wan_ifname;
	dev_name = strsep(&if_name, " ");
	dev = dev_get_by_name(&init_net, dev_name);
	if (!dev) {
		nss_bridge_mgr_warn("Cannot find the net device associated with %s\n", dev_name);
		return -ENODEV;
	}

	if_num = nss_cmn_get_interface_number_by_dev(dev);
	if (!NSS_BRIDGE_MGR_IF_IS_TYPE_PHYSICAL(if_num)) {
		dev_put(dev);
		nss_bridge_mgr_warn("Only physical interfaces can be marked/unmarked, if_num: %d\n", if_num);
		return -ENOMSG;
	}

	if (br_mgr_ctx.wan_if_num != if_num) {
		dev_put(dev);
		nss_bridge_mgr_warn("This interface is not marked as a WAN interface\n");
		return -ENOMSG;
	}

	br_mgr_ctx.wan_if_num = -1;
	dev_put(dev);
	nss_bridge_mgr_always("For deleting if_num: %d as WAN interface, do a network restart\n", if_num);
	return ret;
}

static struct ctl_table nss_bridge_mgr_table[] = {
	{
		.procname	= "add_wanif",
		.data           = &br_mgr_ctx.wan_ifname,
		.maxlen         = sizeof(char) * IFNAMSIZ,
		.mode           = 0644,
		.proc_handler   = &nss_bridge_mgr_wan_intf_add_handler,
	},
	{
		.procname	= "del_wanif",
		.data           = &br_mgr_ctx.wan_ifname,
		.maxlen         = sizeof(char) * IFNAMSIZ,
		.mode           = 0644,
		.proc_handler   = &nss_bridge_mgr_wan_intf_del_handler,
	},
	{ }
};
#endif
/*
 * nss_bridge_mgr_init_module()
 *	bridge_mgr module init function
 */
int __init nss_bridge_mgr_init_module(void)
{
	/*
	 * Monitor bridge activity only on supported platform
	 */
	if (!of_machine_is_compatible("qcom,ipq807x") && !of_machine_is_compatible("qcom,ipq6018") && !of_machine_is_compatible("qcom,ipq8074"))
		return 0;

	INIT_LIST_HEAD(&br_mgr_ctx.list);
	spin_lock_init(&br_mgr_ctx.lock);
	register_netdevice_notifier(&nss_bridge_mgr_netdevice_nb);
	nss_bridge_mgr_info("Module (Build %s) loaded\n", NSS_CLIENT_BUILD_ID);
#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
	br_mgr_ctx.wan_if_num = -1;
	br_fdb_update_register_notify(&nss_bridge_mgr_fdb_update_notifier);
	br_mgr_ctx.nss_bridge_mgr_header = register_sysctl("nss/bridge_mgr", nss_bridge_mgr_table);

	/*
	 * Enable ACL rule to enable L2 exception. This is needed if PPE Virtual ports is added to bridge.
	 *  It is assumed that VP is using flow based bridging, hence L2 exceptions will need to be enabled on PPE bridge.
	 */
	if (!nss_bridge_mgr_l2_exception_acl_enable()) {
		nss_bridge_mgr_warn("Failed to enable ACL\n");
	}
#endif
#if defined (NSS_BRIDGE_MGR_OVS_ENABLE)
	nss_bridge_mgr_ovs_init();
#endif
	return 0;
}

/*
 * nss_bridge_mgr_exit_module()
 *	bridge_mgr module exit function
 */
void __exit nss_bridge_mgr_exit_module(void)
{
	unregister_netdevice_notifier(&nss_bridge_mgr_netdevice_nb);
	nss_bridge_mgr_info("Module unloaded\n");
#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
	br_fdb_update_unregister_notify(&nss_bridge_mgr_fdb_update_notifier);

	if (br_mgr_ctx.nss_bridge_mgr_header) {
		unregister_sysctl_table(br_mgr_ctx.nss_bridge_mgr_header);
	}

	/*
	 * Disable the PPE L2 exceptions which were enabled during module init for PPE virtual ports.
	 */
	nss_bridge_mgr_l2_exception_acl_disable();

#endif
#if defined (NSS_BRIDGE_MGR_OVS_ENABLE)
	nss_bridge_mgr_ovs_exit();
#endif
}

module_init(nss_bridge_mgr_init_module);
module_exit(nss_bridge_mgr_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS bridge manager");

module_param(ovs_enabled, bool, 0644);
MODULE_PARM_DESC(ovs_enabled, "OVS bridge is enabled");

module_param(fdb_disabled, bool, 0644);
MODULE_PARM_DESC(fdb_disabled, "fdb learning is disabled");
