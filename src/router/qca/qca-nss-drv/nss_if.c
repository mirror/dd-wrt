/*
 **************************************************************************
 * Copyright (c) 2014-2016, 2018, The Linux Foundation. All rights reserved.
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
 * nss_if.c
 *	NSS base interfaces
 */

#include "nss_tx_rx_common.h"

/*
 * nss_if_msg_handler()
 *	Handle NSS -> HLOS messages for base class interfaces
 */
void nss_if_msg_handler(struct nss_ctx_instance *nss_ctx, struct nss_cmn_msg *ncm,
		__attribute__((unused))void *app_data)
{
	struct nss_if_msg *nim = (struct nss_if_msg *)ncm;
	nss_if_msg_callback_t cb;

	/*
	 * We only support base class messages with this interface
	 */
	if (ncm->type > NSS_IF_MAX_MSG_TYPES) {
		nss_warning("%p: message type out of range: %d", nss_ctx, ncm->type);
		return;
	}

	if (!nss_is_dynamic_interface(ncm->interface) &&
		!((ncm->interface >= NSS_PHYSICAL_IF_START) && (ncm->interface < NSS_VIRTUAL_IF_START))) {
		nss_warning("%p: interface %d not in physical or dynamic if range\n", nss_ctx, ncm->interface);
		return;
	}

	if (nss_cmn_get_msg_len(ncm) > sizeof(struct nss_if_msg)) {
		nss_warning("%p: message length too big: %d", nss_ctx, nss_cmn_get_msg_len(ncm));
		return;
	}

	/*
	 * Log failures
	 */
	nss_core_log_msg_failures(nss_ctx, ncm);

	/*
	 * Do we have a callback?
	 */
	if (!ncm->cb) {
		return;
	}

	/*
	 * Callback
	 */
	cb = (nss_if_msg_callback_t)ncm->cb;
	cb((void *)ncm->app_data, nim);
}

/*
 * nss_if_tx_buf()
 *	Send packet to interface owned by NSS
 */
nss_tx_status_t nss_if_tx_buf(struct nss_ctx_instance *nss_ctx, struct sk_buff *os_buf, uint32_t if_num)
{
	nss_trace("%p: If Tx packet, id:%d, data=%p", nss_ctx, if_num, os_buf->data);

	if (!nss_is_dynamic_interface(if_num) &&
		!((if_num >= NSS_PHYSICAL_IF_START) && (if_num < NSS_VIRTUAL_IF_START))) {
		nss_warning("%p: interface %d not in physical or dynamic if range\n", nss_ctx, if_num);
		return NSS_TX_FAILURE_BAD_PARAM;
	}

	return nss_core_send_packet(nss_ctx, os_buf, if_num, 0);
}

/*
 * nss_if_tx_msg()
 *	Transmit a message to the specific interface on this core.
 */
nss_tx_status_t nss_if_tx_msg(struct nss_ctx_instance *nss_ctx, struct nss_if_msg *nim)
{
	struct nss_cmn_msg *ncm = &nim->cm;
	struct net_device *dev;

	NSS_VERIFY_CTX_MAGIC(nss_ctx);

	/*
	 * Sanity check the message
	 */
	if (ncm->type > NSS_IF_MAX_MSG_TYPES) {
		nss_warning("%p: message type out of range: %d", nss_ctx, ncm->type);
		return NSS_TX_FAILURE;
	}

	dev = nss_ctx->subsys_dp_register[ncm->interface].ndev;
	if (!dev) {
		nss_warning("%p: Unregister interface %d: no context", nss_ctx, ncm->interface);
		return NSS_TX_FAILURE_BAD_PARAM;
	}

	return nss_core_send_cmd(nss_ctx, nim, sizeof(*nim), NSS_NBUF_PAYLOAD_SIZE);
}

/*
 * nss_if_register()
 *	Primary registration for receiving data and msgs from an interface.
 */
struct nss_ctx_instance *nss_if_register(uint32_t if_num,
				nss_if_rx_callback_t rx_callback,
				nss_if_msg_callback_t msg_callback,
				struct net_device *if_ctx)
{
	return NULL;
}

/*
 * nss_if_unregister()
 *	Unregisteer the callback for this interface
 */
void nss_if_unregister(uint32_t if_num)
{
}

EXPORT_SYMBOL(nss_if_tx_msg);
EXPORT_SYMBOL(nss_if_register);
EXPORT_SYMBOL(nss_if_unregister);
