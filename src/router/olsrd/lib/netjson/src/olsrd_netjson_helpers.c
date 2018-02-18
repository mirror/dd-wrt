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

#include "olsrd_netjson_helpers.h"

#include "olsr.h"
#include <stddef.h>

struct node_entry * netjson_constructMidSelf(struct mid_entry *mid) {
  struct node_entry *node_self;
  struct olsr_if *ifs;

  memset(mid, 0, sizeof(*mid));
  mid->main_addr = olsr_cnf->main_addr;

  node_self = olsr_malloc(sizeof(struct node_entry), "netjson NetworkGraph node - MID - self");
  memset(node_self, 0, sizeof(*node_self));
  node_self->avl.key = &olsr_cnf->main_addr;
  node_self->isAlias = false;
  node_self->mid = mid;

  for (ifs = olsr_cnf->interfaces; ifs != NULL ; ifs = ifs->next) {
    union olsr_ip_addr *addr = NULL;
    bool is_self_main;

    if (!ifs->configured) {
      continue;
    }

    if (ifs->host_emul) {
      addr = &ifs->hemu_ip;
    } else {
      struct interface_olsr *iface = ifs->interf;

      if (!iface) {
        continue;
      }

      addr = (olsr_cnf->ip_version == AF_INET) //
          ? (union olsr_ip_addr *) &iface->int_addr.sin_addr //
          : (union olsr_ip_addr *) &iface->int6_addr.sin6_addr;
    }

    is_self_main = (olsr_cnf->ip_version == AF_INET) //
        ? ip4equal(&mid->main_addr.v4, &addr->v4) //
        : ip6equal(&mid->main_addr.v6, &addr->v6);

    if (!is_self_main) {
      struct node_entry *node_self_alias;
      struct mid_address *alias;

      node_self_alias = olsr_malloc(sizeof(struct node_entry), "netjson NetworkGraph node - MID - self alias");
      memset(node_self_alias, 0, sizeof(*node_self_alias));
      node_self_alias->avl.key = addr;
      node_self_alias->isAlias = true;
      node_self_alias->mid = mid;

      alias = olsr_malloc(sizeof(struct mid_address), "netjson NetworkGraph node - MID - self alias");
      memset(alias, 0, sizeof(*alias));
      alias->alias = *addr;
      alias->main_entry = mid;
      alias->next_alias = mid->aliases;
      alias->vtime = 0;

      mid->aliases = alias;
    }
  }

  return node_self;
}

void netjson_cleanup_mid_self(struct node_entry *node_entry) {
  if (node_entry->avl.key != &olsr_cnf->main_addr) {
    return;
  }

  while (node_entry->mid->aliases) {
    struct mid_address *alias = node_entry->mid->aliases;
    node_entry->mid->aliases = node_entry->mid->aliases->next_alias;
    free(alias);
  }
}

void netjson_midIntoNodesTree(struct avl_tree *nodes, struct mid_entry *mid) {
  struct avl_node *avlnode;
  struct node_entry *node;
  struct mid_address *alias;

  avlnode = avl_find(nodes, &mid->main_addr);
  if (!avlnode) {
    /* the IP address is not yet known */

    node = olsr_malloc(sizeof(struct node_entry), "netjson NetworkGraph node - MID - main");
    memset(node, 0, sizeof(*node));
    node->avl.key = &mid->main_addr;
    node->isAlias = false;
    node->mid = mid;
    if (avl_insert(nodes, &node->avl, AVL_DUP_NO) == -1) {
      /* duplicate */
      free(node);
    }
  }

  alias = mid->aliases;
  while (alias) {
    avlnode = avl_find(nodes, &alias->alias);
    if (!avlnode) {
      /* the IP address is not yet known */

      node = olsr_malloc(sizeof(struct node_entry), "netjson NetworkGraph node - MID - alias");
      memset(node, 0, sizeof(*node));
      node->avl.key = &alias->alias;
      node->isAlias = true;
      node->mid = mid;
      if (avl_insert(nodes, &node->avl, AVL_DUP_NO) == -1) {
        /* duplicate */
        free(node);
      }
    }

    alias = alias->next_alias;
  }
}

void netjson_tcIntoNodesTree(struct avl_tree *nodes, struct tc_entry *tc) {
  struct avl_node *avlnode;
  struct node_entry *node;

  avlnode = avl_find(nodes, &tc->addr);
  if (avlnode) {
    /* the IP address is already known */
    return;
  }

  /* the IP address is not yet known */

  node = olsr_malloc(sizeof(struct node_entry), "netjson NetworkGraph node - TC - main");
  memset(node, 0, sizeof(*node));
  node->avl.key = &tc->addr;
  node->isAlias = false;
  node->tc = tc;
  if (avl_insert(nodes, &node->avl, AVL_DUP_NO) == -1) {
    /* duplicate */
    free(node);
  }
}

void netjson_tcEdgeIntoNodesTree(struct avl_tree *nodes, struct tc_edge_entry *tc_edge) {
  struct avl_node *avlnode;
  struct node_entry *node;

  avlnode = avl_find(nodes, &tc_edge->T_dest_addr);
  if (avlnode) {
    /* the IP address is already known */
    return;
  }

  /* the IP address is not yet known */

  node = olsr_malloc(sizeof(struct node_entry), "netjson NetworkGraph node - TC - main");
  memset(node, 0, sizeof(*node));
  node->avl.key = &tc_edge->T_dest_addr;
  node->isAlias = false;
  node->tc_edge = tc_edge;
  if (avl_insert(nodes, &node->avl, AVL_DUP_NO) == -1) {
    /* duplicate */
    free(node);
  }
}

void netjson_linkIntoNodesTree(struct avl_tree *nodes, struct link_entry *link, union olsr_ip_addr *addr) {
  struct avl_node *avlnode;
  struct node_entry *node;

  avlnode = avl_find(nodes, addr);
  if (avlnode) {
    /* the IP address is already known */
    return;
  }

  /* the IP address is not yet known */

  node = olsr_malloc(sizeof(struct node_entry), "netjson NetworkGraph node - link");
  memset(node, 0, sizeof(*node));
  node->avl.key = addr;
  node->isAlias = false;
  node->link = link;
  if (avl_insert(nodes, &node->avl, AVL_DUP_NO) == -1) {
    /* duplicate */
    free(node);
  }
}

void netjson_neighborIntoNodesTree(struct avl_tree *nodes, struct neighbor_entry *neighbor) {
  struct avl_node *avlnode;
  struct node_entry *node;

  avlnode = avl_find(nodes, &neighbor->neighbor_main_addr);
  if (avlnode) {
    /* the IP address is already known */
    return;
  }

  /* the IP address is not yet known */

  node = olsr_malloc(sizeof(struct node_entry), "netjson NetworkGraph node - neighbor");
  memset(node, 0, sizeof(*node));
  node->avl.key = &neighbor->neighbor_main_addr;
  node->isAlias = false;
  node->neighbor = neighbor;
  if (avl_insert(nodes, &node->avl, AVL_DUP_NO) == -1) {
    /* duplicate */
    free(node);
  }
}
