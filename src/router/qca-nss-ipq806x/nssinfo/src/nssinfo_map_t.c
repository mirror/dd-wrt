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
 * @file NSSINFO map_t handler
 */
#include "nssinfo.h"
#include <nss_map_t.h>
#include <nss_nlmap_t_if.h>

static pthread_mutex_t map_t_lock;
static struct nssinfo_stats_info nssinfo_stats_info_map_t[NSS_MAP_T_STATS_MAX];

/*
 * nssinfo_map_t_stats_display()
 *	MAP_T display callback function.
 */
static void nssinfo_map_t_stats_display(int core, char *input)
{
	int i, id;
	struct node *map_t_node;
	uint64_t *stats;
	uint64_t exceptions;
	char str_ex[NSSINFO_STR_LEN];
	char node_name[NSSINFO_STR_LEN];
	char str[NSSINFO_STR_LEN], *tok = NULL;
	int active_if_available = false;

	if (!display_all_stats) {
		return;
	}

	pthread_mutex_lock(&map_t_lock);
	for (i = 0; i < NSS_MAX_NET_INTERFACES; i++) {
		map_t_node = nodes[core][i];
		if (!map_t_node || (map_t_node->type != NSS_DYNAMIC_INTERFACE_TYPE_MAP_T_INNER &&
			map_t_node->type != NSS_DYNAMIC_INTERFACE_TYPE_MAP_T_OUTER)) {
			continue;
		}

		active_if_available = true;

		stats = (uint64_t *)map_t_node->node_stats;

		snprintf(node_name, sizeof(node_name), "%s[%llu]",
				nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_MAP_T].subsystem_name,
				map_t_node->id);

		/*
		 * If the input is "map_t", display summary of all the active interfaces.
		 */
		if (input && !strncmp(input, nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_MAP_T].subsystem_name, strlen(input))) {
			goto display_summary;
		}

		/*
		 * If the input is "map_t[0-255]", display the stats of that interface.
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

			if (id != map_t_node->id) {
				continue;
			}
		}

		if (display_all_stats) {
			nssinfo_print_all(node_name, "MAP_T Stats",
					nssinfo_stats_info_map_t, NSS_MAP_T_STATS_MAX,
					(uint64_t *)stats);
			goto done;
		}

display_summary:
		exceptions = stats[NSS_MAP_T_STATS_V4_TO_V6_PBUF_EXCEPTION] + stats[NSS_MAP_T_STATS_V6_TO_V4_PBUF_EXCEPTION];
		char *format_stats = nssinfo_format_stats(exceptions);
		strlcpy(str_ex, format_stats, sizeof(str_ex));
		nssinfo_stats_print(nssinfo_summary_fmt, node_name, "", "", "", str_ex);
	}

	if (!active_if_available) {
		++invalid_input;
		nssinfo_trace("MAP_T interface not available\n");
	}
done:
	pthread_mutex_unlock(&map_t_lock);
}

/*
 * nssinfo_map_t_stats_notify()
 *	MAP_T stats notify callback function.
 */
static void nssinfo_map_t_stats_notify(void *data)
{
	uint64_t *node_stats;
	struct nss_map_t_stats_notification *nss_stats = (struct nss_map_t_stats_notification *)data;
	struct node *map_t_node;
	struct node **map_t_ptr;

	if (!nssinfo_coreid_ifnum_valid(nss_stats->core_id, nss_stats->if_num)) {
		return;
	}

	pthread_mutex_lock(&map_t_lock);
	map_t_ptr = &nodes[nss_stats->core_id][nss_stats->if_num];
	map_t_node = *map_t_ptr;
	if (map_t_node) {
		memcpy(map_t_node->node_stats, &nss_stats->stats, sizeof(nss_stats->stats));
		pthread_mutex_unlock(&map_t_lock);
		return;
	}
	pthread_mutex_unlock(&map_t_lock);

	map_t_node = (struct node*)calloc(1, sizeof(struct node));
	if (!map_t_node) {
		nssinfo_warn("Failed to allocate memory for map_t node\n");
		return;
	}

	node_stats = (uint64_t *)malloc(sizeof(nss_stats->stats));
	if (!node_stats) {
		nssinfo_warn("Failed to allocate memory for map_t node statistics\n");
		goto map_t_node_free;
	}

	memcpy(node_stats, &nss_stats->stats, sizeof(nss_stats->stats));
	map_t_node->node_stats = node_stats;
	map_t_node->type = nss_stats->if_type;
	map_t_node->id = nss_stats->if_num;
	map_t_node->subsystem_id = NSS_NLCMN_SUBSYS_MAP_T;

	/*
	 * Notify is guaranteed to be single threaded via Netlink listen callback
	 */
	pthread_mutex_lock(&map_t_lock);
	*map_t_ptr = map_t_node;
	pthread_mutex_unlock(&map_t_lock);
	return;

map_t_node_free:
	free(map_t_node);
}

/*
 * nssinfo_map_t_destroy()
 *	Destroy MAP_T node.
 */
static void nssinfo_map_t_destroy(uint32_t core_id, uint32_t if_num)
{
	if (nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_MAP_T].is_inited) {
		nssinfo_node_stats_destroy(&map_t_lock, core_id, if_num);
	}
}

/*
 * nssinfo_map_t_deinit()
 *	Deinitialize map_t module.
 */
void nssinfo_map_t_deinit(void *data)
{
	struct nss_nlmcast_ctx *ctx = (struct nss_nlmcast_ctx *)data;

	if (nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_MAP_T].is_inited) {
		pthread_mutex_destroy(&map_t_lock);
		nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_MAP_T].is_inited = false;
	}

	nss_nlmcast_sock_leave_grp(ctx, NSS_NLMAP_T_MCAST_GRP);
}

/*
 * nssinfo_map_t_init()
 *	Initialize MAP_T module.
 */
int nssinfo_map_t_init(void *data)
{
	int error;
	struct nss_nlmcast_ctx *ctx = (struct nss_nlmcast_ctx *)data;

	/*
	 * Subscribe for MAP_T multicast group.
	 */
	nss_nlsock_set_family(&ctx->sock, NSS_NLMAP_T_FAMILY);
	error = nss_nlmcast_sock_join_grp(ctx, NSS_NLMAP_T_MCAST_GRP);
	if (error) {
		nssinfo_warn("Unable to join MAP_T multicast group.\n");
		return error;
	}

	if (nssinfo_stats_info_init(nssinfo_stats_info_map_t,
				"/sys/kernel/debug/qca-nss-drv/strings/map_t") != 0) {
		goto fail;
	}

	if (pthread_mutex_init(&map_t_lock, NULL) != 0) {
		nssinfo_warn("Mutex init has failed for MAP_T\n");
		goto fail;
	}

	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_MAP_T].display = nssinfo_map_t_stats_display;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_MAP_T].notify = nssinfo_map_t_stats_notify;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_MAP_T].destroy = nssinfo_map_t_destroy;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_MAP_T].is_inited = true;
	return 0;
fail:
	nss_nlmcast_sock_leave_grp(ctx, NSS_NLMAP_T_MCAST_GRP);
	return -1;
}
