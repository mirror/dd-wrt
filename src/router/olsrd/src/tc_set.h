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

#ifndef _OLSR_TOP_SET
#define _OLSR_TOP_SET

#include "defs.h"
#include "packet.h"
#include "common/avl.h"
#include "common/list.h"
#include "scheduler.h"

/*
 * This file holds the definitions for the link state database.
 * The lsdb consists of nodes and edges.
 * During input parsing all nodes and edges are extracted and synthesized.
 * The SPF calculation operates on these datasets.
 */

struct tc_edge_entry {
  struct avl_node edge_node;           /* edge_tree node in tc_entry */
  union olsr_ip_addr T_dest_addr;      /* edge_node key */
  struct tc_edge_entry *edge_inv;      /* shortcut, used during SPF calculation */
  struct tc_entry *tc;                 /* backpointer to owning tc entry */
  olsr_linkcost cost;                  /* metric used for SPF calculation */
  uint16_t ansn;                       /* ansn of this edge, used for multipart msgs */
  uint32_t linkquality[0];
};

AVLNODE2STRUCT(edge_tree2tc_edge, struct tc_edge_entry, edge_node);

struct tc_entry {
  struct avl_node vertex_node;         /* node keyed by ip address */
  union olsr_ip_addr addr;             /* vertex_node key */
  struct avl_node cand_tree_node;      /* SPF candidate heap, node keyed by path_etx */
  olsr_linkcost path_cost;             /* SPF calculated distance, cand_tree_node key */
  struct list_node path_list_node;     /* SPF result list */
  struct avl_tree edge_tree;           /* subtree for edges */
  struct avl_tree prefix_tree;         /* subtree for prefixes */
  struct link_entry *next_hop;         /* SPF calculated link to the 1st hop neighbor */
  struct timer_entry *edge_gc_timer;   /* used for edge garbage collection */
  struct timer_entry *validity_timer;  /* tc validity time */
  uint32_t refcount;                   /* reference counter */
  uint16_t msg_seq;                    /* sequence number of the tc message */
  uint8_t msg_hops;                    /* hopcount as per the tc message */
  uint8_t hops;                        /* SPF calculated hopcount */
  uint16_t ansn;                       /* ANSN number of the tc message */
  uint16_t ignored;                    /* how many TC messages ignored in a sequence
                                          (kindof emergency brake) */
  uint16_t err_seq;                    /* sequence number of an unplausible TC */
  bool err_seq_valid;                  /* do we have an error (unplauible seq/ansn) */
};

/*
 * Garbage collection time for edges.
 * This is used for multipart messages.
 */
#define OLSR_TC_EDGE_GC_TIME (2*1000)   /* milliseconds */
#define OLSR_TC_EDGE_GC_JITTER 5        /* percent */

#define OLSR_TC_VTIME_JITTER 5          /* percent */

AVLNODE2STRUCT(vertex_tree2tc, struct tc_entry, vertex_node);
AVLNODE2STRUCT(cand_tree2tc, struct tc_entry, cand_tree_node);
LISTNODE2STRUCT(pathlist2tc, struct tc_entry, path_list_node);

/*
 * macros for traversing vertices, edges and prefixes in the link state database.
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
    tc = vertex_tree2tc(tc_tree_node);
#define OLSR_FOR_ALL_TC_ENTRIES_END(tc) }}

#define OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc, tc_edge) \
{ \
  struct avl_node *tc_edge_node, *next_tc_edge_node; \
  for (tc_edge_node = avl_walk_first(&tc->edge_tree); \
    tc_edge_node; tc_edge_node = next_tc_edge_node) { \
    next_tc_edge_node = avl_walk_next(tc_edge_node); \
    tc_edge = edge_tree2tc_edge(tc_edge_node);
#define OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(tc, tc_edge) }}

#define OLSR_FOR_ALL_PREFIX_ENTRIES(tc, rtp) \
{ \
  struct avl_node *rtp_node, *next_rtp_node; \
  for (rtp_node = avl_walk_first(&tc->prefix_tree); \
    rtp_node; rtp_node = next_rtp_node) { \
    next_rtp_node = avl_walk_next(rtp_node); \
    rtp = rtp_prefix_tree2rtp(rtp_node);
#define OLSR_FOR_ALL_PREFIX_ENTRIES_END(tc, rtp) }}

extern struct avl_tree tc_tree;
extern struct tc_entry *tc_myself;

void olsr_init_tc(void);
void olsr_delete_all_tc_entries(void);
void olsr_change_myself_tc(void);
#ifndef NODEBUG
void olsr_print_tc_table(void);
#else
#define olsr_print_tc_table() do { } while(0)
#endif
void olsr_time_out_tc_set(void);

/* tc msg input parser */
bool olsr_input_tc(union olsr_message *, struct interface_olsr *, union olsr_ip_addr *from);

/* tc_entry manipulation */
struct tc_entry *olsr_lookup_tc_entry(union olsr_ip_addr *);
struct tc_entry *olsr_locate_tc_entry(union olsr_ip_addr *);
void olsr_lock_tc_entry(struct tc_entry *);
void olsr_unlock_tc_entry(struct tc_entry *);

/* tc_edge_entry manipulation */
bool olsr_delete_outdated_tc_edges(struct tc_entry *);
char *olsr_tc_edge_to_string(struct tc_edge_entry *);
struct tc_edge_entry *olsr_lookup_tc_edge(struct tc_entry *, union olsr_ip_addr *);
struct tc_edge_entry *olsr_add_tc_edge_entry(struct tc_entry *, union olsr_ip_addr *, uint16_t);
void olsr_delete_tc_entry(struct tc_entry *);
void olsr_delete_tc_edge_entry(struct tc_edge_entry *);
bool olsr_calc_tc_edge_entry_etx(struct tc_edge_entry *);
void olsr_set_tc_edge_timer(struct tc_edge_entry *, unsigned int);
// static bool olsr_etx_significant_change(float, float);

#endif /* _OLSR_TOP_SET */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
