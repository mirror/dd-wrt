/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
 * LSDB rewrite (c) 2007, Hannes Gredler (hannes@gredler.at)
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
 * $Id: tc_set.h,v 1.18 2007/10/16 09:54:44 bernd67 Exp $
 */

#ifndef _OLSR_TOP_SET
#define _OLSR_TOP_SET

#include "defs.h"
#include "packet.h"

/*
 * This file holds the definitions for the link state database.
 * The lsdb consists of nodes and edges.
 * During input parsing all nodes and edges are extracted and syntesized.
 * The SPF calculation operates on these datasets.
 */

struct tc_edge_entry
{
  struct avl_node    edge_node; /* edge_tree node in tc_entry */
  union olsr_ip_addr T_dest_addr; /* edge_node key */
  struct tc_edge_entry *edge_inv; /* shortcut, used during SPF calculation */
  struct tc_entry    *tc; /* backpointer to owning tc entry */
  clock_t            T_time; /* expiration timer, timer_node key */
  olsr_u16_t         T_seq; /* sequence number */
  olsr_u16_t         flags; /* misc flags */
  float              etx; /* metric used for SPF calculation */
  float              link_quality;
  float              inverse_link_quality;
};

#define OLSR_TC_EDGE_DOWN (1 <<  0) /* this edge is down */

/*
 * Garbage collection time for downed edges
 */
#define OLSR_TC_EDGE_GC_TIME 15*1000 /* milliseconds */

struct tc_entry
{
  struct avl_node    vertex_node; /* node keyed by ip address */
  struct avl_node    cand_tree_node; /* SPF candidate heap, node keyed by path_etx */
  struct list_node   path_list_node; /* SPF result list */
  union olsr_ip_addr addr; /* vertex_node key */
  struct avl_tree    edge_tree; /* subtree for edges */
  struct link_entry *next_hop; /* SPF calculated link to the 1st hop neighbor */
  float              path_etx; /* SPF calculated distance, cand_tree_node key */
  olsr_u8_t          hops; /* SPF calculated hopcount */
};

/*
 * macros for traversing the vertices and edges in the link state database.
 * it is recommended to use this because it hides all the internal
 * datastructure from the callers.
 *
 * the loop prefetches the next node in order to not loose context if
 * for example the caller wants to delete the current entry.
 */
#define OLSR_FOR_ALL_TC_ENTRIES(tc) \
{ \
  struct avl_node *tc_tree_node, *next_tc_tree_node; \
  for (tc_tree_node = avl_walk_first(&tc_tree); \
    tc_tree_node; tc_tree_node = next_tc_tree_node) { \
    next_tc_tree_node = avl_walk_next(tc_tree_node); \
    tc = tc_tree_node->data;
#define OLSR_FOR_ALL_TC_ENTRIES_END(tc) }}

#define OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc, tc_edge) \
{ \
  struct avl_node *tc_edge_node, *next_tc_edge_node; \
  for (tc_edge_node = avl_walk_first(&tc->edge_tree); \
    tc_edge_node; tc_edge_node = next_tc_edge_node) { \
    next_tc_edge_node = avl_walk_next(tc_edge_node); \
    tc_edge = tc_edge_node->data;
#define OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(tc, tc_edge) }}

extern struct avl_tree tc_tree;
extern struct tc_entry *tc_myself;

int olsr_init_tc(void);
void olsr_change_myself_tc(void);
int olsr_tc_delete_mprs(struct tc_entry *, struct tc_message *);
int olsr_tc_update_mprs(struct tc_entry *, struct tc_message *);
int olsr_print_tc_table(void);
void olsr_time_out_tc_set(void);

/* tc_entry manipulation */
struct tc_entry *olsr_lookup_tc_entry(union olsr_ip_addr *);
struct tc_entry *olsr_add_tc_entry(union olsr_ip_addr *);
struct tc_entry *olsr_getnext_tc_entry(struct tc_entry *);

/* tc_edge_entry manipulation */
char *olsr_tc_edge_to_string(struct tc_edge_entry *);
struct tc_edge_entry *olsr_lookup_tc_edge(struct tc_entry *,
                                          union olsr_ip_addr *);
struct tc_edge_entry *olsr_add_tc_edge_entry(struct tc_entry *,
                                             union olsr_ip_addr *, olsr_u16_t,
                                             unsigned int, float, float);
void olsr_delete_tc_edge_entry(struct tc_edge_entry *);
void olsr_calc_tc_edge_entry_etx(struct tc_edge_entry *);

#endif

/*
 * Local Variables:
 * c-basic-offset: 2
 * End:
 */
