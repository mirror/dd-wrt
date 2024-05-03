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
 * @file NSSINFO ipv6_reasm handler
 */
#include "nssinfo.h"
#include <nss_ipv6_reasm.h>
#include <nss_nlipv6_reasm_if.h>

static pthread_mutex_t ipv6_reasm_lock;
static struct nssinfo_stats_info nss_stats_str_node[NSS_STATS_NODE_MAX];
static struct nssinfo_stats_info nss_ipv6_reasm_stats_str[NSS_IPV6_REASM_STATS_MAX];

/*
 * nssinfo_ipv6_reasm_stats_display()
 *	IPv6 reassembly display callback function.
 */
static void nssinfo_ipv6_reasm_stats_display(int core, char *input)
{
	struct node *ipv6_reasm_node;

	if (input && strncmp(input, nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_IPV6_REASM].subsystem_name, strlen(input))) {
		++invalid_input;
		nssinfo_trace("Invalid node name: %s\n", input);
		return;
	}

	pthread_mutex_lock(&ipv6_reasm_lock);
	ipv6_reasm_node = nodes[core][NSS_IPV6_REASM_INTERFACE];
	if (!ipv6_reasm_node) {
		pthread_mutex_unlock(&ipv6_reasm_lock);
		return;
	}

	if (display_all_stats) {
		nssinfo_print_all("ipv6_reasm", "ipv6_reasm Common Stats", nss_stats_str_node, NSS_STATS_NODE_MAX, (uint64_t *)ipv6_reasm_node->cmn_node_stats);
		nssinfo_print_all("ipv6_reasm", "ipv6_reasm Stats", nss_ipv6_reasm_stats_str, NSS_IPV6_REASM_STATS_MAX, (uint64_t *)ipv6_reasm_node->node_stats);

		pthread_mutex_unlock(&ipv6_reasm_lock);
		return;
	}

	nssinfo_print_summary("ipv6_rsm", (uint64_t *)ipv6_reasm_node->cmn_node_stats, NULL, 0);
	pthread_mutex_unlock(&ipv6_reasm_lock);
}

/*
 * nssinfo_ipv6_reasm_stats_notify()
 *	IPv6 stats notify callback function.
 */
static void nssinfo_ipv6_reasm_stats_notify(void *data)
{
	uint64_t *cmn_node_stats, *node_stats;
	struct nss_ipv6_reasm_stats_notification *nss_stats = (struct nss_ipv6_reasm_stats_notification *)data;
	struct node *ipv6_reasm_node;
	struct node **ipv6_reasm_ptr;

	if (!nssinfo_coreid_ifnum_valid(nss_stats->core_id, NSS_IPV6_REASM_INTERFACE)) {
		return;
	}

	pthread_mutex_lock(&ipv6_reasm_lock);
	ipv6_reasm_ptr = &nodes[nss_stats->core_id][NSS_IPV6_REASM_INTERFACE];
	ipv6_reasm_node = *ipv6_reasm_ptr;
	if (ipv6_reasm_node) {
		memcpy(ipv6_reasm_node->cmn_node_stats, &nss_stats->cmn_node_stats, sizeof(nss_stats->cmn_node_stats));
		memcpy(ipv6_reasm_node->node_stats, &nss_stats->ipv6_reasm_stats, sizeof(nss_stats->ipv6_reasm_stats));
		pthread_mutex_unlock(&ipv6_reasm_lock);
		return;
	}
	pthread_mutex_unlock(&ipv6_reasm_lock);

	ipv6_reasm_node = (struct node *)calloc(1, sizeof(struct node));
	if (!ipv6_reasm_node) {
		nssinfo_warn("Failed to allocate memory for ipv6_reasm node\n");
		return;
	}

	cmn_node_stats = (uint64_t *)malloc(sizeof(nss_stats->cmn_node_stats));
	if (!cmn_node_stats) {
		nssinfo_warn("Failed to allocate memory for ipv6_reasm common node statistics\n");
		goto ipv6_reasm_node_free;
	}

	node_stats = (uint64_t *)malloc(sizeof(nss_stats->ipv6_reasm_stats));
	if (!node_stats) {
		nssinfo_warn("Failed to allocate memory for ipv6_reasm connection stats\n");
		goto cmn_node_stats_free;
	}

	memcpy(cmn_node_stats, &nss_stats->cmn_node_stats, sizeof(nss_stats->cmn_node_stats));
	memcpy(node_stats, &nss_stats->ipv6_reasm_stats, sizeof(nss_stats->ipv6_reasm_stats));
	ipv6_reasm_node->cmn_node_stats = cmn_node_stats;
	ipv6_reasm_node->node_stats = node_stats;
	ipv6_reasm_node->subsystem_id = NSS_NLCMN_SUBSYS_IPV6_REASM;

	/*
	 * Notify is guaranteed to be single threaded via Netlink listen callback
	 */
	pthread_mutex_lock(&ipv6_reasm_lock);
	*ipv6_reasm_ptr = ipv6_reasm_node;
	pthread_mutex_unlock(&ipv6_reasm_lock);
	return;

cmn_node_stats_free:
	free(cmn_node_stats);

ipv6_reasm_node_free:
	free(ipv6_reasm_node);
}

/*
 * nssinfo_ipv6_reasm_destroy()
 *	Destroy IPv6 reassembly node.
 */
static void nssinfo_ipv6_reasm_destroy(uint32_t core_id, uint32_t if_num)
{
	if (nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_IPV6_REASM].is_inited) {
		nssinfo_node_stats_destroy(&ipv6_reasm_lock, core_id, NSS_IPV6_REASM_INTERFACE);
	}
}

/*
 * nssinfo_ipv6_reasm_deinit()
 *	Deinitialize ipv6_reasm module.
 */
void nssinfo_ipv6_reasm_deinit(void *data)
{
	struct nss_nlmcast_ctx *ctx = (struct nss_nlmcast_ctx *)data;

	if (nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_IPV6_REASM].is_inited) {
		pthread_mutex_destroy(&ipv6_reasm_lock);
		nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_IPV6_REASM].is_inited = false;
	}

	nss_nlmcast_sock_leave_grp(ctx, NSS_NLIPV6_REASM_MCAST_GRP);
}

/*
 * nssinfo_ipv6_reasm_init()
 *	Initialize IPv6 reassembly module.
 */
int nssinfo_ipv6_reasm_init(void *data)
{
	int error;
	struct nss_nlmcast_ctx *ctx = (struct nss_nlmcast_ctx *)data;

	/*
	 * Subscribe for IPv6 reassembly multicast group.
	 */
	nss_nlsock_set_family(&ctx->sock, NSS_NLIPV6_REASM_FAMILY);
	error = nss_nlmcast_sock_join_grp(ctx, NSS_NLIPV6_REASM_MCAST_GRP);
	if (error) {
		nssinfo_warn("Unable to join IPv6 reasm multicast group\n");
		return error;
	}

	if (nssinfo_stats_info_init(nss_stats_str_node,
				"/sys/kernel/debug/qca-nss-drv/strings/common_node_stats") != 0) {
		goto fail;
	}

	if (nssinfo_stats_info_init(nss_ipv6_reasm_stats_str,
				"/sys/kernel/debug/qca-nss-drv/strings/ipv6_reasm") != 0) {
		goto fail;
	}

	if (pthread_mutex_init(&ipv6_reasm_lock, NULL) != 0) {
		nssinfo_warn("Mutex init has failed for IPv6 reassembly\n");
		goto fail;
	}

	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_IPV6_REASM].display = nssinfo_ipv6_reasm_stats_display;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_IPV6_REASM].notify = nssinfo_ipv6_reasm_stats_notify;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_IPV6_REASM].destroy = nssinfo_ipv6_reasm_destroy;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_IPV6_REASM].is_inited = true;
	return 0;
fail:
	nss_nlmcast_sock_leave_grp(ctx, NSS_NLIPV6_REASM_MCAST_GRP);
	return -1;
}
