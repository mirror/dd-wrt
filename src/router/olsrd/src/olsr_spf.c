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

/*
 * Implementation of Dijkstras algorithm. Initially all nodes
 * are initialized to infinite cost. First we put ourselves
 * on the heap of reachable nodes. Our heap implementation
 * is based on an AVL tree which gives interesting performance
 * characteristics for the frequent operations of minimum key
 * extraction and re-keying. Next all neighbors of a node are
 * explored and put on the heap if the cost of reaching them is
 * better than reaching the current candidate node.
 * The SPF calculation is terminated if there are no more nodes
 * on the heap.
 */

#include "ipcalc.h"
#include "defs.h"
#include "olsr.h"
#include "tc_set.h"
#include "neighbor_table.h"
#include "two_hop_neighbor_table.h"
#include "link_set.h"
#include "routing_table.h"
#include "mid_set.h"
#include "hna_set.h"
#include "common/list.h"
#include "common/avl.h"
#include "olsr_spf.h"
#include "net_olsr.h"
#include "lq_plugin.h"
#include "gateway.h"

#ifdef SPF_PROFILING
#include <time.h>
#endif /* SPF_PROFILING */

struct timer_entry *spf_backoff_timer = NULL;

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
avl_comp_etx(const void *etx1, const void *etx2)
{
  if (*(const olsr_linkcost *)etx1 < *(const olsr_linkcost *)etx2) {
    return -1;
  }

  if (*(const olsr_linkcost *)etx1 > *(const olsr_linkcost *)etx2) {
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
olsr_spf_add_cand_tree(struct avl_tree *tree, struct tc_entry *tc)
{
#if !defined(NODEBUG) && defined(DEBUG)
  struct ipaddr_str buf;
  struct lqtextbuffer lqbuffer;
#endif /* !defined(NODEBUG) && defined(DEBUG) */
  tc->cand_tree_node.key = &tc->path_cost;

#ifdef DEBUG
  OLSR_PRINTF(2, "SPF: insert candidate %s, cost %s\n", olsr_ip_to_string(&buf, &tc->addr),
              get_linkcost_text(tc->path_cost, true, &lqbuffer));
#endif /* DEBUG */

  avl_insert(tree, &tc->cand_tree_node, AVL_DUP);
}

/*
 * olsr_spf_del_cand_tree
 *
 * Unkey an existing vertex from a candidate tree.
 */
static void
olsr_spf_del_cand_tree(struct avl_tree *tree, struct tc_entry *tc)
{

#ifdef DEBUG
#ifndef NODEBUG
  struct ipaddr_str buf;
  struct lqtextbuffer lqbuffer;
#endif /* NODEBUG */
  OLSR_PRINTF(2, "SPF: delete candidate %s, cost %s\n", olsr_ip_to_string(&buf, &tc->addr),
              get_linkcost_text(tc->path_cost, true, &lqbuffer));
#endif /* DEBUG */

  avl_delete(tree, &tc->cand_tree_node);
}

/*
 * olsr_spf_add_path_list
 *
 * Insert an SPF result at the end of the path list.
 */
static void
olsr_spf_add_path_list(struct list_node *head, int *path_count, struct tc_entry *tc)
{
#if !defined(NODEBUG) && defined(DEBUG)
  struct ipaddr_str pathbuf, nbuf;
  struct lqtextbuffer lqbuffer;
#endif /* !defined(NODEBUG) && defined(DEBUG) */

#ifdef DEBUG
  OLSR_PRINTF(2, "SPF: append path %s, cost %s, via %s\n", olsr_ip_to_string(&pathbuf, &tc->addr),
              get_linkcost_text(tc->path_cost, true, &lqbuffer), tc->next_hop ? olsr_ip_to_string(&nbuf,
                                                                                                   &tc->next_hop->
                                                                                                   neighbor_iface_addr) : "-");
#endif /* DEBUG */

  list_add_before(head, &tc->path_list_node);
  *path_count = *path_count + 1;
}

/*
 * olsr_spf_extract_best
 *
 * return the node with the minimum pathcost.
 */
static struct tc_entry *
olsr_spf_extract_best(struct avl_tree *tree)
{
  struct avl_node *node = avl_walk_first(tree);

  return (node ? cand_tree2tc(node) : NULL);
}

/*
 * olsr_spf_relax
 *
 * Explore all edges of a node and add the node
 * to the candidate tree if the if the aggregate
 * path cost is better.
 */
static void
olsr_spf_relax(struct avl_tree *cand_tree, struct tc_entry *tc)
{
  struct avl_node *edge_node;
  olsr_linkcost new_cost;

#ifdef DEBUG
#ifndef NODEBUG
  struct ipaddr_str buf, nbuf;
  struct lqtextbuffer lqbuffer;
#endif /* NODEBUG */
  OLSR_PRINTF(2, "SPF: exploring node %s, cost %s\n", olsr_ip_to_string(&buf, &tc->addr),
              get_linkcost_text(tc->path_cost, true, &lqbuffer));
#endif /* DEBUG */

  /*
   * loop through all edges of this vertex.
   */
  for (edge_node = avl_walk_first(&tc->edge_tree); edge_node; edge_node = avl_walk_next(edge_node)) {

    struct tc_entry *new_tc;
    struct tc_edge_entry *tc_edge = edge_tree2tc_edge(edge_node);

    /*
     * We are not interested in dead-end edges.
     */
    if (!tc_edge->edge_inv) {
#ifdef DEBUG
      OLSR_PRINTF(2, "SPF:   ignoring edge %s\n", olsr_ip_to_string(&buf, &tc_edge->T_dest_addr));
      if (!tc_edge->edge_inv) {
        OLSR_PRINTF(2, "SPF:     no inverse edge\n");
      }
#endif /* DEBUG */
      continue;
    }

    if (tc_edge->cost >= LINK_COST_BROKEN) {
#ifdef DEBUG
      OLSR_PRINTF(2, "SPF:   ignore edge %s (broken)\n", olsr_ip_to_string(&buf, &tc_edge->T_dest_addr));
#endif /* DEBUG */
      continue;
    }
    /*
     * total quality of the path through this vertex
     * to the destination of this edge
     */
    new_cost = tc->path_cost + tc_edge->cost;

#ifdef DEBUG
    OLSR_PRINTF(2, "SPF:   exploring edge %s, cost %s\n", olsr_ip_to_string(&buf, &tc_edge->T_dest_addr),
                get_linkcost_text(new_cost, true, &lqbuffer));
#endif /* DEBUG */

    /*
     * if it's better than the current path quality of this edge's
     * destination node, then we've found a better path to this node.
     */
    new_tc = tc_edge->edge_inv->tc;

    if (new_cost < new_tc->path_cost) {

      /* if this node has been on the candidate tree delete it */
      if (new_tc->path_cost < ROUTE_COST_BROKEN) {
        olsr_spf_del_cand_tree(cand_tree, new_tc);
      }

      /* re-insert on candidate tree with the better metric */
      new_tc->path_cost = new_cost;
      olsr_spf_add_cand_tree(cand_tree, new_tc);

      /* pull-up the next-hop and bump the hop count */
      if (tc->next_hop) {
        new_tc->next_hop = tc->next_hop;
      }
      new_tc->hops = tc->hops + 1;

#ifdef DEBUG
      OLSR_PRINTF(2, "SPF:   better path to %s, cost %s, via %s, hops %u\n", olsr_ip_to_string(&buf, &new_tc->addr),
                  get_linkcost_text(new_cost, true, &lqbuffer), tc->next_hop ? olsr_ip_to_string(&nbuf,
                                                                                                 &tc->next_hop->neighbor_iface_addr)
                  : "<none>", new_tc->hops);
#endif /* DEBUG */

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
olsr_spf_run_full(struct avl_tree *cand_tree, struct list_node *path_list, int *path_count)
{
  struct tc_entry *tc;

  *path_count = 0;

  while ((tc = olsr_spf_extract_best(cand_tree))) {

    olsr_spf_relax(cand_tree, tc);

    /*
     * move the best path from the candidate tree
     * to the path list.
     */
    olsr_spf_del_cand_tree(cand_tree, tc);
    olsr_spf_add_path_list(path_list, path_count, tc);
  }
}

/**
 * Callback for the SPF backoff timer.
 */
static void
olsr_expire_spf_backoff(void *context __attribute__ ((unused)))
{
  spf_backoff_timer = NULL;
}

#ifdef SPF_PROFILING
static void timer_sub(struct timespec * end, struct timespec * start, struct timespec * t) {
  t->tv_sec = end->tv_sec - start->tv_sec;
  t->tv_nsec = end->tv_nsec - start->tv_nsec;

  if (t->tv_nsec < 0) {
    t->tv_sec--;
    t->tv_nsec += 1000000000;
  }
}
#endif /* SPF_PROFILING */

void
olsr_calculate_routing_table(bool force)
{
#ifdef SPF_PROFILING
  struct timespec t1, t2, t3, t4, t5, spf_init, spf_run, route, kernel, total;
#endif /* SPF_PROFILING */
  struct avl_tree cand_tree;
  struct avl_node *rtp_tree_node;
  struct list_node path_list;          /* head of the path_list */
  struct tc_entry *tc;
  struct rt_path *rtp;
  struct tc_edge_entry *tc_edge;
  struct neighbor_entry *neigh;
  struct link_entry *link;
  int path_count = 0;

  /* We are done if our backoff timer is running */
  if (!force) {
    if (spf_backoff_timer) {
      return;
    }

    /* start new backoff timer */
    spf_backoff_timer = olsr_start_timer(1000, 5, OLSR_TIMER_ONESHOT, &olsr_expire_spf_backoff, NULL, 0);
  }

#ifdef SPF_PROFILING
  clock_gettime(CLOCK_MONOTONIC, &t1);
#endif /* SPF_PROFILING */

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
    tc->path_cost = ROUTE_COST_BROKEN;
    tc->hops = 0;
  }
  OLSR_FOR_ALL_TC_ENTRIES_END(tc);

  /*
   * Check if there was a change in the main IP address.
   * Bail if there is no main IP address.
   */
  olsr_change_myself_tc();
  if (!tc_myself) {

    /*
     * All gone now. Flush all routes.
     */
    olsr_update_rib_routes();
    olsr_update_kernel_routes();
    return;
  }

  /*
   * zero ourselves and add us to the candidate tree.
   */
  tc_myself->path_cost = ZERO_ROUTE_COST;
  olsr_spf_add_cand_tree(&cand_tree, tc_myself);

  /*
   * add edges to and from our neighbours.
   */
  OLSR_FOR_ALL_NBR_ENTRIES(neigh) {

    if (neigh->status != SYM) {
      tc_edge = olsr_lookup_tc_edge(tc_myself, &neigh->neighbor_main_addr);
      if (tc_edge) {
        olsr_delete_tc_edge_entry(tc_edge);
      }
    }
    else {
      tc_edge = olsr_lookup_tc_edge(tc_myself, &neigh->neighbor_main_addr);
      link = get_best_link_to_neighbor(&neigh->neighbor_main_addr);
      if (!link || lookup_link_status(link) == LOST_LINK) {

        /*
         * If there is no best link to this neighbor
         * and we had an edge before then flush the edge.
         */
        if (tc_edge) {
          olsr_delete_tc_edge_entry(tc_edge);
        }
        continue;
      }

      /* find the interface for the link */
      if (link->if_name) {
        link->inter = if_ifwithname(link->if_name);
      } else {
        link->inter = if_ifwithaddr(&link->local_iface_addr);
      }

      /*
       * Set the next-hops of our neighbors.
       */
      if (!tc_edge) {
        tc_edge = olsr_add_tc_edge_entry(tc_myself, &neigh->neighbor_main_addr, 0);
      } else {

        /*
         * Update LQ and timers, such that the edge does not get deleted.
         */
        olsr_copylq_link_entry_2_tc_edge_entry(tc_edge, link);
        olsr_calc_tc_edge_entry_etx(tc_edge);
      }
      if (tc_edge->edge_inv) {
        tc_edge->edge_inv->tc->next_hop = link;
      }
    }
  }
  OLSR_FOR_ALL_NBR_ENTRIES_END(neigh);

#ifdef SPF_PROFILING
  clock_gettime(CLOCK_MONOTONIC, &t2);
#endif /* SPF_PROFILING */

  /*
   * Run the SPF calculation.
   */
  olsr_spf_run_full(&cand_tree, &path_list, &path_count);

  OLSR_PRINTF(2, "\n--- %s ------------------------------------------------- DIJKSTRA\n\n", olsr_wallclock_string());

#ifdef SPF_PROFILING
  clock_gettime(CLOCK_MONOTONIC, &t3);
#endif /* SPF_PROFILING */

  /*
   * In the path list we have all the reachable nodes in our topology.
   */
  for (; !list_is_empty(&path_list); list_remove(path_list.next)) {

    tc = pathlist2tc(path_list.next);
    link = tc->next_hop;

    if (!link) {
#ifdef DEBUG
      /*
       * Supress the error msg when our own tc_entry
       * does not contain a next-hop.
       */
      if (tc != tc_myself) {
        struct ipaddr_str buf;
        OLSR_PRINTF(2, "SPF: %s no next-hop\n", olsr_ip_to_string(&buf, &tc->addr));
      }
#endif /* DEBUG */
      continue;
    }

    /*
     * Now walk all prefixes advertised by that node.
     * Since the node is reachable, insert the prefix into the global RIB.
     * If the prefix is already in the RIB, refresh the entry such
     * that olsr_delete_outdated_routes() does not purge it off.
     */
    for (rtp_tree_node = avl_walk_first(&tc->prefix_tree); rtp_tree_node; rtp_tree_node = avl_walk_next(rtp_tree_node)) {

      rtp = rtp_prefix_tree2rtp(rtp_tree_node);

      if (rtp->rtp_rt) {

        /*
         * If there is a route entry, the prefix is already in the global RIB.
         */
        olsr_update_rt_path(rtp, tc, link);

      } else {

        /*
         * The prefix is reachable and not yet in the global RIB.
         * Build a rt_entry for it.
         */
        olsr_insert_rt_path(rtp, tc, link);
      }
    }
  }
#ifdef __linux__
  /* check gateway tunnels */
  olsr_trigger_gatewayloss_check();
#endif /* __linux__ */

  /* Update the RIB based on the new SPF results */

  olsr_update_rib_routes();

#ifdef SPF_PROFILING
  clock_gettime(CLOCK_MONOTONIC, &t4);
#endif /* SPF_PROFILING */

  /* move the route changes into the kernel */

  olsr_update_kernel_routes();

#ifdef SPF_PROFILING
  clock_gettime(CLOCK_MONOTONIC, &t5);
#endif /* SPF_PROFILING */

#ifdef SPF_PROFILING
  timer_sub(&t2, &t1, &spf_init);
  timer_sub(&t3, &t2, &spf_run);
  timer_sub(&t4, &t3, &route);
  timer_sub(&t5, &t4, &kernel);
  timer_sub(&t5, &t1, &total);
  OLSR_PRINTF(1, "\n--- SPF-stats for %d nodes, %d routes (total/init/run/route/kern): %ld, %ld, %ld, %ld, %ld (nsec)\n", //
      path_count, //
      routingtree.count, //
      (long int) total.tv_nsec, //
      (long int) spf_init.tv_nsec, //
      (long int) spf_run.tv_nsec, //
      (long int) route.tv_nsec, //
      (long int) kernel.tv_nsec);
#endif /* SPF_PROFILING */
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
