/*
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include "nss_tx_rx_common.h"
#include "nss_trustsec_rx_stats.h"
#include "nss_trustsec_rx_log.h"

#define NSS_TRUSTSEC_RX_TIMEOUT 3000 /* 3 Seconds */

/*
 * Private data structure for trustsec_rx interface
 */
static struct nss_trustsec_rx_pvt {
	struct semaphore sem;
	struct completion complete;
	int response;
} ttx;

/*
 * nss_trustsec_rx_handler()
 *	Handle NSS -> HLOS messages for trustsec_rx
 */
static void nss_trustsec_rx_handler(struct nss_ctx_instance *nss_ctx, struct nss_cmn_msg *ncm,
				__attribute__((unused))void *app_data)
{
	nss_trustsec_rx_msg_callback_t cb;
	struct nss_trustsec_rx_msg *msg = (struct nss_trustsec_rx_msg *)ncm;

	BUG_ON(ncm->interface != NSS_TRUSTSEC_RX_INTERFACE);

	/*
	 * Trace messages.
	 */
	nss_trustsec_rx_log_rx_msg(msg);

	/*
	 * Is this a valid request/response packet?
	 */
	if (ncm->type >= NSS_TRUSTSEC_RX_MSG_MAX) {
		nss_warning("%px: received invalid message %d for trustsec_rx interface", nss_ctx, ncm->type);
		return;
	}

	if (nss_cmn_get_msg_len(ncm) > sizeof(struct nss_trustsec_rx_msg)) {
		nss_warning("%px: message size incorrect: %d", nss_ctx, nss_cmn_get_msg_len(ncm));
		return;
	}

	/*
	 * Log failures
	 */
	nss_core_log_msg_failures(nss_ctx, ncm);

	switch (ncm->type) {
	case NSS_TRUSTSEC_RX_MSG_STATS_SYNC:
		/*
		 * Update trustsec_rx statistics.
		 */
		nss_trustsec_rx_stats_sync(nss_ctx, &msg->msg.stats_sync);
		break;
	}

	/*
	 * Update the callback and app_data for NOTIFY messages, trustsec_rx sends all notify messages
	 * to the same callback/app_data.
	 */
	if (ncm->response == NSS_CMN_RESPONSE_NOTIFY) {
		ncm->cb = (nss_ptr_t)nss_core_get_msg_handler(nss_ctx, ncm->interface);
		ncm->app_data = (nss_ptr_t)nss_ctx->subsys_dp_register[ncm->interface].ndev;
	}

	/*
	 * Do we have a call back
	 */
	if (!ncm->cb) {
		return;
	}

	/*
	 * callback
	 */
	cb = (nss_trustsec_rx_msg_callback_t)ncm->cb;

	cb((void *)ncm->app_data, msg);
}

/*
 * nss_trustsec_rx_msg()
 *	Transmit a trustsec_rx message to NSSFW
 */
static nss_tx_status_t nss_trustsec_rx_msg(struct nss_ctx_instance *nss_ctx, struct nss_trustsec_rx_msg *msg)
{
	struct nss_cmn_msg *ncm = &msg->cm;

	/*
	 * Trace messages.
	 */
	nss_trustsec_rx_log_tx_msg(msg);

	/*
	 * Sanity check the message
	 */
	if (ncm->interface != NSS_TRUSTSEC_RX_INTERFACE) {
		nss_warning("%px: tx request for another interface: %d", nss_ctx, ncm->interface);
		return NSS_TX_FAILURE;
	}

	if (ncm->type > NSS_TRUSTSEC_RX_MSG_MAX) {
		nss_warning("%px: message type out of range: %d", nss_ctx, ncm->type);
		return NSS_TX_FAILURE;
	}

	return nss_core_send_cmd(nss_ctx, msg, sizeof(*msg), NSS_NBUF_PAYLOAD_SIZE);
}

/*
 * nss_trustsec_rx_callback
 *	Callback to handle the completion of NSS ->HLOS messages.
 */
static void nss_trustsec_rx_callback(void *app_data, struct nss_trustsec_rx_msg *msg)
{
	if (msg->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_warning("trustsec_rx error response %d\n", msg->cm.response);
		ttx.response = NSS_TX_FAILURE;
		complete(&ttx.complete);
		return;
	}

	ttx.response = NSS_TX_SUCCESS;
	complete(&ttx.complete);
}

/*
 * nss_trustsec_rx_msg_sync()
 *	Send a message to trustsec_rx interface & wait for the response.
 */
nss_tx_status_t nss_trustsec_rx_msg_sync(struct nss_ctx_instance *nss_ctx, struct nss_trustsec_rx_msg *msg)
{
	nss_tx_status_t status;
	int ret = 0;

	down(&ttx.sem);

	msg->cm.cb = (nss_ptr_t)nss_trustsec_rx_callback;
	msg->cm.app_data = (nss_ptr_t)NULL;

	status = nss_trustsec_rx_msg(nss_ctx, msg);
	if (status != NSS_TX_SUCCESS) {
		nss_warning("%px: nss_trustsec_rx_msg failed\n", nss_ctx);
		up(&ttx.sem);
		return status;
	}

	ret = wait_for_completion_timeout(&ttx.complete, msecs_to_jiffies(NSS_TRUSTSEC_RX_TIMEOUT));
	if (!ret) {
		nss_warning("%px: trustsec_rx tx failed due to timeout\n", nss_ctx);
		ttx.response = NSS_TX_FAILURE;
	}

	status = ttx.response;
	up(&ttx.sem);

	return status;
}
EXPORT_SYMBOL(nss_trustsec_rx_msg_sync);

/*
 * nss_trustsec_rx_get_ctx()
 *	Return a TrustSec TX NSS context.
 */
struct nss_ctx_instance *nss_trustsec_rx_get_ctx()
{
	return &nss_top_main.nss[nss_top_main.trustsec_rx_handler_id];
}
EXPORT_SYMBOL(nss_trustsec_rx_get_ctx);

/*
 * nss_trustsec_rx_msg_init()
 *	Initialize trustsec_rx message.
 */
void nss_trustsec_rx_msg_init(struct nss_trustsec_rx_msg *msg, uint16_t if_num, uint32_t type, uint32_t len,
				nss_trustsec_rx_msg_callback_t cb, void *app_data)
{
	nss_cmn_msg_init(&msg->cm, if_num, type, len, (void *)cb, app_data);
}
EXPORT_SYMBOL(nss_trustsec_rx_msg_init);

/*
 * nss_trustsec_rx_register_handler()
 *	Registering handler for sending msg to trustsec_rx node on NSS.
 */
void nss_trustsec_rx_register_handler(void)
{
	struct nss_ctx_instance *nss_ctx = nss_trustsec_rx_get_ctx();

	nss_core_register_handler(nss_ctx, NSS_TRUSTSEC_RX_INTERFACE, nss_trustsec_rx_handler, NULL);

	nss_trustsec_rx_stats_dentry_create();

	sema_init(&ttx.sem, 1);
	init_completion(&ttx.complete);
}
