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
#include "nss_portid_stats.h"

extern spinlock_t nss_portid_spinlock;
extern struct nss_portid_handle nss_portid_hdl[];

/*
 * nss_portid_stats_str
 *	PortID statistics strings
 */
static int8_t *nss_portid_stats_str[NSS_PORTID_STATS_MAX] = {
	"RX_INVALID_HEADER",
};

uint64_t nss_portid_stats[NSS_PORTID_STATS_MAX];

/*
 * nss_portid_stats_read()
 *	Read PortID stats
 */
static ssize_t nss_portid_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	int32_t i;
	/*
	 * max output lines = #stats + start tag line + end tag line + three blank lines
	 */
	uint32_t max_output_lines = NSS_STATS_NODE_MAX + NSS_PORTID_STATS_MAX + 5;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	uint64_t *stats_shadow;

	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(lbuf == NULL)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	stats_shadow = kzalloc(NSS_STATS_NODE_MAX * 8, GFP_KERNEL);
	if (unlikely(stats_shadow == NULL)) {
		nss_warning("Could not allocate memory for local shadow buffer");
		kfree(lbuf);
		return 0;
	}

	size_wr = scnprintf(lbuf, size_al, "portid stats start:\n\n");

	size_wr = nss_stats_fill_common_stats(NSS_PORTID_INTERFACE, lbuf, size_wr, size_al);

	/*
	 * PortID node stats
	 */
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nportid node stats:\n\n");

	spin_lock_bh(&nss_top_main.stats_lock);
	for (i = 0; (i < NSS_PORTID_STATS_MAX); i++) {
		stats_shadow[i] = nss_portid_stats[i];
	}

	spin_unlock_bh(&nss_top_main.stats_lock);

	for (i = 0; (i < NSS_PORTID_STATS_MAX); i++) {
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
					"%s = %llu\n", nss_portid_stats_str[i], stats_shadow[i]);
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\nportid stats end\n\n");

	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, strlen(lbuf));
	kfree(lbuf);
	kfree(stats_shadow);

	return bytes_read;
}

/*
 * nss_portid_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(portid)

/*
 * nss_portid_stats_dentry_create()
 *	Create portid node statistics debug entry.
 */
void nss_portid_stats_dentry_create(void)
{
	nss_stats_create_dentry("portid", &nss_portid_stats_ops);
}

/*
 * nss_portid_stats_sync()
 *	Update portid node stats.
 */
void nss_portid_stats_sync(struct nss_ctx_instance *nss_ctx, struct nss_portid_stats_sync_msg *npsm)
{
	struct nss_top_instance *nss_top = nss_ctx->nss_top;
	struct nss_portid_handle *hdl;
	int j;

	if (npsm->port_id == NSS_PORTID_MAX_SWITCH_PORT) {
		/*
		 * Update PORTID base node stats.
		 */
		spin_lock_bh(&nss_top->stats_lock);
		nss_top->stats_node[NSS_PORTID_INTERFACE][NSS_STATS_NODE_RX_PKTS] += npsm->node_stats.rx_packets;
		nss_top->stats_node[NSS_PORTID_INTERFACE][NSS_STATS_NODE_RX_BYTES] += npsm->node_stats.rx_bytes;
		nss_top->stats_node[NSS_PORTID_INTERFACE][NSS_STATS_NODE_TX_PKTS] += npsm->node_stats.tx_packets;
		nss_top->stats_node[NSS_PORTID_INTERFACE][NSS_STATS_NODE_TX_BYTES] += npsm->node_stats.tx_bytes;

		for (j = 0; j < NSS_MAX_NUM_PRI; j++) {
			nss_top->stats_node[NSS_PORTID_INTERFACE][NSS_STATS_NODE_RX_QUEUE_0_DROPPED + j] += npsm->node_stats.rx_dropped[j];
		}

		nss_portid_stats[NSS_PORTID_STATS_RX_INVALID_HEADER] += npsm->rx_invalid_header;
		spin_unlock_bh(&nss_top->stats_lock);
		return;
	}

	if (npsm->port_id >= NSS_PORTID_MAX_SWITCH_PORT) {
		nss_warning("port_id %d exceeds NSS_PORTID_MAX_SWITCH_PORT\n", npsm->port_id);
		return;
	}

	/*
	 * Update PORTID interface stats.
	 */
	spin_lock_bh(&nss_portid_spinlock);
	hdl = &nss_portid_hdl[npsm->port_id];
	if (hdl->if_num == 0) {
		nss_warning("%p: nss_portid recv'd stats with unconfigured port %d", nss_ctx, npsm->port_id);
		spin_unlock_bh(&nss_portid_spinlock);
		return;
	}
	hdl->stats.rx_packets += npsm->node_stats.rx_packets;
	hdl->stats.rx_bytes += npsm->node_stats.rx_bytes;
	hdl->stats.rx_dropped += nss_cmn_rx_dropped_sum(&npsm->node_stats);
	hdl->stats.tx_packets += npsm->node_stats.tx_packets;
	hdl->stats.tx_bytes += npsm->node_stats.tx_bytes;
	spin_unlock_bh(&nss_portid_spinlock);
}
