/*
 **************************************************************************
 * Copyright (c) 2019-2020, The Linux Foundation. All rights reserved.
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
#include "nss_gre_redir_mark_stats.h"
#include "nss_gre_redir_mark_log.h"
#define NSS_GRE_REDIR_MARK_TX_TIMEOUT 3000 /* 3 Seconds */

/*
 * Private data structure for handling synchronous messaging.
 */
static struct {
	struct semaphore sem;
	struct completion complete;
	int response;
} nss_gre_redir_mark_pvt;

/*
 * Spinlock to update GRE redir mark stats.
 */
static DEFINE_SPINLOCK(nss_gre_redir_mark_stats_lock);

/*
 * Global GRE redir mark stats structure.
 */
static struct nss_gre_redir_mark_stats gre_mark_stats;

/*
 * nss_gre_redir_mark_msg_sync_callback()
 *	Callback to handle the completion of HLOS-->NSS messages.
 */
static void nss_gre_redir_mark_msg_sync_callback(void *app_data, struct nss_gre_redir_mark_msg *nim)
{
	nss_gre_redir_mark_pvt.response = NSS_TX_SUCCESS;
	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_warning("GRE mark Error response %d\n", nim->cm.response);
		nss_gre_redir_mark_pvt.response = NSS_TX_FAILURE;
	}

	complete(&nss_gre_redir_mark_pvt.complete);
}

/*
 * nss_gre_redir_mark_stats_sync()
 *	Update GRE redir mark stats.
 */
static void nss_gre_redir_mark_stats_sync(struct nss_ctx_instance *nss_ctx, int if_num, struct nss_gre_redir_mark_stats_sync_msg *ngss)
{
	struct net_device *dev;
	dev = nss_cmn_get_interface_dev(nss_ctx, if_num);
	if (!dev) {
		nss_warning("%px: Unable to find net device for the interface %d\n", nss_ctx, if_num);
		return;
	}

	if (if_num != NSS_GRE_REDIR_MARK_INTERFACE) {
		nss_warning("%px: Unknown type for interface %d\n", nss_ctx, if_num);
		return;
	}

	/*
	 * Update the stats in exclusive mode to prevent the read from the process
	 * context through debug fs.
	 */
	spin_lock_bh(&nss_gre_redir_mark_stats_lock);

	/*
	 * Update the common node stats
	 */
	gre_mark_stats.stats[NSS_GRE_REDIR_MARK_STATS_TX_PKTS] += ngss->node_stats.tx_packets;
	gre_mark_stats.stats[NSS_GRE_REDIR_MARK_STATS_TX_BYTES] += ngss->node_stats.tx_bytes;
	gre_mark_stats.stats[NSS_GRE_REDIR_MARK_STATS_RX_PKTS] += ngss->node_stats.rx_packets;
	gre_mark_stats.stats[NSS_GRE_REDIR_MARK_STATS_RX_BYTES] += ngss->node_stats.rx_bytes;
	gre_mark_stats.stats[NSS_GRE_REDIR_MARK_STATS_RX_DROPS] += nss_cmn_rx_dropped_sum(&(ngss->node_stats));

	/*
	 * Update the GRE redir mark specific stats
	 */
	gre_mark_stats.stats[NSS_GRE_REDIR_MARK_STATS_HLOS_MAGIC_FAILED] += ngss->hlos_magic_fail;
	gre_mark_stats.stats[NSS_GRE_REDIR_MARK_STATS_INV_DST_IF_DROPS] += ngss->invalid_dst_drop;
	gre_mark_stats.stats[NSS_GRE_REDIR_MARK_STATS_DST_IF_ENQUEUE] += ngss->dst_enqueue_success;
	gre_mark_stats.stats[NSS_GRE_REDIR_MARK_STATS_DST_IF_ENQUEUE_DROPS] += ngss->dst_enqueue_drop;
	gre_mark_stats.stats[NSS_GRE_REDIR_MARK_STATS_INV_APPID] += ngss->inv_appid;
	gre_mark_stats.stats[NSS_GRE_REDIR_MARK_STATS_HEADROOM_UNAVAILABLE] += ngss->headroom_unavail;
	gre_mark_stats.stats[NSS_GRE_REDIR_MARK_STATS_TX_COMPLETION_SUCCESS] += ngss->tx_completion_success;
	gre_mark_stats.stats[NSS_GRE_REDIR_MARK_STATS_TX_COMPLETION_DROPS] += ngss->tx_completion_drop;

	spin_unlock_bh(&nss_gre_redir_mark_stats_lock);
}

/*
 * nss_gre_redir_mark_handler()
 *	Handle NSS to HLOS messages for GRE redir mark
 */
static void nss_gre_redir_mark_handler(struct nss_ctx_instance *nss_ctx, struct nss_cmn_msg *ncm, void *app_data)
{
	struct nss_gre_redir_mark_msg *ngrm = (struct nss_gre_redir_mark_msg *)ncm;
	nss_gre_redir_mark_msg_callback_t cb;

	NSS_VERIFY_CTX_MAGIC(nss_ctx);

	/*
	 * Is this a valid request/response packet?
	 */
	if (ncm->type >= NSS_GRE_REDIR_MARK_MSG_MAX) {
		nss_warning("%px: received invalid message %d for GRE redir mark interface", nss_ctx, ncm->type);
		return;
	}

	if (nss_cmn_get_msg_len(ncm) > sizeof(struct nss_gre_redir_mark_msg)) {
		nss_warning("%px: length of message is greater than required: %d", nss_ctx, nss_cmn_get_msg_len(ncm));
		return;
	}

	/*
	 * Log failures
	 */
	nss_core_log_msg_failures(nss_ctx, ncm);

	/*
	 * Trace messages.
	 */
	nss_gre_redir_mark_log_rx_msg((struct nss_gre_redir_mark_msg *)ncm);

	if (ncm->type == NSS_GRE_REDIR_MARK_STATS_SYNC_MSG) {
		nss_gre_redir_mark_stats_sync(nss_ctx, ncm->interface, &ngrm->msg.stats_sync);
	}

	/*
	 * Update the callback and app_data for NOTIFY messages, GRE redir mark sends all notify messages
	 * to the same callback/app_data. The app data here represent the netdevice of the GRE redir mark
	 * interface.
	 */
	if (ncm->response == NSS_CMN_RESPONSE_NOTIFY) {
		ncm->cb = (nss_ptr_t)nss_core_get_msg_handler(nss_ctx, ncm->interface);
		ncm->app_data = (nss_ptr_t)nss_ctx->subsys_dp_register[ncm->interface].ndev;
	}

	/*
	 * load and call the registered synchronous message callback.
	 */
	cb = (nss_gre_redir_mark_msg_callback_t)ncm->cb;
	if (unlikely(!cb)) {
		return;
	}

	cb((void *)ncm->app_data, ncm);
}

/*
 * nss_gre_redir_mark_get_stats()
 *	Get gre_redir tunnel stats.
 */
bool nss_gre_redir_mark_get_stats(void *stats_mem)
{
	struct nss_gre_redir_mark_stats *stats = (struct nss_gre_redir_mark_stats *)stats_mem;
	if (!stats) {
		nss_warning("No memory to copy GRE redir mark stats");
		return false;
	}

	/*
	 * Copy the GRE redir mark stats in the memory.
	 */
	spin_lock_bh(&nss_gre_redir_mark_stats_lock);
	memcpy(stats, &gre_mark_stats, sizeof(struct nss_gre_redir_mark_stats));
	spin_unlock_bh(&nss_gre_redir_mark_stats_lock);
	return true;
}
EXPORT_SYMBOL(nss_gre_redir_mark_get_stats);

/*
 * nss_gre_redir_mark_reg_cb()
 *	Configure a callback on VAP.
 */
nss_tx_status_t nss_gre_redir_mark_reg_cb(int ifnum,
                struct nss_gre_redir_mark_register_cb_msg *ngrcm)
{
        struct nss_gre_redir_mark_msg config;
        struct nss_ctx_instance *nss_ctx __maybe_unused = nss_gre_redir_mark_get_context();
        nss_tx_status_t status;
        uint32_t vap_type;
        uint32_t len = sizeof(struct nss_gre_redir_mark_register_cb_msg);

        if (!nss_ctx) {
                nss_warning("Unable to retrieve NSS context.\n");
                return NSS_TX_FAILURE_BAD_PARAM;
        }

        vap_type = nss_dynamic_interface_get_type(nss_ctx, ngrcm->nss_if_num);
        if ((vap_type != NSS_DYNAMIC_INTERFACE_TYPE_VAP)) {
                nss_warning("%px: Incorrect type for vap interface type = %u", nss_ctx, vap_type);
                return NSS_TX_FAILURE_BAD_PARAM;
        }

        /*
         * Configure the node
         */
        nss_cmn_msg_init(&config.cm, NSS_GRE_REDIR_MARK_INTERFACE, NSS_GRE_REDIR_MARK_REG_CB_MSG, len, NULL, NULL);
        config.msg.reg_cb_msg.nss_if_num = ngrcm->nss_if_num;

        status = nss_gre_redir_mark_tx_msg_sync(nss_ctx, &config);
        if (status != NSS_TX_SUCCESS) {
                nss_warning("%px: Unable to register callback from GRE redir mark interface %d\n", nss_ctx, ifnum);
        }

        return status;
}
EXPORT_SYMBOL(nss_gre_redir_mark_reg_cb);

/*
 * nss_gre_redir_mark_tx_msg()
 *	Transmit a GRE MARK configuration message to NSS FW.
 */
nss_tx_status_t nss_gre_redir_mark_tx_msg(struct nss_ctx_instance *nss_ctx, struct nss_gre_redir_mark_msg *msg)
{
	struct nss_cmn_msg *ncm = &msg->cm;

	/*
	 * Trace Messages
	 */
	nss_gre_redir_mark_log_tx_msg(msg);

	/*
	 * interface should be of type of redir mark
	 */
	if (ncm->interface != NSS_GRE_REDIR_MARK_INTERFACE) {
		nss_warning("%px: tx request for another interface: %d", nss_ctx, ncm->interface);
		return NSS_TX_FAILURE;
	}

	if (ncm->type >= NSS_GRE_REDIR_MARK_MSG_MAX) {
		nss_warning("%px: message type out of range: %d", nss_ctx, ncm->type);
		return NSS_TX_FAILURE;
	}

	return nss_core_send_cmd(nss_ctx, msg, sizeof(*msg), NSS_NBUF_PAYLOAD_SIZE);
}
EXPORT_SYMBOL(nss_gre_redir_mark_tx_msg);

/*
 * nss_gre_redir_mark_tx_msg_sync()
 *	Transmit a GRE redir mark message to NSS firmware synchronously.
 */
nss_tx_status_t nss_gre_redir_mark_tx_msg_sync(struct nss_ctx_instance *nss_ctx, struct nss_gre_redir_mark_msg *ngrm)
{
	nss_tx_status_t status;
	int ret = 0;

	/*
	 * Decrease the semaphore count to send the message exclusively.
	 */
	down(&nss_gre_redir_mark_pvt.sem);
	ngrm->cm.cb = (nss_ptr_t)nss_gre_redir_mark_msg_sync_callback;
	ngrm->cm.app_data = (nss_ptr_t)NULL;
	status = nss_gre_redir_mark_tx_msg(nss_ctx, ngrm);
	if (status != NSS_TX_SUCCESS) {
		nss_warning("%px: GRE redir mark tx_msg failed\n", nss_ctx);
		up(&nss_gre_redir_mark_pvt.sem);
		return status;
	}

	ret = wait_for_completion_timeout(&nss_gre_redir_mark_pvt.complete, msecs_to_jiffies(NSS_GRE_REDIR_MARK_TX_TIMEOUT));
	if (!ret) {
		nss_warning("%px: GRE redir mark message tx sync failed due to timeout\n", nss_ctx);
		nss_gre_redir_mark_pvt.response = NSS_TX_FAILURE;
	}

	status = nss_gre_redir_mark_pvt.response;
	up(&nss_gre_redir_mark_pvt.sem);
	return status;
}
EXPORT_SYMBOL(nss_gre_redir_mark_tx_msg_sync);

/*
 * nss_gre_redir_mark_tx_buf()
 *	Send packet to GRE redir mark interface owned by NSS.
 */
nss_tx_status_t nss_gre_redir_mark_tx_buf(struct nss_ctx_instance *nss_ctx, struct sk_buff *os_buf, uint32_t if_num)
{
	nss_trace("%px: GRE redir mark If Tx packet, interface id:%d, data=%px", nss_ctx, if_num, os_buf->data);

	/*
	 * We expect Tx packets to the GRE redir mark interface only.
	 */
	if (if_num != NSS_GRE_REDIR_MARK_INTERFACE) {
		nss_warning("%px: Invalid interface:%d for GRE redir mark packets\n", nss_ctx, if_num);
		return NSS_TX_FAILURE_BAD_PARAM;
	}

	return nss_core_send_packet(nss_ctx, os_buf, if_num, 0);
}
EXPORT_SYMBOL(nss_gre_redir_mark_tx_buf);

/*
 * nss_gre_redir_mark_get_context()
 *	Return NSS GRE redir mark context.
 */
struct nss_ctx_instance *nss_gre_redir_mark_get_context(void)
{
	return &nss_top_main.nss[nss_top_main.gre_redir_mark_handler_id];
}
EXPORT_SYMBOL(nss_gre_redir_mark_get_context);

/*
 * nss_gre_redir_mark_unregister_if()
 *	Unregister dynamic node for GRE_REDIR_MARK redir.
 */
bool nss_gre_redir_mark_unregister_if(uint32_t if_num)
{
	struct nss_ctx_instance *nss_ctx __maybe_unused = (struct nss_ctx_instance *)&nss_top_main.nss[nss_top_main.gre_redir_handler_id];
	struct net_device *dev;
	uint32_t status;

	nss_assert(nss_ctx);
	nss_assert(if_num == NSS_GRE_REDIR_MARK_INTERFACE);

	dev = nss_cmn_get_interface_dev(nss_ctx, if_num);

	BUG_ON(!dev);

	nss_core_unregister_subsys_dp(nss_ctx, if_num);
	status = nss_core_unregister_msg_handler(nss_ctx, if_num);
	if (status != NSS_CORE_STATUS_SUCCESS) {
		nss_warning("%px: Not able to unregister handler for gre_redir_mark interface %d with NSS core\n",
				nss_ctx, if_num);
		return false;
	}

	nss_ctx->nss_rx_interface_handlers[nss_ctx->id][if_num].msg_cb = NULL;
	return true;
}
EXPORT_SYMBOL(nss_gre_redir_mark_unregister_if);

/*
 * nss_gre_redir_mark_register_if()
 *	Register staticr GRE redir mark interface with data-plane.
 */
struct nss_ctx_instance *nss_gre_redir_mark_register_if(struct net_device *netdev, uint32_t if_num,
		nss_gre_redir_mark_data_callback_t cb_func_data, nss_gre_redir_mark_msg_callback_t cb_func_msg,
		uint32_t features)
{
	struct nss_ctx_instance *nss_ctx = (struct nss_ctx_instance *)&nss_top_main.nss[nss_top_main.gre_redir_handler_id];
	uint32_t status;

	nss_assert(nss_ctx);
	nss_assert(if_num == NSS_GRE_REDIR_MARK_INTERFACE);

	/*
	 * Registering the interface with network data path.
	 */
	nss_core_register_subsys_dp(nss_ctx, if_num, cb_func_data, NULL, NULL, netdev, features);
	status = nss_core_register_msg_handler(nss_ctx, NSS_GRE_REDIR_MARK_INTERFACE, cb_func_msg);
	if (status != NSS_CORE_STATUS_SUCCESS) {
		nss_warning("%px: Not able to register handler for gre_redir_mark interface %d with NSS core\n",
				nss_ctx, if_num);
		return NULL;
	}

	return nss_ctx;
}
EXPORT_SYMBOL(nss_gre_redir_mark_register_if);

/*
 * nss_gre_redir_mark_get_device()
 *	Gets the original device from probe.
 */
struct device *nss_gre_redir_mark_get_device(void)
{
	struct nss_ctx_instance *nss_ctx = nss_gre_redir_mark_get_context();
	return nss_ctx->dev;
}
EXPORT_SYMBOL(nss_gre_redir_mark_get_device);

/*
 * nss_gre_redir_mark_register_handler()
 *	Register GRE redir mark and register handler
 */
void nss_gre_redir_mark_register_handler(void)
{
	struct nss_ctx_instance *nss_ctx = nss_gre_redir_mark_get_context();
	struct dentry *gre_redir_mark_dentry = NULL;
	uint32_t status = NSS_CORE_STATUS_FAILURE;

	/*
	 * Create the debug fs entry for the stats.
	 */
	gre_redir_mark_dentry = nss_gre_redir_mark_stats_dentry_create();
	if (!gre_redir_mark_dentry) {
		nss_warning("%px: Not able to create debugfs entry\n", nss_ctx);
		return;
	}

	sema_init(&nss_gre_redir_mark_pvt.sem, 1);
	init_completion(&nss_gre_redir_mark_pvt.complete);

	nss_info("nss_gre_redir_mark_register_handler\n");
	status = nss_core_register_handler(nss_ctx, NSS_GRE_REDIR_MARK_INTERFACE, nss_gre_redir_mark_handler, NULL);
	if (status != NSS_CORE_STATUS_SUCCESS) {
		debugfs_remove_recursive(gre_redir_mark_dentry);
		gre_redir_mark_dentry = NULL;
		nss_warning("%px: Not able to register handler for GRE redir mark with NSS core\n", nss_ctx);
		return;
	}
}
