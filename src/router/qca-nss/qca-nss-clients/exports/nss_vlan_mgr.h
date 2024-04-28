/*
 **************************************************************************
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
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
 * nss_vlan_mgr.h
 *	vlan manager interface definitions.
 */
#ifndef _NSS_VLAN_MGR_H_
#define _NSS_VLAN_MGR_H_

/*
 * nss_vlan_mgr_join_bridge()
 *	update ingress and egress vlan translation rule to use bridge VSI
 *
 * @param dev[IN] vlan device which joined in bridge
 * @param bridge_vsi[IN] bridge VSI to attach to
 * @return 0 for success, -1 for failure
 */
int nss_vlan_mgr_join_bridge(struct net_device *dev, uint32_t bridge_vsi);

/*
 * nss_vlan_mgr_leave_bridge()
 *	update ingress and egress vlan translation rule to restore vlan VSI
 *
 * @param dev[IN] vlan device which left from bridge
 * @param bridge_vsi[IN] bridge VSI to detach from
 * @return 0 for success, -1 for failure
 */
int nss_vlan_mgr_leave_bridge(struct net_device *dev, uint32_t bridge_vsi);

#ifdef NSS_VLAN_MGR_PPE_SUPPORT
/*
 * nss_vlan_mgr_add_bond_slave()
 *	update ingress and egress vlan translation rule to use bond slave
 *
 * @param bond_dev[IN] bond device
 * @param slave_dev[IN] slave device
 * @return 0 for success, -1 for failure
 */
int nss_vlan_mgr_add_bond_slave(struct net_device *bond_dev,
				struct net_device *slave_dev);

/*
 * nss_vlan_mgr_delete_bond_slave()
 *	update ingress and egress vlan translation rule to use bond slave
 *
 * @param slave_dev[IN] slave device
 * @return 0 for success, -1 for failure
 */
int nss_vlan_mgr_delete_bond_slave(struct net_device *slave_dev);
#endif

/*
 * nss_vlan_mgr_get_real_dev()
 *	get real_dev for the vlan
 *
 * @param vlan_dev[IN] device
 * @return real_dev
 */
struct net_device *nss_vlan_mgr_get_real_dev(struct net_device *dev);
#endif /* _NSS_VLAN_MGR_H_ */
