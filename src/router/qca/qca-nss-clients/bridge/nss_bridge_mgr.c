/*
 **************************************************************************
 * Copyright (c) 2016-2019, The Linux Foundation. All rights reserved.
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
 * nss_bridge_mgr.c
 *	NSS to HLOS Bridge Interface manager
 */
#include <linux/sysctl.h>
#include <linux/etherdevice.h>
#include <linux/if_vlan.h>
#include <linux/of.h>
#include <linux/if_bridge.h>
#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
#include <ref/ref_vsi.h>
#include <nss_vlan_mgr.h>
#include <fal/fal_fdb.h>
#include <fal/fal_stp.h>
#include <fal/fal_acl.h>
#include <fal/fal_api.h>
#include <fal/fal_port_ctrl.h>
#endif
#include <nss_api_if.h>

#if (NSS_BRIDGE_MGR_DEBUG_LEVEL < 1)
#define nss_bridge_mgr_assert(fmt, args...)
#else
#define nss_bridge_mgr_assert(c) BUG_ON(!(c))
#endif /* NSS_BRIDGE_MGR_DEBUG_LEVEL */

/*
 * Compile messages for dynamic enable/disable
 */
#if defined(CONFIG_DYNAMIC_DEBUG)
#define nss_bridge_mgr_warn(s, ...) \
		pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_bridge_mgr_info(s, ...) \
		pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_bridge_mgr_trace(s, ...) \
		pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#else /* CONFIG_DYNAMIC_DEBUG */
/*
 * Statically compile messages at different levels
 */
#if (NSS_BRIDGE_MGR_DEBUG_LEVEL < 2)
#define nss_bridge_mgr_warn(s, ...)
#else
#define nss_bridge_mgr_warn(s, ...) \
		pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_BRIDGE_MGR_DEBUG_LEVEL < 3)
#define nss_bridge_mgr_info(s, ...)
#else
#define nss_bridge_mgr_info(s, ...) \
		pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_BRIDGE_MGR_DEBUG_LEVEL < 4)
#define nss_bridge_mgr_trace(s, ...)
#else
#define nss_bridge_mgr_trace(s, ...) \
		pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif
#endif /* CONFIG_DYNAMIC_DEBUG */

/*
 * nss interface check
 */
#define NSS_BRIDGE_MGR_PHY_PORT_MIN 1
#define NSS_BRIDGE_MGR_PHY_PORT_MAX 6
#define NSS_BRIDGE_MGR_IF_IS_TYPE_PHYSICAL(if_num) \
	(((if_num) >= NSS_BRIDGE_MGR_PHY_PORT_MIN) && \
	((if_num) <= NSS_BRIDGE_MGR_PHY_PORT_MAX))

#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
#define NSS_BRIDGE_MGR_SWITCH_ID	0
#define NSS_BRIDGE_MGR_SPANNING_TREE_ID	0
#define NSS_BRIDGE_MGR_DISABLE_PPE_EXCEPTION	0
#define NSS_BRIDGE_MGR_ENABLE_PPE_EXCEPTION	1

#define NSS_BRIDGE_MGR_ACL_DEV_ID 0
#define NSS_BRIDGE_MGR_ACL_LIST_ID 61
#define NSS_BRIDGE_MGR_ACL_LIST_PRIORITY 0
#define NSS_BRIDGE_MGR_ACL_RULE_NR 1
#define NSS_BRIDGE_MGR_ACL_FRAG_RULE_ID 0
#define NSS_BRIDGE_MGR_ACL_FIN_RULE_ID 1
#define NSS_BRIDGE_MGR_ACL_SYN_RULE_ID 2
#define NSS_BRIDGE_MGR_ACL_RST_RULE_ID 3

#endif

/*
 * bridge manager context structure
 */
struct nss_bridge_mgr_context {
	struct list_head list;		/* List of bridge instance */
	spinlock_t lock;		/* Lock to protect bridge instance */
#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
	int32_t wan_if_num;		/* WAN interface number */
	char wan_ifname[IFNAMSIZ];	/* WAN interface name */
	struct ctl_table_header *nss_bridge_mgr_header;	/* bridge sysctl */
#endif
} br_mgr_ctx;

/*
 * bridge manager private structure
 */
struct nss_bridge_pvt {
	struct list_head list;			/* List of bridge instance */
	struct net_device *dev;			/* Bridge netdevice */
	uint32_t ifnum;				/* Dynamic interface for bridge */
#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
	uint32_t vsi;				/* VSI set for bridge */
	uint32_t port_vsi[NSS_BRIDGE_MGR_PHY_PORT_MAX];	/* port VSI set for physical interfaces	*/
	uint32_t lag_ports[NSS_BRIDGE_MGR_PHY_PORT_MAX]; 	/* List of slave ports in LAG */
	int bond_slave_num;			/* Total number of bond devices added into
						   bridge device */
	bool wan_if_enabled;			/* Is WAN interface enabled? */
	int32_t wan_if_num;			/* WAN interface number, if enabled */
#endif
	uint32_t mtu;				/* MTU for bridge */
	uint8_t dev_addr[ETH_ALEN];		/* MAC address for bridge */
};

/*
 * nss_bridge_mgr_create_instance()
 *	Create a bridge instance.
 */
static struct nss_bridge_pvt *nss_bridge_mgr_create_instance(struct net_device *dev)
{
	struct nss_bridge_pvt *br;

	if (!netif_is_bridge_master(dev))
		return NULL;

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
	br->dev = NULL;
	if (!list_empty(&br->list))
		list_del(&br->list);

	spin_unlock(&br_mgr_ctx.lock);

	kfree(br);
}

/*
 * nss_bridge_mgr_find_instance()
 *	Find a bridge instance from bridge list.
 */
static struct nss_bridge_pvt *nss_bridge_mgr_find_instance(
						struct net_device *dev)
{
	struct nss_bridge_pvt *br;

	if (!netif_is_bridge_master(dev))
		return NULL;

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
		nss_bridge_mgr_warn("%p: Failed to enable station move for Bridge vsi\n", br);
		return -1;
	}

	/*
	 * Enable FDB learning in PPE
	 */
	newaddr_lrn.lrn_en = 1;
	newaddr_lrn.action = FAL_MAC_FRWRD;
	if (fal_vsi_newaddr_lrn_set(NSS_BRIDGE_MGR_SWITCH_ID, br->vsi, &newaddr_lrn)) {
		nss_bridge_mgr_warn("%p: Failed to enable FDB learning for Bridge vsi\n", br);
		goto disable_sta_move;
	}

	/*
	 * Send a notification to NSS for FDB learning enable.
	 */
	if (nss_bridge_tx_set_fdb_learn_msg(br->ifnum, NSS_BRIDGE_FDB_LEARN_ENABLE) != NSS_TX_SUCCESS) {
		nss_bridge_mgr_warn("%p: Tx message failed for FDB learning status\n", br);
		goto disable_fdb_learning;
	}

	return 0;

disable_fdb_learning:
	newaddr_lrn.lrn_en = 0;
	newaddr_lrn.action = FAL_MAC_FRWRD;
	if (fal_vsi_newaddr_lrn_set(NSS_BRIDGE_MGR_SWITCH_ID, br->vsi, &newaddr_lrn))
		nss_bridge_mgr_warn("%p: Failed to disable FDB learning for Bridge vsi\n", br);

disable_sta_move:
	sta_move.stamove_en = 0;
	sta_move.action = FAL_MAC_FRWRD;
	if (fal_vsi_stamove_set(NSS_BRIDGE_MGR_SWITCH_ID, br->vsi, &sta_move))
		nss_bridge_mgr_warn("%p: Failed to disable station move for Bridge vsi\n", br);

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
		nss_bridge_mgr_warn("%p: Failed to disable station move for Bridge vsi\n", br);
		return -1;
	}

	/*
	 * Disable FDB learning in PPE
	 */
	newaddr_lrn.lrn_en = 0;
	newaddr_lrn.action = FAL_MAC_FRWRD;
	if (fal_vsi_newaddr_lrn_set(NSS_BRIDGE_MGR_SWITCH_ID, br->vsi, &newaddr_lrn)) {
		nss_bridge_mgr_warn("%p: Failed to disable FDB learning for Bridge vsi\n", br);
		goto enable_sta_move;
	}

	/*
	 * Flush FDB table for the bridge vsi
	 */
	if (fal_fdb_entry_del_byfid(NSS_BRIDGE_MGR_SWITCH_ID, br->vsi, FAL_FDB_DEL_STATIC)) {
		nss_bridge_mgr_warn("%p: Failed to flush FDB table for vsi:%d in PPE\n", br, br->vsi);
		goto enable_fdb_learning;
	}

	/*
	 * Send a notification to NSS for FDB learning disable.
	 */
	if (nss_bridge_tx_set_fdb_learn_msg(br->ifnum, NSS_BRIDGE_FDB_LEARN_DISABLE) != NSS_TX_SUCCESS) {
		nss_bridge_mgr_warn("%p: Tx message failed for FDB learning status\n", br);
		goto enable_fdb_learning;
	}

	return 0;

enable_fdb_learning:
	newaddr_lrn.lrn_en = 1;
	newaddr_lrn.action = FAL_MAC_FRWRD;
	if (fal_vsi_newaddr_lrn_set(NSS_BRIDGE_MGR_SWITCH_ID, br->vsi, &newaddr_lrn))
		nss_bridge_mgr_warn("%p: Failed to enable FDB learning for Bridge vsi\n", br);

enable_sta_move:
	sta_move.stamove_en = 1;
	sta_move.action = FAL_MAC_FRWRD;
	if (fal_vsi_stamove_set(NSS_BRIDGE_MGR_SWITCH_ID, br->vsi, &sta_move))
		nss_bridge_mgr_warn("%p: Failed to enable station move for Bridge vsi\n", br);

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
#if IS_ENABLED(CONFIG_BONDING)
	bondid = bond_get_id(bond_master);
#endif
	if (bondid < 0) {
		nss_bridge_mgr_warn("%p: Invalid LAG group id 0x%x\n",
				b_pvt, bondid);
		return -1;
	}

	lagid = bondid + NSS_LAG0_INTERFACE_NUM;

	nss_bridge_mgr_trace("%p: Bond Slave %s is added bridge\n",
			b_pvt, slave->name);

	ifnum = nss_cmn_get_interface_number_by_dev(slave);

	/*
	 * Hardware supports only PHYSICAL Ports as trunk ports
	 */
	if (!NSS_BRIDGE_MGR_IF_IS_TYPE_PHYSICAL(ifnum)) {
		nss_bridge_mgr_warn("%p: Interface %s is not Physical Interface\n",
				b_pvt, slave->name);
		return -1;
	}

	nss_bridge_mgr_trace("%p: Interface %s adding into bridge\n",
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
		nss_bridge_mgr_warn("%p: Couldn't get VSI for port %d\n",
				b_pvt, port_id);
		return -1;
	}

	if (ppe_port_vsi_set(NSS_BRIDGE_MGR_SWITCH_ID, port_id, b_pvt->vsi)) {
		spin_unlock(&br_mgr_ctx.lock);
		nss_bridge_mgr_warn("%p: Couldn't set bridge VSI for port %d\n",
				b_pvt, port_id);
		return -1;
	}
	spin_unlock(&br_mgr_ctx.lock);

	if (nss_bridge_tx_join_msg(b_pvt->ifnum,
				slave) != NSS_TX_SUCCESS) {
		if (ppe_port_vsi_set(NSS_BRIDGE_MGR_SWITCH_ID, port_id, *port_vsi))
			nss_bridge_mgr_warn("%p: Couldn't set bridge VSI for port %d\n", b_pvt, port_id);
		nss_bridge_mgr_warn("%p: Couldn't add port %d in bridge",
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
#if IS_ENABLED(CONFIG_BONDING)
	bondid = bond_get_id(bond_master);
#endif
	if (bondid < 0) {
		nss_bridge_mgr_warn("%p: Invalid LAG group id 0x%x\n",
				b_pvt, bondid);
		return -1;
	}

	lagid = bondid + NSS_LAG0_INTERFACE_NUM;

	nss_bridge_mgr_trace("%p: Bond Slave %s leaving bridge\n",
			b_pvt, slave->name);

	ifnum = nss_cmn_get_interface_number_by_dev(slave);

	/*
	 * Hardware supports only PHYSICAL Ports as trunk ports
	 */
	if (!NSS_BRIDGE_MGR_IF_IS_TYPE_PHYSICAL(ifnum)) {
		nss_bridge_mgr_warn("%p: Interface %s is not Physical Interface\n",
				b_pvt, slave->name);
		return -1;
	}

	nss_bridge_mgr_trace("%p: Interface %s leaving from bridge\n",
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
		nss_bridge_mgr_warn("%p: failed to restore port VSI for port %d\n", b_pvt, port_id);
		return -1;
	}
	spin_unlock(&br_mgr_ctx.lock);

	if (nss_bridge_tx_leave_msg(b_pvt->ifnum,
				slave) != NSS_TX_SUCCESS) {
		ppe_port_vsi_set(NSS_BRIDGE_MGR_SWITCH_ID, port_id, b_pvt->vsi);
		nss_bridge_mgr_trace("%p: Failed to remove port %d from bridge\n",
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
 * nss_bridge_mgr_bond_master_join()
 *	Add a bond interface to bridge
 */
static int nss_bridge_mgr_bond_master_join(struct net_device *bond_master,
		struct nss_bridge_pvt *b_pvt)
{
	struct net_device *slave;

	/*
	 * Join each of the bonded slaves to the VSI group
	 */
	rcu_read_lock();
	for_each_netdev_in_bond_rcu(bond_master, slave) {
		if (nss_bridge_mgr_add_bond_slave(bond_master, slave, b_pvt)) {
			rcu_read_unlock();
			nss_bridge_mgr_warn("%p: Failed to add slave (%s) state in Bridge\n", b_pvt, slave->name);
			goto cleanup;
		}
	}
	rcu_read_unlock();

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
	spin_unlock(&br_mgr_ctx.lock);

	/*
	 * This is the first bond device being attached to bridge. In order to enforce Linux
	 * bond slave selection in bridge flows involving bond interfaces, we need to disable
	 * fdb learning on this bridge master to allow flow based bridging.
	 */
	if (!nss_bridge_mgr_disable_fdb_learning(b_pvt)) {
		spin_lock(&br_mgr_ctx.lock);
		b_pvt->bond_slave_num = 1;
		spin_unlock(&br_mgr_ctx.lock);

		return NOTIFY_DONE;
	}

cleanup:
	rcu_read_lock();
	for_each_netdev_in_bond_rcu(bond_master, slave) {
		if (nss_bridge_mgr_del_bond_slave(bond_master, slave, b_pvt)) {
			nss_bridge_mgr_warn("%p: Failed to remove slave (%s) from Bridge\n", b_pvt, slave->name);
		}
	}
	rcu_read_unlock();

	return NOTIFY_BAD;
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

	/*
	 * Remove each of the bonded slaves from the VSI group
	 */
	rcu_read_lock();
	for_each_netdev_in_bond_rcu(bond_master, slave) {
		if (nss_bridge_mgr_del_bond_slave(bond_master, slave, b_pvt)) {
			rcu_read_unlock();
			nss_bridge_mgr_warn("%p: Failed to remove slave (%s) state in Bridge\n", b_pvt, slave->name);
			goto cleanup;
		}
	}
	rcu_read_unlock();

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
	spin_unlock(&br_mgr_ctx.lock);

	/*
	 * The last bond interface is removed from bridge, we can switch back to FDB
	 * learning mode.
	 */
	if (!nss_bridge_mgr_enable_fdb_learning(b_pvt)) {
		spin_lock(&br_mgr_ctx.lock);
		b_pvt->bond_slave_num = 0;
		spin_unlock(&br_mgr_ctx.lock);

		return NOTIFY_DONE;
	}

cleanup:
	rcu_read_lock();
	for_each_netdev_in_bond_rcu(bond_master, slave) {
		if (nss_bridge_mgr_add_bond_slave(bond_master, slave, b_pvt)) {
			nss_bridge_mgr_warn("%p: Failed to add slave (%s) to Bridge\n", b_pvt, slave->name);
		}
	}
	rcu_read_unlock();

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
		pr_err("List creation failed with error = %d\n", error);
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
		pr_err("Could not add fragment acl rule, error = %d\n", error);
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
		pr_err("Could not add TCP FIN rule, error = %d\n", error);
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
		pr_err("Could not add TCP SYN rule, error = %d\n", error);
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
		pr_err("Could not add TCP RST rule, error = %d\n", error);
		goto rst_fail;
	}

	/*
	 * Bind ACL list with service code
	 */
	error = fal_acl_list_bind(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID,
				FAL_ACL_DIREC_IN, FAL_ACL_BIND_SERVICE_CODE, NSS_PPE_SC_VLAN_FILTER_BYPASS);
	if (error != SW_OK) {
		pr_err("Could not bind ACL list, error = %d\n", error);
		goto bind_fail;
	}

	pr_info("Created ACL rule\n");
	return true;

bind_fail:
	error = fal_acl_rule_delete(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID,
				NSS_BRIDGE_MGR_ACL_RST_RULE_ID, NSS_BRIDGE_MGR_ACL_RULE_NR);
	if (error != SW_OK) {
		pr_err("TCP RST rule deletion failed, error %d\n", error);
	}

rst_fail:
	error = fal_acl_rule_delete(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID,
				NSS_BRIDGE_MGR_ACL_SYN_RULE_ID, NSS_BRIDGE_MGR_ACL_RULE_NR);
	if (error != SW_OK) {
		pr_err("TCP SYN rule deletion failed, error %d\n", error);
	}

syn_fail:
	error = fal_acl_rule_delete(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID,
				NSS_BRIDGE_MGR_ACL_FIN_RULE_ID, NSS_BRIDGE_MGR_ACL_RULE_NR);
	if (error != SW_OK) {
		pr_err("TCP FIN rule deletion failed, error %d\n", error);
	}

fin_fail:
	error = fal_acl_rule_delete(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID,
				NSS_BRIDGE_MGR_ACL_FRAG_RULE_ID, NSS_BRIDGE_MGR_ACL_RULE_NR);
	if (error != SW_OK) {
		pr_err("IP fragmentation rule deletion failed, error %d\n", error);
	}

frag_fail:
	error = fal_acl_list_destroy(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID);
	if (error != SW_OK) {
		pr_err("ACL list destroy failed, error %d\n", error);
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
		pr_err("TCP SYN rule deletion failed, error %d\n", error);
	}

	error = fal_acl_rule_delete(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID,
				NSS_BRIDGE_MGR_ACL_FIN_RULE_ID, NSS_BRIDGE_MGR_ACL_RULE_NR);
	if (error != SW_OK) {
		pr_err("TCP FIN rule deletion failed, error %d\n", error);
	}

	error = fal_acl_rule_delete(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID,
				NSS_BRIDGE_MGR_ACL_RST_RULE_ID, NSS_BRIDGE_MGR_ACL_RULE_NR);
	if (error != SW_OK) {
		pr_err("TCP RST rule deletion failed, error %d\n", error);
	}

	error = fal_acl_rule_delete(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID,
				NSS_BRIDGE_MGR_ACL_FRAG_RULE_ID, NSS_BRIDGE_MGR_ACL_RULE_NR);
	if (error != SW_OK) {
		pr_err("IP fragmentation rule deletion failed, error %d\n", error);
	}

	error = fal_acl_list_destroy(NSS_BRIDGE_MGR_ACL_DEV_ID, NSS_BRIDGE_MGR_ACL_LIST_ID);
	if (error != SW_OK) {
		pr_err("ACL list destroy failed, error %d\n", error);
	}
}

#endif

/*
 * nss_bridge_mgr_join_bridge()
 *	Netdevice join bridge and send netdevice joining bridge message to NSS FW.
 */
static int nss_bridge_mgr_join_bridge(struct net_device *dev, struct nss_bridge_pvt *br, int32_t ifnum)
{
#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
	fal_port_t port_num = (fal_port_t)ifnum;
	struct net_device *real_dev;

	if (NSS_BRIDGE_MGR_IF_IS_TYPE_PHYSICAL(ifnum)) {
		/*
		 * If there is a wan interface added in bridge, create a
		 * separate VSI for it, hence avoiding FDB based forwarding.
		 * This is done by not sending join message to the bridge in NSS.
		 */
		if (br_mgr_ctx.wan_if_num == ifnum) {
			if (!nss_bridge_mgr_l2_exception_acl_enable()) {
				nss_bridge_mgr_warn("%p: failed to enable ACL\n", br);
				return -1;
			}
			br->wan_if_enabled = true;
			br->wan_if_num = ifnum;
			nss_bridge_mgr_info("if_num %d is added as WAN interface \n", ifnum);
			return 0;
		}

		if (ppe_port_vsi_get(NSS_BRIDGE_MGR_SWITCH_ID, port_num, &br->port_vsi[port_num - 1])) {
			nss_bridge_mgr_warn("%p: failed to save port VSI of physical interface\n", br);
			return -1;
		}

		if (ppe_port_vsi_set(NSS_BRIDGE_MGR_SWITCH_ID, port_num, br->vsi)) {
			nss_bridge_mgr_warn("%p: failed to set bridge VSI for physical interface\n", br);
			return -1;
		}
	} else if (is_vlan_dev(dev)) {
		/*
		 * Find real_dev associated with the VLAN
		 */
		real_dev = nss_vlan_mgr_get_real_dev(dev);
		if (real_dev && is_vlan_dev(real_dev))
			real_dev = nss_vlan_mgr_get_real_dev(real_dev);
		if (real_dev == NULL) {
			nss_bridge_mgr_warn("%p: real dev for the vlan: %s in NULL\n", br, dev->name);
			return -1;
		}

		/*
		 * This is a valid vlan dev, add the vlan dev to bridge
		 */
		if (nss_vlan_mgr_join_bridge(dev, br->vsi)) {
			nss_bridge_mgr_warn("%p: vlan device failed to join bridge\n", br);
			return -1;
		}

		/*
		 * dev is a bond with VLAN and VLAN is added to bridge
		 */
		if (netif_is_bond_master(real_dev)) {
			if (nss_bridge_tx_join_msg(br->ifnum, dev) != NSS_TX_SUCCESS) {
				nss_bridge_mgr_warn("%p: Interface %s join bridge failed\n", br, dev->name);
				nss_vlan_mgr_leave_bridge(dev, br->vsi);
				return -1;
			}

			/*
			 * Add the bond_master to bridge.
			 */
			if (nss_bridge_mgr_bond_master_join(real_dev, br) != NOTIFY_DONE) {
				nss_bridge_mgr_warn("%p: Slaves of bond interface %s join bridge failed\n", br, real_dev->name);
				nss_bridge_tx_leave_msg(br->ifnum, dev);
				nss_vlan_mgr_leave_bridge(dev, br->vsi);
				return -1;
			}

			return 0;
		}
	}
#endif

	if (nss_bridge_tx_join_msg(br->ifnum, dev) != NSS_TX_SUCCESS) {
		nss_bridge_mgr_warn("%p: Interface %s join bridge failed\n", br, dev->name);
#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
		if (NSS_BRIDGE_MGR_IF_IS_TYPE_PHYSICAL(ifnum))
			ppe_port_vsi_set(NSS_BRIDGE_MGR_SWITCH_ID, port_num, br->port_vsi[port_num - 1]);
#endif
		return -1;
	}

	return 0;
}

/*
 * nss_bridge_mgr_leave_bridge()
 *	Netdevice leave bridge and send netdevice leaving bridge message to NSS FW.
 */
static int nss_bridge_mgr_leave_bridge(struct net_device *dev, struct nss_bridge_pvt *br, int32_t ifnum)
{
#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
	fal_port_t port_num = (fal_port_t)ifnum;
	struct net_device *real_dev;

	if (NSS_BRIDGE_MGR_IF_IS_TYPE_PHYSICAL(ifnum)) {
		if (fal_stp_port_state_set(NSS_BRIDGE_MGR_SWITCH_ID, NSS_BRIDGE_MGR_SPANNING_TREE_ID, port_num, FAL_STP_FORWARDING)) {
			nss_bridge_mgr_warn("%p: faied to set the STP state to forwarding\n", br);
			return -1;
		}

		/*
		 * If there is a wan interface added in bridge, a separate
		 * VSI is created for it by not sending join message to NSS.
		 * Hence a leave message should also be avaoided.
		 */
		if ((br->wan_if_enabled) && (br->wan_if_num == ifnum)) {
			nss_bridge_mgr_l2_exception_acl_disable();
			br->wan_if_enabled = false;
			br->wan_if_num = -1;
			nss_bridge_mgr_info("if_num %d is added as WAN interface\n", ifnum);
			return 0;
		}

		if (ppe_port_vsi_set(NSS_BRIDGE_MGR_SWITCH_ID, port_num, br->port_vsi[port_num - 1])) {
			nss_bridge_mgr_warn("%p: failed to restore port VSI of physical interface\n", br);
			fal_stp_port_state_set(NSS_BRIDGE_MGR_SWITCH_ID, NSS_BRIDGE_MGR_SPANNING_TREE_ID, port_num, FAL_STP_DISABLED);
			return -1;
		}
	} else if (is_vlan_dev(dev)) {
		/*
		 * Find real_dev associated with the VLAN.
		 */
		real_dev = nss_vlan_mgr_get_real_dev(dev);
		if (real_dev && is_vlan_dev(real_dev))
			real_dev = nss_vlan_mgr_get_real_dev(real_dev);
		if (real_dev == NULL) {
			nss_bridge_mgr_warn("%p: real dev for the vlan: %s in NULL\n", br, dev->name);
			return -1;
		}

		/*
		 * This is a valid vlan dev, remove the vlan dev from bridge.
		 */
		if (nss_vlan_mgr_leave_bridge(dev, br->vsi)) {
			nss_bridge_mgr_warn("%p: vlan device failed to leave bridge\n", br);
			return -1;
		}

		/*
		 * dev is a bond with VLAN and VLAN is removed from bridge
		 */
		if (netif_is_bond_master(real_dev)) {
			if (nss_bridge_tx_leave_msg(br->ifnum, dev) != NSS_TX_SUCCESS) {
				nss_bridge_mgr_warn("%p: Interface %s leave bridge failed\n", br, dev->name);
				nss_vlan_mgr_join_bridge(dev, br->vsi);
				nss_bridge_tx_join_msg(br->ifnum, dev);
				return -1;
			}

			/*
			 * Remove the bond_master from bridge.
			 */
			if (nss_bridge_mgr_bond_master_leave(real_dev, br) != NOTIFY_DONE) {
				nss_bridge_mgr_warn("%p: Slaves of bond interface %s leave bridge failed\n", br, real_dev->name);
				nss_vlan_mgr_join_bridge(dev, br->vsi);
				nss_bridge_tx_join_msg(br->ifnum, dev);
				return -1;
			}

			return 0;
		}
	}
#endif

	if (nss_bridge_tx_leave_msg(br->ifnum, dev) != NSS_TX_SUCCESS) {
		nss_bridge_mgr_warn("%p: Interface %s leave bridge failed\n", br, dev->name);
#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
		if (is_vlan_dev(dev)) {
			nss_vlan_mgr_join_bridge(dev, br->vsi);
			nss_bridge_tx_join_msg(br->ifnum, dev);
		} else if (NSS_BRIDGE_MGR_IF_IS_TYPE_PHYSICAL(ifnum)) {
			fal_stp_port_state_set(NSS_BRIDGE_MGR_SWITCH_ID, NSS_BRIDGE_MGR_SPANNING_TREE_ID, port_num, FAL_STP_DISABLED);
			ppe_port_vsi_set(NSS_BRIDGE_MGR_SWITCH_ID, port_num, br->vsi);
		}
#endif
		return -1;
	}

	return 0;
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

	/*
	 * Add or remove the slave based based on linking event
	 */
	if (cu_info->linking) {
		if (nss_bridge_mgr_add_bond_slave(cu_info->upper_dev, bond_slave, b_pvt)) {
			nss_bridge_mgr_warn("%p: Failed to add slave (%s) state in Bridge %s\n", b_pvt,
					cu_info->upper_dev->name, master->name);
		}
	} else {
		if (nss_bridge_mgr_del_bond_slave(cu_info->upper_dev, bond_slave, b_pvt)) {
			nss_bridge_mgr_warn("%p: Failed to remove slave (%s) state in Bridge %s\n", b_pvt,
					cu_info->upper_dev->name, master->name);
		}
	}

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

	nss_bridge_mgr_trace("%p: MTU changed to %d, send message to NSS\n", b_pvt, dev->mtu);

	if (nss_bridge_tx_set_mtu_msg(b_pvt->ifnum, dev->mtu) != NSS_TX_SUCCESS) {
		nss_bridge_mgr_warn("%p: Failed to send change MTU message to NSS\n", b_pvt);
		return NOTIFY_BAD;
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
		nss_bridge_mgr_trace("%p: MAC are the same..skip processing it\n", b_pvt);
		return NOTIFY_DONE;
	}
	spin_unlock(&br_mgr_ctx.lock);

	nss_bridge_mgr_trace("%p: MAC changed to %pM, update NSS\n", b_pvt, dev->dev_addr);

	if (nss_bridge_tx_set_mac_addr_msg(b_pvt->ifnum, dev->dev_addr) != NSS_TX_SUCCESS) {
		nss_bridge_mgr_warn("%p: Failed to send change MAC address message to NSS\n", b_pvt);
		return NOTIFY_BAD;
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
	struct netdev_notifier_changeupper_info *cu_info;
	struct nss_bridge_pvt *b_pvt;
	int32_t slave_ifnum;

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

	/*
	 * Check if upper_dev is a known bridge.
	 */
	b_pvt = nss_bridge_mgr_find_instance(cu_info->upper_dev);
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

	slave_ifnum = nss_cmn_get_interface_number_by_dev(dev);
	if (slave_ifnum < 0) {
		nss_bridge_mgr_warn("%s: failed to find interface number\n", dev->name);
		return NOTIFY_DONE;
	}

	if (cu_info->linking) {
		nss_bridge_mgr_trace("%p: Interface %s joining bridge %s\n", b_pvt, dev->name, cu_info->upper_dev->name);
		if (nss_bridge_mgr_join_bridge(dev, b_pvt, slave_ifnum)) {
			nss_bridge_mgr_warn("%p: Interface %s failed to join bridge %s\n", b_pvt, dev->name, cu_info->upper_dev->name);
			return NOTIFY_BAD;
		}

		return NOTIFY_DONE;
	}

	nss_bridge_mgr_trace("%p: Interface %s leaving bridge %s\n", b_pvt, dev->name, cu_info->upper_dev->name);
	if (nss_bridge_mgr_leave_bridge(dev, b_pvt, slave_ifnum)) {
		nss_bridge_mgr_warn("%p: Interface %s failed to leave bridge %s\n", b_pvt, dev->name, cu_info->upper_dev->name);
		return NOTIFY_BAD;
	}

	return NOTIFY_DONE;
}

/*
 * nss_bridge_mgr_register_event()
 *	Bridge manager handles bridge registration notification.
 */
static int nss_bridge_mgr_register_event(struct netdev_notifier_info *info)
{
	struct net_device *dev = netdev_notifier_info_to_dev(info);
	struct nss_bridge_pvt *b_pvt;
	int ifnum;
#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
	uint32_t vsi_id = 0;
#endif

	b_pvt = nss_bridge_mgr_create_instance(dev);
	if (!b_pvt)
		return NOTIFY_DONE;

	b_pvt->dev = dev;

	ifnum = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_BRIDGE);
	if (ifnum < 0) {
		nss_bridge_mgr_warn("%p: failed to alloc bridge di\n", b_pvt);
		nss_bridge_mgr_delete_instance(b_pvt);
		return NOTIFY_DONE;
	}

	if (!nss_bridge_register(ifnum, dev, NULL, NULL, 0, b_pvt)) {
		nss_bridge_mgr_warn("%p: failed to register bridge di to NSS\n", b_pvt);
		goto fail;
	}

#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
	if (ppe_vsi_alloc(NSS_BRIDGE_MGR_SWITCH_ID, &vsi_id)) {
		nss_bridge_mgr_warn("%p: failed to alloc bridge vsi\n", b_pvt);
		goto fail_1;
	}

	b_pvt->vsi = vsi_id;

	if (nss_bridge_tx_vsi_assign_msg(ifnum, vsi_id) != NSS_TX_SUCCESS) {
		nss_bridge_mgr_warn("%p: failed to assign vsi msg\n", b_pvt);
		goto fail_2;
	}
#endif

	if (nss_bridge_tx_set_mac_addr_msg(ifnum, dev->dev_addr) != NSS_TX_SUCCESS) {
		nss_bridge_mgr_warn("%p: failed to set mac_addr msg\n", b_pvt);
		goto fail_3;
	}

	if (nss_bridge_tx_set_mtu_msg(ifnum, dev->mtu) != NSS_TX_SUCCESS) {
		nss_bridge_mgr_warn("%p: failed to set mtu msg\n", b_pvt);
		goto fail_3;
	}

	/*
	 * All done, take a snapshot of the current mtu and mac addrees
	 */
	b_pvt->ifnum = ifnum;
	b_pvt->mtu = dev->mtu;
	b_pvt->wan_if_num = -1;
	b_pvt->wan_if_enabled = false;
	ether_addr_copy(b_pvt->dev_addr, dev->dev_addr);
	spin_lock(&br_mgr_ctx.lock);
	list_add(&b_pvt->list, &br_mgr_ctx.list);
	spin_unlock(&br_mgr_ctx.lock);

	return NOTIFY_DONE;

fail_3:
#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
	if (nss_bridge_tx_vsi_unassign_msg(ifnum, vsi_id) != NSS_TX_SUCCESS)
		nss_bridge_mgr_warn("%p: failed to unassign vsi\n", b_pvt);

fail_2:
	ppe_vsi_free(NSS_BRIDGE_MGR_SWITCH_ID, vsi_id);

fail_1:
#endif
	nss_bridge_unregister(ifnum);

fail:
	if (nss_dynamic_interface_dealloc_node(ifnum, NSS_DYNAMIC_INTERFACE_TYPE_BRIDGE) != NSS_TX_SUCCESS)
		nss_bridge_mgr_warn("%p: failed to dealloc bridge di\n", b_pvt);

	nss_bridge_mgr_delete_instance(b_pvt);

	return NOTIFY_DONE;
}

/*
 * nss_bridge_mgr_unregister_event()
 *	Bridge manager handles bridge unregistration notification.
 */
static int nss_bridge_mgr_unregister_event(struct netdev_notifier_info *info)
{
	struct net_device *dev = netdev_notifier_info_to_dev(info);
	struct nss_bridge_pvt *b_pvt;

	/*
	 * Do we have it on record?
	 */
	b_pvt = nss_bridge_mgr_find_instance(dev);
	if (!b_pvt)
		return NOTIFY_DONE;

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
		nss_bridge_mgr_warn("%p: failed to unassign vsi\n", b_pvt);

	ppe_vsi_free(NSS_BRIDGE_MGR_SWITCH_ID, b_pvt->vsi);

	/*
	 * It may happen that the same VSI is allocated again,
	 * so there is a need to flush bridge FDB table.
	 */
	if (fal_fdb_entry_del_byfid(NSS_BRIDGE_MGR_SWITCH_ID, b_pvt->vsi, FAL_FDB_DEL_STATIC)) {
		nss_bridge_mgr_warn("%p: Failed to flush FDB table for vsi:%d in PPE\n", b_pvt, b_pvt->vsi);
	}
#endif

	nss_bridge_mgr_trace("%p: Bridge %s unregsitered. Freeing bridge di %d\n", b_pvt, dev->name, b_pvt->ifnum);

	nss_bridge_unregister(b_pvt->ifnum);

	if (nss_dynamic_interface_dealloc_node(b_pvt->ifnum, NSS_DYNAMIC_INTERFACE_TYPE_BRIDGE) != NSS_TX_SUCCESS)
		nss_bridge_mgr_warn("%p: dealloc bridge di failed\n", b_pvt);

	nss_bridge_mgr_delete_instance(b_pvt);

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
		nss_bridge_mgr_warn("%p: interface %s is not physical interface\n",
				root_dev, root_dev->name);
		return false;
	}

	return true;

error:
	nss_bridge_mgr_warn("%p: cannot find the real device for VLAN %s\n", dev, dev->name);
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
		nss_bridge_mgr_warn("%p: bridge device not found\n", event->br);
		return NOTIFY_DONE;
	}

	nss_bridge_mgr_trace("%p: MAC: %pM, original source: %s, new source: %s, bridge: %s\n",
			event, event->addr, event->orig_dev->name, event->dev->name, br_dev->name);

	/*
	 * When a MAC address move from a physical interface to a non-physical
	 * interface, the FDB entry in the PPE needs to be flushed.
	 */
	if (!nss_bridge_mgr_is_physical_dev(event->orig_dev)) {
		nss_bridge_mgr_trace("%p: original source is not a physical interface\n", event->orig_dev);
		dev_put(br_dev);
		return NOTIFY_DONE;
	}

	if (nss_bridge_mgr_is_physical_dev(event->dev)) {
		nss_bridge_mgr_trace("%p: new source is not a non-physical interface\n", event->dev);
		dev_put(br_dev);
		return NOTIFY_DONE;
	}

	b_pvt = nss_bridge_mgr_find_instance(br_dev);
	dev_put(br_dev);
	if (!b_pvt) {
		nss_bridge_mgr_warn("%p: bridge instance not found\n", event->br);
		return NOTIFY_DONE;
	}

	memset(&entry, 0, sizeof(entry));
	memcpy(&entry.addr, event->addr, ETH_ALEN);
	entry.fid = b_pvt->vsi;
	if (SW_OK != fal_fdb_entry_del_bymac(NSS_BRIDGE_MGR_SWITCH_ID, &entry)) {
		nss_bridge_mgr_warn("%p: FDB entry delete failed with MAC %pM and fid %d\n",
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
		nss_bridge_mgr_warn("Only physical interfaces can be marked as WAN interface: if_num %d\n", if_num);
		return -ENOMSG;
	}

	if (br_mgr_ctx.wan_if_num != -1) {
		nss_bridge_mgr_warn("Cannot overwrite a pre-existing wan interface\n");
		return -ENOMSG;
	}

	br_mgr_ctx.wan_if_num = if_num;
	printk("For adding if_num: %d as WAN interface, do a network restart\n", if_num);
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
		nss_bridge_mgr_warn("Only physical interfaces can be marked/unmarked, if_num: %d\n", if_num);
		return -ENOMSG;
	}

	if (br_mgr_ctx.wan_if_num != if_num) {
		nss_bridge_mgr_warn("This interface is not marked as a WAN interface\n");
		return -ENOMSG;
	}

	br_mgr_ctx.wan_if_num = -1;
	printk("For deleting if_num: %d as WAN interface, do a network restart\n", if_num);
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

static struct ctl_table nss_bridge_mgr_dir[] = {
	{
		.procname	= "bridge_mgr",
		.mode		= 0555,
		.child		= nss_bridge_mgr_table,
	},
	{ }
};

static struct ctl_table nss_bridge_mgr_root_dir[] = {
	{
		.procname	= "nss",
		.mode		= 0555,
		.child		= nss_bridge_mgr_dir,
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
	if (!of_machine_is_compatible("qcom,ipq807x") && !of_machine_is_compatible("qcom,ipq6018"))
		return 0;

	INIT_LIST_HEAD(&br_mgr_ctx.list);
	spin_lock_init(&br_mgr_ctx.lock);
	register_netdevice_notifier(&nss_bridge_mgr_netdevice_nb);
	nss_bridge_mgr_info("Module (Build %s) loaded\n", NSS_CLIENT_BUILD_ID);
#if defined(NSS_BRIDGE_MGR_PPE_SUPPORT)
	br_mgr_ctx.wan_if_num = -1;
	br_fdb_update_register_notify(&nss_bridge_mgr_fdb_update_notifier);
	br_mgr_ctx.nss_bridge_mgr_header = register_sysctl_table(nss_bridge_mgr_root_dir);
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
#endif
}

module_init(nss_bridge_mgr_init_module);
module_exit(nss_bridge_mgr_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS bridge manager");
