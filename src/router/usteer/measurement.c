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
 *   Copyright (C) 2021 David Bauer <mail@david-bauer.net> 
 */

#include "usteer.h"

LIST_HEAD(measurements);
static struct usteer_timeout_queue tq;

int
usteer_measurement_get_rssi(struct usteer_measurement_report *report)
{
	/* Apple devices always set the RSNI to 0, while
	 * it should be set to 255 in case RSNI is unavailable.
	 *
	 * For them, RCPI seems to be calculated as RCPI = 255 + (RSSI)
	 */

	if (!report->rsni)
		return report->rcpi - 255;

	return (report->rcpi / 2) - 110;
}

void
usteer_measurement_report_node_cleanup(struct usteer_node *node)
{
	struct usteer_measurement_report *mr, *tmp;

	list_for_each_entry_safe(mr, tmp, &node->measurements, node_list)
		usteer_measurement_report_del(mr);
}

void
usteer_measurement_report_sta_cleanup(struct sta *sta)
{
	struct usteer_measurement_report *mr, *tmp;

	list_for_each_entry_safe(mr, tmp, &sta->measurements, sta_list)
		usteer_measurement_report_del(mr);
}

struct usteer_measurement_report *
usteer_measurement_report_get(struct sta *sta, struct usteer_node *node, bool create)
{
	struct usteer_measurement_report *mr;

	list_for_each_entry(mr, &sta->measurements, sta_list) {
		if (mr->node == node)
			return mr;
	}

	if (!create)
		return NULL;

	mr = calloc(1, sizeof(*mr));
	if (!mr)
		return NULL;

	/* Set node & add to nodes list */
	mr->node = node;
	list_add(&mr->node_list, &node->measurements);

	/* Set sta & add to STAs list */
	mr->sta = sta;
	list_add(&mr->sta_list, &sta->measurements);

	/* Add to Measurement list */
	list_add(&mr->list, &measurements);

	/* Set measurement expiration */
	usteer_timeout_set(&tq, &mr->timeout, config.measurement_report_timeout);

	return mr;
}

struct usteer_measurement_report *
usteer_measurement_report_add(struct sta *sta, struct usteer_node *node,
			      uint8_t rcpi, uint8_t rsni, uint64_t timestamp)
{
	struct usteer_measurement_report *mr = usteer_measurement_report_get(sta, node, true);

	if (!mr)
		return NULL;

	mr->timestamp = timestamp;
	mr->rsni = rsni;
	mr->rcpi = rcpi;

	/* Reset timeout */
	usteer_timeout_set(&tq, &mr->timeout, config.measurement_report_timeout);

	return mr;
}

void
usteer_measurement_report_del(struct usteer_measurement_report *mr)
{
	usteer_timeout_cancel(&tq, &mr->timeout);
	list_del(&mr->node_list);
	list_del(&mr->sta_list);
	list_del(&mr->list);
	free(mr);
}

static void
usteer_measurement_timeout(struct usteer_timeout_queue *q, struct usteer_timeout *t)
{
	struct usteer_measurement_report *mr = container_of(t, struct usteer_measurement_report, timeout);

	usteer_measurement_report_del(mr);
}

static void __usteer_init usteer_measurement_init(void)
{
	usteer_timeout_init(&tq);
	tq.cb = usteer_measurement_timeout;
}
