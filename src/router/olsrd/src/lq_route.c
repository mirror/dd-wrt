/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Thomas Lopatic (thomas@lopatic.de)
 * IPv4 performance optimization (c) 2006, sven-ola(gmx.de)
 * SPF implementation (c) 2007, Hannes Gredler (hannes@gredler.at)
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
 * $Id: lq_route.c,v 1.55 2007/10/20 15:16:32 bernd67 Exp $
 */

#include "defs.h"
#include "olsr.h"
#include "tc_set.h"
#include "neighbor_table.h"
#include "two_hop_neighbor_table.h"
#include "link_set.h"
#include "routing_table.h"
#include "mid_set.h"
#include "hna_set.h"
#include "lq_list.h"
#include "lq_avl.h"
#include "lq_route.h"

/*
 * avl_comp_etx
 *
 * compare two etx metrics.
 * return 0 if there is an exact match and
 * -1 / +1 depending on being smaller or bigger.
 * note that this results in the most optimal code
 * after compiler optimization.
 */
static int
avl_comp_etx (void *etx1, void *etx2)
{       
  if (*(float *)etx1 < *(float *)etx2) {
    return -1;
  }

  if (*(float *)etx1 > *(float *)etx2) {
    return +1;
  }

  return 0;
}

/*
 * olsr_spf_add_cand_tree
 *
 * Key an existing vertex to a candidate tree.
 */
static void
olsr_spf_add_cand_tree (struct avl_tree *tree,
                        struct tc_entry *vert)
{
  vert->cand_tree_node.key = &vert->path_etx;
  vert->cand_tree_node.data = vert;

#ifdef DEBUG
  OLSR_PRINTF(1, "SPF: insert candidate %s, cost %f\n",
              olsr_ip_to_string(&(vert->addr)),
              vert->path_etx);
#endif

  avl_insert(tree, &vert->cand_tree_node, AVL_DUP);
}

/*
 * olsr_spf_del_cand_tree
 *
 * Unkey an existing vertex from a candidate tree.
 */
static void
olsr_spf_del_cand_tree (struct avl_tree *tree,
                        struct tc_entry *vert)
{

#ifdef DEBUG
  OLSR_PRINTF(1, "SPF: delete candidate %s, cost %f\n",
              olsr_ip_to_string(&(vert->addr)),
              vert->path_etx);
#endif

  avl_delete(tree, &vert->cand_tree_node);
}

/*
 * olsr_spf_add_path_list
 *
 * Insert an SPF result at the end of the path list.
 */
static void
olsr_spf_add_path_list (struct list_node *head,
                        int *path_count,
                        struct tc_entry *vert)
{
  vert->path_list_node.data = vert;

#ifdef DEBUG
  OLSR_PRINTF(1, "SPF: append path %s, cost %f, via %s\n",
              olsr_ip_to_string(&(vert->addr)),
              vert->path_etx,
              olsr_ip_to_string(vert->next_hop ? &vert->next_hop->neighbor_iface_addr : NULL));
#endif

  list_add_before(head, &vert->path_list_node);
  *path_count = *path_count + 1;
}

/*
 * olsr_spf_extract_best
 *
 * return the node with the minimum pathcost.
 */
static struct tc_entry *
olsr_spf_extract_best (struct avl_tree *tree)
{
  struct avl_node *node;

  node = avl_walk_first(tree);

  return (node ? node->data :  NULL);
}


char *olsr_etx_to_string(float etx)
{
  static char buff[20];

  if (etx == INFINITE_ETX)
    return "INF";

  snprintf(buff, 20, "%.6f", etx);
  return buff;
}


/*
 * olsr_spf_relax
 *
 * Explore all edges of a node and add the node
 * to the candidate tree if the if the aggregate
 * path cost is better.
 */
static void
olsr_spf_relax (struct avl_tree *cand_tree, struct tc_entry *vert)
{
  struct tc_entry *new_vert;
  struct tc_edge_entry *tc_edge;
  struct avl_node *edge_node;
  float new_etx;

#ifdef DEBUG
  OLSR_PRINTF(1, "SPF: exploring node %s, cost %f\n",
              olsr_ip_to_string(&vert->addr),
              vert->path_etx);
#endif

  /*
   * loop through all edges of this vertex.
   */
  for (edge_node = avl_walk_first(&vert->edge_tree);
       edge_node;
       edge_node = avl_walk_next(edge_node)) {

    tc_edge = edge_node->data;

    /*
     * We are not interested in dead-end or dying edges.
     */
    if (!tc_edge->edge_inv || (tc_edge->flags & OLSR_TC_EDGE_DOWN)) {
#ifdef DEBUG
      OLSR_PRINTF(1, "SPF:   ignoring edge %s\n",
                  olsr_ip_to_string(&tc_edge->T_dest_addr));
      if (tc_edge->flags & OLSR_TC_EDGE_DOWN) {
        OLSR_PRINTF(1, "SPF:     edge down\n");
      }
      if (!tc_edge->edge_inv) {
        OLSR_PRINTF(1, "SPF:     no inverse edge\n");
      }
#endif
      continue;
    }

    /*
     * total quality of the path through this vertex
     * to the destination of this edge
     */
    new_etx = vert->path_etx + tc_edge->etx;

#ifdef DEBUG
      OLSR_PRINTF(1, "SPF:   exploring edge %s, cost %s\n",
                  olsr_ip_to_string(&(tc_edge->T_dest_addr)),
                  olsr_etx_to_string(new_etx));
#endif

      /* 
       * if it's better than the current path quality of this edge's
       * destination node, then we've found a better path to this node.
       */
    new_vert = tc_edge->edge_inv->tc;
    if (new_etx < new_vert->path_etx) {

      /* if this node has been on the candidate tree delete it */
      if (new_vert->path_etx != INFINITE_ETX) {
        olsr_spf_del_cand_tree(cand_tree, new_vert);
      }

      /* re-insert on candidate tree with the better metric */
      new_vert->path_etx = new_etx;
      olsr_spf_add_cand_tree(cand_tree, new_vert);

      /* pull-up the next-hop and bump the hop count */
      if (vert->next_hop) {
        new_vert->next_hop = vert->next_hop;
      }
      new_vert->hops = vert->hops + 1;

#ifdef DEBUG
      OLSR_PRINTF(1, "SPF:   better path to %s, cost %s -> %s, via %s, hops %u\n",
                  olsr_ip_to_string(&new_vert->addr),
                  olsr_etx_to_string(new_vert->path_etx),
                  olsr_etx_to_string(new_etx),
                  olsr_ip_to_string(vert->next_hop ?
                                    &vert->next_hop->neighbor_iface_addr : NULL),
                  new_vert->hops);
#endif

    }
  }
}

/*
 * olsr_spf_run_full
 *
 * Run the Dijkstra algorithm.
 * 
 * A node gets added to the candidate tree when one of its edges has
 * an overall better root path cost than the node itself.
 * The node with the shortest metric gets moved from the candidate to
 * the path list every pass.
 * The SPF computation is completed when there are no more nodes
 * on the candidate tree. 
 */ 
static void
olsr_spf_run_full (struct avl_tree *cand_tree, struct list_node *path_list,
                   int *path_count)
{
  struct tc_entry *vert;

  *path_count = 0;

  while ((vert = olsr_spf_extract_best(cand_tree))) {

    olsr_spf_relax(cand_tree, vert);

    /*
     * move the best path from the candidate tree
     * to the path list.
     */
    olsr_spf_del_cand_tree(cand_tree, vert);
    olsr_spf_add_path_list(path_list, path_count, vert);
  }
}

void
olsr_calculate_routing_table (void)
{
  struct avl_tree cand_tree;
  struct list_node path_list;
  int i, plen, path_count = 0;
  struct tc_entry *tc;
  struct tc_edge_entry *tc_edge;
  struct tc_entry *vert;
  struct neighbor_entry *neigh;
  struct mid_address *mid_walker;
  struct hna_entry *hna_gw;
  struct hna_net *hna;
  struct interface *inter;
  struct link_entry *link;

#ifdef SPF_PROFILING
  struct timeval t1, t2, t3, t4, t5, spf_init, spf_run, route, kernel, total;

  gettimeofday(&t1, NULL);
#endif

  /*
   * Prepare the candidate tree and result list.
   */
  avl_init(&cand_tree, avl_comp_etx);
  list_head_init(&path_list);
  olsr_bump_routingtree_version();

  /*
   * Initialize vertices in the lsdb.
   */
  OLSR_FOR_ALL_TC_ENTRIES(tc) {
    tc->next_hop = NULL;
    tc->path_etx = INFINITE_ETX;
    tc->hops = 0;
  } OLSR_FOR_ALL_TC_ENTRIES_END(tc);

  /*
   * zero ourselves and add us to the candidate tree.
   */
  olsr_change_myself_tc();
  tc_myself->path_etx = ZERO_ETX;
  olsr_spf_add_cand_tree(&cand_tree, tc_myself);

  /*
   * add edges to and from our neighbours.
   */
  for (i = 0; i < HASHSIZE; i++)
    for (neigh = neighbortable[i].next; neigh != &neighbortable[i];
         neigh = neigh->next) {

      if (neigh->status == SYM) {

        tc_edge = olsr_lookup_tc_edge(tc_myself, &neigh->neighbor_main_addr);
        link = get_best_link_to_neighbor(&neigh->neighbor_main_addr);
	if (!link) {

          /*
           * If there is no best link to this neighbor
           * and we had an edge before then flush the edge.
           */
          if (tc_edge) {
            olsr_delete_tc_edge_entry(tc_edge);
          }
	  continue;
        }

        /*
         * Set the next-hops of our neighbors. 
         */
        if (!tc_edge) {
          tc_edge = olsr_add_tc_edge_entry(tc_myself, &neigh->neighbor_main_addr,
                                           0, link->last_htime,
                                           link->loss_link_quality2,
                                           link->neigh_link_quality2);
        } else {
          tc_edge->link_quality = link->loss_link_quality2;
          tc_edge->inverse_link_quality = link->neigh_link_quality2;
          olsr_calc_tc_edge_entry_etx(tc_edge);
        }
        if (tc_edge->edge_inv) {
          tc_edge->edge_inv->tc->next_hop = link;
        }
      }
    }

#ifdef SPF_PROFILING
  gettimeofday(&t2, NULL);
#endif

  /*
   * Run the SPF calculation.
   */
  olsr_spf_run_full(&cand_tree, &path_list, &path_count);

  OLSR_PRINTF(2, "\n--- %02d:%02d:%02d.%02d ------------------------------------------------- DIJKSTRA\n\n",
              nowtm->tm_hour,
              nowtm->tm_min,
              nowtm->tm_sec,
              (int)now.tv_usec/10000);

#ifdef SPF_PROFILING
  gettimeofday(&t3, NULL);
#endif

  /*
   * In the path tree we have all the reachable nodes in our topology.
   */
  for (; !list_is_empty(&path_list); list_remove(path_list.next)) {

    vert = path_list.next->data;
    link = vert->next_hop;

    if (!link) {
      OLSR_PRINTF(2, "%s no next-hop\n", olsr_ip_to_string(&vert->addr));
      continue;
    }

    /* find the interface for the found link */
    inter = link->if_name ? if_ifwithname(link->if_name)
      : if_ifwithaddr(&link->local_iface_addr);

    /* interface is up ? */
    if (inter) {

      /* add a route to the main address of the destination node */
      olsr_insert_routing_table(&vert->addr, olsr_cnf->maxplen, &vert->addr,
                                &link->neighbor_iface_addr, inter->if_index,
                                vert->hops, vert->path_etx);

      /* add routes to the remaining interfaces of the destination node */
      for (mid_walker = mid_lookup_aliases(&vert->addr);
           mid_walker != NULL;
           mid_walker = mid_walker->next_alias) {

        olsr_insert_routing_table(&mid_walker->alias, olsr_cnf->maxplen, &vert->addr,
                                  &link->neighbor_iface_addr, inter->if_index,
                                  vert->hops, vert->path_etx);
      }

      /* find the node's HNAs */
      hna_gw = olsr_lookup_hna_gw(&vert->addr);

      /* node doesn't announce any HNAs */
      if (!hna_gw) {
        continue;
      }

      /* loop through the node's HNAs */
      for (hna = hna_gw->networks.next;
           hna != &hna_gw->networks;
           hna = hna->next) {

        plen = olsr_get_hna_prefix_len(hna);
        if (vert->path_etx != INFINITE_ETX)
        olsr_insert_routing_table(&hna->A_network_addr, plen, &vert->addr,
                                  &link->neighbor_iface_addr, inter->if_index,
                                  vert->hops, vert->path_etx);
      }
    }
  }

#ifdef SPF_PROFILING
  gettimeofday(&t4, NULL);
#endif

  /* move the route changes into the kernel */

  olsr_update_kernel_routes();

#ifdef SPF_PROFILING
  gettimeofday(&t5, NULL);
#endif

#ifdef SPF_PROFILING
  timersub(&t2, &t1, &spf_init);
  timersub(&t3, &t2, &spf_run);
  timersub(&t4, &t3, &route);
  timersub(&t5, &t4, &kernel);
  timersub(&t5, &t1, &total);
  olsr_printf(1, "\n--- SPF-stats for %d nodes, %d routes (total/init/run/route/kern): "
              "%d, %d, %d, %d, %d\n",
              path_count, routingtree.count,
              (int)total.tv_usec, (int)spf_init.tv_usec, (int)spf_run.tv_usec,
              (int)route.tv_usec, (int)kernel.tv_usec);
#endif
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * End:
 */
