/*
 **************************************************************************
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
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

#include <nss_api_if.h>
#include <nss_cmn.h>
#include <net/netfilter/nf_conntrack_dscpremark_ext.h>
#include "nss_mirred.h"
#include "nss_igs.h"
#include "nss_ifb.h"

/*
 * TODO: Current implementation only supports one IFB interface to be mapped with
 * any one interface at a time. This has to be changed to support the mapping of
 * an IFB device to multiple interfaces.
 */
static LIST_HEAD(nss_ifb_list);			/* List of IFB and its mapped interface */
static DEFINE_SPINLOCK(nss_ifb_list_lock);	/* Lock for the ifb list */

/*
 * nss_ifb_msg_response
 *	NSS firmware message response structure.
 */
static struct nss_ifb_msg_response {
	struct semaphore sem;
	wait_queue_head_t wq;
	enum nss_cmn_response response;
	bool cond;
} msg_response;

/*
 * nss_ifb_igs_ip_pre_routing_hook()
 *	Copy class-id to Linux CT structure.
 *
 * Copy class-id from tc_index field of skb in ingress QoS fields inside
 * DSCP CT extention structure.
 */
unsigned int nss_ifb_igs_ip_pre_routing_hook(void *priv, struct sk_buff *skb,
		 const struct nf_hook_state *state)
{
	struct nf_conn *ct;
	struct nf_ct_dscpremark_ext *dscpcte;
	enum ip_conntrack_info ctinfo;

	if (unlikely(!skb))
		return NF_ACCEPT;

	/*
	 * Return if ingress qostag value (saved in tc_index field) is 0.
	 */
	if (likely(!skb->tc_index))
		return NF_ACCEPT;

	ct = nf_ct_get(skb, &ctinfo);
	if (!ct)
		return NF_ACCEPT;

	spin_lock_bh(&ct->lock);
	dscpcte = nf_ct_dscpremark_ext_find(ct);
	if (!dscpcte) {
		spin_unlock_bh(&ct->lock);
		return NF_ACCEPT;
	}

	/*
	 * Copy ingress qostag value (saved in tc_index) to ingress
	 * qostag fields of DSCP CT extension structure.
	 */
	if (IP_CT_DIR_ORIGINAL == CTINFO2DIR(ctinfo)) {
		dscpcte->igs_flow_qos_tag = skb->tc_index;
	} else {
		dscpcte->igs_reply_qos_tag = skb->tc_index;
	}
	spin_unlock_bh(&ct->lock);

	/*
	 * Reset the tc_index field as it no longer required.
	 */
	skb->tc_index = 0;
	return NF_ACCEPT;
}

/*
 * nss_ifb_list_del()
 *	API to delete member in ifb list.
 */
void nss_ifb_list_del(struct nss_ifb_info *ifb_info)
{
	spin_lock_bh(&nss_ifb_list_lock);
	list_del(&ifb_info->map_list);
	spin_unlock_bh(&nss_ifb_list_lock);
}

/*
 * nss_ifb_list_add()
 *	API to add member in ifb list.
 */
static void nss_ifb_list_add(struct nss_ifb_info *ifb_info)
{
	spin_lock_bh(&nss_ifb_list_lock);
	list_add(&(ifb_info->map_list), &nss_ifb_list);
	spin_unlock_bh(&nss_ifb_list_lock);
}

/*
 * nss_ifb_is_mapped()
 *	Returns the map status of the given ifb bind structure.
 */
bool nss_ifb_is_mapped(struct nss_ifb_info *ifb_info)
{
	bool is_mapped;

	spin_lock_bh(&nss_ifb_list_lock);
	is_mapped = ifb_info->is_mapped;
	spin_unlock_bh(&nss_ifb_list_lock);
	return is_mapped;
}

/*
 * nss_ifb_config_msg_init()
 *	Initialize IFB configure interface's message.
 */
static void nss_ifb_config_msg_init(struct nss_if_msg *ncm, uint16_t if_num,
		 uint32_t type,  uint32_t len, void *cb, void *app_data)
{
	nss_cmn_msg_init(&ncm->cm, if_num, type, len, cb, app_data);
}

/*
 * nss_ifb_clear_config_cb()
 *	CLEAR configure handler for an IFB mapped interface.
 */
static void nss_ifb_clear_config_cb(void *app_data, struct nss_if_msg *nim)
{
	struct nss_ifb_info *ifb_info = (struct nss_ifb_info *)app_data;
	bool ret;

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_igs_error("Response error: %d\n", nim->cm.response);
		return;
	}

	do {
		ret = spin_trylock_bh(&nss_ifb_list_lock);
	} while (!ret);
	ifb_info->is_mapped = false;
	spin_unlock_bh(&nss_ifb_list_lock);
}

/*
 * nss_ifb_async_cb()
 *	IFB asynchronous handler for an IFB mapped interface.
 */
static void nss_ifb_async_cb(void *app_data, struct nss_if_msg *nim)
{
	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_igs_error("Response error: %d\n", nim->cm.response);
	}
}

/*
 * nss_ifb_wake_up_cb()
 *	IFB wake up handler for an IFB mapped interface.
 */
static void nss_ifb_wake_up_cb(void *app_data, struct nss_if_msg *nim)
{
	msg_response.response = nim->cm.response;
	msg_response.cond = 0;
	wake_up(&msg_response.wq);
}

/*
 * nss_ifb_config_msg_fill()
 *	Fill the IFB configure message.
 */
static bool nss_ifb_config_msg_fill(struct nss_if_msg *nim_ptr, struct net_device *dev,
		int32_t ifb_num, enum nss_ifb_if_config config, void *cb)
{
	uint32_t if_src_num;

	if (netif_is_ifb_dev(dev)) {
		if_src_num = nss_cmn_get_interface_number_by_dev_and_type(dev, NSS_DYNAMIC_INTERFACE_TYPE_IGS);
		if (if_src_num < 0) {
			nss_igs_error("invalid IFB device %s\n", dev->name);
			return -1;
		}
	} else {
		if_src_num = nss_cmn_get_interface_number_by_dev(dev);
		if (if_src_num < 0) {
			nss_igs_error("invalid device %s\n", dev->name);
			return -1;
		}
	}

	switch (config) {
	case NSS_IFB_SET_IGS_NODE:
		nss_ifb_config_msg_init(nim_ptr, if_src_num, NSS_IF_SET_IGS_NODE,
			sizeof(struct nss_if_igs_config), nss_ifb_wake_up_cb, cb);
		nim_ptr->msg.config_igs.igs_num = ifb_num;
		break;
	case NSS_IFB_CLEAR_IGS_NODE:
		nss_ifb_config_msg_init(nim_ptr, if_src_num, NSS_IF_CLEAR_IGS_NODE,
			sizeof(struct nss_if_igs_config), nss_ifb_clear_config_cb, cb);
		nim_ptr->msg.config_igs.igs_num = ifb_num;
		break;
	case NSS_IFB_SET_NEXTHOP:
		nss_ifb_config_msg_init(nim_ptr, if_src_num, NSS_IF_SET_NEXTHOP,
			sizeof(struct nss_if_set_nexthop), nss_ifb_wake_up_cb, cb);
		nim_ptr->msg.set_nexthop.nexthop = ifb_num;
		break;
	case NSS_IFB_RESET_NEXTHOP:
		nss_ifb_config_msg_init(nim_ptr, if_src_num, NSS_IF_RESET_NEXTHOP,
			sizeof(struct nss_if_set_nexthop), nss_ifb_async_cb, cb);
		nim_ptr->msg.set_nexthop.nexthop = ifb_num;
		break;
	case NSS_IFB_OPEN:
		nss_ifb_config_msg_init(nim_ptr, if_src_num, NSS_IF_OPEN,
			sizeof(struct nss_if_open), nss_ifb_async_cb, cb);
		/*
		 * Reset the elements of interface's open configuration.
		 */
		memset (&nim_ptr->msg.open, 0, sizeof(struct nss_if_open));
		break;
	case NSS_IFB_CLOSE:
		nss_ifb_config_msg_init(nim_ptr, if_src_num, NSS_IF_CLOSE,
			sizeof(struct nss_if_close), nss_ifb_async_cb, cb);
		/*
		 * Reset the elements of interface's close configuration.
		 */
		memset (&nim_ptr->msg.close, 0, sizeof(struct nss_if_close));
		break;
	}

	return 0;
}

/*
 * nss_ifb_config_msg_tx()
 *	Send IFB configure message to an IFB mapped interface.
 */
int32_t nss_ifb_config_msg_tx(struct net_device *dev, int32_t ifb_num,
		 enum nss_ifb_if_config config, void *cb)
{
	struct nss_if_msg nim;
	int32_t ret;

	if ((ret = nss_ifb_config_msg_fill(&nim, dev, ifb_num, config, cb))) {
		nss_igs_error("Error in setting up IFB %d config message\n", config);
		return -1;
	}

	ret = nss_if_tx_msg(nss_igs_get_context(), &nim);
	if (ret != NSS_TX_SUCCESS) {
		nss_igs_error("failed to send config message\n");
		return -1;
	}

	return 0;
}

/*
 * nss_ifb_config_msg_tx_sync()
 *	Send IFB configure message to an IFB mapped interface and wait for the response.
 */
int32_t nss_ifb_config_msg_tx_sync(struct net_device *dev, int32_t ifb_num,
		 enum nss_ifb_if_config config, void *cb)
{
	struct nss_if_msg nim;
	int32_t ret;

	if ((ret = nss_ifb_config_msg_fill(&nim, dev, ifb_num, config, cb))) {
		nss_igs_error("Error in setting up IFB %d config message\n", config);
		return -1;
	}

	down(&msg_response.sem);
	ret = nss_if_tx_msg(nss_igs_get_context(), &nim);
	if (ret != NSS_TX_SUCCESS) {
		up(&msg_response.sem);
		nss_igs_error("failed to send config message\n");
		return -1;
	}

	msg_response.cond = 1;
	if (!wait_event_timeout(msg_response.wq, msg_response.cond == 0, NSS_IFB_MSG_TIMEOUT)) {
		nss_igs_error("config for attach interface msg timeout\n");
		up(&msg_response.sem);
		return -1;
	} else if (msg_response.response != NSS_CMN_RESPONSE_ACK) {
		up(&msg_response.sem);
		nss_igs_error("config for attach interface msg return with response: %d\n", msg_response.response);
		return -1;
	}

	up(&msg_response.sem);
	return 0;
}

/*
 * nss_ifb_reset_nexthop()
 *	Send RESET NEXTHOP configure message to an IFB mapped interface.
 */
bool nss_ifb_reset_nexthop(struct nss_ifb_info *ifb_info)
{
	int32_t if_num;

	spin_lock_bh(&nss_ifb_list_lock);
	if (!(ifb_info->is_mapped)) {
		nss_igs_info("%s IFB device mapped flag is not set\n", ifb_info->ifb_dev->name);
		spin_unlock_bh(&nss_ifb_list_lock);
		return true;
	}

	/*
	 * Send RESET NEXTHOP config message to the mapped interface.
	 */
	if_num = nss_cmn_get_interface_number_by_dev_and_type(ifb_info->ifb_dev, NSS_DYNAMIC_INTERFACE_TYPE_IGS);
	if (if_num < 0) {
		nss_igs_error("No %s IFB device found in NSS firmware\n", ifb_info->ifb_dev->name);
	}

	if (nss_ifb_config_msg_tx(ifb_info->map_dev, if_num, NSS_IFB_RESET_NEXTHOP, NULL) < 0) {
		nss_igs_error("Sending RESET NEXTHOP config to %s dev failed\n", ifb_info->map_dev->name);
		spin_unlock_bh(&nss_ifb_list_lock);
		return false;
	}
	spin_unlock_bh(&nss_ifb_list_lock);
	return true;
}

/*
 * nss_ifb_down()
 *	Send interface's DOWN configure message to an IFB interface.
 */
bool nss_ifb_down(struct nss_ifb_info *ifb_info)
{
	int32_t ifb_num;

	spin_lock_bh(&nss_ifb_list_lock);
	if (!(ifb_info->is_mapped)) {
		nss_igs_info("%s IFB device mapped flag is not set\n", ifb_info->ifb_dev->name);
		spin_unlock_bh(&nss_ifb_list_lock);
		return true;
	}

	/*
	 * Send interface's DOWN config message to an IFB interface.
	 */
	ifb_num = nss_cmn_get_interface_number_by_dev_and_type(ifb_info->ifb_dev, NSS_DYNAMIC_INTERFACE_TYPE_IGS);
	if (ifb_num < 0) {
		nss_igs_error("No %s IFB device found in NSS FW\n", ifb_info->ifb_dev->name);
		spin_unlock_bh(&nss_ifb_list_lock);
		return false;
	}

	if (nss_ifb_config_msg_tx(ifb_info->ifb_dev, ifb_num, NSS_IFB_CLOSE, ifb_info) < 0) {
		nss_igs_error("Sending unassign to %s dev failed\n", ifb_info->map_dev->name);
		spin_unlock_bh(&nss_ifb_list_lock);
		return false;
	}
	spin_unlock_bh(&nss_ifb_list_lock);
	return true;
}

/*
 * nss_ifb_up()
 *	Send interface's UP configure message to an IFB interface.
 */
bool nss_ifb_up(struct nss_ifb_info *ifb_info)
{
	int32_t ifb_num;

	spin_lock_bh(&nss_ifb_list_lock);
	if (!(ifb_info->is_mapped)) {
		nss_igs_info("%s IFB device mapped flag is not set\n", ifb_info->ifb_dev->name);
		spin_unlock_bh(&nss_ifb_list_lock);
		return true;
	}

	/*
	 * Send interface's UP config message to an IFB interface.
	 */
	ifb_num = nss_cmn_get_interface_number_by_dev_and_type(ifb_info->ifb_dev, NSS_DYNAMIC_INTERFACE_TYPE_IGS);
	if (ifb_num < 0) {
		nss_igs_error("No %s IFB device found in NSS FW\n", ifb_info->ifb_dev->name);
		spin_unlock_bh(&nss_ifb_list_lock);
		return false;
	}

	if (nss_ifb_config_msg_tx(ifb_info->ifb_dev, ifb_num, NSS_IFB_OPEN, ifb_info) < 0) {
		nss_igs_error("Sending unassign to %s dev failed\n", ifb_info->map_dev->name);
		spin_unlock_bh(&nss_ifb_list_lock);
		return false;
	}
	spin_unlock_bh(&nss_ifb_list_lock);
	return true;
}

/*
 * nss_ifb_clear_igs_node()
 *	Send CLEAR configure message to an IFB mapped interface.
 */
bool nss_ifb_clear_igs_node(struct nss_ifb_info *ifb_info)
{
	int32_t if_num;

	spin_lock_bh(&nss_ifb_list_lock);
	if (!(ifb_info->is_mapped)) {
		nss_igs_info("%s IFB device mapped flag is not set\n", ifb_info->ifb_dev->name);
		spin_unlock_bh(&nss_ifb_list_lock);
		return true;
	}

	/*
	 * Send CLEAR config message to the mapped interface.
	 */
	if_num = nss_cmn_get_interface_number_by_dev_and_type(ifb_info->ifb_dev, NSS_DYNAMIC_INTERFACE_TYPE_IGS);
	if (if_num < 0) {
		nss_igs_error("No %s IFB device found in NSS firmware\n", ifb_info->ifb_dev->name);
	}

	if (nss_ifb_config_msg_tx(ifb_info->map_dev, if_num, NSS_IFB_CLEAR_IGS_NODE, ifb_info) < 0) {
		nss_igs_error("Sending unassign to %s dev failed\n", ifb_info->map_dev->name);
		spin_unlock_bh(&nss_ifb_list_lock);
		return false;
	}
	spin_unlock_bh(&nss_ifb_list_lock);
	return true;
}

/*
 * nss_ifb_init()
 *	Initialization API.
 */
void nss_ifb_init()
{
	sema_init(&msg_response.sem, 1);
	init_waitqueue_head(&msg_response.wq);
}

/*
 * nss_ifb_find_map_dev()
 *	Find and return the IFB mapped netdev in the ifb list.
 */
struct nss_ifb_info *nss_ifb_find_map_dev(struct net_device *dev)
{
	struct nss_ifb_info *ifb_info;

	spin_lock_bh(&nss_ifb_list_lock);
	list_for_each_entry(ifb_info, &nss_ifb_list, map_list) {
		if (ifb_info->map_dev == dev) {
			spin_unlock_bh(&nss_ifb_list_lock);
			return ifb_info;
		}
	}
	spin_unlock_bh(&nss_ifb_list_lock);
	return NULL;
}

/*
 * nss_ifb_find_dev()
 *	Find and return the IFB netdev in the ifb list.
 */
struct nss_ifb_info *nss_ifb_find_dev(struct net_device *dev)
{
	struct nss_ifb_info *ifb_info;

	spin_lock_bh(&nss_ifb_list_lock);
	list_for_each_entry(ifb_info, &nss_ifb_list, map_list) {
		if (ifb_info->ifb_dev == dev) {
			spin_unlock_bh(&nss_ifb_list_lock);
			return ifb_info;
		}
	}
	spin_unlock_bh(&nss_ifb_list_lock);
	return NULL;
}

/*
 * nss_ifb_bind()
 *	API to bind an IFB device with its requested mapped interface.
 */
int32_t nss_ifb_bind(struct nss_ifb_info *ifb_info, struct net_device *from_dev,
		struct net_device *to_dev)
{
	if (!ifb_info) {
		/*
		 * IFB not present in local LL.
		 * Add the entry in LL.
		 */
		ifb_info = kmalloc(sizeof(*ifb_info), GFP_KERNEL);
		if (!ifb_info) {
			nss_igs_error("kmalloc failed\n");
			return -ENOMEM;
		}
		ifb_info->ifb_dev = to_dev;
		ifb_info->map_dev = from_dev;
		ifb_info->is_mapped = true;
		nss_ifb_list_add(ifb_info);
	} else {
		/*
		 * IFB present in local LL and its is_mapped is not set.
		 * make the ifb_info's is_mapped to true again.
		 */
		spin_lock_bh(&nss_ifb_list_lock);
		ifb_info->map_dev = from_dev;
		ifb_info->is_mapped = true;
		spin_unlock_bh(&nss_ifb_list_lock);
	}
	return 0;
}

/*
 * nss_ifb_update_dev_stats()
 *	IFB stats function to copy the common stats to the netdevice.
 */
static void nss_ifb_update_dev_stats(struct net_device *dev, struct nss_igs_stats_sync_msg *sync_stats)
{
	struct pcpu_sw_netstats stats;
	struct nss_cmn_node_stats *node_stats = &(sync_stats->node_stats);

	if (!dev) {
		nss_igs_error("Device is NULL\n");
		return;
	}

	u64_stats_init(&stats.syncp);
	u64_stats_update_begin(&stats.syncp);

	/*
	 * In NSS firmware, the IFB interface's stats are getting updated
	 * post shaping. Therefore IFB interface's stats should be updated
	 * with NSS firmware's IFB TX stats only.
	 */
	u64_stats_set(&stats.rx_packets, node_stats->tx_packets);
   u64_stats_set(&stats.tx_packets, node_stats->tx_packets);
	u64_stats_set(&stats.rx_bytes, node_stats->tx_bytes);
   u64_stats_set(&stats.tx_bytes, node_stats->tx_bytes);
	dev->stats.rx_dropped = dev->stats.tx_dropped += sync_stats->igs_stats.tx_dropped;
	u64_stats_update_end(&stats.syncp);

	ifb_update_offload_stats(dev, &stats);
}

/*
 * nss_ifb_event_cb()
 *	Event Callback for IFB interface to receive events from NSS firmware.
 */
static void nss_ifb_event_cb(void *if_ctx, struct nss_cmn_msg *ncm)
{
	struct net_device *netdev = if_ctx;
	struct nss_igs_msg *nim = (struct nss_igs_msg *)ncm;

	switch (ncm->type) {
	case NSS_IGS_MSG_SYNC_STATS:
		nss_ifb_update_dev_stats(netdev, (struct nss_igs_stats_sync_msg *)&nim->msg.stats);
		break;

	default:
		nss_igs_error("%px: Unknown Event from NSS\n", netdev);
		break;
	}
}

/*
 * nss_ifb_delete_if()
 *	Delete an IFB interface in NSS Firmware.
 */
void nss_ifb_delete_if(int32_t if_num)
{
	nss_igs_unregister_if(if_num);
	if (nss_dynamic_interface_dealloc_node(if_num, NSS_DYNAMIC_INTERFACE_TYPE_IGS)
			 != NSS_TX_SUCCESS) {
		nss_igs_error("Failed to de-alloc IFB dynamic interface\n");
	}
}

/*
 * nss_ifb_create_if()
 *	Create an IFB interface in NSS Firmware.
 */
int32_t nss_ifb_create_if(struct net_device *dev)
{
	int32_t if_num, ret;
	uint32_t features = 0;
	struct nss_ctx_instance *nss_ctx;

	if_num = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_IGS);
	if (if_num < 0) {
		nss_igs_error("%d interface creation failed\n", if_num);
		return -1;
	}

	nss_ctx = nss_igs_register_if(if_num,
			NSS_DYNAMIC_INTERFACE_TYPE_IGS,
			nss_ifb_event_cb,
			dev,
			features);
	if (!nss_ctx) {
		nss_igs_error("%d interface registration failed\n", if_num);
		goto registration_fail;
	}
	return if_num;

registration_fail:
	ret = nss_dynamic_interface_dealloc_node(if_num, NSS_DYNAMIC_INTERFACE_TYPE_IGS);
	if (ret != NSS_TX_SUCCESS) {
		nss_igs_error("%d interface dealloc failed\n", if_num);
	}
	return -1;
}
