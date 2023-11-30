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
 *   Copyright (C) 2022 David Bauer <mail@david-bauer.net>
 */

#include "usteer.h"
#include "node.h"

void usteer_band_steering_sta_update(struct sta_info *si)
{
	if (si->signal < usteer_snr_to_signal(si->node, config.band_steering_min_snr))
		si->band_steering.below_snr = true;
}

bool usteer_band_steering_is_target(struct usteer_local_node *ln, struct usteer_node *node)
{
	if (&ln->node == node)
		return false;

	if (strcmp(ln->node.ssid, node->ssid))
		return false;

	if (node->freq < 4000)
		return false;

	if (!usteer_policy_node_below_max_assoc(node))
		return false;
	
	/* ToDo: Skip nodes with active load-kick */
	
	return true;
 }


static bool usteer_band_steering_has_target_iface(struct usteer_local_node *ln)
{
	struct usteer_node *node;

	for_each_local_node(node) {
		if (usteer_band_steering_is_target(ln, node))
			return true;
	}

	return false;
}

void usteer_band_steering_perform_steer(struct usteer_local_node *ln)
{
	unsigned int min_count = DIV_ROUND_UP(config.band_steering_interval, config.local_sta_update);
	struct sta_info *si;

	if (!config.band_steering_interval)
		return;

	/* Band-Steering is only available on 2.4 GHz interfaces */
	if (ln->node.freq > 4000)
		return;

	/* Check if we have an interface we can steer to */
	if (!usteer_band_steering_has_target_iface(ln))
		return;

	/* Only steer every interval */
	if (ln->band_steering_interval < min_count) {
		ln->band_steering_interval++;
		return;
	}

	ln->band_steering_interval = 0;

	list_for_each_entry(si, &ln->node.sta_info, node_list) {
		/* Check if client is eligable to be steerd */
		if (!usteer_policy_can_perform_roam(si))
			continue;

		/* Skip clients with insufficient SNR-state */
		if (si->band_steering.below_snr) {
			si->band_steering.below_snr = false;
			continue;
		}

		if (si->bss_transition)
			usteer_ubus_band_steering_request(si);

		si->band_steering.below_snr = false;
	}
}