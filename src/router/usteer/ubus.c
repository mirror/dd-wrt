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

#include "usteer.h"
#include "node.h"
#include "event.h"

static struct blob_buf b;
static KVLIST(host_info, kvlist_blob_len);

static void *
blobmsg_open_table_mac(struct blob_buf *buf, uint8_t *addr)
{
	char str[20];
	sprintf(str, MAC_ADDR_FMT, MAC_ADDR_DATA(addr));
	return blobmsg_open_table(buf, str);
}

static int
usteer_ubus_get_clients(struct ubus_context *ctx, struct ubus_object *obj,
		       struct ubus_request_data *req, const char *method,
		       struct blob_attr *msg)
{
	struct sta_info *si;
	struct sta *sta;
	void *_s, *_cur_n;

	blob_buf_init(&b, 0);
	avl_for_each_element(&stations, sta, avl) {
		_s = blobmsg_open_table_mac(&b, sta->addr);
		list_for_each_entry(si, &sta->nodes, list) {
			_cur_n = blobmsg_open_table(&b, usteer_node_name(si->node));
			blobmsg_add_u8(&b, "connected", si->connected);
			blobmsg_add_u32(&b, "signal", si->signal);
			blobmsg_add_u32(&b, "noise", si->node->noise);
			blobmsg_add_u32(&b, "snr", usteer_signal_to_snr(si->node, si->signal));
			blobmsg_close_table(&b, _cur_n);
		}
		blobmsg_close_table(&b, _s);
	}
	ubus_send_reply(ctx, req, b.head);
	return 0;
}

static struct blobmsg_policy client_arg[] = {
	{ .name = "address", .type = BLOBMSG_TYPE_STRING, },
};

static void
usteer_ubus_add_stats(struct sta_info_stats *stats, const char *name)
{
	void *s;

	s = blobmsg_open_table(&b, name);
	blobmsg_add_u32(&b, "requests", stats->requests);
	blobmsg_add_u32(&b, "blocked_cur", stats->blocked_cur);
	blobmsg_add_u32(&b, "blocked_total", stats->blocked_total);
	blobmsg_close_table(&b, s);
}

static int
usteer_ubus_get_client_info(struct ubus_context *ctx, struct ubus_object *obj,
			   struct ubus_request_data *req, const char *method,
			   struct blob_attr *msg)
{
	struct sta_info *si;
	struct sta *sta;
	struct blob_attr *mac_str;
	uint8_t *mac;
	void *_n, *_cur_n, *_s;
	int i;

	blobmsg_parse(client_arg, 1, &mac_str, blob_data(msg), blob_len(msg));
	if (!mac_str)
		return UBUS_STATUS_INVALID_ARGUMENT;

	mac = (uint8_t *) ether_aton(blobmsg_data(mac_str));
	if (!mac)
		return UBUS_STATUS_INVALID_ARGUMENT;

	sta = usteer_sta_get(mac, false);
	if (!sta)
		return UBUS_STATUS_NOT_FOUND;

	blob_buf_init(&b, 0);
	blobmsg_add_u8(&b, "2ghz", sta->seen_2ghz);
	blobmsg_add_u8(&b, "5ghz", sta->seen_5ghz);
	_n = blobmsg_open_table(&b, "nodes");
	list_for_each_entry(si, &sta->nodes, list) {
		_cur_n = blobmsg_open_table(&b, usteer_node_name(si->node));
		blobmsg_add_u8(&b, "connected", si->connected);
		blobmsg_add_u32(&b, "signal", si->signal);
		blobmsg_add_u32(&b, "noise", si->node->noise);
		blobmsg_add_u32(&b, "snr", usteer_signal_to_snr(si->node, si->signal));
		_s = blobmsg_open_table(&b, "stats");
		for (i = 0; i < __EVENT_TYPE_MAX; i++)
			usteer_ubus_add_stats(&si->stats[EVENT_TYPE_PROBE], event_types[i]);
		blobmsg_close_table(&b, _s);
		blobmsg_close_table(&b, _cur_n);
	}
	blobmsg_close_table(&b, _n);

	ubus_send_reply(ctx, req, b.head);

	return 0;
}

enum cfg_type {
	CFG_BOOL,
	CFG_I32,
	CFG_U32,
	CFG_ARRAY_CB,
	CFG_STRING_CB,
};

struct cfg_item {
	enum cfg_type type;
	union {
		bool *BOOL;
		uint32_t *U32;
		int32_t *I32;
		struct {
			void (*set)(struct blob_attr *data);
			void (*get)(struct blob_buf *buf);
		} CB;
	} ptr;
};

#define __config_items \
	_cfg(BOOL, syslog), \
	_cfg(U32, debug_level), \
	_cfg(BOOL, ipv6), \
	_cfg(BOOL, local_mode), \
	_cfg(U32, sta_block_timeout), \
	_cfg(U32, local_sta_timeout), \
	_cfg(U32, local_sta_update), \
	_cfg(U32, max_neighbor_reports), \
	_cfg(U32, max_retry_band), \
	_cfg(U32, seen_policy_timeout), \
	_cfg(U32, measurement_report_timeout), \
	_cfg(U32, load_balancing_threshold), \
	_cfg(U32, band_steering_threshold), \
	_cfg(U32, remote_update_interval), \
	_cfg(U32, remote_node_timeout), \
	_cfg(BOOL, assoc_steering), \
	_cfg(I32, min_connect_snr), \
	_cfg(I32, min_snr), \
	_cfg(U32, min_snr_kick_delay), \
	_cfg(U32, steer_reject_timeout), \
	_cfg(U32, roam_process_timeout), \
	_cfg(I32, roam_scan_snr), \
	_cfg(U32, roam_scan_tries), \
	_cfg(U32, roam_scan_timeout), \
	_cfg(U32, roam_scan_interval), \
	_cfg(I32, roam_trigger_snr), \
	_cfg(U32, roam_trigger_interval), \
	_cfg(U32, roam_kick_delay), \
	_cfg(U32, signal_diff_threshold), \
	_cfg(U32, initial_connect_delay), \
	_cfg(I32, budget_5ghz), \
	_cfg(BOOL, prefer_5ghz), \
	_cfg(BOOL, prefer_he), \
	_cfg(BOOL, load_kick_enabled), \
	_cfg(U32, load_kick_threshold), \
	_cfg(U32, load_kick_delay), \
	_cfg(U32, load_kick_min_clients), \
	_cfg(U32, load_kick_reason_code), \
	_cfg(U32, band_steering_interval), \
	_cfg(I32, band_steering_min_snr), \
	_cfg(U32, link_measurement_interval), \
	_cfg(ARRAY_CB, interfaces), \
	_cfg(STRING_CB, node_up_script), \
	_cfg(ARRAY_CB, event_log_types), \
	_cfg(ARRAY_CB, ssid_list)

enum cfg_items {
#define _cfg(_type, _name) CFG_##_name
	__config_items,
#undef _cfg
	__CFG_MAX,
};

static const struct blobmsg_policy config_policy[__CFG_MAX] = {
#define _cfg_policy(_type, _name) [CFG_##_name] = { .name = #_name, .type = BLOBMSG_TYPE_ ## _type }
#define _cfg_policy_BOOL(_name) _cfg_policy(BOOL, _name)
#define _cfg_policy_U32(_name) _cfg_policy(INT32, _name)
#define _cfg_policy_I32(_name) _cfg_policy(INT32, _name)
#define _cfg_policy_ARRAY_CB(_name) _cfg_policy(ARRAY, _name)
#define _cfg_policy_STRING_CB(_name) _cfg_policy(STRING, _name)
#define _cfg(_type, _name) _cfg_policy_##_type(_name)
	__config_items,
#undef _cfg
};

static const struct cfg_item config_data[__CFG_MAX] = {
#define _cfg_data_BOOL(_name) .ptr.BOOL = &config._name
#define _cfg_data_U32(_name) .ptr.U32 = &config._name
#define _cfg_data_I32(_name) .ptr.I32 = &config._name
#define _cfg_data_ARRAY_CB(_name) .ptr.CB = { .set = config_set_##_name, .get = config_get_##_name }
#define _cfg_data_STRING_CB(_name) .ptr.CB = { .set = config_set_##_name, .get = config_get_##_name }
#define _cfg(_type, _name) [CFG_##_name] = { .type = CFG_##_type, _cfg_data_##_type(_name) }
	__config_items,
#undef _cfg
};

static int
usteer_ubus_get_config(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	int i;

	blob_buf_init(&b, 0);
	for (i = 0; i < __CFG_MAX; i++) {
		switch(config_data[i].type) {
		case CFG_BOOL:
			blobmsg_add_u8(&b, config_policy[i].name,
					*config_data[i].ptr.BOOL);
			break;
		case CFG_I32:
		case CFG_U32:
			blobmsg_add_u32(&b, config_policy[i].name,
					*config_data[i].ptr.U32);
			break;
		case CFG_ARRAY_CB:
		case CFG_STRING_CB:
			config_data[i].ptr.CB.get(&b);
			break;
		}
	}
	ubus_send_reply(ctx, req, b.head);
	return 0;
}

static int
usteer_ubus_set_config(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__CFG_MAX];
	int i;

	if (!strcmp(method, "set_config"))
		usteer_init_defaults();

	blobmsg_parse(config_policy, __CFG_MAX, tb, blob_data(msg), blob_len(msg));
	for (i = 0; i < __CFG_MAX; i++) {
		switch(config_data[i].type) {
		case CFG_BOOL:
			if (!tb[i])
				continue;

			*config_data[i].ptr.BOOL = blobmsg_get_u8(tb[i]);
			break;
		case CFG_I32:
		case CFG_U32:
			if (!tb[i])
				continue;

			*config_data[i].ptr.U32 = blobmsg_get_u32(tb[i]);
			break;
		case CFG_ARRAY_CB:
		case CFG_STRING_CB:
			config_data[i].ptr.CB.set(tb[i]);
			break;
		}
	}

	usteer_interface_init();

	return 0;
}

void usteer_dump_node(struct blob_buf *buf, struct usteer_node *node)
{
	void *c, *roam_events;

	c = blobmsg_open_table(buf, usteer_node_name(node));
	blobmsg_printf(buf, "bssid", MAC_ADDR_FMT, MAC_ADDR_DATA(node->bssid));
	blobmsg_add_string(buf, "ssid", node->ssid);
	blobmsg_add_u32(buf, "freq", node->freq);
	blobmsg_add_u32(buf, "n_assoc", node->n_assoc);
	blobmsg_add_u32(buf, "noise", node->noise);
	blobmsg_add_u32(buf, "load", node->load);
	blobmsg_add_u32(buf, "max_assoc", node->max_assoc);

	roam_events = blobmsg_open_table(buf, "roam_events");
	blobmsg_add_u32(buf, "source", node->roam_events.source);
	blobmsg_add_u32(buf, "target", node->roam_events.target);
	blobmsg_close_table(buf, roam_events);

	if (node->rrm_nr)
		blobmsg_add_field(buf, BLOBMSG_TYPE_ARRAY, "rrm_nr",
				  blobmsg_data(node->rrm_nr),
				  blobmsg_data_len(node->rrm_nr));
	if (node->node_info)
		blobmsg_add_field(buf, BLOBMSG_TYPE_TABLE, "node_info",
				  blob_data(node->node_info),
				  blob_len(node->node_info));

	blobmsg_close_table(buf, c);
}

void usteer_dump_host(struct blob_buf *buf, struct usteer_remote_host *host)
{
	void *c;

	c = blobmsg_open_table(buf, host->addr);
	blobmsg_add_u32(buf, "id", (uint32_t)(uintptr_t)host->avl.key);
	if (host->host_info)
		blobmsg_add_field(buf, BLOBMSG_TYPE_TABLE, "host_info",
				  blobmsg_data(host->host_info),
				  blobmsg_len(host->host_info));
	blobmsg_close_table(buf, c);
}

static int
usteer_ubus_local_info(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct usteer_node *node;

	blob_buf_init(&b, 0);

	for_each_local_node(node)
		usteer_dump_node(&b, node);

	ubus_send_reply(ctx, req, b.head);

	return 0;
}

static int
usteer_ubus_remote_hosts(struct ubus_context *ctx, struct ubus_object *obj,
			 struct ubus_request_data *req, const char *method,
			 struct blob_attr *msg)
{
	struct usteer_remote_host *host;

	blob_buf_init(&b, 0);

	avl_for_each_element(&remote_hosts, host, avl)
		usteer_dump_host(&b, host);

	ubus_send_reply(ctx, req, b.head);

	return 0;
}

static int
usteer_ubus_remote_info(struct ubus_context *ctx, struct ubus_object *obj,
		       struct ubus_request_data *req, const char *method,
		       struct blob_attr *msg)
{
	struct usteer_remote_node *rn;

	blob_buf_init(&b, 0);

	for_each_remote_node(rn)
		usteer_dump_node(&b, &rn->node);

	ubus_send_reply(ctx, req, b.head);

	return 0;
}

static const char *usteer_get_roam_sm_name(enum roam_trigger_state state)
{
	switch (state) {
		case ROAM_TRIGGER_IDLE:
			return "ROAM_TRIGGER_IDLE";
		case ROAM_TRIGGER_SCAN:
			return "ROAM_TRIGGER_SCAN";
		case ROAM_TRIGGER_SCAN_DONE:
			return "ROAM_TRIGGER_SCAN_DONE";
	}
	return "N/A";
}

static int
usteer_ubus_get_connected_clients(struct ubus_context *ctx, struct ubus_object *obj,
				  struct ubus_request_data *req, const char *method,
				  struct blob_attr *msg)
{
	struct usteer_measurement_report *mr;
	struct usteer_node *node;
	struct sta_info *si;
	void *n, *s, *t, *a;

	blob_buf_init(&b, 0);

	for_each_local_node(node) {
		n = blobmsg_open_table(&b, usteer_node_name(node));

		list_for_each_entry(si, &node->sta_info, node_list) {
			if (si->connected != STA_CONNECTED)
				continue;

			s = blobmsg_open_table_mac(&b, si->sta->addr);
			blobmsg_add_u32(&b, "signal", si->signal);
			blobmsg_add_u32(&b, "noise", si->node->noise);
			blobmsg_add_u32(&b, "snr", (int32_t)usteer_signal_to_snr(si->node, si->signal));
			blobmsg_add_u64(&b, "created", current_time - si->created);
			blobmsg_add_u64(&b, "connected", current_time - si->connected_since);

			t = blobmsg_open_table(&b, "snr-kick");
			blobmsg_add_u32(&b, "seen-below", si->below_min_snr);
			blobmsg_close_table(&b, t);

			t = blobmsg_open_table(&b, "roam-state-machine");
			blobmsg_add_string(&b, "state",usteer_get_roam_sm_name(si->roam_state));
			blobmsg_add_u32(&b, "tries", si->roam_tries);
			blobmsg_add_u64(&b, "event", si->roam_event ? current_time - si->roam_event : 0);
			blobmsg_add_u32(&b, "kick-count", si->kick_count);
			blobmsg_add_u64(&b, "last-kick", si->roam_kick ? current_time - si->roam_kick : 0);
			blobmsg_add_u64(&b, "scan_start", si->roam_scan_start ? current_time - si->roam_scan_start : 0);
			blobmsg_add_u64(&b, "scan_timeout_start", si->roam_scan_timeout_start ? current_time - si->roam_scan_timeout_start : 0);
			blobmsg_close_table(&b, t);

			t = blobmsg_open_table(&b, "bss-transition-response");
			blobmsg_add_u32(&b, "status-code", si->bss_transition_response.status_code);
			blobmsg_add_u64(&b, "age", si->bss_transition_response.timestamp ? current_time - si->bss_transition_response.timestamp : 0);
			blobmsg_close_table(&b, t);

			/* Beacon measurement modes */
			a = blobmsg_open_array(&b, "beacon-measurement-modes");
			if (usteer_sta_supports_beacon_measurement_mode(si, BEACON_MEASUREMENT_PASSIVE))
				blobmsg_add_string(&b, "", "PASSIVE");
			if (usteer_sta_supports_beacon_measurement_mode(si, BEACON_MEASUREMENT_ACTIVE))
				blobmsg_add_string(&b, "", "ACTIVE");
			if (usteer_sta_supports_beacon_measurement_mode(si, BEACON_MEASUREMENT_TABLE))
				blobmsg_add_string(&b, "", "TABLE");
			blobmsg_close_array(&b, a);

			/* Link-Measurement support */
			blobmsg_add_u8(&b, "link-measurement", usteer_sta_supports_link_measurement(si));

			/* BSS-Transition support */
			blobmsg_add_u8(&b, "bss-transition-management", si->bss_transition);

			/* MBO support */
			blobmsg_add_u8(&b, "multi-band-operation", si->mbo);

			/* Measurements */
			a = blobmsg_open_array(&b, "measurements");
			list_for_each_entry(mr, &si->sta->measurements, sta_list) {
				t = blobmsg_open_table(&b, "");
				blobmsg_add_string(&b, "node", usteer_node_name(mr->node));
				blobmsg_add_u32(&b, "rcpi", mr->rcpi);
				blobmsg_add_u32(&b, "rsni", mr->rsni);
				blobmsg_add_u32(&b, "rssi", usteer_measurement_get_rssi(mr));
				blobmsg_add_u64(&b, "age", current_time - mr->timestamp);
				blobmsg_close_table(&b, t);
			}
			blobmsg_close_array(&b, a);

			blobmsg_close_table(&b, s);
		}

		blobmsg_close_table(&b, n);
	}

	ubus_send_reply(ctx, req, b.head);

	return 0;
}

enum {
	NODE_DATA_NODE,
	NODE_DATA_VALUES,
	__NODE_DATA_MAX,
};

static const struct blobmsg_policy set_node_data_policy[] = {
	[NODE_DATA_NODE] = { "node", BLOBMSG_TYPE_STRING },
	[NODE_DATA_VALUES] = { "data", BLOBMSG_TYPE_TABLE },
};

static const struct blobmsg_policy del_node_data_policy[] = {
	[NODE_DATA_NODE] = { "node", BLOBMSG_TYPE_STRING },
	[NODE_DATA_VALUES] = { "names", BLOBMSG_TYPE_ARRAY },
};

static void
usteer_update_kvlist_data(struct kvlist *kv, struct blob_attr *data,
			  bool delete)
{
	struct blob_attr *cur;
	int rem;

	blobmsg_for_each_attr(cur, data, rem) {
		if (delete)
			kvlist_delete(kv, blobmsg_get_string(cur));
		else
			kvlist_set(kv, blobmsg_name(cur), cur);
	}
}

static void
usteer_update_kvlist_blob(struct blob_attr **dest, struct kvlist *kv)
{
	struct blob_attr *val;
	const char *name;

	blob_buf_init(&b, 0);
	kvlist_for_each(kv, name, val)
		blobmsg_add_field(&b, blobmsg_type(val), name,
				  blobmsg_data(val), blobmsg_len(val));

	val = b.head;
	if (!blobmsg_len(val))
		val = NULL;

	usteer_node_set_blob(dest, val);
}

static int
usteer_ubus_update_node_data(struct ubus_context *ctx, struct ubus_object *obj,
			     struct ubus_request_data *req, const char *method,
			     struct blob_attr *msg)
{
	const struct blobmsg_policy *policy;
	struct blob_attr *tb[__NODE_DATA_MAX];
	struct usteer_local_node *ln;
	struct blob_attr *val;
	const char *name;
	bool delete;

	delete = !strncmp(method, "del", 3);
	policy = delete ? del_node_data_policy : set_node_data_policy;

	blobmsg_parse(policy, __NODE_DATA_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[NODE_DATA_NODE] || !tb[NODE_DATA_VALUES])
		return UBUS_STATUS_INVALID_ARGUMENT;

	name = blobmsg_get_string(tb[NODE_DATA_NODE]);
	val = tb[NODE_DATA_VALUES];
	if (delete && blobmsg_check_array(val, BLOBMSG_TYPE_STRING) < 0)
		return UBUS_STATUS_INVALID_ARGUMENT;

	if (strcmp(name, "*") != 0) {
		ln = avl_find_element(&local_nodes, name, ln, node.avl);
		if (!ln)
			return UBUS_STATUS_NOT_FOUND;

		usteer_update_kvlist_data(&ln->node_info, val, delete);
		usteer_update_kvlist_blob(&ln->node.node_info, &ln->node_info);

		return 0;
	}

	usteer_update_kvlist_data(&host_info, val, delete);
	usteer_update_kvlist_blob(&host_info_blob, &host_info);

	return 0;
}

static const struct ubus_method usteer_methods[] = {
	UBUS_METHOD_NOARG("local_info", usteer_ubus_local_info),
	UBUS_METHOD_NOARG("remote_hosts", usteer_ubus_remote_hosts),
	UBUS_METHOD_NOARG("remote_info", usteer_ubus_remote_info),
	UBUS_METHOD_NOARG("connected_clients", usteer_ubus_get_connected_clients),
	UBUS_METHOD_NOARG("get_clients", usteer_ubus_get_clients),
	UBUS_METHOD("get_client_info", usteer_ubus_get_client_info, client_arg),
	UBUS_METHOD_NOARG("get_config", usteer_ubus_get_config),
	UBUS_METHOD("set_config", usteer_ubus_set_config, config_policy),
	UBUS_METHOD("update_config", usteer_ubus_set_config, config_policy),
	UBUS_METHOD("set_node_data", usteer_ubus_update_node_data, set_node_data_policy),
	UBUS_METHOD("delete_node_data", usteer_ubus_update_node_data, del_node_data_policy),
};

static struct ubus_object_type usteer_obj_type =
	UBUS_OBJECT_TYPE("usteer", usteer_methods);

struct ubus_object usteer_obj = {
	.name = "usteer",
	.type = &usteer_obj_type,
	.methods = usteer_methods,
	.n_methods = ARRAY_SIZE(usteer_methods),
};

static bool
usteer_add_nr_entry(struct usteer_node *ln, struct usteer_node *node)
{
	struct blobmsg_policy policy[3] = {
		{ .type = BLOBMSG_TYPE_STRING },
		{ .type = BLOBMSG_TYPE_STRING },
		{ .type = BLOBMSG_TYPE_STRING },
	};
	struct blob_attr *tb[3];

	if (!node->rrm_nr)
		return false;

	if (strcmp(ln->ssid, node->ssid) != 0)
		return false;

	if (!usteer_policy_node_below_max_assoc(node))
		return false;

	blobmsg_parse_array(policy, ARRAY_SIZE(tb), tb,
			    blobmsg_data(node->rrm_nr),
			    blobmsg_data_len(node->rrm_nr));
	if (!tb[2])
		return false;

	blobmsg_add_field(&b, BLOBMSG_TYPE_STRING, "",
			  blobmsg_data(tb[2]),
			  blobmsg_data_len(tb[2]));
	
	return true;
}

static void
usteer_ubus_disassoc_add_neighbor(struct sta_info *si, struct usteer_node *node)
{
	void *c;

	c = blobmsg_open_array(&b, "neighbors");
	usteer_add_nr_entry(si->node, node);
	blobmsg_close_array(&b, c);
}

static void
usteer_ubus_disassoc_add_neighbors(struct sta_info *si)
{
	struct usteer_node *node, *last_remote_neighbor = NULL;
	int i = 0;
	void *c;

	c = blobmsg_open_array(&b, "neighbors");
	for_each_local_node(node) {
		if (i >= config.max_neighbor_reports)
			break;
		if (si->node == node)
			continue;
		if (usteer_add_nr_entry(si->node, node))
			i++;
	}

	while (i < config.max_neighbor_reports) {
		node = usteer_node_get_next_neighbor(si->node, last_remote_neighbor);
		if (!node) {
			/* No more nodes available */
			break;
		}

		last_remote_neighbor = node;
		if (usteer_add_nr_entry(si->node, node))
			i++;
	}
	blobmsg_close_array(&b, c);
}

int usteer_ubus_bss_transition_request(struct sta_info *si,
				       uint8_t dialog_token,
				       bool disassoc_imminent,
				       bool abridged,
				       uint8_t validity_period,
				       struct usteer_node *target_node)
{
	struct usteer_local_node *ln = container_of(si->node, struct usteer_local_node, node);

	blob_buf_init(&b, 0);
	blobmsg_printf(&b, "addr", MAC_ADDR_FMT, MAC_ADDR_DATA(si->sta->addr));
	blobmsg_add_u32(&b, "dialog_token", dialog_token);
	blobmsg_add_u8(&b, "disassociation_imminent", disassoc_imminent);
	blobmsg_add_u8(&b, "abridged", abridged);
	blobmsg_add_u32(&b, "validity_period", validity_period);
	if (!target_node) {
		usteer_ubus_disassoc_add_neighbors(si);
	} else {
		usteer_ubus_disassoc_add_neighbor(si, target_node);
	}
	return ubus_invoke(ubus_ctx, ln->obj_id, "bss_transition_request", b.head, NULL, 0, 100);
}

int usteer_ubus_band_steering_request(struct sta_info *si)
{
	struct usteer_local_node *ln = container_of(si->node, struct usteer_local_node, node);
	struct usteer_node *node;
	void *c;

	blob_buf_init(&b, 0);
	blobmsg_printf(&b, "addr", MAC_ADDR_FMT, MAC_ADDR_DATA(si->sta->addr));
	blobmsg_add_u32(&b, "dialog_token", 0);
	blobmsg_add_u8(&b, "disassociation_imminent", false);
	blobmsg_add_u8(&b, "abridged", false);
	blobmsg_add_u32(&b, "validity_period", 100);

	c = blobmsg_open_array(&b, "neighbors");
	for_each_local_node(node) {
		if (!usteer_band_steering_is_target(ln, node))
			continue;
	
		usteer_add_nr_entry(si->node, node);
	}
	blobmsg_close_array(&b, c);

	return ubus_invoke(ubus_ctx, ln->obj_id, "bss_transition_request", b.head, NULL, 0, 100);
}

int usteer_ubus_trigger_link_measurement(struct sta_info *si)
{
	struct usteer_local_node *ln = container_of(si->node, struct usteer_local_node, node);

	if (!usteer_sta_supports_link_measurement(si))
		return 0;

	blob_buf_init(&b, 0);
	blobmsg_printf(&b, "addr", MAC_ADDR_FMT, MAC_ADDR_DATA(si->sta->addr));
	blobmsg_add_u32(&b, "tx-power-used", 5);
	blobmsg_add_u32(&b, "tx-power-max", 10);
	return ubus_invoke(ubus_ctx, ln->obj_id, "link_measurement_req", b.head, NULL, 0, 100);
}

int usteer_ubus_trigger_client_scan(struct sta_info *si)
{
	struct usteer_local_node *ln = container_of(si->node, struct usteer_local_node, node);

	if (!usteer_sta_supports_beacon_measurement_mode(si, BEACON_MEASUREMENT_ACTIVE)) {
		MSG(DEBUG, "STA does not support beacon measurement sta=" MAC_ADDR_FMT "\n", MAC_ADDR_DATA(si->sta->addr));
		return 0;
	}

	si->scan_band = !si->scan_band;

	blob_buf_init(&b, 0);
	blobmsg_printf(&b, "addr", MAC_ADDR_FMT, MAC_ADDR_DATA(si->sta->addr));
	blobmsg_add_string(&b, "ssid", si->node->ssid);
	blobmsg_add_u32(&b, "mode", BEACON_MEASUREMENT_ACTIVE);
	blobmsg_add_u32(&b, "duration", config.roam_scan_interval / 100);
	blobmsg_add_u32(&b, "channel", 0);
	blobmsg_add_u32(&b, "op_class", si->scan_band ? 1 : 12);
	return ubus_invoke(ubus_ctx, ln->obj_id, "rrm_beacon_req", b.head, NULL, 0, 100);
}

void usteer_ubus_kick_client(struct sta_info *si)
{
	struct usteer_local_node *ln = container_of(si->node, struct usteer_local_node, node);

	blob_buf_init(&b, 0);
	blobmsg_printf(&b, "addr", MAC_ADDR_FMT, MAC_ADDR_DATA(si->sta->addr));
	blobmsg_add_u32(&b, "reason", config.load_kick_reason_code);
	blobmsg_add_u8(&b, "deauth", 1);
	ubus_invoke(ubus_ctx, ln->obj_id, "del_client", b.head, NULL, 0, 100);
	usteer_sta_disconnected(si);
	si->kick_count++;
	si->roam_kick = current_time;
}

void usteer_ubus_init(struct ubus_context *ctx)
{
	ubus_add_object(ctx, &usteer_obj);
}
