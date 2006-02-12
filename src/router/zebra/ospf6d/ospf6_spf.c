/*
 * Copyright (C) 1999 Yasuhiro Ohara
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
 * Boston, MA 02111-1307, USA.  
 */
/* Shortest Path First calculation for OSPFv3 */

#include "ospf6d.h"

#include "linklist.h"
#include "prefix.h"
#include "table.h"

#include "ospf6_proto.h"
#include "ospf6_lsa.h"
#include "ospf6_lsdb.h"
#include "ospf6_route.h"
#include "ospf6_spf.h"
#include "ospf6_neighbor.h"
#include "ospf6_interface.h"
#include "ospf6_area.h"


/* new */
static int
ospf6_spf_lsd_num (struct ospf6_vertex *V, struct ospf6_area *o6a)
{
  u_int16_t type;
  u_int32_t id, adv_router;
  struct ospf6_lsa *lsa;
  struct ospf6_lsa_header *lsa_header;

  if (V->vertex_id.id.s_addr)
    type = htons (OSPF6_LSA_TYPE_NETWORK);
  else
    type = htons (OSPF6_LSA_TYPE_ROUTER);
  id = V->vertex_id.id.s_addr;
  adv_router = V->vertex_id.adv_router.s_addr;

  lsa = ospf6_lsdb_lookup_lsdb (type, id, adv_router, o6a->lsdb);
  if (! lsa)
    {
      zlog_err ("SPF: Can't find associated LSA for V for lsd num");
      return 0;
    }

  lsa_header = (struct ospf6_lsa_header *) lsa->lsa_hdr;
  return ospf6_lsa_lsd_num (lsa_header);
}

/* RFC2328 section 16.1.1 */
static int
ospf6_spf_is_router_to_root (struct ospf6_vertex *c,
                             struct ospf6_spftree *spf_tree)
{
  listnode node;
  struct ospf6_vertex *p;

  if (spf_tree->root == c)
    return 0;

  for (node = listhead (c->parent_list); node; nextnode (node))
    {
      p = (struct ospf6_vertex *) getdata (node);

      if (p == spf_tree->root)
        return 0;

      if (p->vertex_id.id.s_addr == 0) /* this is router */
        continue;
      else if (ospf6_spf_is_router_to_root (p, spf_tree))
        continue;

      return 0;
    }

  return 1;
}

static struct in6_addr *
ospf6_spf_get_ipaddr (u_int32_t ifindex, u_int32_t adv_router)
{
  char buf[64];
  struct ospf6_interface *o6i;
  struct ospf6_neighbor *o6n;

  o6i = ospf6_interface_lookup_by_index (ifindex);
  if (! o6i)
    {
      zlog_err ("SPF: Can't find interface: index %d", ifindex);
      return (struct in6_addr *) NULL;
    }

  o6n = ospf6_neighbor_lookup (adv_router, o6i);
  if (! o6n)
    {
      inet_ntop (AF_INET, &adv_router, buf, sizeof (buf));
      zlog_err ("SPF: Can't find neighbor %s in %s",
                buf, o6i->interface->name);
      return (struct in6_addr *) NULL;
    }

  return &o6n->hisaddr;
}

static int
ospf6_spf_nexthop_calculation (struct ospf6_vertex *W,
                               u_int32_t ifindex,
                               struct ospf6_vertex *V,
                               struct ospf6_spftree *spf_tree)
{
  listnode node;
  struct ospf6_nexthop *nexthop, *n;
  u_int32_t adv_router, id;
  struct in6_addr nexthop_ipaddr, *ipaddr;
  unsigned int nexthop_ifindex;
  char buf[64];

  /* until this, nexthop_list should be untouched */
  assert (list_isempty (W->nexthop_list));

  /* If ther is at least one intervening router from root to W */
  if (ospf6_spf_is_router_to_root (W, spf_tree))
    {
      if (IS_OSPF6_DUMP_SPF)
        zlog_info ("SPF: %s inherits %s's nexthop_list", W->string, V->string);

      for (node = listhead (V->nexthop_list); node; nextnode (node))
        {
          nexthop = (struct ospf6_nexthop *) getdata (node);
          listnode_add (W->nexthop_list, nexthop);
          nexthop->lock++;
        }

      return 0;
    }

  adv_router = W->vertex_id.adv_router.s_addr;
  id = W->vertex_id.id.s_addr;

  /* nexthop */
  nexthop_ifindex = 0;
  memset (&nexthop_ipaddr, 0, sizeof (struct in6_addr));
  if (spf_tree->root && V == spf_tree->root)
    {
      if (IS_OSPF6_DUMP_SPF)
        zlog_info ("SPF: %s is the first hop", W->string);

      nexthop_ifindex = ifindex;
      if (! id)
        {
          ipaddr = ospf6_spf_get_ipaddr (ifindex, adv_router);
          if (! ipaddr)
            {
              /* xxx, should trigger error and quit SPF calculation... */
              memset (&nexthop_ipaddr, 0xff, sizeof (struct in6_addr));
              return -1;
            }
          else
            memcpy (&nexthop_ipaddr, ipaddr, sizeof (struct in6_addr));
        }
    }
  else
    {
      /* V is broadcast network, W is router */
      assert (V->vertex_id.id.s_addr != 0);
      assert (W->vertex_id.id.s_addr == 0);

      /* there should be the only one nexthop in V */
      assert (listcount (V->nexthop_list) == 1);
      n = (struct ospf6_nexthop *) getdata (listhead (V->nexthop_list));
      nexthop_ifindex = n->ifindex;
      ipaddr = ospf6_spf_get_ipaddr (n->ifindex, adv_router);
      if (! ipaddr)
        {
          /* xxx, should trigger error and quit SPF calculation... */
          memset (&nexthop_ipaddr, 0xff, sizeof (struct in6_addr));
          return -1;
        }
      else
        memcpy (&nexthop_ipaddr, ipaddr, sizeof (struct in6_addr));
    }

  if (IS_OSPF6_DUMP_SPF)
    {
      inet_ntop (AF_INET6, &nexthop_ipaddr, buf, sizeof (buf));
      zlog_info ("SPF: %s: nexthop: %s ifindex: %d", W->string,
                 buf, nexthop_ifindex);
    }
  nexthop = ospf6_nexthop_create (nexthop_ifindex, &nexthop_ipaddr, 0);
  listnode_add (W->nexthop_list, nexthop);

  return 0;
}

static struct ospf6_vertex *
ospf6_spf_vertex_create (int index, struct ospf6_vertex *V,
                         struct ospf6_area *o6a)
{
  struct ospf6_lsa *vlsa, *wlsa;
  struct ospf6_lsa_header *vlsa_header, *wlsa_header;
  struct ospf6_router_lsa *router_lsa;
  struct ospf6_router_lsd *router_lsd;
  struct ospf6_network_lsa *network_lsa;
  struct ospf6_network_lsd *network_lsd;
  u_int32_t id, adv_router;
  u_int16_t type;
  void *lsd;
  struct ospf6_vertex *W;
  u_int16_t distance;
  u_int32_t ifindex;
  char buf[64];
  int backreference, lsdnum, i;

  /* Find V's associated LSA */
  id = V->vertex_id.id.s_addr;
  adv_router = V->vertex_id.adv_router.s_addr;
  if (id == ntohl (0))
    type = htons (OSPF6_LSA_TYPE_ROUTER);
  else
    type = htons (OSPF6_LSA_TYPE_NETWORK);

  vlsa = ospf6_lsdb_lookup_lsdb (type, id, adv_router, o6a->lsdb);

  if (! vlsa)
    {
      /* save the case that Router-LSA's ls id != 0 */
      if (type == htons (OSPF6_LSA_TYPE_ROUTER))
        {
          struct ospf6_lsa *lsa;
          struct ospf6_lsdb_node *node;

          for (node = ospf6_lsdb_head (o6a->lsdb); node;
               node = ospf6_lsdb_next (node))
            {
              lsa = node->lsa;
              if (lsa->header->type != htons (OSPF6_LSA_TYPE_ROUTER))
                continue;
              if (lsa->header->adv_router != adv_router)
                continue;
              vlsa = lsa;

              ospf6_lsdb_end (node);
              break;
            }
        }
      else
        {
          zlog_err ("SPF: LSA for W: not found");
          zlog_err ("SPF:   type: %#x, id: %d, adv_router: %s",
                    ntohs (type), ntohl (id),
                    inet_ntop (AF_INET, &adv_router, buf, sizeof (buf)));
          return (struct ospf6_vertex *) NULL;
        }

      /* still not found */
      if (vlsa == NULL)
        {
          char tbuf[256];
          inet_ntop (AF_INET6, &adv_router, tbuf, sizeof (tbuf));
          zlog_err ("SPF: Router-LSA of V(%s): not found", tbuf);
          return (struct ospf6_vertex *) NULL;
        }
    }

  if (ospf6_lsa_is_maxage (vlsa))
    {
      zlog_err ("SPF: Associated LSA for V is MaxAge: %s", V->string);
      return (struct ospf6_vertex *) NULL;
    }

  /* Get Linkstate description */
  vlsa_header = (struct ospf6_lsa_header *) vlsa->lsa_hdr;
  lsd = ospf6_lsa_lsd_get (index, vlsa_header);
  if (! lsd)
    {
      zlog_err ("SPF: Can't find Linkstate description from V");
      return (struct ospf6_vertex *) NULL;
    }

  /* Check Link state description */
  distance = 0;
  ifindex = 0;
  if (vlsa_header->type == ntohs (OSPF6_LSA_TYPE_ROUTER))
    {
      router_lsd = lsd;
      if (router_lsd->type == OSPF6_ROUTER_LSD_TYPE_POINTTOPOINT)
        {
          type = htons (OSPF6_LSA_TYPE_ROUTER);
          id = htonl (0);
        }
      else if (router_lsd->type == OSPF6_ROUTER_LSD_TYPE_TRANSIT_NETWORK)
        {
          type = htons (OSPF6_LSA_TYPE_NETWORK);
          id = router_lsd->neighbor_interface_id;
        }
      adv_router = router_lsd->neighbor_router_id;
      distance = ntohs (router_lsd->metric);
      ifindex = ntohl (router_lsd->interface_id);
    }
  else if (vlsa_header->type == ntohs (OSPF6_LSA_TYPE_NETWORK))
    {
      network_lsd = lsd;
      type = htons (OSPF6_LSA_TYPE_ROUTER);
      id = htonl (0);
      adv_router = network_lsd->adv_router;
    }

  /* Avoid creating candidate of myself */
  if (adv_router == o6a->ospf6->router_id &&
      type == htons (OSPF6_LSA_TYPE_ROUTER))
    {
      if (IS_OSPF6_DUMP_SPF)
        zlog_info ("SPF: Ignore link description to myself");
      return (struct ospf6_vertex *) NULL;
    }

  /* Find Associated LSA for W */
  wlsa = ospf6_lsdb_lookup_lsdb (type, id, adv_router, o6a->lsdb);

  if (! wlsa)
    {
      /* save the case that Router-LSA's ls id != 0 */
      if (type == htons (OSPF6_LSA_TYPE_ROUTER))
        {
          struct ospf6_lsa *lsa = NULL;
          struct ospf6_lsdb_node *node;
          for (node = ospf6_lsdb_head (o6a->lsdb); node;
               node = ospf6_lsdb_next (node))
            {
              lsa = node->lsa;
              if (lsa->header->type != htons (OSPF6_LSA_TYPE_ROUTER))
                continue;
              if (lsa->header->adv_router != adv_router)
                continue;
              wlsa = lsa;

              ospf6_lsdb_end (node);
              break;
            }
        }
      else
        {
          zlog_err ("SPF: LSA for W: not found");
          zlog_err ("SPF:   type: %#x, id: %d, adv_router: %s",
                    ntohs (type), ntohl (id),
                    inet_ntop (AF_INET, &adv_router, buf, sizeof (buf)));
          return (struct ospf6_vertex *) NULL;
        }

      /* still not found */
      if (wlsa == NULL)
        {
          char tbuf[256];
          inet_ntop (AF_INET6, &adv_router, tbuf, sizeof (tbuf));
          zlog_err ("SPF: Router-LSA of W(%s): not found", tbuf);
          return (struct ospf6_vertex *) NULL;
        }
    }
  if (ospf6_lsa_is_maxage (wlsa))
    {
      zlog_err ("SPF: Associated LSA for W is MaxAge: %s", wlsa->str);
      return (struct ospf6_vertex *) NULL;
    }

  /* Check back reference from wlsa to vlsa */
  backreference = 0;
  wlsa_header = (struct ospf6_lsa_header *) wlsa->lsa_hdr;
  lsdnum = ospf6_lsa_lsd_num (wlsa_header);
  for (i = 0; i < lsdnum; i++)
    {
      if (ospf6_lsa_lsd_is_refer_ok (i, wlsa_header, index, vlsa_header))
        backreference++;
    }
  if (! backreference)
    {
      zlog_err ("SPF: Back reference failed: V: %s, W: %s",
                vlsa->str, wlsa->str);
      return (struct ospf6_vertex *) NULL;
    }

  /* Allocate new ospf6_vertex for W */
  W = (struct ospf6_vertex *) XMALLOC (MTYPE_OSPF6_VERTEX,
                                       sizeof (struct ospf6_vertex));
  if (! W)
    {
      zlog_err ("SPF: Can't allocate memory for Vertex");
      return (struct ospf6_vertex *) NULL;
    }
  memset (W, 0, sizeof (struct ospf6_vertex));

  /* Initialize */
  W->vertex_id.family = AF_INET6;
  W->vertex_id.prefixlen = 64;
  if (type == htons (OSPF6_LSA_TYPE_ROUTER))
    W->vertex_id.id.s_addr = htonl (0); /* XXX */
  else
    W->vertex_id.id.s_addr = wlsa_header->ls_id;
  W->vertex_id.adv_router.s_addr = wlsa_header->advrtr;
  W->nexthop_list = list_new ();
  W->path_list = list_new ();
  W->parent_list = list_new ();
  W->distance = V->distance + distance;
  W->depth = V->depth + 1;
  inet_ntop (AF_INET, &W->vertex_id.adv_router.s_addr, buf, sizeof (buf));
  if (W->vertex_id.id.s_addr)
    snprintf (W->string, sizeof (W->string), "%s[%lu]",
              buf, (unsigned long) ntohl (W->vertex_id.id.s_addr));
  else
    snprintf (W->string, sizeof (W->string), "%s", buf);

  /* capability bits and optional capabilities */
  if (W->vertex_id.id.s_addr == 0)
    {
      router_lsa = (struct ospf6_router_lsa *) (wlsa_header + 1);
      W->capability_bits = router_lsa->bits;
      memcpy (W->opt_capability, router_lsa->options,
              sizeof (W->opt_capability));
    }
  else
    {
      network_lsa = (struct ospf6_network_lsa *) (wlsa_header + 1);
      W->capability_bits = network_lsa->reserved;
      memcpy (W->opt_capability, network_lsa->options,
              sizeof (W->opt_capability));
    }

  /* Parent node */
  listnode_add (W->parent_list, V);

  /* Nexthop Calculation */
  if (ospf6_spf_nexthop_calculation (W, ifindex, V, o6a->spf_tree) < 0)
    return NULL;

  return W;
}

static void
ospf6_spf_vertex_delete (struct ospf6_vertex *v)
{
  listnode node;
  struct ospf6_nexthop *nexthop;

  for (node = listhead (v->nexthop_list); node; nextnode (node))
    {
      nexthop = (struct ospf6_nexthop *) getdata (node);
      ospf6_nexthop_delete (nexthop);
    }
  list_delete (v->nexthop_list);

  list_delete (v->path_list);
  list_delete (v->parent_list);
  XFREE (MTYPE_OSPF6_VERTEX, v);
}

static void
ospf6_spf_vertex_merge (struct ospf6_vertex *w, struct ospf6_vertex *x)
{
  listnode node;
  struct ospf6_nexthop *nexthop;

  /* merge should be done on two nodes which are
     almost the same */

  /* these w and x should be both candidate.
     candidate should not have any children */
  assert (list_isempty (w->path_list));
  assert (list_isempty (x->path_list));

  /* merge parent list */
  for (node = listhead (w->parent_list); node; nextnode (node))
    {
      if (listnode_lookup (x->parent_list, getdata (node)))
        continue;
      listnode_add (x->parent_list, getdata (node));
    }

  /* merge nexthop list */
  for (node = listhead (w->nexthop_list); node; nextnode (node))
    {
      nexthop = (struct ospf6_nexthop *) getdata (node);
      if (listnode_lookup (x->nexthop_list, nexthop))
        continue;
      listnode_add (x->nexthop_list, nexthop);
      nexthop->lock++;
    }
}

static void
ospf6_spf_initialize (list candidate_list, struct ospf6_area *o6a)
{
  listnode node;
  struct ospf6_vertex *v;
  struct ospf6_lsa *lsa;
  u_int16_t type;
  u_int32_t id, adv_router;

  /* delete topology routing table for this area */
  ospf6_route_delete_all (o6a->table_topology);

  /* Delete previous spf tree */
  for (node = listhead (o6a->spf_tree->list); node; nextnode (node))
    {
      v = (struct ospf6_vertex *) getdata (node);
      ospf6_spf_vertex_delete (v);
    }
  list_delete_all_node (o6a->spf_tree->list);

  /* Find self originated Router-LSA */
  type = htons (OSPF6_LSA_TYPE_ROUTER);
  id = htonl (0);
  adv_router = ospf6->router_id;

  lsa = ospf6_lsdb_lookup_lsdb (type, id, adv_router, o6a->lsdb);

  if (! lsa)
    {
      if (IS_OSPF6_DUMP_SPF)
        zlog_info ("SPF: Can't find self originated Router-LSA");
      return;
    }
  if (ospf6_lsa_is_maxage (lsa))
    {
      zlog_err ("SPF: MaxAge self originated Router-LSA");
      return;
    }

  /* Create root vertex */
  v = (struct ospf6_vertex *) XMALLOC (MTYPE_OSPF6_VERTEX,
                                       sizeof (struct ospf6_vertex));
  if (! v)
    {
      zlog_err ("SPF: Can't allocate memory for root vertex");
      return;
    }
  memset (v, 0, sizeof (struct ospf6_vertex));

  v->vertex_id.id.s_addr = htonl (0);
  v->vertex_id.adv_router.s_addr = ospf6->router_id;
  v->nexthop_list = list_new ();
  v->path_list = list_new ();
  v->parent_list = list_new ();
  v->distance = 0;
  v->depth = 0;
  inet_ntop (AF_INET, &ospf6->router_id, v->string, sizeof (v->string));

  o6a->spf_tree->root = v;
  listnode_add (candidate_list, v);
}

static struct ospf6_vertex *
ospf6_spf_get_closest_candidate (list candidate_list)
{
  listnode node;
  struct ospf6_vertex *candidate, *closest;

  closest = (struct ospf6_vertex *) NULL;
  for (node = listhead (candidate_list); node; nextnode (node))
    {
      candidate = (struct ospf6_vertex *) getdata (node);

      if (closest && candidate->distance > closest->distance)
        continue;

      /* always choose network vertices if those're the same cost */
      if (closest && candidate->distance == closest->distance
          && closest->vertex_id.id.s_addr != 0)
        continue;

      closest = candidate;
    }

  return closest;
}

static struct ospf6_vertex *
ospf6_spf_get_same_candidate (struct ospf6_vertex *w, list candidate_list)
{
  listnode node;
  struct ospf6_vertex *c, *same;

  same = (struct ospf6_vertex *) NULL;
  for (node = listhead (candidate_list); node; nextnode (node))
    {
      c = (struct ospf6_vertex *) getdata (node);
      if (w->vertex_id.adv_router.s_addr != c->vertex_id.adv_router.s_addr)
        continue;
      if (w->vertex_id.id.s_addr != c->vertex_id.id.s_addr)
        continue;

      if (same)
        zlog_warn ("SPF: duplicate candidates in candidate_list");

      same = c;
    }

  return same;
}

static void
ospf6_spf_install (struct ospf6_vertex *vertex, struct ospf6_area *o6a)
{
  listnode node;
  struct ospf6_vertex *parent;

  if (IS_OSPF6_DUMP_SPF)
    zlog_info ("SPF: installing vertex %s", vertex->string);

  listnode_add (o6a->spf_tree->list, vertex);

  for (node = listhead (vertex->parent_list); node; nextnode (node))
    {
      parent = (struct ospf6_vertex *) getdata (node);
      listnode_add (parent->path_list, vertex);
      vertex->depth = parent->depth + 1;
    }

  if (vertex == o6a->spf_tree->root)
    return;

  ospf6_route_create ((struct prefix *) &vertex->vertex_id,
                      vertex->string, vertex->opt_capability,
                      vertex->capability_bits, o6a->area_id,
                      OSPF6_ROUTE_PATH_TYPE_INTRA, vertex->distance,
                      0, vertex->vertex_id.id.s_addr,
                      vertex->vertex_id.adv_router.s_addr,
                      vertex->nexthop_list, o6a->table_topology);
}

struct ospf6_vertex *
ospf6_spf_lookup (struct ospf6_vertex *w, struct ospf6_area *o6a)
{
  listnode node;
  struct ospf6_vertex *v;

  for (node = listhead (o6a->spf_tree->list); node; nextnode (node))
    {
      v = (struct ospf6_vertex *) getdata (node);

      if (w->vertex_id.adv_router.s_addr != v->vertex_id.adv_router.s_addr)
        continue;
      if (w->vertex_id.id.s_addr != v->vertex_id.id.s_addr)
        continue;

      return v;
    }

  return (struct ospf6_vertex *) NULL;
}

/* RFC2328 section 16.1 , RFC2740 section 3.8.1 */
static int
ospf6_spf_calculation (struct ospf6_area *o6a)
{
  list candidate_list;
  struct ospf6_vertex *V, *W, *X;
  int ldnum, i;

  if (! o6a || ! o6a->spf_tree)
    {
      zlog_err ("SPF: Can't calculate SPF tree: malformed area");
      return -1;
    }

  if (IS_OSPF6_DUMP_SPF)
    zlog_info ("SPF: Calculation for area %s", o6a->str);

  /* (1): Initialize the algorithm's data structures */
  candidate_list = list_new ();
  ospf6_spf_initialize (candidate_list, o6a);

  /* (3): Install closest from candidate list; if empty, break */
  while (listcount (candidate_list))
    {
      V = ospf6_spf_get_closest_candidate (candidate_list);
      listnode_delete (candidate_list, V);
      ospf6_spf_install (V, o6a);

      /* (2): Examin LSA of just added vertex */
      ldnum = ospf6_spf_lsd_num (V, o6a);
      for (i = 0; i < ldnum; i++)
        {
          /* (b): If no LSA, or MaxAge, or LinkBack fail, examin next */
          W = ospf6_spf_vertex_create (i, V, o6a);
          if (! W)
            continue;

          if (IS_OSPF6_DUMP_SPF)
            zlog_info ("SPF: Examining Vertex: %s", W->string);

          /* (c) */
          if (ospf6_spf_lookup (W, o6a))
            {
              ospf6_spf_vertex_delete (W);
              if (IS_OSPF6_DUMP_SPF)
                zlog_info ("SPF: already in SPF tree: %s", W->string);
              continue;
            }

          /* (d) */
          X = ospf6_spf_get_same_candidate (W, candidate_list);
          if (X && X->distance < W->distance)
            {
              if (IS_OSPF6_DUMP_SPF)
                zlog_info ("SPF: shorter path found in cadidate list: %s",
                           W->string);
              ospf6_spf_vertex_delete (W);
              continue;
            }
          if (X && X->distance == W->distance)
            {
              if (IS_OSPF6_DUMP_SPF)
                zlog_info ("SPF: equal cost multi-path found to %s",
                           W->string);
              ospf6_spf_vertex_merge (W, X);
              ospf6_spf_vertex_delete (W);
              continue;
            }
          if (X)
            {
              if (IS_OSPF6_DUMP_SPF)
                zlog_info ("SPF: delete longer path to %s from candidate list",
                           W->string);
              listnode_delete (candidate_list, X);
              ospf6_spf_vertex_delete (X);
            }

          if (IS_OSPF6_DUMP_SPF)
            zlog_info ("SPF: new node add to candidate list: %s", W->string);
          listnode_add (candidate_list, W);
        }
    }

  assert (listcount (candidate_list) == 0);
  list_free (candidate_list);

  /* Clear thread timer */
  o6a->spf_tree->t_spf_calculation = (struct thread *) NULL;

  if (IS_OSPF6_DUMP_SPF)
    zlog_info ("SPF: Calculation for area %s done", o6a->str);

  return 0;
}

int
ospf6_spf_calculation_thread (struct thread *t)
{
  struct ospf6_area *o6a;
  struct timeval start, end, runtime, interval;

  o6a = (struct ospf6_area *) THREAD_ARG (t);
  if (! o6a)
    {
      zlog_err ("SPF: Thread error");
      return -1;
    }

  if (! o6a->spf_tree)
    {
      zlog_err ("SPF: Can't find SPF Tree for area: %s", o6a->str);
      return -1;
    }

  /* execute SPF calculation */
  gettimeofday (&start, (struct timezone *) NULL);
  ospf6_spf_calculation (o6a);
  gettimeofday (&end, (struct timezone *) NULL);

  /* update statistics */
  o6a->spf_tree->timerun ++;
  ospf6_timeval_sub (&end, &start, &runtime);
  ospf6_timeval_add_equal (&runtime, &o6a->spf_tree->runtime_total);

  if (o6a->spf_tree->timerun == 1)
    {
      o6a->spf_tree->runtime_min.tv_sec = runtime.tv_sec;
      o6a->spf_tree->runtime_min.tv_usec = runtime.tv_usec;
      o6a->spf_tree->runtime_max.tv_sec = runtime.tv_sec;
      o6a->spf_tree->runtime_max.tv_usec = runtime.tv_usec;
    }
  if (ospf6_timeval_cmp (&o6a->spf_tree->runtime_min, &runtime) < 0)
    {
      o6a->spf_tree->runtime_min.tv_sec = runtime.tv_sec;
      o6a->spf_tree->runtime_min.tv_usec = runtime.tv_usec;
    }
  if (ospf6_timeval_cmp (&runtime, &o6a->spf_tree->runtime_max) < 0)
    {
      o6a->spf_tree->runtime_max.tv_sec = runtime.tv_sec;
      o6a->spf_tree->runtime_max.tv_usec = runtime.tv_usec;
    }

  if (o6a->spf_tree->timerun == 1)
    {
      ospf6_timeval_sub (&start, &ospf6->starttime, &interval);
      ospf6_timeval_add_equal (&interval, &o6a->spf_tree->interval_total);
      o6a->spf_tree->interval_min.tv_sec = interval.tv_sec;
      o6a->spf_tree->interval_min.tv_usec = interval.tv_usec;
      o6a->spf_tree->interval_max.tv_sec = interval.tv_sec;
      o6a->spf_tree->interval_max.tv_usec = interval.tv_usec;
    }
  else
    {
      ospf6_timeval_sub (&start, &o6a->spf_tree->updated_time, &interval);
      ospf6_timeval_add_equal (&interval, &o6a->spf_tree->interval_total);
      if (ospf6_timeval_cmp (&o6a->spf_tree->interval_min, &interval) < 0)
        {
          o6a->spf_tree->interval_min.tv_sec = interval.tv_sec;
          o6a->spf_tree->interval_min.tv_usec = interval.tv_usec;
        }
      if (ospf6_timeval_cmp (&interval, &o6a->spf_tree->interval_max) < 0)
        {
          o6a->spf_tree->interval_max.tv_sec = interval.tv_sec;
          o6a->spf_tree->interval_max.tv_usec = interval.tv_usec;
        }
    }
  o6a->spf_tree->updated_time.tv_sec = end.tv_sec;
  o6a->spf_tree->updated_time.tv_usec = end.tv_usec;

  /* clear thread */
  o6a->spf_tree->t_spf_calculation = (struct thread *) NULL;

  return 0;
}

void
ospf6_spf_calculation_schedule (u_int32_t area_id)
{
  struct ospf6_area *o6a;
  char buf[64];

  o6a = ospf6_area_lookup (area_id, ospf6);
  if (! o6a)
    {
      inet_ntop (AF_INET, &area_id, buf, sizeof (buf));
      zlog_err ("SPF: Can't find area: %s", buf);
      return;
    }

  if (! o6a->spf_tree)
    {
      zlog_err ("SPF: Can't find SPF Tree for area: %s", o6a->str);
      return;
    }

  if (o6a->spf_tree->t_spf_calculation)
    return;

  o6a->spf_tree->t_spf_calculation =
    thread_add_event (master, ospf6_spf_calculation_thread, o6a, 0);
}

struct ospf6_spftree *
ospf6_spftree_create ()
{
  struct ospf6_spftree *spf_tree;
  spf_tree = (struct ospf6_spftree *) XMALLOC (MTYPE_OSPF6_SPFTREE,
                                               sizeof (struct ospf6_spftree));
  if (! spf_tree)
    {
      zlog_err ("SPF: Can't allocate memory for SPF tree");
      return (struct ospf6_spftree *) NULL;
    }
  memset (spf_tree, 0, sizeof (spf_tree));

  spf_tree->list = list_new ();

  return spf_tree;
}

void
ospf6_spftree_delete (struct ospf6_spftree *spf_tree)
{
  listnode node;
  struct ospf6_vertex *v;

  /* Delete spf tree */
  for (node = listhead (spf_tree->list); node; nextnode (node))
    {
      v = (struct ospf6_vertex *) getdata (node);
      ospf6_spf_vertex_delete (v);
    }
  list_delete_all_node (spf_tree->list);

  XFREE (MTYPE_OSPF6_SPFTREE, spf_tree);
}

void
ospf6_nexthop_show (struct vty *vty, struct ospf6_nexthop *nexthop)
{
  char buf[128], *ifname;
  struct ospf6_interface *o6i;

  ifname = NULL;

  o6i = ospf6_interface_lookup_by_index (nexthop->ifindex);
  if (! o6i)
    {
      zlog_err ("Spf: invalid ifindex %d in nexthop", nexthop->ifindex);
    }
  else
    ifname = o6i->interface->name;

  inet_ntop (AF_INET6, &nexthop->ipaddr, buf, sizeof (buf));
  vty_out (vty, "    %s%%%s(%d)%s", buf, ifname,
           nexthop->ifindex, VTY_NEWLINE);
}

void
ospf6_vertex_show (struct vty *vty, struct ospf6_vertex *vertex)
{
  listnode node;
  struct ospf6_nexthop *n;
  struct ospf6_vertex *v;

  vty_out (vty, "SPF node %s%s", vertex->string, VTY_NEWLINE);
  vty_out (vty, "  cost to this node: %d%s", vertex->distance, VTY_NEWLINE);
  vty_out (vty, "  hops to this node: %d%s", vertex->depth, VTY_NEWLINE);

  vty_out (vty, "  nexthops reachable to this node:%s", VTY_NEWLINE);
  for (node = listhead (vertex->nexthop_list); node; nextnode (node))
    {
      n = (struct ospf6_nexthop *) getdata (node);
      ospf6_nexthop_show (vty, n);
    }

  vty_out (vty, "  parent nodes to this node:%s", VTY_NEWLINE);
  if (! list_isempty (vertex->parent_list))
    vty_out (vty, "    ");
  for (node = listhead (vertex->parent_list); node; nextnode (node))
    {
      v = (struct ospf6_vertex *) getdata (node);
      vty_out (vty, "%s ", v->string);
    }
  if (! list_isempty (vertex->parent_list))
    vty_out (vty, "%s", VTY_NEWLINE);

  vty_out (vty, "  child nodes to this node:%s", VTY_NEWLINE);
  if (! list_isempty (vertex->path_list))
    vty_out (vty, "    ");
  for (node = listhead (vertex->path_list); node; nextnode (node))
    {
      v = (struct ospf6_vertex *) getdata (node);
      vty_out (vty, "%s ", v->string);
    }
  if (! list_isempty (vertex->path_list))
    vty_out (vty, "%s", VTY_NEWLINE);

  vty_out (vty, "%s", VTY_NEWLINE);
}

void
ospf6_spf_statistics_show (struct vty *vty, struct ospf6_spftree *spf_tree)
{
  listnode node;
  struct ospf6_vertex *vertex;
  u_int router_count, network_count, maxdepth;
  struct timeval runtime_avg, interval_avg, last_updated, now;
  char rmin[64], rmax[64], ravg[64];
  char imin[64], imax[64], iavg[64];
  char last_updated_string[64];

  maxdepth = router_count = network_count = 0;
  for (node = listhead (spf_tree->list); node; nextnode (node))
    {
      vertex = (struct ospf6_vertex *) getdata (node);
      if (vertex->vertex_id.id.s_addr)
        network_count++;
      else
        router_count++;
      if (maxdepth < vertex->depth)
        maxdepth = vertex->depth;
    }

  ospf6_timeval_div (&spf_tree->runtime_total, spf_tree->timerun,
                     &runtime_avg);
  ospf6_timeval_string (&spf_tree->runtime_min, rmin, sizeof (rmin));
  ospf6_timeval_string (&spf_tree->runtime_max, rmax, sizeof (rmax));
  ospf6_timeval_string (&runtime_avg, ravg, sizeof (ravg));

  ospf6_timeval_div (&spf_tree->interval_total, spf_tree->timerun,
                     &interval_avg);
  ospf6_timeval_string (&spf_tree->interval_min, imin, sizeof (imin));
  ospf6_timeval_string (&spf_tree->interval_max, imax, sizeof (imax));
  ospf6_timeval_string (&interval_avg, iavg, sizeof (iavg));

  gettimeofday (&now, (struct timezone *) NULL);
  ospf6_timeval_sub (&now, &spf_tree->updated_time, &last_updated);
  ospf6_timeval_string (&last_updated, last_updated_string,
                        sizeof (last_updated_string));

  vty_out (vty, "     SPF algorithm executed %d times%s", 
           spf_tree->timerun, VTY_NEWLINE);
  vty_out (vty, "     Average time to run SPF: %s%s",
           ravg, VTY_NEWLINE);
  vty_out (vty, "     Maximum time to run SPF: %s%s",
           rmax, VTY_NEWLINE);
  vty_out (vty, "     Average interval of SPF: %s%s",
           iavg, VTY_NEWLINE);
  vty_out (vty, "     SPF last updated: %s ago%s",
           last_updated_string, VTY_NEWLINE);
  vty_out (vty, "     Current SPF node count: %d%s",
           listcount (spf_tree->list), VTY_NEWLINE);
  vty_out (vty, "       Router: %d Network: %d%s",
           router_count, network_count, VTY_NEWLINE);
  vty_out (vty, "       Maximum of Hop count to nodes: %d%s",
           maxdepth, VTY_NEWLINE);
}

DEFUN (show_ipv6_ospf6_spf_node,
       show_ipv6_ospf6_spf_node_cmd,
       "show ipv6 ospf6 spf node",
       SHOW_STR
       IP6_STR
       OSPF6_STR
       "Shortest Path First caculation\n"
       "vertex infomation\n"
       )
{
  listnode i, j;
  struct ospf6_area *o6a;
  struct ospf6_vertex *vertex;

  OSPF6_CMD_CHECK_RUNNING ();

  for (i = listhead (ospf6->area_list); i; nextnode (i))
    {
      o6a = (struct ospf6_area *) getdata (i);
      vty_out (vty, "%s        SPF node for Area %s%s%s",
               VTY_NEWLINE, o6a->str, VTY_NEWLINE, VTY_NEWLINE);

      for (j = listhead (o6a->spf_tree->list); j; nextnode (j))
        {
          vertex = (struct ospf6_vertex *) getdata (j);
          ospf6_vertex_show (vty, vertex);
        }
    }

  return CMD_SUCCESS;
}


/* show spf tree written by popo@sfc.wide.ad.jp */
#define OSPF6_SPFTREE_MAX_SHOW_DEPTH 64
static int
ospf6_spftree_restnode[OSPF6_SPFTREE_MAX_SHOW_DEPTH];

static void
ospf6_spftree_show (struct vty *vty, struct ospf6_vertex *vertex, int depth)
{
  int i;
  listnode node;

  for (i = 0; i < depth; i++)
    vty_out (vty, ospf6_spftree_restnode[i] > 0 ? " |  " : "    ");
  vty_out (vty, " +- ");

  vty_out (vty, "%s", vertex->string);
  vty_out (vty, " cost %d%s", vertex->distance, VTY_NEWLINE);

  ospf6_spftree_restnode[depth]--;
  ospf6_spftree_restnode[depth + 1] = listcount (vertex->path_list);

  for (node = listhead (vertex->path_list); node; nextnode (node))
    ospf6_spftree_show (vty, (struct ospf6_vertex *) getdata (node),
                         depth + 1); 
}

DEFUN (show_ipv6_ospf6_spf_tree,
       show_ipv6_ospf6_spf_tree_cmd,
       "show ipv6 ospf6 spf tree",
       SHOW_STR
       IP6_STR
       OSPF6_STR
       "Shortest Path First caculation\n"
       "Displays spf tree\n")
{
  struct ospf6_area *o6a;
  listnode node;

  OSPF6_CMD_CHECK_RUNNING ();

  for (node = listhead (ospf6->area_list); node; nextnode (node))
    {
      o6a = (struct ospf6_area *) getdata (node);
      vty_out (vty, "%s        SPF tree for Area %s%s%s",
               VTY_NEWLINE, o6a->str, VTY_NEWLINE, VTY_NEWLINE);

      if (o6a->spf_tree->root)
        {
          ospf6_spftree_restnode[0] = 1;
          ospf6_spftree_show (vty, o6a->spf_tree->root, 0);
        }
    }

  return CMD_SUCCESS;
}

char *
ospf6_capability_bits_string (u_char capability_bits, char *buf, int size)
{
  char w, v, e, b;

  w = (capability_bits & OSPF6_ROUTER_LSA_BIT_B ? 'W' : '-');
  v = (capability_bits & OSPF6_ROUTER_LSA_BIT_V ? 'V' : '-');
  e = (capability_bits & OSPF6_ROUTER_LSA_BIT_E ? 'E' : '-');
  b = (capability_bits & OSPF6_ROUTER_LSA_BIT_B ? 'B' : '-');
  snprintf (buf, size, "%c%c%c%c", w, v, e, b);
  return buf;
}

void
ospf6_spf_table_show (struct vty *vty, struct ospf6_area *o6a)
{
  struct route_node *node;
  listnode node2;
  struct ospf6_route_info *info;
  struct prefix_ls *destination;
  u_int32_t id, adv_router;
  char buf[64], type, dstring[64], ostring[16], bstring[16];
  char nstring[64], *ifname;
  struct ospf6_interface *o6i;
  struct ospf6_nexthop *nexthop;

  vty_out (vty, "%c %-19s %4s %-8s %4s  %-25s %4s%s",
           ' ', "Destination", "Bits", "Options", "Cost", "Nexthop",
           "I/F", VTY_NEWLINE);

  for (node = route_top (o6a->table_topology); node; node = route_next (node))
    {
      info = (struct ospf6_route_info *) node->info;
      if (! info)
        continue;

      /* Destination */
      destination = (struct prefix_ls *) &node->p;
      adv_router = destination->adv_router.s_addr;
      id = destination->id.s_addr;
      inet_ntop (AF_INET, &adv_router, buf, sizeof (buf));
      if (id != 0)
        {
          type = 'N';
          snprintf (dstring, sizeof (dstring), "%s[%lu]",
                    buf, (unsigned long) htonl (id));
        }
      else
        {
          type = 'R';
          snprintf (dstring, sizeof (dstring), "%s", buf);
        }

      /* capability bits */
      ospf6_capability_bits_string (info->capability_bits, bstring,
                                    sizeof (bstring));

      /* Optional Capability */
      ospf6_opt_capability_string (info->opt_capability, ostring,
                                   sizeof (ostring));

      for (node2 = listhead (info->nexthop_list); node2; nextnode (node2))
        {
          nexthop = (struct ospf6_nexthop *) getdata (node2);

          /* Nexthop */
          inet_ntop (AF_INET6, &nexthop->ipaddr, nstring, sizeof (nstring));

          /* I/F */
          o6i = ospf6_interface_lookup_by_index (nexthop->ifindex);
          if (! o6i)
            ifname = "??";
          else
            ifname = o6i->interface->name;

          vty_out (vty, "%c %-19s %4s %-8s %4d  %-25s %4s%s",
                   type, dstring, bstring, ostring, info->cost, nstring, ifname,
                   VTY_NEWLINE);
        }
    }
}

DEFUN (show_ipv6_ospf6_spf_table,
       show_ipv6_ospf6_spf_table_cmd,
       "show ipv6 ospf6 spf table",
       SHOW_STR
       IP6_STR
       OSPF6_STR
       "Shortest Path First caculation\n"
       "Displays SPF topology table\n")
{
  struct ospf6_area *o6a;
  listnode node;

  OSPF6_CMD_CHECK_RUNNING ();

  for (node = listhead (ospf6->area_list); node; nextnode (node))
    {
      o6a = (struct ospf6_area *) getdata (node);
      vty_out (vty, "%s        SPF table for Area %s%s%s",
               VTY_NEWLINE, o6a->str, VTY_NEWLINE, VTY_NEWLINE);

      ospf6_spf_table_show (vty, o6a);
    }
  return CMD_SUCCESS;
}

void
ospf6_spf_init ()
{
  install_element (VIEW_NODE, &show_ipv6_ospf6_spf_node_cmd);
  install_element (VIEW_NODE, &show_ipv6_ospf6_spf_tree_cmd);
  install_element (VIEW_NODE, &show_ipv6_ospf6_spf_table_cmd);
  install_element (ENABLE_NODE, &show_ipv6_ospf6_spf_node_cmd);
  install_element (ENABLE_NODE, &show_ipv6_ospf6_spf_tree_cmd);
  install_element (ENABLE_NODE, &show_ipv6_ospf6_spf_table_cmd);
}


