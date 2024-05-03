/*
 **************************************************************************
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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
 * @file NSSINFO wifili handler
 */
#include "nssinfo.h"
#include <nss_wifili_if.h>
#include <nss_nlwifili_if.h>

static pthread_mutex_t wifili_lock;
static struct nssinfo_stats_info nssinfo_wifili_strings_stats_txrx[NSS_WIFILI_STATS_TXRX_MAX];
static struct nssinfo_stats_info nssinfo_wifili_strings_stats_tcl[NSS_WIFILI_STATS_TCL_MAX];
static struct nssinfo_stats_info nssinfo_wifili_strings_stats_tx_comp[NSS_WIFILI_STATS_TX_DESC_FREE_MAX];
static struct nssinfo_stats_info nssinfo_wifili_strings_stats_reo[NSS_WIFILI_STATS_REO_MAX];
static struct nssinfo_stats_info nssinfo_wifili_strings_stats_txsw_pool[NSS_WIFILI_STATS_TX_DESC_MAX];
static struct nssinfo_stats_info nssinfo_wifili_strings_stats_ext_txsw_pool[NSS_WIFILI_STATS_EXT_TX_DESC_MAX];
static struct nssinfo_stats_info nssinfo_wifili_strings_stats_rxdma_pool[NSS_WIFILI_STATS_RX_DESC_MAX];
static struct nssinfo_stats_info nssinfo_wifili_strings_stats_rxdma_ring[NSS_WIFILI_STATS_RXDMA_DESC_MAX];
static struct nssinfo_stats_info nssinfo_wifili_strings_stats_wbm[NSS_WIFILI_STATS_WBM_MAX];

/*
 * nssinfo_wifili_stats_display()
 *	Wifili display callback function.
 */
static void nssinfo_wifili_stats_display(int core, char *input)
{
	int i;
	struct node *wifili_node;
	struct nss_wifili_stats *stats;
	char str[NSSINFO_STR_LEN], *tok = NULL;
	int id;

	if (!display_all_stats) {
		return;
	}

	pthread_mutex_lock(&wifili_lock);
	wifili_node = nodes[core][NSS_WIFILI_INTERNAL_INTERFACE];
	if (!wifili_node) {
		pthread_mutex_unlock(&wifili_lock);
		return;
	}

	/*
	 * If the input is "wifili", display the wifili IDs.
	 */
	if (input && !strncmp(input, nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_WIFILI].subsystem_name, strlen(input))) {
		for (i = 0; i < NSS_WIFILI_MAX_PDEV_NUM_MSG; i++) {
			nssinfo_stats_print("\t%s ID : %d\n", nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_WIFILI].subsystem_name, i);
		}
		nssinfo_stats_print("\n\tSelect one...\n");
		pthread_mutex_unlock(&wifili_lock);
		return;
	}

	/*
	 * Splitting the input to get the ID.
	 */
	strlcpy(str, input, sizeof(str));
	char *rest = NULL;
	tok = strtok_r(str, "[", &rest);
	tok = strtok_r(NULL, "]", &rest);

	if (!tok || tok[0] == '\0'|| (id = atoi(tok)) < 0 || id >= NSS_WIFILI_MAX_PDEV_NUM_MSG) {
		++invalid_input;
		pthread_mutex_unlock(&wifili_lock);
		nssinfo_trace("Invalid pdev number: %s\n", tok);
		return;
	}

	stats = (struct nss_wifili_stats *)wifili_node->node_stats;
	nssinfo_stats_print("WIFILI ID: %d\n", id);

	nssinfo_print_all("wifili", "wifili TxRx Stats", nssinfo_wifili_strings_stats_txrx, NSS_WIFILI_STATS_TXRX_MAX, stats->stats_txrx[id]);

	/*
	 * TCL ring statistics.
	 */
	nssinfo_print_all("wifili", "wifili TCL Ring Stats", nssinfo_wifili_strings_stats_tcl, NSS_WIFILI_STATS_TCL_MAX, stats->stats_tcl_ring[id]);

	/*
	 * Transmit Classifier completion statistics.
	 */
	nssinfo_print_all("wifili", "wifili Tx Comp Stats", nssinfo_wifili_strings_stats_tx_comp, NSS_WIFILI_STATS_TX_DESC_FREE_MAX, stats->stats_tx_comp[id]);

	/*
	 * Reorder ring statistics.
	 */
	nssinfo_print_all("wifili", "wifili Reo Stats", nssinfo_wifili_strings_stats_reo, NSS_WIFILI_STATS_REO_MAX, stats->stats_reo[id]);

	/*
	 * TX SW Pool.
	 */
	nssinfo_print_all("wifili", "wifili Tx SW Pool Stats", nssinfo_wifili_strings_stats_txsw_pool, NSS_WIFILI_STATS_TX_DESC_MAX, stats->stats_tx_desc[id]);

	/*
	 * TX  EXt SW Pool.
	 */
	nssinfo_print_all("wifili", "wifili Ext Tx SW Pool Stats", nssinfo_wifili_strings_stats_ext_txsw_pool, NSS_WIFILI_STATS_EXT_TX_DESC_MAX, stats->stats_ext_tx_desc[id]);

	/*
	 * Rx DMA pool statistics.
	 */
	nssinfo_print_all("wifili", "wifili Rx DMA Pool Stats", nssinfo_wifili_strings_stats_rxdma_pool, NSS_WIFILI_STATS_RX_DESC_MAX, stats->stats_rx_desc[id]);

	/*
	 * Rx DMA ring statistics.
	 */
	nssinfo_print_all("wifili", "wifili Rx DMA Rings Stats", nssinfo_wifili_strings_stats_rxdma_ring, NSS_WIFILI_STATS_RXDMA_DESC_MAX, stats->stats_rxdma[id]);

	/*
	 * WBM(Wireless Buffer Manager) ring statistics.
	 */
	nssinfo_print_all("wifili", "wifili WBM Rings Stats", nssinfo_wifili_strings_stats_wbm, NSS_WIFILI_STATS_WBM_MAX, stats->stats_wbm);
}

/*
 * nssinfo_wifili_stats_notify()
 *	Wifili stats notify callback function.
 */
static void nssinfo_wifili_stats_notify(void *data)
{
	struct nss_wifili_stats *node_stats;
	struct nss_wifili_stats_notification *nss_stats = (struct nss_wifili_stats_notification *)data;
	struct node *wifili_node;
	struct node **wifili_ptr;

	if (!nssinfo_coreid_ifnum_valid(nss_stats->core_id, NSS_WIFILI_INTERNAL_INTERFACE)) {
		return;
	}

	pthread_mutex_lock(&wifili_lock);
	wifili_ptr = &nodes[nss_stats->core_id][NSS_WIFILI_INTERNAL_INTERFACE];
	wifili_node = *wifili_ptr;
	if (wifili_node) {
		memcpy(wifili_node->node_stats, &nss_stats->stats, sizeof(nss_stats->stats));
		pthread_mutex_unlock(&wifili_lock);
		return;
	}
	pthread_mutex_unlock(&wifili_lock);

	wifili_node = (struct node *)calloc(1, sizeof(struct node));
	if (!wifili_node) {
		nssinfo_warn("Failed to allocate memory for wifili node\n");
		return;
	}

	node_stats = (struct nss_wifili_stats *)malloc(sizeof(nss_stats->stats));
	if (!node_stats) {
		nssinfo_warn("Failed to allocate memory for wifili node statistics\n");
		goto wifili_node_free;
	}

	memcpy(node_stats, &nss_stats->stats, sizeof(nss_stats->stats));
	wifili_node->node_stats = node_stats;
	wifili_node->subsystem_id = NSS_NLCMN_SUBSYS_WIFILI;

	/*
	 * Notify is guaranteed to be single threaded via Netlink listen callback
	 */
	pthread_mutex_lock(&wifili_lock);
	*wifili_ptr = wifili_node;
	pthread_mutex_unlock(&wifili_lock);
	return;

wifili_node_free:
	free(wifili_node);
}

/*
 * nssinfo_wifili_destroy()
 *	Destroy wifili node.
 */
static void nssinfo_wifili_destroy(uint32_t core_id, uint32_t if_num)
{
	if (nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_WIFILI].is_inited) {
		nssinfo_node_stats_destroy(&wifili_lock, core_id, NSS_WIFILI_INTERNAL_INTERFACE);
	}
}

/*
 * nssinfo_wifili_deinit()
 *	Deinitialize wifili module.
 */
void nssinfo_wifili_deinit(void *data)
{
	struct nss_nlmcast_ctx *ctx = (struct nss_nlmcast_ctx *)data;

	if (nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_WIFILI].is_inited) {
		pthread_mutex_destroy(&wifili_lock);
		nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_WIFILI].is_inited = false;
	}

	nss_nlmcast_sock_leave_grp(ctx, NSS_NLWIFILI_MCAST_GRP);
}

/*
 * nssinfo_wifili_init()
 *	Initialize wifili module.
 */
int nssinfo_wifili_init(void *data)
{
	int error;
	struct nss_nlmcast_ctx *ctx = (struct nss_nlmcast_ctx *)data;

	/*
	 * Subscribe for wifili multicast group.
	 */
	nss_nlsock_set_family(&ctx->sock, NSS_NLWIFILI_FAMILY);
	error = nss_nlmcast_sock_join_grp(ctx, NSS_NLWIFILI_MCAST_GRP);
	if (error) {
		nssinfo_warn("Unable to join wifili multicast group.\n");
		return error;
	}

	if (nssinfo_stats_info_init(nssinfo_wifili_strings_stats_txrx,
				"/sys/kernel/debug/qca-nss-drv/strings/wifili/txrx_str") != 0) {
		goto fail;
	}

	if (nssinfo_stats_info_init(nssinfo_wifili_strings_stats_tcl,
				"/sys/kernel/debug/qca-nss-drv/strings/wifili/tcl_ring_str") != 0) {
		goto fail;
	}

	if (nssinfo_stats_info_init(nssinfo_wifili_strings_stats_tx_comp,
				"/sys/kernel/debug/qca-nss-drv/strings/wifili/tcl_comp_str") != 0) {
		goto fail;
	}

	if (nssinfo_stats_info_init(nssinfo_wifili_strings_stats_reo,
				"/sys/kernel/debug/qca-nss-drv/strings/wifili/reo_ring_str") != 0) {
		goto fail;
	}

	if (nssinfo_stats_info_init(nssinfo_wifili_strings_stats_txsw_pool,
				"/sys/kernel/debug/qca-nss-drv/strings/wifili/tx_sw_str") != 0) {
		goto fail;
	}

	if (nssinfo_stats_info_init(nssinfo_wifili_strings_stats_ext_txsw_pool,
				"/sys/kernel/debug/qca-nss-drv/strings/wifili/tx_ext_sw_str") != 0) {
		goto fail;
	}

	if (nssinfo_stats_info_init(nssinfo_wifili_strings_stats_rxdma_pool,
				"/sys/kernel/debug/qca-nss-drv/strings/wifili/rx_dma_pool_str") != 0) {
		goto fail;
	}

	if (nssinfo_stats_info_init(nssinfo_wifili_strings_stats_rxdma_ring,
				"/sys/kernel/debug/qca-nss-drv/strings/wifili/rx_dma_ring_str") != 0) {
		goto fail;
	}

	if (nssinfo_stats_info_init(nssinfo_wifili_strings_stats_wbm,
				"/sys/kernel/debug/qca-nss-drv/strings/wifili/wbm_ring_str") != 0) {
		goto fail;
	}

	if (pthread_mutex_init(&wifili_lock, NULL) != 0) {
		nssinfo_warn("Mutex init has failed for wifili\n");
		goto fail;
	}

	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_WIFILI].display = nssinfo_wifili_stats_display;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_WIFILI].notify = nssinfo_wifili_stats_notify;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_WIFILI].destroy = nssinfo_wifili_destroy;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_WIFILI].is_inited = true;
	return 0;
fail:
	nss_nlmcast_sock_leave_grp(ctx, NSS_NLWIFILI_MCAST_GRP);
	return -1;
}
