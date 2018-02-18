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

#ifndef LIB_NETJSON_SRC_OLSRD_NETJSON_HELPERS_H_
#define LIB_NETJSON_SRC_OLSRD_NETJSON_HELPERS_H_

#include <stdbool.h>

#include "common/avl.h"
#include "mid_set.h"
#include "tc_set.h"
#include "link_set.h"
#include "neighbor_table.h"
#include "olsr_types.h"

struct node_entry {
  struct avl_node avl;
  bool isAlias;
  struct mid_entry *mid;
  struct tc_entry *tc;
  struct tc_edge_entry *tc_edge;
  struct link_entry *link;
  struct neighbor_entry *neighbor;
};

/* static INLINE struct node_entry * avlnode2node(struct avl_node *ptr) */
AVLNODE2STRUCT(avlnode2node, struct node_entry, avl);

struct node_entry * netjson_constructMidSelf(struct mid_entry *mid);
void netjson_cleanup_mid_self(struct node_entry *node_entry);
void netjson_midIntoNodesTree(struct avl_tree *nodes, struct mid_entry *mid);
void netjson_tcIntoNodesTree(struct avl_tree *nodes, struct tc_entry *tc);
void netjson_tcEdgeIntoNodesTree(struct avl_tree *nodes, struct tc_edge_entry *tc_edge);
void netjson_linkIntoNodesTree(struct avl_tree *nodes, struct link_entry *link, union olsr_ip_addr *addr);
void netjson_neighborIntoNodesTree(struct avl_tree *nodes, struct neighbor_entry *neigh);

#endif /* LIB_NETJSON_SRC_OLSRD_NETJSON_HELPERS_H_ */
