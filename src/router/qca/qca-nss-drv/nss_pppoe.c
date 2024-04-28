/*
 **************************************************************************
 * Copyright (c) 2013-2018, The Linux Foundation. All rights reserved.
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
 * nss_pppoe.c
 *	NSS PPPoE APIs
 */

#include "nss_tx_rx_common.h"
#include "nss_pppoe_stats.h"
#include "nss_pppoe_log.h"

#define NSS_PPPOE_TX_TIMEOUT 3000 /* 3 Seconds */

/*
 * Data structures to store pppoe nss debug stats
 */
static DEFINE_SPINLOCK(nss_pppoe_lock);
static struct nss_pppoe_stats_session_debug nss_pppoe_debug_stats[NSS_MAX_PPPOE_DYNAMIC_INTERFACES];

/*
 * Private data structure
 */
static struct nss_pppoe_pvt {
	struct semaphore sem;
	struct completion complete;
	int response;
	void *cb;
	void *app_data;
} pppoe_pvt;

/*
 * nss_pppoe_debug_stats_sync
 *	Per session debug stats for pppoe
 */
static void nss_pppoe_debug_stats_sync(struct nss_ctx_instance *nss_ctx, struct nss_pppoe_sync_stats_msg *stats_msg, uint16_t if_num)
{
	int i;
	spin_lock_bh(&nss_pppoe_lock);
	for (i = 0; i < NSS_MAX_PPPOE_DYNAMIC_INTERFACES; i++) {
		if (nss_pppoe_debug_stats[i].if_num == if_num) {
			nss_pppoe_debug_stats[i].stats[NSS_PPPOE_STATS_RX_PACKETS] += stats_msg->stats.rx_packets;
			nss_pppoe_debug_stats[i].stats[NSS_PPPOE_STATS_RX_BYTES] += stats_msg->stats.rx_bytes;
			nss_pppoe_debug_stats[i].stats[NSS_PPPOE_STATS_TX_PACKETS] += stats_msg->stats.tx_packets;
			nss_pppoe_debug_stats[i].stats[NSS_PPPOE_STATS_TX_BYTES] += stats_msg->stats.tx_bytes;
			nss_pppoe_debug_stats[i].stats[NSS_PPPOE_STATS_SESSION_WRONG_VERSION_OR_TYPE] += stats_msg->exception_events[NSS_PPPOE_EXCEPTION_EVENT_WRONG_VERSION_OR_TYPE];
			nss_pppoe_debug_stats[i].stats[NSS_PPPOE_STATS_SESSION_WRONG_CODE] += stats_msg->exception_events[NSS_PPPOE_EXCEPTION_EVENT_WRONG_CODE];
			nss_pppoe_debug_stats[i].stats[NSS_PPPOE_STATS_SESSION_UNSUPPORTED_PPP_PROTOCOL] += stats_msg->exception_events[NSS_PPPOE_EXCEPTION_EVENT_UNSUPPORTED_PPP_PROTOCOL];
			break;
		}
	}
	spin_unlock_bh(&nss_pppoe_lock);
}
/*
 * nss_pppoe_get_context()
 */
struct nss_ctx_instance *nss_pppoe_get_context(void)
{
	return (struct nss_ctx_instance *)&nss_top_main.nss[nss_top_main.pppoe_handler_id];
}
EXPORT_SYMBOL(nss_pppoe_get_context);

/*
 * nss_pppoe_tx_msg()
 *	Transmit a PPPoE message to NSS firmware
 */
static nss_tx_status_t nss_pppoe_tx_msg(struct nss_ctx_instance *nss_ctx, struct nss_pppoe_msg *msg)
{
	struct nss_cmn_msg *ncm = &msg->cm;

	/*
	 * Trace Messages
	 */
	nss_pppoe_log_tx_msg(msg);

	/*
	 * Sanity check the message
	 */
	if (!nss_is_dynamic_interface(ncm->interface)) {
		nss_warning("%p: tx request for non dynamic interface: %d\n", nss_ctx, ncm->interface);
		return NSS_TX_FAILURE;
	}

	if (nss_dynamic_interface_get_type(nss_pppoe_get_context(), ncm->interface) != NSS_DYNAMIC_INTERFACE_TYPE_PPPOE) {
		nss_warning("%p: tx request for not PPPoE interface: %d\n", nss_ctx, ncm->interface);
		return NSS_TX_FAILURE;
	}

	if (ncm->type >= NSS_PPPOE_MSG_MAX) {
		nss_warning("%p: message type out of range: %d\n", nss_ctx, ncm->type);
		return NSS_TX_FAILURE;
	}

	return nss_core_send_cmd(nss_ctx, msg, sizeof(*msg), NSS_NBUF_PAYLOAD_SIZE);
}

/*
 * nss_pppoe_sync_msg_callback()
 *	Callback to handle the completion of NSS->HLOS messages.
 */
static void nss_pppoe_sync_msg_callback(void *app_data, struct nss_pppoe_msg *npm)
{
	nss_pppoe_msg_callback_t callback = (nss_pppoe_msg_callback_t)pppoe_pvt.cb;
	void *data = pppoe_pvt.app_data;

	pppoe_pvt.cb = NULL;
	pppoe_pvt.app_data = NULL;

	pppoe_pvt.response = NSS_TX_SUCCESS;
	if (npm->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_warning("pppoe Error response %d\n", npm->cm.response);
		pppoe_pvt.response = NSS_TX_FAILURE;
	}

	if (callback) {
		callback(data, npm);
	}

	complete(&pppoe_pvt.complete);
}

/*
 * nss_pppoe_handler()
 *	Handle NSS -> HLOS messages for PPPoE
 */
static void nss_pppoe_handler(struct nss_ctx_instance *nss_ctx, struct nss_cmn_msg *ncm, __attribute__((unused))void *app_data)
{
	struct nss_pppoe_msg *nim = (struct nss_pppoe_msg *)ncm;
	void *ctx;
	nss_pppoe_msg_callback_t cb;

	BUG_ON(!(nss_is_dynamic_interface(ncm->interface) || ncm->interface == NSS_PPPOE_INTERFACE));

	/*
	 * Trace Messages
	 */
	nss_pppoe_log_rx_msg(nim);

	/*
	 * Sanity check the message type
	 */
	if (ncm->type >= NSS_PPPOE_MSG_MAX) {
		nss_warning("%p: message type out of range: %d\n", nss_ctx, ncm->type);
		return;
	}

	if (nss_cmn_get_msg_len(ncm) > sizeof(struct nss_pppoe_msg)) {
		nss_warning("%p: message length is invalid: %d\n", nss_ctx, nss_cmn_get_msg_len(ncm));
		return;
	}

	/*
	 * Log failures
	 */
	nss_core_log_msg_failures(nss_ctx, ncm);

	/*
	 * Handling PPPoE messages coming from NSS fw.
	 */
	switch (nim->cm.type) {
	case NSS_PPPOE_MSG_SYNC_STATS:
		nss_pppoe_debug_stats_sync(nss_ctx, &nim->msg.sync_stats, ncm->interface);
		break;
	default:
		nss_warning("%p: Received response %d for type %d, interface %d\n",
				nss_ctx, ncm->response, ncm->type, ncm->interface);
	}

	/*
	 * Update the callback and app_data for NOTIFY messages, pppoe sends all notify messages
	 * to the same callback/app_data.
	 */
	if (ncm->response == NSS_CMN_RESPONSE_NOTIFY) {
		ncm->cb = (nss_ptr_t)nss_ctx->nss_top->pppoe_msg_callback;
		ncm->app_data = (nss_ptr_t)nss_ctx->subsys_dp_register[ncm->interface].app_data;
	}

	/*
	 * Log failures
	 */
	nss_core_log_msg_failures(nss_ctx, ncm);

	/*
	 * Do we have a call back
	 */
	if (!ncm->cb) {
		return;
	}

	/*
	 * callback
	 */
	cb = (nss_pppoe_msg_callback_t)ncm->cb;
	ctx = (void *)ncm->app_data;

	cb(ctx, nim);
}

/*
 * nss_pppoe_debug_stats_get()
 *	Get session pppoe statistics.
 */
void nss_pppoe_debug_stats_get(void *stats_mem)
{
	struct nss_pppoe_stats_session_debug *stats = (struct nss_pppoe_stats_session_debug *)stats_mem;
	int i;

	if (!stats) {
		nss_warning("No memory to copy pppoe session stats\n");
		return;
	}

	spin_lock_bh(&nss_pppoe_lock);
	for (i = 0; i < NSS_MAX_PPPOE_DYNAMIC_INTERFACES; i++) {
		if (nss_pppoe_debug_stats[i].valid) {
			memcpy(stats, &nss_pppoe_debug_stats[i], sizeof(struct nss_pppoe_stats_session_debug));
			stats++;
		}
	}
	spin_unlock_bh(&nss_pppoe_lock);
}

/*
 * nss_pppoe_tx_msg_sync()
 */
nss_tx_status_t nss_pppoe_tx_msg_sync(struct nss_ctx_instance *nss_ctx,
						struct nss_pppoe_msg *msg)
{
	nss_tx_status_t status;
	int ret = 0;

	down(&pppoe_pvt.sem);
	pppoe_pvt.cb = (void *)msg->cm.cb;
	pppoe_pvt.app_data = (void *)msg->cm.app_data;

	msg->cm.cb = (nss_ptr_t)nss_pppoe_sync_msg_callback;
	msg->cm.app_data = (nss_ptr_t)NULL;

	status = nss_pppoe_tx_msg(nss_ctx, msg);
	if (status != NSS_TX_SUCCESS) {
		nss_warning("%p: nss_pppoe_tx_msg failed\n", nss_ctx);
		up(&pppoe_pvt.sem);
		return status;
	}

	ret = wait_for_completion_timeout(&pppoe_pvt.complete, msecs_to_jiffies(NSS_PPPOE_TX_TIMEOUT));
	if (!ret) {
		nss_warning("%p: PPPoE msg tx failed due to timeout\n", nss_ctx);
		pppoe_pvt.response = NSS_TX_FAILURE;
	}

	status = pppoe_pvt.response;
	up(&pppoe_pvt.sem);
	return status;
}
EXPORT_SYMBOL(nss_pppoe_tx_msg_sync);

/*
 * nss_register_pppoe_session_if()
 */
struct nss_ctx_instance *nss_register_pppoe_session_if(uint32_t if_num,
						       nss_pppoe_msg_callback_t notification_callback,
						       struct net_device *netdev, uint32_t features, void *app_ctx)
{
	struct nss_ctx_instance *nss_ctx = nss_pppoe_get_context();
	int i = 0;

	nss_assert(nss_ctx);
	nss_assert(nss_is_dynamic_interface(if_num));

	spin_lock_bh(&nss_pppoe_lock);
	for (i = 0; i < NSS_MAX_PPPOE_DYNAMIC_INTERFACES; i++) {
		if (!nss_pppoe_debug_stats[i].valid) {
			nss_pppoe_debug_stats[i].valid = true;
			nss_pppoe_debug_stats[i].if_num = if_num;
			nss_pppoe_debug_stats[i].if_index = netdev->ifindex;
			break;
		}
	}
	spin_unlock_bh(&nss_pppoe_lock);

	if (i == NSS_MAX_PPPOE_DYNAMIC_INTERFACES) {
		return NULL;
	}

	nss_core_register_subsys_dp(nss_ctx, if_num, NULL, NULL, app_ctx, netdev, features);

	nss_top_main.pppoe_msg_callback = notification_callback;

	nss_core_register_handler(nss_ctx, if_num, nss_pppoe_handler, NULL);

	return nss_ctx;
}
EXPORT_SYMBOL(nss_register_pppoe_session_if);

/*
 * nss_unregister_pppoe_session_if()
 */
void nss_unregister_pppoe_session_if(uint32_t if_num)
{
	struct nss_ctx_instance *nss_ctx = nss_pppoe_get_context();
	int i;

	nss_assert(nss_ctx);
	nss_assert(nss_is_dynamic_interface(if_num));

	spin_lock_bh(&nss_pppoe_lock);
	for (i = 0; i < NSS_MAX_PPPOE_DYNAMIC_INTERFACES; i++) {
		if (nss_pppoe_debug_stats[i].if_num == if_num) {
			nss_pppoe_debug_stats[i].valid = false;
			nss_pppoe_debug_stats[i].if_num = 0;
			nss_pppoe_debug_stats[i].if_index = 0;
		}
	}
	spin_unlock_bh(&nss_pppoe_lock);

	nss_core_unregister_subsys_dp(nss_ctx, if_num);

	nss_top_main.pppoe_msg_callback = NULL;

	nss_core_unregister_handler(nss_ctx, if_num);

}
EXPORT_SYMBOL(nss_unregister_pppoe_session_if);

/*
 * nss_pppoe_register_handler()
 */
void nss_pppoe_register_handler(void)
{
	int i;

	nss_info("nss_pppoe_register_handler\n");
	nss_core_register_handler(nss_pppoe_get_context(), NSS_PPPOE_INTERFACE, nss_pppoe_handler, NULL);

	spin_lock_bh(&nss_pppoe_lock);
	for (i = 0; i < NSS_MAX_PPPOE_DYNAMIC_INTERFACES; i++) {
		nss_pppoe_debug_stats[i].valid = false;
		nss_pppoe_debug_stats[i].if_num = 0;
		nss_pppoe_debug_stats[i].if_index = 0;
	}
	spin_unlock_bh(&nss_pppoe_lock);

	sema_init(&pppoe_pvt.sem, 1);
	init_completion(&pppoe_pvt.complete);

	nss_pppoe_stats_dentry_create();
}

/*
 * nss_pppoe_msg_init()
 */
void nss_pppoe_msg_init(struct nss_pppoe_msg *npm, uint16_t if_num, uint32_t type, uint32_t len,
			void *cb, void *app_data)
{
	nss_cmn_msg_init(&npm->cm, if_num, type, len, (void *)cb, app_data);

}
EXPORT_SYMBOL(nss_pppoe_msg_init);
