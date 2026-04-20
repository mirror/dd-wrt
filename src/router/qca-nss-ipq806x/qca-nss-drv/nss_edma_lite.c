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

/*
 * nss_edma_lite.c
 *	NSS EDMA APIs
 */
#include "nss_edma_lite_stats.h"
#include "nss_edma_lite_strings.h"
#include <nss_dp_api_if.h>

/*
 **********************************
 Rx APIs
 **********************************
 */
static bool is_map_configured = false;

/*
 * nss_edma_lite_interface_handler()
 *	Handle NSS -> HLOS messages for EDMA node
 */
static void nss_edma_lite_interface_handler(struct nss_ctx_instance *nss_ctx, struct nss_cmn_msg *ncm, __attribute__((unused))void *app_data)
{
	struct nss_edma_lite_msg *nelm = (struct nss_edma_lite_msg *)ncm;
	nss_edma_lite_msg_callback_t cb;

	/*
	 * Is this a valid request/response packet?
	 */
	if (nelm->cm.type >= NSS_EDMA_LITE_MSG_TYPE_MAX) {
		nss_warning("%px: received invalid message %d for edma_lite interface", nss_ctx, nelm->cm.type);
		return;
	}

	/*
	 * Handle different types of messages
	 */
	switch (nelm->cm.type) {
	case NSS_EDMA_LITE_MSG_NODE_STATS_SYNC:
		nss_edma_lite_node_stats_sync(nss_ctx, &nelm->msg.node_stats);
		break;
	case NSS_EDMA_LITE_MSG_RING_STATS_SYNC:
		nss_edma_lite_ring_stats_sync(nss_ctx, &nelm->msg.ring_stats);
		break;
	case NSS_EDMA_LITE_MSG_ERR_STATS_SYNC:
		nss_edma_lite_err_stats_sync(nss_ctx, &nelm->msg.err_stats);
		break;
	default:
		if (ncm->response != NSS_CMN_RESPONSE_ACK) {
			/*
			 * Check response
			 */
			nss_info("%px: Received response %d for type %d, interface %d",
						nss_ctx, ncm->response, ncm->type, ncm->interface);
		}
	}

	/*
	 * Update the callback and app_data for NOTIFY messages, edma_lite sends all notify messages
	 * to the same callback/app_data.
	 */
	if (nelm->cm.response == NSS_CMN_RESPONSE_NOTIFY) {
		ncm->cb = (nss_ptr_t)nss_ctx->edma_lite_callback;
		ncm->app_data = (nss_ptr_t)nss_ctx->edma_lite_ctx;
	}

	/*
	 * Do we have a callback?
	 */
	if (!ncm->cb) {
		return;
	}

	/*
	 * Callback
	 */
	cb = (nss_edma_lite_msg_callback_t)ncm->cb;
	cb((void *)ncm->app_data, nelm);
}

/*
 * nss_edma_lite_msg_cfg_map_callback()
 */
static void nss_edma_lite_msg_cfg_map_callback(void *app_data, struct nss_edma_lite_msg *nelm)
{
	struct nss_ctx_instance *nss_ctx __attribute__((unused)) = (struct nss_ctx_instance *)app_data;
	if (nelm->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_warning("%px: nss edma_lite_map configuration failed: %d for NSS core %d\n",
				nss_ctx, nelm->cm.error, nss_ctx->id);
		return;
	}

	/* TODO: SHould we protect this with some lock */
	if (nelm->cm.type == NSS_EDMA_LITE_MSG_TYPE_RING_MAP) {
		is_map_configured = true;
	}

	nss_info("%px: nss edma_lite_map configuration succeeded for NSS core %d\n", nss_ctx, nss_ctx->id);
}

/*
 * nss_edma_lite_tx_msg()
 *	Transmit an EDMA lite config message to the FW with a specified size.
 */
nss_tx_status_t nss_edma_lite_tx_msg(struct nss_ctx_instance *nss_ctx, struct nss_edma_lite_msg *nelm)
{
	struct nss_cmn_msg *ncm = &nelm->cm;

	if (ncm->type >= NSS_EDMA_LITE_MSG_TYPE_MAX) {
		nss_warning("%px: message type out of range: %d", nss_ctx, ncm->type);
		return NSS_TX_FAILURE;
	}

	return nss_core_send_cmd(nss_ctx, nelm, sizeof(*nelm), NSS_NBUF_PAYLOAD_SIZE);
}
EXPORT_SYMBOL(nss_edma_lite_tx_msg);

bool nss_edma_lite_is_configured(void)
{
	return is_map_configured;
}
EXPORT_SYMBOL(nss_edma_lite_is_configured);

/*
 * nss_edma_lite_msg_cfg_map()
 *	Send ring number to EDMA lite.
 */
nss_tx_status_t nss_edma_lite_msg_cfg_map(struct nss_ctx_instance *nss_ctx)
{
	int32_t status;
	struct nss_edma_lite_msg nelm;
	struct nss_edma_lite_ring_map *cfg_map;
	uint32_t txdesc_num, txcmpl_num, rxfill_num, rxdesc_num;

	nss_edma_lite_msg_init(&nelm, NSS_EDMA_LITE_INTERFACE, NSS_EDMA_LITE_MSG_TYPE_RING_MAP,
		sizeof(struct nss_edma_lite_ring_map), nss_edma_lite_msg_cfg_map_callback, (void *)nss_ctx);

	/*
	 * Invoke DP API to get point offload information
	 */
	nss_dp_point_offload_info_get(&txdesc_num, &txcmpl_num, &rxfill_num, &rxdesc_num);

	cfg_map = &nelm.msg.map;
	cfg_map->txdesc_num = txdesc_num;
	cfg_map->txcmpl_num = txcmpl_num;
	cfg_map->rxfill_num = rxfill_num;
	cfg_map->rxdesc_num = rxdesc_num;

	status = nss_edma_lite_tx_msg(nss_ctx, &nelm);
	if (unlikely(status != NSS_TX_SUCCESS)) {
		return status;
	}

	return NSS_TX_SUCCESS;
}

/*
 * nss_edma_lite_msg_init()
 *	Initialize EDMA LITE message.
 */
void nss_edma_lite_msg_init(struct nss_edma_lite_msg *nelm, uint16_t if_num, uint32_t type, uint32_t len,
			nss_edma_lite_msg_callback_t cb, void *app_data)
{
	nss_cmn_msg_init(&nelm->cm, if_num, type, len, (void *)cb, app_data);
}
EXPORT_SYMBOL(nss_edma_lite_msg_init);

/*
 * nss_edma_lite_notify_register()
 *	Register to received EDMA events.
 */
void nss_edma_lite_notify_register(nss_edma_lite_msg_callback_t cb, void *app_data)
{
	struct nss_ctx_instance *nss_ctx;
	int i = 0;

	for (i = 0; i < NSS_MAX_CORES; i++) {
		int id = nss_top_main.edma_lite_handler_id[i];
		if (id >= 0) {
			nss_ctx = &nss_top_main.nss[id];
			nss_ctx->edma_lite_callback = cb;
			nss_ctx->edma_lite_ctx = app_data;
		}
	}
}
EXPORT_SYMBOL(nss_edma_lite_notify_register);

/*
 * nss_edma_lite_notify_unregister()
 *	Unregister to received EDMA events.
 */
void nss_edma_lite_notify_unregister(void)
{
	struct nss_ctx_instance *nss_ctx;
	int i = 0;

	for (i = 0; i < NSS_MAX_CORES; i++) {
		int id = nss_top_main.edma_lite_handler_id[i];
		if (id >= 0) {
			nss_ctx = &nss_top_main.nss[id];
			nss_ctx->edma_lite_callback = NULL;
		}
	}
}
EXPORT_SYMBOL(nss_edma_lite_notify_unregister);

/*
 * nss_edma_lite_enabled()
 */
bool nss_edma_lite_enabled(struct nss_ctx_instance *nss_ctx)
{
	int id = nss_top_main.edma_lite_handler_id[nss_ctx->id];
	if (id == nss_ctx->id)
		return true;

	return false;
}
EXPORT_SYMBOL(nss_edma_lite_enabled);

/*
 * nss_edma_lite_register_handler()
 */
void nss_edma_lite_register_handler(struct nss_ctx_instance *nss_ctx)
{
	nss_core_register_handler(nss_ctx, NSS_EDMA_LITE_INTERFACE, nss_edma_lite_interface_handler, NULL);

	nss_edma_lite_stats_dentry_create();
	nss_edma_lite_strings_dentry_create();
}
