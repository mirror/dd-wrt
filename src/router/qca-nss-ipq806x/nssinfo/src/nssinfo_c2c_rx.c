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
 * @file NSSINFO c2c_rx handler
 */
#include "nssinfo.h"
#include <nss_c2c_rx.h>
#include <nss_nlc2c_rx_if.h>

static pthread_mutex_t c2c_rx_lock;
static struct nssinfo_stats_info nssinfo_c2c_rx_stats_str[NSS_C2C_RX_STATS_MAX];

/*
 * nssinfo_c2c_rx_stats_display()
 *	Core-to-core Rx display callback function.
 */
static void nssinfo_c2c_rx_stats_display(int core, char *input)
{
	uint64_t *stats;

	if (input && strncmp(input, nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_C2C_RX].subsystem_name, strlen(input))) {
		++invalid_input;
		nssinfo_trace("Invalid node name: %s\n", input);
		return;
	}

	pthread_mutex_lock(&c2c_rx_lock);
	struct node *c2c_rx_node = nodes[core][NSS_C2C_RX_INTERFACE];
	if (!c2c_rx_node) {
		pthread_mutex_unlock(&c2c_rx_lock);
		return;
	}

	stats = (uint64_t *)c2c_rx_node->node_stats;
	if (display_all_stats) {
		nssinfo_print_all("c2c_rx", "c2c_rx Stats", nssinfo_c2c_rx_stats_str, NSS_C2C_RX_STATS_MAX, (uint64_t *)c2c_rx_node->node_stats);
		pthread_mutex_unlock(&c2c_rx_lock);
		return;
	}

	nssinfo_print_summary("c2c_rx", stats, NULL, 0);
	pthread_mutex_unlock(&c2c_rx_lock);
}

/*
 * nssinfo_c2c_rx_stats_notify()
 *	C2C Rx stats notify callback function.
 */
static void nssinfo_c2c_rx_stats_notify(void *data)
{
	uint64_t *node_stats;
	struct nss_c2c_rx_stats_notification *nss_stats = (struct nss_c2c_rx_stats_notification *)data;
	struct node *c2c_rx_node;
	struct node **c2c_rx_ptr;

	if (!nssinfo_coreid_ifnum_valid(nss_stats->core_id, NSS_C2C_RX_INTERFACE)) {
		return;
	}

	pthread_mutex_lock(&c2c_rx_lock);
	c2c_rx_ptr = &nodes[nss_stats->core_id][NSS_C2C_RX_INTERFACE];
	c2c_rx_node = *c2c_rx_ptr;
	if (c2c_rx_node) {
		memcpy(c2c_rx_node->node_stats, &nss_stats->stats, sizeof(nss_stats->stats));
		pthread_mutex_unlock(&c2c_rx_lock);
		return;
	}
	pthread_mutex_unlock(&c2c_rx_lock);

	c2c_rx_node = (struct node *)calloc(1, sizeof(struct node));
	if (!c2c_rx_node) {
		nssinfo_warn("Failed to allocate memory for C2C Rx node\n");
		return;
	}

	node_stats = (uint64_t *)malloc(sizeof(nss_stats->stats));
	if (!node_stats) {
		nssinfo_warn("Failed to allocate memory for C2C Rx node statistics\n");
		goto c2c_rx_node_free;
	}

	memcpy(node_stats, &nss_stats->stats, sizeof(nss_stats->stats));
	c2c_rx_node->node_stats = node_stats;
	c2c_rx_node->subsystem_id = NSS_NLCMN_SUBSYS_C2C_RX;

	/*
	 * Notify is guaranteed to be single threaded via Netlink listen callback.
	 */
	pthread_mutex_lock(&c2c_rx_lock);
	*c2c_rx_ptr = c2c_rx_node;
	pthread_mutex_unlock(&c2c_rx_lock);
	return;

c2c_rx_node_free:
	free(c2c_rx_node);
}

/*
 * nssinfo_c2c_rx_destroy()
 *      Destroy C2C Rx node.
 */
static void nssinfo_c2c_rx_destroy(uint32_t core_id, uint32_t if_num)
{
	if (nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_C2C_RX].is_inited) {
		nssinfo_node_stats_destroy(&c2c_rx_lock, core_id, NSS_C2C_RX_INTERFACE);
	}
}

/*
 * nssinfo_c2c_rx_deinit()
 *	Deinitialize c2c_rx module.
 */
void nssinfo_c2c_rx_deinit(void *data)
{
	struct nss_nlmcast_ctx *ctx = (struct nss_nlmcast_ctx *)data;

	if (nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_C2C_RX].is_inited) {
		pthread_mutex_destroy(&c2c_rx_lock);
		nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_C2C_RX].is_inited = false;
	}

	nss_nlmcast_sock_leave_grp(ctx, NSS_NLC2C_RX_MCAST_GRP);
}

/*
 * nssinfo_c2c_rx_init()
 *	Initialize C2C Rx module.
 */
int nssinfo_c2c_rx_init(void *data)
{
	int error;
	struct nss_nlmcast_ctx *ctx = (struct nss_nlmcast_ctx *)data;

	/*
	 * Subscribe for C2C Rx multicast group.
	 */
	nss_nlsock_set_family(&ctx->sock, NSS_NLC2C_RX_FAMILY);
	error = nss_nlmcast_sock_join_grp(ctx, NSS_NLC2C_RX_MCAST_GRP);
	if (error) {
		nssinfo_warn("Unable to join C2C Rx mcast group.\n");
		return error;
	}

	if (nssinfo_stats_info_init(nssinfo_c2c_rx_stats_str,
				"/sys/kernel/debug/qca-nss-drv/strings/c2c_rx") != 0) {
		goto fail;
	}

	if (pthread_mutex_init(&c2c_rx_lock, NULL) != 0) {
		nssinfo_warn("Mutex init has failed for C2C Rx\n");
		goto fail;
	}

	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_C2C_RX].display = nssinfo_c2c_rx_stats_display;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_C2C_RX].notify = nssinfo_c2c_rx_stats_notify;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_C2C_RX].destroy = nssinfo_c2c_rx_destroy;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_C2C_RX].is_inited = true;
	return 0;
fail:
	nss_nlmcast_sock_leave_grp(ctx, NSS_NLC2C_RX_MCAST_GRP);
	return -1;
}
