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
#include "usteer.h"
#include "event.h"

#define UEV_LOG_MAXLEN	256

static struct blob_buf b;
static const char * const uev_name[] = {
	[UEV_PROBE_REQ_ACCEPT] = "probe_req_accept",
	[UEV_PROBE_REQ_DENY] = "probe_req_deny",
	[UEV_AUTH_REQ_ACCEPT] = "auth_req_accept",
	[UEV_AUTH_REQ_DENY] = "auth_req_deny",
	[UEV_ASSOC_REQ_ACCEPT] = "assoc_req_accept",
	[UEV_ASSOC_REQ_DENY] = "assoc_req_deny",
	[UEV_LOAD_KICK_TRIGGER] = "load_kick_trigger",
	[UEV_LOAD_KICK_RESET] = "load_kick_reset",
	[UEV_LOAD_KICK_MIN_CLIENTS] = "load_kick_min_clients",
	[UEV_LOAD_KICK_NO_CLIENT] = "load_kick_no_client",
	[UEV_LOAD_KICK_CLIENT] = "load_kick_client",
	[UEV_SIGNAL_KICK] = "signal_kick",

};
static const char * const uev_reason[] = {
	[UEV_REASON_NONE] = "none",
	[UEV_REASON_RETRY_EXCEEDED] = "retry_exceeded",
	[UEV_REASON_LOW_SIGNAL] = "low_signal",
	[UEV_REASON_CONNECT_DELAY] = "connect_delay",
	[UEV_REASON_BETTER_CANDIDATE] = "better_candidate",
};

static const char * const uev_select_reason[] = {
	[UEV_SELECT_REASON_NUM_ASSOC] = "n_assoc",
	[UEV_SELECT_REASON_SIGNAL] = "signal",
	[UEV_SELECT_REASON_LOAD] = "load",
};

static void
usteer_event_add_node_status(struct usteer_node *node)
{
	blobmsg_add_u32(&b, "load", node->load);
	blobmsg_add_u32(&b, "assoc", node->n_assoc);
}

static void
usteer_event_send_ubus(struct uevent *ev)
{
	void *c;
	int i;

	if (!usteer_obj.has_subscribers)
		return;

	blob_buf_init(&b, 0);

	if (ev->node_local)
		blobmsg_add_string(&b, "node", usteer_node_name(ev->node_local));

	if (ev->sta)
		blobmsg_printf(&b, "sta", MAC_ADDR_FMT, MAC_ADDR_DATA(ev->sta->addr));

	if (ev->si_cur) {
		blobmsg_add_u32(&b, "signal", (int32_t)ev->si_cur->signal);
		blobmsg_add_u32(&b, "noise", (int32_t)ev->si_cur->node->noise);
		blobmsg_add_u32(&b, "snr", (int32_t)usteer_signal_to_snr(ev->si_cur->node, ev->si_cur->signal));
	}
	if (ev->reason)
		blobmsg_add_string(&b, "reason", uev_reason[ev->reason]);

	if (ev->threshold.ref) {
		c = blobmsg_open_array(&b, "threshold");
		blobmsg_add_u32(&b, NULL, ev->threshold.cur);
		blobmsg_add_u32(&b, NULL, ev->threshold.ref);
		blobmsg_close_array(&b, c);
	}

	if (ev->select_reasons) {
		c = blobmsg_open_array(&b, "select_reason");
		for (i = 0; i < ARRAY_SIZE(uev_select_reason); i++) {
			if (!(ev->select_reasons & (1 << i)) ||
				!uev_select_reason[i])
				continue;

			blobmsg_add_string(&b, NULL, uev_select_reason[i]);
		}
		blobmsg_close_array(&b, c);
	}

	if (ev->node_cur) {
		c = blobmsg_open_table(&b, "local");
		usteer_event_add_node_status(ev->node_cur);
		blobmsg_close_table(&b, c);
	}

	if (ev->node_other) {
		c = blobmsg_open_table(&b, "remote");
		blobmsg_add_string(&b, "name", usteer_node_name(ev->node_other));
		if (ev->si_other) {
			blobmsg_add_u32(&b, "signal", (int32_t)ev->si_other->signal);
			blobmsg_add_u32(&b, "noise", (int32_t)ev->si_cur->node->noise);
			blobmsg_add_u32(&b, "snr", (int32_t)usteer_signal_to_snr(ev->si_other->node, ev->si_other->signal));
		}
		usteer_event_add_node_status(ev->node_other);
		blobmsg_close_table(&b, c);
	}

	if (ev->count)
		blobmsg_add_u32(&b, "count", ev->count);

	ubus_notify(ubus_ctx, &usteer_obj, uev_name[ev->type], b.head, -1);
}

static int
usteer_event_log_node(char *buf, int len, const char *prefix, struct usteer_node *node)
{
	char *cur = buf;
	char *end = buf + len;

	cur += snprintf(cur, end - cur, " %sassoc=%d %sload=%d",
			prefix, node->n_assoc,
			prefix, node->load);

	return cur - buf;
}

static void
usteer_event_log(struct uevent *ev)
{
	char *str, *cur, *end;
	int i;

	if (!(config.event_log_mask & (1 << ev->type)))
		return;

	blob_buf_init(&b, 0);
	cur = str = blobmsg_alloc_string_buffer(&b, NULL, UEV_LOG_MAXLEN);
	end = str + UEV_LOG_MAXLEN;
	cur += snprintf(cur, end - cur, "usteer event=%s", uev_name[ev->type]);
	if (ev->node_local)
		cur += snprintf(cur, end - cur, " node=%s", usteer_node_name(ev->node_local));
	if (ev->sta)
		cur += snprintf(cur, end - cur, " sta=" MAC_ADDR_FMT, MAC_ADDR_DATA(ev->sta->addr));
	if (ev->reason)
		cur += snprintf(cur, end - cur, " reason=%s", uev_reason[ev->reason]);
	if (ev->si_cur)
		cur += snprintf(cur, end - cur, " signal=%d", ev->si_cur->signal);
	if (ev->si_cur)
		cur += snprintf(cur, end - cur, " snr=%d", usteer_signal_to_snr(ev->si_cur->node, ev->si_cur->signal));
	if (ev->threshold.ref)
		cur += snprintf(cur, end - cur, " thr=%d/%d", ev->threshold.cur, ev->threshold.ref);
	if (ev->count)
		cur += snprintf(cur, end - cur, " count=%d", ev->count);
	if (ev->node_cur)
		cur += usteer_event_log_node(cur, end - cur, "", ev->node_cur);
	if (ev->select_reasons) {
		bool first = true;

		cur += snprintf(cur, end - cur, " select_reason");
		for (i = 0; i < ARRAY_SIZE(uev_select_reason); i++) {
			if (!(ev->select_reasons & (1 << i)) ||
				!uev_select_reason[i])
				continue;

			cur += snprintf(cur, end - cur, "%c%s", first ? '=' : ',',
						    uev_select_reason[i]);
			first = false;
		}
	}
	if (ev->node_other) {
		cur += snprintf(cur, end - cur, " remote=%s", usteer_node_name(ev->node_other));
		if (ev->si_other)
			cur += snprintf(cur, end - cur, " remote_signal=%d",
					ev->si_other->signal);
		cur += usteer_event_log_node(cur, end - cur, "remote_", ev->node_other);
	}

	log_msg(str);
}

void usteer_event(struct uevent *ev)
{
	if (ev->type >= ARRAY_SIZE(uev_name) || !uev_name[ev->type])
		return;

	if (ev->reason >= ARRAY_SIZE(uev_reason) || !uev_reason[ev->reason])
		return;

	if (ev->si_cur) {
		if (!ev->node_local)
			ev->node_local = ev->si_cur->node;
		if (!ev->sta)
			ev->sta = ev->si_cur->sta;
	}

	if (!ev->node_local && ev->node_cur)
		ev->node_local = ev->node_cur;

	if (ev->si_other && ev->node_cur && !ev->node_other)
		ev->node_other = ev->si_other->node;

	usteer_event_send_ubus(ev);
	usteer_event_log(ev);
}

void config_set_event_log_types(struct blob_attr *attr)
{
	struct blob_attr *cur;
	int i, rem;

	config.event_log_mask = 0;
	if (!attr) {
		static const uint32_t default_log[] = {
			[MSG_INFO] =
				(1 << UEV_LOAD_KICK_CLIENT) |
				(1 << UEV_SIGNAL_KICK) |
				(1 << UEV_AUTH_REQ_DENY) |
				(1 << UEV_ASSOC_REQ_DENY),
			[MSG_VERBOSE] =
				(1 << UEV_PROBE_REQ_DENY),
			[MSG_DEBUG] =
				(1 << UEV_AUTH_REQ_ACCEPT) |
				(1 << UEV_ASSOC_REQ_ACCEPT) |
				(1 << UEV_LOAD_KICK_TRIGGER) |
				(1 << UEV_LOAD_KICK_RESET) |
				(1 << UEV_LOAD_KICK_MIN_CLIENTS) |
				(1 << UEV_LOAD_KICK_NO_CLIENT),
		};

		if (config.debug_level >= MSG_DEBUG_ALL) {
			config.event_log_mask = ~0;
			return;
		}

		for (i = 0; i < ARRAY_SIZE(default_log) && i <= config.debug_level; i++)
			config.event_log_mask |= default_log[i];

		return;
	}

	if (blobmsg_check_array(attr, BLOBMSG_TYPE_STRING) < 0)
		return;

	blobmsg_for_each_attr(cur, attr, rem) {
		const char *name = blobmsg_get_string(cur);

		for (i = 0; i < ARRAY_SIZE(uev_name); i++) {
			if (!uev_name[i] || strcmp(uev_name[i], name) != 0)
				continue;

			config.event_log_mask |= (1 << i);
			break;
		}
	}
}

void config_get_event_log_types(struct blob_buf *buf)
{
	uint32_t mask = config.event_log_mask;
	void *c;
	int i;

	c = blobmsg_open_array(buf, "event_log_types");
	for (i = 0; mask && i < ARRAY_SIZE(uev_name); i++) {
		bool cur = mask & 1;

		mask >>= 1;
		if (!cur)
			continue;

		blobmsg_add_string(buf, NULL, uev_name[i]);
	}
	blobmsg_close_array(buf, c);
}
