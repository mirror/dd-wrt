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
 * @file NSSINFO capwap handler
 */
#include "nssinfo.h"

static pthread_mutex_t capwap_lock;
static struct nssinfo_stats_info nssinfo_stats_info_capwap_encap[NSS_CAPWAP_STATS_ENCAP_MAX];
static struct nssinfo_stats_info nssinfo_stats_info_capwap_decap[NSS_CAPWAP_STATS_DECAP_MAX];

/*
 * nssinfo_capwap_encap_stats()
 *	Make a row for CAPWAP encap stats.
 */
static void nssinfo_capwap_encap_stats(struct nss_capwap_tunnel_stats *s, uint64_t *t)
{
	int i;

	for ( i = 0 ; i < NSS_CAPWAP_STATS_ENCAP_MAX; i++) {
		switch (i) {
		case NSS_CAPWAP_STATS_ENCAP_TX_PKTS:
			t[i] = s->pnode_stats.tx_packets;
			break;
		case NSS_CAPWAP_STATS_ENCAP_TX_BYTES:
			t[i] = s->pnode_stats.tx_bytes;
			break;
		case NSS_CAPWAP_STATS_ENCAP_TX_SEGMENTS:
			t[i] = s->tx_segments;
			break;
		case NSS_CAPWAP_STATS_ENCAP_TX_DROP_SG_REF:
			t[i] = s->tx_dropped_sg_ref;
			break;
		case NSS_CAPWAP_STATS_ENCAP_TX_DROP_VER_MISMATCH:
			t[i] = s->tx_dropped_ver_mis;
			break;
		case NSS_CAPWAP_STATS_ENCAP_TX_DROP_UNALIGN:
			t[i] = 0;
			break;
		case NSS_CAPWAP_STATS_ENCAP_TX_DROP_HEADER_ROOM:
			t[i] = s->tx_dropped_hroom;
			break;
		case NSS_CAPWAP_STATS_ENCAP_TX_DROP_DTLS:
			t[i] = s->tx_dropped_dtls;
			break;
		case NSS_CAPWAP_STATS_ENCAP_TX_DROP_NWIRELESS:
			t[i] = s->tx_dropped_nwireless;
			break;
		case NSS_CAPWAP_STATS_ENCAP_TX_DROP_QUEUE_FULL:
			t[i] = s->tx_queue_full_drops;
			break;
		case NSS_CAPWAP_STATS_ENCAP_TX_DROP_MEM_FAIL:
			t[i] = s->tx_mem_failure_drops;
			break;
		case NSS_CAPWAP_STATS_ENCAP_FAST_MEM:
			t[i] = s->fast_mem;
			break;
		default:
			return;
		}
	}
}

/*
 * nssinfo_capwap_decap_stats()
 *	Make a row for CAPWAP decap stats.
 */
static void nssinfo_capwap_decap_stats(struct nss_capwap_tunnel_stats *s, uint64_t *t)
{
	int i;

	for (i = 0 ; i < NSS_CAPWAP_STATS_DECAP_MAX; i++) {
		switch (i) {
		case NSS_CAPWAP_STATS_DECAP_RX_PKTS:
			t[i] = s->pnode_stats.rx_packets;
			break;
		case NSS_CAPWAP_STATS_DECAP_RX_BYTES:
			t[i] = s->pnode_stats.rx_bytes;
			break;
		case NSS_CAPWAP_STATS_DECAP_RX_DTLS_PKTS:
			t[i] = s->dtls_pkts;
			break;
		case NSS_CAPWAP_STATS_DECAP_RX_SEGMENTS:
			t[i] = s->rx_segments;
			break;
		case NSS_CAPWAP_STATS_DECAP_RX_DROP:
			t[i] = s->pnode_stats.rx_dropped;
			break;
		case NSS_CAPWAP_STATS_DECAP_RX_DROP_OVERSIZE:
			t[i] = s->rx_oversize_drops;
			break;
		case NSS_CAPWAP_STATS_DECAP_RX_DROP_FRAG_TIMEOUT:
			t[i] = s->rx_frag_timeout_drops;
			break;
		case NSS_CAPWAP_STATS_DECAP_RX_DROP_DUP_FRAG:
			t[i] = s->rx_dup_frag;
			break;
		case NSS_CAPWAP_STATS_DECAP_RX_DROP_FRAG_GAP:
			t[i] = s->rx_frag_gap_drops;
			break;
		case NSS_CAPWAP_STATS_DECAP_RX_DROP_QUEUE_FULL:
			t[i] = s->rx_n2h_drops;
			break;
		case NSS_CAPWAP_STATS_DECAP_RX_DROP_N2H_QUEUE_FULL:
			t[i] = s->rx_n2h_queue_full_drops;
			break;
		case NSS_CAPWAP_STATS_DECAP_RX_DROP_MEM_FAIL:
			t[i] = s->rx_mem_failure_drops;
			break;
		case NSS_CAPWAP_STATS_DECAP_RX_DROP_CHECKSUM:
			t[i] = s->rx_csum_drops;
			break;
		case NSS_CAPWAP_STATS_DECAP_RX_MALFORMED:
			t[i] = s->rx_malformed;
			break;
		case NSS_CAPWAP_STATS_DECAP_FAST_MEM:
			t[i] = s->fast_mem;
			break;
		default:
			break;
		}
	}
}

/*
 * nssinfo_capwap_stats_display()
 *	CAPWAP display callback function.
 */
static void nssinfo_capwap_stats_display(int core, char *input)
{
	int i, id;
	struct node *capwap_node;
	struct nss_capwap_tunnel_stats *stats;
	char *str_rx, str_tx[NSSINFO_STR_LEN], str_drop[NSSINFO_STR_LEN];
	char node_name[NSSINFO_STR_LEN];
	char str[NSSINFO_STR_LEN], *tok = NULL;
	bool active_if_available = false;
	uint64_t decap_stats[NSS_CAPWAP_STATS_DECAP_MAX];
	uint64_t encap_stats[NSS_CAPWAP_STATS_ENCAP_MAX];

	pthread_mutex_lock(&capwap_lock);
	for (i = 0; i < NSS_MAX_NET_INTERFACES; i++) {
		capwap_node = nodes[core][i];
		if (!capwap_node || (capwap_node->type != NSS_DYNAMIC_INTERFACE_TYPE_CAPWAP_HOST_INNER
			&& capwap_node->type != NSS_DYNAMIC_INTERFACE_TYPE_CAPWAP_OUTER)) {
			continue;
		}

		active_if_available = true;

		stats = (struct nss_capwap_tunnel_stats *)capwap_node->node_stats;

		snprintf(node_name, sizeof(node_name), "%s[%llu]",
				nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_CAPWAP].subsystem_name,
				capwap_node->id);

		/*
		 * If the input is "capwap", display summary of all the active interfaces.
		 */
		if (input && !strncmp(input, nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_CAPWAP].subsystem_name, strlen(input))) {
			goto display_summary;
		}

		/*
		 * If the input is "capwap[0-255]", display the stats of that interface.
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

			if (id != capwap_node->id) {
				continue;
			}
		}

		if (display_all_stats) {
			nssinfo_capwap_decap_stats(stats, decap_stats);
			nssinfo_print_all(node_name, "CAPWAP Decap Stats",
					nssinfo_stats_info_capwap_decap, NSS_CAPWAP_STATS_DECAP_MAX,
					(uint64_t *)decap_stats);

			nssinfo_capwap_encap_stats(stats, encap_stats);
			nssinfo_print_all(node_name, "CAPWAP Encap Stats",
					nssinfo_stats_info_capwap_encap, NSS_CAPWAP_STATS_ENCAP_MAX,
					(uint64_t *)encap_stats);

			goto done;
		}

display_summary:
		if (stats->pnode_stats.rx_packets > 0 || stats->pnode_stats.tx_packets > 0 || arguments.verbose) {
			str_rx = &str[0];
			char *format_stats = nssinfo_format_stats(stats->pnode_stats.rx_packets);
			strlcpy(str_rx, format_stats, sizeof(str_rx));
			format_stats = nssinfo_format_stats(stats->pnode_stats.tx_packets);
			strlcpy(str_tx, format_stats, sizeof(str_tx));
			format_stats = nssinfo_format_stats(stats->pnode_stats.rx_dropped);
			strlcpy(str_drop, format_stats, sizeof(str_drop));
			nssinfo_stats_print(nssinfo_summary_fmt, node_name, str_rx, str_tx, str_drop, "");
		}
	}

	if (display_all_stats && !active_if_available) {
		++invalid_input;
		nssinfo_trace("CAPWAP interface not available\n");
	}
done:
	pthread_mutex_unlock(&capwap_lock);
}

/*
 * nssinfo_capwap_stats_notify()
 *	CAPWAP stats notify callback function.
 */
static void nssinfo_capwap_stats_notify(void *data)
{
	uint64_t *node_stats;
	struct nss_capwap_stats_notification *nss_stats = (struct nss_capwap_stats_notification *)data;
	struct node *capwap_node;
	struct node **capwap_ptr;

	if (!nssinfo_coreid_ifnum_valid(nss_stats->core_id, nss_stats->if_num)) {
		return;
	}

	pthread_mutex_lock(&capwap_lock);
	capwap_ptr = &nodes[nss_stats->core_id][nss_stats->if_num];
	capwap_node = *capwap_ptr;
	if (capwap_node) {
		memcpy(capwap_node->node_stats, &nss_stats->stats, sizeof(nss_stats->stats));
		pthread_mutex_unlock(&capwap_lock);
		return;
	}
	pthread_mutex_unlock(&capwap_lock);

	capwap_node = (struct node *)calloc(1, sizeof(struct node));
	if (!capwap_node) {
		nssinfo_warn("Failed to allocate memory for capwap node\n");
		return;
	}

	node_stats = (uint64_t *)malloc(sizeof(nss_stats->stats));
	if (!node_stats) {
		nssinfo_warn("Failed to allocate memory for capwap node stats\n");
		goto capwap_node_free;
	}

	memcpy(node_stats, &nss_stats->stats, sizeof(nss_stats->stats));
	capwap_node->node_stats = node_stats;
	capwap_node->id = nss_stats->if_num;
	capwap_node->subsystem_id = NSS_NLCMN_SUBSYS_CAPWAP;

	/*
	 * Notify is guaranteed to be single threaded via Netlink listen callback
	 */
	pthread_mutex_lock(&capwap_lock);
	*capwap_ptr = capwap_node;
	pthread_mutex_unlock(&capwap_lock);
	return;

capwap_node_free:
	free(capwap_node);
}

/*
 * nssinfo_capwap_destroy()
 *	Destroy CAPWAP node.
 */
static void nssinfo_capwap_destroy(uint32_t core_id, uint32_t if_num)
{
	if (nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_CAPWAP].is_inited) {
		nssinfo_node_stats_destroy(&capwap_lock, core_id, if_num);
	}
}

/*
 * nssinfo_capwap_deinit()
 *	Deinitialize capwap module.
 */
void nssinfo_capwap_deinit(void *data)
{
	struct nss_nlmcast_ctx *ctx = (struct nss_nlmcast_ctx *)data;

	if (nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_CAPWAP].is_inited) {
		pthread_mutex_destroy(&capwap_lock);
		nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_CAPWAP].is_inited = false;
	}

	nss_nlmcast_sock_leave_grp(ctx, NSS_NLCAPWAP_MCAST_GRP);
}

/*
 * nssinfo_capwap_init()
 *	Initialize CAPWAP module.
 */
int nssinfo_capwap_init(void *data)
{
	int error;
	struct nss_nlmcast_ctx *ctx = (struct nss_nlmcast_ctx *)data;

	/*
	 * Subscribe for CAPWAP MCAST group.
	 */
	nss_nlsock_set_family(&ctx->sock, NSS_NLCAPWAP_FAMILY);
	error = nss_nlmcast_sock_join_grp(ctx, NSS_NLCAPWAP_MCAST_GRP);
	if (error) {
		nssinfo_warn("Unable to join CAPWAP mcast group.\n");
		return error;
	}

	if (nssinfo_stats_info_init(nssinfo_stats_info_capwap_encap,
				"/sys/kernel/debug/qca-nss-drv/strings/capwap_encap") != 0) {
		goto fail;
	}

	if (nssinfo_stats_info_init(nssinfo_stats_info_capwap_decap,
				"/sys/kernel/debug/qca-nss-drv/strings/capwap_decap") != 0) {
		goto fail;
	}

	if (pthread_mutex_init(&capwap_lock, NULL) != 0) {
		nssinfo_warn("Mutex init has failed for CAPWAP\n");
		goto fail;
	}

	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_CAPWAP].display = nssinfo_capwap_stats_display;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_CAPWAP].notify = nssinfo_capwap_stats_notify;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_CAPWAP].destroy = nssinfo_capwap_destroy;
	nssinfo_subsystem_array[NSS_NLCMN_SUBSYS_CAPWAP].is_inited = true;
	return 0;
fail:
	nss_nlmcast_sock_leave_grp(ctx, NSS_NLCAPWAP_MCAST_GRP);
	return -1;
}
