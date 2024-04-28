/*
 **************************************************************************
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
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
#include "nss_dtls_stats.h"

/*
 * nss_dtls_stats_session_str
 *	DTLS statistics strings for nss session stats
 */
static int8_t *nss_dtls_stats_session_str[NSS_DTLS_STATS_SESSION_MAX] = {
	"RX_PKTS",
	"TX_PKTS",
	"RX_DROPPED_0",
	"RX_DROPPED_1",
	"RX_DROPPED_2",
	"RX_DROPPED_3",
	"RX_AUTH_DONE",
	"TX_AUTH_DONE",
	"RX_CIPHER_DONE",
	"TX_CIPHER_DONE",
	"RX_CBUF_ALLOC_FAIL",
	"TX_CBUF_ALLOC_FAIL",
	"TX_CENQUEUE_FAIL",
	"RX_CENQUEUE_FAIL",
	"TX_DROPPED_HROOM",
	"TX_DROPPED_TROOM",
	"TX_FORWARD_ENQUEUE_FAIL",
	"RX_FORWARD_ENQUEUE_FAIL",
	"RX_INVALID_VERSION",
	"RX_INVALID_EPOCH",
	"RX_MALFORMED",
	"RX_CIPHER_FAIL",
	"RX_AUTH_FAIL",
	"RX_CAPWAP_CLASSIFY_FAIL",
	"RX_SINGLE_REC_DGRAM",
	"RX_MULTI_REC_DGRAM",
	"RX_REPLAY_FAIL",
	"RX_REPLAY_DUPLICATE",
	"RX_REPLAY_OUT_OF_WINDOW",
	"OUTFLOW_QUEUE_FULL",
	"DECAP_QUEUE_FULL",
	"PBUF_ALLOC_FAIL",
	"PBUF_COPY_FAIL",
	"EPOCH",
	"TX_SEQ_HIGH",
	"TX_SEQ_LOW",
};

/*
 * nss_dtls_stats_read()
 * 	Read DTLS session statistics
 */
static ssize_t nss_dtls_stats_read(struct file *fp, char __user *ubuf,
				   size_t sz, loff_t *ppos)
{
	uint32_t max_output_lines = 2 + (NSS_MAX_DTLS_SESSIONS
					* (NSS_DTLS_STATS_SESSION_MAX + 2)) + 2;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	struct net_device *dev;
	int id, i;
	struct nss_dtls_stats_session *dtls_session_stats = NULL;

	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(lbuf == NULL)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	dtls_session_stats = kzalloc((sizeof(struct nss_dtls_stats_session)
				     * NSS_MAX_DTLS_SESSIONS), GFP_KERNEL);
	if (unlikely(dtls_session_stats == NULL)) {
		nss_warning("Could not allocate memory for populating DTLS stats");
		kfree(lbuf);
		return 0;
	}

	/*
	 * Get all stats
	 */
	nss_dtls_session_stats_get(dtls_session_stats);

	/*
	 * Session stats
	 */
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
			     "\nDTLS session stats start:\n\n");

	for (id = 0; id < NSS_MAX_DTLS_SESSIONS; id++) {
		if (!dtls_session_stats[id].valid)
			break;

		dev = dev_get_by_index(&init_net, dtls_session_stats[id].if_index);
		if (likely(dev)) {
			size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
					     "%d. nss interface id=%d, netdevice=%s\n",
					     id, dtls_session_stats[id].if_num,
					     dev->name);
			dev_put(dev);
		} else {
			size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
					     "%d. nss interface id=%d\n", id,
					     dtls_session_stats[id].if_num);
		}

		for (i = 0; i < NSS_DTLS_STATS_SESSION_MAX; i++) {
			size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
					     "\t%s = %llu\n",
					     nss_dtls_stats_session_str[i],
					     dtls_session_stats[id].stats[i]);
		}

		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n");
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
			     "\nDTLS session stats end\n");
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, size_wr);

	kfree(dtls_session_stats);
	kfree(lbuf);
	return bytes_read;
}

/*
 * nss_dtls_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(dtls)

/*
 * nss_dtls_stats_dentry_create()
 *	Create DTLS statistics debug entry.
 */
void nss_dtls_stats_dentry_create(void)
{
	nss_stats_create_dentry("dtls", &nss_dtls_stats_ops);
}
