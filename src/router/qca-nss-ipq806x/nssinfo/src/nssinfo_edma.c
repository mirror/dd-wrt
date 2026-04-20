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
 * @file NSSINFO edma handler
 */
#include "nssinfo.h"
#include <nss_edma.h>
#include <nss_nledma_if.h>

static pthread_mutex_t edma_lock;
static struct nssinfo_stats_info nss_stats_str_node[NSS_STATS_NODE_MAX];

/*
 * nssinfo_edma_stats_display()
 *	EDMA display callback function.
 */
static void nssinfo_edma_stats_display(int core, char *input)
{
	int id;
	struct node *edma_node;
	char node_name[NSSINFO_STR_LEN];
	char str[NSSINFO_STR_LEN], *tok = NULL;
	bool summary_header_is_displayed = false;

	pthread_mutex_lock(&edma_lock);
	edma_node = nodes[core][NSS_EDMA_INTERFACE];

	while (edma_node) {

		snprintf(node_name, sizeof(node_name), "%s[%llu]",
				nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_EDMA].subsystem_name,
				edma_node->id);

		/*
		 * If the input is "edma", display summary of all the ports.
		 */
		if (input && !strncmp(input, nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_EDMA].subsystem_name, strlen(input))) {
			if (display_all_stats && !summary_header_is_displayed) {
				nssinfo_print_summary_header();
				summary_header_is_displayed = true;
			}
			goto display_summary;
		}

		/*
		 * If the input is "edma[0-255]", display the stats of that port.
		 */
		if (input) {
		 	/* Parsing 'input' to get the port number */
			strlcpy(str, input, sizeof(str));
			char *rest = NULL;
			tok = strtok_r(str, "[", &rest);
			tok = strtok_r(NULL, "]", &rest);

			if (!tok || tok[0] == '\0' || (id = atoi(tok)) < 0 || id >= NSS_EDMA_NUM_PORTS_MAX) {
				++invalid_input;
				pthread_mutex_unlock(&edma_lock);
				nssinfo_trace("Invalid port number: %s\n", tok);
				return;
			}

			if (id != edma_node->id) {
				edma_node = edma_node->next;
				continue;
			}
		}

		if (display_all_stats) {
			nssinfo_print_all(node_name, "EDMA Common Stats", nss_stats_str_node, NSS_STATS_NODE_MAX, (uint64_t *)edma_node->cmn_node_stats);
			pthread_mutex_unlock(&edma_lock);
			return;
		}

display_summary:
		nssinfo_print_summary(node_name, (uint64_t *)edma_node->cmn_node_stats, NULL, 0);

		edma_node = edma_node->next;
	}
	pthread_mutex_unlock(&edma_lock);
}

/*
 * nssinfo_edma_find_port()
 *      Find an edma port node.
 */
static struct node* nssinfo_edma_find_port(struct node *head, int id)
{
	struct node *current = head;

	while (current) {
		if (current->id == id) {
			return current;
		}
		current = current->next;
	}
	return NULL;
}

/*
 * nssinfo_edma_add_port()
 *      Add an EDMA port node.
 */
static struct node* nssinfo_edma_add_port(struct node **head, struct nss_nledma_stats *stats)
{
	struct node *current, *new_node;
	uint64_t *cmn_node_stats;
	uint64_t id = stats->port_id;

	new_node = (struct node*)calloc(1, sizeof(struct node));
	if (!new_node) {
		nssinfo_warn("Failed to allocate memory for EDMA node\n");
		return NULL;
	}

	cmn_node_stats = (uint64_t *)malloc(sizeof(stats->cmn_node_stats));
	if (!cmn_node_stats) {
		nssinfo_warn("Failed to allocate memory for EDMA node stats\n");
		goto edma_node_free;
	}

	memcpy(cmn_node_stats, &stats->cmn_node_stats, sizeof(stats->cmn_node_stats));
	new_node->cmn_node_stats = cmn_node_stats;
	new_node->id = stats->port_id;
	new_node->subsystem_id = NSS_NLCMN_SUBSYS_EDMA;

	if (*head == NULL || (*head)->id >= id) {
		new_node->next = *head;
		*head = new_node;

		nodes[stats->core_id][NSS_EDMA_INTERFACE] = new_node;
	} else {
		current = *head;
		while (current->next && current->next->id < id) {
			current = current->next;
		}
		new_node->next = current->next;
		current->next = new_node;
	}

	return new_node;

edma_node_free:
	free(new_node);
	return NULL;
}

/*
 * nssinfo_edma_stats_notify()
 *	EDMA stats notify callback function.
 */
static void nssinfo_edma_stats_notify(void *data)
{
	struct node *port_node;
	struct nss_nledma_stats *nss_stats = (struct nss_nledma_stats *)data;
	struct node *edma_head;

	if (!nssinfo_coreid_ifnum_valid(nss_stats->core_id, NSS_EDMA_INTERFACE)) {
		return;
	}

	pthread_mutex_lock(&edma_lock);
	edma_head = nodes[nss_stats->core_id][NSS_EDMA_INTERFACE];
	if (!edma_head) {
		edma_head = nssinfo_edma_add_port(&edma_head, nss_stats);
		pthread_mutex_unlock(&edma_lock);
		return;
	}

	port_node = nssinfo_edma_find_port(edma_head, nss_stats->port_id);
	if (!port_node) {
		port_node = nssinfo_edma_add_port(&edma_head, nss_stats);
	} else {
		memcpy(port_node->cmn_node_stats, &nss_stats->cmn_node_stats, sizeof(nss_stats->cmn_node_stats));
	}
	pthread_mutex_unlock(&edma_lock);
}

/*
 * nssinfo_edma_destroy()
 *      Destroy EDMA node.
 */
static void nssinfo_edma_destroy(uint32_t core_id, uint32_t if_num)
{
	if (nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_EDMA].is_inited) {
		nssinfo_node_stats_destroy(&edma_lock, core_id, NSS_EDMA_INTERFACE);
	}
}

/*
 * nssinfo_edma_deinit()
 *	Deinitialize edma module.
 */
void nssinfo_edma_deinit(void *data)
{
	struct nss_nlmcast_ctx *ctx = (struct nss_nlmcast_ctx *)data;

	if (nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_EDMA].is_inited) {
		pthread_mutex_destroy(&edma_lock);
		nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_EDMA].is_inited = false;
	}

	nss_nlmcast_sock_leave_grp(ctx, NSS_NLEDMA_MCAST_GRP);
}

/*
 * nssinfo_edma_init()
 *	Initialize EDMA module.
 */
int nssinfo_edma_init(void *data)
{
	int error;
	struct nss_nlmcast_ctx *ctx = (struct nss_nlmcast_ctx *)data;

	/*
	 * Subscribe for EDMA MCAST group.
	 */
	nss_nlsock_set_family(&ctx->sock, NSS_NLEDMA_FAMILY);
	error = nss_nlmcast_sock_join_grp(ctx, NSS_NLEDMA_MCAST_GRP);
	if (error) {
		nssinfo_warn("Unable to join edma mcast group. \n");
		return error;
	}

	if (nssinfo_stats_info_init(nss_stats_str_node,
				"/sys/kernel/debug/qca-nss-drv/strings/edma/ports/common_stats_str") != 0) {
		goto fail;
	}

	if (pthread_mutex_init(&edma_lock, NULL) != 0) {
		nssinfo_warn("Mutex init has failed for EDMA\n");
		goto fail;
	}

	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_EDMA].display = nssinfo_edma_stats_display;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_EDMA].notify = nssinfo_edma_stats_notify;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_EDMA].destroy = nssinfo_edma_destroy;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_EDMA].is_inited = true;
	return 0;
fail:
	nss_nlmcast_sock_leave_grp(ctx, NSS_NLEDMA_MCAST_GRP);
	return -1;
}
