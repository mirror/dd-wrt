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
 * @file NSSINFO pppoe handler
 */
#include "nssinfo.h"
#include <nss_pppoe.h>
#include <nss_nlpppoe_if.h>

static pthread_mutex_t pppoe_lock;
static struct nssinfo_stats_info nssinfo_pppoe_session_stats_str[NSS_PPPOE_STATS_SESSION_MAX];
static struct nssinfo_stats_info nssinfo_pppoe_base_stats_str[NSS_PPPOE_STATS_BASE_MAX];

/*
 * nssinfo_pppoe_stats_display()
 *	PPPOE display callback function.
 */
static void nssinfo_pppoe_stats_display(int core, char *input)
{
	int i, id;
	struct node *pppoe_node;
	uint64_t *stats;
	char node_name[NSSINFO_STR_LEN];
	char str[NSSINFO_STR_LEN], *tok = NULL;
	bool active_if_available = false;

	pthread_mutex_lock(&pppoe_lock);
	for (i = 0; i < NSS_MAX_NET_INTERFACES; i++) {
		pppoe_node = nodes[core][i];
		if (!pppoe_node || pppoe_node->type != NSS_DYNAMIC_INTERFACE_TYPE_PPPOE) {
			continue;
		}

		active_if_available = true;

		stats = (uint64_t *)pppoe_node->node_stats;

		snprintf(node_name, sizeof(node_name), "%s[%llu]",
				nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_PPPOE].subsystem_name,
				pppoe_node->id);

		/*
		 * If the input is "pppoe", display summary of all the active interfaces.
		 */
		if (input && !strncmp(input, nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_PPPOE].subsystem_name, strlen(input))) {
			goto display_summary;
		}

		/*
		 * If the input is "pppoe[0-255]", display the stats of that interface.
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

			if (id != pppoe_node->id) {
				continue;
			}
		}

		if (display_all_stats) {
			nssinfo_print_all(node_name, "pppoe Session Stats",
					nssinfo_pppoe_session_stats_str, NSS_PPPOE_STATS_SESSION_MAX,
					(uint64_t *)pppoe_node->cmn_node_stats);

			nssinfo_print_all(node_name, "pppoe Base Stats",
					nssinfo_pppoe_base_stats_str, NSS_PPPOE_STATS_BASE_MAX,
					(uint64_t *)pppoe_node->node_stats);
			goto done;
		}

display_summary:
		nssinfo_print_summary(node_name, (uint64_t *)pppoe_node->node_stats,
				&stats[NSS_PPPOE_STATS_BASE_SHORT_PPPOE_HDR_LENGTH],
				NSS_PPPOE_STATS_BASE_MAX - NSS_PPPOE_STATS_BASE_SHORT_PPPOE_HDR_LENGTH);
	}

	if (!active_if_available) {
		++invalid_input;
		//nssinfo_trace("PPPoE interface not available\n");
	}
done:
	pthread_mutex_unlock(&pppoe_lock);
}

/*
 * nssinfo_pppoe_stats_notify()
 *	PPPoE stats notify callback function.
 */
static void nssinfo_pppoe_stats_notify(void *data)
{
	uint64_t *node_stats, *cmn_node_stats;
	struct nss_pppoe_stats_notification *nss_stats = (struct nss_pppoe_stats_notification *)data;
	struct node *pppoe_node;
	struct node **pppoe_ptr;

	if (!nssinfo_coreid_ifnum_valid(nss_stats->core_id, nss_stats->if_num)) {
		return;
	}

	pthread_mutex_lock(&pppoe_lock);
	pppoe_ptr = &nodes[nss_stats->core_id][nss_stats->if_num];
	pppoe_node = *pppoe_ptr;
	if (pppoe_node) {
		memcpy(pppoe_node->cmn_node_stats, &nss_stats->session_stats, sizeof(nss_stats->session_stats));
		memcpy(pppoe_node->node_stats, &nss_stats->base_stats, sizeof(nss_stats->base_stats));
		pthread_mutex_unlock(&pppoe_lock);
		return;
	}
	pthread_mutex_unlock(&pppoe_lock);

	pppoe_node = (struct node *)calloc(1, sizeof(struct node));
	if (!pppoe_node) {
		nssinfo_warn("Failed to allocate memory for pppoe node\n");
		return;
	}

	cmn_node_stats = (uint64_t *)malloc(sizeof(nss_stats->session_stats));
	if (!cmn_node_stats) {
		nssinfo_warn("Failed to allocate memory for pppoe session node stats\n");
		goto pppoe_node_free;
	}

	node_stats = (uint64_t *)malloc(sizeof(nss_stats->base_stats));
	if (!node_stats) {
		nssinfo_warn("Failed to allocate memory for pppoe base node stats\n");
		goto pppoe_cmn_node_free;
	}

	memcpy(cmn_node_stats, &nss_stats->session_stats, sizeof(nss_stats->session_stats));
	memcpy(node_stats, &nss_stats->base_stats, sizeof(nss_stats->base_stats));
	pppoe_node->cmn_node_stats = cmn_node_stats;
	pppoe_node->node_stats = node_stats;
	pppoe_node->type = NSS_DYNAMIC_INTERFACE_TYPE_PPPOE;
	pppoe_node->id = nss_stats->if_num;
	pppoe_node->subsystem_id = NSS_NLCMN_SUBSYS_PPPOE;

	/*
	 * Notify is guaranteed to be single threaded via Netlink listen callback
	 */
	pthread_mutex_lock(&pppoe_lock);
	*pppoe_ptr = pppoe_node;
	pthread_mutex_unlock(&pppoe_lock);
	return;

pppoe_cmn_node_free:
	free(cmn_node_stats);
pppoe_node_free:
	free(pppoe_node);
}

/*
 * nssinfo_pppoe_destroy()
 *	Destroy PPPoE node.
 */
static void nssinfo_pppoe_destroy(uint32_t core_id, uint32_t if_num)
{
	if (nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_PPPOE].is_inited) {
		nssinfo_node_stats_destroy(&pppoe_lock, core_id, if_num);
	}
}

/*
 * nssinfo_pppoe_deinit()
 *	Deinitialize pppoe module.
 */
void nssinfo_pppoe_deinit(void *data)
{
	struct nss_nlmcast_ctx *ctx = (struct nss_nlmcast_ctx *)data;

	if (nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_PPPOE].is_inited) {
		pthread_mutex_destroy(&pppoe_lock);
		nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_PPPOE].is_inited = false;
	}

	nss_nlmcast_sock_leave_grp(ctx, NSS_NLPPPOE_MCAST_GRP);
}

/*
 * nssinfo_pppoe_init()
 *	Initialize PPPoE module.
 */
int nssinfo_pppoe_init(void *data)
{
	int error;
	struct nss_nlmcast_ctx *ctx = (struct nss_nlmcast_ctx *)data;

	/*
	 * Subscribe for PPPoE multicast group.
	 */
	nss_nlsock_set_family(&ctx->sock, NSS_NLPPPOE_FAMILY);
	error = nss_nlmcast_sock_join_grp(ctx, NSS_NLPPPOE_MCAST_GRP);
	if (error) {
		nssinfo_warn("Unable to join PPPoE multicast group.\n");
		return error;
	}

	if (nssinfo_stats_info_init(nssinfo_pppoe_session_stats_str,
				"/sys/kernel/debug/qca-nss-drv/strings/pppoe/session_stats_str") != 0) {
		goto fail;
	}

	if (nssinfo_stats_info_init(nssinfo_pppoe_base_stats_str,
				"/sys/kernel/debug/qca-nss-drv/strings/pppoe/base_stats_str") != 0) {
		goto fail;
	}

	if (pthread_mutex_init(&pppoe_lock, NULL) != 0) {
		nssinfo_warn("Mutex init has failed for PPPoE\n");
		goto fail;
	}

	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_PPPOE].display = nssinfo_pppoe_stats_display;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_PPPOE].notify = nssinfo_pppoe_stats_notify;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_PPPOE].destroy = nssinfo_pppoe_destroy;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_PPPOE].is_inited = true;
	return 0;
fail:
	nss_nlmcast_sock_leave_grp(ctx, NSS_NLPPPOE_MCAST_GRP);
	return -1;
}
