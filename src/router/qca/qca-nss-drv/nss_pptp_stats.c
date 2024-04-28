/*
 **************************************************************************
 * Copyright (c) 2016-2018, The Linux Foundation. All rights reserved.
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
#include "nss_pptp_stats.h"

struct nss_pptp_stats_session_debug pptp_session_stats[NSS_MAX_PPTP_DYNAMIC_INTERFACES];

/*
 * nss_pptp_stats_session_debug_str
 *	PPTP statistics strings for nss session stats
 */
static int8_t *nss_pptp_stats_session_debug_str[NSS_PPTP_STATS_SESSION_MAX] = {
	"ENCAP_RX_PACKETS",
	"ENCAP_RX_BYTES",
	"ENCAP_TX_PACKETS",
	"ENCAP_TX_BYTES",
	"ENCAP_RX_QUEUE_0_DROP",
	"ENCAP_RX_QUEUE_1_DROP",
	"ENCAP_RX_QUEUE_2_DROP",
	"ENCAP_RX_QUEUE_3_DROP",
	"DECAP_RX_PACKETS",
	"DECAP_RX_BYTES",
	"DECAP_TX_PACKETS",
	"DECAP_TX_BYTES",
	"DECAP_RX_QUEUE_0_DROP",
	"DECAP_RX_QUEUE_1_DROP",
	"DECAP_RX_QUEUE_2_DROP",
	"DECAP_RX_QUEUE_3_DROP",
	"ENCAP_HEADROOM_ERR",
	"ENCAP_SMALL_SIZE",
	"ENCAP_PNODE_ENQUEUE_FAIL",
	"DECAP_NO_SEQ_NOR_ACK",
	"DECAP_INVAL_GRE_FLAGS",
	"DECAP_INVAL_GRE_PROTO",
	"DECAP_WRONG_SEQ",
	"DECAP_INVAL_PPP_HDR",
	"DECAP_PPP_LCP",
	"DECAP_UNSUPPORTED_PPP_PROTO",
	"DECAP_PNODE_ENQUEUE_FAIL",
};

/*
 * nss_pptp_stats_read()
 *	Read pptp statistics
 */
static ssize_t nss_pptp_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{

	uint32_t max_output_lines = 2 /* header & footer for session stats */
					+ NSS_MAX_PPTP_DYNAMIC_INTERFACES * (NSS_PPTP_STATS_SESSION_MAX + 2) /*session stats */
					+ 2;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines ;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	struct net_device *dev;
	int id, i;

	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(lbuf == NULL)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	memset(&pptp_session_stats, 0, sizeof(struct nss_pptp_stats_session_debug) * NSS_MAX_PPTP_DYNAMIC_INTERFACES);

	/*
	 * Get all stats
	 */
	nss_pptp_session_debug_stats_get((void *)&pptp_session_stats);

	/*
	 * Session stats
	 */
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\npptp session stats start:\n\n");
	for (id = 0; id < NSS_MAX_PPTP_DYNAMIC_INTERFACES; id++) {

			if (!pptp_session_stats[id].valid) {
				break;
			}

			dev = dev_get_by_index(&init_net, pptp_session_stats[id].if_index);
			if (likely(dev)) {

				size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "%d. nss interface id=%d, netdevice=%s\n", id,
						pptp_session_stats[id].if_num, dev->name);
				dev_put(dev);
			} else {
				size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "%d. nss interface id=%d\n", id,
						pptp_session_stats[id].if_num);
			}

			for (i = 0; i < NSS_PPTP_STATS_SESSION_MAX; i++) {
				size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
						     "\t%s = %llu\n", nss_pptp_stats_session_debug_str[i],
						      pptp_session_stats[id].stats[i]);
			}
			size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n");
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\npptp session stats end\n");
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, size_wr);

	kfree(lbuf);
	return bytes_read;
}

/*
 * nss_pptp_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(pptp)

/*
 * nss_pptp_stats_dentry_create()
 *	Create PPTP node statistics debug entry.
 */
void nss_pptp_stats_dentry_create(void)
{
	nss_stats_create_dentry("pptp", &nss_pptp_stats_ops);
}
