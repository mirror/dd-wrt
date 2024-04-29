/*
 ***************************************************************************
 * Copyright (c) 2015-2016,2018-2020, The Linux Foundation. All rights reserved.
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
 ***************************************************************************
 */

#include <linux/version.h>
#include <linux/etherdevice.h>
#include <linux/if_ether.h>
#include <net/genetlink.h>
#include <nss_api_if.h>
#include <nss_nl_if.h>
#include "nss_nlcmn_if.h"
#include "nss_nl.h"
#include "nss_nlgre_redir_if.h"
#include "nss_nlgre_redir_cmn.h"
#include "nss_nlgre_redir_lag.h"

/*
 * Spin_lock for lag_pvt_data
 */
static DEFINE_SPINLOCK(lock);

static struct nss_nlgre_redir_lag_pvt_data lag_pvt_data;
static const struct net_device_ops dummy_netdev_ops;

/*
 * nss_nlgre_redir_lag_msg_completion_cb()
 * 	HLOS->NSS message completion callback.
 */
static void nss_nlgre_redir_lag_msg_completion_cb(void *app_data, struct nss_cmn_msg *cmnmsg)
{
	struct nss_ctx_instance *nss_ctx = nss_gre_redir_get_context();
	nss_nl_info("%px: callback gre_redir tunnel msg from NSS\n", nss_ctx);
}

/*
 * nss_nlgre_redir_lag_us_data_cb()
 *	Exception handler for LAG_US node.
 */
static void nss_nlgre_redir_lag_us_data_cb(struct net_device *netdev, struct sk_buff *skb, struct napi_struct *napi)
{
	nss_nl_trace("Exception packet on lag_us node:\n");
	nss_nlgre_redir_cmn_print_hex_dump(skb);
	dev_kfree_skb_any(skb);
}

/*
 * nss_nlgre_redir_lag_ds_data_cb()
 *	Exception handler for LAG_DS node.
 */
static void nss_nlgre_redir_lag_ds_data_cb(struct net_device *netdev, struct sk_buff *skb, struct napi_struct *napi)
{
	nss_nl_trace("Exception packet on lag_ds node:\n");
	nss_nlgre_redir_cmn_print_hex_dump(skb);
	dev_kfree_skb_any(skb);
}

/*
 * nss_nlgre_redir_lag_us_msg_cb()
 *	GRE LAG upstream  NSS->HLOS message callback.
 */
static void nss_nlgre_redir_lag_us_msg_cb(void *app_data, struct nss_cmn_msg *cmnmsg)
{
	struct nss_gre_redir_lag_us_msg *tunmsg = (struct nss_gre_redir_lag_us_msg *)cmnmsg;
	int i;

	if (tunmsg->cm.type != NSS_GRE_REDIR_LAG_US_DB_HASH_NODE_MSG) {
		return;
	}

	nss_nl_trace("%px: callback_stats: count = %u index = %u\n", tunmsg,
			tunmsg->msg.hash_stats.count, tunmsg->msg.hash_stats.db_entry_idx);
	for (i = 0; i < tunmsg->msg.hash_stats.count; i++) {
		nss_nl_trace("%px: hits = %llu smac = %pM dmac = %pM\n", tunmsg,
				tunmsg->msg.hash_stats.hstats[i].hits,
				&(tunmsg->msg.hash_stats.hstats[i].src_mac),
				&(tunmsg->msg.hash_stats.hstats[i].dest_mac));
	}
}

/*
 * nss_nlgre_redir_lag_ds_msg_cb()
 *	GRE LAG upstream  NSS->HLOS message callback.
 */
static void nss_nlgre_redir_lag_ds_msg_cb(void *app_data, struct nss_cmn_msg *cmnmsg)
{
	struct nss_gre_redir_lag_ds_msg *tunmsg = (struct nss_gre_redir_lag_ds_msg *)cmnmsg;

	if (tunmsg->cm.type != NSS_GRE_REDIR_LAG_DS_STATS_SYNC_MSG) {
		return;
	}

	nss_nl_trace("%px: callback_stats: invalid_dest: %u, exception_cnt: %u\n", tunmsg,
			tunmsg->msg.ds_sync_stats.ds_stats.dst_invalid,
			tunmsg->msg.ds_sync_stats.ds_stats.exception_cnt);

}

/*
 * nss_nlgre_redir_lag_get_lag_pvt_data()
 * 	Return lag_pvt_data after lock checking
 */
static struct nss_nlgre_redir_lag_pvt_data nss_nlgre_redir_lag_get_lag_pvt_data(void)
{
	struct nss_nlgre_redir_lag_pvt_data ret_lag_pvt_data;

	spin_lock(&lock);
	ret_lag_pvt_data = lag_pvt_data;
	spin_unlock(&lock);
	return ret_lag_pvt_data;
}

/*
 * nss_nlgre_redir_lag_set_lag_pvt_data()
 * 	Return lag_pvt_data after lock checking
 */
static void nss_nlgre_redir_lag_set_lag_pvt_data(struct nss_nlgre_redir_lag_pvt_data *data)
{
	spin_lock(&lock);
	lag_pvt_data = *data;
	spin_unlock(&lock);
}

/*
 * nss_nlgre_redir_lag_destroy_tun()
 *	Destroy tunnel in LAG mode.
 */
int nss_nlgre_redir_lag_destroy_tun(struct net_device *dev)
{
	struct nss_ctx_instance *nss_ctx = nss_gre_redir_get_context();
	struct nss_nlgre_redir_lag_pvt_data lag_pvt_data;
	uint8_t tun_idx;
	int ret;

	if (!dev) {
		nss_nl_error("%px: Dev is null\n", nss_ctx);
		return -1;
	}

	dev_hold(dev);

	/*
	 * Unregister and deallocate the lag US and DS interfaces
	 */
	lag_pvt_data = nss_nlgre_redir_lag_get_lag_pvt_data();
	ret = nss_gre_redir_lag_us_unregister_and_dealloc(lag_pvt_data.inner_ifnum);
	if (ret) {
		nss_nl_error("%px: Unable to deallocate node %d\n", dev, lag_pvt_data.inner_ifnum);
	}

	ret = nss_gre_redir_lag_ds_unregister_and_dealloc(lag_pvt_data.outer_ifnum);
	if (ret) {
		nss_nl_error("%px: Unable to deallocate node %d\n", dev, lag_pvt_data.outer_ifnum);
	}

	for (tun_idx = 0; tun_idx < NSS_NLGRE_REDIR_LAG_SLAVES; tun_idx++) {
		if (!lag_pvt_data.slaves[tun_idx]) {
			nss_nl_error("%px: Slave tunnel index out of range\n", nss_ctx);
			ret = -1;
			goto done;
		}

		ret = nss_nlgre_redir_cmn_destroy_tun(lag_pvt_data.slaves[tun_idx]);
		if (ret == -1) {
			nss_nl_error("%px: Unable to destroy tunnel associated with slave %d\n", nss_ctx, tun_idx+1);
			goto done;
		}

		nss_nl_info("%px: Successfully destroyed slave tunnel %d\n", nss_ctx, tun_idx+1);
	}

done:
	/*
	 * Free the lag dummy dev resources
	 */
	dev_put(dev);
	unregister_netdev(dev);
	free_netdev(dev);
	lag_pvt_data.dev = NULL;
	return ret;
}

/*
 * nss_nlgre_redir_lag_create_tun()
 * 	Cretate GRE redir tunnel in LAG mode.
 */
int nss_nlgre_redir_lag_create_tun(struct nss_nlgre_redir_create_tun *create_params)
{
	struct nss_nlgre_redir_lag_pvt_data lag_pvt_data;
	struct nss_gre_redir_lag_us_config_msg config;
	struct nss_gre_redir_cmn_ndev_priv *gr;
	struct nss_ctx_instance *nss_ctx;
	struct net_device *dummy_dev;
	bool status;
	int iter;

	nss_ctx = nss_gre_redir_get_context();
	lag_pvt_data = nss_nlgre_redir_lag_get_lag_pvt_data();
	lag_pvt_data.slaves[0] = nss_nlgre_redir_cmn_create_tun(create_params->sip, create_params->dip, create_params->iptype);
	if (!lag_pvt_data.slaves[0]) {
		nss_nl_error("%px: Unable to create tunnel for %dst slave\n", nss_ctx, 1);
		goto fail0;
	}

	lag_pvt_data.slaves[1] = nss_nlgre_redir_cmn_create_tun(create_params->ssip, create_params->sdip, create_params->iptype);
	if (!lag_pvt_data.slaves[1]) {
		nss_nl_error("%px: Unable to create tunnel for %dnd slave\n", nss_ctx, 2);
		goto fail0;
	}

	dummy_dev = alloc_netdev(sizeof(*gr), "grelag%d", NET_NAME_UNKNOWN, ether_setup);
	if (!dummy_dev) {
		nss_nl_error("%px: Unable to allocate net_dev for dummy_dev\n", nss_ctx);
		goto fail0;
	}

	dummy_dev->needed_headroom = NSS_NLGRE_REDIR_CMN_NEEDED_HEADROOM;
	dummy_dev->netdev_ops = &dummy_netdev_ops;
	lag_pvt_data.inner_ifnum = nss_gre_redir_lag_us_alloc_and_register_node(dummy_dev,
			nss_nlgre_redir_lag_us_data_cb, nss_nlgre_redir_lag_us_msg_cb, dummy_dev);

	if (lag_pvt_data.inner_ifnum == -1) {
		nss_nl_error("%px: Unable to allocate or register LAG US dynamic node.\n", nss_ctx);
		goto fail1;
	}

	nss_nl_info("%px: LAG US interface number = %d\n", nss_ctx, lag_pvt_data.inner_ifnum);
	lag_pvt_data.outer_ifnum = nss_gre_redir_lag_ds_alloc_and_register_node(dummy_dev,
			nss_nlgre_redir_lag_ds_data_cb, nss_nlgre_redir_lag_ds_msg_cb, dummy_dev);

	if (lag_pvt_data.outer_ifnum == -1) {
		nss_nl_error("%px: Unable to allocate or register LAG DS dynamic node.\n", nss_ctx);
		goto fail2;
	}

	nss_nl_info("%px: LAG DS interface number = %d\n", nss_ctx, lag_pvt_data.outer_ifnum);

	config.hash_mode = create_params->hash_mode;
	config.num_slaves = NSS_NLGRE_REDIR_LAG_SLAVES;

	for (iter = 0; iter < NSS_NLGRE_REDIR_LAG_SLAVES; iter++) {
		config.if_num[iter] = nss_nlgre_redir_cmn_get_tun_ifnum(NSS_NLGRE_REDIR_CMN_MODE_TYPE_WIFI, lag_pvt_data.slaves[iter]);
	}

	status = nss_gre_redir_lag_us_configure_node(lag_pvt_data.inner_ifnum, &config);
	if (!status) {
		nss_nl_info("%px: Unable to configure LAG US node.\n", nss_ctx);
		goto fail3;
	}

	if (register_netdev(dummy_dev)) {
		nss_nl_error("%px: Unable to register dummy_dev\n", nss_ctx);
		goto fail3;
	}

	lag_pvt_data.dev = dummy_dev;
	nss_nlgre_redir_lag_set_lag_pvt_data(&lag_pvt_data);
	return 0;

fail3:
	nss_nlgre_redir_cmn_unregister_and_deallocate(dummy_dev, NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_LAG_DS);
fail2:
	nss_nlgre_redir_cmn_unregister_and_deallocate(dummy_dev, NSS_DYNAMIC_INTERFACE_TYPE_GRE_REDIR_LAG_US);
fail1:
	free_netdev(dummy_dev);
fail0:
	for (iter = 0; iter < NSS_NLGRE_REDIR_LAG_SLAVES; iter++) {
		if (!lag_pvt_data.slaves[iter]) {
			nss_nlgre_redir_cmn_destroy_tun(lag_pvt_data.slaves[iter]);
			continue;
		}

		break;
	}

	return -1;
}

/*
 * nss_nlgre_redir_lag_map_interface()
 * 	Maps the nss interface to tunnel ID
 */
int nss_nlgre_redir_lag_map_interface(struct nss_nlgre_redir_map *map_params)
{
	struct nss_nlgre_redir_lag_pvt_data lag_pvt_data;
	struct nss_ctx_instance *nss_ctx;
	uint32_t nexthop_nssif;
	int ret;

	nss_ctx = nss_gre_redir_get_context();
	lag_pvt_data = nss_nlgre_redir_lag_get_lag_pvt_data();
	nexthop_nssif = lag_pvt_data.outer_ifnum;
	ret = nss_nlgre_redir_cmn_map_interface(nexthop_nssif, 1, map_params);
	if (ret == -1) {
		nss_nl_error("%px: Unable to map nss interface\n", nss_ctx);
		return -1;
	}

	nss_nl_info("Successfully mapped the nss interface to tunnel ID\n");
	return 0;
}

/*
 * nss_nlgre_redir_lag_set_next_hop()
 * 	Sets the next hop of ath0 interface as lag US node's interface
 */
int nss_nlgre_redir_lag_set_next_hop(struct nss_nlgre_redir_set_next *set_next_params)
{
	struct nss_nlgre_redir_lag_pvt_data lag_pvt_data;
	struct nss_ctx_instance *nss_ctx;
	uint32_t nexthop_ifnum;
	int ret;

	nss_ctx = nss_gre_redir_get_context();
	lag_pvt_data = nss_nlgre_redir_lag_get_lag_pvt_data();
	nexthop_ifnum = lag_pvt_data.inner_ifnum;
	ret = nss_nlgre_redir_cmn_set_next_hop(nexthop_ifnum, set_next_params);
	if (ret == -1) {
		nss_nl_error("%px: Unable to set the next hop as lag US node's interface\n", nss_ctx);
		return -1;
	}

	nss_nl_info("Successfully set the next hop as lag US node's interface\n");
	return 0;
}

/*
 * nss_nlgre_redir_lag_add_hash()
 *	Add hash entry.
 */
int nss_nlgre_redir_lag_add_hash(struct nss_nlgre_redir_hash_ops *hash_ops)
{
	struct nss_nlgre_redir_lag_pvt_data lag_pvt_data;
	struct nss_gre_redir_lag_us_msg *nglm;
	struct nss_ctx_instance *nss_ctx;
	nss_tx_status_t status;
	uint32_t len;
	int i;

	lag_pvt_data = nss_nlgre_redir_lag_get_lag_pvt_data();
	nss_ctx = nss_gre_redir_lag_us_get_context();
	if (!nss_ctx) {
		nss_nl_error("Unable to get nss context\n");
		return -1;
	}

	nglm = kmalloc(sizeof(struct nss_gre_redir_lag_us_msg), GFP_KERNEL);
	if (!nglm) {
		nss_nl_error("%px: Unable to allocate memory to send add hash msg.\n", nss_ctx);
		return -1;
	}

	len = sizeof(struct nss_gre_redir_lag_us_msg) - sizeof(struct nss_cmn_msg);
	nss_cmn_msg_init(&nglm->cm, lag_pvt_data.inner_ifnum, NSS_GRE_REDIR_LAG_US_ADD_HASH_NODE_MSG, len,
			nss_nlgre_redir_lag_msg_completion_cb, NULL);
	memcpy((void *)(nglm->msg.add_hash.src_mac), (void *)hash_ops->smac, sizeof(nglm->msg.add_hash.src_mac));
	memcpy((void *)(nglm->msg.add_hash.dest_mac), (void *)hash_ops->dmac, sizeof(nglm->msg.add_hash.dest_mac));
	for (i = 0; i < NSS_NLGRE_REDIR_LAG_SLAVES; i++) {
		if (hash_ops->slave != i) {
			continue;
		}

		nglm->msg.add_hash.if_num = nss_nlgre_redir_cmn_get_tun_ifnum(NSS_NLGRE_REDIR_CMN_MODE_TYPE_WIFI,
				lag_pvt_data.slaves[i]);
		break;
	}

	if (i == NSS_NLGRE_REDIR_LAG_SLAVES || nglm->msg.add_hash.if_num == -1) {
		nss_nl_error("%px: Invalid value for index, valid slaves: [%s, %s]\n", nss_ctx,
				lag_pvt_data.slaves[0]->name, lag_pvt_data.slaves[1]->name);
		return -1;
	}

	nss_nl_info("smac = %pM dmac = %pM ifnum = %d\n", &(nglm->msg.add_hash.src_mac),
			&(nglm->msg.add_hash.dest_mac), nglm->msg.add_hash.if_num);
	status = nss_gre_redir_lag_us_tx_msg_sync(nss_ctx, nglm);
	kfree(nglm);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_error("%px: Unable to add hash entry.\n", nss_ctx);
		return -1;
	}

	return 0;
}

/*
 * nss_nlgre_redir_lag_del_hash()
 *	Delete hash entry.
 */
int nss_nlgre_redir_lag_del_hash(struct nss_nlgre_redir_hash_ops *hash_ops)
{
	struct nss_nlgre_redir_lag_pvt_data lag_pvt_data;
	struct nss_gre_redir_lag_us_msg *nglm;
	struct nss_ctx_instance *nss_ctx;
	nss_tx_status_t status;
	uint32_t len;

	lag_pvt_data = nss_nlgre_redir_lag_get_lag_pvt_data();
	nss_ctx = nss_gre_redir_lag_us_get_context();
	if (!nss_ctx) {
		nss_nl_error("Unable to get nss context\n");
		return -1;
	}

	nglm = kmalloc(sizeof(struct nss_gre_redir_lag_us_msg), GFP_KERNEL);
	if (!nglm) {
		nss_nl_error("%px: Unable to allocate memory to send del hash msg.\n", nss_ctx);
		return -1;
	}

	len = sizeof(struct nss_gre_redir_lag_us_msg) - sizeof(struct nss_cmn_msg);
	nss_cmn_msg_init(&nglm->cm, lag_pvt_data.inner_ifnum, NSS_GRE_REDIR_LAG_US_DEL_HASH_NODE_MSG, len,
			nss_nlgre_redir_lag_msg_completion_cb, NULL);
	memcpy((void *)(nglm->msg.del_hash.src_mac), (void *)hash_ops->smac, sizeof(nglm->msg.del_hash.src_mac));
	memcpy((void *)(nglm->msg.del_hash.dest_mac), (void *)hash_ops->dmac, sizeof(nglm->msg.del_hash.dest_mac));
	nss_nl_info("smac = %pM dmac = %pM\n", &(nglm->msg.del_hash.src_mac), &(nglm->msg.del_hash.dest_mac));
	status = nss_gre_redir_lag_us_tx_msg_sync(nss_ctx, nglm);
	kfree(nglm);
	if (status != NSS_TX_SUCCESS) {
		nss_nl_error("%px: Unable to delete hash entry.\n", nss_ctx);
		return -1;
	}

	return 0;
}
