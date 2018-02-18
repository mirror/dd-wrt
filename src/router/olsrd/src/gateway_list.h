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

#ifndef _GW_LIST_H
#define _GW_LIST_H

#ifdef __linux__

#include "gateway.h"
#include "defs.h"
#include "common/list.h"
#include "kernel_tunnel.h"
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

/** Holds the list head and list administration */
struct gw_list {
		struct list_node head; /**< The (ordered) list of entries */
		uint8_t count_max; /**< The maximum number of entries in the list */
		uint8_t count; /**< The number of entries in the list */
};

/** A container for a gateway and its tunnel */
struct gw_container_entry {
		struct gateway_entry * gw; /**< the gateway entry */
		struct olsr_iptunnel_entry * tunnel; /**< the gateway tunnel */
		struct list_node list_node; /**< the list node */
};

/** Cast from list_node to gw_container_entry */
LISTNODE2STRUCT(olsr_gw_list_node2entry, struct gw_container_entry, list_node);

/** Deletion safe macro for gateway list traversal (do not delete the previous or next node, current node is ok) */
#define OLSR_FOR_ALL_GWS(head, gw) {\
  struct list_node * _list_node; \
  struct list_node * _next_list_node; \
  for (_list_node = (head)->next; _list_node != (head); _list_node = _next_list_node) { \
    _next_list_node = _list_node->next; \
    gw = olsr_gw_list_node2entry(_list_node); \
    if (gw) {
#define OLSR_FOR_ALL_GWS_END(gw) }}}

/**
 * @param list a pointer to the list
 * @return true when multiple gateways mode is enabled
 */
static INLINE bool olsr_gw_list_isModeMulti(struct gw_list * list) {
	assert(list);
	return (list->count_max > 1);
}

void olsr_gw_list_init(struct gw_list * list, uint8_t count);
void olsr_gw_list_cleanup(struct gw_list * list);

/**
 * @param list a pointer to the list
 * @return true if the list is empty
 */
static INLINE bool olsr_gw_list_empty(struct gw_list * list) {
	assert(list);
	return (list->count == 0);
}

/**
 * @param list a pointer to the list
 * @return true if the list is full
 */
static INLINE bool olsr_gw_list_full(struct gw_list * list) {
	assert(list);
	return (list->count >= list->count_max);
}

/**
 * Get the best entry that is on the list
 *
 * @param list a pointer to the list
 * @return a pointer to the best entry, or NULL when the list is empty
 */
static INLINE struct gw_container_entry * olsr_gw_list_get_best_entry(struct gw_list * list) {
	assert(list);

	if (olsr_gw_list_empty(list)) {
		return NULL;
	}

	/* get the best (first) entry of the list */
	return olsr_gw_list_node2entry(list->head.next);
}

/**
 * Get the worst entry that is on the list
 *
 * @param list a pointer to the list
 * @return a pointer to the worst entry
 */
static INLINE struct gw_container_entry * olsr_gw_list_get_worst_entry(struct gw_list * list) {
	assert(list);

	if (olsr_gw_list_empty(list)) {
		return NULL;
	}

	/* get the worst (last) entry of the list */
	return olsr_gw_list_node2entry(list->head.prev);
}

struct gw_container_entry * olsr_gw_list_find(struct gw_list * list, struct gateway_entry * entry);
struct gw_container_entry * olsr_gw_list_add(struct gw_list * list, struct gw_container_entry * entry);
struct gw_container_entry * olsr_gw_list_update(struct gw_list * list, struct gw_container_entry * entry);
struct gw_container_entry * olsr_gw_list_remove(struct gw_list * list, struct gw_container_entry * entry);

#endif /* __linux__ */
#endif /* _GW_LIST_H */
