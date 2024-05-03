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
 * @file NSSINFO ipv4_reasm handler
 */
#include "nssinfo.h"
#include <nss_ipv4_reasm.h>
#include <nss_nlipv4_reasm_if.h>

static pthread_mutex_t ipv4_reasm_lock;
static struct nssinfo_stats_info nss_stats_str_node[NSS_STATS_NODE_MAX];
static struct nssinfo_stats_info nss_ipv4_reasm_stats_str[NSS_IPV4_REASM_STATS_MAX];

/*
 * nssinfo_ipv4_reasm_stats_display()
 *      IPv4 reassembly display callback function.
 */
static void nssinfo_ipv4_reasm_stats_display(int core, char *input)
{
	struct node *ipv4_reasm_node;

	if (input && strncmp(input, nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_IPV4_REASM].subsystem_name, strlen(input))) {
		++invalid_input;
		nssinfo_trace("Invalid node name: %s\n", input);
		return;
	}

	pthread_mutex_lock(&ipv4_reasm_lock);
	ipv4_reasm_node = nodes[core][NSS_IPV4_REASM_INTERFACE];
	if (!ipv4_reasm_node) {
		pthread_mutex_unlock(&ipv4_reasm_lock);
		return;
	}

	if (display_all_stats) {
		nssinfo_print_all("ipv4_reasm", "ipv4_reasm Common Stats", nss_stats_str_node, NSS_STATS_NODE_MAX, (uint64_t *)ipv4_reasm_node->cmn_node_stats);
		nssinfo_print_all("ipv4_reasm", "ipv4_reasm Stats", nss_ipv4_reasm_stats_str, NSS_IPV4_REASM_STATS_MAX, (uint64_t *)ipv4_reasm_node->node_stats);

		pthread_mutex_unlock(&ipv4_reasm_lock);
		return;
	}

	nssinfo_print_summary("ipv4_rsm", (uint64_t *)ipv4_reasm_node->cmn_node_stats, NULL, 0);
	pthread_mutex_unlock(&ipv4_reasm_lock);
}

/*
 * nssinfo_ipv4_reasm_stats_notify()
 * 	IPv4 stats notify callback function.
 */
static void nssinfo_ipv4_reasm_stats_notify(void *data)
{
	uint64_t *cmn_node_stats, *node_stats;
	struct nss_ipv4_reasm_stats_notification *nss_stats = (struct nss_ipv4_reasm_stats_notification *)data;
	struct node *ipv4_reasm_node;
	struct node **ipv4_reasm_ptr;

	if (!nssinfo_coreid_ifnum_valid(nss_stats->core_id, NSS_IPV4_REASM_INTERFACE)) {
		return;
	}

	pthread_mutex_lock(&ipv4_reasm_lock);
	ipv4_reasm_ptr = &nodes[nss_stats->core_id][NSS_IPV4_REASM_INTERFACE];
	ipv4_reasm_node = *ipv4_reasm_ptr;
	if (ipv4_reasm_node) {
		memcpy(ipv4_reasm_node->cmn_node_stats, &nss_stats->cmn_node_stats, sizeof(nss_stats->cmn_node_stats));
		memcpy(ipv4_reasm_node->node_stats, &nss_stats->ipv4_reasm_stats, sizeof(nss_stats->ipv4_reasm_stats));
		pthread_mutex_unlock(&ipv4_reasm_lock);
		return;
	}
	pthread_mutex_unlock(&ipv4_reasm_lock);

	ipv4_reasm_node = (struct node *)calloc(1, sizeof(struct node));
	if (!ipv4_reasm_node) {
		nssinfo_warn("Failed to allocate memory for ipv4_reasm node\n");
		return;
	}

	cmn_node_stats = (uint64_t *)malloc(sizeof(nss_stats->cmn_node_stats));
	if (!cmn_node_stats) {
		nssinfo_warn("Failed to allocate memory for ipv4_reasm common node statistics\n");
		goto ipv4_reasm_node_free;
	}

	node_stats = (uint64_t *)malloc(sizeof(nss_stats->ipv4_reasm_stats));
	if (!node_stats) {
		nssinfo_warn("Failed to allocate memory for ipv4_reasm connection stats\n");
		goto cmn_node_stats_free;
	}

	memcpy(cmn_node_stats, &nss_stats->cmn_node_stats, sizeof(nss_stats->cmn_node_stats));
	memcpy(node_stats, &nss_stats->ipv4_reasm_stats, sizeof(nss_stats->ipv4_reasm_stats));
	ipv4_reasm_node->cmn_node_stats = cmn_node_stats;
	ipv4_reasm_node->node_stats = node_stats;
	ipv4_reasm_node->subsystem_id = NSS_NLCMN_SUBSYS_IPV4_REASM;

	/*
	 * Notify is guaranteed to be single threaded via Netlink listen callback
	 */
	pthread_mutex_lock(&ipv4_reasm_lock);
	*ipv4_reasm_ptr = ipv4_reasm_node;
	pthread_mutex_unlock(&ipv4_reasm_lock);
	return;

cmn_node_stats_free:
	free(cmn_node_stats);

ipv4_reasm_node_free:
	free(ipv4_reasm_node);
}

/*
 * nssinfo_ipv4_reasm_destroy()
 *	Destroy IPv4 reassembly node.
 */
static void nssinfo_ipv4_reasm_destroy(uint32_t core_id, uint32_t if_num)
{
	if (nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_IPV4_REASM].is_inited) {
		nssinfo_node_stats_destroy(&ipv4_reasm_lock, core_id, NSS_IPV4_REASM_INTERFACE);
	}
}

/*
 * nssinfo_ipv4_reasm_deinit()
 *	Deinitialize ipv4_reasm module.
 */
void nssinfo_ipv4_reasm_deinit(void *data)
{
	struct nss_nlmcast_ctx *ctx = (struct nss_nlmcast_ctx *)data;

	if (nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_IPV4_REASM].is_inited) {
		pthread_mutex_destroy(&ipv4_reasm_lock);
		nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_IPV4_REASM].is_inited = false;
	}

	nss_nlmcast_sock_leave_grp(ctx, NSS_NLIPV4_REASM_MCAST_GRP);
}

/*
 * nssinfo_ipv4_reasm_init()
 *	Initialize IPv4 reassembly module.
 */
int nssinfo_ipv4_reasm_init(void *data)
{
	int error;
	struct nss_nlmcast_ctx *ctx = (struct nss_nlmcast_ctx *)data;

	/*
	 * Subscribe for IPv4 reassembly multicast group.
	 */
	nss_nlsock_set_family(&ctx->sock, NSS_NLIPV4_REASM_FAMILY);
	error = nss_nlmcast_sock_join_grp(ctx, NSS_NLIPV4_REASM_MCAST_GRP);
	if (error) {
		nssinfo_warn("Unable to join IPv4 reasm multicast group\n");
		return error;
	}

	if (nssinfo_stats_info_init(nss_stats_str_node,
				"/sys/kernel/debug/qca-nss-drv/strings/common_node_stats") != 0) {
		goto fail;
	}

	if (nssinfo_stats_info_init(nss_ipv4_reasm_stats_str,
				"/sys/kernel/debug/qca-nss-drv/strings/ipv4_reasm") != 0) {
		goto fail;
	}

	if (pthread_mutex_init(&ipv4_reasm_lock, NULL) != 0) {
		nssinfo_warn("Mutex init has failed for IPv4 reassembly\n");
		goto fail;
	}

	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_IPV4_REASM].display = nssinfo_ipv4_reasm_stats_display;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_IPV4_REASM].notify = nssinfo_ipv4_reasm_stats_notify;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_IPV4_REASM].destroy = nssinfo_ipv4_reasm_destroy;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_IPV4_REASM].is_inited = true;
	return 0;
fail:
	nss_nlmcast_sock_leave_grp(ctx, NSS_NLIPV4_REASM_MCAST_GRP);
	return -1;
}
