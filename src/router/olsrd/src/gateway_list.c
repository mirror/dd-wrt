/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#ifdef __linux__

#include "gateway_list.h"

#include "gateway.h"
#include "common/list.h"

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

/*
 * Exported functions
 */

/**
 * Initialization
 *
 * @param list a pointer to the list
 * @param count the maximum number of entries to be kept in the list. Must be
 * larger than zero
 */
void olsr_gw_list_init(struct gw_list * list, uint8_t count) {
	assert(list);
	assert(count > 0);

	list_head_init(&list->head);
	list->count_max = count;
	list->count = 0;
}

/**
 * Cleanup
 *
 * @param list a pointer to the list
 */
void olsr_gw_list_cleanup(struct gw_list * list __attribute__((unused))) {
	/* nothing to do */
}

/**
 * Find an entry on the list
 *
 * @param list a pointer to the list
 * @param entry a pointer to the entry to find
 * @return a pointer to the entry, or NULL when not found
 */
struct gw_container_entry * olsr_gw_list_find(struct gw_list * list, struct gateway_entry * entry) {
	struct gw_container_entry * gw;

	assert(list);
	assert(entry);

	OLSR_FOR_ALL_GWS(&list->head, gw) {
		if (gw->gw && (gw->gw == entry)) {
			return gw;
		}
	}
	OLSR_FOR_ALL_GWS_END(gw);

	return NULL;
}

/**
 * Add an entry to the list.
 *
 * The list is ordered on costs with the lowest costs (best) first and the
 * highest costs (worst) last. In case of equal costs, the entry is added
 * _before_ the one(s) that is(are) already in the list.
 *
 * @param list a pointer to the list
 * @param entry a pointer to the entry
 * @return a pointer to the added entry
 */
struct gw_container_entry * olsr_gw_list_add(struct gw_list * list, struct gw_container_entry * entry) {
	struct gw_container_entry * gw;

	assert(list);
	assert(entry);
	assert(!olsr_gw_list_full(list));

	list_node_init(&entry->list_node);

	OLSR_FOR_ALL_GWS(&list->head, gw) {
	  assert(gw);
	  assert(gw->gw);
	  assert(entry->gw);
	  if (entry->gw->path_cost <= gw->gw->path_cost) {
			/* add before the iterated list entry: the gateway to insert has lower
			 * costs or has equal costs but is newer (since we insert it) */
			list_add_before(&gw->list_node, &entry->list_node);
			list->count++;
			return entry;
		}
	}
	OLSR_FOR_ALL_GWS_END(gw);

	/* add at the end */
	list_add_after(list->head.prev, &entry->list_node);
	list->count++;
	return entry;
}

/**
 * Update an entry on the list (re-sort the list.)
 *
 * @param list a pointer to the list
 * @param entry a pointer to the entry
 * @return a pointer to the updated entry
 */
struct gw_container_entry * olsr_gw_list_update(struct gw_list * list, struct gw_container_entry * entry) {
	assert(list);
	assert(entry);
	assert(!olsr_gw_list_empty(list));

	/* don't touch gw */
	/* don't touch tunnel */
	/* don't touch list_node */

	list_remove(&entry->list_node);
	list->count--;
	return olsr_gw_list_add(list, entry);
}

/**
 * Remove a gateway from the list (but do not free it's memory)
 *
 * @param list a pointer to the list
 * @param entry a pointer to the gateway
 * @return a pointer to the removed entry
 */
struct gw_container_entry * olsr_gw_list_remove(struct gw_list * list, struct gw_container_entry * entry) {
	assert(list);
	assert(entry);
	assert(!olsr_gw_list_empty(list));

	list_remove(&entry->list_node);
	list->count--;
	return entry;
}

#endif /* __linux__ */
