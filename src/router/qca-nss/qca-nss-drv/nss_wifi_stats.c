/*
 **************************************************************************
 * Copyright (c) 2016-2017, The Linux Foundation. All rights reserved.
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

#include "nss_stats.h"
#include "nss_core.h"
#include "nss_wifi.h"
#include "nss_wifi_stats.h"

/*
 * nss_wifi_stats_str
 * 	Wifi statistics strings
 */
static int8_t *nss_wifi_stats_str[NSS_WIFI_STATS_MAX] = {
	"RX_PACKETS",
	"RX_QUEUE_0_DROPPED",
	"RX_QUEUE_1_DROPPED",
	"RX_QUEUE_2_DROPPED",
	"RX_QUEUE_3_DROPPED",
	"TX_PACKETS",
	"TX_DROPPED",
	"TX_TRANSMIT_COMPLETED",
	"TX_MGMT_RECEIVED",
	"TX_MGMT_TRANSMITTED",
	"TX_MGMT_DROPPED",
	"TX_MGMT_COMPLETED",
	"TX_INV_PEER_ENQ_CNT",
	"RX_INV_PEER_RCV_CNT",
	"RX_PN_CHECK_FAILED",
	"RX_PKTS_DELIVERD",
	"RX_BYTES_DELIVERED",
	"TX_BYTES_COMPLETED",
	"RX_DELIVER_UNALIGNED_DROP_CNT",
	"TIDQ_ENQUEUE_CNT_0",
	"TIDQ_ENQUEUE_CNT_1",
	"TIDQ_ENQUEUE_CNT_2",
	"TIDQ_ENQUEUE_CNT_3",
	"TIDQ_ENQUEUE_CNT_4",
	"TIDQ_ENQUEUE_CNT_5",
	"TIDQ_ENQUEUE_CNT_6",
	"TIDQ_ENQUEUE_CNT_7",
	"TIDQ_DEQUEUE_CNT_0",
	"TIDQ_DEQUEUE_CNT_1",
	"TIDQ_DEQUEUE_CNT_2",
	"TIDQ_DEQUEUE_CNT_3",
	"TIDQ_DEQUEUE_CNT_4",
	"TIDQ_DEQUEUE_CNT_5",
	"TIDQ_DEQUEUE_CNT_6",
	"TIDQ_DEQUEUE_CNT_7",
	"TIDQ_ENQUEUE_FAIL_CNT_0",
	"TIDQ_ENQUEUE_FAIL_CNT_1",
	"TIDQ_ENQUEUE_FAIL_CNT_2",
	"TIDQ_ENQUEUE_FAIL_CNT_3",
	"TIDQ_ENQUEUE_FAIL_CNT_4",
	"TIDQ_ENQUEUE_FAIL_CNT_5",
	"TIDQ_ENQUEUE_FAIL_CNT_6",
	"TIDQ_ENQUEUE_FAIL_CNT_7",
	"TIDQ_TTL_EXPIRE_CNT_0",
	"TIDQ_TTL_EXPIRE_CNT_1",
	"TIDQ_TTL_EXPIRE_CNT_2",
	"TIDQ_TTL_EXPIRE_CNT_3",
	"TIDQ_TTL_EXPIRE_CNT_4",
	"TIDQ_TTL_EXPIRE_CNT_5",
	"TIDQ_TTL_EXPIRE_CNT_6",
	"TIDQ_TTL_EXPIRE_CNT_7",
	"TIDQ_DEQUEUE_REQ_CNT_0",
	"TIDQ_DEQUEUE_REQ_CNT_1",
	"TIDQ_DEQUEUE_REQ_CNT_2",
	"TIDQ_DEQUEUE_REQ_CNT_3",
	"TIDQ_DEQUEUE_REQ_CNT_4",
	"TIDQ_DEQUEUE_REQ_CNT_5",
	"TIDQ_DEQUEUE_REQ_CNT_6",
	"TIDQ_DEQUEUE_REQ_CNT_7",
	"TOTAL_TIDQ_DEPTH",
	"RX_HTT_FETCH_CNT",
	"TOTAL_TIDQ_BYPASS_CNT",
	"GLOBAL_Q_FULL_CNT",
	"TIDQ_FULL_CNT",
};

uint64_t nss_wifi_stats[NSS_MAX_WIFI_RADIO_INTERFACES][NSS_WIFI_STATS_MAX]; /* WIFI statistics */

/*
 * nss_wifi_stats_read()
 *	Read wifi statistics
 */
static ssize_t nss_wifi_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	uint32_t i, id;

	/*
	 * max output lines = ((#stats + start tag + one blank) * #WIFI RADIOs) + start/end tag + 3 blank
	 */
	uint32_t max_output_lines = ((NSS_WIFI_STATS_MAX + 2) * NSS_MAX_WIFI_RADIO_INTERFACES) + 5;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	uint64_t *stats_shadow;

	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(lbuf == NULL)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	stats_shadow = kzalloc(NSS_WIFI_STATS_MAX * 8, GFP_KERNEL);
	if (unlikely(stats_shadow == NULL)) {
		nss_warning("Could not allocate memory for local shadow buffer");
		kfree(lbuf);
		return 0;
	}

	size_wr = scnprintf(lbuf, size_al, "wifi stats start:\n\n");

	for (id = 0; id < NSS_MAX_WIFI_RADIO_INTERFACES; id++) {
		spin_lock_bh(&nss_top_main.stats_lock);
		for (i = 0; (i < NSS_WIFI_STATS_MAX); i++) {
			stats_shadow[i] = nss_wifi_stats[id][i];
		}

		spin_unlock_bh(&nss_top_main.stats_lock);

		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "WIFI ID: %d\n", id);
		for (i = 0; (i < NSS_WIFI_STATS_MAX); i++) {
			size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
					"%s = %llu\n", nss_wifi_stats_str[i], stats_shadow[i]);
		}
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n");
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nwifi stats end\n\n");
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, strlen(lbuf));
	kfree(lbuf);
	kfree(stats_shadow);

	return bytes_read;
}

/*
 * nss_wifi_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(wifi)

/*
 * nss_wifi_stats_dentry_create()
 *	Create wifi statistics debug entry.
 */
void nss_wifi_stats_dentry_create(void)
{
	nss_stats_create_dentry("wifi", &nss_wifi_stats_ops);
}

/*
 * nss_wifi_stats_sync()
 *	Handle the syncing of WIFI stats.
 */
void nss_wifi_stats_sync(struct nss_ctx_instance *nss_ctx,
		struct nss_wifi_stats_sync_msg *stats, uint16_t interface)
{
	struct nss_top_instance *nss_top = nss_ctx->nss_top;
	uint32_t radio_id = interface - NSS_WIFI_INTERFACE0;
	uint8_t i = 0;

	if (radio_id >= NSS_MAX_WIFI_RADIO_INTERFACES) {
		nss_warning("%p: invalid interface: %d", nss_ctx, interface);
		return;
	}

	spin_lock_bh(&nss_top->stats_lock);

	/*
	 * Tx/Rx stats
	 */
	nss_wifi_stats[radio_id][NSS_WIFI_STATS_RX_PKTS] += stats->node_stats.rx_packets;
	for (i = 0; i < NSS_MAX_NUM_PRI; i++) {
		nss_wifi_stats[radio_id][NSS_WIFI_STATS_RX_QUEUE_0_DROPPED + i] += stats->node_stats.rx_dropped[i];
	}
	nss_wifi_stats[radio_id][NSS_WIFI_STATS_TX_PKTS] += stats->node_stats.tx_packets;
	nss_wifi_stats[radio_id][NSS_WIFI_STATS_TX_DROPPED] += stats->tx_transmit_dropped;
	nss_wifi_stats[radio_id][NSS_WIFI_STATS_TX_COMPLETED] += stats->tx_transmit_completions;
	nss_wifi_stats[radio_id][NSS_WIFI_STATS_MGMT_RCV_CNT] += stats->tx_mgmt_rcv_cnt;
	nss_wifi_stats[radio_id][NSS_WIFI_STATS_MGMT_TX_PKTS] += stats->tx_mgmt_pkts;
	nss_wifi_stats[radio_id][NSS_WIFI_STATS_MGMT_TX_DROPPED] += stats->tx_mgmt_dropped;
	nss_wifi_stats[radio_id][NSS_WIFI_STATS_MGMT_TX_COMPLETIONS] += stats->tx_mgmt_completions;
	nss_wifi_stats[radio_id][NSS_WIFI_STATS_TX_INV_PEER_ENQUEUE_CNT] += stats->tx_inv_peer_enq_cnt;
	nss_wifi_stats[radio_id][NSS_WIFI_STATS_RX_INV_PEER_RCV_CNT] += stats->rx_inv_peer_rcv_cnt;
	nss_wifi_stats[radio_id][NSS_WIFI_STATS_RX_PN_CHECK_FAILED] += stats->rx_pn_check_failed;
	nss_wifi_stats[radio_id][NSS_WIFI_STATS_RX_DELIVERED] += stats->rx_pkts_deliverd;
	nss_wifi_stats[radio_id][NSS_WIFI_STATS_RX_BYTES_DELIVERED] += stats->rx_bytes_deliverd;
	nss_wifi_stats[radio_id][NSS_WIFI_STATS_TX_BYTES_COMPLETED] += stats->tx_bytes_transmit_completions;
	nss_wifi_stats[radio_id][NSS_WIFI_STATS_RX_DELIVER_UNALIGNED_DROP_CNT] += stats->rx_deliver_unaligned_drop_cnt;

	for (i = 0; i < NSS_WIFI_TX_NUM_TOS_TIDS; i++) {
		nss_wifi_stats[radio_id][NSS_WIFI_STATS_TIDQ_ENQUEUE_CNT + i] += stats->tidq_enqueue_cnt[i];
		nss_wifi_stats[radio_id][NSS_WIFI_STATS_TIDQ_DEQUEUE_CNT + i] += stats->tidq_dequeue_cnt[i];
		nss_wifi_stats[radio_id][NSS_WIFI_STATS_TIDQ_ENQUEUE_FAIL_CNT + i] += stats->tidq_enqueue_fail_cnt[i];
		nss_wifi_stats[radio_id][NSS_WIFI_STATS_TIDQ_TTL_EXPIRE_CNT + i] += stats->tidq_ttl_expire_cnt[i];
		nss_wifi_stats[radio_id][NSS_WIFI_STATS_TIDQ_DEQUEUE_REQ_CNT + i] += stats->tidq_dequeue_req_cnt[i];
	}

	nss_wifi_stats[radio_id][NSS_WIFI_STATS_RX_HTT_FETCH_CNT] += stats->rx_htt_fetch_cnt;
	nss_wifi_stats[radio_id][NSS_WIFI_STATS_TOTAL_TIDQ_DEPTH] = stats->total_tidq_depth;
	nss_wifi_stats[radio_id][NSS_WIFI_STATS_TOTAL_TIDQ_BYPASS_CNT] += stats->total_tidq_bypass_cnt;
	nss_wifi_stats[radio_id][NSS_WIFI_STATS_GLOBAL_Q_FULL_CNT] += stats->global_q_full_cnt;
	nss_wifi_stats[radio_id][NSS_WIFI_STATS_TIDQ_FULL_CNT] += stats->tidq_full_cnt;

	spin_unlock_bh(&nss_top->stats_lock);
}
