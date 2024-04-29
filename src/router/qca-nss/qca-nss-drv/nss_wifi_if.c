/*
 **************************************************************************
 * Copyright (c) 2015-2020, The Linux Foundation. All rights reserved.
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
 * nss_wifi_if.c
 *	NSS wifi/redirect handler APIs
 */

#include "nss_tx_rx_common.h"
#include "nss_wifi_if_stats.h"
#include <net/arp.h>

#define NSS_WIFI_IF_TX_TIMEOUT			3000 /* 3 Seconds */
#define NSS_WIFI_IF_GET_INDEX(if_num)	(if_num-NSS_DYNAMIC_IF_START)

extern int nss_ctl_redirect;

/*
 * Data structure that holds the wifi interface context.
 */
struct nss_wifi_if_handle *wifi_handle[NSS_MAX_DYNAMIC_INTERFACES];

/*
 * Spinlock to protect the global data structure wifi_handle.
 */
DEFINE_SPINLOCK(wifi_if_lock);

/*
 * nss_wifi_if_msg_handler()
 *	Handle NSS -> HLOS messages for wifi interface
 */
static void nss_wifi_if_msg_handler(struct nss_ctx_instance *nss_ctx,
					struct nss_cmn_msg *ncm,
					__attribute__((unused))void *app_data)
{
	struct nss_wifi_if_msg *nwim = (struct nss_wifi_if_msg *)ncm;
	int32_t if_num;

	nss_wifi_if_msg_callback_t cb;
	struct nss_wifi_if_handle *handle = NULL;

	/*
	 * Sanity check the message type
	 */
	if (ncm->type >= NSS_WIFI_IF_MAX_MSG_TYPES) {
		nss_warning("%px: message type out of range: %d",
						nss_ctx, ncm->type);
		return;
	}

	if (nss_cmn_get_msg_len(ncm) > sizeof(struct nss_wifi_if_msg)) {
		nss_warning("%px: Length of message is greater than required: %d", nss_ctx, nss_cmn_get_msg_len(ncm));
		return;
	}

	if (!NSS_IS_IF_TYPE(DYNAMIC, ncm->interface)) {
		nss_warning("%px: response for another interface: %d", nss_ctx, ncm->interface);
		return;
	}

	/*
	 * Log failures
	 */
	nss_core_log_msg_failures(nss_ctx, ncm);

	if_num = NSS_WIFI_IF_GET_INDEX(ncm->interface);

	spin_lock_bh(&wifi_if_lock);
	if (!wifi_handle[if_num]) {
		spin_unlock_bh(&wifi_if_lock);
		nss_warning("%px: wifi_if handle is NULL\n", nss_ctx);
		return;
	}

	handle = wifi_handle[if_num];
	spin_unlock_bh(&wifi_if_lock);

	switch (nwim->cm.type) {
	case NSS_WIFI_IF_STATS_SYNC_MSG:
		nss_wifi_if_stats_sync(handle, &nwim->msg.stats);
		break;
	}

	/*
	 * Update the callback and app_data for NOTIFY messages.
	 */
	if (nwim->cm.response == NSS_CMN_RESPONSE_NOTIFY) {
		ncm->cb = (nss_ptr_t)handle->cb;
		ncm->app_data = (nss_ptr_t)handle->app_data;
	}

	/*
	 * Do we have a callback?
	 */
	if (!ncm->cb) {
		nss_warning("cb is NULL\n");
		return;
	}

	/*
	 * Callback
	 */
	cb = (nss_wifi_if_msg_callback_t)ncm->cb;
	cb((void *)ncm->app_data, ncm);
}

/*
 * nss_wifi_if_register_handler()
 *	Register the message handler & initialize semaphore & completion for the * interface if_num
 */
static uint32_t nss_wifi_if_register_handler(struct nss_wifi_if_handle *handle)
{
	struct nss_ctx_instance *nss_ctx = &nss_top_main.nss[nss_top_main.wlan_handler_id];
	uint32_t ret;
	struct nss_wifi_if_pvt *nwip = NULL;
	int32_t if_num = handle->if_num;

	ret = nss_core_register_handler(nss_ctx, if_num, nss_wifi_if_msg_handler, NULL);

	if (ret != NSS_CORE_STATUS_SUCCESS) {
		nss_warning("%d: Message handler failed to be registered for interface ret %d\n", if_num, ret);
		return NSS_WIFI_IF_CORE_FAILURE;
	}

	nwip = handle->pvt;
	if (!nwip->sem_init_done) {
		sema_init(&nwip->sem, 1);
		init_completion(&nwip->complete);
		nwip->sem_init_done = 1;
	}

	nss_wifi_if_stats_dentry_create();
	return NSS_WIFI_IF_SUCCESS;
}

/*
 * nss_wifi_if_callback
 *	Callback to handle the completion of NSS->HLOS messages.
 */
static void nss_wifi_if_callback(void *app_data, struct nss_cmn_msg *ncm)
{
	struct nss_wifi_if_handle *handle = (struct nss_wifi_if_handle *)app_data;
	struct nss_wifi_if_pvt *nwip = handle->pvt;

	if (ncm->response != NSS_CMN_RESPONSE_ACK) {
		nss_warning("%px: wifi_if Error response %d\n",
						handle->nss_ctx, ncm->response);
		nwip->response = NSS_TX_FAILURE;
		complete(&nwip->complete);
		return;
	}

	nwip->response = NSS_TX_SUCCESS;
	complete(&nwip->complete);
}

/*
 * nss_wifi_if_tx_msg()
 *	Send a message from HLOS to NSS asynchronously.
 */
nss_tx_status_t nss_wifi_if_tx_msg(struct nss_ctx_instance *nss_ctx, struct nss_wifi_if_msg *nwim)
{
	struct nss_cmn_msg *ncm = &nwim->cm;

	if (ncm->type > NSS_WIFI_IF_MAX_MSG_TYPES) {
		nss_warning("%px: message type out of range: %d\n", nss_ctx, ncm->type);
		return NSS_TX_FAILURE;
	}

	return nss_core_send_cmd(nss_ctx, nwim, sizeof(*nwim), NSS_NBUF_PAYLOAD_SIZE);
}

/*
 * nss_wifi_if_tx_msg_sync
 *	Send a message from HLOS to NSS synchronously.
 */
static nss_tx_status_t nss_wifi_if_tx_msg_sync(struct nss_wifi_if_handle *handle,
							struct nss_wifi_if_msg *nwim)
{
	nss_tx_status_t status;
	int ret = 0;
	struct nss_wifi_if_pvt *nwip = handle->pvt;
	struct nss_ctx_instance *nss_ctx = handle->nss_ctx;

	down(&nwip->sem);

	status = nss_wifi_if_tx_msg(nss_ctx, nwim);
	if (status != NSS_TX_SUCCESS) {
		nss_warning("%px: nss_wifi_if_msg failed\n", nss_ctx);
		up(&nwip->sem);
		return status;
	}

	ret = wait_for_completion_timeout(&nwip->complete,
						msecs_to_jiffies(NSS_WIFI_IF_TX_TIMEOUT));
	if (!ret) {
		nss_warning("%px: wifi_if tx failed due to timeout\n", nss_ctx);
		nwip->response = NSS_TX_FAILURE;
	}

	status = nwip->response;
	up(&nwip->sem);

	return status;
}

/*
 * nss_wifi_if_handle_destroy()
 *	Destroy the wifi handle either due to request from WLAN or due to error.
 */
static int nss_wifi_if_handle_destroy(struct nss_wifi_if_handle *handle)
{
	int32_t if_num;
	int32_t index;
	struct nss_ctx_instance *nss_ctx;
	nss_tx_status_t status;

	if (!handle) {
		nss_warning("Destroy failed as wifi_if handle is NULL\n");
		return NSS_TX_FAILURE_BAD_PARAM;
	}

	if_num = handle->if_num;
	index = NSS_WIFI_IF_GET_INDEX(if_num);
	nss_ctx = handle->nss_ctx;

	spin_lock_bh(&wifi_if_lock);
	wifi_handle[index] = NULL;
	spin_unlock_bh(&wifi_if_lock);

	status = nss_dynamic_interface_dealloc_node(if_num, NSS_DYNAMIC_INTERFACE_TYPE_WIFI);
	if (status != NSS_TX_SUCCESS) {
		nss_warning("%px: Dynamic interface destroy failed status %d\n", nss_ctx, status);
		return status;
	}

	kfree(handle->pvt);
	kfree(handle);

	return status;
}

/*
 * nss_wifi_if_handle_init()
 *	Initialize wifi handle which holds the if_num and stats per interface.
 */
static struct nss_wifi_if_handle *nss_wifi_if_handle_create(struct nss_ctx_instance *nss_ctx,
										int32_t *cmd_rsp)
{
	int32_t index;
	int32_t if_num = 0;
	struct nss_wifi_if_handle *handle;

	if_num = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_WIFI);
	if (if_num < 0) {
		nss_warning("%px:failure allocating wifi if\n", nss_ctx);
		*cmd_rsp = NSS_WIFI_IF_DYNAMIC_IF_FAILURE;
		return NULL;
	}

	index = NSS_WIFI_IF_GET_INDEX(if_num);

	handle = (struct nss_wifi_if_handle *)kzalloc(sizeof(struct nss_wifi_if_handle),
									GFP_KERNEL);
	if (!handle) {
		nss_warning("%px: handle memory alloc failed\n", nss_ctx);
		*cmd_rsp = NSS_WIFI_IF_ALLOC_FAILURE;
		goto error1;
	}

	handle->nss_ctx = nss_ctx;
	handle->if_num = if_num;
	handle->pvt = (struct nss_wifi_if_pvt *)kzalloc(sizeof(struct nss_wifi_if_pvt),
								GFP_KERNEL);
	if (!handle->pvt) {
		nss_warning("%px: failure allocating memory for nss_wifi_if_pvt\n", nss_ctx);
		*cmd_rsp = NSS_WIFI_IF_ALLOC_FAILURE;
		goto error2;
	}

	handle->cb = NULL;
	handle->app_data = NULL;

	spin_lock_bh(&wifi_if_lock);
	wifi_handle[index] = handle;
	spin_unlock_bh(&wifi_if_lock);

	*cmd_rsp = NSS_WIFI_IF_SUCCESS;

	return handle;

error2:
	kfree(handle);
error1:
	nss_dynamic_interface_dealloc_node(if_num, NSS_DYNAMIC_INTERFACE_TYPE_WIFI);
	return NULL;
}

/* nss_wifi_if_msg_init()
 *	Initialize wifi specific message structure.
 */
static void nss_wifi_if_msg_init(struct nss_wifi_if_msg *nwim,
					uint16_t if_num,
					uint32_t type,
					uint32_t len,
					nss_wifi_if_msg_callback_t cb,
					struct nss_wifi_if_handle *app_data)
{
	nss_cmn_msg_init(&nwim->cm, if_num, type, len, (void *)cb, (void *)app_data);
}

/*
 * nss_wifi_if_create_sync()
 *	Create a wifi interface and associate it with the netdev
 */
struct nss_wifi_if_handle *nss_wifi_if_create_sync(struct net_device *netdev)
{
	struct nss_ctx_instance *nss_ctx = &nss_top_main.nss[nss_top_main.wlan_handler_id];
	struct nss_wifi_if_msg nwim;
	struct nss_wifi_if_create_msg *nwcm;
	uint32_t ret;
	struct nss_wifi_if_handle *handle = NULL;

	if (unlikely(nss_ctx->state != NSS_CORE_STATE_INITIALIZED)) {
		nss_warning("%px: Interface could not be created as core not ready\n", nss_ctx);
		return NULL;
	}

	handle = nss_wifi_if_handle_create(nss_ctx, &ret);
	if (!handle) {
		nss_warning("%px:wifi_if handle creation failed ret %d\n", nss_ctx, ret);
		return NULL;
	}

	/* Initializes the semaphore and also sets the msg handler for if_num */
	ret = nss_wifi_if_register_handler(handle);
	if (ret != NSS_WIFI_IF_SUCCESS) {
		nss_warning("%px: Registration handler failed reason: %d\n", nss_ctx, ret);
		goto error;
	}

	nss_wifi_if_msg_init(&nwim, handle->if_num, NSS_WIFI_IF_TX_CREATE_MSG,
				sizeof(struct nss_wifi_if_create_msg), nss_wifi_if_callback, handle);

	nwcm = &nwim.msg.create;
	nwcm->flags = 0;
	memcpy(nwcm->mac_addr, netdev->dev_addr, ETH_ALEN);

	ret = nss_wifi_if_tx_msg_sync(handle, &nwim);
	if (ret != NSS_TX_SUCCESS) {
		nss_warning("%px: nss_wifi_if_tx_msg_sync failed %u\n", nss_ctx, ret);
		goto error;
	}

	nss_core_register_subsys_dp(nss_ctx, handle->if_num, NULL, NULL, NULL, netdev, 0);

	/*
	 * Hold a reference to the net_device
	 */
	dev_hold(netdev);

	/*
	 * The context returned is the interface # which is, essentially, the index into the if_ctx
	 * array that is holding the net_device pointer
	 */

	return handle;

error:
	nss_wifi_if_handle_destroy(handle);
	return NULL;
}
EXPORT_SYMBOL(nss_wifi_if_create_sync);

/*
 * nss_wifi_if_destroy_sync()
 *	Destroy the wifi interface associated with the interface number.
 */
nss_tx_status_t nss_wifi_if_destroy_sync(struct nss_wifi_if_handle *handle)
{
	nss_tx_status_t status;
	struct net_device *dev;
	int32_t if_num = handle->if_num;
	struct nss_ctx_instance *nss_ctx = handle->nss_ctx;

	if (unlikely(nss_ctx->state != NSS_CORE_STATE_INITIALIZED)) {
		nss_warning("%px: Interface could not be destroyed as core not ready\n", nss_ctx);
		return NSS_TX_FAILURE_NOT_READY;
	}

	spin_lock_bh(&nss_top_main.lock);
	if (!nss_ctx->subsys_dp_register[if_num].ndev) {
		spin_unlock_bh(&nss_top_main.lock);
		nss_warning("%px: Unregister wifi interface %d: no context\n", nss_ctx, if_num);
		return NSS_TX_FAILURE_BAD_PARAM;
	}

	dev = nss_ctx->subsys_dp_register[if_num].ndev;
	nss_core_unregister_subsys_dp(nss_ctx, if_num);
	spin_unlock_bh(&nss_top_main.lock);
	dev_put(dev);

	status = nss_wifi_if_handle_destroy(handle);
	return status;
}
EXPORT_SYMBOL(nss_wifi_if_destroy_sync);

/*
 * nss_wifi_if_register()
 *	Register cb, netdev associated with the if_num to the nss data plane
 * to receive data packets.
 */
void nss_wifi_if_register(struct nss_wifi_if_handle *handle,
				nss_wifi_if_data_callback_t rx_callback,
				struct net_device *netdev)
{
	struct nss_ctx_instance *nss_ctx;
	int32_t if_num;

	if (!handle) {
		nss_warning("nss_wifi_if_register handle is NULL\n");
		return;
	}

	nss_ctx = handle->nss_ctx;
	if_num = handle->if_num;
	nss_assert(NSS_IS_IF_TYPE(DYNAMIC, if_num));

	nss_core_register_subsys_dp(nss_ctx, if_num, rx_callback, NULL, NULL, netdev, netdev->features);
}
EXPORT_SYMBOL(nss_wifi_if_register);

/*
 * nss_wifi_if_unregister()
 *	Unregister the cb, netdev associated with the if_num.
 */
void nss_wifi_if_unregister(struct nss_wifi_if_handle *handle)
{
	struct nss_ctx_instance *nss_ctx;
	int32_t if_num;

	if (!handle) {
		nss_warning("nss_wifi_if_unregister handle is NULL\n");
		return;
	}

	nss_ctx = handle->nss_ctx;
	if_num = handle->if_num;

	nss_core_unregister_subsys_dp(nss_ctx, if_num);
}
EXPORT_SYMBOL(nss_wifi_if_unregister);

/*
 * nss_wifi_if_tx_buf()
 *	HLOS interface has received a packet which we redirect to the NSS.
 */
nss_tx_status_t nss_wifi_if_tx_buf(struct nss_wifi_if_handle *handle,
						struct sk_buff *skb)
{
	struct nss_ctx_instance *nss_ctx;
	int32_t if_num;
	int cpu = 0;

	if (!handle) {
		nss_warning("nss_wifi_if_tx_buf handle is NULL\n");
		return NSS_TX_FAILURE;
	}

	nss_ctx = handle->nss_ctx;
	if_num = handle->if_num;

	/*
	 * redirect should be turned on in /proc/
	 */
	if (unlikely(nss_ctl_redirect == 0)) {
		return NSS_TX_FAILURE_NOT_ENABLED;
	}

	if (unlikely(skb->vlan_tci)) {
		return NSS_TX_FAILURE_NOT_SUPPORTED;
	}

	nss_assert(NSS_IS_IF_TYPE(DYNAMIC, if_num));

	/*
	 * Sanity check the SKB to ensure that it's suitable for us
	 */
	if (unlikely(skb->len <= ETH_HLEN)) {
		nss_warning("%px: Rx packet: %px too short", nss_ctx, skb);
		return NSS_TX_FAILURE_TOO_SHORT;
	}

	/*
	 * set skb queue mapping
	 */
	cpu = get_cpu();
	put_cpu();
	skb_set_queue_mapping(skb, cpu);

	return nss_core_send_packet(nss_ctx, skb, if_num, H2N_BIT_FLAG_VIRTUAL_BUFFER);
}
EXPORT_SYMBOL(nss_wifi_if_tx_buf);
