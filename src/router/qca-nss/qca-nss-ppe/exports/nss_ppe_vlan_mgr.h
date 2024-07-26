/*
 **************************************************************************
 * Copyright (c) 2017, 2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
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
 * nss_vlan_mgr.h
 *	vlan manager interface definitions.
 */
#ifndef _NSS_PPE_VLAN_MGR_H_
#define _NSS_PPE_VLAN_MGR_H_

/*
 * enum nss_ppe_vlan_mgr_ingress_br_vlan_rule
 *	Enum for creating and deleting ingress rule for VLAN over bridge topology
 */
enum nss_ppe_vlan_mgr_ingress_br_vlan_rule {
	NSS_PPE_VLAN_MGR_INGRESS_BR_VLAN_RULE_ADD,	/**< Used for adding the ingress rules. */
	NSS_PPE_VLAN_MGR_INGRESS_BR_VLAN_RULE_DEL,	/**< Used for deleting the ingress rules. */
};

/*
 * enum nss_ppe_vlan_mgr_vlan
 *	Enum for incrementing and decreasing the number of bridge VLAN netdev in bridge mgr
 */
enum nss_ppe_vlan_mgr_vlan {
	NSS_PPE_VLAN_MGR_BR_VLAN_INC,	/**< Used for incrementing the number of bridge VLAN netdev. */
	NSS_PPE_VLAN_MGR_BR_VLAN_DEC,	/**< Used for decrementing the number of bridge VLAN netdev. */
};

/*
 * nss_ppe_vlan_mgr_br_vlan_cb_t
 *	Callback API to update bridge manager at bridge netdev level
 *
 * @param bridge_dev[IN] netdevice of bridge
 * @param br_action[IN] Add or delete the ingress rule
 * @return true for success, false for failure
 */
typedef bool (*nss_ppe_vlan_mgr_br_vlan_cb_t)(struct net_device *bridge_dev, enum nss_ppe_vlan_mgr_vlan br_action);

/*
 * nss_ppe_vlan_mgr_leave_bridge()
 *	update ingress and egress vlan translation rule to restore vlan VSI
 *
 * @param dev[IN] vlan device which left from bridge
 * @param bridge_iface[IN] bridge interface to detach from
 * @return 0 for success, -1 for failure
 */
int nss_ppe_vlan_mgr_leave_bridge(struct net_device *dev, struct ppe_drv_iface *bridge_iface);

/*
 * nss_vlan_mgr_join_bridge()
 *	update ingress and egress vlan translation rule to use bridge VSI
 *
 * @param dev[IN] vlan device which joined in bridge
 * @param bridge_iface[IN] bridge interface to attach to
 * @return 0 for success, -1 for failure
 */
int nss_ppe_vlan_mgr_join_bridge(struct net_device *dev, struct ppe_drv_iface *bridge_iface);

/*
 * nss_ppe_vlan_mgr_get_real_dev()
 *	get real_dev for the vlan
 *
 * @param vlan_dev[IN] device
 * @return real_dev
 */
struct net_device *nss_ppe_vlan_mgr_get_real_dev(struct net_device *dev);

/*
 * nss_ppe_vlan_mgr_delete_bond_slave()
 *	update ingress and egress vlan translation rule to use bond slave
 *
 * @param slave_dev[IN] slave device
 * @return 0 for success, -1 for failure
 */
int nss_ppe_vlan_mgr_delete_bond_slave(struct net_device *slave_dev);

/*
 * nss_ppe_vlan_mgr_add_bond_slave()
 *	update ingress and egress vlan translation rule to use bond slave
 *
 * @param bond_dev[IN] bond device
 * @param slave_dev[IN] slave device
 * @return 0 for success, -1 for failure
 */
int nss_ppe_vlan_mgr_add_bond_slave(struct net_device *bond_dev,
				struct net_device *slave_dev);

/*
 * nss_ppe_vlan_mgr_del_vlan_rule()
 *	Delete VLAN translation rule in PPE
 *
 * @param dev[IN] physical device
 * @param bridge_iface[IN] bridge PPE interface
 * @param vid[IN] VLAN ID
 */
void nss_ppe_vlan_mgr_del_vlan_rule(struct net_device *dev, struct ppe_drv_iface *bridge_iface, int vid);

/*
 * nss_ppe_vlan_mgr_add_vlan_rule()
 *	Add VLAN translation rule in PPE
 *
 * @param dev[IN] physical device
 * @param bridge_iface[IN] bridge PPE interface
 * @param vid[IN] VLAN ID
 */
void nss_ppe_vlan_mgr_add_vlan_rule(struct net_device *dev, struct ppe_drv_iface *bridge_iface, int vid);

/*
 * nss_ppe_vlan_mgr_vlan_over_bridge_unregister_cb()
 *	Un-registering the callback in VLAN manager
 *
 * @param no input and output parameter
 */
void nss_ppe_vlan_mgr_vlan_over_bridge_unregister_cb(void);

/*
 * nss_ppe_vlan_mgr_vlan_over_bridge_register_cb()
 *	Registering the callback in VLAN manager
 *
 * @param Callback API to register it in VLAN mgr
 */
void nss_ppe_vlan_mgr_vlan_over_bridge_register_cb(nss_ppe_vlan_mgr_br_vlan_cb_t cb);

/*
 * nss_ppe_vlan_mgr_config_bridge_vlan_ingress_rule()
 *	Create and delete the ingress VLAN translation rule in the slave
 *
 * @param slave_iface[IN] iface of the slave
 * @param bridge_dev[IN] net device of the bridge
 * @param action[IN] Delete or install the ingress VLAN translation rule
 *
 * @return 0 for success, -1 for failure
 */
int nss_ppe_vlan_mgr_config_bridge_vlan_ingress_rule(struct ppe_drv_iface *slave_iface, struct net_device *bridge_dev,
						     enum nss_ppe_vlan_mgr_ingress_br_vlan_rule rule_action);
#endif /* _NSS_PPE_VLAN_MGR_H_ */
