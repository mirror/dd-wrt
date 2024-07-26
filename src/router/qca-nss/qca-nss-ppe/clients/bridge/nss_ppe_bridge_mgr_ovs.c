/*
 **************************************************************************
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
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
 * nss_ppe_bridge_mgr_ovs.c
 *	Handle OVS bridge notifications.
 *
 * TODO: This file needs to be reinvestigated when OVS is enabled/tested.
 */
#include <linux/netdevice.h>
#include <linux/notifier.h>
#include <ovsmgr.h>
#include <ppe_drv_iface.h>
#include <nss_ppe_vlan_mgr.h>

#include "nss_ppe_bridge_mgr.h"

/*
 * nss_ppe_bridge_mgr_ovs_handle_port_event()
 *	Handle OVS bridge port events
 */
static int nss_ppe_bridge_mgr_ovs_handle_port_event(struct ovsmgr_notifiers_info *ovs_info, unsigned long event)
{
	struct ovsmgr_dp_port_info *port;
	struct net_device *master_dev, *dev;
	int err;

	port = ovs_info->port;
	if (!port || !port->master || !port->dev) {
		nss_ppe_bridge_mgr_warn("%px: Invalid ovs_info\n", ovs_info);
		return -EINVAL;
	}

	master_dev = port->master;
	dev = port->dev;

	/*
	 * add port to the bridge.
	 */
	if (event == OVSMGR_DP_PORT_ADD) {
		nss_ppe_bridge_mgr_trace("%px: Interface %s joining bridge %s\n", ovs_info, dev->name, master_dev->name);

		err = nss_ppe_bridge_mgr_join_bridge(dev, master_dev);
		if (err) {
			nss_ppe_bridge_mgr_warn("%px: Interface %s failed to join bridge %s\n", ovs_info, dev->name, master_dev->name);
			return err;
		}

		return 0;
	}

	/*
	 * delete port from bridge.
	 */
	nss_ppe_bridge_mgr_trace("%px: Interface %s leaving bridge %s\n", ovs_info, dev->name, master_dev->name);

	err = nss_ppe_bridge_mgr_leave_bridge(dev, master_dev);
	if (err) {
		nss_ppe_bridge_mgr_warn("%px: Interface %s failed to leave bridge %s\n", ovs_info, dev->name, master_dev->name);
		return err;
	}

	return 0;
}

/*
 * nss_ppe_bridge_mgr_ovs_handle_vlan_event()
 *	Handle VLAN events OVS bridge port
 */
static void nss_ppe_bridge_mgr_ovs_handle_vlan_event(struct ovsmgr_notifiers_info *ovs_info, unsigned long event)
{
	struct ovsmgr_dp_port_vlan_info *vlan;
	struct nss_ppe_bridge_mgr_pvt *b_pvt;
	struct net_device *master_dev, *dev;

	vlan = ovs_info->vlan;
	if (!vlan || !vlan->master || !vlan->dev) {
		nss_ppe_bridge_mgr_warn("%px: Invalid ovs_info\n", ovs_info);
		return;
	}

	master_dev = vlan->master;
	dev = vlan->dev;

	/*
	 * Check if upper_dev is a known bridge.
	 */
	b_pvt = nss_ppe_bridge_mgr_find_instance(master_dev);
	if (!b_pvt) {
		nss_ppe_bridge_mgr_warn("%px: Couldn't find bridge instance for master: %s\n", vlan, master_dev->name);
		return;
	}

	/*
	 * add VLAN in bridge.
	 */
	if (event == OVSMGR_DP_VLAN_ADD) {
		nss_ppe_bridge_mgr_trace("%px: VLAN = %d, add on port %s, bridge %s\n",
				b_pvt, vlan->vh.h_vlan_TCI, dev->name, master_dev->name);

		nss_ppe_vlan_mgr_add_vlan_rule(dev, b_pvt->iface, vlan->vh.h_vlan_TCI);
		return;
	}

	/*
	 * delete VLAN from bridge.
	 */
	nss_ppe_bridge_mgr_trace("%px: VLAN = %d, delete on port %s, bridge %s\n",
					b_pvt, vlan->vh.h_vlan_TCI, dev->name, master_dev->name);
	nss_ppe_vlan_mgr_del_vlan_rule(dev, b_pvt->iface, vlan->vh.h_vlan_TCI);
}

/*
 * nss_ppe_bridge_mgr_ovs_notifier_callback()
 *	Netdevice notifier callback to inform us of change of state of a netdevice
 */
static int nss_ppe_bridge_mgr_ovs_notifier_callback(struct notifier_block *nb, unsigned long event, void *data)
{
	struct ovsmgr_notifiers_info *ovs_info = (struct ovsmgr_notifiers_info *)data;

	nss_ppe_bridge_mgr_info("OVS notifier event: %lu\n", event);

	switch (event) {
	case OVSMGR_DP_BR_ADD:
		nss_ppe_bridge_mgr_register_br(ovs_info->dev);
		break;
	case OVSMGR_DP_BR_DEL:
		nss_ppe_bridge_mgr_unregister_br(ovs_info->dev);
		break;
	case OVSMGR_DP_PORT_ADD:
	case OVSMGR_DP_PORT_DEL:
		nss_ppe_bridge_mgr_ovs_handle_port_event(ovs_info, event);
		break;
	case OVSMGR_DP_VLAN_ADD:
	case OVSMGR_DP_VLAN_DEL:
		nss_ppe_bridge_mgr_ovs_handle_vlan_event(ovs_info, event);
		break;
	}

	return NOTIFY_DONE;
}

/*
 * struct notifier_block nss_ppe_bridge_mgr_ovs_notifier
 *	Registration for OVS events
 */
static struct notifier_block ovs_notifier __read_mostly = {
	.notifier_call = nss_ppe_bridge_mgr_ovs_notifier_callback,
};

/*
 * nss_ppe_bridge_mgr_is_ovs_port()
 *	Return true if dev is an OVS port.
 */
int nss_ppe_bridge_mgr_is_ovs_port(struct net_device *dev)
{
	if (dev->priv_flags & IFF_OVS_DATAPATH) {
		return true;
	}

	return false;
}

/*
 * nss_ppe_bridge_mgr_ovs_exit()
 *	Cleanup OVS bridge handlers.
 */
void nss_ppe_bridge_mgr_ovs_exit(void)
{
	ovsmgr_notifier_unregister(&ovs_notifier);
}

/*
 * nss_ppe_bridge_mgr_exit_module()
 *	Initialize OVS bridge handlers.
 */
void nss_ppe_bridge_mgr_ovs_init(void)
{
	ovsmgr_notifier_register(&ovs_notifier);
}
