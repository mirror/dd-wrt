/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Thomas Lopatic (thomas@lopatic.de)
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
 * $Id: lq_route.c,v 1.40 2005/11/29 18:37:58 kattemat Exp $
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

struct dijk_edge
{
  struct avl_node tree_node;
  struct list_node node;
  struct dijk_vertex *dest;
  float etx;
};

struct dijk_vertex
{
  struct avl_node tree_node;
  struct list_node node;
  union olsr_ip_addr addr;
  struct avl_tree edge_tree;
  struct list edge_list;
  float path_etx;
  struct dijk_vertex *prev;
  olsr_bool done;
};

static int avl_comp_ipv6(void *ip1, void *ip2)
{
  return memcmp(ip1, ip2, ipsize);
}

static int avl_comp_ipv4(void *ip1, void *ip2)
{
  if (*(unsigned int *)ip1 < *(unsigned int *)ip2)
    return -1;

  if (*(unsigned int *)ip1 == *(unsigned int *)ip2)
    return 0;

  return 1;
}

static int (*avl_comp)(void *, void *);

static void add_vertex(struct avl_tree *vertex_tree, union olsr_ip_addr *addr,
                       float path_etx)
{
  struct avl_node *node;
  struct dijk_vertex *vert;

  // see whether this destination is already in our list

  node = avl_find(vertex_tree, addr);

  // if it's not in our list, add it

  if (node == NULL)
  {
    vert = olsr_malloc(sizeof (struct dijk_vertex), "Dijkstra vertex");

    vert->tree_node.data = vert;
    vert->tree_node.key = &vert->addr;

    COPY_IP(&vert->addr, addr);
    
    vert->path_etx = path_etx;
    vert->prev = NULL;
    vert->done = OLSR_FALSE;

    avl_init(&vert->edge_tree, avl_comp);
    list_init(&vert->edge_list);

    avl_insert(vertex_tree, &vert->tree_node);
  }
}

static void add_edge(struct avl_tree *vertex_tree,
                     union olsr_ip_addr *src, union olsr_ip_addr *dst,
                     float etx)
{
  struct avl_node *node;
  struct dijk_vertex *svert;
  struct dijk_vertex *dvert;
  struct dijk_edge *edge;

  // source lookup

  node = avl_find(vertex_tree, src);

  // source vertex does not exist

  if (node == NULL)
    return;

  svert = node->data;

  // destination lookup

  node = avl_find(vertex_tree, dst);

  // destination vertex does not exist

  if (node == NULL)
    return;

  dvert = node->data;

  // check for existing forward edge

  if (avl_find(&svert->edge_tree, dst) == NULL)
  {
    // add forward edge

    edge = olsr_malloc(sizeof (struct dijk_vertex), "Dijkstra edge 1");

    edge->tree_node.data = edge;
    edge->tree_node.key = &dvert->addr;

    edge->node.data = edge;

    edge->dest = dvert;
    edge->etx = etx;

    avl_insert(&svert->edge_tree, &edge->tree_node);
    list_add_tail(&svert->edge_list, &edge->node);
  }

  // check for existing inverse edge

  if (avl_find(&dvert->edge_tree, src) == NULL)
  {
    // add inverse edge

    edge = olsr_malloc(sizeof (struct dijk_vertex), "Dijkstra edge 2");

    edge->tree_node.data = edge;
    edge->tree_node.key = &svert->addr;

    edge->node.data = edge;

    edge->dest = svert;
    edge->etx = etx;

    avl_insert(&dvert->edge_tree, &edge->tree_node);
    list_add_tail(&dvert->edge_list, &edge->node);
  }
}

static void create_vertex_list_rec(struct list *vertex_list,
                                   struct avl_node *node,
                                   int (*comp)(void *, void *))
{
  struct dijk_vertex *vert = node->data;

  if (node->left != NULL)
    create_vertex_list_rec(vertex_list, node->left, comp);

  // add the vertex to the list, if it's not us

  if ((*comp)(&main_addr, node->key) != 0)
  {
    vert->node.data = vert;
    list_add_tail(vertex_list, &vert->node);
  }

  if (node->right != NULL)
    create_vertex_list_rec(vertex_list, node->right, comp);
}

static void create_vertex_list(struct avl_tree *vertex_tree,
                               struct list *vertex_list)
{
  struct avl_node *node;
  struct dijk_vertex *vert;

  // make ourselves the first vertex in the list

  node = avl_find(vertex_tree, &main_addr);
  vert = node->data;

  vert->node.data = vert;
  list_add_tail(vertex_list, &vert->node);

  // add the remaining vertices ordered by their main address

  create_vertex_list_rec(vertex_list, vertex_tree->root, vertex_tree->comp);
}

static void free_everything(struct list *vertex_list)
{
  struct list_node *vnode, *enode;
  struct dijk_vertex *vert;
  struct dijk_edge *edge;

  vnode = list_get_head(vertex_list);

  // loop through all vertices

  while (vnode != NULL)
  {
    vert = vnode->data;

    enode = list_get_head(&vert->edge_list);

    // loop through all edges of the current vertex

    while (enode != NULL)
    {
      edge = enode->data;

      enode = list_get_next(enode);
      free(edge);
    }

    vnode = list_get_next(vnode);
    free(vert);
  }
}

// XXX - bad complexity

static struct dijk_vertex *extract_best(struct list *vertex_list)
{
  float best_etx = INFINITE_ETX + 1.0;
  struct list_node *node;
  struct dijk_vertex *vert;
  struct dijk_vertex *res = NULL;

  node = list_get_head(vertex_list);

  // loop through all vertices
  
  while (node != NULL)
  {
    vert = node->data;

    // see whether the current vertex is better than what we have

    if (!vert->done && vert->path_etx < best_etx)
    {
      best_etx = vert->path_etx;
      res = vert;
    }

    node = list_get_next(node);
  }

  // if we've found a vertex, remove it from the set

  if (res != NULL)
    res->done = OLSR_TRUE;

  return res;
}

static void relax(struct dijk_vertex *vert)
{
  struct dijk_edge *edge;
  float new_etx;
  struct list_node *node;

  node = list_get_head(&vert->edge_list);

  // loop through all edges of this vertex

  while (node != NULL)
  {
    edge = node->data;

    // total quality of the path through this vertex to the
    // destination of this edge

    new_etx = vert->path_etx + edge->etx;

    // if it's better than the current path quality of this
    // edge's destination, then we've found a better path to
    // this destination

    if (new_etx < edge->dest->path_etx)
    {
      edge->dest->path_etx = new_etx;
      edge->dest->prev = vert;
    }

    node = list_get_next(node);
  }
}

static char *etx_to_string(float etx)
{
  static char buff[20];

  if (etx == INFINITE_ETX)
    return "INF";

  snprintf(buff, 20, "%.2f", etx);
  return buff;
}

void olsr_calculate_lq_routing_table(void)
{
  struct avl_tree vertex_tree;
  struct list vertex_list;
  int i;
  struct tc_entry *tcsrc;
  struct topo_dst *tcdst;
  struct dijk_vertex *vert;
  struct link_entry *link;
  struct neighbor_entry *neigh;
  struct list_node *node;
  struct dijk_vertex *myself;
  struct dijk_vertex *walker;
  int hops;
  struct mid_address *mid_walker;
  float etx;
  struct hna_entry *hna_gw;
  struct hna_net *hna;
  struct rt_entry *gw_rt, *hna_rt, *head_rt;
  struct neighbor_2_entry *neigh2;
#if 0
  struct neighbor_list_entry *neigh_walker;
#endif
  struct interface *inter;

  if (ipsize == 4)
    avl_comp = avl_comp_ipv4;

  else
    avl_comp = avl_comp_ipv6;

  // initialize the graph

  avl_init(&vertex_tree, avl_comp);
  list_init(&vertex_list);

  // add ourselves to the vertex tree

  add_vertex(&vertex_tree, &main_addr, 0.0);

  // add our neighbours

  for (i = 0; i < HASHSIZE; i++)
    for (neigh = neighbortable[i].next; neigh != &neighbortable[i];
         neigh = neigh->next)
      if (neigh->status == SYM)
        add_vertex(&vertex_tree, &neigh->neighbor_main_addr, INFINITE_ETX);

  // add our two-hop neighbours

  for (i = 0; i < HASHSIZE; i++)
    for (neigh2 = two_hop_neighbortable[i].next;
         neigh2 != &two_hop_neighbortable[i];
         neigh2 = neigh2->next)
      add_vertex(&vertex_tree, &neigh2->neighbor_2_addr, INFINITE_ETX);

  // add remaining vertices

  for (i = 0; i < HASHSIZE; i++)
    for (tcsrc = tc_table[i].next; tcsrc != &tc_table[i]; tcsrc = tcsrc->next)
    {
      // add source

      add_vertex(&vertex_tree, &tcsrc->T_last_addr, INFINITE_ETX);

      // add destinations of this source

      for (tcdst = tcsrc->destinations.next; tcdst != &tcsrc->destinations;
           tcdst = tcdst->next)
        add_vertex(&vertex_tree, &tcdst->T_dest_addr, INFINITE_ETX);
    }

  // add edges to and from our neighbours

  for (i = 0; i < HASHSIZE; i++)
    for (neigh = neighbortable[i].next; neigh != &neighbortable[i];
         neigh = neigh->next)
      if (neigh->status == SYM)
      {
        link = get_best_link_to_neighbor(&neigh->neighbor_main_addr);

	if(!link)
	  continue;

        if (link->loss_link_quality2 >= MIN_LINK_QUALITY &&
            link->neigh_link_quality2 >= MIN_LINK_QUALITY)
          {
            etx = 1.0 / (link->loss_link_quality2 * link->neigh_link_quality2);

            add_edge(&vertex_tree, &neigh->neighbor_main_addr, &main_addr, etx);
          }
      }

// we now rely solely on TC messages for routes to our two-hop neighbours

#if 0

  // add edges between our neighbours and our two-hop neighbours

  for (i = 0; i < HASHSIZE; i++)
    for (neigh2 = two_hop_neighbortable[i].next;
         neigh2 != &two_hop_neighbortable[i];
         neigh2 = neigh2->next)
      for (neigh_walker = neigh2->neighbor_2_nblist.next;
           neigh_walker != &neigh2->neighbor_2_nblist;
           neigh_walker = neigh_walker->next)
      {
        if (neigh_walker->second_hop_link_quality >=
            MIN_LINK_QUALITY * MIN_LINK_QUALITY)
        {
          neigh = neigh_walker->neighbor;

          etx = 1.0 / neigh_walker->second_hop_link_quality;

          add_edge(&vertex_tree, &neigh2->neighbor_2_addr,
                   &neigh->neighbor_main_addr, etx);
        }
      }

#endif

  // add remaining edges

  for (i = 0; i < HASHSIZE; i++)
    for (tcsrc = tc_table[i].next; tcsrc != &tc_table[i]; tcsrc = tcsrc->next)
      for (tcdst = tcsrc->destinations.next; tcdst != &tcsrc->destinations;
           tcdst = tcdst->next)
      {
        if (tcdst->link_quality >= MIN_LINK_QUALITY &&
            tcdst->inverse_link_quality >= MIN_LINK_QUALITY)
          {
            etx = 1.0 / (tcdst->link_quality * tcdst->inverse_link_quality);

            add_edge(&vertex_tree, &tcdst->T_dest_addr, &tcsrc->T_last_addr,
                     etx);
          }
      }

  // create a sorted vertex list from the vertex tree

  create_vertex_list(&vertex_tree, &vertex_list);

  // run Dijkstra's algorithm

  for (;;)
  {
    vert = extract_best(&vertex_list);

    if (vert == NULL)
      break;

    relax(vert);
  }

  // save the old routing table

  olsr_move_route_table(routingtable, old_routes);

  node = list_get_head(&vertex_list);

  // we're the first vertex in the list
  
  myself = node->data;

  OLSR_PRINTF(2, "\n--- %02d:%02d:%02d.%02d ------------------------------------------------- DIJKSTRA\n\n",
              nowtm->tm_hour,
              nowtm->tm_min,
              nowtm->tm_sec,
              (int)now.tv_usec/10000)

  for (node = list_get_next(node); node != NULL; node = list_get_next(node))
  {
    vert = node->data;

    hops = 1;

    // count hops to until the path ends or until we have reached a
    // one-hop neighbour

    for (walker = vert; walker != NULL && walker->prev != myself;
         walker = walker->prev)
    {
      OLSR_PRINTF(2, "%s:%s <- ", olsr_ip_to_string(&walker->addr),
                  etx_to_string(walker->path_etx))
      hops++;
    }

    // if no path to a one-hop neighbour was found, ignore this node

    if (walker != NULL)
    {
      OLSR_PRINTF(2, "%s:%s (one-hop)\n", olsr_ip_to_string(&walker->addr),
                  etx_to_string(walker->path_etx))

      // node reachable => add to the set of unprocessed nodes
      // for HNA processing

      vert->done = OLSR_FALSE;
    }

    else
    {
      OLSR_PRINTF(2, "%s FAILED\n", olsr_ip_to_string(&vert->addr))
      continue;
    }

    // find the best link to the one-hop neighbour

    link = get_best_link_to_neighbor(&walker->addr);

    // we may see NULL here, if the one-hop neighbour is not in the
    // link and neighbour sets any longer, but we have derived an edge
    // between us and the one-hop neighbour from the TC set

    if (link != NULL)
    {
      // find the interface for the found link

      inter = if_ifwithaddr(&link->local_iface_addr);

      // we may see NULL here if the interface is down, but we have
      // links that haven't timed out, yet

      if (inter != NULL)
      {
        // XXX - fix me: structurally prevent duplicates, don't test via
        //       olsr_lookup_routing_table()

        // route addition, case A - add a route to the main address of the
        // destination node

        if (olsr_lookup_routing_table(&vert->addr) == NULL)
          olsr_insert_routing_table(&vert->addr, &link->neighbor_iface_addr,
                                    inter, hops, vert->path_etx);

        // route addition, case B - add routes to the remaining interfaces
        // of the destination node

        for (mid_walker = mid_lookup_aliases(&vert->addr); mid_walker != NULL;
             mid_walker = mid_walker->next_alias)
          if (olsr_lookup_routing_table(&mid_walker->alias) == NULL)
            olsr_insert_routing_table(&mid_walker->alias,
                                      &link->neighbor_iface_addr, inter, hops, 
                                      vert->path_etx);

        // XXX - we used to use olsr_lookup_routing_table() only here, but
        //       this assumed that case A or case B had already happened for
        //       this destination; if case A or case B happened after case C
        //       for the same destination, we had duplicates, as cases A and
        //       B would not test whether case C had already happened

        // route addition, case C - make sure that we have a route to the
        // router - e.g. in case the router's not the main address and it's
        // MID entry has timed out

        if (olsr_lookup_routing_table(&link->neighbor_iface_addr) == NULL)
          olsr_insert_routing_table(&link->neighbor_iface_addr,
                                    &link->neighbor_iface_addr, inter, 1,
                                    vert->path_etx);
      }
    }
  }

  // save the old HNA routing table

  olsr_move_route_table(hna_routes, old_hna);

  // add HNA routes - the set of unprocessed network nodes contains
  // all reachable network nodes

  for (;;)
  {
    // extract the network node with the best ETX and remove it
    // from the set of unprocessed network nodes

    vert = extract_best(&vertex_list);

    // no more nodes left

    if (vert == NULL)
      break;

    // find the node's HNAs

    hna_gw = olsr_lookup_hna_gw(&vert->addr);

    // node doesn't announce any HNAs

    if (hna_gw == NULL)
      continue;

    // find route to the node

    gw_rt = olsr_lookup_routing_table(&hna_gw->A_gateway_addr);

    // maybe we haven't found a link or an interface for the gateway above
    // and hence haven't added a route - skip the HNA in this case

    if (gw_rt == NULL)
      continue;

    // loop through the node's HNAs

    for (hna = hna_gw->networks.next; hna != &hna_gw->networks;
         hna = hna->next)
    {
      // we already have a route via a previous (better) node

      if (olsr_lookup_hna_routing_table(&hna->A_network_addr) != NULL)
        continue;

      // create route for the HNA

      hna_rt = olsr_malloc(sizeof(struct rt_entry), "LQ HNA route entry");

      // set HNA IP address and netmask

      COPY_IP(&hna_rt->rt_dst, &hna->A_network_addr);
      hna_rt->rt_mask = hna->A_netmask;

      // copy remaining fields from the node's route

      COPY_IP(&hna_rt->rt_router, &gw_rt->rt_router);
      hna_rt->rt_metric = gw_rt->rt_metric;
      hna_rt->rt_etx = gw_rt->rt_etx;
      hna_rt->rt_if = gw_rt->rt_if;

      // we're not a host route

      hna_rt->rt_flags = RTF_UP | RTF_GATEWAY;

      // find the correct list

      head_rt = &hna_routes[olsr_hashing(&hna->A_network_addr)];

      // enqueue

      head_rt->next->prev = hna_rt;
      hna_rt->next = head_rt->next;

      head_rt->next = hna_rt;
      hna_rt->prev = head_rt;
    }
  }

  // free the graph

  free_everything(&vertex_list);

  // move the route changes into the kernel

  olsr_update_kernel_routes();
  olsr_update_kernel_hna_routes();

  // free the saved routing table

  olsr_free_routing_table(old_routes);
  olsr_free_routing_table(old_hna);
}
