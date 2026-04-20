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

#include "nss_stats.h"
#include "nss_core.h"
#include <nss_edma_lite.h>
#include "nss_strings.h"

/*
 * nss_edma_lite_strings_stats_node
 *	EDMA statistics strings.
 */
struct nss_stats_info nss_edma_lite_strings_stats_node[NSS_STATS_NODE_MAX] = {
	{"rx_pkts"		, NSS_STATS_TYPE_COMMON},
	{"rx_byts"		, NSS_STATS_TYPE_COMMON},
	{"tx_pkts"		, NSS_STATS_TYPE_COMMON},
	{"tx_byts"		, NSS_STATS_TYPE_COMMON},
	{"rx_queue[0]_drops"	, NSS_STATS_TYPE_DROP},
	{"rx_queue[1]_drops"	, NSS_STATS_TYPE_DROP},
	{"rx_queue[2]_drops"	, NSS_STATS_TYPE_DROP},
	{"rx_queue[3]_drops"	, NSS_STATS_TYPE_DROP}
};

/*
 * nss_edma_lite_node_stats_strings_read()
 *	Read EDMA node statistics names.
 */
static ssize_t nss_edma_lite_node_stats_strings_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	return nss_strings_print(ubuf, sz, ppos, nss_edma_lite_strings_stats_node, NSS_STATS_NODE_MAX);
}

/*
 * nss_edma_lite_node_stats_strings_ops
 */
NSS_STRINGS_DECLARE_FILE_OPERATIONS(edma_lite_node_stats);

/*
 * nss_edma_lite_strings_stats_tx
 */
struct nss_stats_info nss_edma_lite_strings_stats_tx[NSS_EDMA_LITE_STATS_TX_MAX] = {
	{"tx_err"	, NSS_STATS_TYPE_ERROR},
	{"tx_drops"	, NSS_STATS_TYPE_DROP},
	{"desc_cnt"	, NSS_STATS_TYPE_SPECIAL}
};

/*
 * nss_edma_lite_txring_strings_read()
 *      Read EDMA txring names.
 */
static ssize_t nss_edma_lite_txring_strings_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	return nss_strings_print(ubuf, sz, ppos, nss_edma_lite_strings_stats_tx, NSS_EDMA_LITE_STATS_TX_MAX);
}

/*
 * edma_txring_strings_ops
 */
NSS_STRINGS_DECLARE_FILE_OPERATIONS(edma_lite_txring);

/*
 * nss_edma_lite_strings_stats_rx
 */
struct nss_stats_info nss_edma_lite_strings_stats_rx[NSS_EDMA_LITE_STATS_RX_MAX] = {
	{"desc_cnt"		, NSS_STATS_TYPE_SPECIAL}
};

/*
 * nss_edma_lite_rxring_strings_read()
 *      Read EDMA rxring names.
 */
static ssize_t nss_edma_lite_rxring_strings_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	return nss_strings_print(ubuf, sz, ppos, nss_edma_lite_strings_stats_rx, NSS_EDMA_LITE_STATS_RX_MAX);
}

/*
 * edma_rxring_strings_ops
 */
NSS_STRINGS_DECLARE_FILE_OPERATIONS(edma_lite_rxring);

/*
 * nss_edma_lite_strings_stats_txcmpl
 */
struct nss_stats_info nss_edma_lite_strings_stats_txcmpl[NSS_EDMA_LITE_STATS_TXCMPL_MAX] = {
	{"desc_cnt"	, NSS_STATS_TYPE_SPECIAL}
};

/*
 * nss_edma_lite_txcmplring_strings_read()
 *      Read EDMA txcmplring names.
 */
static ssize_t nss_edma_lite_txcmplring_strings_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	return nss_strings_print(ubuf, sz, ppos, nss_edma_lite_strings_stats_txcmpl, NSS_EDMA_LITE_STATS_TXCMPL_MAX);
}

/*
 * edma_txcmplring_strings_ops
 */
NSS_STRINGS_DECLARE_FILE_OPERATIONS(edma_lite_txcmplring);

/*
 * nss_edma_lite_strings_stats_rxfill
 */
struct nss_stats_info nss_edma_lite_strings_stats_rxfill[NSS_EDMA_LITE_STATS_RXFILL_MAX] = {
	{"desc_cnt"	, NSS_STATS_TYPE_SPECIAL}
};

/*
 * nss_edma_lite_rxfillring_strings_read()
 *      Read EDMA rxfillring names.
 */
static ssize_t nss_edma_lite_rxfillring_strings_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	return nss_strings_print(ubuf, sz, ppos, nss_edma_lite_strings_stats_rxfill, NSS_EDMA_LITE_STATS_RXFILL_MAX);
}

/*
 * edma_rxfillring_strings_ops
 */
NSS_STRINGS_DECLARE_FILE_OPERATIONS(edma_lite_rxfillring);

/*
 * nss_edma_lite_strings_stats_err_map
 */
struct nss_stats_info nss_edma_lite_strings_stats_err_map[NSS_EDMA_LITE_ERR_STATS_MAX] = {
	{"alloc_fail_cnt"	, NSS_STATS_TYPE_ERROR},
	{"unknown_pkt_cnt"	, NSS_STATS_TYPE_ERROR},
};

/*
 * nss_edma_lite_err_strings_read()
 *      Read EDMA error names.
 */
static ssize_t nss_edma_lite_err_strings_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	return nss_strings_print(ubuf, sz, ppos, nss_edma_lite_strings_stats_err_map, NSS_EDMA_LITE_ERR_STATS_MAX);
}

/*
 * edma_err_strings_ops
 */
NSS_STRINGS_DECLARE_FILE_OPERATIONS(edma_lite_err);

/*
 * nss_edma_lite_strings_dentry_create()
 *      Create EDMA statistics strings debug entry.
 */
void nss_edma_lite_strings_dentry_create(void)
{
	struct dentry *edma_d;
	struct dentry *edma_rings_dir_d;
	struct dentry *file_d;

	if (!nss_top_main.strings_dentry) {
		nss_warning("qca-nss-drv/strings is not present");
		return;
	}

	edma_d = debugfs_create_dir("edma", nss_top_main.strings_dentry);
	if (!edma_d) {
		nss_warning("Failed to create qca-nss-drv/strings/edma directory");
		return;
	}

	/*
	 *  edma node stats
	 */
	file_d = debugfs_create_file("node_stats", 0400, edma_d, &nss_top_main, &nss_edma_lite_node_stats_strings_ops);
	if (!file_d) {
		nss_warning("Failed to create qca-nss-drv/strings/edma/node_stats file");
		goto fail;
	}

	/*
	 *  edma error stats
	 */
	file_d = debugfs_create_file("err_stats", 0400, edma_d, &nss_top_main, &nss_edma_lite_err_strings_ops);
	if (!file_d) {
		nss_warning("Failed to create qca-nss-drv/strings/edma/err_stats file");
		goto fail;
	}

	/*
	 * edma ring stats
	 */
	edma_rings_dir_d = debugfs_create_dir("rings", edma_d);
	if (!edma_rings_dir_d) {
		nss_warning("Failed to create qca-nss-drv/strings/edma/rings directory");
		goto fail;
	}

	/*
	 * edma tx ring stats
	 */
	file_d = debugfs_create_file("tx", 0400, edma_rings_dir_d, &nss_top_main, &nss_edma_lite_txring_strings_ops);
	if (!file_d) {
		nss_warning("Failed to create qca-nss-drv/strings/edma/rings/tx file");
		goto fail;
	}

	/*
	 * edma rx ring stats
	 */
	file_d = debugfs_create_file("rx", 0400, edma_rings_dir_d, &nss_top_main, &nss_edma_lite_rxring_strings_ops);
	if (!file_d) {
		nss_warning("Failed to create qca-nss-drv/strings/edma/rings/rx file");
		goto fail;
	}

	/*
	 * edma tx cmpl ring stats
	 */
	file_d = debugfs_create_file("txcmpl", 0400, edma_rings_dir_d, &nss_top_main, &nss_edma_lite_txcmplring_strings_ops);
	if (!file_d) {
		nss_warning("Failed to create qca-nss-drv/strings/edma/rings/txcmpl file");
		goto fail;
	}

	/*
	 * edma rx fill ring stats
	 */
	file_d = debugfs_create_file("rxfill", 0400, edma_rings_dir_d, &nss_top_main, &nss_edma_lite_rxfillring_strings_ops);
	if (!file_d) {
		nss_warning("Failed to create qca-nss-drv/strings/edma/rings/rxfill file");
		goto fail;
	}

	return;
fail:
	debugfs_remove_recursive(edma_d);
}
