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
 * @file NSSINFO l2tpv2 handler
 */
#include "nssinfo.h"
#include <nss_l2tpv2.h>
#include <nss_nll2tpv2_if.h>

static pthread_mutex_t l2tpv2_lock;
static struct nssinfo_stats_info nssinfo_l2tpv2_stats_str[NSS_L2TPV2_STATS_SESSION_MAX];

/*
 * nssinfo_l2tpv2_stats_display()
 *	L2TPV2 display callback function.
 */
static void nssinfo_l2tpv2_stats_display(int core, char *input)
{
	int i, id;
	struct node *l2tpv2_node;
	uint64_t *stats;
	uint64_t exceptions;
	char str_ex[NSSINFO_STR_LEN];
	char node_name[NSSINFO_STR_LEN];
	char str[NSSINFO_STR_LEN], *tok = NULL;
	int active_if_available = false;

	pthread_mutex_lock(&l2tpv2_lock);
	for (i = 0; i < NSS_MAX_NET_INTERFACES; i++) {
		l2tpv2_node = nodes[core][i];
		if (!l2tpv2_node || l2tpv2_node->type != NSS_DYNAMIC_INTERFACE_TYPE_L2TPV2) {
			continue;
		}

		active_if_available = true;

		stats = (uint64_t *)l2tpv2_node->node_stats;

		snprintf(node_name, sizeof(node_name), "%s[%llu]",
				nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_L2TPV2].subsystem_name,
				l2tpv2_node->id);

		/*
		 * If the input is "l2tpv2", display summary of all the active interfaces.
		 */
		if (input && !strncmp(input, nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_L2TPV2].subsystem_name, strlen(input))) {
			goto display_summary;
		}

		/*
		 * If the input is "l2tpv2[0-255]", display stats of that interface.
		 */
		if (input) {
		 	/* Parsing 'input' to get the interface number */
			strlcpy(str, input, sizeof(str));
			char *rest = NULL;
			tok = strtok_r(str, "[", &rest);
			tok = strtok_r(NULL, "]", &rest);

			if (!tok || tok[0] == '\0' || (id = atoi(tok)) < 0 || id >= NSS_MAX_NET_INTERFACES) {
				++invalid_input;
				nssinfo_trace("Invalid interface number: %s\n", tok);
				goto done;
			}

			if (id != l2tpv2_node->id) {
				continue;
			}
		}

		if (display_all_stats) {
			nssinfo_print_all(node_name, "l2tpv2 Stats",
					nssinfo_l2tpv2_stats_str, NSS_L2TPV2_STATS_SESSION_MAX,
					stats);
			goto done;
		}

display_summary:
		exceptions = stats[NSS_L2TPV2_STATS_SESSION_RX_PPP_LCP_PKTS] + stats[NSS_L2TPV2_STATS_SESSION_RX_EXP_DATA_PKTS];
		char *format_stats = nssinfo_format_stats(exceptions);
		strlcpy(str_ex, format_stats, sizeof(str_ex));
		nssinfo_stats_print(nssinfo_summary_fmt, node_name, "", "", "", str_ex);
	}

	if (!active_if_available) {
		++invalid_input;
		nssinfo_trace("L2TPv2 interface not available\n");
	}
done:
	pthread_mutex_unlock(&l2tpv2_lock);
}

/*
 * nssinfo_l2tpv2_stats_notify()
 *	L2TPv2 stats notify callback function.
 */
static void nssinfo_l2tpv2_stats_notify(void *data)
{
	uint64_t *node_stats;
	struct nss_l2tpv2_stats_notification *nss_stats = (struct nss_l2tpv2_stats_notification *)data;
	struct node *l2tpv2_node;
	struct node **l2tpv2_ptr;

	if (!nssinfo_coreid_ifnum_valid(nss_stats->core_id, nss_stats->if_num)) {
		return;
	}

	pthread_mutex_lock(&l2tpv2_lock);
	l2tpv2_ptr = &nodes[nss_stats->core_id][nss_stats->if_num];
	l2tpv2_node = *l2tpv2_ptr;
	if (l2tpv2_node) {
		memcpy(l2tpv2_node->node_stats, &nss_stats->stats, sizeof(nss_stats->stats));
		pthread_mutex_unlock(&l2tpv2_lock);
		return;
	}
	pthread_mutex_unlock(&l2tpv2_lock);

	l2tpv2_node = (struct node *)calloc(1, sizeof(struct node));
	if (!l2tpv2_node) {
		nssinfo_warn("Failed to allocate memory for l2tpv2 node\n");
		return;
	}

	node_stats = (uint64_t *)malloc(sizeof(nss_stats->stats));
	if (!node_stats) {
		nssinfo_warn("Failed to allocate memory for l2tpv2 node stats\n");
		goto l2tpv2_node_free;
	}

	memcpy(node_stats, &nss_stats->stats, sizeof(nss_stats->stats));
	l2tpv2_node->node_stats = node_stats;
	l2tpv2_node->type = NSS_DYNAMIC_INTERFACE_TYPE_L2TPV2;
	l2tpv2_node->id = nss_stats->if_num;
	l2tpv2_node->subsystem_id = NSS_NLCMN_SUBSYS_L2TPV2;

	/*
	 * Notify is guaranteed to be single threaded via Netlink listen callback
	 */
	pthread_mutex_lock(&l2tpv2_lock);
	*l2tpv2_ptr = l2tpv2_node;
	pthread_mutex_unlock(&l2tpv2_lock);
	return;

l2tpv2_node_free:
	free(l2tpv2_node);
}

/*
 * nssinfo_l2tpv2_destroy()
 *	Destroy L2TPv2 node.
 */
static void nssinfo_l2tpv2_destroy(uint32_t core_id, uint32_t if_num)
{
	if (nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_L2TPV2].is_inited) {
		nssinfo_node_stats_destroy(&l2tpv2_lock, core_id, if_num);
	}
}

/*
 * nssinfo_l2tpv2_deinit()
 *	Deinitialize l2tpv2 module.
 */
void nssinfo_l2tpv2_deinit(void *data)
{
	struct nss_nlmcast_ctx *ctx = (struct nss_nlmcast_ctx *)data;

	if (nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_L2TPV2].is_inited) {
		pthread_mutex_destroy(&l2tpv2_lock);
		nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_L2TPV2].is_inited = false;
	}

	nss_nlmcast_sock_leave_grp(ctx, NSS_NLL2TPV2_MCAST_GRP);
}

/*
 * nssinfo_l2tpv2_init()
 *	Initialize L2TPv2 module.
 */
int nssinfo_l2tpv2_init(void *data)
{
	int error;
	struct nss_nlmcast_ctx *ctx = (struct nss_nlmcast_ctx *)data;

	/*
	 * Subscribe for L2TPv2 multicast group.
	 */
	nss_nlsock_set_family(&ctx->sock, NSS_NLL2TPV2_FAMILY);
	error = nss_nlmcast_sock_join_grp(ctx, NSS_NLL2TPV2_MCAST_GRP);
	if (error) {
		nssinfo_warn("Unable to join L2TPv2 multicast group.\n");
		return error;
	}

	if (nssinfo_stats_info_init(nssinfo_l2tpv2_stats_str,
				"/sys/kernel/debug/qca-nss-drv/strings/l2tpv2") != 0) {
		goto fail;
	}

	if (pthread_mutex_init(&l2tpv2_lock, NULL) != 0) {
		nssinfo_warn("Mutex init has failed for L2TPv2\n");
		goto fail;
	}

	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_L2TPV2].display = nssinfo_l2tpv2_stats_display;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_L2TPV2].notify = nssinfo_l2tpv2_stats_notify;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_L2TPV2].destroy = nssinfo_l2tpv2_destroy;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_L2TPV2].is_inited = true;
	return 0;
fail:
	nss_nlmcast_sock_leave_grp(ctx, NSS_NLL2TPV2_MCAST_GRP);
	return -1;
}
