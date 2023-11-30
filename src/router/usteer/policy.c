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
#include "node.h"
#include "event.h"

static bool
below_assoc_threshold(struct usteer_node *node_cur, struct usteer_node *node_new)
{
	int n_assoc_cur = node_cur->n_assoc;
	int n_assoc_new = node_new->n_assoc;
	bool ref_5g = node_cur->freq > 4000;
	bool node_5g = node_new->freq > 4000;

	if (!config.load_balancing_threshold)
		return false;

	if (ref_5g && !node_5g)
		n_assoc_new += config.band_steering_threshold;
	else if (!ref_5g && node_5g)
		n_assoc_cur += config.band_steering_threshold;

	n_assoc_new += config.load_balancing_threshold;

	return n_assoc_new <= n_assoc_cur;
}

static bool
better_signal_strength(int signal_cur, int signal_new)
{
	const bool is_better = signal_new - signal_cur
				> (int) config.signal_diff_threshold;

	if (!config.signal_diff_threshold)
		return false;

	return is_better;
}

static bool
below_load_threshold(struct usteer_node *node)
{
	return node->n_assoc >= config.load_kick_min_clients &&
	       node->load > config.load_kick_threshold;
}

static bool
has_better_load(struct usteer_node *node_cur, struct usteer_node *node_new)
{
	return !below_load_threshold(node_cur) && below_load_threshold(node_new);
}

bool
usteer_policy_node_below_max_assoc(struct usteer_node *node)
{
	return !node->max_assoc || node->n_assoc < node->max_assoc;
}

static bool
over_min_signal(struct usteer_node *node, int signal)
{
	if (config.min_snr && signal < usteer_snr_to_signal(node, config.min_snr))
		return false;

	if (config.roam_trigger_snr && signal < usteer_snr_to_signal(node, config.roam_trigger_snr))
		return false;
	
	return true;
}

static uint32_t
is_better_candidate(struct sta_info *si_cur, struct sta_info *si_new)
{
	struct usteer_node *current_node = si_cur->node;
	struct usteer_node *new_node = si_new->node;
	int current_signal = si_cur->signal;
	int new_signal = si_new->signal;
	uint32_t reasons = 0;

	if (!usteer_policy_node_below_max_assoc(new_node))
		return 0;

	if (!over_min_signal(new_node, new_signal))
		return 0;

	if (below_assoc_threshold(current_node, new_node) &&
	    !below_assoc_threshold(new_node, current_node))
		reasons |= (1 << UEV_SELECT_REASON_NUM_ASSOC);

	if (better_signal_strength(current_signal, new_signal))
		reasons |= (1 << UEV_SELECT_REASON_SIGNAL);

	if (has_better_load(current_node, new_node) &&
		!has_better_load(current_node, new_node))
		reasons |= (1 << UEV_SELECT_REASON_LOAD);

	return reasons;
}

static struct sta_info *
find_better_candidate(struct sta_info *si_ref, struct uevent *ev, uint32_t required_criteria, uint64_t max_age)
{
	struct sta_info *si, *candidate = NULL;
	struct sta *sta = si_ref->sta;
	uint32_t reasons;

	list_for_each_entry(si, &sta->nodes, list) {
		if (si == si_ref)
			continue;

		if (current_time - si->seen > config.seen_policy_timeout)
			continue;

		if (strcmp(si->node->ssid, si_ref->node->ssid) != 0)
			continue;

		if (max_age && max_age < current_time - si->seen)
			continue;

		reasons = is_better_candidate(si_ref, si);
		if (!reasons)
			continue;

		if (!(reasons & required_criteria))
			continue;

		if (ev) {
			ev->si_other = si;
			ev->select_reasons = reasons;
		}

		if (!candidate || si->signal > candidate->signal)
			candidate = si;
	}

	return candidate;
}

int
usteer_snr_to_signal(struct usteer_node *node, int snr)
{
	int noise = -95;

	if (snr < 0)
		return snr;

	if (node->noise)
		noise = node->noise;

	return noise + snr;
}

bool
usteer_check_request(struct sta_info *si, enum usteer_event_type type)
{
	struct uevent ev = {
		.si_cur = si,
	};
	int min_signal;
	bool ret = true;

	if (type == EVENT_TYPE_PROBE && !config.probe_steering)
		goto out;

	if (type == EVENT_TYPE_AUTH)
		goto out;

	if (type == EVENT_TYPE_ASSOC) {
		/* Check if assoc request has lower signal than min_signal.
		 * If this is the case, block assoc even when assoc steering is enabled.
		 *
		 * Otherwise, the client potentially ends up in a assoc - kick loop.
		 */
		if (config.min_snr && si->signal < usteer_snr_to_signal(si->node, config.min_snr)) {
			ev.reason = UEV_REASON_LOW_SIGNAL;
			ev.threshold.cur = si->signal;
			ev.threshold.ref = usteer_snr_to_signal(si->node, config.min_snr);
			ret = false;
			goto out;
		} else if (!config.assoc_steering) {
			goto out;
		}
	}

	min_signal = usteer_snr_to_signal(si->node, config.min_connect_snr);
	if (si->signal < min_signal) {
		ev.reason = UEV_REASON_LOW_SIGNAL;
		ev.threshold.cur = si->signal;
		ev.threshold.ref = min_signal;
		ret = false;
		goto out;
	}

	if (current_time - si->created < config.initial_connect_delay) {
		ev.reason = UEV_REASON_CONNECT_DELAY;
		ev.threshold.cur = current_time - si->created;
		ev.threshold.ref = config.initial_connect_delay;
		ret = false;
		goto out;
	}

	if (!find_better_candidate(si, &ev, UEV_SELECT_REASON_ALL, 0))
		goto out;

	ev.reason = UEV_REASON_BETTER_CANDIDATE;
	ev.node_cur = si->node;
	ret = false;

out:
	switch (type) {
	case EVENT_TYPE_PROBE:
		ev.type = ret ? UEV_PROBE_REQ_ACCEPT : UEV_PROBE_REQ_DENY;
		break;
	case EVENT_TYPE_ASSOC:
		ev.type = ret ? UEV_ASSOC_REQ_ACCEPT : UEV_ASSOC_REQ_DENY;
		break;
	case EVENT_TYPE_AUTH:
		ev.type = ret ? UEV_AUTH_REQ_ACCEPT : UEV_AUTH_REQ_DENY;
		break;
	default:
		break;
	}

	if (!ret && si->stats[type].blocked_cur >= config.max_retry_band) {
		ev.reason = UEV_REASON_RETRY_EXCEEDED;
		ev.threshold.cur = si->stats[type].blocked_cur;
		ev.threshold.ref = config.max_retry_band;
	}
	usteer_event(&ev);

	return ret;
}

static bool
is_more_kickable(struct sta_info *si_cur, struct sta_info *si_new)
{
	if (!si_cur)
		return true;

	if (si_new->kick_count > si_cur->kick_count)
		return false;

	return si_cur->signal > si_new->signal;
}

static void
usteer_roam_set_state(struct sta_info *si, enum roam_trigger_state state,
		      struct uevent *ev)
{
	/* NOP in case we remain idle */
	if (si->roam_state == state && si->roam_state == ROAM_TRIGGER_IDLE) {
		si->roam_tries = 0;
		return;
	}

	si->roam_event = current_time;

	if (si->roam_state == state) {
		si->roam_tries++;
	} else {
		si->roam_tries = 0;
	}

	si->roam_state = state;
	usteer_event(ev);
}

static void
usteer_roam_sm_start_scan(struct sta_info *si, struct uevent *ev)
{
	/* Start scanning in case we are not timeout-constrained or timeout has expired */
	if (!config.roam_scan_timeout ||
	    current_time > si->roam_scan_timeout_start + config.roam_scan_timeout) {
		usteer_roam_set_state(si, ROAM_TRIGGER_SCAN, ev);
		return;
	}

	/* We are currently in scan timeout / cooldown.
	 * Check if we are in ROAM_TRIGGER_IDLE state. Enter this state if not.
	 */
	if (si->roam_state == ROAM_TRIGGER_IDLE)
		return;

	/* Enter idle state */
	usteer_roam_set_state(si, ROAM_TRIGGER_IDLE, ev);
}

static struct sta_info *
usteer_roam_sm_found_better_node(struct sta_info *si, struct uevent *ev, enum roam_trigger_state next_state)
{
	uint64_t max_age = 2 * config.roam_scan_interval;
	struct sta_info *candidate;

	if (max_age > current_time - si->roam_scan_start)
		max_age = current_time - si->roam_scan_start;

	candidate = find_better_candidate(si, ev, (1 << UEV_SELECT_REASON_SIGNAL), max_age);
	if (candidate)
		usteer_roam_set_state(si, next_state, ev);

	return candidate;
}

static bool
usteer_roam_trigger_sm(struct usteer_local_node *ln, struct sta_info *si)
{
	struct sta_info *candidate;
	struct uevent ev = {
		.si_cur = si,
	};

	switch (si->roam_state) {
	case ROAM_TRIGGER_SCAN:
		if (!si->roam_tries) {
			si->roam_scan_start = current_time;
		}

		/* Check if we've found a better node regardless of the scan-interval */
		if (usteer_roam_sm_found_better_node(si, &ev, ROAM_TRIGGER_SCAN_DONE))
			break;

		/* Only scan every scan-interval */
		if (current_time - si->roam_event < config.roam_scan_interval)
			break;

		/* Check if no node was found within roam_scan_tries tries */
		if (config.roam_scan_tries && si->roam_tries >= config.roam_scan_tries) {
			if (!config.roam_scan_timeout) {
				/* Prepare to kick client */
				usteer_roam_set_state(si, ROAM_TRIGGER_SCAN_DONE, &ev);
			} else {
				/* Kick in scan timeout */
				si->roam_scan_timeout_start = current_time;
				usteer_roam_set_state(si, ROAM_TRIGGER_IDLE, &ev);
			}
			break;
		}

		/* Send beacon-request to client */
		usteer_ubus_trigger_client_scan(si);
		usteer_roam_sm_start_scan(si, &ev);
		break;

	case ROAM_TRIGGER_IDLE:
		usteer_roam_sm_start_scan(si, &ev);
		break;

	case ROAM_TRIGGER_SCAN_DONE:
		candidate = usteer_roam_sm_found_better_node(si, &ev, ROAM_TRIGGER_SCAN_DONE);
		/* Kick back in case no better node is found */
		if (!candidate) {
			usteer_roam_set_state(si, ROAM_TRIGGER_IDLE, &ev);
			break;
		}

		usteer_ubus_bss_transition_request(si, 1, false, false, 100, candidate->node);
		si->kick_time = current_time + config.roam_kick_delay;
		usteer_roam_set_state(si, ROAM_TRIGGER_IDLE, &ev);
		break;
	}

	return false;
}

bool usteer_policy_can_perform_roam(struct sta_info *si)
{
	/* Only trigger for connected STAs */
	if (si->connected != STA_CONNECTED)
		return false;

	/* Skip on pending kick */
	if (si->kick_time)
		return false;

	/* Skip on rejected transition */
	if (si->bss_transition_response.status_code && current_time - si->bss_transition_response.timestamp < config.steer_reject_timeout)
		return false;

	/* Skip on previous kick attempt */
	if (current_time - si->roam_kick < config.roam_trigger_interval)
		return false;

	/* Skip if connection is established shorter than the trigger-interval */
	if (current_time - si->connected_since < config.roam_trigger_interval)
		return false;
	
	return true;
}

static bool
usteer_local_node_roam_sm_active(struct sta_info *si, int min_signal)
{
	if (!usteer_policy_can_perform_roam(si))
		return false;

	/* Signal has to be below scan / roam threshold */
	if (si->signal >= min_signal)
		return false;

	return true;
}

static void
usteer_local_node_roam_check(struct usteer_local_node *ln, struct uevent *ev)
{
	struct sta_info *si;
	int min_signal;

	if (config.roam_scan_snr)
		min_signal = config.roam_scan_snr;
	else if (config.roam_trigger_snr)
		min_signal = config.roam_trigger_snr;
	else
		return;

	usteer_update_time();
	min_signal = usteer_snr_to_signal(&ln->node, min_signal);

	list_for_each_entry(si, &ln->node.sta_info, node_list) {
		if (!usteer_local_node_roam_sm_active(si, min_signal)) {
			usteer_roam_set_state(si, ROAM_TRIGGER_IDLE, ev);
			continue;
		}

		/*
		 * If the state machine kicked a client, other clients should wait
		 * until the next turn
		 */
		if (usteer_roam_trigger_sm(ln, si))
			return;
	}
}

static void
usteer_local_node_snr_kick(struct usteer_local_node *ln)
{
	unsigned int min_count = DIV_ROUND_UP(config.min_snr_kick_delay, config.local_sta_update);
	struct uevent ev = {
		.node_local = &ln->node,
	};
	struct sta_info *si;
	int min_signal;

	if (!config.min_snr)
		return;

	min_signal = usteer_snr_to_signal(&ln->node, config.min_snr);
	ev.threshold.ref = min_signal;

	list_for_each_entry(si, &ln->node.sta_info, node_list) {
		if (si->connected != STA_CONNECTED)
			continue;

		if (si->signal >= min_signal) {
			si->below_min_snr = 0;
			continue;
		} else {
			si->below_min_snr++;
		}

		if (si->below_min_snr <= min_count)
			continue;

		ev.type = UEV_SIGNAL_KICK;
		ev.threshold.cur = si->signal;
		ev.count = si->kick_count;
		usteer_event(&ev);

		usteer_ubus_kick_client(si);
		return;
	}
}

static void
usteer_local_node_load_kick(struct usteer_local_node *ln)
{
	struct usteer_node *node = &ln->node;
	struct sta_info *kick1 = NULL, *kick2 = NULL;
	struct sta_info *candidate = NULL;
	struct sta_info *si;
	struct uevent ev = {
		.node_local = &ln->node,
	};
	unsigned int min_count = DIV_ROUND_UP(config.load_kick_delay, config.local_sta_update);

	if (!config.load_kick_enabled || !config.load_kick_threshold ||
	    !config.load_kick_delay)
		return;

	if (node->load < config.load_kick_threshold) {
		if (!ln->load_thr_count)
			return;

		ln->load_thr_count = 0;
		ev.type = UEV_LOAD_KICK_RESET;
		ev.threshold.cur = node->load;
		ev.threshold.ref = config.load_kick_threshold;
		goto out;
	}

	if (++ln->load_thr_count <= min_count) {
		if (ln->load_thr_count > 1)
			return;

		ev.type = UEV_LOAD_KICK_TRIGGER;
		ev.threshold.cur = node->load;
		ev.threshold.ref = config.load_kick_threshold;
		goto out;
	}

	ln->load_thr_count = 0;
	if (node->n_assoc < config.load_kick_min_clients) {
		ev.type = UEV_LOAD_KICK_MIN_CLIENTS;
		ev.threshold.cur = node->n_assoc;
		ev.threshold.ref = config.load_kick_min_clients;
		goto out;
	}

	list_for_each_entry(si, &ln->node.sta_info, node_list) {
		struct sta_info *tmp;

		if (si->connected != STA_CONNECTED)
			continue;

		if (is_more_kickable(kick1, si))
			kick1 = si;

		tmp = find_better_candidate(si, NULL, (1 << UEV_SELECT_REASON_LOAD), 0);
		if (!tmp)
			continue;

		if (is_more_kickable(kick2, si)) {
			kick2 = si;
			candidate = tmp;
		}
	}

	if (!kick1) {
		ev.type = UEV_LOAD_KICK_NO_CLIENT;
		goto out;
	}

	if (kick2)
		kick1 = kick2;

	kick1->kick_count++;

	ev.type = UEV_LOAD_KICK_CLIENT;
	ev.si_cur = kick1;
	ev.si_other = candidate;
	ev.count = kick1->kick_count;

	usteer_ubus_kick_client(kick1);

out:
	usteer_event(&ev);
}

static void
usteer_local_node_perform_kick(struct usteer_local_node *ln)
{
	struct sta_info *si;

	list_for_each_entry(si, &ln->node.sta_info, node_list) {
		if (!si->kick_time || si->kick_time > current_time)
			continue;

		usteer_ubus_kick_client(si);
	}
}

void
usteer_local_node_kick(struct usteer_local_node *ln)
{
	struct uevent ev = {
		.node_local = &ln->node,
	};

	usteer_local_node_perform_kick(ln);

	usteer_local_node_snr_kick(ln);
	usteer_local_node_load_kick(ln);
	usteer_local_node_roam_check(ln, &ev);
}
