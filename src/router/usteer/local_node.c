/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Copyright (C) 2020 embedd.ch 
 *   Copyright (C) 2020 Felix Fietkau <nbd@nbd.name> 
 *   Copyright (C) 2020 John Crispin <john@phrozen.org> 
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#ifdef linux
#include <netinet/ether.h>
#endif
#include <net/if.h>
#include <stdlib.h>

#include <libubox/avl-cmp.h>
#include <libubox/blobmsg_json.h>
#include "usteer.h"
#include "node.h"

AVL_TREE(local_nodes, avl_strcmp, false, NULL);
static struct blob_buf b;
static char *node_up_script;

static void
usteer_local_node_state_reset(struct usteer_local_node *ln)
{
	if (ln->req_state == REQ_IDLE)
		return;

	ubus_abort_request(ubus_ctx, &ln->req);
	uloop_timeout_cancel(&ln->req_timer);
	ln->req_state = REQ_IDLE;
}

static void
usteer_local_node_pending_bss_tm_free(struct usteer_local_node *ln)
{
	struct usteer_bss_tm_query *query, *tmp;

	list_for_each_entry_safe(query, tmp, &ln->bss_tm_queries, list) {
		list_del(&query->list);
		free(query);
	}
}

static void
usteer_free_node(struct ubus_context *ctx, struct usteer_local_node *ln)
{
	struct usteer_node_handler *h;

	list_for_each_entry(h, &node_handlers, list) {
		if (!h->free_node)
			continue;
		h->free_node(&ln->node);
	}

	usteer_local_node_pending_bss_tm_free(ln);
	usteer_local_node_state_reset(ln);
	usteer_sta_node_cleanup(&ln->node);
	usteer_measurement_report_node_cleanup(&ln->node);
	uloop_timeout_cancel(&ln->update);
	uloop_timeout_cancel(&ln->bss_tm_queries_timeout);
	avl_delete(&local_nodes, &ln->node.avl);
	ubus_unregister_subscriber(ctx, &ln->ev);
	kvlist_free(&ln->node_info);
	free(ln);
}

struct usteer_local_node *usteer_local_node_by_bssid(uint8_t *bssid) {
	struct usteer_local_node *ln;
	struct usteer_node *n;

	for_each_local_node(n) {
		ln = container_of(n, struct usteer_local_node, node);
		if (!memcmp(n->bssid, bssid, 6))
			return ln;
	}

	return NULL;
}

static void
usteer_handle_remove(struct ubus_context *ctx, struct ubus_subscriber *s,
		    uint32_t id)
{
	struct usteer_local_node *ln = container_of(s, struct usteer_local_node, ev);

	usteer_free_node(ctx, ln);
}

static int
usteer_handle_bss_tm_query(struct usteer_local_node *ln, struct blob_attr *msg)
{
	enum {
		BSS_TM_QUERY_ADDRESS,
		BSS_TM_QUERY_DIALOG_TOKEN,
		BSS_TM_QUERY_CANDIDATE_LIST,
		__BSS_TM_QUERY_MAX
	};
	struct blobmsg_policy policy[__BSS_TM_QUERY_MAX] = {
		[BSS_TM_QUERY_ADDRESS] = { .name = "address", .type = BLOBMSG_TYPE_STRING },
		[BSS_TM_QUERY_DIALOG_TOKEN] = { .name = "dialog-token", .type = BLOBMSG_TYPE_INT8 },
		[BSS_TM_QUERY_CANDIDATE_LIST] = { .name = "candidate-list", .type = BLOBMSG_TYPE_STRING },
	};
	struct blob_attr *tb[__BSS_TM_QUERY_MAX];
	struct usteer_bss_tm_query *query;
	uint8_t *sta_addr;

	blobmsg_parse(policy, __BSS_TM_QUERY_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[BSS_TM_QUERY_ADDRESS] || !tb[BSS_TM_QUERY_DIALOG_TOKEN])
		return 0;

	query = calloc(1, sizeof(*query));
	if (!query)
		return 0;

	query->dialog_token = blobmsg_get_u8(tb[BSS_TM_QUERY_DIALOG_TOKEN]);

	sta_addr = (uint8_t *) ether_aton(blobmsg_get_string(tb[BSS_TM_QUERY_ADDRESS]));
	if (!sta_addr)
		return 0;

	memcpy(query->sta_addr, sta_addr, 6);

	list_add(&query->list, &ln->bss_tm_queries);
	uloop_timeout_set(&ln->bss_tm_queries_timeout, 1);

	return 1;
}

static int
usteer_handle_bss_tm_response(struct usteer_local_node *ln, struct blob_attr *msg)
{
	enum {
		BSS_TM_RESPONSE_ADDRESS,
		BSS_TM_RESPONSE_STATUS_CODE,
		__BSS_TM_RESPONSE_MAX
	};
	struct blobmsg_policy policy[__BSS_TM_RESPONSE_MAX] = {
		[BSS_TM_RESPONSE_ADDRESS] = { .name = "address", .type = BLOBMSG_TYPE_STRING },
		[BSS_TM_RESPONSE_STATUS_CODE] = { .name = "status-code", .type = BLOBMSG_TYPE_INT8 },
	};
	struct blob_attr *tb[__BSS_TM_RESPONSE_MAX];
	struct sta_info *si;
	struct sta *sta;
	uint8_t *sta_addr;

	blobmsg_parse(policy, __BSS_TM_RESPONSE_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[BSS_TM_RESPONSE_ADDRESS] || !tb[BSS_TM_RESPONSE_STATUS_CODE])
		return 0;

	sta_addr = (uint8_t *) ether_aton(blobmsg_get_string(tb[BSS_TM_RESPONSE_ADDRESS]));
	if (!sta_addr)
		return 0;

	sta = usteer_sta_get(sta_addr, false);
	if (!sta)
		return 0;

	si = usteer_sta_info_get(sta, &ln->node, false);
	if (!si)
		return 0;

	si->bss_transition_response.status_code = blobmsg_get_u8(tb[BSS_TM_RESPONSE_STATUS_CODE]);
	si->bss_transition_response.timestamp = current_time;

	if (si->bss_transition_response.status_code) {
		/* Cancel imminent kick in case BSS transition was rejected */
		si->kick_time = 0;
	}

	return 0;
}

static int
usteer_local_node_handle_beacon_report(struct usteer_local_node *ln, struct blob_attr *msg)
{
	enum {
		BR_ADDRESS,
		BR_BSSID,
		BR_RCPI,
		BR_RSNI,
		__BR_MAX
	};
	struct blobmsg_policy policy[__BR_MAX] = {
		[BR_ADDRESS] = { .name = "address", .type = BLOBMSG_TYPE_STRING },
		[BR_BSSID] = { .name = "bssid", .type = BLOBMSG_TYPE_STRING },
		[BR_RCPI] = { .name = "rcpi", .type = BLOBMSG_TYPE_INT16 },
		[BR_RSNI] = { .name = "rsni", .type = BLOBMSG_TYPE_INT16 },
	};
	struct blob_attr *tb[__BR_MAX];
	struct usteer_node *node;
	uint8_t *addr;
	struct sta *sta;

	blobmsg_parse(policy, __BR_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[BR_ADDRESS] || !tb[BR_BSSID] || !tb[BR_RCPI] || !tb[BR_RSNI])
		return 0;

	addr = (uint8_t *) ether_aton(blobmsg_get_string(tb[BR_ADDRESS]));
	if (!addr)
		return 0;

	sta = usteer_sta_get(addr, false);
	if (!sta)
		return 0;

	addr = (uint8_t *) ether_aton(blobmsg_get_string(tb[BR_BSSID]));
	if (!addr)
		return 0;

	node = usteer_node_by_bssid(addr);
	if (!node)
		return 0;

	usteer_measurement_report_add(sta, node,
				      (uint8_t)blobmsg_get_u16(tb[BR_RCPI]),
				      (uint8_t)blobmsg_get_u16(tb[BR_RSNI]),
				      current_time);
	return 0;
}

static int
usteer_local_node_handle_link_measurement_report(struct usteer_local_node *ln, struct blob_attr *msg)
{
	enum {
		LMR_ADDRESS,
		LMR_RCPI,
		LMR_RSNI,
		__LMR_MAX
	};
	struct blobmsg_policy policy[__LMR_MAX] = {
		[LMR_ADDRESS] = { .name = "address", .type = BLOBMSG_TYPE_STRING },
		[LMR_RCPI] = { .name = "rcpi", .type = BLOBMSG_TYPE_INT16 },
		[LMR_RSNI] = { .name = "rsni", .type = BLOBMSG_TYPE_INT16 },
	};
	struct blob_attr *tb[__LMR_MAX];
	uint8_t *addr;
	struct sta *sta;

	blobmsg_parse(policy, __LMR_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[LMR_ADDRESS] || !tb[LMR_RCPI] || !tb[LMR_RSNI])
		return 0;

	addr = (uint8_t *) ether_aton(blobmsg_get_string(tb[LMR_ADDRESS]));
	if (!addr)
		return 0;

	sta = usteer_sta_get(addr, false);
	if (!sta)
		return 0;

	usteer_measurement_report_add(sta, &ln->node,
				      (uint8_t)blobmsg_get_u16(tb[LMR_RCPI]),
				      (uint8_t)blobmsg_get_u16(tb[LMR_RSNI]),
				      current_time);
	return 0;
}

static int
usteer_handle_event(struct ubus_context *ctx, struct ubus_object *obj,
		   struct ubus_request_data *req, const char *method,
		   struct blob_attr *msg)
{
	enum {
		EVENT_ADDR,
		EVENT_SIGNAL,
		EVENT_TARGET,
		EVENT_FREQ,
		__EVENT_MAX
	};
	struct blobmsg_policy policy[__EVENT_MAX] = {
		[EVENT_ADDR] = { .name = "address", .type = BLOBMSG_TYPE_STRING },
		[EVENT_SIGNAL] = { .name = "signal", .type = BLOBMSG_TYPE_INT32 },
		[EVENT_TARGET] = { .name = "target", .type = BLOBMSG_TYPE_STRING },
		[EVENT_FREQ] = { .name = "freq", .type = BLOBMSG_TYPE_INT32 },
	};
	enum usteer_event_type ev_type = __EVENT_TYPE_MAX;
	struct blob_attr *tb[__EVENT_MAX];
	struct usteer_local_node *ln;
	struct usteer_node *node;
	int signal = NO_SIGNAL;
	int freq = 0;
	const char *addr_str;
	const uint8_t *addr;
	int i;
	bool ret;

	usteer_update_time();

	ln = container_of(obj, struct usteer_local_node, ev.obj);

	if(!strcmp(method, "bss-transition-query")) {
		return usteer_handle_bss_tm_query(ln, msg);
	} else if(!strcmp(method, "bss-transition-response")) {
		return usteer_handle_bss_tm_response(ln, msg);
	} else if(!strcmp(method, "beacon-report")) {
		return usteer_local_node_handle_beacon_report(ln, msg);
	} else if(!strcmp(method, "link-measurement-report")) {
		return usteer_local_node_handle_link_measurement_report(ln, msg);
	}

	for (i = 0; i < ARRAY_SIZE(event_types); i++) {
		if (strcmp(method, event_types[i]) != 0)
			continue;

		ev_type = i;
		break;
	}

	ln = container_of(obj, struct usteer_local_node, ev.obj);
	node = &ln->node;
	blobmsg_parse(policy, __EVENT_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[EVENT_ADDR] || !tb[EVENT_FREQ])
		return UBUS_STATUS_INVALID_ARGUMENT;

	if (tb[EVENT_SIGNAL])
		signal = (int32_t) blobmsg_get_u32(tb[EVENT_SIGNAL]);

	if (tb[EVENT_FREQ])
		freq = blobmsg_get_u32(tb[EVENT_FREQ]);

	addr_str = blobmsg_data(tb[EVENT_ADDR]);
	addr = (uint8_t *) ether_aton(addr_str);
	if (!addr)
		return UBUS_STATUS_INVALID_ARGUMENT;

	ret = usteer_handle_sta_event(node, addr, ev_type, freq, signal);

	MSG(DEBUG, "received %s event from %s, signal=%d, freq=%d, handled:%s\n",
	    method, addr_str, signal, freq, ret ? "true" : "false");

	return ret ? 0 : 17 /* WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA */;
}

static void
usteer_local_node_assoc_update(struct sta_info *si, struct blob_attr *data)
{
	enum {
		MSG_ASSOC,
		__MSG_MAX,
	};
	static struct blobmsg_policy policy[__MSG_MAX] = {
		[MSG_ASSOC] = { "assoc", BLOBMSG_TYPE_BOOL },
	};
	struct blob_attr *tb[__MSG_MAX];
	struct usteer_remote_node *rn;
	struct sta_info *remote_si;

	blobmsg_parse(policy, __MSG_MAX, tb, blobmsg_data(data), blobmsg_data_len(data));
	if (tb[MSG_ASSOC] && blobmsg_get_u8(tb[MSG_ASSOC])) {
		if (si->connected == STA_NOT_CONNECTED) {
			/* New connection. Check if STA roamed. */
			for_each_remote_node(rn) {
				remote_si = usteer_sta_info_get(si->sta, &rn->node, NULL);
				if (!remote_si)
					continue;

				if (current_time - remote_si->last_connected < config.roam_process_timeout) {
					rn->node.roam_events.source++;
					/* Don't abort looking for roam sources here.
					 * The client might have roamed via another node
					 * within the roam-timeout.
					 */
				}
			}
		}
		si->connected = STA_CONNECTED;
	}
}

static void
usteer_local_node_update_sta_rrm_wnm(struct sta_info *si, struct blob_attr *client_attr)
{
	static const struct blobmsg_policy mbo_policy = {
		.name = "mbo",
		.type = BLOBMSG_TYPE_BOOL,
	};
	static const struct blobmsg_policy rrm_policy = {
		.name = "rrm",
		.type = BLOBMSG_TYPE_ARRAY,
	};
	static const struct blobmsg_policy ext_capa_policy = {
		.name = "extended_capabilities",
		.type = BLOBMSG_TYPE_ARRAY,
	};
	struct blob_attr *mbo_blob = NULL, *rrm_blob = NULL, *wnm_blob = NULL, *cur;
	int rem;
	int i = 0;

	/* MBO */
	blobmsg_parse(&mbo_policy, 1, &mbo_blob, blobmsg_data(client_attr), blobmsg_data_len(client_attr));

	if (mbo_blob)
		si->mbo = blobmsg_get_u8(mbo_blob);
	else
		si->mbo = false;

	/* RRM */
	blobmsg_parse(&rrm_policy, 1, &rrm_blob, blobmsg_data(client_attr), blobmsg_data_len(client_attr));
	if (!rrm_blob)
		return;

	si->rrm = blobmsg_get_u32(blobmsg_data(rrm_blob));

	/* Extended Capabilities / WNM */
	blobmsg_parse(&ext_capa_policy, 1, &wnm_blob, blobmsg_data(client_attr), blobmsg_data_len(client_attr));
	if (!wnm_blob)
		return;

	blobmsg_for_each_attr(cur, wnm_blob, rem) {
		if (blobmsg_type(cur) != BLOBMSG_TYPE_INT32)
			return;
		
		if (i == 2) {
			if (blobmsg_get_u32(cur) & (1 << 3))
				si->bss_transition = true;
		}

		i++;
	}
}

static void
usteer_local_node_set_assoc(struct usteer_local_node *ln, struct blob_attr *cl)
{
	struct usteer_node *node = &ln->node;
	struct usteer_node_handler *h;
	struct blob_attr *cur;
	struct sta_info *si;
	struct sta *sta;
	int n_assoc = 0;
	int rem;

	usteer_update_time();

	list_for_each_entry(si, &node->sta_info, node_list) {
		if (si->connected)
			si->connected = STA_DISCONNECTED;
	}

	blobmsg_for_each_attr(cur, cl, rem) {
		uint8_t *addr = (uint8_t *) ether_aton(blobmsg_name(cur));
		bool create;

		if (!addr)
			continue;

		sta = usteer_sta_get(addr, true);
		si = usteer_sta_info_get(sta, node, &create);
		list_for_each_entry(h, &node_handlers, list) {
			if (!h->update_sta)
				continue;

			h->update_sta(node, si);
		}
		usteer_local_node_assoc_update(si, cur);
		if (si->connected == STA_CONNECTED) {
			si->last_connected = current_time;
			n_assoc++;
		}

		/* Read RRM information */
		usteer_local_node_update_sta_rrm_wnm(si, cur);
	}

	node->n_assoc = n_assoc;

	list_for_each_entry(si, &node->sta_info, node_list) {
		if (si->connected != STA_DISCONNECTED)
			continue;

		usteer_sta_disconnected(si);
		MSG(VERBOSE, "station "MAC_ADDR_FMT" disconnected from node %s\n",
			MAC_ADDR_DATA(si->sta->addr), usteer_node_name(node));
	}
}

static void
usteer_local_node_list_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	enum {
		MSG_FREQ,
		MSG_CLIENTS,
		__MSG_MAX,
	};
	static struct blobmsg_policy policy[__MSG_MAX] = {
		[MSG_FREQ] = { "freq", BLOBMSG_TYPE_INT32 },
		[MSG_CLIENTS] = { "clients", BLOBMSG_TYPE_TABLE },
	};
	struct blob_attr *tb[__MSG_MAX];
	struct usteer_local_node *ln;
	struct usteer_node *node;

	ln = container_of(req, struct usteer_local_node, req);
	node = &ln->node;

	blobmsg_parse(policy, __MSG_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[MSG_FREQ] || !tb[MSG_CLIENTS])
		return;

	node->freq = blobmsg_get_u32(tb[MSG_FREQ]);
	usteer_local_node_set_assoc(ln, tb[MSG_CLIENTS]);
}

static void
usteer_local_node_status_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	enum {
		MSG_FREQ,
		MSG_CHANNEL,
		MSG_OP_CLASS,
		MSG_BEACON_INTERVAL,
		MSG_SSID,
		MSG_BSSID,
		MSG_N,
		MSG_VHT,
		MSG_HE,
		__MSG_MAX,
	};
	static struct blobmsg_policy policy[__MSG_MAX] = {
		[MSG_FREQ] = { "freq", BLOBMSG_TYPE_INT32 },
		[MSG_CHANNEL] = { "channel", BLOBMSG_TYPE_INT32 },
		[MSG_OP_CLASS] = { "op_class", BLOBMSG_TYPE_INT32 },
		[MSG_BEACON_INTERVAL] = { "beacon_interval", BLOBMSG_TYPE_INT32 },
		[MSG_SSID] = { "ssid", BLOBMSG_TYPE_STRING },
		[MSG_BSSID] = { "bssid", BLOBMSG_TYPE_STRING },
		[MSG_N] = { "n", BLOBMSG_TYPE_INT32 },
		[MSG_VHT] = { "vht", BLOBMSG_TYPE_INT32 },
		[MSG_HE] = { "he", BLOBMSG_TYPE_INT32 },
	};
	struct blob_attr *tb[__MSG_MAX];
	struct usteer_local_node *ln;
	struct usteer_node *node;

	ln = container_of(req, struct usteer_local_node, req);
	node = &ln->node;

	blobmsg_parse(policy, __MSG_MAX, tb, blob_data(msg), blob_len(msg));
	if (tb[MSG_FREQ])
		node->freq = blobmsg_get_u32(tb[MSG_FREQ]);
	if (tb[MSG_N])
		node->n = blobmsg_get_u32(tb[MSG_N]);
	if (tb[MSG_VHT])
		node->vht = blobmsg_get_u32(tb[MSG_VHT]);
	if (tb[MSG_HE])
		node->he = blobmsg_get_u32(tb[MSG_HE]);
	if (tb[MSG_CHANNEL])
		node->channel = blobmsg_get_u32(tb[MSG_CHANNEL]);
	if (tb[MSG_OP_CLASS])
		node->op_class = blobmsg_get_u32(tb[MSG_OP_CLASS]);	
	if (tb[MSG_BSSID]) {
		uint8_t *addr = (uint8_t *) ether_aton(blobmsg_get_string(tb[MSG_BSSID]));
		memcpy(node->bssid, addr, 6);	
	}
	if (tb[MSG_SSID]) {
		char *ssid = blobmsg_get_string(tb[MSG_SSID]);
		strcpy(node->ssid, ssid);
	}
	/* Local-Node */
	if (tb[MSG_BEACON_INTERVAL])
		ln->beacon_interval = blobmsg_get_u32(tb[MSG_BEACON_INTERVAL]);
}

static void
usteer_local_node_rrm_nr_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	static const struct blobmsg_policy policy = {
		"value", BLOBMSG_TYPE_ARRAY
	};
	struct usteer_local_node *ln;
	struct blob_attr *tb;

	ln = container_of(req, struct usteer_local_node, req);

	blobmsg_parse(&policy, 1, &tb, blob_data(msg), blob_len(msg));
	if (!tb)
		return;

	usteer_node_set_blob(&ln->node.rrm_nr, tb);
}

static void
usteer_local_node_req_cb(struct ubus_request *req, int ret)
{
	struct usteer_local_node *ln;

	ln = container_of(req, struct usteer_local_node, req);
	uloop_timeout_set(&ln->req_timer, 1);
}

static bool
usteer_add_rrm_data(struct usteer_local_node *ln, struct usteer_node *node)
{
	if (node == &ln->node)
		return false;

	if (!node->rrm_nr)
		return false;

	/* Remote node only adds same SSID. Required for local-node. */
	if (strcmp(ln->node.ssid, node->ssid) != 0)
		return false;

	blobmsg_add_field(&b, BLOBMSG_TYPE_ARRAY, "",
			  blobmsg_data(node->rrm_nr),
			  blobmsg_data_len(node->rrm_nr));

	return true;
}

static void
usteer_local_node_prepare_rrm_set(struct usteer_local_node *ln)
{
	struct usteer_node *node, *last_remote_neighbor = NULL;
	int i = 0;
	void *c;

	c = blobmsg_open_array(&b, "list");
	for_each_local_node(node) {
		if (i >= config.max_neighbor_reports)
			break;
		if (usteer_add_rrm_data(ln, node))
			i++;
	}

	while (i < config.max_neighbor_reports) {
		node = usteer_node_get_next_neighbor(&ln->node, last_remote_neighbor);
		if (!node) {
			/* No more nodes available */
			break;
		}

		last_remote_neighbor = node;
		if (usteer_add_rrm_data(ln, node))
			i++;
	}
		
	blobmsg_close_array(&b, c);
}

static void
usteer_local_node_state_next(struct uloop_timeout *timeout)
{
	struct usteer_local_node *ln;

	ln = container_of(timeout, struct usteer_local_node, req_timer);

	ln->req_state++;
	if (ln->req_state >= __REQ_MAX) {
		ln->req_state = REQ_IDLE;
		return;
	}

	blob_buf_init(&b, 0);
	switch (ln->req_state) {
	case REQ_CLIENTS:
		ubus_invoke_async(ubus_ctx, ln->obj_id, "get_clients", b.head, &ln->req);
		ln->req.data_cb = usteer_local_node_list_cb;
		break;
	case REQ_STATUS:
		ubus_invoke_async(ubus_ctx, ln->obj_id, "get_status", b.head, &ln->req);
		ln->req.data_cb = usteer_local_node_status_cb;
		break;
	case REQ_RRM_SET_LIST:
		usteer_local_node_prepare_rrm_set(ln);
		ubus_invoke_async(ubus_ctx, ln->obj_id, "rrm_nr_set", b.head, &ln->req);
		ln->req.data_cb = NULL;
		break;
	case REQ_RRM_GET_OWN:
		ubus_invoke_async(ubus_ctx, ln->obj_id, "rrm_nr_get_own", b.head, &ln->req);
		ln->req.data_cb = usteer_local_node_rrm_nr_cb;
		break;
	default:
		break;
	}
	ln->req.complete_cb = usteer_local_node_req_cb;
	ubus_complete_request_async(ubus_ctx, &ln->req);
}

static void
usteer_local_node_request_link_measurement(struct usteer_local_node *ln)
{
	unsigned int min_count = DIV_ROUND_UP(config.link_measurement_interval, config.local_sta_update);
	struct usteer_node *node;
	struct sta_info *si;

	node = &ln->node;

	if (ln->link_measurement_tries < min_count) {
		ln->link_measurement_tries++;
		return;
	}
	
	ln->link_measurement_tries = 0;

	if (!config.link_measurement_interval)
		return;

	list_for_each_entry(si, &node->sta_info, node_list) {
		if (si->connected != STA_CONNECTED)
			continue;

		usteer_ubus_trigger_link_measurement(si);
	}
}

static void
usteer_local_node_update(struct uloop_timeout *timeout)
{
	struct usteer_local_node *ln;
	struct usteer_node_handler *h;
	struct usteer_node *node;

	ln = container_of(timeout, struct usteer_local_node, update);
	node = &ln->node;

	list_for_each_entry(h, &node_handlers, list) {
		if (!h->update_node)
			continue;

		h->update_node(node);
	}

	usteer_local_node_state_reset(ln);
	uloop_timeout_set(&ln->req_timer, 1);
	usteer_local_node_kick(ln);
	usteer_band_steering_perform_steer(ln);
	usteer_local_node_request_link_measurement(ln);

	uloop_timeout_set(timeout, config.local_sta_update);
}

static void
usteer_local_node_process_bss_tm_queries(struct uloop_timeout *timeout)
{
	struct usteer_bss_tm_query *query, *tmp;
	struct usteer_local_node *ln;
	struct usteer_node *node;
	struct sta_info *si;
	struct sta *sta;
	uint8_t validity_period;

	ln = container_of(timeout, struct usteer_local_node, bss_tm_queries_timeout);
	node = &ln->node;

	validity_period = 10000 / usteer_local_node_get_beacon_interval(ln); /* ~ 10 seconds */

	list_for_each_entry_safe(query, tmp, &ln->bss_tm_queries, list) {
		sta = usteer_sta_get(query->sta_addr, false);
		if (!sta)
			continue;

		si = usteer_sta_info_get(sta, node, false);
		if (!si)
			continue;

		usteer_ubus_bss_transition_request(si, query->dialog_token, false, false, validity_period, NULL);
	}

	/* Free pending queries we can not handle */
	usteer_local_node_pending_bss_tm_free(ln);
}

static struct usteer_local_node *
usteer_get_node(struct ubus_context *ctx, const char *name)
{
	struct usteer_local_node *ln;
	struct usteer_node *node;
	char *str;

	ln = avl_find_element(&local_nodes, name, ln, node.avl);
	if (ln)
		return ln;

	ln = calloc_a(sizeof(*ln), &str, strlen(name) + 1);
	node = &ln->node;
	node->type = NODE_TYPE_LOCAL;
	node->created = current_time;
	node->avl.key = strcpy(str, name);
	ln->ev.remove_cb = usteer_handle_remove;
	ln->ev.cb = usteer_handle_event;
	ln->update.cb = usteer_local_node_update;
	ln->req_timer.cb = usteer_local_node_state_next;
	ubus_register_subscriber(ctx, &ln->ev);
	avl_insert(&local_nodes, &node->avl);
	kvlist_init(&ln->node_info, kvlist_blob_len);
	INIT_LIST_HEAD(&node->sta_info);
	INIT_LIST_HEAD(&node->measurements);

	ln->bss_tm_queries_timeout.cb = usteer_local_node_process_bss_tm_queries;
	INIT_LIST_HEAD(&ln->bss_tm_queries);
	return ln;
}

static void
usteer_node_run_update_script(struct usteer_node *node)
{
	struct usteer_local_node *ln = container_of(node, struct usteer_local_node, node);
	char *val;

	if (!node_up_script)
		return;

	val = alloca(strlen(node_up_script) + strlen(ln->iface) + 8);
	sprintf(val, "%s '%s'", node_up_script, ln->iface);
	if (system(val))
		MSG(INFO, "failed to execute %s\n", val);
}

static void
usteer_check_node_enabled(struct usteer_local_node *ln)
{
	bool ssid_disabled = config.ssid_list;
	struct blob_attr *cur;
	int rem;

	blobmsg_for_each_attr(cur, config.ssid_list, rem) {
		char *config_ssid = blobmsg_get_string(cur);
		char *ssid = ln->node.ssid;
		if (strcmp(config_ssid, ssid) != 0)
			continue;
		ssid_disabled = false;
		break;
	}

	if (ln->node.disabled == ssid_disabled)
		return;

	ln->node.disabled = ssid_disabled;

	if (ssid_disabled) {
		MSG(INFO, "Disconnecting from local node %s\n", usteer_node_name(&ln->node));
		usteer_local_node_state_reset(ln);
		usteer_sta_node_cleanup(&ln->node);
		usteer_measurement_report_node_cleanup(&ln->node);
		uloop_timeout_cancel(&ln->update);
		ubus_unsubscribe(ubus_ctx, &ln->ev, ln->obj_id);
		return;
	}

	MSG(INFO, "Connecting %s to local node %s\n", ln->node.ssid, usteer_node_name(&ln->node));
	ubus_subscribe(ubus_ctx, &ln->ev, ln->obj_id);
	uloop_timeout_set(&ln->update, 1);
	usteer_node_run_update_script(&ln->node);
}

static void
usteer_register_node(struct ubus_context *ctx, const char *name, uint32_t id)
{
	struct usteer_local_node *ln;
	struct usteer_node_handler *h;
	const char *iface;
	int offset = sizeof("hostapd.") - 1;

	iface = name + offset;
	if (strncmp(name, "hostapd.", iface - name) != 0)
		return;

	MSG(INFO, "Creating local node %s\n", name);
	ln = usteer_get_node(ctx, name);
	ln->obj_id = id;
	ln->iface = usteer_node_name(&ln->node) + offset;
	ln->ifindex = if_nametoindex(ln->iface);

	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "notify_response", 1);
	ubus_invoke(ctx, id, "notify_response", b.head, NULL, NULL, 1000);

	blob_buf_init(&b, 0);
	blobmsg_add_u8(&b, "neighbor_report", 1);
	blobmsg_add_u8(&b, "link_measurement", 1);
	blobmsg_add_u8(&b, "beacon_report", 1);
	blobmsg_add_u8(&b, "bss_transition", 1);
	ubus_invoke(ctx, id, "bss_mgmt_enable", b.head, NULL, NULL, 1000);

	list_for_each_entry(h, &node_handlers, list) {
		if (!h->init_node)
			continue;

		h->init_node(&ln->node);
	}

	ln->node.disabled = true;
	usteer_check_node_enabled(ln);
}

static void
usteer_event_handler(struct ubus_context *ctx, struct ubus_event_handler *ev,
		    const char *type, struct blob_attr *msg)
{
	static const struct blobmsg_policy policy[2] = {
		{ .name = "id", .type = BLOBMSG_TYPE_INT32 },
		{ .name = "path", .type = BLOBMSG_TYPE_STRING },
	};
	struct blob_attr *tb[2];
	const char *path;

	blobmsg_parse(policy, 2, tb, blob_data(msg), blob_len(msg));

	if (!tb[0] || !tb[1])
		return;

	path = blobmsg_data(tb[1]);
	usteer_register_node(ctx, path, blobmsg_get_u32(tb[0]));
}

static void
usteer_register_events(struct ubus_context *ctx)
{
	static struct ubus_event_handler handler = {
	    .cb = usteer_event_handler
	};

	ubus_register_event_handler(ctx, &handler, "ubus.object.add");
}

static void
node_list_cb(struct ubus_context *ctx, struct ubus_object_data *obj, void *priv)
{
	usteer_register_node(ctx, obj->path, obj->id);
}

int
usteer_local_node_get_beacon_interval(struct usteer_local_node *ln)
{
	/* Check if beacon-interval is not available (pre-21.02+) */
	if (ln->beacon_interval < 1)
		return 100;

	return ln->beacon_interval;
}

void config_set_node_up_script(struct blob_attr *data)
{
	const char *val;
	struct usteer_node *node;

	if (!data)
		return;

	val = blobmsg_get_string(data);
	if (node_up_script && !strcmp(val, node_up_script))
		return;

	free(node_up_script);

	if (!strlen(val)) {
		node_up_script = NULL;
		return;
	}

	node_up_script = strdup(val);

	for_each_local_node(node)
		usteer_node_run_update_script(node);
}

void config_get_node_up_script(struct blob_buf *buf)
{
	if (!node_up_script)
		return;

	blobmsg_add_string(buf, "node_up_script", node_up_script);
}

void config_set_ssid_list(struct blob_attr *data)
{
	struct usteer_local_node *ln;
	
	if (config.ssid_list)
		free(config.ssid_list);

	if (data && blobmsg_len(data))
		config.ssid_list = blob_memdup(data);
	else
		config.ssid_list = NULL;

	avl_for_each_element(&local_nodes, ln, node.avl)
		usteer_check_node_enabled(ln);
}

void config_get_ssid_list(struct blob_buf *buf)
{
	if (config.ssid_list)
		blobmsg_add_blob(buf, config.ssid_list);
}

void
usteer_local_nodes_init(struct ubus_context *ctx)
{
	usteer_register_events(ctx);
	ubus_lookup(ctx, "hostapd.*", node_list_cb, NULL);
}
