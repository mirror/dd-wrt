/*
 **************************************************************************
 * Copyright (c) 2017-2020, The Linux Foundation. All rights reserved.
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

#include "nss_tx_rx_common.h"
#include "nss_wifili_stats.h"
#include "nss_wifili_log.h"
#include "nss_wifili_strings.h"

#define NSS_WIFILI_TX_TIMEOUT 1000 /* Millisecond to jiffies*/

/*
 * nss_wifili_pvt
 *	Private data structure
 */
static struct nss_wifili_pvt {
	struct semaphore sem;
	struct completion complete;
	int response;
	void *cb;
	void *app_data;
} wifili_pvt;

/*
 * nss_wifili_handler()
 *	Handle NSS -> HLOS messages for wifi
 */
static void nss_wifili_handler(struct nss_ctx_instance *nss_ctx, struct nss_cmn_msg *ncm, __attribute__((unused))void *app_data)
{
	struct nss_wifili_msg *ntm = (struct nss_wifili_msg *)ncm;
	void *ctx;
	nss_wifili_msg_callback_t cb;

	nss_info("%px: NSS->HLOS message for wifili\n", nss_ctx);

	/*
	 * The interface number shall be wifili soc interface or wifili radio interface
	 */
	BUG_ON((nss_is_dynamic_interface(ncm->interface))
		|| ((ncm->interface != NSS_WIFILI_INTERNAL_INTERFACE)
#if (NSS_FW_VERSION_CODE > NSS_FW_VERSION(11,0))
		&& (ncm->interface != NSS_WIFILI_EXTERNAL_INTERFACE0)
		&& (ncm->interface != NSS_WIFILI_EXTERNAL_INTERFACE1)
#endif
		));

	/*
	 * Trace messages.
	 */
	nss_wifili_log_rx_msg(ntm);

	/*
	 * Is this a valid request/response packet?
	 */
	if (ncm->type >= NSS_WIFILI_MAX_MSG) {
		nss_warning("%px: Received invalid message %d for wifili interface", nss_ctx, ncm->type);
		return;
	}

	if ((nss_cmn_get_msg_len(ncm) > sizeof(struct nss_wifili_msg)) &&
		ntm->cm.type != NSS_WIFILI_PEER_EXT_STATS_MSG) {
		nss_warning("%px: Length of message is greater than required: %d", nss_ctx, nss_cmn_get_msg_len(ncm));
		return;
	}

	/*
	 * Snoop messages for local driver and handle
	 */
	switch (ntm->cm.type) {
	case NSS_WIFILI_STATS_MSG:
		/*
		 * Update WIFI driver statistics and send statistics notifications to the registered modules
		 */
		nss_wifili_stats_sync(nss_ctx, &ntm->msg.wlsoc_stats, ncm->interface);
		nss_wifili_stats_notify(nss_ctx, ncm->interface);
		break;
	}

	/*
	 * Update the callback and app_data for notify messages, wifili sends all notify messages
	 * to the same callback/app_data.
	 */
	if (ncm->response == NSS_CMN_RESPONSE_NOTIFY) {
		ncm->cb = (nss_ptr_t)nss_ctx->nss_top->wifili_msg_callback;
	}

	/*
	 * Log failures
	 */
	nss_core_log_msg_failures(nss_ctx, ncm);

	/*
	 * Do we have a call back
	 */
	if (!ncm->cb) {
		nss_info("%px: cb null for wifili interface %d", nss_ctx, ncm->interface);
		return;
	}

	/*
	 * Get callback & context
	 */
	cb = (nss_wifili_msg_callback_t)ncm->cb;
	ctx = nss_ctx->subsys_dp_register[ncm->interface].ndev;

	/*
	 * call wifili msg callback
	 */
	if (!ctx) {
		nss_warning("%px: Event received for wifili interface %d before registration", nss_ctx, ncm->interface);
		return;
	}

	cb(ctx, ntm);
}

/*
 * nss_wifili_callback()
 *	Callback to handle the completion of NSS->HLOS messages.
 */
static void nss_wifili_callback(void *app_data, struct nss_wifili_msg *nvm)
{
	nss_wifili_msg_callback_t callback = (nss_wifili_msg_callback_t)wifili_pvt.cb;
	void *data = wifili_pvt.app_data;

	wifili_pvt.response = NSS_TX_SUCCESS;
	wifili_pvt.cb = NULL;
	wifili_pvt.app_data = NULL;

	if (nvm->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_warning("wifili error response %d\n", nvm->cm.response);
		wifili_pvt.response = nvm->cm.response;
	}

	if (callback) {
		callback(data, nvm);
	}
	complete(&wifili_pvt.complete);
}

/*
 * nss_wifili_tx_msg
 *	Transmit a wifili message to NSS FW
 *
 * NOTE: The caller is expected to handle synchronous wait for message
 * response if needed.
 */
nss_tx_status_t nss_wifili_tx_msg(struct nss_ctx_instance *nss_ctx, struct nss_wifili_msg *msg)
{
	struct nss_cmn_msg *ncm = &msg->cm;

	/*
	 * Trace messages.
	 */
	nss_wifili_log_tx_msg(msg);

	if (ncm->type >= NSS_WIFILI_MAX_MSG) {
		nss_warning("%px: wifili message type out of range: %d", nss_ctx, ncm->type);
		return NSS_TX_FAILURE;
	}

	/*
	 * The interface number shall be one of the wifili soc interfaces
	 */
	if ((ncm->interface != NSS_WIFILI_INTERNAL_INTERFACE)
#if (NSS_FW_VERSION_CODE > NSS_FW_VERSION(11,0))
		&& (ncm->interface != NSS_WIFILI_EXTERNAL_INTERFACE0)
		&& (ncm->interface != NSS_WIFILI_EXTERNAL_INTERFACE1))
#else
	)
#endif
	{
		nss_warning("%px: tx request for interface that is not a wifili: %d", nss_ctx, ncm->interface);
		return NSS_TX_FAILURE;
	}

	return nss_core_send_cmd(nss_ctx, msg, sizeof(*msg), NSS_NBUF_PAYLOAD_SIZE);
}
EXPORT_SYMBOL(nss_wifili_tx_msg);

/*
 * nss_wifili_tx_msg_sync()
 *	Transmit a wifili message to NSS firmware synchronously.
 */
nss_tx_status_t nss_wifili_tx_msg_sync(struct nss_ctx_instance *nss_ctx, struct nss_wifili_msg *nvm)
{
	nss_tx_status_t status;
	int ret = 0;

	down(&wifili_pvt.sem);
	wifili_pvt.cb = (void *)nvm->cm.cb;
	wifili_pvt.app_data = (void *)nvm->cm.app_data;

	nvm->cm.cb = (nss_ptr_t)nss_wifili_callback;
	nvm->cm.app_data = (nss_ptr_t)NULL;

	status = nss_wifili_tx_msg(nss_ctx, nvm);
	if (status != NSS_TX_SUCCESS) {
		nss_warning("%px: wifili_tx_msg failed\n", nss_ctx);
		up(&wifili_pvt.sem);
		return status;
	}

	ret = wait_for_completion_timeout(&wifili_pvt.complete, msecs_to_jiffies(NSS_WIFILI_TX_TIMEOUT));
	if (!ret) {
		nss_warning("%px: wifili msg tx failed due to timeout\n", nss_ctx);
		wifili_pvt.response = NSS_TX_FAILURE;
	}

	status = wifili_pvt.response;
	up(&wifili_pvt.sem);
	return status;
}
EXPORT_SYMBOL(nss_wifili_tx_msg_sync);

/*
 * nss_wifili_get_context()
 */
struct nss_ctx_instance *nss_wifili_get_context(void)
{
	return (struct nss_ctx_instance *)&nss_top_main.nss[nss_top_main.wifi_handler_id];
}
EXPORT_SYMBOL(nss_wifili_get_context);

/*
 * nss_get_available_wifili_external_if()
 *	Check and return the available external interface
 */
#if (NSS_FW_VERSION_CODE > NSS_FW_VERSION(11,0))
uint32_t nss_get_available_wifili_external_if(void)
{
	struct nss_ctx_instance *nss_ctx = (struct nss_ctx_instance *)&nss_top_main.nss[nss_top_main.wifi_handler_id];
	/*
	 * Check if the external interface is registered.
	 * Return the interface number if not registered.
	 */
	if (!(nss_ctx->subsys_dp_register[NSS_WIFILI_EXTERNAL_INTERFACE0].ndev)) {
		return NSS_WIFILI_EXTERNAL_INTERFACE0;
	}

	if (!(nss_ctx->subsys_dp_register[NSS_WIFILI_EXTERNAL_INTERFACE1].ndev)) {
		return NSS_WIFILI_EXTERNAL_INTERFACE1;
	}

	nss_warning("%px: No available external intefaces\n", nss_ctx);

	return NSS_MAX_NET_INTERFACES;
}
EXPORT_SYMBOL(nss_get_available_wifili_external_if);
#endif
/*
 * nss_wifili_msg_init()
 *	Initialize nss_wifili_msg.
 */
void nss_wifili_msg_init(struct nss_wifili_msg *ncm, uint16_t if_num, uint32_t type, uint32_t len, void *cb, void *app_data)
{
	nss_cmn_msg_init(&ncm->cm, if_num, type, len, cb, app_data);
}
EXPORT_SYMBOL(nss_wifili_msg_init);

/*
 ****************************************
 * Register/Unregister/Miscellaneous APIs
 ****************************************
 */

/*
 * nss_register_wifili_if()
 *	Register wifili with nss driver
 */
struct nss_ctx_instance *nss_register_wifili_if(uint32_t if_num, nss_wifili_callback_t wifili_callback,
			nss_wifili_callback_t wifili_ext_callback,
			nss_wifili_msg_callback_t event_callback, struct net_device *netdev, uint32_t features)
{
	struct nss_ctx_instance *nss_ctx = (struct nss_ctx_instance *)&nss_top_main.nss[nss_top_main.wifi_handler_id];

	/*
	 * The interface number shall be wifili soc interface
	 */
	nss_assert((if_num == NSS_WIFILI_INTERNAL_INTERFACE)
			|| (if_num == NSS_WIFILI_EXTERNAL_INTERFACE0)
			|| (if_num == NSS_WIFILI_EXTERNAL_INTERFACE1));

	nss_info("nss_register_wifili_if if_num %d wifictx %px", if_num, netdev);

	nss_core_register_subsys_dp(nss_ctx, if_num, wifili_callback, wifili_ext_callback, NULL, netdev, features);

	nss_top_main.wifili_msg_callback = event_callback;

	return (struct nss_ctx_instance *)&nss_top_main.nss[nss_top_main.wifi_handler_id];
}
EXPORT_SYMBOL(nss_register_wifili_if);

/*
 * nss_unregister_wifili_if()
 *	Unregister wifili with nss driver
 */
void nss_unregister_wifili_if(uint32_t if_num)
{
	struct nss_ctx_instance *nss_ctx = (struct nss_ctx_instance *)&nss_top_main.nss[nss_top_main.wifi_handler_id];

	/*
	 * The interface number shall be wifili soc interface
	 */
	nss_assert((if_num == NSS_WIFILI_INTERNAL_INTERFACE)
			|| (if_num == NSS_WIFILI_EXTERNAL_INTERFACE0)
			|| (if_num == NSS_WIFILI_EXTERNAL_INTERFACE1));

	nss_core_unregister_subsys_dp(nss_ctx, if_num);
}
EXPORT_SYMBOL(nss_unregister_wifili_if);

/*
 * nss_register_wifili_radio_if()
 *	Register wifili radio with nss driver
 */
struct nss_ctx_instance *nss_register_wifili_radio_if(uint32_t if_num, nss_wifili_callback_t wifili_callback,
			nss_wifili_callback_t wifili_ext_callback,
			nss_wifili_msg_callback_t event_callback, struct net_device *netdev, uint32_t features)
{
	struct nss_ctx_instance *nss_ctx = (struct nss_ctx_instance *)&nss_top_main.nss[nss_top_main.wifi_handler_id];

	/*
	 * The interface number shall be wifili radio dynamic interface
	 */
	nss_assert(nss_is_dynamic_interface(if_num));
	nss_info("nss_register_wifili_if if_num %d wifictx %px", if_num, netdev);

	nss_core_register_subsys_dp(nss_ctx, if_num, wifili_callback, wifili_ext_callback, NULL, netdev, features);

	return (struct nss_ctx_instance *)&nss_top_main.nss[nss_top_main.wifi_handler_id];
}
EXPORT_SYMBOL(nss_register_wifili_radio_if);

/*
 * nss_unregister_wifili_radio_if()
 *	Unregister wifili radio with nss driver
 */
void nss_unregister_wifili_radio_if(uint32_t if_num)
{
	struct nss_ctx_instance *nss_ctx = (struct nss_ctx_instance *)&nss_top_main.nss[nss_top_main.wifi_handler_id];

	/*
	 * The interface number shall be wifili radio dynamic interface
	 */
	nss_assert(nss_is_dynamic_interface(if_num));

	nss_core_unregister_subsys_dp(nss_ctx, if_num);
}
EXPORT_SYMBOL(nss_unregister_wifili_radio_if);

/*
 * nss_wifili_register_handler()
 *	Register handle for notfication messages received on wifi interface
 */
void nss_wifili_register_handler(void)
{
	struct nss_ctx_instance *nss_ctx = (struct nss_ctx_instance *)&nss_top_main.nss[nss_top_main.wifi_handler_id];

	nss_info("nss_wifili_register_handler");
	nss_core_register_handler(nss_ctx, NSS_WIFILI_INTERNAL_INTERFACE, nss_wifili_handler, NULL);
#if (NSS_FW_VERSION_CODE > NSS_FW_VERSION(11,0))
	nss_core_register_handler(nss_ctx, NSS_WIFILI_EXTERNAL_INTERFACE0, nss_wifili_handler, NULL);
	nss_core_register_handler(nss_ctx, NSS_WIFILI_EXTERNAL_INTERFACE1, nss_wifili_handler, NULL);
#endif
	nss_wifili_stats_dentry_create();
	nss_wifili_strings_dentry_create();

	sema_init(&wifili_pvt.sem, 1);
	init_completion(&wifili_pvt.complete);
}
