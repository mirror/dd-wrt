/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
 * RIB implementation (c) 2007, Hannes Gredler (hannes@gredler.at)
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
 * $Id: routing_table.c,v 1.32 2007/10/16 09:54:43 bernd67 Exp $
 */

#include "defs.h"
#include "two_hop_neighbor_table.h"
#include "tc_set.h"
#include "mid_set.h"
#include "neighbor_table.h"
#include "olsr.h"
#include "link_set.h"
#include "routing_table.h"
#include "lq_avl.h"
#include "lq_route.h"
#include "assert.h"

struct avl_tree routingtree;
unsigned int routingtree_version;

/**
 * Bump the version number of the routing tree.
 *
 * After route-insertion compare the version number of the routes
 * against the version number of the table.
 * This is a lightweight detection if a node or prefix went away,
 * rather than brute force old vs. new rt_entry comparision.
 */
unsigned int
olsr_bump_routingtree_version(void)
{
  return(routingtree_version++);
}

/**
 * avl_comp_ipv4_prefix
 *
 * compare two ipv4 prefixes.
 * first compare the prefixes, then
 *  then compare the prefix lengths.
 *
 * return 0 if there is an exact match and
 * -1 / +1 depending on being smaller or bigger.
 */
int
avl_comp_ipv4_prefix (void *prefix1, void *prefix2)
{       
  struct olsr_ip_prefix *pfx1, *pfx2;

  pfx1 = prefix1;
  pfx2 = prefix2;

  /* prefix */
  if (pfx1->prefix.v4 < pfx2->prefix.v4) {
    return -1;
  }
  if (pfx1->prefix.v4 > pfx2->prefix.v4) {
    return +1;
  }

  /* prefix length */
  if (pfx1->prefix_len < pfx2->prefix_len) {
    return -1;
  }
  if (pfx1->prefix_len > pfx2->prefix_len) {
    return +1;
  }

  return 0;
}

/**
 * avl_comp_ipv6_prefix
 *
 * compare two ipv6 prefixes.
 * first compare the prefixes, then
 *  then compare the prefix lengths.
 *
 * return 0 if there is an exact match and
 * -1 / +1 depending on being smaller or bigger.
 */
int
avl_comp_ipv6_prefix (void *prefix1, void *prefix2)
{       
  struct olsr_ip_prefix *pfx1, *pfx2;
  int res;

  pfx1 = prefix1;
  pfx2 = prefix2;

  /* prefix */
  res = memcmp(&pfx1->prefix.v6, &pfx2->prefix.v6, 16);
  if (res != 0) {
    return res;
  } 
  /* prefix length */
  if (pfx1->prefix_len < pfx2->prefix_len) {
    return -1;
  }
  if (pfx1->prefix_len > pfx2->prefix_len) {
    return +1;
  }

  return 0;
}

/**
 * Initialize the routingtree and kernel change queues.
 */
int
olsr_init_routing_table(void)
{
  /* the routing tree */
  avl_init(&routingtree, avl_comp_prefix_default);
  routingtree_version = 0;

  /* the add/chg/del kernel queues */
  list_head_init(&add_kernel_list);
  list_head_init(&chg_kernel_list);
  list_head_init(&del_kernel_list);

  return 1;
}

/**
 * Look up a maxplen entry (= /32 or /128) in the routing table.
 *
 * @param dst the address of the entry
 *
 * @return a pointer to a rt_entry struct 
 * representing the route entry.
 */
struct rt_entry *
olsr_lookup_routing_table(const union olsr_ip_addr *dst)
{
  struct avl_node *rt_tree_node;
  struct olsr_ip_prefix prefix;

  COPY_IP(&prefix, dst);
  prefix.prefix_len = olsr_cnf->maxplen;

  rt_tree_node = avl_find(&routingtree, &prefix);

  return (rt_tree_node ? rt_tree_node->data : NULL);
}

/**
 * Update the fields in an routing entry.
 * Depending on the update mask update either the old,
 * the new or both arrays for gateway/interface/etx/hopcount.
 */
static void
olsr_update_routing_entry(struct rt_path *rtp, union olsr_ip_addr *gateway,
                          int iif_index, int metric, float etx)
{

  rtp->rtp_version = routingtree_version;

  /* gateway */
  rtp->rtp_nexthop.gateway = *gateway;

  /* interface */
  rtp->rtp_nexthop.iif_index = iif_index;

  /* etx */
  rtp->rtp_metric.hops = metric;
  if (etx < 0.0) {
    /* non-LQ case */
    rtp->rtp_metric.etx = (float)metric;
  } else {
    /* LQ case */
    rtp->rtp_metric.etx = etx;
  }
}

/**
 * Alloc and key a new rt_entry.
 */
static struct rt_entry *
olsr_alloc_rt_entry(struct olsr_ip_prefix *prefix)
{
  struct rt_entry *rt;

  rt = olsr_malloc(sizeof(struct rt_entry), __FUNCTION__);

  if (!rt) {
    return NULL;
  }

  memset(rt, 0, sizeof(struct rt_entry));
  
  /* Mark this entry as fresh (see process_routes.c:512) */
  rt->rt_nexthop.iif_index = -1;

  /* set key and backpointer prior to tree insertion */
  rt->rt_dst = *prefix;

  rt->rt_tree_node.key = &rt->rt_dst;
  rt->rt_tree_node.data = rt;
  avl_insert(&routingtree, &rt->rt_tree_node, AVL_DUP_NO);

  /* init the originator subtree */
  avl_init(&rt->rt_path_tree, avl_comp_default);

  return rt;
}


/**
 * Alloc and key a new rt_path.
 */
static struct rt_path *
olsr_alloc_rt_path(struct rt_entry *rt,
                   union olsr_ip_addr *originator)
{
  struct rt_path *rtp;

  rtp = olsr_malloc(sizeof(struct rt_path), __FUNCTION__);

  if (!rtp) {
    return NULL;
  }

  memset(rtp, 0, sizeof(struct rt_path));

  COPY_IP(&rtp->rtp_originator, originator);

  /* set key and backpointer prior to tree insertion */
  rtp->rtp_tree_node.key = &rtp->rtp_originator;
  rtp->rtp_tree_node.data = rtp;

  /* insert to the route entry originator tree */
  avl_insert(&rt->rt_path_tree, &rtp->rtp_tree_node, AVL_DUP_NO);

  /* backlink to the owning route entry */
  rtp->rtp_rt = rt;

  return rtp;
}

/**
 * Check if there is an interface or gateway change.
 */
olsr_bool
olsr_nh_change(struct rt_nexthop *nh1, struct rt_nexthop *nh2)
{
  if ((!COMP_IP(&nh1->gateway, &nh2->gateway)) ||
      (nh1->iif_index != nh2->iif_index)) {
    return OLSR_TRUE;
  }
  return OLSR_FALSE;
}

/**
 * depending on the operation (add/chg/del) the nexthop
 * field from the route entry or best route path shall be used.
 */
const struct rt_nexthop *
olsr_get_nh(const struct rt_entry *rt)
{

  if(rt->rt_best) {

    /* this is a route add/chg - grab nexthop from the best route. */
    return &rt->rt_best->rtp_nexthop;
  } 
  
  /* this is a route deletion - all routes are gone. */
  return &rt->rt_nexthop;
}

/**
 * compare two route paths.
 *
 * returns TRUE if the first path is better
 * than the second one, FALSE otherwise.
 */
static olsr_bool
olsr_cmp_rtp(struct rt_path *rtp1, struct rt_path *rtp2)
{
   /* etx comes first */
    if (rtp1->rtp_metric.etx < rtp2->rtp_metric.etx) {
      return OLSR_TRUE;
    }

    /* hopcount is next tie breaker */
    if ((rtp1->rtp_metric.etx == rtp2->rtp_metric.etx) &&
        (rtp1->rtp_metric.hops < rtp2->rtp_metric.hops)) {
      return OLSR_TRUE;
    }

    /* originator (which is guaranteed to be unique) is final tie breaker */
    if ((rtp1->rtp_metric.hops == rtp2->rtp_metric.hops) &&
        (memcmp(&rtp1->rtp_originator, &rtp2->rtp_originator,
                olsr_cnf->ipsize) == -1)) {
      return OLSR_TRUE;
    }

    return OLSR_FALSE;
}

/**
 * compare the best path of two route entries.
 *
 * returns TRUE if the first entry is better
 * than the second one, FALSE otherwise.
 */
olsr_bool
olsr_cmp_rt(struct rt_entry *rt1, struct rt_entry *rt2)
{
  return(olsr_cmp_rtp(rt1->rt_best, rt2->rt_best));
}

/**
 * run best route selection among a
 * set of identical prefixes.
 */
void
olsr_rt_best(struct rt_entry *rt)
{
  struct rt_path *rtp;
  struct avl_node *node;

  /* grab the first entry */
  node = avl_walk_first(&rt->rt_path_tree);

  if (!node) {
    assert(0); /* should not happen */
  }

  rt->rt_best = node->data;

  /* walk all remaining originator entries */
  while ((node = avl_walk_next(node))) {

    rtp = node->data;

    if (olsr_cmp_rtp(rtp, rt->rt_best)) {
      rt->rt_best = rtp;
    }
  }
}

/**
 * Insert/Update a route entry into the routing table.
 *
 * Check is the route exisits and depending if this is a
 * new version of the RIB do a full inplace update.
 * If there is already a route from this table version then 
 * check if the new route is better.
 *
 * For exisiting routes only interface or gateway router changes
 * do trigger a kernel change.
 *
 *@param dst the destination
 *@param plen the prefix length
 *@param gateway the next-hop router
 *@param iface the next-hop interface
 *@param metric the hopcount
 *@param etx the LQ extension metric
 *
 *@return the new rt_entry struct
 */
struct rt_path *
olsr_insert_routing_table(union olsr_ip_addr *dst, 
                          int plen,
                          union olsr_ip_addr *originator,
			  union olsr_ip_addr *gateway,
			  int iif_index,
			  int metric,
			  float etx)
{
  struct rt_entry *rt;
  struct rt_path *rtp;
  struct avl_node *node;
  struct olsr_ip_prefix prefix;

  /*
   * no unreachable routes please.
   */
  if (etx >= INFINITE_ETX) {
    return NULL;
  }

  /*
   * first check if there is a route_entry for the prefix.
   */
  prefix.prefix = *dst;
  prefix.prefix_len = plen;

  node = avl_find(&routingtree, &prefix);

  if (!node) {

    /* no route entry yet */
    rt = olsr_alloc_rt_entry(&prefix);

    if (!rt) {
      return NULL;
    }

  } else {
    rt = node->data;
  }

  /*
   * next check if the route path from this originator is known
   */
  node = avl_find(&rt->rt_path_tree, originator);

  if (!node) {

    /* no route path from this originator yet */
    rtp = olsr_alloc_rt_path(rt, originator);

    if (!rtp) {
      return NULL;
    }

  } else {
    rtp = node->data;
  }

  /* update the version field and relevant parameters */
  olsr_update_routing_entry(rtp, gateway, iif_index, metric, etx);

  return rtp;
}

/**
 * format a route entry into a buffer
 */
char *
olsr_rt_to_string(struct rt_entry *rt)
{
  static char buff[128];

  snprintf(buff, sizeof(buff),
           "%s/%u via %s",
           olsr_ip_to_string(&rt->rt_dst.prefix),
           rt->rt_dst.prefix_len,
           olsr_ip_to_string(&rt->rt_nexthop.gateway));

  return buff;
}

/**
 * format a route path into a buffer
 */
char *
olsr_rtp_to_string(struct rt_path *rtp)
{
  struct rt_entry *rt;
  static char buff[128];

  rt = rtp->rtp_rt;

  snprintf(buff, sizeof(buff),
           "%s/%u from %s via %s, "
           "etx %.3f, metric %u, v %u",
           olsr_ip_to_string(&rt->rt_dst.prefix),
           rt->rt_dst.prefix_len,
           olsr_ip_to_string(&rtp->rtp_originator),
           olsr_ip_to_string(&rtp->rtp_nexthop.gateway),
           rtp->rtp_metric.etx,
           rtp->rtp_metric.hops,
           rtp->rtp_version);

  return buff;
}

/**
 *Calculate the HNA routes
 *
 */
void
olsr_calculate_hna_routes(void)
{
  int index, plen;
  struct rt_entry *rt;

#ifdef DEBUG
  OLSR_PRINTF(3, "Calculating HNA routes\n");
#endif

  for(index=0;index<HASHSIZE;index++)
  {
    struct hna_entry *tmp_hna;
    /* All entries */
    for(tmp_hna = hna_set[index].next;
        tmp_hna != &hna_set[index];
        tmp_hna = tmp_hna->next)
    {
      struct hna_net *tmp_net;
      /* All networks */
      for(tmp_net = tmp_hna->networks.next;
          tmp_net != &tmp_hna->networks;
          tmp_net = tmp_net->next) {

        /* If no route to gateway - skip */
        if((rt = olsr_lookup_routing_table(&tmp_hna->A_gateway_addr)) == NULL)
          continue;

        /* update if better */
        plen = olsr_get_hna_prefix_len(tmp_net);
        olsr_insert_routing_table(&tmp_net->A_network_addr, plen,
                                  &tmp_hna->A_gateway_addr,
                                  &rt->rt_best->rtp_nexthop.gateway,
                                  rt->rt_best->rtp_nexthop.iif_index,
                                  rt->rt_best->rtp_metric.hops,
                                  rt->rt_best->rtp_metric.etx);

      }
    }
  }

  /* Update kernel */
  olsr_update_kernel_routes();
}

/**
 * Print the routingtree to STDOUT
 *
 */
void
olsr_print_routing_table(struct avl_tree *tree)
{
  struct rt_entry *rt;
  struct rt_path *rtp;

  struct avl_node *rt_tree_node, *rtp_tree_node;

  printf("ROUTING TABLE\n");

  for (rt_tree_node = avl_walk_first(tree);
       rt_tree_node;
       rt_tree_node = avl_walk_next(rt_tree_node)) {

    rt = rt_tree_node->data;

    /* first the route entry */
    printf("%s/%u, via %s, best-originator %s\n",
           olsr_ip_to_string(&rt->rt_dst.prefix),
           rt->rt_dst.prefix_len,
           olsr_ip_to_string(&rt->rt_nexthop.gateway),
           olsr_ip_to_string(&rt->rt_best->rtp_originator));

    /* walk the per-originator path tree of routes */
    for (rtp_tree_node = avl_walk_first(&rt->rt_path_tree);
         rtp_tree_node;
         rtp_tree_node = avl_walk_next(rtp_tree_node)) {

      rtp = rtp_tree_node->data;

      printf("\tfrom %s, etx %.3f, metric %u, via %s, %s, v %u\n",
             olsr_ip_to_string(&rtp->rtp_originator),
             rtp->rtp_metric.etx,
             rtp->rtp_metric.hops,
             olsr_ip_to_string(&rtp->rtp_nexthop.gateway),
             if_ifwithindex_name(rt->rt_nexthop.iif_index),
             rtp->rtp_version);
    
    }
  }
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * End:
 */
