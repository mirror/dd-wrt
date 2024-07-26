/*
 **************************************************************************
 * Copyright (c) 2016-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024, Qualcomm Innovation Center, Inc. All rights reserved.
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
 * nss_ppe_bridge_mgr.c
 *	NSS PPE Bridge Interface manager
 */
#include <linux/sysctl.h>
#include <linux/etherdevice.h>
#include <linux/if_vlan.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/if_bridge.h>
#include <linux/netdevice.h>
#include <net/bonding.h>
#include <fal/fal_fdb.h>
#include <ppe_drv_public.h>
#include <nss_ppe_vlan_mgr.h>
#include "nss_ppe_bridge_mgr.h"

#if defined(NSS_PPE_BRIDGE_MGR_OVS_ENABLE)
#include <ovsmgr.h>
#endif

/*
 * Module parameter to enable/disable OVS bridge.
 */
static bool ovs_enabled = false;

/*
 * Module parameter to enable/disable FDB learning.
 * by default disable for open profile and enable
 * for all other profile
 */
#if defined(NSS_PPE_BRIDGE_MGR_FDB_DISABLE)
static bool fdb_disabled = true;
#else
static bool fdb_disabled = false;
#endif

static struct nss_ppe_bridge_mgr_context br_mgr_ctx;

/*
 * nss_ppe_bridge_mgr_delete_instance()
 *	Delete a bridge instance from bridge list and free the bridge instance.
 */
static void nss_ppe_bridge_mgr_delete_instance(struct nss_ppe_bridge_mgr_pvt *b_pvt)
{
	spin_lock(&br_mgr_ctx.lock);
	if (!list_empty(&b_pvt->list)) {
		list_del(&b_pvt->list);
	}

	spin_unlock(&br_mgr_ctx.lock);

	if (b_pvt->iface) {
		ppe_drv_iface_deref(b_pvt->iface);
		b_pvt->iface = NULL;
	}

	kfree(b_pvt);
}

/*
 * nss_ppe_bridge_mgr_create_instance()
 *	Create a private bridge instance.
 */
static struct nss_ppe_bridge_mgr_pvt *nss_ppe_bridge_mgr_create_instance(struct net_device *dev)
{
	struct nss_ppe_bridge_mgr_pvt *br;

#if !defined(NSS_PPE_BRIDGE_MGR_OVS_ENABLE)
	if (!netif_is_bridge_master(dev)) {
		return NULL;
	}
#else
	/*
	 * When OVS is enabled, we have to check for both bridge master
	 * and OVS master.
	 */
	if (!netif_is_bridge_master(dev) && !ovsmgr_is_ovs_master(dev)) {
		return NULL;
	}
#endif

	br = kzalloc(sizeof(*br), GFP_KERNEL);
	if (!br) {
		nss_ppe_bridge_mgr_warn("%px: failed to allocate nss_ppe_bridge_mgr_pvt instance\n", dev);
		return NULL;
	}

	br->iface = ppe_drv_iface_alloc(PPE_DRV_IFACE_TYPE_BRIDGE, dev);
	if (!br->iface) {
		nss_ppe_bridge_mgr_warn("%px: failed to allocate PPE iface instance\n", dev);
		kfree(br);
		return NULL;
	}

	INIT_LIST_HEAD(&br->list);
	return br;
}

/*
 * nss_ppe_bridge_mgr_ppe_unregister_br()
 *	Unregisters the bridge from PPE.
 */
static int nss_ppe_bridge_mgr_ppe_unregister_br(struct nss_ppe_bridge_mgr_pvt *b_pvt)
{
	int res = 0;
	ppe_drv_ret_t ret = ppe_drv_iface_mac_addr_clear(b_pvt->iface);
	if (ret != PPE_DRV_RET_SUCCESS) {
		nss_ppe_bridge_mgr_warn("%px: failed to clear MAC address, error = %d\n", b_pvt->dev, ret);
		res = -EFAULT;
	}

	ret = ppe_drv_br_deinit(b_pvt->iface);
	if (ret != PPE_DRV_RET_SUCCESS) {
		nss_ppe_bridge_mgr_warn("%px: failed to de-initialize bridge, error = %d\n", b_pvt->dev, ret);
		res = -EFAULT;
	}

	return res;
}

/*
 * nss_ppe_bridge_mgr_ppe_register_br()
 *	Registers the bridge to PPE.
 */
static bool nss_ppe_bridge_mgr_ppe_register_br(struct nss_ppe_bridge_mgr_pvt *b_pvt)
{
	ppe_drv_ret_t ret = ppe_drv_br_init(b_pvt->iface);
	if (ret != PPE_DRV_RET_SUCCESS) {
		nss_ppe_bridge_mgr_warn("%px: failed to alloc bridge vsi, error = %d\n", b_pvt->dev, ret);
		return false;
	}

	ret = ppe_drv_iface_mac_addr_set(b_pvt->iface, b_pvt->dev_addr);
	if (ret != PPE_DRV_RET_SUCCESS) {
		nss_ppe_bridge_mgr_warn("%px: failed to set mac_addr, error = %d \n", b_pvt->dev, ret);
		goto fail;
	}

	ret = ppe_drv_iface_mtu_set(b_pvt->iface, b_pvt->mtu);
	if (ret != PPE_DRV_RET_SUCCESS) {
		nss_ppe_bridge_mgr_warn("%px: failed to set mtu, error = %d \n", b_pvt->dev, ret);
		goto fail2;
	}

	/*
	 * Disable FDB learning if OVS is enabled.
	 * TODO: This is incorrect. We need disable individual bridges that are with OVS.
	 */
	b_pvt->fdb_lrn_enabled = true;
	if (ovs_enabled || fdb_disabled) {
		if (ppe_drv_br_fdb_lrn_ctrl(b_pvt->iface, false) != PPE_DRV_RET_SUCCESS) {
			nss_ppe_bridge_mgr_warn("%px: Failed to disable FDB learning\n", b_pvt);
		} else {
			b_pvt->fdb_lrn_enabled = false;
		}
	}

	return true;

fail2:
	ppe_drv_iface_mac_addr_clear(b_pvt->iface);
fail:
	ppe_drv_br_deinit(b_pvt->iface);
	return false;
}

/*
 * nss_ppe_bridge_mgr_bridge_vlan_interfaces_get()
 *	Return the number of VLAN over bridge interfaces present in the bridge
 */
static int nss_ppe_bridge_mgr_bridge_vlan_interfaces_get(struct nss_ppe_bridge_mgr_pvt *b_pvt)
{
	nss_ppe_bridge_mgr_trace("%px: bridge %s bridge_vlan_iface_cnt %lld\n", b_pvt, b_pvt->dev->name,
				 atomic64_read(&b_pvt->bridge_vlan_iface_cnt));
	return atomic64_read(&b_pvt->bridge_vlan_iface_cnt);
}

/*
 * nss_ppe_bridge_mgr_vlan_over_bridge_notfication()
 *	API for incrementing and decrementing the number of VLAN over bridge interfaces present in the bridge
 */
bool nss_ppe_bridge_mgr_vlan_over_bridge_notfication(struct net_device *bridge_dev,
						     enum nss_ppe_vlan_mgr_vlan br_action)
{
	struct nss_ppe_bridge_mgr_pvt *b_pvt;
	bool ret = true;

	b_pvt = nss_ppe_bridge_mgr_find_instance(bridge_dev);
	if (!b_pvt) {
		nss_ppe_bridge_mgr_warn("%px: b_pvt not found for bridge %s\n", b_pvt, bridge_dev->name);
		ret = false;
		return ret;
	}

	if (br_action == NSS_PPE_VLAN_MGR_BR_VLAN_INC) {
		atomic64_inc(&b_pvt->bridge_vlan_iface_cnt);
	} else if (br_action == NSS_PPE_VLAN_MGR_BR_VLAN_DEC) {
		atomic64_dec(&b_pvt->bridge_vlan_iface_cnt);
	} else {
		ret = false;
		nss_ppe_bridge_mgr_warn("Invalid action %d in bridge %s\n", br_action, bridge_dev->name);
	}

	nss_ppe_bridge_mgr_trace("Bridge %s action %d bridge_vlan_iface_cnt %lld\n", bridge_dev->name, br_action,
				 atomic64_read(&b_pvt->bridge_vlan_iface_cnt));
	return ret;
}

/*
 * nss_ppe_bridge_mgr_ppe_leave_br()
 *	Leave net_device from the bridge.
 */
static int nss_ppe_bridge_mgr_ppe_leave_br(struct nss_ppe_bridge_mgr_pvt *b_pvt, struct net_device *dev, bool is_wan)
{
	ppe_drv_ret_t ret;
	struct ppe_drv_iface *iface = ppe_drv_iface_get_by_dev(dev);
	if (!iface) {
		nss_ppe_bridge_mgr_warn("%px: failed to find PPE interface\n", dev);
		return -EPERM;
	}

	ret = ppe_drv_br_stp_state_set(b_pvt->iface, dev, FAL_STP_FORWARDING);
	if (ret != PPE_DRV_RET_SUCCESS) {
		nss_ppe_bridge_mgr_warn("%px: failed to set the STP state to forwarding\n", dev);
		return -EPERM;
	}

	/*
	 * If the configuration WAN port is added to bridge,
	 * The configuration will be done separely. No need to do any change here.
	 */
	if (is_wan) {
		return 1;
	}

	ret = ppe_drv_br_leave(b_pvt->iface, dev);
	if (ret != PPE_DRV_RET_SUCCESS) {
		nss_ppe_bridge_mgr_warn("%px: net_dev (%s) failed to leave bridge\n", dev, dev->name);
		ppe_drv_br_stp_state_set(b_pvt->iface, dev, FAL_STP_DISABLED);
		return -EPERM;
	}

	return 0;
}

/*
 * nss_ppe_bridge_mgr_del_bond_slave()
 *	A slave interface being removed from a bond master that belongs to a bridge.
 */
static int nss_ppe_bridge_mgr_del_bond_slave(struct net_device *bond_master,
		struct net_device *slave, struct nss_ppe_bridge_mgr_pvt *b_pvt)
{
	struct ppe_drv_iface *iface;
	int res;

	nss_ppe_bridge_mgr_trace("%px: Bond Slave %s leaving bridge\n",
			b_pvt, slave->name);

	iface = ppe_drv_iface_get_by_dev(slave);
	if (!iface) {
		nss_ppe_bridge_mgr_warn("%px: PPE interface cannot be found\n", b_pvt);
		return -1;
	}

	/*
	 * Take bridge lock since we are updating the PPE
	 */
	spin_lock(&br_mgr_ctx.lock);
	res = nss_ppe_bridge_mgr_ppe_leave_br(b_pvt, slave, false);
	if (res) {
		spin_unlock(&br_mgr_ctx.lock);
		nss_ppe_bridge_mgr_warn("%px: Unable to leave bridge %s\n", b_pvt, slave->name);
		return -1;
	}
	spin_unlock(&br_mgr_ctx.lock);
	return 0;
}

/*
 * nss_ppe_bridge_mgr_add_bond_slave()
 *	A slave interface being added to a bond master that belongs to a bridge.
 */
static int nss_ppe_bridge_mgr_add_bond_slave(struct net_device *bond_master,
		struct net_device *slave, struct nss_ppe_bridge_mgr_pvt *b_pvt)
{
	struct ppe_drv_iface *iface;
	ppe_drv_ret_t ret;

	nss_ppe_bridge_mgr_trace("%px: Bond Slave %s is added bridge\n",
			b_pvt, slave->name);

	iface = ppe_drv_iface_get_by_dev(slave);
	if (!iface) {
		nss_ppe_bridge_mgr_warn("%px: failed to find PPE interface\n", slave);
		return -EPERM;
	}


	nss_ppe_bridge_mgr_trace("%px: Interface %s adding into bridge\n",
			b_pvt, slave->name);

	/*
	 * Take bridge lock as we are updating vsi and port forwarding
	 * details in PPE Hardware
	 * TODO: Investigate whether this lock is needed or not.
	 */
	spin_lock(&br_mgr_ctx.lock);
	ret = ppe_drv_br_join(b_pvt->iface, slave);
	if (ret != PPE_DRV_RET_SUCCESS) {
		spin_unlock(&br_mgr_ctx.lock);
		nss_ppe_bridge_mgr_warn("%px: Unable to join bridge %s\n", b_pvt, slave->name);
		return -1;
	}
	spin_unlock(&br_mgr_ctx.lock);
	return 0;
}

/*
 * nss_ppe_bridge_mgr_bond_fdb_join()
 *      Update FDB state when a bond interface joining bridge.
 */
static bool nss_ppe_bridge_mgr_bond_fdb_join(struct nss_ppe_bridge_mgr_pvt *b_pvt)
{
	/*
	 * If already other bond devices are attached to bridge,
	 * only increment bond_slave_num,
	 */
	spin_lock(&br_mgr_ctx.lock);
	if (b_pvt->bond_slave_num) {
		b_pvt->bond_slave_num++;
		spin_unlock(&br_mgr_ctx.lock);
		return true;
	}

	b_pvt->bond_slave_num = 1;
	spin_unlock(&br_mgr_ctx.lock);

	/*
	 * This is the first bond device being attached to bridge. In order to enforce Linux
	 * bond slave selection in bridge flows involving bond interfaces, we need to disable
	 * fdb learning on this bridge master to allow flow based bridging.
	 */
	if (ppe_drv_br_fdb_lrn_ctrl(b_pvt->iface, false) != PPE_DRV_RET_SUCCESS) {
		nss_ppe_bridge_mgr_warn("%px: Failed to disable FDB learning\n", b_pvt);
		return false;
	}

	return true;
}

/*
 * nss_ppe_bridge_mgr_bond_fdb_leave()
 *      Update FDB state when a bond interface leaving bridge.
 */
static bool nss_ppe_bridge_mgr_bond_fdb_leave(struct nss_ppe_bridge_mgr_pvt *b_pvt)
{
	nss_ppe_bridge_mgr_assert(b_pvt->bond_slave_num == 0);

	/*
	 * If already other bond devices are attached to bridge,
	 * only increment bond_slave_num,
	 */
	spin_lock(&br_mgr_ctx.lock);
	if (b_pvt->bond_slave_num > 1) {
		b_pvt->bond_slave_num--;
		spin_unlock(&br_mgr_ctx.lock);
		return true;
	}

	b_pvt->bond_slave_num = 0;
	spin_unlock(&br_mgr_ctx.lock);

	if (ovs_enabled || fdb_disabled) {
		return true;
	}

	/*
	 * This is the last bond device being detached from the bridge. Enable the FDB
	 * learning to allow fdb based bridging.
	 */
	if (ppe_drv_br_fdb_lrn_ctrl(b_pvt->iface, true) != PPE_DRV_RET_SUCCESS) {
		nss_ppe_bridge_mgr_warn("%px: Failed to enable FDB learning\n", b_pvt);
		return false;
	}

	return true;
}

/*
 * nss_ppe_bridge_mgr_bond_slave_changeupper()
 *	Add bond slave to bridge VSI
 */
static int nss_ppe_bridge_mgr_bond_slave_changeupper(struct netdev_notifier_changeupper_info *cu_info,
		struct net_device *bond_slave)
{
	struct net_device *master;
	struct nss_ppe_bridge_mgr_pvt *b_pvt;

	/*
	 * Checking if our bond master is part of a bridge
	 */
	master = netdev_master_upper_dev_get(cu_info->upper_dev);
	if (!master) {
		return NOTIFY_DONE;
	}

	b_pvt = nss_ppe_bridge_mgr_find_instance(master);
	if (!b_pvt) {
		nss_ppe_bridge_mgr_warn("The bond master is not part of Bridge dev:%s\n", master->name);
		return NOTIFY_DONE;
	}

	/*
	 * Add or remove the slave based based on linking event
	 */
	if (cu_info->linking) {
		if (nss_ppe_bridge_mgr_add_bond_slave(cu_info->upper_dev, bond_slave, b_pvt)) {
			nss_ppe_bridge_mgr_warn("%px: Failed to add slave (%s) state in Bridge %s\n", b_pvt,
					cu_info->upper_dev->name, master->name);
		}
	} else {
		if (nss_ppe_bridge_mgr_del_bond_slave(cu_info->upper_dev, bond_slave, b_pvt)) {
			nss_ppe_bridge_mgr_warn("%px: Failed to remove slave (%s) state in Bridge %s\n", b_pvt,
					cu_info->upper_dev->name, master->name);
		}
	}

	return NOTIFY_DONE;
}

/*
 * nss_ppe_bridge_mgr_bond_master_join()
 *	Add a bond interface to bridge
 */
static int nss_ppe_bridge_mgr_bond_master_join(struct net_device *bond_master,
		struct nss_ppe_bridge_mgr_pvt *b_pvt)
{
	struct slave *slave;
	struct list_head *iter;
	struct bonding *bond;
	ppe_drv_ret_t ret;

	ASSERT_RTNL();

	nss_ppe_bridge_mgr_assert(netif_is_bond_master(bond_master));
	bond = netdev_priv(bond_master);

	/*
	 * Join each of the bonded slaves to the VSI group
	 */
	bond_for_each_slave(bond, slave, iter) {
		if (nss_ppe_bridge_mgr_add_bond_slave(bond_master, slave->dev, b_pvt)) {
			nss_ppe_bridge_mgr_warn("%px: Failed to add slave (%s) state in Bridge\n", b_pvt, slave->dev->name);
			goto cleanup;
		}
	}

	spin_lock(&br_mgr_ctx.lock);
	ret = ppe_drv_br_join(b_pvt->iface, bond_master);
	if (ret != PPE_DRV_RET_SUCCESS) {
		spin_unlock(&br_mgr_ctx.lock);
		nss_ppe_bridge_mgr_warn("%px: Unable to join bridge %s\n", b_pvt, bond_master->name);
		goto cleanup;
	}

	spin_unlock(&br_mgr_ctx.lock);

	if (nss_ppe_bridge_mgr_bond_fdb_join(b_pvt)) {
		return NOTIFY_DONE;
	}

cleanup:
	bond_for_each_slave(bond, slave, iter) {
		if (nss_ppe_bridge_mgr_del_bond_slave(bond_master, slave->dev, b_pvt)) {
			nss_ppe_bridge_mgr_warn("%px: Failed to remove slave (%s) from Bridge\n", b_pvt, slave->dev->name);
		}
	}

	return NOTIFY_DONE;
}

/*
 * nss_ppe_bridge_mgr_bond_master_leave()
 *	Remove a bond interface from bridge
 */
static int nss_ppe_bridge_mgr_bond_master_leave(struct net_device *bond_master,
		struct nss_ppe_bridge_mgr_pvt *b_pvt)
{
	struct slave *slave;
	struct list_head *iter;
	struct bonding *bond;
	ppe_drv_ret_t ret;

	ASSERT_RTNL();

	nss_ppe_bridge_mgr_assert(netif_is_bond_master(bond_master));
	bond = netdev_priv(bond_master);

	nss_ppe_bridge_mgr_assert(b_pvt->bond_slave_num == 0);

	if (nss_ppe_bridge_mgr_bridge_vlan_interfaces_get(b_pvt)) {
		nss_ppe_bridge_mgr_warn("Bond interface %s not allowed to be leave in VLAN over bridge case "
					"%s", bond_master->name, b_pvt->dev->name);
		return NOTIFY_DONE;
	}

	ret = ppe_drv_br_leave(b_pvt->iface, bond_master);
	if (ret != PPE_DRV_RET_SUCCESS) {
		nss_ppe_bridge_mgr_warn("%px: net_dev (%s) failed to leave bridge\n", bond_master, bond_master->name);
		return NOTIFY_DONE;
	}

	/*
	 * Remove each of the bonded slaves from the VSI group
	 */
	bond_for_each_slave(bond, slave, iter) {
		if (nss_ppe_bridge_mgr_del_bond_slave(bond_master, slave->dev, b_pvt)) {
			nss_ppe_bridge_mgr_warn("%px: Failed to remove slave (%s) from Bridge\n", b_pvt, slave->dev->name);
			goto cleanup;
		}
	}

	if (nss_ppe_bridge_mgr_bond_fdb_leave(b_pvt)) {
		b_pvt->fdb_lrn_enabled = true;
		return NOTIFY_DONE;
	}

cleanup:
	bond_for_each_slave(bond, slave, iter) {
		if (nss_ppe_bridge_mgr_add_bond_slave(bond_master, slave->dev, b_pvt)) {
			nss_ppe_bridge_mgr_warn("%px: Failed to add slave (%s) state in Bridge\n", b_pvt, slave->dev->name);
		}
	}

	ret = ppe_drv_br_join(b_pvt->iface, bond_master);
	if (ret != PPE_DRV_RET_SUCCESS) {
		nss_ppe_bridge_mgr_warn("%px: Unable to join bridge %s\n", b_pvt, bond_master->name);
	}

	return NOTIFY_DONE;
}

/*
 * nss_ppe_bridge_mgr_changemtu_event()
 *     Change bridge MTU.
 */
static int nss_ppe_bridge_mgr_changemtu_event(struct netdev_notifier_info *info)
{
	ppe_drv_ret_t ret;
	struct net_device *dev = netdev_notifier_info_to_dev(info);
	struct nss_ppe_bridge_mgr_pvt *b_pvt = nss_ppe_bridge_mgr_find_instance(dev);

	if (!b_pvt) {
		return NOTIFY_DONE;
	}

	spin_lock(&br_mgr_ctx.lock);
	if (b_pvt->mtu == dev->mtu) {
		spin_unlock(&br_mgr_ctx.lock);
		return NOTIFY_DONE;
	}
	spin_unlock(&br_mgr_ctx.lock);

	nss_ppe_bridge_mgr_trace("%px: MTU changed to %d, \n", b_pvt, dev->mtu);
	ret = ppe_drv_iface_mtu_set(b_pvt->iface, dev->mtu);
	if (ret != PPE_DRV_RET_SUCCESS) {
		nss_ppe_bridge_mgr_warn("%px: failed to set mtu, error = %d \n", dev, ret);
		return NOTIFY_BAD;
	}

	spin_lock(&br_mgr_ctx.lock);
	b_pvt->mtu = dev->mtu;
	spin_unlock(&br_mgr_ctx.lock);
	return NOTIFY_DONE;
}

/*
 * nss_ppe_bridge_mgr_changeaddr_event()
 *	Change bridge MAC address.
 */
static int nss_ppe_bridge_mgr_changeaddr_event(struct netdev_notifier_info *info)
{
	ppe_drv_ret_t ret;
	struct net_device *dev = netdev_notifier_info_to_dev(info);
	struct nss_ppe_bridge_mgr_pvt *b_pvt = nss_ppe_bridge_mgr_find_instance(dev);

	if (!b_pvt) {
		return NOTIFY_DONE;
	}

	spin_lock(&br_mgr_ctx.lock);
	if (!memcmp(b_pvt->dev_addr, dev->dev_addr, ETH_ALEN)) {
		spin_unlock(&br_mgr_ctx.lock);
		nss_ppe_bridge_mgr_trace("%px: MAC are the same..skip processing it\n", b_pvt);
		return NOTIFY_DONE;
	}

	spin_unlock(&br_mgr_ctx.lock);

	nss_ppe_bridge_mgr_trace("%px: MAC changed to %pM, update PPE\n", b_pvt, dev->dev_addr);

	ret = ppe_drv_iface_mac_addr_clear(b_pvt->iface);
	if (ret != PPE_DRV_RET_SUCCESS) {
		nss_ppe_bridge_mgr_warn("%px: failed to clear MAC address, error = %d\n", b_pvt->dev, ret);
		return NOTIFY_DONE;
	}

	ret = ppe_drv_iface_mac_addr_set(b_pvt->iface, (uint8_t *)dev->dev_addr);
	if (ret != PPE_DRV_RET_SUCCESS) {
		nss_ppe_bridge_mgr_warn("%px: failed to set mac_addr, error = %d \n", dev, ret);
		return NOTIFY_DONE;
	}

	spin_lock(&br_mgr_ctx.lock);
	ether_addr_copy(b_pvt->dev_addr, dev->dev_addr);
	spin_unlock(&br_mgr_ctx.lock);
	return NOTIFY_DONE;
}

/*
 * nss_ppe_bridge_mgr_changeupper_event()
 *	Bridge manager handles netdevice joining or leaving bridge notification.
 */
static int nss_ppe_bridge_mgr_changeupper_event(struct netdev_notifier_info *info)
{
	struct net_device *dev = netdev_notifier_info_to_dev(info);
	struct net_device *master_dev;
	struct netdev_notifier_changeupper_info *cu_info;
	struct nss_ppe_bridge_mgr_pvt *b_pvt;

	cu_info = (struct netdev_notifier_changeupper_info *)info;

	/*
	 * Check if the master pointer is valid
	 */
	if (!cu_info->master) {
		return NOTIFY_DONE;
	}

	/*
	 * The master is a bond that we don't need to process, but the bond might be part of a bridge.
	 */
	if (netif_is_bond_slave(dev)) {
		return nss_ppe_bridge_mgr_bond_slave_changeupper(cu_info, dev);
	}

	master_dev = cu_info->upper_dev;

	/*
	 * Check if upper_dev is a known bridge.
	 */
	b_pvt = nss_ppe_bridge_mgr_find_instance(master_dev);
	if (!b_pvt)
		return NOTIFY_DONE;

	/*
	 * Slave device is bond master and it is added/removed to/from bridge
	 */
	if (netif_is_bond_master(dev)) {
		if (cu_info->linking) {
			return nss_ppe_bridge_mgr_bond_master_join(dev, b_pvt);
		} else {
			return nss_ppe_bridge_mgr_bond_master_leave(dev, b_pvt);
		}
	}

	if (cu_info->linking) {
		nss_ppe_bridge_mgr_trace("%px: Interface %s joining bridge %s\n", dev, dev->name, master_dev->name);
		if (nss_ppe_bridge_mgr_join_bridge(dev, master_dev)) {
			nss_ppe_bridge_mgr_warn("%px: Interface %s failed to join bridge %s\n", dev, dev->name, master_dev->name);
		}

		return NOTIFY_DONE;
	}

	nss_ppe_bridge_mgr_trace("%px: Interface %s leaving bridge %s\n", dev, dev->name, master_dev->name);
	if (nss_ppe_bridge_mgr_leave_bridge(dev, master_dev)) {
		nss_ppe_bridge_mgr_warn("%px: Interface %s failed to leave bridge %s\n", dev, dev->name, master_dev->name);
	}

	return NOTIFY_DONE;
}

/*
 * nss_ppe_bridge_mgr_register_event()
 *	Bridge manager handles bridge registration notification.
 */
static int nss_ppe_bridge_mgr_register_event(struct netdev_notifier_info *info)
{
	nss_ppe_bridge_mgr_register_br(netdev_notifier_info_to_dev(info));
	return NOTIFY_DONE;
}

/*
 * nss_ppe_bridge_mgr_unregister_event()
 *	Bridge manager handles bridge unregistration notification.
 */
static int nss_ppe_bridge_mgr_unregister_event(struct netdev_notifier_info *info)
{
	nss_ppe_bridge_mgr_unregister_br(netdev_notifier_info_to_dev(info));
	return NOTIFY_DONE;
}

/*
 * nss_ppe_bridge_mgr_netdevice_event()
 *	Bridge manager handles bridge operation notifications.
 */
static int nss_ppe_bridge_mgr_netdevice_event(struct notifier_block *unused,
				unsigned long event, void *ptr)
{
	struct netdev_notifier_info *info = (struct netdev_notifier_info *)ptr;

	switch (event) {
	case NETDEV_CHANGEADDR:
		return nss_ppe_bridge_mgr_changeaddr_event(info);
	case NETDEV_CHANGEMTU:
		return nss_ppe_bridge_mgr_changemtu_event(info);
	case NETDEV_REGISTER:
		return nss_ppe_bridge_mgr_register_event(info);
	case NETDEV_UNREGISTER:
		return nss_ppe_bridge_mgr_unregister_event(info);
	}

	/*
	 * Notify done for all the events we don't care
	 */
	return NOTIFY_DONE;
}

/*
 * nss_ppe_bridge_mgr_event_notifier_drv()
 *	Bridge manager handles ppe drv operation notifications.
 */
static int nss_ppe_bridge_mgr_event_notifier_drv(struct ppe_drv_notifier_ops *unused,
			int event, struct netdev_notifier_info *info)
{
	switch (event) {
	case PPE_DRV_EVENT_CHANGEUPPER:
		nss_ppe_bridge_mgr_changeupper_event(info);
		break;
	}

	/*
	 * Notify done for all the events we don't care
	 */
	return NOTIFY_DONE;
}

static struct notifier_block nss_ppe_bridge_mgr_netdevice_nb __read_mostly = {
	.notifier_call = nss_ppe_bridge_mgr_netdevice_event,
};

/* register ppe drv netdev notifier callback */
static struct ppe_drv_notifier_ops ppe_drv_notifier_ops_bridge_mgr = {
	.notifier_call = nss_ppe_bridge_mgr_event_notifier_drv,
	.priority = PPE_DRV_NOTIFIER_PRI_2,
};

/*
 * nss_ppe_bridge_mgr_is_physical()
 *	Check if the device is represented on PPE as physical interface.
 */
static bool nss_ppe_bridge_mgr_is_physical(struct net_device *dev)
{
	struct ppe_drv_iface *iface;
	if (!dev) {
		return false;
	}

	iface = ppe_drv_iface_get_by_dev(dev);
	if (!iface) {
		nss_ppe_bridge_mgr_warn("%px: failed to find PPE interface\n", dev);
		return false;
	}

	return ppe_drv_iface_is_physical(iface);
}

/*
 * nss_ppe_bridge_mgr_is_ppe()
 *	Check if the device is represented on PPE.
 */
static bool nss_ppe_bridge_mgr_is_ppe(struct net_device *dev)
{
	struct net_device *real_dev = dev;
	struct ppe_drv_iface *iface;
	if (!dev) {
		return false;
	}

	/*
	 * Check if it is VLAN first because VLAN can be over bond interface.
	 * However, the bond over VLAN is not supported in our driver.
	 */
	if (is_vlan_dev(dev)) {
		real_dev = nss_ppe_vlan_mgr_get_real_dev(dev);
		if (!real_dev) {
			goto error;
		}

		if (is_vlan_dev(real_dev)) {
			real_dev = nss_ppe_vlan_mgr_get_real_dev(real_dev);
			if (!real_dev) {
				goto error;
			}
		}
	}

	/*
	 * Don't consider bond interface because FDB learning is disabled.
	 */
	if (netif_is_bond_master(real_dev)) {
		return false;
	}

	/*
	 * Check DEV. VLAN interfaces are also represented in PPE.
	 */
	iface = ppe_drv_iface_get_by_dev(dev);
	if (!iface) {
		nss_ppe_bridge_mgr_warn("%px: failed to find PPE interface\n", dev);
		return false;
	}

	return true;

error:
	nss_ppe_bridge_mgr_warn("%px: cannot find the real device for VLAN %s\n", dev, dev->name);
	return false;
}

/*
 * nss_ppe_bridge_mgr_fdb_update_callback()
 *	Get invoked when there is a FDB update.
 */
static int nss_ppe_bridge_mgr_fdb_update_callback(struct notifier_block *notifier,
					      unsigned long val, void *ctx)
{
	struct br_fdb_event *event = (struct br_fdb_event *)ctx;
	struct nss_ppe_bridge_mgr_pvt *b_pvt = NULL;
	struct net_device *br_dev = NULL;
	ppe_drv_ret_t ret;

	if (!event->br)
		return NOTIFY_DONE;

	br_dev = br_fdb_bridge_dev_get_and_hold(event->br);
	if (!br_dev) {
		nss_ppe_bridge_mgr_warn("%px: bridge device not found\n", event->br);
		return NOTIFY_DONE;
	}

	nss_ppe_bridge_mgr_trace("%px: MAC: %pM, original source: %s, new source: %s, bridge: %s\n",
			event, event->addr, event->orig_dev->name, event->dev->name, br_dev->name);

	/*
	 * When a MAC address move from a PPE represented interface to a non-PPE represented
	 * interface, the FDB entry in the PPE needs to be flushed.
	 */
	if (!nss_ppe_bridge_mgr_is_ppe(event->orig_dev)) {
		nss_ppe_bridge_mgr_trace("%px: original source is not a physical interface\n", event->orig_dev);
		dev_put(br_dev);
		return NOTIFY_DONE;
	}

	if (nss_ppe_bridge_mgr_is_ppe(event->dev) && nss_ppe_bridge_mgr_is_physical(event->dev)) {
		nss_ppe_bridge_mgr_trace("%px: new source is a PPE physical interface\n", event->dev);
		dev_put(br_dev);
		return NOTIFY_DONE;
	}

	b_pvt = nss_ppe_bridge_mgr_find_instance(br_dev);
	dev_put(br_dev);
	if (!b_pvt) {
		nss_ppe_bridge_mgr_warn("%px: bridge instance not found\n", event->br);
		return NOTIFY_DONE;
	}

	ret = ppe_drv_br_fdb_del_bymac(b_pvt->iface, event->addr);
	if (ret != PPE_DRV_RET_SUCCESS) {
		nss_ppe_bridge_mgr_warn("%px: FDB entry delete failed with MAC %pM\n",
				    b_pvt, event->addr);
	}

	return NOTIFY_OK;
}

/*
 * Notifier block for FDB update
 */
static struct notifier_block nss_ppe_bridge_mgr_fdb_update_notifier = {
	.notifier_call = nss_ppe_bridge_mgr_fdb_update_callback,
};

/*
 * nss_ppe_bridge_mgr_wan_inf_add_handler
 *	Marks an interface as a WAN interface for special handling by bridge.
 */
static int nss_ppe_bridge_mgr_wan_intf_add_handler(struct ctl_table *table,
						int write, void __user *buffer,
						size_t *lenp, loff_t *ppos)
{
	struct net_device *dev;
	char *dev_name;
	char *if_name;
	int ret;
	struct ppe_drv_iface *iface;

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
		nss_ppe_bridge_mgr_warn("Cannot find the net device associated with %s\n", dev_name);
		return -ENODEV;
	}

	iface = ppe_drv_iface_get_by_dev(dev);
	if (!iface) {
		nss_ppe_bridge_mgr_warn("%px: failed to find PPE interface\n", dev);
		return -EPERM;
	}

	if (br_mgr_ctx.wan_netdev) {
		dev_put(dev);
		nss_ppe_bridge_mgr_warn("Cannot overwrite a pre-existing wan interface\n");
		return -ENOMSG;
	}

	br_mgr_ctx.wan_netdev = dev;
	dev_put(dev);
	nss_ppe_bridge_mgr_always("For adding netdev: %s as WAN interface, do a network restart\n", dev_name);
	return ret;
}

/*
 * nss_ppe_bridge_mgr_wan_inf_del_handler
 *	Un-marks an interface as a WAN interface.
 */
static int nss_ppe_bridge_mgr_wan_intf_del_handler(struct ctl_table *table,
						int write, void __user *buffer,
						size_t *lenp, loff_t *ppos)
{
	struct net_device *dev;
	char *dev_name;
	char *if_name;
	int ret;
	struct ppe_drv_iface *iface;

	ret = proc_dostring(table, write, buffer, lenp, ppos);
	if (ret)
		return ret;

	if (!write)
		return ret;

	if_name = br_mgr_ctx.wan_ifname;
	dev_name = strsep(&if_name, " ");
	dev = dev_get_by_name(&init_net, dev_name);
	if (!dev) {
		nss_ppe_bridge_mgr_warn("Cannot find the net device associated with %s\n", dev_name);
		return -ENODEV;
	}

	/*
	 * TODO: Investigate this, this should be check for physical.
	 * Checking Iface could be fine. But, not sure if someone sets VLAN as WAN
	 */
	iface = ppe_drv_iface_get_by_dev(dev);
	if (!iface) {
		nss_ppe_bridge_mgr_warn("%px: failed to find PPE interface\n", dev);
		return -EPERM;
	}

	if (br_mgr_ctx.wan_netdev != dev) {
		dev_put(dev);
		nss_ppe_bridge_mgr_warn("This interface is not marked as a WAN interface\n");
		return -ENOMSG;
	}

	br_mgr_ctx.wan_netdev = NULL;
	dev_put(dev);
	nss_ppe_bridge_mgr_always("For deleting netdev: %s as WAN interface, do a network restart\n", dev_name);
	return ret;
}

/*
 * nss_ppe_bridge_mgr_fdb_handler
 *	disable/enable the PPE fdb.
 */
static int nss_ppe_bridge_mgr_fdb_handler(struct ctl_table *table,
						int write, void __user *buffer,
						size_t *lenp, loff_t *ppos)
{
	int ret = 0;

	ret = proc_dointvec(table, write, buffer, lenp, ppos);
	if (ret)
		return ret;

	if (!write)
		return ret;

	return ret;
}

static struct ctl_table nss_ppe_bridge_mgr_table[] = {
	{
		.procname	= "add_wanif",
		.data           = &br_mgr_ctx.wan_ifname,
		.maxlen         = sizeof(char) * IFNAMSIZ,
		.mode           = 0644,
		.proc_handler   = &nss_ppe_bridge_mgr_wan_intf_add_handler,
	},
	{
		.procname	= "del_wanif",
		.data           = &br_mgr_ctx.wan_ifname,
		.maxlen         = sizeof(char) * IFNAMSIZ,
		.mode           = 0644,
		.proc_handler   = &nss_ppe_bridge_mgr_wan_intf_del_handler,
	},
	{
		.procname	= "fdb_disabled",
		.data           = &fdb_disabled,
		.maxlen         = sizeof(int),
		.mode           = 0644,
		.proc_handler   = &nss_ppe_bridge_mgr_fdb_handler,
	},
	{ }
};

/*
 * nss_ppe_bridge_mgr_find_instance()
 *	Find a bridge instance from bridge list.
 */
struct nss_ppe_bridge_mgr_pvt *nss_ppe_bridge_mgr_find_instance(struct net_device *dev)
{
	struct nss_ppe_bridge_mgr_pvt *br;

#if !defined(NSS_PPE_BRIDGE_MGR_OVS_ENABLE)
	if (!netif_is_bridge_master(dev)) {
		return NULL;
	}
#else
	/*
	 * When OVS is enabled, we have to check for both bridge master
	 * and OVS master.
	 */
	if (!netif_is_bridge_master(dev) && !ovsmgr_is_ovs_master(dev)) {
		return NULL;
	}
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

/*
 * nss_ppe_bridge_mgr_leave_bridge()
 *	Netdevice leave bridge.
 */
int nss_ppe_bridge_mgr_leave_bridge(struct net_device *dev, struct net_device *bridge_dev)
{
	int res;
	bool is_wan = false;
	struct net_device *real_dev;
	ppe_drv_ret_t ret;
	struct ppe_drv_iface *iface;
	struct nss_ppe_bridge_mgr_pvt *b_pvt;
	enum nss_ppe_vlan_mgr_ingress_br_vlan_rule rule_action = NSS_PPE_VLAN_MGR_INGRESS_BR_VLAN_RULE_DEL;

	b_pvt = nss_ppe_bridge_mgr_find_instance(bridge_dev);
	if (!b_pvt) {
		nss_ppe_bridge_mgr_warn("%px: failed to find bridge instance\n", dev);
		return -ENOENT;
	}

	if (!is_vlan_dev(dev)) {

		iface = ppe_drv_iface_get_by_dev(dev);
		if (!iface) {
			nss_ppe_bridge_mgr_warn("%px: failed to find PPE interface\n", dev);
			return -EPERM;
		}

		if (nss_ppe_bridge_mgr_bridge_vlan_interfaces_get(b_pvt)) {
			if (nss_ppe_vlan_mgr_config_bridge_vlan_ingress_rule(iface, b_pvt->dev, rule_action)) {
				nss_ppe_bridge_mgr_warn("Ingress xlate rule delete failed for %s\n", dev->name);
			}
		}

		/*
		 * If there is a wan interface added in bridge, a separate
		 * VSI is created for it.
		 */
		if ((b_pvt->wan_if_enabled) && (b_pvt->wan_netdev == dev)) {
			is_wan = true;
			ppe_drv_br_wanif_clear(dev);
		}

		res = nss_ppe_bridge_mgr_ppe_leave_br(b_pvt, dev, is_wan);
		if (res < 0) {
			nss_ppe_bridge_mgr_warn("%px: failed to leave bridge\n", b_pvt);
			return res;
		} else if (res == 1) {
			b_pvt->wan_if_enabled = false;
			b_pvt->wan_netdev = NULL;
			nss_ppe_bridge_mgr_info("Netdev %px (%s) is added as WAN interface \n", dev, dev->name);
			return 0;
		}

		return 0;
	}

	/*
	 * Post creating bridge VLAN netdev (br-wan1.100), with below sequence ingress rule is not removed for eth4.10
	 * 1) Create eth4.10 - Ingress rule is created is created in eth4 with CVID as 10
	 * 2) Add eth4.10 in the bridge (br-wan1) using brctl cmd
	 * 3) Delete eth4.10 from the bridge (br-wan1)
	 * 4) Delete eth4.10 - Ingres rule is not removed
	 * In #4,nss_ppe_vlan_mgr_join_bridge invokes add xlate rule API
	 * Hence, returning here to avoid creation of rule
	 */
	if (nss_ppe_bridge_mgr_bridge_vlan_interfaces_get(b_pvt)) {
		nss_ppe_bridge_mgr_warn("VLAN interface is created over bridge(%s) and so, removing VLAN interface(%s)"
					"from bridge in PPE is not required", b_pvt->dev->name, dev->name);
		return -EINVAL;
	}
	/*
	 * Find real_dev associated with the VLAN.
	 */
	real_dev = nss_ppe_vlan_mgr_get_real_dev(dev);
	if (real_dev && is_vlan_dev(real_dev)) {
		real_dev = nss_ppe_vlan_mgr_get_real_dev(real_dev);
	}

	if (real_dev == NULL) {
		nss_ppe_bridge_mgr_warn("%px: real dev for the vlan: %s in NULL\n", b_pvt, dev->name);
		return -1;
	}

	iface = ppe_drv_iface_get_by_dev(real_dev);
	if (!iface) {
		nss_ppe_bridge_mgr_warn("%px: failed to find PPE interface\n", real_dev);
		return -EPERM;
	}

	/*
	 * This is a valid vlan dev, remove the vlan dev from bridge.
	 */
	if (nss_ppe_vlan_mgr_leave_bridge(dev, b_pvt->iface)) {
		nss_ppe_bridge_mgr_warn("%px: vlan device failed to leave bridge\n", b_pvt);
		return -1;
	}

	/*
	 * dev is a bond with VLAN and VLAN is removed from bridge
	 */
	if (netif_is_bond_master(real_dev)) {

		/*
		 * Remove the bond_master from bridge.
		 */
		if (!nss_ppe_bridge_mgr_bond_fdb_leave(b_pvt)) {
			nss_ppe_bridge_mgr_warn("%px: Slaves of bond interface %s leave bridge failed\n", b_pvt, real_dev->name);
			nss_ppe_vlan_mgr_join_bridge(dev, b_pvt->iface);
			return -1;
		}
	}

	/*
	 * VLAN mgr updates STP state of the interface.
	 * So just need to update bridge.
	 */
	ret = ppe_drv_br_leave(b_pvt->iface, dev);
	if (ret == PPE_DRV_RET_SUCCESS) {
		return 0;
	}

	/*
	 * Revert the changes since br_leave failed.
	 */
	if (netif_is_bond_master(real_dev)) {

		/*
		 * Remove the bond_master from bridge.
		 */
		if (nss_ppe_bridge_mgr_bond_master_join(real_dev, b_pvt) != NOTIFY_DONE) {
			nss_ppe_bridge_mgr_warn("%px: Slaves of bond interface %s join bridge failed\n", b_pvt, real_dev->name);
		}
	}

	if (nss_ppe_vlan_mgr_join_bridge(dev, b_pvt->iface)) {
		nss_ppe_bridge_mgr_warn("%px: vlan device failed to join bridge\n", b_pvt);
	}

	nss_ppe_bridge_mgr_warn("%px: net_dev (%s) failed to leave bridge\n", dev, dev->name);
	ppe_drv_br_stp_state_set(b_pvt->iface, dev, FAL_STP_DISABLED);
	return -1;
}
EXPORT_SYMBOL(nss_ppe_bridge_mgr_leave_bridge);

/*
 * nss_ppe_bridge_mgr_join_bridge()
 *	Netdevice join bridge.
 */
int nss_ppe_bridge_mgr_join_bridge(struct net_device *dev, struct net_device *bridge_dev)
{
	ppe_drv_ret_t ret;
	struct net_device *real_dev;
	struct ppe_drv_iface *iface;
	struct nss_ppe_bridge_mgr_pvt *b_pvt;
	enum nss_ppe_vlan_mgr_ingress_br_vlan_rule rule_action = NSS_PPE_VLAN_MGR_INGRESS_BR_VLAN_RULE_ADD;

	b_pvt = nss_ppe_bridge_mgr_find_instance(bridge_dev);
	if (!b_pvt) {
		nss_ppe_bridge_mgr_warn("%px: failed to find bridge instance\n", dev);
		return -ENOENT;
	}

	/*
	 * If device is VLAN, we need get real_dev.
	 * TODO: Same might apply when we work with virtual interfaces.
	 */
	if (!is_vlan_dev(dev)) {

		iface = ppe_drv_iface_get_by_dev(dev);
		if (!iface) {
			nss_ppe_bridge_mgr_warn("%px: failed to find PPE interface\n", dev);
			return -EPERM;
		}

		if (nss_ppe_bridge_mgr_bridge_vlan_interfaces_get(b_pvt)) {
			if (nss_ppe_vlan_mgr_config_bridge_vlan_ingress_rule(iface, b_pvt->dev, rule_action)) {
				nss_ppe_bridge_mgr_warn("Ingress xlate rule add failed for %s\n", dev->name);
			}
		}

		/*
		 * If there is a wan interface added in bridge, create a
		 * separate VSI for it, hence avoiding FDB based forwarding.
		 * This is done by not setting fal_port
		 */
		if (br_mgr_ctx.wan_netdev == dev) {
			b_pvt->wan_if_enabled = true;
			b_pvt->wan_netdev = dev;
			ppe_drv_br_wanif_set(dev);
			nss_ppe_bridge_mgr_info("Netdev %px (%s) is added as WAN interface \n", dev, dev->name);
			return 0;
		}

		ret = ppe_drv_br_join(b_pvt->iface, dev);
		if (ret != PPE_DRV_RET_SUCCESS) {
			nss_ppe_bridge_mgr_warn("%px: failed to join bridge\n", b_pvt);
			return -EIO;
		}

		return 0;
	}

	/*
	 * When a VLAN interface is already created over bridge (br-wan1.100), we are not allowing any VLAN interfaces
	 * associated with slaves which are part of parent bridge
	 */

	if (nss_ppe_bridge_mgr_bridge_vlan_interfaces_get(b_pvt)) {
		nss_ppe_bridge_mgr_warn("VLAN interface is created over bridge(%s) and so, adding VLAN interface(%s)"
					"in the bridge is not supported", b_pvt->dev->name, dev->name);
		return -EINVAL;
	}
	/*
	 * Find real_dev associated with the VLAN
	 */
	real_dev = nss_ppe_vlan_mgr_get_real_dev(dev);
	if (real_dev && is_vlan_dev(real_dev)) {
		real_dev = nss_ppe_vlan_mgr_get_real_dev(real_dev);
	}

	if (real_dev == NULL) {
		nss_ppe_bridge_mgr_warn("%px: real dev for the vlan: %s in NULL\n", b_pvt, dev->name);
		return -EINVAL;
	}

	iface = ppe_drv_iface_get_by_dev(real_dev);
	if (!iface) {
		nss_ppe_bridge_mgr_warn("%px: failed to find PPE interface\n", real_dev);
		return -EPERM;
	}

	/*
	 * This is a valid vlan dev, add the vlan dev to bridge
	 * TODO: Use temp API. Then update when VLAN is updated.
	 */
	if (nss_ppe_vlan_mgr_join_bridge(dev, b_pvt->iface)) {
		nss_ppe_bridge_mgr_warn("%px: vlan device failed to join bridge\n", b_pvt);
		return -ENODEV;
	}

	/*
	 * dev is a bond with VLAN and VLAN is added to bridge
	 */
	if (netif_is_bond_master(real_dev)) {

		/*
		 * Add the bond_master to bridge.
		 */
		if (!nss_ppe_bridge_mgr_bond_fdb_join(b_pvt)) {
			nss_ppe_bridge_mgr_warn("%px: Slaves of bond interface %s join bridge failed\n", b_pvt, real_dev->name);
			nss_ppe_vlan_mgr_leave_bridge(dev, b_pvt->iface);
			return -EINVAL;
		}
	}

	ret = ppe_drv_br_join(b_pvt->iface, dev);
	if (ret == PPE_DRV_RET_SUCCESS) {
		return 0;
	}

	/*
	 * Clean up since joing failed.
	 */
	if (netif_is_bond_master(real_dev)) {

		/*
		 * Add the bond_master to bridge.
		 */
		if (nss_ppe_bridge_mgr_bond_master_leave(real_dev, b_pvt) != NOTIFY_DONE) {
			nss_ppe_bridge_mgr_warn("%px: Slaves of bond interface %s leave bridge failed\n", b_pvt, real_dev->name);
		}
	}

	/*
	 * This is a valid vlan dev, add the vlan dev to bridge
	 */
	if (nss_ppe_vlan_mgr_leave_bridge(dev, b_pvt->iface)) {
		nss_ppe_bridge_mgr_warn("%px: vlan device failed to leave bridge\n", b_pvt);
	}


	nss_ppe_bridge_mgr_warn("%px: failed to join bridge\n", b_pvt);
	return -EIO;
}
EXPORT_SYMBOL(nss_ppe_bridge_mgr_join_bridge);

/*
 * nss_ppe_bridge_mgr_unregister_br()
 *	Unregister bridge device, dev, from bridge manager database.
 */
int nss_ppe_bridge_mgr_unregister_br(struct net_device *dev)
{
	struct nss_ppe_bridge_mgr_pvt *b_pvt;
	int res = 0;

	/*
	 * Do we have it on record?
	 */
	b_pvt = nss_ppe_bridge_mgr_find_instance(dev);
	if (!b_pvt) {
		return res;
	}

	res = nss_ppe_bridge_mgr_ppe_unregister_br(b_pvt);

	nss_ppe_bridge_mgr_trace("%px: Bridge %s unregistered. Freeing bridge\n", b_pvt, dev->name);
	nss_ppe_bridge_mgr_delete_instance(b_pvt);
	return res;
}

/*
 * nss_ppe_bridge_mgr_register_br()
 *	Register new bridge instance in bridge manager database.
 */
int nss_ppe_bridge_mgr_register_br(struct net_device *dev)
{
	struct nss_ppe_bridge_mgr_pvt *b_pvt = nss_ppe_bridge_mgr_create_instance(dev);
	if (!b_pvt) {
		return -EINVAL;
	}

	nss_ppe_bridge_mgr_info("%px: Bridge register: %s\n", dev, dev->name);

	b_pvt->dev = dev;
	b_pvt->mtu = dev->mtu;
	ether_addr_copy(b_pvt->dev_addr, dev->dev_addr);

	if (!nss_ppe_bridge_mgr_ppe_register_br(b_pvt)) {
		nss_ppe_bridge_mgr_warn("%px: PPE registeration failed for net_dev %s\n", b_pvt, dev->name);
		nss_ppe_bridge_mgr_delete_instance(b_pvt);
		return -EFAULT;
	}

	/*
	 * All done, take a snapshot of the current mtu and mac addrees
	 */
	b_pvt->wan_netdev = NULL;
	b_pvt->wan_if_enabled = false;
	b_pvt->bond_slave_num = 0;

	spin_lock(&br_mgr_ctx.lock);
	list_add(&b_pvt->list, &br_mgr_ctx.list);
	spin_unlock(&br_mgr_ctx.lock);
	return 0;
}

/*
 * nss_ppe_bridge_mgr_exit_module()
 *	bridge_mgr module exit function
 */
static void __exit nss_ppe_bridge_mgr_exit_module(void)
{
	unregister_netdevice_notifier(&nss_ppe_bridge_mgr_netdevice_nb);
	ppe_drv_notifier_ops_unregister(&ppe_drv_notifier_ops_bridge_mgr);
	nss_ppe_bridge_mgr_info("Module unloaded\n");
	br_fdb_update_unregister_notify(&nss_ppe_bridge_mgr_fdb_update_notifier);

	if (br_mgr_ctx.nss_ppe_bridge_mgr_header) {
		unregister_sysctl_table(br_mgr_ctx.nss_ppe_bridge_mgr_header);
	}

#if defined(NSS_PPE_BRIDGE_MGR_OVS_ENABLE)
	nss_ppe_bridge_mgr_ovs_exit();
#endif
	nss_ppe_vlan_mgr_vlan_over_bridge_unregister_cb();
}

/*
 * nss_ppe_bridge_mgr_init_module()
 *	bridge_mgr module init function
 */
static int __init nss_ppe_bridge_mgr_init_module(void)
{
	/*
	 * Monitor bridge activity only on supported platform
	 * TODO: Remove the devsoc machine compatibility check after SOD.
	 */
	if (!of_machine_is_compatible("qcom,ipq9574-emulation")
			&& !of_machine_is_compatible("qcom,ipq9574")
			&& !of_machine_is_compatible("qcom,ipq5332")
			&& !of_machine_is_compatible("qcom,devsoc")) {
		return -EINVAL;
	}

	INIT_LIST_HEAD(&br_mgr_ctx.list);
	spin_lock_init(&br_mgr_ctx.lock);
	ppe_drv_notifier_ops_register(&ppe_drv_notifier_ops_bridge_mgr);
	register_netdevice_notifier(&nss_ppe_bridge_mgr_netdevice_nb);
	nss_ppe_bridge_mgr_info("Module (Build %s) loaded\n", NSS_PPE_BUILD_ID);
	br_mgr_ctx.wan_netdev = NULL;
	br_fdb_update_register_notify(&nss_ppe_bridge_mgr_fdb_update_notifier);
	br_mgr_ctx.nss_ppe_bridge_mgr_header = register_sysctl("ppe/bridge_mgr", nss_ppe_bridge_mgr_table);
#if defined(NSS_PPE_BRIDGE_MGR_OVS_ENABLE)
	nss_ppe_bridge_mgr_ovs_init();
#endif
	nss_ppe_vlan_mgr_vlan_over_bridge_register_cb(nss_ppe_bridge_mgr_vlan_over_bridge_notfication);
	return 0;
}

module_init(nss_ppe_bridge_mgr_init_module);
module_exit(nss_ppe_bridge_mgr_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS PPE bridge manager");

module_param(ovs_enabled, bool, 0644);
MODULE_PARM_DESC(ovs_enabled, "OVS bridge is enabled");

module_param(fdb_disabled, bool, 0644);
MODULE_PARM_DESC(fdb_disabled, "fdb learning is disabled");
