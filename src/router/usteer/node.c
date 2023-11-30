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

#include "node.h"
#include "usteer.h"

struct usteer_remote_node *usteer_remote_node_by_bssid(uint8_t *bssid) {
	struct usteer_remote_node *rn;

	for_each_remote_node(rn) {
		if (!memcmp(rn->node.bssid, bssid, 6))
			return rn;
	}

	return NULL;
}

struct usteer_node *usteer_node_by_bssid(uint8_t *bssid) {
	struct usteer_remote_node *rn;
	struct usteer_local_node *ln;

	rn = usteer_remote_node_by_bssid(bssid);
	if (rn)
		return &rn->node;

	ln = usteer_local_node_by_bssid(bssid);
	if (ln)
		return &ln->node;

	return NULL;
}

void usteer_node_set_blob(struct blob_attr **dest, struct blob_attr *val)
{
	int new_len;
	int len;

	if (!val) {
		free(*dest);
		*dest = NULL;
		return;
	}

	len = *dest ? blob_pad_len(*dest) : 0;
	new_len = blob_pad_len(val);
	if (new_len != len)
		*dest = realloc(*dest, new_len);
	memcpy(*dest, val, new_len);
}

static struct usteer_node *
usteer_node_higher_bssid(struct usteer_node *node1, struct usteer_node *node2)
{
	int i;

	for (i = 0; i < 6; i++) {
		if (node1->bssid[i] == node2->bssid[i])
			continue;
		if (node1->bssid[i] < node2->bssid[i])
			return node2;

		break;
	}

	return node1;
}

static struct usteer_node *
usteer_node_higher_roamability(struct usteer_node *node, struct usteer_node *ref)
{
	uint64_t roamability_node, roamability_ref;

	roamability_node = ((uint64_t)(node->roam_events.source + node->roam_events.target)) * current_time / ((current_time - node->created) + 1);
	roamability_ref = ((uint64_t)(ref->roam_events.source + ref->roam_events.target)) * current_time / ((current_time - ref->created) + 1);

	if (roamability_node < roamability_ref)
		return ref;

	return node;
}

static struct usteer_node *
usteer_node_better_neighbor(struct usteer_node *node, struct usteer_node *ref)
{
	struct usteer_node *n1, *n2;

	/**
	 * 1. Return one node if the other one is NULL
	 * 2. Return the node with the higher roam events.
	 * 3. Return the node with the higher BSSID.
	 * 4. Return first method argument.
	 */

	if (!ref)
		return node;

	if (!node)
		return ref;

	n1 = usteer_node_higher_roamability(node, ref);
	n2 = usteer_node_higher_roamability(ref, node);
	if (n1 == n2)
		return n1;

	/* Identical roam interactions. Check BSSID */
	n1 = usteer_node_higher_bssid(node, ref);
	n2 = usteer_node_higher_bssid(ref, node);
	if (n1 == n2)
		return n1;

	return node;
}

struct usteer_node *
usteer_node_get_next_neighbor(struct usteer_node *current_node, struct usteer_node *last)
{
	struct usteer_remote_node *rn;
	struct usteer_node *next = NULL, *n1, *n2;

	for_each_remote_node(rn) {
		if (next == &rn->node)
			continue;

		if (strcmp(current_node->ssid, rn->node.ssid))
			continue;

		/* Skip nodes which can't handle additional STA */
		if (rn->node.max_assoc && rn->node.n_assoc >= rn->node.max_assoc)
			continue;

		/* Check if this node is ranked lower than the last one */
		n1 = usteer_node_better_neighbor(last, &rn->node);
		n2 = usteer_node_better_neighbor(&rn->node, last);
		if (n1 != n2) {
			/* Identical rank. Skip. */
			continue;
		} else if (last && n1 == &rn->node) {
			/* Candidate rank is higher than the last neighbor. Skip. */
			continue;
		}

		/* Check with current next candidate */
		n1 = usteer_node_better_neighbor(next, &rn->node);
		n2 = usteer_node_better_neighbor(&rn->node, next);
		if (n1 != n2) {
			/* Identical rank. Skip. */
			continue;
		} else if (n1 != &rn->node) {
			/* Next candidate ranked higher. */
			continue;
		}

		next = n1;		
	}

	return next;
}
