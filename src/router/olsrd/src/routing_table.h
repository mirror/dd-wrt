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
 * $Id: routing_table.h,v 1.23 2007/10/16 09:54:43 bernd67 Exp $
 */

#ifndef _OLSR_ROUTING_TABLE
#define _OLSR_ROUTING_TABLE

#include <net/route.h>
#include "hna_set.h"
#include "lq_avl.h"
#include "lq_list.h"

#define NETMASK_HOST 0xffffffff
#define NETMASK_DEFAULT 0x0

/*
 * the kernel FIB does not need to know the metric of a route.
 * this saves us from enqueuing/dequeueing hopcount only changes.
 */
#define RT_METRIC_DEFAULT 2

/* a composite metric is used for path selection */
struct rt_metric
{
  float                 etx;
  olsr_u32_t 	        hops;
};

/* a nexthop is a pointer to a gateway router plus an interface */
struct rt_nexthop
{
  union olsr_ip_addr    gateway; /* gateway router */
  int                   iif_index; /* outgoing interface index */
};

/*
 * Every prefix in our RIB needs a route entry that contains
 * the nexthop of the best path as installed in the kernel FIB.
 * The route entry is the root of a rt_path tree of equal prefixes
 * originated by different routers. It also contains a shortcut
 * for accessing the best route among all contributing routes.
 */
struct rt_entry
{
  struct olsr_ip_prefix rt_dst;
  struct avl_node       rt_tree_node; 
  struct rt_path        *rt_best; /* shortcut to the best path */
  struct rt_nexthop     rt_nexthop; /* nexthop of FIB route */
  struct avl_tree       rt_path_tree;
  struct list_node      rt_change_node; /* queue for kernel FIB add/chg/del */
};

/*
 * For every received route a rt_path is added to the RIB.
 * Depending on the results of the SPF calculation we perform a
 * best_path calculation and pick the one with the lowest etx/metric.
 */
struct rt_path
{
  struct rt_entry       *rtp_rt; /* backpointer to owning route head */
  struct rt_nexthop     rtp_nexthop;
  struct rt_metric      rtp_metric;
  union olsr_ip_addr    rtp_originator; /* originator of the route */
  struct avl_node       rtp_tree_node; 
  olsr_u32_t            rtp_version;
};

/*
 * macro for traversing the entire routing table.
 * it is recommended to use this because it hides all the internal
 * datastructure from the callers.
 *
 * the loop prefetches the next node in order to not loose context if
 * for example the caller wants to delete the current rt_entry.
 */
#define OLSR_FOR_ALL_RT_ENTRIES(rt) \
{ \
  struct avl_node *rt_tree_node, *next_rt_tree_node; \
  for (rt_tree_node = avl_walk_first(&routingtree); \
    rt_tree_node; rt_tree_node = next_rt_tree_node) { \
    next_rt_tree_node = avl_walk_next(rt_tree_node); \
    rt = rt_tree_node->data;
#define OLSR_FOR_ALL_RT_ENTRIES_END(rt) }}

/**
 * IPv4 <-> IPv6 wrapper
 */
union olsr_kernel_route
{
  struct
  {
    struct sockaddr rt_dst;
    struct sockaddr rt_gateway;
    olsr_u32_t metric;
  } v4;

  struct
  {
    struct in6_addr rtmsg_dst;
    struct in6_addr rtmsg_gateway;
    olsr_u32_t rtmsg_metric;
  } v6;
};


extern struct avl_tree routingtree;
extern unsigned int routingtree_version;

int
olsr_init_routing_table(void);

unsigned int olsr_bump_routingtree_version(void);

int avl_comp_ipv4_prefix (void *, void *);
int avl_comp_ipv6_prefix (void *, void *);

void olsr_rt_best(struct rt_entry *);
olsr_bool olsr_nh_change(struct rt_nexthop *, struct rt_nexthop *);
olsr_bool olsr_cmp_rt(struct rt_entry *, struct rt_entry *);

void olsr_calculate_hna_routes(void);
char *olsr_rt_to_string(struct rt_entry *);
char *olsr_rtp_to_string(struct rt_path *);
void olsr_print_routing_table(struct avl_tree *);

const struct rt_nexthop * olsr_get_nh(const struct rt_entry *);

struct rt_path *
olsr_insert_routing_table(union olsr_ip_addr *, int,
                          union olsr_ip_addr *,
                          union olsr_ip_addr *,
                          int, int, float);

struct rt_entry *
olsr_lookup_routing_table(const union olsr_ip_addr *);


#endif

/*
 * Local Variables:
 * c-basic-offset: 2
 * End:
 */
