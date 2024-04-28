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
#include "nss_pppoe_stats.h"

/*
 * nss_pppoe_stats_str
 *	PPPoE stats strings
 */
static int8_t *nss_pppoe_stats_debug_str[NSS_PPPOE_STATS_SESSION_MAX] = {
	"RX_PACKETS",
	"RX_BYTES",
	"TX_PACKETS",
	"TX_BYTES",
	"WRONG_VERSION_OR_TYPE",
	"WRONG_CODE",
	"UNSUPPORTED_PPP_PROTOCOL",
};

/*
 * nss_pppoe_stats_read()
 *	Read pppoe statistics
 */
static ssize_t nss_pppoe_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{

	uint32_t max_output_lines = 2 /* header & footer for session stats */
					+ NSS_MAX_PPPOE_DYNAMIC_INTERFACES * (NSS_PPPOE_STATS_SESSION_MAX + 2) /*session stats */
					+ 2;
	size_t size_al = NSS_STATS_MAX_STR_LENGTH * max_output_lines;
	size_t size_wr = 0;
	ssize_t bytes_read = 0;
	struct net_device *dev;
	struct nss_pppoe_stats_session_debug pppoe_session_stats[NSS_MAX_PPPOE_DYNAMIC_INTERFACES];
	int id, i;

	char *lbuf = kzalloc(size_al, GFP_KERNEL);
	if (unlikely(lbuf == NULL)) {
		nss_warning("Could not allocate memory for local statistics buffer");
		return 0;
	}

	memset(&pppoe_session_stats, 0, sizeof(struct nss_pppoe_stats_session_debug) * NSS_MAX_PPPOE_DYNAMIC_INTERFACES);

	/*
	 * Get all stats
	 */
	nss_pppoe_debug_stats_get((void *)&pppoe_session_stats);

	/*
	 * Session stats
	 */
	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\npppoe session stats start:\n\n");
	for (id = 0; id < NSS_MAX_PPPOE_DYNAMIC_INTERFACES; id++) {
		if (!pppoe_session_stats[id].valid) {
			break;
		}

		dev = dev_get_by_index(&init_net, pppoe_session_stats[id].if_index);
		if (likely(dev)) {
			size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "%d. nss interface id=%d, netdevice=%s\n", id,
					pppoe_session_stats[id].if_num, dev->name);
			dev_put(dev);
		} else {
			size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "%d. nss interface id=%d\n", id,
					pppoe_session_stats[id].if_num);
		}

		for (i = 0; i < NSS_PPPOE_STATS_SESSION_MAX; i++) {
			size_wr += scnprintf(lbuf + size_wr, size_al - size_wr,
					     "\t%s = %llu\n", nss_pppoe_stats_debug_str[i],
					      pppoe_session_stats[id].stats[i]);
		}
		size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\n");
	}

	size_wr += scnprintf(lbuf + size_wr, size_al - size_wr, "\npppoe session stats end\n");
	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, lbuf, size_wr);

	kfree(lbuf);
	return bytes_read;
}

/*
 * nss_pppoe_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(pppoe)

/*
 * nss_pppoe_stats_dentry_create()
 *	Create PPPoE node statistics debug entry.
 */
void nss_pppoe_stats_dentry_create(void)
{
	nss_stats_create_dentry("pppoe", &nss_pppoe_stats_ops);
}

