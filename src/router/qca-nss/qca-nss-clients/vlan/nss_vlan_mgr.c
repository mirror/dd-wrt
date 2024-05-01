/*
 **************************************************************************
 * Copyright (c) 2017-2018, 2020 The Linux Foundation. All rights reserved.
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
 * nss_vlan_mgr.c
 *	NSS to HLOS vlan Interface manager
 */
#include <linux/etherdevice.h>
#include <linux/if_vlan.h>
#include <linux/proc_fs.h>
#include <linux/sysctl.h>
#include <linux/module.h>
#include <net/bonding.h>
#include <nss_api_if.h>
#ifdef NSS_VLAN_MGR_PPE_SUPPORT
#include <ref/ref_vsi.h>
#include <fal/fal_portvlan.h>
#include <fal/fal_stp.h>
#endif

#if (NSS_VLAN_MGR_DEBUG_LEVEL < 1)
#define nss_vlan_mgr_assert(fmt, args...)
#else
#define nss_vlan_mgr_assert(c) BUG_ON(!(c))
#endif /* NSS_VLAN_MGR_DEBUG_LEVEL */

/*
 * Compile messages for dynamic enable/disable
 */
#if defined(CONFIG_DYNAMIC_DEBUG)
#define nss_vlan_mgr_warn(s, ...) \
		pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_vlan_mgr_info(s, ...) \
		pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_vlan_mgr_trace(s, ...) \
		pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#else /* CONFIG_DYNAMIC_DEBUG */
/*
 * Statically compile messages at different levels
 */
#if (NSS_VLAN_MGR_DEBUG_LEVEL < 2)
#define nss_vlan_mgr_warn(s, ...)
#else
#define nss_vlan_mgr_warn(s, ...) \
		pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_VLAN_MGR_DEBUG_LEVEL < 3)
#define nss_vlan_mgr_info(s, ...)
#else
#define nss_vlan_mgr_info(s, ...) \
		pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_VLAN_MGR_DEBUG_LEVEL < 4)
#define nss_vlan_mgr_trace(s, ...)
#else
#define nss_vlan_mgr_trace(s, ...) \
		pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif
#endif /* CONFIG_DYNAMIC_DEBUG */

#define NSS_VLAN_PHY_PORT_MIN 1
#define NSS_VLAN_PHY_PORT_MAX 6
#define NSS_VLAN_PHY_PORT_NUM 8
#define NSS_VLAN_PHY_PORT_CHK(n) ((n) >= NSS_VLAN_PHY_PORT_MIN && (n) <= NSS_VLAN_PHY_PORT_MAX)
#define NSS_VLAN_MGR_TAG_CNT(v) ((v->parent) ? NSS_VLAN_TYPE_DOUBLE : NSS_VLAN_TYPE_SINGLE)
#define NSS_VLAN_TPID_SHIFT 16
#define NSS_VLAN_PORT_ROLE_CHANGED 1

#define NSS_VLAN_MGR_SWITCH_ID 0
#define NSS_VLAN_MGR_STP_ID 0

/*
 * vlan client context
 */
struct nss_vlan_mgr_context {
	int ctpid;				/* Customer TPID */
	int stpid;				/* Service TPID */
	int port_role[NSS_VLAN_PHY_PORT_NUM];	/* Role of physical ports */
	struct list_head list;			/* List of vlan private instance */
	spinlock_t lock;			/* Lock to protect vlan private instance */
	struct ctl_table_header *sys_hdr;	/* "/pro/sys/nss/vlan_client" directory */
} vlan_mgr_ctx;

/*
 * vlan manager private structure
 */
struct nss_vlan_pvt {
	struct list_head list;			/* list head */
	struct nss_vlan_pvt *parent;		/* parent vlan instance */
	uint32_t nss_if;			/* nss interface number of this vlan device */

	/*
	 * Fields for Linux information
	 */
	int ifindex;				/* netdev ifindex */
	int32_t port[NSS_VLAN_PHY_PORT_MAX];	/* real physical port of this vlan */
	int32_t bond_ifnum;			/* bond interface number, if vlan is created over bond */
	uint32_t vid;				/* vid info */
	uint32_t tpid;				/* tpid info */
	uint32_t mtu;				/* mtu info */
	uint8_t dev_addr[ETH_ALEN];		/* mac address */

#ifdef NSS_VLAN_MGR_PPE_SUPPORT
	/*
	 * Fields for PPE information
	 */
	uint32_t ppe_vsi;			/* VLAN VSI info */
	uint32_t bridge_vsi;			/* Bridge's VSI when vlan is a member of a bridge */
	uint32_t ppe_cvid;			/* ppe_cvid info */
	uint32_t ppe_svid;			/* ppe_svid info */
	fal_vlan_trans_adv_rule_t eg_xlt_rule;	/* VLAN Translation Rule */
	fal_vlan_trans_adv_action_t eg_xlt_action;	/* VLAN Translation Action */
#endif
	int refs;				/* reference count */
};

/*
 * nss_vlan_mgr_instance_find_and_ref()
 */
static struct nss_vlan_pvt *nss_vlan_mgr_instance_find_and_ref(
						struct net_device *dev)
{
	struct nss_vlan_pvt *v;

	if (!is_vlan_dev(dev)) {
		return NULL;
	}

	spin_lock(&vlan_mgr_ctx.lock);
	list_for_each_entry(v, &vlan_mgr_ctx.list, list) {
		if (v->ifindex == dev->ifindex) {
			v->refs++;
			spin_unlock(&vlan_mgr_ctx.lock);
			return v;
		}
	}
	spin_unlock(&vlan_mgr_ctx.lock);

	return NULL;
}

/*
 * nss_vlan_mgr_instance_deref()
 */
static void nss_vlan_mgr_instance_deref(struct nss_vlan_pvt *v)
{
	spin_lock(&vlan_mgr_ctx.lock);
	BUG_ON(!(--v->refs));
	spin_unlock(&vlan_mgr_ctx.lock);
}

#ifdef NSS_VLAN_MGR_PPE_SUPPORT
/*
 * nss_vlan_mgr_calculate_new_port_role()
 *	check if we can change this port to edge port
 */
static bool nss_vlan_mgr_calculate_new_port_role(int32_t port,
						int32_t portindex)
{
	struct nss_vlan_pvt *vif;
	bool to_edge_port = true;

	if (vlan_mgr_ctx.port_role[port] == FAL_QINQ_EDGE_PORT)
		return false;

	if (vlan_mgr_ctx.ctpid != vlan_mgr_ctx.stpid)
		return false;

	/*
	 * If no other double VLAN interface on the same physcial port,
	 * we set physical port as edge port
	 */
	spin_lock(&vlan_mgr_ctx.lock);
	list_for_each_entry(vif, &vlan_mgr_ctx.list, list) {
		if ((vif->port[portindex] == port) && (vif->parent)) {
			to_edge_port = false;
			break;
		}
	}
	spin_unlock(&vlan_mgr_ctx.lock);

	if (to_edge_port) {
		fal_port_qinq_role_t mode;

		mode.mask = FAL_PORT_QINQ_ROLE_INGRESS_EN | FAL_PORT_QINQ_ROLE_EGRESS_EN;
		mode.ingress_port_role = FAL_QINQ_EDGE_PORT;
		mode.egress_port_role = FAL_QINQ_EDGE_PORT;

		if (fal_port_qinq_mode_set(NSS_VLAN_MGR_SWITCH_ID, port, &mode)) {
			nss_vlan_mgr_warn("failed to set %d as edge port\n", port);
			return false;
		}

		vlan_mgr_ctx.port_role[port] = FAL_QINQ_EDGE_PORT;
	}

	return to_edge_port;
}

/*
 * nss_vlan_mgr_port_role_update()
 *	Update physical port role between EDGE and CORE.
 */
static void nss_vlan_mgr_port_role_update(struct nss_vlan_pvt *vif,
					uint32_t new_ppe_cvid,
					uint32_t new_ppe_svid,
					uint32_t port_id)
{
	int rc;

	/*
	 * Delete old ingress vlan translation rule
	 */
	rc = ppe_port_vlan_vsi_set(NSS_VLAN_MGR_SWITCH_ID, port_id, vif->ppe_svid, vif->ppe_cvid, PPE_VSI_INVALID);
	if (rc != SW_OK) {
		nss_vlan_mgr_warn("Failed to delete old ingress vlan translation rule of port %d, error: %d\n", port_id, rc);
		return;
	}

	/*
	 * Delete old egress vlan translation rule
	 */
	rc = fal_port_vlan_trans_adv_del(NSS_VLAN_MGR_SWITCH_ID, port_id,
			FAL_PORT_VLAN_EGRESS,
			&vif->eg_xlt_rule, &vif->eg_xlt_action);
	if (rc != SW_OK) {
		nss_vlan_mgr_warn("Failed to delete old egress vlan translation of port %d, error: %d\n", port_id, rc);
		return;
	}

	/*
	 * Update ppe_civd and ppe_svid
	 */
	vif->ppe_cvid = new_ppe_cvid;
	vif->ppe_svid = new_ppe_svid;

	/*
	 * Add new ingress vlan translation rule
	 */
	rc = ppe_port_vlan_vsi_set(NSS_VLAN_MGR_SWITCH_ID, port_id, vif->ppe_svid,
				vif->ppe_cvid,
				(vif->bridge_vsi ? vif->bridge_vsi : vif->ppe_vsi));
	if (rc != SW_OK) {
		nss_vlan_mgr_warn("Failed to update ingress vlan translation of port %d, error: %d\n", port_id, rc);
		return;
	}

	/*
	 * Add new egress vlan translation rule
	 */
	vif->eg_xlt_action.cvid_xlt_cmd = (vif->ppe_cvid == FAL_VLAN_INVALID) ? 0 : FAL_VID_XLT_CMD_ADDORREPLACE;
	vif->eg_xlt_action.cvid_xlt = (vif->ppe_cvid == FAL_VLAN_INVALID) ? 0 : vif->ppe_cvid;
	vif->eg_xlt_action.svid_xlt_cmd = (vif->ppe_svid == FAL_VLAN_INVALID) ? 0 : FAL_VID_XLT_CMD_ADDORREPLACE;
	vif->eg_xlt_action.svid_xlt = (vif->ppe_svid == FAL_VLAN_INVALID) ? 0 : vif->ppe_svid;

	rc = fal_port_vlan_trans_adv_add(NSS_VLAN_MGR_SWITCH_ID, port_id,
			FAL_PORT_VLAN_EGRESS, &vif->eg_xlt_rule, &vif->eg_xlt_action);
	if (rc != SW_OK)
		nss_vlan_mgr_warn("Failed to update egress vlan translation of port: %d. error: %d\n", port_id, rc);
}

/*
 * nss_vlan_mgr_port_role_over_bond_update()
 *	Update port role for bond slaves.
 */
static void nss_vlan_mgr_port_role_over_bond_update(struct nss_vlan_pvt *vif,
					uint32_t new_ppe_cvid,
					uint32_t new_ppe_svid)
{
	int i;
	fal_vid_xlt_cmd_t old_cvid_xlt_cmd, old_svid_xlt_cmd;
	uint32_t old_cvid_xlt, old_svid_xlt;

	vif->eg_xlt_rule.port_bitmap = 0;

	/*
	 * For vlan over bond, the vif->eg_xlt_action will be modified while
	 * updating the first slave. Hence we need to store the old vif->eg_xlt_action
	 * and pass the same to modify the existing entry for the other ports/slaves. After
	 * modification of all ports we should update vif->eg_xlt_action with the
	 * new value which is passed to ssdk.
	 */
	old_cvid_xlt_cmd = vif->eg_xlt_action.cvid_xlt_cmd;
	old_cvid_xlt = vif->eg_xlt_action.cvid_xlt;
	old_svid_xlt_cmd = vif->eg_xlt_action.svid_xlt_cmd;
	old_svid_xlt = vif->eg_xlt_action.svid_xlt;
	for (i = 0; i < NSS_VLAN_PHY_PORT_MAX; i++) {
		if (!vif->port[i])
			continue;
		vif->eg_xlt_rule.port_bitmap |= (1 << vif->port[i]);
		nss_vlan_mgr_port_role_update(vif, new_ppe_cvid, new_ppe_svid, vif->port[i]);

		/*
		 * Update vif->eg_xlt_action with old value to modify the entry
		 * for next port/slave
		 */
		vif->eg_xlt_action.cvid_xlt_cmd = old_cvid_xlt_cmd;
		vif->eg_xlt_action.cvid_xlt = old_cvid_xlt;
		vif->eg_xlt_action.svid_xlt_cmd = old_svid_xlt_cmd;
		vif->eg_xlt_action.svid_xlt = old_svid_xlt;
	}

	/*
	 * All ports/slaves are updated now, reset the vif->eg_xlt_action
	 * with the value that is passed to ssdk.
	 */
	vif->eg_xlt_action.cvid_xlt_cmd = (vif->ppe_cvid == FAL_VLAN_INVALID) ? 0 : FAL_VID_XLT_CMD_ADDORREPLACE;
	vif->eg_xlt_action.cvid_xlt = (vif->ppe_cvid == FAL_VLAN_INVALID) ? 0 : vif->ppe_cvid;
	vif->eg_xlt_action.svid_xlt_cmd = (vif->ppe_svid == FAL_VLAN_INVALID) ? 0 : FAL_VID_XLT_CMD_ADDORREPLACE;
	vif->eg_xlt_action.svid_xlt = (vif->ppe_svid == FAL_VLAN_INVALID) ? 0 : vif->ppe_svid;
}

/*
 * nss_vlan_mgr_port_role_event()
 *	Decide port role updation for bond or physical device
 */
static void nss_vlan_mgr_port_role_event(int32_t port, int portindex)
{
	struct nss_vlan_pvt *vif;
	bool vlan_over_bond = false;

	if (vlan_mgr_ctx.ctpid != vlan_mgr_ctx.stpid)
		return;

	spin_lock(&vlan_mgr_ctx.lock);
	list_for_each_entry(vif, &vlan_mgr_ctx.list, list) {
		if ((vif->port[portindex] == port) && (!vif->parent)) {
			vlan_over_bond = vif->bond_ifnum ? true : false;
			if ((vlan_mgr_ctx.port_role[port] == FAL_QINQ_EDGE_PORT) &&
			    (vif->vid != vif->ppe_cvid)) {
				if (!vlan_over_bond)
					nss_vlan_mgr_port_role_update(vif, vif->vid, PPE_VSI_INVALID, vif->port[0]);
				else
					nss_vlan_mgr_port_role_over_bond_update(vif, vif->vid, PPE_VSI_INVALID);
			}

			if ((vlan_mgr_ctx.port_role[port] == FAL_QINQ_CORE_PORT) &&
			    (vif->vid != vif->ppe_svid)) {
				if (!vlan_over_bond)
					nss_vlan_mgr_port_role_update(vif, PPE_VSI_INVALID, vif->vid, vif->port[0]);
				else
					nss_vlan_mgr_port_role_over_bond_update(vif, PPE_VSI_INVALID, vif->vid);
			}
		}
	}
	spin_unlock(&vlan_mgr_ctx.lock);
}

/*
 * nss_vlan_mgr_bond_configure_ppe()
 *	Configure PPE for bond device
 */
static int nss_vlan_mgr_bond_configure_ppe(struct nss_vlan_pvt *v, struct net_device *bond_dev)
{
	uint32_t vsi;
	int ret = 0;
	struct net_device *slave;
	int32_t port;
	int vlan_mgr_bond_port_role = -1;

	if (ppe_vsi_alloc(NSS_VLAN_MGR_SWITCH_ID, &vsi)) {
		nss_vlan_mgr_warn("%s: failed to allocate VSI for bond vlan device", bond_dev->name);
		return -1;
	}

	if (nss_vlan_tx_vsi_attach_msg(v->nss_if, vsi) != NSS_TX_SUCCESS) {
		nss_vlan_mgr_warn("%s: failed to attach VSI to bond vlan interface\n", bond_dev->name);
		goto free_vsi;
	}

	/*
	 * Set vlan_mgr_bond_port_role and check
	 * if all the bond slaves are physical ports
	 */
	rcu_read_lock();
	for_each_netdev_in_bond_rcu(bond_dev, slave) {
		port = nss_cmn_get_interface_number_by_dev(slave);
		if (!NSS_VLAN_PHY_PORT_CHK(port)) {
			rcu_read_unlock();
			nss_vlan_mgr_warn("%s: %d is not valid physical port\n", slave->name, port);
			goto free_vsi;
		}

		/*
		 * vlan_mgr_bond_port_role is same for all the slaves in the bond group
		 */
		if (vlan_mgr_bond_port_role == -1) {
			vlan_mgr_bond_port_role = vlan_mgr_ctx.port_role[port];
		}
	}
	rcu_read_unlock();

	/*
	 * In case the bond interface has no slaves, we do not want to proceed further
	 */
	if (vlan_mgr_bond_port_role == -1) {
		goto free_vsi;
	}

	/*
	 * Calculate ppe cvid and svid
	 */
	if (NSS_VLAN_MGR_TAG_CNT(v) == NSS_VLAN_TYPE_DOUBLE) {
		v->ppe_cvid = v->vid;
		v->ppe_svid = v->parent->vid;
	} else {
		if (((vlan_mgr_ctx.ctpid != vlan_mgr_ctx.stpid) && (v->tpid == vlan_mgr_ctx.ctpid)) ||
				((vlan_mgr_ctx.ctpid == vlan_mgr_ctx.stpid) &&
				 (vlan_mgr_bond_port_role == FAL_QINQ_EDGE_PORT))) {
			v->ppe_cvid = v->vid;
			v->ppe_svid = FAL_VLAN_INVALID;
		} else {
			v->ppe_cvid = FAL_VLAN_INVALID;
			v->ppe_svid = v->vid;
		}
	}

	/*
	 * Add ingress vlan translation rule
	 */
	rcu_read_lock();
	for_each_netdev_in_bond_rcu(bond_dev, slave) {
		port = nss_cmn_get_interface_number_by_dev(slave);
		ret = ppe_port_vlan_vsi_set(NSS_VLAN_MGR_SWITCH_ID, v->port[port - 1], v->ppe_svid, v->ppe_cvid, vsi);
		if (ret != SW_OK) {
			rcu_read_unlock();
			nss_vlan_mgr_warn("bond:%s -> slave:%s: failed to set ingress vlan translation, error: %d\n", bond_dev->name, slave->name, ret);
			goto detach_vsi;
		}
	}
	rcu_read_unlock();

	/*
	 * Add egress vlan translation rule
	 */
	memset(&v->eg_xlt_rule, 0, sizeof(v->eg_xlt_rule));
	memset(&v->eg_xlt_action, 0, sizeof(v->eg_xlt_action));

	/*
	 * Fields for match
	 */
	v->eg_xlt_rule.vsi_valid = true;	/* Use vsi as search key*/
	v->eg_xlt_rule.vsi_enable = true;	/* Use vsi as search key*/
	v->eg_xlt_rule.vsi = vsi;		/* Use vsi as search key*/
	v->eg_xlt_rule.s_tagged = 0x7;		/* Accept tagged/untagged/priority tagged svlan */
	v->eg_xlt_rule.c_tagged = 0x7;		/* Accept tagged/untagged/priority tagged cvlan */

	/*
	 * Fields for action
	 */
	v->eg_xlt_action.cvid_xlt_cmd = (v->ppe_cvid == FAL_VLAN_INVALID) ? 0 : FAL_VID_XLT_CMD_ADDORREPLACE;
	v->eg_xlt_action.cvid_xlt = (v->ppe_cvid == FAL_VLAN_INVALID) ? 0 : v->ppe_cvid;
	v->eg_xlt_action.svid_xlt_cmd = (v->ppe_svid == FAL_VLAN_INVALID) ? 0 : FAL_VID_XLT_CMD_ADDORREPLACE;
	v->eg_xlt_action.svid_xlt = (v->ppe_svid == FAL_VLAN_INVALID) ? 0 : v->ppe_svid;

	rcu_read_lock();
	for_each_netdev_in_bond_rcu(bond_dev, slave) {
		port = nss_cmn_get_interface_number_by_dev(slave);
		v->eg_xlt_rule.port_bitmap |= (1 << v->port[port - 1]);
		ret = fal_port_vlan_trans_adv_add(NSS_VLAN_MGR_SWITCH_ID, v->port[port - 1],
				FAL_PORT_VLAN_EGRESS, &v->eg_xlt_rule, &v->eg_xlt_action);
		if (ret != SW_OK) {
			rcu_read_unlock();
			nss_vlan_mgr_warn("bond:%s -> slave:%s: failed to set egress vlan translation. error:%d\n", bond_dev->name, slave->name, ret);
			goto delete_ingress_rule;
		}
	}
	rcu_read_unlock();

	/*
	 * Update vlan port role
	 */
	if ((v->ppe_svid != FAL_VLAN_INVALID) && (vlan_mgr_bond_port_role != FAL_QINQ_CORE_PORT)) {
		rcu_read_lock();
		for_each_netdev_in_bond_rcu(bond_dev, slave) {
			fal_port_qinq_role_t mode;
			port = nss_cmn_get_interface_number_by_dev(slave);

			/*
			 * If double tag, we should set physical port as core port
			 */
			vlan_mgr_ctx.port_role[port] = FAL_QINQ_CORE_PORT;

			/*
			 * Update port role in PPE
			 */
			mode.mask = FAL_PORT_QINQ_ROLE_INGRESS_EN | FAL_PORT_QINQ_ROLE_EGRESS_EN;
			mode.ingress_port_role = FAL_QINQ_CORE_PORT;
			mode.egress_port_role = FAL_QINQ_CORE_PORT;

			if (fal_port_qinq_mode_set(NSS_VLAN_MGR_SWITCH_ID, port, &mode)) {
				rcu_read_unlock();
				nss_vlan_mgr_warn("bond:%s -> slave:%s: failed to set %d as core port\n", bond_dev->name, slave->name, port);
				goto delete_egress_rule;
			}
		}
		rcu_read_unlock();
		ret = NSS_VLAN_PORT_ROLE_CHANGED;
	}

	v->ppe_vsi = vsi;
	return ret;

delete_egress_rule:
	rcu_read_lock();
	for_each_netdev_in_bond_rcu(bond_dev, slave) {
		port = nss_cmn_get_interface_number_by_dev(slave);
		ret = fal_port_vlan_trans_adv_del(NSS_VLAN_MGR_SWITCH_ID, v->port[port - 1],
				FAL_PORT_VLAN_EGRESS,
				&v->eg_xlt_rule, &v->eg_xlt_action);
		if (ret != SW_OK) {
			nss_vlan_mgr_warn("%px: Failed to delete egress translation rule for port:%d, error: %d\n", v, v->port[port - 1], ret);
		}
	}
	rcu_read_unlock();

delete_ingress_rule:
	rcu_read_lock();
	for_each_netdev_in_bond_rcu(bond_dev, slave) {
		port = nss_cmn_get_interface_number_by_dev(slave);
		ret = ppe_port_vlan_vsi_set(NSS_VLAN_MGR_SWITCH_ID, v->port[port - 1], v->ppe_svid, v->ppe_cvid, PPE_VSI_INVALID);
		if (ret != SW_OK) {
			nss_vlan_mgr_warn("%px: Failed to delete ingress translation rule for port:%d, error: %d\n", v, v->port[port - 1], ret);
		}
	}
	rcu_read_unlock();

detach_vsi:
	if (nss_vlan_tx_vsi_detach_msg(v->nss_if, vsi)) {
		nss_vlan_mgr_warn("%px: Failed to detach vsi %d\n", v, vsi);
	}

free_vsi:
	if (ppe_vsi_free(NSS_VLAN_MGR_SWITCH_ID, vsi)) {
		nss_vlan_mgr_warn("%px: Failed to free VLAN VSI\n", v);
	}

	return -1;
}
/*
 * nss_vlan_mgr_configure_ppe()
 *	Configure PPE for physical devices
 */
static int nss_vlan_mgr_configure_ppe(struct nss_vlan_pvt *v, struct net_device *dev)
{
	uint32_t vsi;
	int ret = 0;

	if (ppe_vsi_alloc(NSS_VLAN_MGR_SWITCH_ID, &vsi)) {
		nss_vlan_mgr_warn("%s: failed to allocate VSI for vlan device", dev->name);
		return -1;
	}

	if (nss_vlan_tx_vsi_attach_msg(v->nss_if, vsi) != NSS_TX_SUCCESS) {
		nss_vlan_mgr_warn("%s: failed to attach VSI to vlan interface\n", dev->name);
		goto free_vsi;
	}

	/*
	 * Calculate ppe cvid and svid
	 */
	if (NSS_VLAN_MGR_TAG_CNT(v) == NSS_VLAN_TYPE_DOUBLE) {
		v->ppe_cvid = v->vid;
		v->ppe_svid = v->parent->vid;
	} else {
		if (((vlan_mgr_ctx.ctpid != vlan_mgr_ctx.stpid) && (v->tpid == vlan_mgr_ctx.ctpid)) ||
		    ((vlan_mgr_ctx.ctpid == vlan_mgr_ctx.stpid) &&
		     (vlan_mgr_ctx.port_role[v->port[0]] == FAL_QINQ_EDGE_PORT))) {
			v->ppe_cvid = v->vid;
			v->ppe_svid = FAL_VLAN_INVALID;
		} else {
			v->ppe_cvid = FAL_VLAN_INVALID;
			v->ppe_svid = v->vid;
		}
	}

	/*
	 * Add ingress vlan translation rule
	 */
	ret = ppe_port_vlan_vsi_set(NSS_VLAN_MGR_SWITCH_ID, v->port[0], v->ppe_svid, v->ppe_cvid, vsi);
	if (ret != SW_OK) {
		nss_vlan_mgr_warn("%s: failed to set ingress vlan translation, error: %d\n", dev->name, ret);
		goto detach_vsi;
	}

	/*
	 * Add egress vlan translation rule
	 */
	memset(&v->eg_xlt_rule, 0, sizeof(v->eg_xlt_rule));
	memset(&v->eg_xlt_action, 0, sizeof(v->eg_xlt_action));

	/*
	 * Fields for match
	 */
	v->eg_xlt_rule.vsi_valid = true;	/* Use vsi as search key*/
	v->eg_xlt_rule.vsi_enable = true;	/* Use vsi as search key*/
	v->eg_xlt_rule.vsi = vsi;		/* Use vsi as search key*/
	v->eg_xlt_rule.s_tagged = 0x7;		/* Accept tagged/untagged/priority tagged svlan */
	v->eg_xlt_rule.c_tagged = 0x7;		/* Accept tagged/untagged/priority tagged cvlan */
	v->eg_xlt_rule.port_bitmap = (1 << v->port[0]); /* Use port as search key*/

	/*
	 * Fields for action
	 */
	v->eg_xlt_action.cvid_xlt_cmd = (v->ppe_cvid == FAL_VLAN_INVALID) ? 0 : FAL_VID_XLT_CMD_ADDORREPLACE;
	v->eg_xlt_action.cvid_xlt = (v->ppe_cvid == FAL_VLAN_INVALID) ? 0 : v->ppe_cvid;
	v->eg_xlt_action.svid_xlt_cmd = (v->ppe_svid == FAL_VLAN_INVALID) ? 0 : FAL_VID_XLT_CMD_ADDORREPLACE;
	v->eg_xlt_action.svid_xlt = (v->ppe_svid == FAL_VLAN_INVALID) ? 0 : v->ppe_svid;

	ret = fal_port_vlan_trans_adv_add(NSS_VLAN_MGR_SWITCH_ID, v->port[0],
				FAL_PORT_VLAN_EGRESS, &v->eg_xlt_rule,
				&v->eg_xlt_action);
	if (ret != SW_OK) {
		nss_vlan_mgr_warn("%s: failed to set egress vlan translation\n", dev->name);
		goto delete_ingress_rule;
	}

	if ((v->ppe_svid != FAL_VLAN_INVALID) && (vlan_mgr_ctx.port_role[v->port[0]] != FAL_QINQ_CORE_PORT)) {
		fal_port_qinq_role_t mode;

		/*
		 * If double tag, we should set physical port as core port
		 */
		vlan_mgr_ctx.port_role[v->port[0]] = FAL_QINQ_CORE_PORT;

		/*
		 * Update port role in PPE
		 */
		mode.mask = FAL_PORT_QINQ_ROLE_INGRESS_EN | FAL_PORT_QINQ_ROLE_EGRESS_EN;
		mode.ingress_port_role = FAL_QINQ_CORE_PORT;
		mode.egress_port_role = FAL_QINQ_CORE_PORT;

		if (fal_port_qinq_mode_set(NSS_VLAN_MGR_SWITCH_ID, v->port[0], &mode)) {
			nss_vlan_mgr_warn("%s: failed to set %d as core port\n", dev->name, v->port[0]);
			goto delete_egress_rule;
		}
		ret = NSS_VLAN_PORT_ROLE_CHANGED;
	}

	v->ppe_vsi = vsi;
	return ret;

delete_egress_rule:
	ret = fal_port_vlan_trans_adv_del(NSS_VLAN_MGR_SWITCH_ID, v->port[0],
				FAL_PORT_VLAN_EGRESS,
				&v->eg_xlt_rule, &v->eg_xlt_action);
	if (ret != SW_OK) {
		nss_vlan_mgr_warn("%px: Failed to delete egress translation rule, error: %d\n", v, ret);
	}

delete_ingress_rule:
	ret = ppe_port_vlan_vsi_set(NSS_VLAN_MGR_SWITCH_ID, v->port[0], v->ppe_svid, v->ppe_cvid, PPE_VSI_INVALID);
	if (ret != SW_OK) {
		nss_vlan_mgr_warn("%px: Failed to delete ingress translation rule, error: %d\n", v, ret);
	}

detach_vsi:
	if (nss_vlan_tx_vsi_detach_msg(v->nss_if, vsi)) {
		nss_vlan_mgr_warn("%px: Failed to detach vsi %d\n", v, vsi);
	}

free_vsi:
	if (ppe_vsi_free(NSS_VLAN_MGR_SWITCH_ID, vsi)) {
		nss_vlan_mgr_warn("%px: Failed to free VLAN VSI\n", v);
	}

	return -1;
}
#endif

/*
 * nss_vlan_mgr_create_instance()
 *	Create vlan instance
 */
static struct nss_vlan_pvt *nss_vlan_mgr_create_instance(
						struct net_device *dev)
{
	struct nss_vlan_pvt *v;
	struct vlan_dev_priv *vlan;
	struct net_device *real_dev;
	struct net_device *slave;
	int32_t port, bondid = -1;

	if (!is_vlan_dev(dev)) {
		return NULL;
	}

	v = kzalloc(sizeof(*v), GFP_KERNEL);
	if (!v) {
		return NULL;
	}

	INIT_LIST_HEAD(&v->list);

	vlan = vlan_dev_priv(dev);
	real_dev = vlan->real_dev;
	v->vid = vlan->vlan_id;
	v->tpid = ntohs(vlan->vlan_proto);

	/*
	 * Check if the vlan has any parent.
	 *
	 * 1. While adding eth0.10/bond0.10, the real_dev will become
	 * eth0/bond0. In this case, v->parent should be NULL and respective
	 * port numbers will be assigned to v->port.
	 *
	 * 2. While adding eth0.10.20/bond0.10.20, the real_dev will
	 * become eth0.10/bond0.10, so v->parent should be valid. But v->parent->parent
	 * should be NULL, as explained above. In this case, we need to copy the
	 * v->parent->port numbers to v->ports as the double vlan is created
	 * on the same physical port(s).
	 *
	 * 3. We ignore the remaining case as we support only 2 valn tags.
	 */
	v->parent = nss_vlan_mgr_instance_find_and_ref(real_dev);
	if (!v->parent) {
		if (!netif_is_bond_master(real_dev)) {
			v->port[0] = nss_cmn_get_interface_number_by_dev(real_dev);
			if (!NSS_VLAN_PHY_PORT_CHK(v->port[0])) {
				nss_vlan_mgr_warn("%s: %d is not valid physical port\n", real_dev->name, v->port[0]);
				kfree(v);
				return NULL;
			}
		} else {
#if defined(BONDING_SUPPORT)
			bondid = bond_get_id(real_dev);
#endif
			if (bondid < 0) {
				nss_vlan_mgr_warn("%px: Invalid LAG group id 0x%x\n", v, bondid);
				kfree(v);
				return NULL;
			}
			rcu_read_lock();
			for_each_netdev_in_bond_rcu(real_dev, slave) {
				port = nss_cmn_get_interface_number_by_dev(slave);
				if (!NSS_VLAN_PHY_PORT_CHK(port)) {
					rcu_read_unlock();
					nss_vlan_mgr_warn("%s: %d is not valid physical port\n", slave->name, port);
					kfree(v);
					return NULL;
				}
				v->port[port - 1] = port;
			}
			rcu_read_unlock();
			v->bond_ifnum = bondid + NSS_LAG0_INTERFACE_NUM;
		}
	} else if (!v->parent->parent) {
		if (is_vlan_dev(real_dev)) {
			vlan = vlan_dev_priv(real_dev);
			real_dev = vlan->real_dev;
		}
		if (!netif_is_bond_master(real_dev)) {
			v->port[0] = v->parent->port[0];
		} else {
			rcu_read_lock();
			for_each_netdev_in_bond_rcu(real_dev, slave) {
				port = nss_cmn_get_interface_number_by_dev(slave);
				if (!NSS_VLAN_PHY_PORT_CHK(port)) {
					rcu_read_unlock();
					nss_vlan_mgr_warn("%s: %d is not valid physical port\n", slave->name, port);
					kfree(v);
					return NULL;
				}
				v->port[port - 1] = v->parent->port[port - 1];
			}
			rcu_read_unlock();
			v->bond_ifnum = v->parent->bond_ifnum;
		}
	} else {
		nss_vlan_mgr_warn("%s: don't support more than 2 vlans\n", dev->name);
		nss_vlan_mgr_instance_deref(v->parent);
		kfree(v);
		return NULL;
	}

	/*
	 * Check if TPID is permited
	 */
	if ((NSS_VLAN_MGR_TAG_CNT(v) == NSS_VLAN_TYPE_DOUBLE) &&
	    ((v->tpid != vlan_mgr_ctx.ctpid) || (v->parent->tpid != vlan_mgr_ctx.stpid))) {
		nss_vlan_mgr_warn("%s: double tag: tpid %04x not match global tpid(%04x, %04x)\n", dev->name, v->tpid, vlan_mgr_ctx.ctpid,
				vlan_mgr_ctx.stpid);
		nss_vlan_mgr_instance_deref(v->parent);
		kfree(v);
		return NULL;
	}

	if ((NSS_VLAN_MGR_TAG_CNT(v) == NSS_VLAN_TYPE_SINGLE) &&
	    ((v->tpid != vlan_mgr_ctx.ctpid) && (v->tpid != vlan_mgr_ctx.stpid))) {
		nss_vlan_mgr_warn("%s: single tag: tpid %04x not match global tpid(%04x, %04x)\n", dev->name, v->tpid, vlan_mgr_ctx.ctpid, vlan_mgr_ctx.stpid);
		kfree(v);
		return NULL;
	}

	v->mtu = dev->mtu;
	ether_addr_copy(v->dev_addr, dev->dev_addr);
	v->ifindex = dev->ifindex;
	v->refs = 1;

	return v;
}

/*
 * nss_vlan_mgr_instance_free()
 *	Destroy vlan instance
 */
static void nss_vlan_mgr_instance_free(struct nss_vlan_pvt *v)
{
#ifdef NSS_VLAN_MGR_PPE_SUPPORT
	int32_t i;
	int ret = 0;
#endif

	spin_lock(&vlan_mgr_ctx.lock);
	BUG_ON(--v->refs);
	if (!list_empty(&v->list)) {
		list_del(&v->list);
	}
	spin_unlock(&vlan_mgr_ctx.lock);

#ifdef NSS_VLAN_MGR_PPE_SUPPORT
	if (v->ppe_vsi) {
		/*
		 * Detach VSI
		 */
		if (nss_vlan_tx_vsi_detach_msg(v->nss_if, v->ppe_vsi)) {
			nss_vlan_mgr_warn("%px: Failed to detach vsi %d\n", v, v->ppe_vsi);
		}

		/*
		 * Delete ingress vlan translation rule
		 */
		for (i = 0; i < NSS_VLAN_PHY_PORT_MAX; i++) {
			if (!v->port[i])
				continue;
			ret = ppe_port_vlan_vsi_set(NSS_VLAN_MGR_SWITCH_ID, v->port[i], v->ppe_svid, v->ppe_cvid, PPE_VSI_INVALID);
			if (ret != SW_OK)
				nss_vlan_mgr_warn("%px: Failed to delete old ingress translation rule, error: %d\n", v, ret);
		}

		/*
		 * Delete egress vlan translation rule
		 */
		v->eg_xlt_rule.port_bitmap = 0;
		for (i = 0; i < NSS_VLAN_PHY_PORT_MAX; i++) {
			if (!v->port[i])
				continue;
			v->eg_xlt_rule.port_bitmap |= (1 << v->port[i]);
			ret = fal_port_vlan_trans_adv_del(NSS_VLAN_MGR_SWITCH_ID, v->port[i],
						FAL_PORT_VLAN_EGRESS,
						&v->eg_xlt_rule, &v->eg_xlt_action);
			if (ret != SW_OK) {
				nss_vlan_mgr_warn("%px: Failed to delete vlan translation rule, error:%d\n", v, ret);
			}
		}

		/*
		 * Free PPE VSI
		 */
		if (ppe_vsi_free(NSS_VLAN_MGR_SWITCH_ID, v->ppe_vsi)) {
			nss_vlan_mgr_warn("%px: Failed to free VLAN VSI\n", v);
		}
	}

	/*
	 * Need to change the physical port role. While adding
	 * eth0.10.20/bond0.10.20, the role of the physical port(s) changed
	 * from EDGE to CORE. So, while removing eth0.10.20/bond0.10.20, the
	 * role of the physical port(s) should be changed from CORE to EDGE.
	 */
	for (i = 0; i < NSS_VLAN_PHY_PORT_MAX; i++) {
		if (v->port[i]) {
			if (nss_vlan_mgr_calculate_new_port_role(v->port[i], i)) {
				nss_vlan_mgr_port_role_event(v->port[i], i);
			}
		}
	}
#endif

	if (v->nss_if) {
		nss_unregister_vlan_if(v->nss_if);
		if (nss_dynamic_interface_dealloc_node(v->nss_if, NSS_DYNAMIC_INTERFACE_TYPE_VLAN) != NSS_TX_SUCCESS)
			nss_vlan_mgr_warn("%px: Failed to dealloc vlan dynamic interface\n", v);
	}

	if (v->parent)
		nss_vlan_mgr_instance_deref(v->parent);

	kfree(v);
}

/*
 * nss_vlan_mgr_changemtu_event()
 */
static int nss_vlan_mgr_changemtu_event(struct netdev_notifier_info *info)
{
	struct net_device *dev = netdev_notifier_info_to_dev(info);
	struct nss_vlan_pvt *v_pvt = nss_vlan_mgr_instance_find_and_ref(dev);

	if (!v_pvt)
		return NOTIFY_DONE;

	spin_lock(&vlan_mgr_ctx.lock);
	if (v_pvt->mtu == dev->mtu) {
		spin_unlock(&vlan_mgr_ctx.lock);
		nss_vlan_mgr_instance_deref(v_pvt);
		return NOTIFY_DONE;
	}
	spin_unlock(&vlan_mgr_ctx.lock);

	if (nss_vlan_tx_set_mtu_msg(v_pvt->nss_if, dev->mtu) != NSS_TX_SUCCESS) {
		nss_vlan_mgr_warn("%s: Failed to send change MTU(%d) message to NSS\n", dev->name, dev->mtu);
		nss_vlan_mgr_instance_deref(v_pvt);
		return NOTIFY_BAD;
	}

	spin_lock(&vlan_mgr_ctx.lock);
	v_pvt->mtu = dev->mtu;
	spin_unlock(&vlan_mgr_ctx.lock);
	nss_vlan_mgr_trace("%s: MTU changed to %d, NSS updated\n", dev->name, dev->mtu);
	nss_vlan_mgr_instance_deref(v_pvt);
	return NOTIFY_DONE;
}

/*
 * int nss_vlan_mgr_changeaddr_event()
 */
static int nss_vlan_mgr_changeaddr_event(struct netdev_notifier_info *info)
{
	struct net_device *dev = netdev_notifier_info_to_dev(info);
	struct nss_vlan_pvt *v_pvt = nss_vlan_mgr_instance_find_and_ref(dev);

	if (!v_pvt)
		return NOTIFY_DONE;

	spin_lock(&vlan_mgr_ctx.lock);
	if (!memcmp(v_pvt->dev_addr, dev->dev_addr, ETH_ALEN)) {
		spin_unlock(&vlan_mgr_ctx.lock);
		nss_vlan_mgr_instance_deref(v_pvt);
		return NOTIFY_DONE;
	}
	spin_unlock(&vlan_mgr_ctx.lock);

	if (nss_vlan_tx_set_mac_addr_msg(v_pvt->nss_if, dev->dev_addr) != NSS_TX_SUCCESS) {
		nss_vlan_mgr_warn("%s: Failed to send change MAC address message to NSS\n", dev->name);
		nss_vlan_mgr_instance_deref(v_pvt);
		return NOTIFY_BAD;
	}

	spin_lock(&vlan_mgr_ctx.lock);
	ether_addr_copy(v_pvt->dev_addr, dev->dev_addr);
	spin_unlock(&vlan_mgr_ctx.lock);
	nss_vlan_mgr_trace("%s: MAC changed to %pM, updated NSS\n", dev->name, dev->dev_addr);
	nss_vlan_mgr_instance_deref(v_pvt);
	return NOTIFY_DONE;
}

/*
 * nss_vlan_mgr_register_event()
 */
static int nss_vlan_mgr_register_event(struct netdev_notifier_info *info)
{
	struct net_device *dev = netdev_notifier_info_to_dev(info);
	struct nss_vlan_pvt *v;
	int if_num;
#ifdef NSS_VLAN_MGR_PPE_SUPPORT
	int ret;
#endif
	uint32_t vlan_tag;
#ifdef NSS_VLAN_MGR_PPE_SUPPORT
	struct net_device *slave;
	int32_t port;
#endif
	int32_t port_if;
	struct vlan_dev_priv *vlan;
	struct net_device *real_dev;
	bool is_bond_master = false;

	v = nss_vlan_mgr_create_instance(dev);
	if (!v)
		return NOTIFY_DONE;

	if_num = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_VLAN);
	if (if_num < 0) {
		nss_vlan_mgr_warn("%s: failed to alloc NSS dynamic interface\n", dev->name);
		nss_vlan_mgr_instance_free(v);
		return NOTIFY_DONE;
	}

	if (!nss_register_vlan_if(if_num, NULL, dev, 0, v)) {
		nss_vlan_mgr_warn("%s: failed to register NSS dynamic interface", dev->name);
		if (nss_dynamic_interface_dealloc_node(if_num, NSS_DYNAMIC_INTERFACE_TYPE_VLAN) != NSS_TX_SUCCESS)
			nss_vlan_mgr_warn("%px: Failed to dealloc vlan dynamic interface\n", v);
		nss_vlan_mgr_instance_free(v);
		return NOTIFY_DONE;
	}
	v->nss_if = if_num;

	vlan = vlan_dev_priv(dev);
	real_dev = vlan->real_dev;
	if (is_vlan_dev(real_dev)) {
		vlan = vlan_dev_priv(real_dev);
		real_dev = vlan->real_dev;
	}
	is_bond_master = netif_is_bond_master(real_dev);

#ifdef NSS_VLAN_MGR_PPE_SUPPORT
	if (!is_bond_master)
		ret = nss_vlan_mgr_configure_ppe(v, dev);
	else
		ret = nss_vlan_mgr_bond_configure_ppe(v, real_dev);

	if (ret < 0) {
		nss_vlan_mgr_instance_free(v);
		return NOTIFY_DONE;
	}
#endif

	if (nss_vlan_tx_set_mac_addr_msg(v->nss_if, v->dev_addr) != NSS_TX_SUCCESS) {
		nss_vlan_mgr_warn("%s: failed to set mac_addr msg\n", dev->name);
		nss_vlan_mgr_instance_free(v);
		return NOTIFY_DONE;
	}

	if (nss_vlan_tx_set_mtu_msg(v->nss_if, v->mtu) != NSS_TX_SUCCESS) {
		nss_vlan_mgr_warn("%s: failed to set mtu msg\n", dev->name);
		nss_vlan_mgr_instance_free(v);
		return NOTIFY_DONE;
	}

	vlan_tag = (v->tpid << NSS_VLAN_TPID_SHIFT | v->vid);
	port_if = is_bond_master ? v->bond_ifnum : v->port[0];
	if (nss_vlan_tx_add_tag_msg(v->nss_if, vlan_tag,
				(v->parent ? v->parent->nss_if : port_if),
				port_if) != NSS_TX_SUCCESS) {
		nss_vlan_mgr_warn("%s: failed to add vlan in nss\n", dev->name);
		nss_vlan_mgr_instance_free(v);
		return NOTIFY_DONE;
	}

	spin_lock(&vlan_mgr_ctx.lock);
	list_add(&v->list, &vlan_mgr_ctx.list);
	spin_unlock(&vlan_mgr_ctx.lock);

#ifdef NSS_VLAN_MGR_PPE_SUPPORT
	if (ret == NSS_VLAN_PORT_ROLE_CHANGED) {
		if (!is_bond_master) {
			nss_vlan_mgr_port_role_event(v->port[0], 0);
		} else {
			rcu_read_lock();
			for_each_netdev_in_bond_rcu(real_dev, slave) {
				port = nss_cmn_get_interface_number_by_dev(slave);
				if (!NSS_VLAN_PHY_PORT_CHK(port)) {
					rcu_read_unlock();
					nss_vlan_mgr_warn("%s: %d is not valid physical port\n", slave->name, port);
					return NOTIFY_DONE;
				}
				nss_vlan_mgr_port_role_event(v->port[port - 1], port-1);
				break;
			}
			rcu_read_unlock();
		}
	}
#endif
	return NOTIFY_DONE;
}

/*
 * nss_vlan_mgr_unregister_event()
 */
static int nss_vlan_mgr_unregister_event(struct netdev_notifier_info *info)
{
	struct net_device *dev = netdev_notifier_info_to_dev(info);
	struct nss_vlan_pvt *v = nss_vlan_mgr_instance_find_and_ref(dev);

	/*
	 * Do we have it on record?
	 */
	if (!v)
		return NOTIFY_DONE;

	nss_vlan_mgr_trace("Vlan %s unregsitered. Freeing NSS dynamic interface %d\n", dev->name, v->nss_if);

	/*
	 * Release reference got by "nss_vlan_mgr_instance_find_and_ref"
	 */
	nss_vlan_mgr_instance_deref(v);

	/*
	 * Free instance
	 */
	nss_vlan_mgr_instance_free(v);

	return NOTIFY_DONE;
}

/*
 * nss_vlan_mgr_netdevice_event()
 */
static int nss_vlan_mgr_netdevice_event(struct notifier_block *unused,
				unsigned long event, void *ptr)
{
	struct netdev_notifier_info *info = (struct netdev_notifier_info *)ptr;

	switch (event) {
	case NETDEV_CHANGEADDR:
		return nss_vlan_mgr_changeaddr_event(info);
	case NETDEV_CHANGEMTU:
		return nss_vlan_mgr_changemtu_event(info);
	case NETDEV_REGISTER:
		return nss_vlan_mgr_register_event(info);
	case NETDEV_UNREGISTER:
		return nss_vlan_mgr_unregister_event(info);
	}

	/*
	 * Notify done for all the events we don't care
	 */
	return NOTIFY_DONE;
}

static struct notifier_block nss_vlan_mgr_netdevice_nb __read_mostly = {
	.notifier_call = nss_vlan_mgr_netdevice_event,
};

/*
 * nss_vlan_mgr_get_real_dev()
 *	Get real dev for vlan interface
 */
struct net_device *nss_vlan_mgr_get_real_dev(struct net_device *dev)
{
	struct vlan_dev_priv *vlan;

	if (!dev)
		return NULL;

	vlan = vlan_dev_priv(dev);
	return vlan->real_dev;
}
EXPORT_SYMBOL(nss_vlan_mgr_get_real_dev);

#ifdef NSS_VLAN_MGR_PPE_SUPPORT
/*
 * nss_vlan_mgr_port_vsi_update()
 * 	Update vlan port with the new vsi value
 */
static int nss_vlan_mgr_port_vsi_update(struct nss_vlan_pvt *v, uint32_t new_vsi)
{
	int ret;
	uint32_t old_vsi;

	/*
	 * Delete old ingress vlan translation rule
	 */
	ret = ppe_port_vlan_vsi_set(NSS_VLAN_MGR_SWITCH_ID, v->port[0], v->ppe_svid, v->ppe_cvid, PPE_VSI_INVALID);
	if (ret != SW_OK) {
		nss_vlan_mgr_warn("%px: Failed to delete old ingress translation rule for port: %d, error: %d\n", v, v->port[0], ret);
		return -1;
	}

	/*
	 * Delete old egress vlan translation rule
	 */
	ret = fal_port_vlan_trans_adv_del(NSS_VLAN_MGR_SWITCH_ID, v->port[0],
			FAL_PORT_VLAN_EGRESS, &v->eg_xlt_rule, &v->eg_xlt_action);
	if (ret != SW_OK) {
		nss_vlan_mgr_warn("%px: failed to delete egress vlan translation while joining bridge for port: %d, error: %d\n", v, v->port[0], ret);
		return -1;
	}

	/*
	 * Add new ingress vlan translation rule to use new VSI
	 */
	ret = ppe_port_vlan_vsi_set(NSS_VLAN_MGR_SWITCH_ID, v->port[0], v->ppe_svid, v->ppe_cvid, new_vsi);
	if (ret != SW_OK) {
		nss_vlan_mgr_warn("%px: failed to change ingress vlan translation while joining bridge for port: %d, error: %d\n", v, v->port[0], ret);
		return -1;
	}

	/*
	 * Add new egress vlan translation rule to use new VSI
	 */
	old_vsi = v->eg_xlt_rule.vsi;
	v->eg_xlt_rule.vsi = new_vsi;
	ret = fal_port_vlan_trans_adv_add(NSS_VLAN_MGR_SWITCH_ID, v->port[0],
			FAL_PORT_VLAN_EGRESS, &v->eg_xlt_rule, &v->eg_xlt_action);
	if (ret != SW_OK) {
		nss_vlan_mgr_warn("%px: failed to change egress vlan translation while joining bridge for port: %d. error:%d\n", v, v->port[0], ret);
		v->eg_xlt_rule.vsi = old_vsi;
		return -1;
	}
	return 0;
}

/*
 * nss_vlan_mgr_over_bond_port_vsi_update()
 * 	Update bond slaves with the new vsi value
 */
static int nss_vlan_mgr_over_bond_port_vsi_update(struct net_device *real_dev, struct nss_vlan_pvt *v, uint32_t new_vsi)
{
	int port, ret;
	uint32_t old_vsi;
	struct net_device *slave;

	v->eg_xlt_rule.port_bitmap = 0;
	old_vsi = v->eg_xlt_rule.vsi;

	rcu_read_lock();
	for_each_netdev_in_bond_rcu(real_dev, slave) {
		port = nss_cmn_get_interface_number_by_dev(slave);
		if (!NSS_VLAN_PHY_PORT_CHK(port)) {
			rcu_read_unlock();
			nss_vlan_mgr_warn("%px: bond: %s, slave is not a physical interface\n", v, real_dev->name);
			return -1;
		}
		v->eg_xlt_rule.port_bitmap |= (1 << v->port[port - 1]);

		/*
		 * Delete old ingress vlan translation rule
		 */
		ret = ppe_port_vlan_vsi_set(NSS_VLAN_MGR_SWITCH_ID, v->port[port - 1], v->ppe_svid, v->ppe_cvid, PPE_VSI_INVALID);
		if (ret != SW_OK) {
			rcu_read_unlock();
			nss_vlan_mgr_warn("%px: Failed to delete old ingress translation rule for port: %d\n, error: %d", v, v->port[port - 1], ret);
			return -1;
		}

		/*
		 * Delete old egress vlan translation rule
		 */
		ret = fal_port_vlan_trans_adv_del(NSS_VLAN_MGR_SWITCH_ID, v->port[port - 1],
				FAL_PORT_VLAN_EGRESS, &v->eg_xlt_rule, &v->eg_xlt_action);
		if (ret != SW_OK) {
			rcu_read_unlock();
			nss_vlan_mgr_warn("%px: failed to delete egress vlan translation while joining bridge for port: %d, error:%d\n", v, v->port[port - 1], ret);
			return -1;
		}

		/*
		 * Add new ingress vlan translation rule to use bridge VSI
		 */
		ret = ppe_port_vlan_vsi_set(NSS_VLAN_MGR_SWITCH_ID, v->port[port - 1], v->ppe_svid, v->ppe_cvid, new_vsi);
		if (ret != SW_OK) {
			rcu_read_unlock();
			nss_vlan_mgr_warn("%px: failed to change ingress vlan translation while joining bridge for port: %d, error: %d\n", v, v->port[port - 1], ret);
			return -1;
		}

		/*
		 * Add new egress vlan translation rule to use bridge VSI
		 */
		v->eg_xlt_rule.vsi = new_vsi;
		ret = fal_port_vlan_trans_adv_add(NSS_VLAN_MGR_SWITCH_ID, v->port[port - 1],
				FAL_PORT_VLAN_EGRESS, &v->eg_xlt_rule, &v->eg_xlt_action);
		if (ret != SW_OK) {
			rcu_read_unlock();
			nss_vlan_mgr_warn("%px: failed to change egress vlan translation while joining bridge for port: %d. error:%d\n", v, v->port[port - 1], ret);
			return -1;
		}
		v->eg_xlt_rule.vsi = old_vsi;
	}
	rcu_read_unlock();

	v->eg_xlt_rule.vsi = new_vsi;
	return 0;
}

/*
 * nss_vlan_mgr_over_bond_join_bridge()
 *	Join bond interface to bridge
 */
static int nss_vlan_mgr_over_bond_join_bridge(struct net_device *real_dev, struct nss_vlan_pvt *v, uint32_t bridge_vsi)
{
	int ret;
	uint32_t vlan_vsi;

	vlan_vsi = v->eg_xlt_rule.vsi;
	ret = nss_vlan_mgr_over_bond_port_vsi_update(real_dev, v, bridge_vsi);
	if (ret) {
		nss_vlan_mgr_warn("%px: failed to update bond slaves with the bridge vsi: %d\n", v, bridge_vsi);
		goto return_with_error;
	}

	v->bridge_vsi = bridge_vsi;
	return 0;

return_with_error:
	ret = nss_vlan_mgr_over_bond_port_vsi_update(real_dev, v, vlan_vsi);
	if (ret) {
		nss_vlan_mgr_warn("%px: failed to update bond slaves with the vlan vsi: %d\n", v, vlan_vsi);
	}
	return -1;
}

/*
 * nss_vlan_mgr_over_bond_leave_bridge()
 *	Leave bond interface from bridge
 */
static int nss_vlan_mgr_over_bond_leave_bridge(struct net_device *real_dev, struct nss_vlan_pvt *v)
{
	int port, ret;
	struct net_device *slave;
	uint32_t bridge_vsi;

	bridge_vsi = v->eg_xlt_rule.vsi;
	ret = nss_vlan_mgr_over_bond_port_vsi_update(real_dev, v, v->ppe_vsi);
	if (ret) {
		nss_vlan_mgr_warn("%px: failed to update bond slaves with the vlan vsi: %d\n", v, v->ppe_vsi);
		goto return_with_error;
	}
	v->bridge_vsi = 0;

	rcu_read_lock();
	for_each_netdev_in_bond_rcu(real_dev, slave) {
		port = nss_cmn_get_interface_number_by_dev(slave);
		if (!NSS_VLAN_PHY_PORT_CHK(port)) {
			rcu_read_unlock();
			nss_vlan_mgr_warn("%s: %d is not valid physical port\n", slave->name, port);
			goto return_with_error;
		}

		/*
		 * Set port STP state to forwarding after bond interfaces leave bridge
		 */
		fal_stp_port_state_set(NSS_VLAN_MGR_SWITCH_ID, NSS_VLAN_MGR_STP_ID,
				v->port[port - 1], FAL_STP_FORWARDING);
	}
	rcu_read_unlock();
	return 0;

return_with_error:
	ret = nss_vlan_mgr_over_bond_port_vsi_update(real_dev, v, bridge_vsi);
	if (ret) {
		nss_vlan_mgr_warn("%px: failed to update bond slaves with the bridge vsi: %d\n", v, bridge_vsi);
	}
	return -1;

}
#endif

/*
 * nss_vlan_mgr_join_bridge()
 *	update ingress and egress vlan translation rule to use bridge VSI
 */
int nss_vlan_mgr_join_bridge(struct net_device *dev, uint32_t bridge_vsi)
{
	struct nss_vlan_pvt *v = nss_vlan_mgr_instance_find_and_ref(dev);
#ifdef NSS_VLAN_MGR_PPE_SUPPORT
	struct net_device *real_dev;
	int ret;
#endif

	if (!v)
		return 0;

#ifdef NSS_VLAN_MGR_PPE_SUPPORT
	if ((v->bridge_vsi == bridge_vsi) || v->bridge_vsi) {
		nss_vlan_mgr_warn("%s is already in bridge VSI %d, can't change to %d\n", dev->name, v->bridge_vsi, bridge_vsi);
		nss_vlan_mgr_instance_deref(v);
		return 0;
	}

	/*
	 * If real_dev is bond_master, update for all slaves
	 */
	real_dev = nss_vlan_mgr_get_real_dev(dev);
	if (real_dev && is_vlan_dev(real_dev)) {
		real_dev = nss_vlan_mgr_get_real_dev(real_dev);
	}
	if (real_dev == NULL) {
		nss_vlan_mgr_warn("%px: real dev for the vlan: %s is NULL\n", v, dev->name);
		nss_vlan_mgr_instance_deref(v);
		return -1;
	}

	/*
	 * Check if real_dev is bond master
	 */
	if (netif_is_bond_master(real_dev)) {
		ret = nss_vlan_mgr_over_bond_join_bridge(real_dev, v, bridge_vsi);
		nss_vlan_mgr_instance_deref(v);
		if (ret) {
			nss_vlan_mgr_warn("%px: Bond master: %s failed to join bridge\n", v, real_dev->name);
			return -1;
		}
		return 0;
	}

	/*
	 * real_dev is not bond but a physical device
	 */
	ret = nss_vlan_mgr_port_vsi_update(v, bridge_vsi);
	if (ret) {
		nss_vlan_mgr_warn("%px: failed to join bridge %s\n", v, real_dev->name);
	} else {
		v->bridge_vsi = bridge_vsi;
	}
#endif
	nss_vlan_mgr_instance_deref(v);
	return 0;
}
EXPORT_SYMBOL(nss_vlan_mgr_join_bridge);

/*
 * nss_vlan_mgr_leave_bridge()
 *	update ingress and egress vlan translation rule to restore vlan VSI
 */
int nss_vlan_mgr_leave_bridge(struct net_device *dev, uint32_t bridge_vsi)
{
	struct nss_vlan_pvt *v = nss_vlan_mgr_instance_find_and_ref(dev);
#ifdef NSS_VLAN_MGR_PPE_SUPPORT
	struct net_device *real_dev;
	int ret;
#endif

	if (!v)
		return 0;

#ifdef NSS_VLAN_MGR_PPE_SUPPORT
	if (v->bridge_vsi != bridge_vsi) {
		nss_vlan_mgr_warn("%s is not in bridge VSI %d, ignore\n", dev->name, bridge_vsi);
		nss_vlan_mgr_instance_deref(v);
		return 0;
	}

	/*
	 * If real_dev is bond_master, update for all slaves
	 */
	real_dev = nss_vlan_mgr_get_real_dev(dev);
	if (real_dev && is_vlan_dev(real_dev)) {
		real_dev = nss_vlan_mgr_get_real_dev(real_dev);
	}

	if (!real_dev) {
		nss_vlan_mgr_warn("%px: real dev for the vlan: %s is NULL\n", v, dev->name);
		nss_vlan_mgr_instance_deref(v);
		return -1;
	}

	/*
	 * Check if real_dev is bond master
	 */
	if (netif_is_bond_master(real_dev)) {
		ret = nss_vlan_mgr_over_bond_leave_bridge(real_dev, v);
		nss_vlan_mgr_instance_deref(v);
		if (ret) {
			nss_vlan_mgr_warn("%px: Bond master: %s failed to leave bridge\n", v, real_dev->name);
			return -1;
		}
		return 0;
	}

	/*
	 * real_dev is not bond but a physical device
	 */
	ret = nss_vlan_mgr_port_vsi_update(v, v->ppe_vsi);
	if (ret) {
		nss_vlan_mgr_warn("%px: failed to leave bridge %s\n", v, real_dev->name);
		nss_vlan_mgr_instance_deref(v);
		return -1;
	}
	v->bridge_vsi = 0;

	/*
	 * Set port STP state to forwarding after vlan interface leaves bridge
	 */
	fal_stp_port_state_set(NSS_VLAN_MGR_SWITCH_ID, NSS_VLAN_MGR_STP_ID,
					v->port[0], FAL_STP_FORWARDING);
#endif
	nss_vlan_mgr_instance_deref(v);
	return 0;
}
EXPORT_SYMBOL(nss_vlan_mgr_leave_bridge);

#ifdef NSS_VLAN_MGR_PPE_SUPPORT
/*
 * int nss_vlan_mgr_update_ppe_tpid()
 */
static int nss_vlan_mgr_update_ppe_tpid(void)
{
	fal_tpid_t tpid;

	tpid.mask = FAL_TPID_CTAG_EN | FAL_TPID_STAG_EN;
	tpid.ctpid = vlan_mgr_ctx.ctpid;
	tpid.stpid = vlan_mgr_ctx.stpid;

	if (fal_ingress_tpid_set(NSS_VLAN_MGR_SWITCH_ID, &tpid) || fal_egress_tpid_set(NSS_VLAN_MGR_SWITCH_ID, &tpid)) {
		nss_vlan_mgr_warn("failed to set ctpid %d stpid %d\n", tpid.ctpid, tpid.stpid);
		return -1;
	}

	return 0;
}

/*
 * nss_vlan_mgr_tpid_proc_handler()
 *	Sets customer TPID and service TPID
 */
static int nss_vlan_mgr_tpid_proc_handler(struct ctl_table *ctl,
					  int write, void __user *buffer,
					  size_t *lenp, loff_t *ppos)
{
	int ret = proc_dointvec(ctl, write, buffer, lenp, ppos);
	if (write)
		nss_vlan_mgr_update_ppe_tpid();

	return ret;
}

/*
 * nss_vlan sysctl table
 */
static struct ctl_table nss_vlan_table[] = {
	{
		.procname	= "ctpid",
		.data		= &vlan_mgr_ctx.ctpid,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &nss_vlan_mgr_tpid_proc_handler,
	},
	{
		.procname	= "stpid",
		.data		= &vlan_mgr_ctx.stpid,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &nss_vlan_mgr_tpid_proc_handler,
	},
	{ }
};

/*
 * nss_vlan sysctl dir
 */
static struct ctl_table nss_vlan_dir[] = {
	{
		.procname		= "vlan_client",
		.mode			= 0555,
		.child			= nss_vlan_table,
	},
	{ }
};

/*
 * nss_vlan systel root dir
 */
static struct ctl_table nss_vlan_root_dir[] = {
	{
		.procname		= "nss",
		.mode			= 0555,
		.child			= nss_vlan_dir,
	},
	{ }
};

/*
 * nss_vlan_mgr_add_bond_slave()
 *	Add new slave port to bond_vlan
 */
int nss_vlan_mgr_add_bond_slave(struct net_device *bond_dev,
			struct net_device *slave_dev)
{
	struct nss_vlan_pvt *v;
	int32_t bond_ifnum, vsi = 0, port, bondid = -1;
	int ret;

#if defined(BONDING_SUPPORT)
	bondid = bond_get_id(bond_dev);
#endif
	if (bondid < 0) {
		nss_vlan_mgr_warn("%s: Invalid LAG group id 0x%x\n", bond_dev->name, bondid);
		return -1;
	}
	bond_ifnum = bondid + NSS_LAG0_INTERFACE_NUM;

	/*
	 * find all the vlan_pvt structure which has parent bond_dev
	 */
	spin_lock(&vlan_mgr_ctx.lock);
	list_for_each_entry(v, &vlan_mgr_ctx.list, list) {
		if (v->bond_ifnum != bond_ifnum)
			continue;

		/*
		 * Add Ingress and Egress vlan_vsi
		 */
		port = nss_cmn_get_interface_number_by_dev(slave_dev);
		if (!NSS_VLAN_PHY_PORT_CHK(port)) {
			spin_unlock(&vlan_mgr_ctx.lock);
			nss_vlan_mgr_warn("%s: %d is not valid physical port\n", slave_dev->name, port);
			return -1;
		}
		v->port[port - 1] = port;

		/*
		 * Set correct vsi for the bond slave
		 */
		vsi = v->bridge_vsi ? v->bridge_vsi : v->ppe_vsi;

		/*
		 * Add ingress vlan tranlation table
		 */
		ret = ppe_port_vlan_vsi_set(NSS_VLAN_MGR_SWITCH_ID, v->port[port - 1], v->ppe_svid, v->ppe_cvid, vsi);
		if (ret != SW_OK) {
			spin_unlock(&vlan_mgr_ctx.lock);
			nss_vlan_mgr_warn("bond: %s -> slave: %s: failed to set ingress vlan translation, error: %d\n", bond_dev->name, slave_dev->name, ret);
			return -1;
		}

		/*
		 * Add egress vlan tranlation table
		 */
		v->eg_xlt_rule.port_bitmap |= (1 << v->port[port - 1]);
		ret = fal_port_vlan_trans_adv_add(NSS_VLAN_MGR_SWITCH_ID, v->port[port - 1],
					FAL_PORT_VLAN_EGRESS,
					&v->eg_xlt_rule,
					&v->eg_xlt_action);
		if (ret != SW_OK) {
			spin_unlock(&vlan_mgr_ctx.lock);
			nss_vlan_mgr_warn("bond:%s -> slave:%s failed to set egress vlan translation. error: %d\n", bond_dev->name, slave_dev->name, ret);
			goto delete_ingress_rule;
		}

		/*
		 * Update port role
		 */
		if ((v->ppe_svid != FAL_VLAN_INVALID) &&
				(vlan_mgr_ctx.port_role[v->port[port - 1]] != FAL_QINQ_CORE_PORT)) {
			fal_port_qinq_role_t mode;

			/*
			 * If double tag, we should set physical port as core port
			 */
			vlan_mgr_ctx.port_role[v->port[port - 1]] = FAL_QINQ_CORE_PORT;

			/*
			 * Update port role in PPE
			 */
			mode.mask = FAL_PORT_QINQ_ROLE_INGRESS_EN | FAL_PORT_QINQ_ROLE_EGRESS_EN;
			mode.ingress_port_role = FAL_QINQ_CORE_PORT;
			mode.egress_port_role = FAL_QINQ_CORE_PORT;
			if (fal_port_qinq_mode_set(NSS_VLAN_MGR_SWITCH_ID, v->port[port - 1], &mode)) {
				spin_unlock(&vlan_mgr_ctx.lock);
				nss_vlan_mgr_warn("bond:%s -> slave:%s failed to set %d as core port\n",
						bond_dev->name, slave_dev->name,
						v->port[port - 1]);
				goto delete_egress_rule;
			}
		}
	}
	spin_unlock(&vlan_mgr_ctx.lock);
	return 0;

delete_egress_rule:
	ret = fal_port_vlan_trans_adv_del(NSS_VLAN_MGR_SWITCH_ID, v->port[port - 1],
				FAL_PORT_VLAN_EGRESS,
				&v->eg_xlt_rule, &v->eg_xlt_action);
	if (ret != SW_OK) {
		nss_vlan_mgr_warn("%px: Failed to delete egress translation rule, error: %d\n", v, ret);
	}
delete_ingress_rule:
	ret = ppe_port_vlan_vsi_set(NSS_VLAN_MGR_SWITCH_ID, v->port[port - 1], v->ppe_svid, v->ppe_cvid, PPE_VSI_INVALID);
	if (ret != SW_OK) {
		nss_vlan_mgr_warn("%px: Failed to delete ingress translation rule, error: %d\n", v, ret);
	}

	return -1;
}
EXPORT_SYMBOL(nss_vlan_mgr_add_bond_slave);

/*
 * nss_vlan_mgr_delete_bond_slave()
 *	Delete new slave port from bond_vlan
 */
int nss_vlan_mgr_delete_bond_slave(struct net_device *slave_dev)
{
	struct nss_vlan_pvt *v;
	uint32_t port;
	fal_port_qinq_role_t mode;
	int ret;

	/*
	 * Find port id for the slave
	 */
	port = nss_cmn_get_interface_number_by_dev(slave_dev);
	if (!NSS_VLAN_PHY_PORT_CHK(port)) {
		spin_unlock(&vlan_mgr_ctx.lock);
		nss_vlan_mgr_warn("%s: %d is not valid physical port\n", slave_dev->name, port);
		return -1;
	}

	spin_lock(&vlan_mgr_ctx.lock);
	list_for_each_entry(v, &vlan_mgr_ctx.list, list) {
		if (v->port[port - 1] != port)
			continue;

		/*
		 * Delete ingress vlan tranlation table
		 */
		ret = ppe_port_vlan_vsi_set(NSS_VLAN_MGR_SWITCH_ID, v->port[port - 1], v->ppe_svid, v->ppe_cvid, PPE_VSI_INVALID);
		if (ret != SW_OK) {
			spin_unlock(&vlan_mgr_ctx.lock);
			nss_vlan_mgr_warn("%px: Failed to delete old ingress translation rule, error: %d\n", v, ret);
			return -1;
		}

		/*
		 * Delete egress vlan tranlation table
		 */
		ret = fal_port_vlan_trans_adv_del(NSS_VLAN_MGR_SWITCH_ID, v->port[port - 1],
				FAL_PORT_VLAN_EGRESS,
				&v->eg_xlt_rule, &v->eg_xlt_action);
		if (ret != SW_OK) {
			spin_unlock(&vlan_mgr_ctx.lock);
			nss_vlan_mgr_warn("%px: Failed to delete vlan translation rule, error: %d\n", v, ret);
			return -1;
		}
		v->eg_xlt_rule.port_bitmap = v->eg_xlt_rule.port_bitmap ^ (1 << port);

		/*
		 * Change port role to edge
		 */
		mode.mask = FAL_PORT_QINQ_ROLE_INGRESS_EN | FAL_PORT_QINQ_ROLE_EGRESS_EN;
		mode.ingress_port_role = FAL_QINQ_EDGE_PORT;
		mode.egress_port_role = FAL_QINQ_EDGE_PORT;
		if (fal_port_qinq_mode_set(NSS_VLAN_MGR_SWITCH_ID, port, &mode)) {
			spin_unlock(&vlan_mgr_ctx.lock);
			nss_vlan_mgr_warn("failed to set %d as edge port\n", port);
			return -1;
		}
		vlan_mgr_ctx.port_role[port] = FAL_QINQ_EDGE_PORT;

		/*
		 * Set vlan port
		 */
		v->port[port - 1] = 0;
	}
	spin_unlock(&vlan_mgr_ctx.lock);

	return 0;
}
EXPORT_SYMBOL(nss_vlan_mgr_delete_bond_slave);

/*
 * nss_vlan_mgr_add_vlan_rule()
 *	Add VLAN translation rule in PPE
 */
void nss_vlan_mgr_add_vlan_rule(struct net_device *dev, int bridge_vsi, int vid)
{
	fal_vlan_trans_adv_rule_t eg_xlt_rule;
	fal_vlan_trans_adv_action_t eg_xlt_action;
	int port_id;
	int ret;

	port_id = nss_cmn_get_interface_number_by_dev(dev);
	if (!NSS_VLAN_PHY_PORT_CHK(port_id)) {
		nss_vlan_mgr_warn("%px: %s:%d is not valid physical port\n", dev, dev->name, port_id);
		return;
	}

	/*
	 * Add new ingress vlan translation rule to use bridge VSI
	 */
	ret = ppe_port_vlan_vsi_set(NSS_VLAN_MGR_SWITCH_ID, port_id, FAL_VLAN_INVALID, vid, bridge_vsi);
	if (ret != SW_OK) {
		nss_vlan_mgr_warn("%px: failed to change ingress vlan translation for port: %d, error: %d\n",
				dev, port_id, ret);
		return;
	}

	/*
	 * Add egress vlan translation rule
	 */
	memset(&eg_xlt_rule, 0, sizeof(eg_xlt_rule));
	memset(&eg_xlt_action, 0, sizeof(eg_xlt_action));

	/*
	 * Fields for match
	 */
	eg_xlt_rule.vsi_valid = true;	/* Use vsi as search key */
	eg_xlt_rule.vsi_enable = true;	/* Use vsi as search key */
	eg_xlt_rule.vsi = bridge_vsi;	/* Use vsi as search key */
	eg_xlt_rule.s_tagged = 0x7;	/* Accept tagged/untagged/priority tagged svlan */
	eg_xlt_rule.c_tagged = 0x7;	/* Accept tagged/untagged/priority tagged cvlan */
	eg_xlt_rule.port_bitmap = (1 << port_id); /* Use port as search key */

	/*
	 * Fields for action
	 */
	eg_xlt_action.cvid_xlt_cmd = FAL_VID_XLT_CMD_ADDORREPLACE;
	eg_xlt_action.cvid_xlt = vid;

	ret = fal_port_vlan_trans_adv_add(NSS_VLAN_MGR_SWITCH_ID, port_id,
				FAL_PORT_VLAN_EGRESS, &eg_xlt_rule,
				&eg_xlt_action);
	if (ret != SW_OK) {
		nss_vlan_mgr_warn("%px: Failed to update egress vlan(%x) translation rule for port: %d, error: %d\n",
				dev, vid, port_id, ret);
		/*
		 * Delete ingress vlan translation rule
		 */
		if (ret != SW_ALREADY_EXIST) {
			ppe_port_vlan_vsi_set(NSS_VLAN_MGR_SWITCH_ID, port_id, FAL_VLAN_INVALID, vid, PPE_VSI_INVALID);
		}

		return;
	}

	nss_vlan_mgr_info("%px: Added egress vlan(%x) translation rule for port: %d\n", dev, vid, port_id);
}
EXPORT_SYMBOL(nss_vlan_mgr_add_vlan_rule);

/*
 * nss_vlan_mgr_del_vlan_rule()
 *	Delete VLAN translation rule in PPE
 */
void nss_vlan_mgr_del_vlan_rule(struct net_device *dev, int bridge_vsi, int vid)
{
	fal_vlan_trans_adv_rule_t eg_xlt_rule;	/* VLAN Translation Rule */
	fal_vlan_trans_adv_action_t eg_xlt_action;	/* VLAN Translation Action */
	int port_id;
	int ret;

	port_id = nss_cmn_get_interface_number_by_dev(dev);
	if (!NSS_VLAN_PHY_PORT_CHK(port_id)) {
		nss_vlan_mgr_warn("%px: %s:%d is not valid physical port\n", dev, dev->name, port_id);
		return;
	}

	/*
	 * Delete ingress vlan translation rule
	 */
	ret = ppe_port_vlan_vsi_set(NSS_VLAN_MGR_SWITCH_ID, port_id, FAL_VLAN_INVALID, vid, PPE_VSI_INVALID);
	if (ret != SW_OK) {
		nss_vlan_mgr_warn("%px: failed to delete ingress vlan translation for port: %d, error: %d\n", dev, port_id, ret);
		return;
	}

	/*
	 * Delete egress vlan translation rule
	 */
	memset(&eg_xlt_rule, 0, sizeof(eg_xlt_rule));
	memset(&eg_xlt_action, 0, sizeof(eg_xlt_action));

	/*
	 * Fields for match
	 */
	eg_xlt_rule.vsi_valid = true;	/* Use vsi as search key */
	eg_xlt_rule.vsi_enable = true;	/* Use vsi as search key */
	eg_xlt_rule.vsi = bridge_vsi;	/* Use vsi as search key */
	eg_xlt_rule.s_tagged = 0x7;	/* Accept tagged/untagged/priority tagged svlan */
	eg_xlt_rule.c_tagged = 0x7;	/* Accept tagged/untagged/priority tagged cvlan */
	eg_xlt_rule.port_bitmap = (1 << port_id); /* Use port as search key */

	/*
	 * Fields for action
	 */
	eg_xlt_action.cvid_xlt_cmd = FAL_VID_XLT_CMD_ADDORREPLACE;
	eg_xlt_action.cvid_xlt = vid;

	/*
	 * Delete old egress vlan translation rule
	 */
	ret = fal_port_vlan_trans_adv_del(NSS_VLAN_MGR_SWITCH_ID, port_id,
				FAL_PORT_VLAN_EGRESS, &eg_xlt_rule,
				&eg_xlt_action);
	if (ret != SW_OK) {
		nss_vlan_mgr_warn("%px: Failed to update egress vlan translation of port: %d. error: %d\n", dev, port_id, ret);
		ppe_port_vlan_vsi_set(NSS_VLAN_MGR_SWITCH_ID, port_id, FAL_VLAN_INVALID, vid, bridge_vsi);
		return;
	}

	nss_vlan_mgr_info("%px: deleted egress vlan(%x) translation rule for port: %d\n", dev, vid, port_id);
}
EXPORT_SYMBOL(nss_vlan_mgr_del_vlan_rule);
#endif

/*
 * nss_vlan_mgr_init_module()
 *	vlan_mgr module init function
 */
int __init nss_vlan_mgr_init_module(void)
{
#ifdef NSS_VLAN_MGR_PPE_SUPPORT
	int idx;
#endif

	INIT_LIST_HEAD(&vlan_mgr_ctx.list);
	spin_lock_init(&vlan_mgr_ctx.lock);

	vlan_mgr_ctx.ctpid = ETH_P_8021Q;
	vlan_mgr_ctx.stpid = ETH_P_8021Q;

#ifdef NSS_VLAN_MGR_PPE_SUPPORT
	vlan_mgr_ctx.sys_hdr = register_sysctl_table(nss_vlan_root_dir);
	if (!vlan_mgr_ctx.sys_hdr) {
		nss_vlan_mgr_warn("Unabled to register sysctl table for vlan manager\n");
		return -EFAULT;
	}

	if (nss_vlan_mgr_update_ppe_tpid()) {
		unregister_sysctl_table(vlan_mgr_ctx.sys_hdr);
		return -EFAULT;
	}

	for (idx = 0; idx < NSS_VLAN_PHY_PORT_NUM; idx++) {
		vlan_mgr_ctx.port_role[idx] = FAL_QINQ_EDGE_PORT;
	}
#endif
	register_netdevice_notifier(&nss_vlan_mgr_netdevice_nb);

	nss_vlan_mgr_info("Module (Build %s) loaded\n", NSS_CLIENT_BUILD_ID);
	return 0;
}

/*
 * nss_vlan_mgr_exit_module()
 *	vlan_mgr module exit function
 */
void __exit nss_vlan_mgr_exit_module(void)
{
	unregister_netdevice_notifier(&nss_vlan_mgr_netdevice_nb);

#ifdef NSS_VLAN_MGR_PPE_SUPPORT
	if (vlan_mgr_ctx.sys_hdr)
		unregister_sysctl_table(vlan_mgr_ctx.sys_hdr);
#endif
	nss_vlan_mgr_info("Module unloaded\n");
}

module_init(nss_vlan_mgr_init_module);
module_exit(nss_vlan_mgr_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS vlan manager");
