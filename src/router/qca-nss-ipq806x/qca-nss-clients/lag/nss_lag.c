/*
 **************************************************************************
 * Copyright (c) 2016-2017, 2020, The Linux Foundation. All rights reserved.
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
 * nss_lag.c
 *	HLOS to NSS LAG Interface manager
 */
#include <linux/etherdevice.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <net/bonding.h>
#if defined(NSS_LAG_PPE_SUPPORT)
#include <nss_vlan_mgr.h>
#include <fal/fal_trunk.h>
#endif

#include <nss_api_if.h>

/*
 * Compile messages for dynamic enable/disable
 */
#if defined(CONFIG_DYNAMIC_DEBUG)
#define nss_lag_warn(s, ...) \
		pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_lag_info(s, ...) \
		pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#else /* CONFIG_DYNAMIC_DEBUG */
/*
 * Statically compile messages at different levels
 */
#if (NSS_LAG_MGR_DEBUG_LEVEL < 2)
#define nss_lag_warn(s, ...)
#else
#define nss_lag_warn(s, ...) \
		pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_LAG_MGR_DEBUG_LEVEL < 3)
#define nss_lag_info(s, ...)
#else
#define nss_lag_info(s, ...) \
		pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif
#endif /* CONFIG_DYNAMIC_DEBUG */

#define NSS_LAG_MAX_BOND_DEVICES 4
#define NSS_LAG_MAX_PPE_BOND_DEVICES 2
#define NSS_LAG_MAX_SLAVES_PER_BONDID 16
#define NSS_LAG_BOND_DEV_STATE_ACTIVE 1
#define NSS_LAG_BOND_DEV_STATE_INACTIVE 2

/*
 * LAG manager private structure
 */
struct nss_bond_pvt {
	int32_t bond_dev_state;	/* Active or Inactive */
	int32_t bondid;		/* Bond ID */
	uint32_t members;	/* Enslaved devices:
				 *   bit 0 - eth0,
				 *   bit1 - eth1 ...
				 */
	struct net_device *slaves[NSS_LAG_MAX_SLAVES_PER_BONDID];
} bond_pvt[NSS_LAG_MAX_BOND_DEVICES];

DEFINE_SPINLOCK(nss_lag_spinlock);

/*
 * NSS Context for LAG function
 */
static void *nss_lag_nss_context;	/* Registration for LAG */

#if defined(NSS_LAG_PPE_SUPPORT)
/*
 * nss_lag_update_ppe()
 *	Update and/or Configure LAG in Hardware when there is change in bonding
 *	slave's state
 */
static int nss_lag_update_ppe(int32_t bondid, int32_t slave_ifnum,
		enum nss_lag_state_change_ev slave_event)
{
	struct nss_bond_pvt *pvt;
	int32_t enable = 1;

	if (bondid > NSS_LAG_MAX_PPE_BOND_DEVICES) {
		nss_lag_warn("bondid is wrong: bondid=%d\n", bondid);
		return -1;
	}

	if (slave_ifnum > NSS_MAX_PHYSICAL_INTERFACES) {
		nss_lag_warn("HW LAG Supports only Physical ports: slave=%d\n",
				slave_ifnum);
		return -1;
	}

	spin_lock(&nss_lag_spinlock);

	pvt = &bond_pvt[bondid];

	if (slave_event == NSS_LAG_ENSLAVE)
		pvt->members |= (1 << (slave_ifnum - 1));
	else
		pvt->members &= ~(1 << (slave_ifnum - 1));

	if (pvt->members)
		pvt->bond_dev_state = NSS_LAG_BOND_DEV_STATE_ACTIVE;
	else {
		pvt->bond_dev_state = NSS_LAG_BOND_DEV_STATE_INACTIVE;
		enable = 0;
	}

	spin_unlock(&nss_lag_spinlock);

	return fal_trunk_group_set(0, bondid, enable, pvt->members);
}
#endif

/*
 * nss_lag_send_lag_state()
 *	Send the currnet LAG state of a physical interface that has changed
 *	state in the bonding driver.
 */
static nss_tx_status_t nss_lag_send_lag_state(struct nss_ctx_instance *nss_ctx,
		int32_t bondid, struct net_device *slave,
		enum nss_lag_state_change_ev slave_state)
{
	int32_t lagid = 0;
	int32_t slave_ifnum;
	nss_tx_status_t nss_tx_status;

	nss_lag_info("Send LAG update for: %px (%s)\n", slave, slave->name);

	lagid = bondid + NSS_LAG0_INTERFACE_NUM;

	slave_ifnum = nss_cmn_get_interface_number_by_dev(slave);
	if (slave_ifnum < 0)
		return NSS_TX_FAILURE_BAD_PARAM;

	nss_lag_info("slave_state=%d, slave_ifnum=%d, bondid=%d\n",
			slave_state, slave_ifnum, bondid);

	nss_tx_status = nss_lag_tx_slave_state(lagid, slave_ifnum, slave_state);
	if (nss_tx_status != NSS_TX_SUCCESS) {
		nss_lag_warn("%px: Send LAG update failed, status: %d\n", slave,
				nss_tx_status);
		return NSS_TX_FAILURE;
	}
	nss_lag_info("%px: Send LAG update success\n", slave);

#if defined(NSS_LAG_PPE_SUPPORT)
	ret = nss_lag_update_ppe(bondid, slave_ifnum, slave_state);
	if (ret)
		nss_lag_warn("%px: Couldn't update PPE: ret =  %d\n",
				slave, ret);
#endif
	return NSS_TX_SUCCESS;
}

/*
 * nss_lag_event_cb()
 *	Handle LAG event from the NSS driver
 */
static void nss_lag_event_cb(void *if_ctx, struct nss_lag_msg *msg)
{
	/*
	 * TODO: Figure out if there is anything we need to do here,
	 * the old CM did nothing..
	 */
	switch (msg->cm.type) {
	default:
		nss_lag_warn("Unknown LAG event from NSS: %d", msg->cm.type);
		break;
	}
}

/*
 * nss_lag_update_slave()
 *	Monitor bonding slaves and send updates to hardware
 */
static int nss_lag_update_slave(struct netdev_notifier_info *info)
{
	struct net_device *slave_dev = netdev_notifier_info_to_dev(info);
	struct net_device *bond_dev;
	int32_t bondid = -1;
	uint32_t slave_ifnum, i;
	struct nss_bond_pvt *pvt;

	if (!netif_is_bond_slave(slave_dev))
		return NOTIFY_DONE;

	/*
	 * Only care about interfaces known by NSS
	 */
	slave_ifnum = nss_cmn_get_interface_number_by_dev(slave_dev);
	if (slave_ifnum < 0)
		return NOTIFY_DONE;

	bond_dev = netdev_master_upper_dev_get(slave_dev);
	if (bond_dev) {

		/* bond_dev is valid when slave is Enslaved */

		nss_lag_info("Interface %s added in LAG ID=%d: %s\n",
				slave_dev->name, bondid, bond_dev->name);

		/*
		 * Figure out the aggregation id of this slave
		 */
		bondid = bond_get_id(bond_dev);
		if ((bondid < 0) || (bondid >= NSS_LAG_MAX_BOND_DEVICES)) {
			nss_lag_warn("Invalid LAG group id 0x%x\n", bondid);
			return NOTIFY_DONE;
		}

		spin_lock(&nss_lag_spinlock);

		pvt = &bond_pvt[bondid];
		pvt->bondid = bondid;

		for (i = 0; i < NSS_LAG_MAX_SLAVES_PER_BONDID; i++) {
			if (!pvt->slaves[i])
				break;
		}

		if (i == NSS_LAG_MAX_SLAVES_PER_BONDID) {
			spin_unlock(&nss_lag_spinlock);
			nss_lag_warn("More than max %d slaves are added\n",
					NSS_LAG_MAX_SLAVES_PER_BONDID);
			return NOTIFY_DONE;
		}

		pvt->slaves[i] = slave_dev;
		spin_unlock(&nss_lag_spinlock);

		if (nss_lag_send_lag_state(nss_lag_nss_context,
			bondid, slave_dev, NSS_LAG_ENSLAVE)) {
			nss_lag_warn("Enslave %s dev failed\n",
					slave_dev->name);
			return NOTIFY_BAD;
		}

#if defined(NSS_LAG_PPE_SUPPORT)
		if (nss_vlan_mgr_add_bond_slave(bond_dev, slave_dev))
			nss_lag_warn("%px: Adding vlan for %s dev failed\n", slave_dev, slave_dev->name);
#endif
		return NOTIFY_DONE;
	}

	/* Slave is removed from bond */
	spin_lock(&nss_lag_spinlock);
	for (i = 0; i < NSS_LAG_MAX_BOND_DEVICES; i++) {
		int j;

		pvt = &bond_pvt[i];
		for (j = 0; j < NSS_LAG_MAX_SLAVES_PER_BONDID; j++)
			if (pvt->slaves[j] == slave_dev) {
				pvt->slaves[j] = NULL;
				bondid = pvt->bondid;
			}

		if (bondid != -1)
			break;
	}
	spin_unlock(&nss_lag_spinlock);

	if (bondid == -1)
		return NOTIFY_DONE;

	nss_lag_info("Interface %s removed from LAG ID=%d\n",
			slave_dev->name, bondid);

	if (nss_lag_send_lag_state(nss_lag_nss_context,
		bondid, slave_dev, NSS_LAG_RELEASE)) {
		nss_lag_warn("Deslave %s dev failed\n", slave_dev->name);
		return NOTIFY_BAD;
	}

#if defined(NSS_LAG_PPE_SUPPORT)
	if (nss_vlan_mgr_delete_bond_slave(slave_dev))
		nss_lag_warn("%px: Delete vlan for %s dev failed\n", slave_dev, slave_dev->name);
#endif
	return NOTIFY_DONE;
}

/*
 * nss_lag_netdevice_event()
 *	Handle events received from network stack
 */
static int nss_lag_netdevice_event(struct notifier_block *unused,
		unsigned long event, void *ptr)
{
	switch (event) {
	case NETDEV_CHANGEUPPER:
		return nss_lag_update_slave((struct netdev_notifier_info *)ptr);
	}
	return NOTIFY_DONE;
}

/* register netdev notifier callback */
static struct notifier_block nss_lag_netdevice __read_mostly = {
	.notifier_call = nss_lag_netdevice_event,
};

/*
 * nss_lag_init()
 *	Initialize NSS LAG client
 */
int __init nss_lag_init(void)
{
	int ret;
	/*
	 * Register Link Aggregation interfaces with NSS driver
	 * assuming all four LAGs are on same interface.
	 */
	nss_lag_nss_context = nss_register_lag_if(NSS_LAG0_INTERFACE_NUM, NULL,
						nss_lag_event_cb, NULL);
	if (!nss_lag_nss_context) {
		nss_lag_info("Failed to register LAG interface with NSS\n");
		return -1;
	}

	(void)nss_register_lag_if(NSS_LAG1_INTERFACE_NUM, NULL,
			nss_lag_event_cb, NULL);
	(void)nss_register_lag_if(NSS_LAG2_INTERFACE_NUM, NULL,
			nss_lag_event_cb, NULL);
	(void)nss_register_lag_if(NSS_LAG3_INTERFACE_NUM, NULL,
			nss_lag_event_cb, NULL);

	ret = register_netdevice_notifier(&nss_lag_netdevice);
	if (ret) {
		nss_lag_warn("Failed to register NETDEV notifier, error=%d\n",
				ret);

		nss_unregister_lag_if(NSS_LAG0_INTERFACE_NUM);
		nss_unregister_lag_if(NSS_LAG1_INTERFACE_NUM);
		nss_unregister_lag_if(NSS_LAG2_INTERFACE_NUM);
		nss_unregister_lag_if(NSS_LAG3_INTERFACE_NUM);
	} else
		nss_lag_info("LAG Manager Installed\n");

	return ret;
}

/*
 * nss_lag_exit()
 *	Cleanup NSS LAG client and exit
 */
void __exit nss_lag_exit(void)
{
	unregister_netdevice_notifier(&nss_lag_netdevice);

	/*
	 * Unregister Link Aggregation interfaces with NSS driver
	 */
	nss_unregister_lag_if(NSS_LAG0_INTERFACE_NUM);
	nss_unregister_lag_if(NSS_LAG1_INTERFACE_NUM);
	nss_unregister_lag_if(NSS_LAG2_INTERFACE_NUM);
	nss_unregister_lag_if(NSS_LAG3_INTERFACE_NUM);

	nss_lag_info("LAG Manager Removed\n");
}

module_init(nss_lag_init);
module_exit(nss_lag_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS LAG Client");
