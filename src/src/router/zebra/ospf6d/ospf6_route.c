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
/* ospf6 routing table calculation function */

#include "ospf6d.h"

#include "if.h"
#include "table.h"
#include "vty.h"

#include "ospf6_proto.h"
#include "ospf6_prefix.h"
#include "ospf6_route.h"


/* new */

static int
ospf6_route_info_is_same_attribute (struct ospf6_route_info *new,
                                    struct ospf6_route_info *old)
{
  if (new->capability_bits != old->capability_bits)
    return 0;
  if (new->opt_capability[0] != old->opt_capability[0])
    return 0;
  if (new->opt_capability[1] != old->opt_capability[1])
    return 0;
  if (new->opt_capability[2] != old->opt_capability[2])
    return 0;
  if (new->area_id != old->area_id)
    return 0;
  if (new->path_type != old->path_type)
    return 0;
  if (new->cost != old->cost)
    return 0;
  if (new->cost_e2 != old->cost_e2)
    return 0;
  if (new->origin_id != old->origin_id)
    return 0;
  if (new->origin_adv_router != old->origin_adv_router)
    return 0;
  return 1;
}

int
ospf6_route_info_is_same (struct ospf6_route_info *ri1,
                          struct ospf6_route_info *ri2)
{
  listnode node;
  struct ospf6_nexthop *nexthop;

  if (! ospf6_route_info_is_same_attribute (ri1, ri2))
    return 0;

  for (node = listhead (ri1->nexthop_list); node; nextnode (node))
    {
      nexthop = (struct ospf6_nexthop *) getdata (node);
      if (listnode_lookup (ri2->nexthop_list, nexthop) == NULL)
        return 0;
    }

  for (node = listhead (ri2->nexthop_list); node; nextnode (node))
    {
      nexthop = (struct ospf6_nexthop *) getdata (node);
      if (listnode_lookup (ri1->nexthop_list, nexthop) == NULL)
        return 0;
    }

  return 1;
}

struct ospf6_nexthop *
ospf6_nexthop_create (unsigned int ifindex, struct in6_addr *ipaddr,
                      u_int32_t adv_router)
{
  listnode node;
  struct ospf6_nexthop *nexthop, *new;

  char buf [64];
  inet_ntop (AF_INET6, ipaddr, buf, sizeof (buf));

  for (node = listhead (ospf6->nexthop_list); node; nextnode (node))
    {
      nexthop = (struct ospf6_nexthop *) getdata (node);
      if (nexthop->ifindex != ifindex)
        continue;
      if (memcmp (&nexthop->ipaddr, ipaddr, sizeof (struct in6_addr)) != 0)
        continue;
      if (nexthop->adv_router != adv_router)
        continue;

      nexthop->lock++;
      return nexthop;
    }

  new = (struct ospf6_nexthop *) XMALLOC (MTYPE_OSPF6_NEXTHOP,
                                          sizeof (struct ospf6_nexthop));
  if (! new)
    {
      zlog_err ("ROUTE: Can't allocate memory for nexthop");
      return (struct ospf6_nexthop *) NULL;
    }
  memset (new, 0, sizeof (struct ospf6_nexthop));
  new->ifindex = ifindex;
  memcpy (&new->ipaddr, ipaddr, sizeof (struct in6_addr));
  new->adv_router = adv_router;
  new->lock = 1;

  listnode_add (ospf6->nexthop_list, new);
  return new;
}

void
ospf6_nexthop_delete (struct ospf6_nexthop *nexthop)
{
  nexthop->lock--;
  if (nexthop->lock == 0)
    {
      listnode_delete (ospf6->nexthop_list, nexthop);
      XFREE (MTYPE_OSPF6_NEXTHOP, nexthop);
    }
}

static struct ospf6_route_info *
ospf6_route_info_create (u_char opt_capability[3], u_char capability_bits,
                         u_int32_t area_id, u_char path_type,
                         u_int32_t cost, u_int32_t cost_e2,
                         u_int32_t origin_id, u_int32_t origin_adv_router,
                         list nexthop_list)
{
  listnode node;
  struct ospf6_route_info *new;
  struct ospf6_nexthop *n, *nexthop;

  new = (struct ospf6_route_info *) XMALLOC (MTYPE_OSPF6_ROUTE,
                                             sizeof (struct ospf6_route_info));
  if (! new)
    {
      zlog_err ("ROUTE: Can't allocate memory for route info");
      return (struct ospf6_route_info *) NULL;
    }
  memset (new, 0, sizeof (struct ospf6_route_info));

  new->opt_capability[0] = opt_capability[0];
  new->opt_capability[1] = opt_capability[1];
  new->opt_capability[2] = opt_capability[2];
  new->capability_bits = capability_bits;
  new->area_id = area_id;
  new->path_type = path_type;
  new->cost = cost;
  new->cost_e2 = cost_e2;
  new->origin_id = origin_id;
  new->origin_adv_router = origin_adv_router;
  new->nexthop_list = list_new ();

  /* nexthop */
  for (node = listhead (nexthop_list); node; nextnode (node))
    {
      n = (struct ospf6_nexthop *) getdata (node);
      nexthop = ospf6_nexthop_create (n->ifindex, &n->ipaddr, n->adv_router);
      listnode_add (new->nexthop_list, nexthop);
    }

  return new;
}

static void
ospf6_route_info_delete (struct ospf6_route_info *route_info)
{
  listnode node;
  struct ospf6_nexthop *nexthop;

  for (node = listhead (route_info->nexthop_list); node; nextnode (node))
    {
      nexthop = (struct ospf6_nexthop *) getdata (node);
      ospf6_nexthop_delete (nexthop);
    }
  list_delete (route_info->nexthop_list);
  XFREE (MTYPE_OSPF6_ROUTE, route_info);
}

static void
ospf6_route_info_merge (struct ospf6_route_info *new,
                        struct ospf6_route_info *old)
{
  listnode node;
  struct ospf6_nexthop *nexthop;

  for (node = listhead (new->nexthop_list); node; nextnode (node))
    {
      nexthop = (struct ospf6_nexthop *) getdata (node);
      if (listnode_lookup (old->nexthop_list, nexthop))
        continue;
      listnode_add (old->nexthop_list, nexthop);
      nexthop->lock++;
    }
}

/* RFC2328 section 11 */
static int
ospf6_route_info_is_new_prefered (struct ospf6_route_info *new,
                                  struct ospf6_route_info *old)
{
  if (! CHECK_FLAG (old->flag, OSPF6_ROUTE_FLAG_ACTIVE))
    return 1;

  if (new->path_type < old->path_type)
    return 1;
  else if (new->path_type > old->path_type)
    return 0;

  if (new->cost_e2 < old->cost_e2)
    return 1;
  else if (new->cost_e2 > old->cost_e2)
    return 0;

  if (new->cost < old->cost)
    return 1;
  else if (new->cost > old->cost)
    return 0;

  zlog_err ("ROUTE: Can't decide if the new route info is prefered");
  return 1;
}

struct ospf6_route_info *
ospf6_route_create (struct prefix *destination, char *string,
                    u_char opt_capability[3], u_char capability_bits,
                    u_int32_t area_id, u_char path_type,
                    u_int32_t cost, u_int32_t cost_e2,
                    u_int32_t origin_id, u_int32_t origin_adv_router,
                    list nexthop_list,
                    struct route_table *table)
{
  struct route_node *route_node;
  struct ospf6_route_info *new, *old;
  u_char flag;
  struct ospf6_route_info *route_info;

  new = ospf6_route_info_create (opt_capability, capability_bits,
                                 area_id, path_type, cost, cost_e2,
                                 origin_id, origin_adv_router, nexthop_list);
  if (! new)
    {
      zlog_err ("ROUTE: Can't create route info for %s", string);
      return (struct ospf6_route_info *) NULL;
    }

  route_node = route_node_get (table, destination);
  old = route_node->info;

  if (! old)
    {
      /* this is the first route for this destination */
      if (IS_OSPF6_DUMP_ROUTE)
        zlog_info ("ROUTE: route created: %s", string);

      SET_FLAG (new->flag, OSPF6_ROUTE_FLAG_ACTIVE);
      route_node->info = new;
      return new;
    }

  route_info = (struct ospf6_route_info *) NULL;
  flag = 0;

#if 0
  if (IS_OSPF6_DUMP_ROUTE)
    zlog_info ("ROUTE: route already exists: %s", string);
#endif

  if (ospf6_route_info_is_same (new, old))
    {
      /* the same entry exists, nothing to do */
#if 0
      if (IS_OSPF6_DUMP_ROUTE)
        zlog_info ("ROUTE: route untouched: %s", string);
#endif
      ospf6_route_info_delete (new);
      route_info = old;
    }
  else if (ospf6_route_info_is_same_attribute (new, old))
    {
      /* merge new's nexthop to old's */
      if (IS_OSPF6_DUMP_ROUTE)
        zlog_info ("ROUTE: route merged: %s", string);
      ospf6_route_info_merge (new, old);
      ospf6_route_info_delete (new);
      route_info = old;
    }
  else if (ospf6_route_info_is_new_prefered (new, old))
    {
      /* new route is prefered, replace with old one */
      if (IS_OSPF6_DUMP_ROUTE)
        zlog_info ("ROUTE: route changed: %s", string);
      ospf6_route_info_delete (old);
      route_info = new;
    }
  else
    {
      /* there's already better route for this destination */
#if 0
      if (IS_OSPF6_DUMP_ROUTE)
        zlog_info ("ROUTE: route rejected: %s", string);
#endif
      ospf6_route_info_delete (new);
      route_info = old;
    }

  SET_FLAG (route_info->flag, OSPF6_ROUTE_FLAG_ACTIVE);
  route_node->info = route_info;
  return route_info;
}

void
ospf6_route_delete (struct prefix *destination,
                    struct route_table *table)
{
  struct route_node *route_node;
  struct ospf6_route_info *route_info;
  char pstring[128];

  /* for log */
  prefix2str (destination, pstring, sizeof (pstring));

  route_node = route_node_get (table, destination);
  route_info = (struct ospf6_route_info *) route_node->info;
  if (! route_info)
    {
      if (IS_OSPF6_DUMP_ROUTE)
        zlog_info ("ROUTE: Can't delete route %s: doesn't exist", pstring);
      return;
    }

  if (CHECK_FLAG (route_info->flag, OSPF6_ROUTE_FLAG_ZEBRA_SYNC))
    {
      UNSET_FLAG (route_info->flag, OSPF6_ROUTE_FLAG_ACTIVE);
#if 0
      if (IS_OSPF6_DUMP_ROUTE)
        zlog_info ("ROUTE: delete sync'ed route: %s", pstring);
#else
      if (IS_OSPF6_DUMP_ROUTE)
        zlog_info ("ROUTE: route delete: %s", pstring);
#endif
      ospf6_zebra_route_update ((struct prefix_ipv6 *) destination,
                                route_info);
    }

#if 0
  if (IS_OSPF6_DUMP_ROUTE)
    zlog_info ("ROUTE: delete route %s", pstring);
#endif

  ospf6_route_info_delete (route_info);
  route_node->info = NULL;
}

void
ospf6_route_delete_all (struct route_table *table)
{
  struct route_node *node;

  for (node = route_top (table); node; node = route_next (node))
    {
      if (! node->info)
        continue;

      ospf6_route_delete (&node->p, table);
    }
}

static void
ospf6_route_intra_area (struct prefix_ls *d, struct ospf6_route_info *ri,
                        struct ospf6_area *o6a)
{
  int i;
  u_int16_t type, cost;
  struct ospf6_lsa *lsa;
  struct ospf6_intra_area_prefix_lsa *intra_prefix_lsa;
  struct ospf6_prefix *ospf6_prefix;
  struct prefix_ipv6 prefix;
  char buf[64], node_string[64], prefix_string[64];
  struct ospf6_lsdb_node *node;

  if (d->id.s_addr == 0)
    type = htons (OSPF6_LSA_TYPE_ROUTER);
  else
    type = htons (OSPF6_LSA_TYPE_NETWORK);

  /* for log */
  inet_ntop (AF_INET, &d->adv_router, buf, sizeof (buf));
  snprintf (node_string, sizeof (node_string),
            "%s[%lu]", buf, (unsigned long) ntohl (d->id.s_addr));

  if (IS_OSPF6_DUMP_ROUTE)
    zlog_info ("ROUTE: Calculate Route for %s", node_string);

  /* Foreach appropriate Intra-Area-Prefix-LSA */
  for (node = ospf6_lsdb_head (o6a->lsdb); node; node = ospf6_lsdb_next (node))
    {
      lsa = node->lsa;
      if (ospf6_lsa_is_maxage (lsa))
        continue;

      if (lsa->header->type != htons (OSPF6_LSA_TYPE_INTRA_PREFIX))
        continue;
      if (lsa->header->adv_router != d->adv_router.s_addr)
        continue;

    if (IS_OSPF6_DUMP_ROUTE)
      zlog_info ("ROUTE:  Check %s", lsa->str);

      /* check LS Reference */
      intra_prefix_lsa = (struct ospf6_intra_area_prefix_lsa *)
                            (lsa->header + 1);

      if (intra_prefix_lsa->refer_lstype == htons (OSPF6_LSA_TYPE_NETWORK)
          && ntohs (type) == OSPF6_LSA_TYPE_ROUTER)
        {
          zlog_info ("Network Reference while Router");
          continue;
        }
      if (intra_prefix_lsa->refer_lstype == htons (OSPF6_LSA_TYPE_ROUTER)
          && ntohs (type) == OSPF6_LSA_TYPE_NETWORK)
        {
          zlog_info ("Router Reference while Network ");
          continue;
        }

      /* If Network LSA and Reference-ID wrong */
      if (intra_prefix_lsa->refer_lstype == htons (OSPF6_LSA_TYPE_NETWORK)
          && intra_prefix_lsa->refer_lsid != 0
          && intra_prefix_lsa->refer_lsid != d->id.s_addr)
        {
          if (IS_OSPF6_DUMP_ROUTE)
            {
              zlog_info ("ROUTE: Network Reference-ID wrong: %s <-> %s",
                         node_string, lsa->str);
              zlog_info ("ROUTE:   intra_prefix_lsa->refer_lsid: %d\n",
                         ntohl (intra_prefix_lsa->refer_lsid));
              zlog_info ("ROUTE:   d->id.s_addr : %d\n",
                         ntohl (d->id.s_addr));
            }
          continue;
        }

      /* If Reference-Type is Router and the entry not indicate Router */
      if (intra_prefix_lsa->refer_lstype == htons (OSPF6_LSA_TYPE_ROUTER)
          && d->id.s_addr != 0)
        {
          if (IS_OSPF6_DUMP_ROUTE)
            zlog_info ("ROUTE: Reference-Type (Router) wrong: %s <-> %s",
                       node_string, lsa->str);
          zlog_info ("ROUTE:   intra_prefix_lsa->refer_lsid: %d\n",
                     ntohl (intra_prefix_lsa->refer_lsid));
          zlog_info ("ROUTE:   d->id.s_addr : %d\n", ntohl (d->id.s_addr));
          continue;
        }

      /* If Reference-Type is Network and the entry not indicate Network */
      if (intra_prefix_lsa->refer_lstype == htons (OSPF6_LSA_TYPE_NETWORK)
          && d->id.s_addr == 0)
        {
          if (IS_OSPF6_DUMP_ROUTE)
            zlog_info ("ROUTE: Reference-Type (Network) wrong: %s <-> %s",
                       node_string, lsa->str);
          continue;
        }

      if (IS_OSPF6_DUMP_ROUTE)
        zlog_info ("ROUTE: %s: Prefix LSA %s", node_string, lsa->str);

      /* for each ospf6 prefixes */
      ospf6_prefix = (struct ospf6_prefix *) (intra_prefix_lsa + 1);
      for (i = 0; i < ntohs (intra_prefix_lsa->prefix_number); i++)
        {
          memset (&prefix, 0, sizeof (struct prefix_ipv6));
          prefix.family = AF_INET6;
          ospf6_prefix_in6_addr (ospf6_prefix, &prefix.prefix);
          prefix.prefixlen = ospf6_prefix->prefix_length;
          cost = ri->cost + ntohs (ospf6_prefix->prefix_metric);

          prefix2str ((struct prefix *) &prefix, prefix_string,
                      sizeof (prefix_string));

          if (IS_OSPF6_DUMP_ROUTE)
            zlog_info ("ROUTE: route install %s cost %d", prefix_string, cost);

          ospf6_route_create ((struct prefix *) &prefix, prefix_string,
                              ri->opt_capability, 0, ri->area_id,
                              OSPF6_ROUTE_PATH_TYPE_INTRA,
                              cost, 0, ri->origin_id, ri->origin_adv_router,
                              ri->nexthop_list, ospf6->route_table);

          /* examin next prefix */
          ospf6_prefix = OSPF6_NEXT_PREFIX (ospf6_prefix);
        }
    }
}

void
ospf6_route_calculation_intra_area (struct ospf6_area *o6a)
{
  struct route_node *node;
  struct prefix_ls *destination;
  struct ospf6_route_info *info;

  for (node = route_top (o6a->table_topology); node; node = route_next (node))
    {
      destination = (struct prefix_ls *) &node->p;
      info = (struct ospf6_route_info *) node->info;
      if (! info)
        continue;
      ospf6_route_intra_area (destination, info, o6a);
    }
}

void
ospf6_route_calculation_inter_area (struct ospf6_area *o6a)
{
}

struct ospf6_route_info *
ospf6_route_asbr_entry (u_int32_t adv_router, struct ospf6 *o6)
{
  listnode l_node;
  struct route_node *r_node;
  struct ospf6_area *o6a;
  struct prefix_ls d;
  struct ospf6_route_info *ri;

  for (l_node = listhead (o6->area_list); l_node; nextnode (l_node))
    {
      o6a = (struct ospf6_area *) getdata (l_node);
      d.adv_router.s_addr = adv_router;
      d.id.s_addr = htonl (0);
      d.prefixlen = 64;
      d.family = AF_INET6;

      r_node = route_node_lookup (o6a->table_topology, (struct prefix *) &d);
      if (! r_node)
        return (struct ospf6_route_info *) NULL;

      ri = (struct ospf6_route_info *) r_node->info;
      if (! ri)
        return (struct ospf6_route_info *) NULL;

      if (ri->capability_bits & OSPF6_ROUTER_LSA_BIT_E)
        return ri;
      else
        return (struct ospf6_route_info *) NULL;
    }
  return (struct ospf6_route_info *) NULL;
}

/* RFC2328 section 16.4 */
void
ospf6_route_external (struct ospf6_lsa *lsa)
{
  struct ospf6_lsa_header *lsa_header;
  struct ospf6_as_external_lsa *as_external_lsa;
  struct in6_addr *forwarding_address;
  struct prefix_ipv6 forwarding, prefix;
  struct route_node *node;
  struct ospf6_route_info *ri;
  u_int16_t cost1, cost2;
  u_char path_type;
  char pstring[64];
  u_char opt_capability[3];

  /* prepare destination prefix first */
  lsa_header = (struct ospf6_lsa_header *) lsa->lsa_hdr;
  as_external_lsa = (struct ospf6_as_external_lsa *) (lsa_header + 1);
  prefix.family = AF_INET6;
  prefix.prefixlen = as_external_lsa->ospf6_prefix.prefix_length;
  ospf6_prefix_in6_addr (&as_external_lsa->ospf6_prefix, &prefix.prefix);
  prefix2str ((struct prefix *) &prefix, pstring, sizeof (pstring));
  memset (opt_capability, 0, sizeof (opt_capability));

  if (lsa_header->advrtr == ospf6->router_id)
    {
      if (IS_OSPF6_DUMP_ROUTE)
        zlog_info ("ROUTE: LSA is Self-originated: %s", lsa->str);
      return;
    }

  if (ospf6_lsa_is_maxage (lsa))
    {
      if (IS_OSPF6_DUMP_ROUTE)
        zlog_info ("ROUTE: %s: MaxAge", lsa->str);

      /* If we have the route derived from this LSA, delete the route */
      node = route_node_lookup (ospf6->route_table, (struct prefix *) &prefix);
      if (node && node->info)
        {
          ri = (struct ospf6_route_info *) node->info;
          if (ri->origin_id == lsa_header->ls_id &&
              ri->origin_adv_router == lsa_header->advrtr)
            ospf6_route_delete ((struct prefix *) &prefix, ospf6->route_table);
        }
      return;
    }

  /* test ASBR entry */
  ri = ospf6_route_asbr_entry (lsa_header->advrtr, ospf6);
  if (! ri)
    {
      if (IS_OSPF6_DUMP_ROUTE)
        zlog_info ("ROUTE: %s: no ASBR", lsa->str);
      return;
    }
  cost1 = ri->cost;
  cost2 = 0;

  /* Forwarding Address test */
  if (as_external_lsa->ase_bits & ASE_LSA_BIT_F)
    {
      if (IS_OSPF6_DUMP_ROUTE)
        zlog_info ("ROUTE: Forwarding flag specified");

      forwarding_address = (struct in6_addr *)
        OSPF6_NEXT_PREFIX (&as_external_lsa->ospf6_prefix);
      memcpy (&forwarding.prefix, forwarding_address,
              sizeof (struct in6_addr));
      forwarding.family = AF_INET6;
      forwarding.prefixlen = 128;

      /* test if forwarding address is on the routing table */
      node = route_node_match (ospf6->route_table,
                               (struct prefix *) &forwarding);
      if (! node || ! node->info)
        {
          if (IS_OSPF6_DUMP_ROUTE)
            zlog_info ("ROUTE: Fowarding-address not on the table");
          return;
        }

      /* overwrite asbr internal cost */
      ri = (struct ospf6_route_info *) node->info;
      cost1 = ri->cost;
    }

  if (as_external_lsa->ase_bits & ASE_LSA_BIT_E)
    {
      /* type-2 */
      cost2 = ntohs (as_external_lsa->ase_metric);
      path_type = OSPF6_ROUTE_PATH_TYPE_EXTERNAL2;
    }
  else
    {
      /* type-1 */
      cost1 += ntohs (as_external_lsa->ase_metric);
      path_type = OSPF6_ROUTE_PATH_TYPE_EXTERNAL1;
    }

  ospf6_route_create ((struct prefix *) &prefix, pstring,
                      opt_capability, 0, ri->area_id,
                      path_type, cost1, cost2,
                      lsa_header->ls_id, lsa_header->advrtr,
                      ri->nexthop_list, ospf6->route_table);
}

static void
ospf6_route_calculation_external (struct ospf6 *o6)
{
  struct ospf6_lsdb_node *node;

  for (node = ospf6_lsdb_head (o6->lsdb); node; node = ospf6_lsdb_next (node))
    {
      if (node->lsa->header->type != htons (OSPF6_LSA_TYPE_AS_EXTERNAL))
        continue;

      ospf6_route_external (node->lsa);
    }
}

static void
ospf6_route_invalidate ()
{
  struct route_node *node;
  struct ospf6_route_info *route_info;

  if (IS_OSPF6_DUMP_ROUTE)
    zlog_info ("ROUTE: Invalidate routing table");

  for (node = route_top (ospf6->route_table); node; node = route_next (node))
    {
      if (! node->info)
        continue;
      route_info = (struct ospf6_route_info *) node->info;
      UNSET_FLAG (route_info->flag, OSPF6_ROUTE_FLAG_ACTIVE);
    }
}

static void
ospf6_route_validate ()
{
  struct route_node *node;
  struct ospf6_route_info *route_info;
  char prefix_string[64];

  if (IS_OSPF6_DUMP_ROUTE)
    zlog_info ("ROUTE: Validate routing table");

  for (node = route_top (ospf6->route_table); node; node = route_next (node))
    {
      if (! node->info)
        continue;
      route_info = (struct ospf6_route_info *) node->info;

      prefix2str (&node->p, prefix_string, sizeof (prefix_string));

#if 0
      zlog_info ("ROUTE: DEBUG: update route: %s %s %s",
                 (CHECK_FLAG (route_info->flag, OSPF6_ROUTE_FLAG_ZEBRA_SYNC) ?
                  "sync'ed": "nosync"),
                 (CHECK_FLAG (route_info->flag, OSPF6_ROUTE_FLAG_ACTIVE) ?
                  "active": "inactive"), prefix_string);
#endif

      ospf6_zebra_route_update ((struct prefix_ipv6 *) &node->p,
                                route_info);

      if (! CHECK_FLAG (route_info->flag, OSPF6_ROUTE_FLAG_ACTIVE))
        ospf6_route_delete (&node->p, ospf6->route_table);
    }
}

void
ospf6_route_calculation_new ()
{
  listnode node;
  struct ospf6_area *o6a;

  /* stat */
  ospf6->stat_route_calculation_execed++;

  /* invalidate current routing table */
  ospf6_route_invalidate ();

  /* Intra-Area Routes */
  for (node = listhead (ospf6->area_list); node; nextnode (node))
    {
      o6a = (struct ospf6_area *) getdata (node);
      ospf6_route_calculation_intra_area (o6a);
    }

  /* Inter-Area Routes */
  if (listcount (ospf6->area_list) == 1)
    {
      o6a = (struct ospf6_area *) getdata (listhead (ospf6->area_list));
      ospf6_route_calculation_inter_area (o6a);
    }
  else
    {
      /* calculate Inter-Area routes for backbone */
      o6a = ospf6_area_lookup (htonl (0), ospf6);
      ospf6_route_calculation_inter_area (o6a);
    }

  /* Better Inter-Area Routes if this is Transit Area */
  for (node = listhead (ospf6->area_list); node; nextnode (node))
    {
      o6a = (struct ospf6_area *) getdata (node);
      if (ospf6_area_is_transit (o6a))
        ospf6_route_calculation_inter_area (o6a);
    }

  /* External Routes associated with this area */
  ospf6_route_calculation_external (ospf6);

  /* update zebra routing table */
  ospf6_route_validate ();
}

int
ospf6_route_calculation_thread (struct thread *t)
{
  ospf6_route_calculation_new ();
  ospf6->t_route_calculation = (struct thread *) NULL;
  return 0;
}

void
ospf6_route_calculation_schedule ()
{
  if (ospf6->t_route_calculation)
    return;

  ospf6->t_route_calculation =
    thread_add_event (master, ospf6_route_calculation_thread, NULL, 0);
}

void
ospf6_route_external_incremental (struct ospf6_lsa *lsa)
{
  struct ospf6_lsa_header *lsa_header;
  lsa_header = (struct ospf6_lsa_header *) lsa->lsa_hdr;

  if (lsa_header->type != htons (OSPF6_LSA_TYPE_AS_EXTERNAL))
    return;

  ospf6_route_external (lsa);
  ospf6_route_validate ();
}

char *
ospf6_route_path_type_string (u_char path_type, char *buf, int size)
{
  if (path_type == OSPF6_ROUTE_PATH_TYPE_INTRA)
    snprintf (buf, size, "IA");
  else if (path_type == OSPF6_ROUTE_PATH_TYPE_INTER)
    snprintf (buf, size, "OA");
  else if (path_type == OSPF6_ROUTE_PATH_TYPE_EXTERNAL1)
    snprintf (buf, size, "E1");
  else if (path_type == OSPF6_ROUTE_PATH_TYPE_EXTERNAL2)
    snprintf (buf, size, "E2");
  else
    snprintf (buf, size, "%d", path_type);
  return buf;
}

void
ospf6_route_statistics_show (struct vty *vty, struct route_table *table)
{
  struct route_node *node;
  struct ospf6_route_info *info;
  u_int route_count, intra_count, inter_count, e1_count, e2_count, ecmp_count;

  route_count = intra_count = inter_count = e1_count = e2_count = ecmp_count
              = 0;
  for (node = route_top (table); node; node = route_next (node))
    {
      info = (struct ospf6_route_info *) node->info;
      if (! info)
        continue;

      route_count++;
      if (info->path_type == OSPF6_ROUTE_PATH_TYPE_INTRA)
        intra_count++;
      else if (info->path_type == OSPF6_ROUTE_PATH_TYPE_INTER)
        inter_count++;
      else if (info->path_type == OSPF6_ROUTE_PATH_TYPE_EXTERNAL1)
        e1_count++;
      else if (info->path_type == OSPF6_ROUTE_PATH_TYPE_EXTERNAL2)
        e2_count++;

      if (listcount (info->nexthop_list) > 1)
        ecmp_count++;
    }

  vty_out (vty, " Current Route count: %d%s",
           route_count, VTY_NEWLINE);
  vty_out (vty, "   Intra: %d Inter: %d External: %d (Type1 %d/Type2 %d)%s",
           intra_count, inter_count, e1_count + e2_count,
           e1_count, e2_count, VTY_NEWLINE);
  vty_out (vty, "   Equal-cost multi-path: %d%s",
           ecmp_count, VTY_NEWLINE);
}

void
ospf6_route_entry_show (struct vty *vty, struct prefix_ipv6 *d,
                        struct ospf6_route_info *ri)
{
  listnode node;
  struct ospf6_nexthop *n;
  char buf[64], dstring[64], opt_string[16], nstring[64], ifname[16];
  char astring[32], origin_string[32], pstring[4];

  /* destination */
  prefix2str ((struct prefix *) d, dstring, sizeof (dstring));

  /* optional capability */
  ospf6_opt_capability_string (ri->opt_capability, opt_string,
                               sizeof (opt_string));

  /* Area id */
  inet_ntop (AF_INET, &ri->area_id, astring, sizeof (astring));

  /* Path Type */
  ospf6_route_path_type_string (ri->path_type, pstring, sizeof (pstring));

  /* Link state origin */
  inet_ntop (AF_INET, &ri->origin_adv_router, buf, sizeof (buf));
  snprintf (origin_string, sizeof (origin_string),
            "%s[%lu]", buf, (unsigned long) ntohl (ri->origin_id));

  for (node = listhead (ri->nexthop_list); node; nextnode (node))
    {
      n = (struct ospf6_nexthop *) getdata (node);

      inet_ntop (AF_INET6, &n->ipaddr, nstring, sizeof (nstring));

      if (if_indextoname (n->ifindex, buf))
        snprintf (ifname, sizeof (ifname), "%s", buf);
      else
        snprintf (ifname, sizeof (ifname), "%d", n->ifindex);

      vty_out (vty, "%c%2s %-43s %-9s %-8s %4d %-5d %-19s %-25s %4s%s",
               (CHECK_FLAG (ri->flag, OSPF6_ROUTE_FLAG_ZEBRA_SYNC) ? '*' : ' '),
               pstring, dstring, opt_string, astring, ri->cost, ri->cost_e2,
               origin_string, nstring, ifname, VTY_NEWLINE);
    }
}

void
ospf6_route_table_show_new (struct vty *vty, struct route_table *table)
{
  struct route_node *node;
  struct ospf6_route_info *info;

  vty_out (vty, "%3s %-43s %-9s %-8s %4s %-5s %-19s %-25s %4s%s",
           "   ", "Destination", "Options", "Area", "Cost", "Type2",
           "LS Origin", "Nexthop", "I/F", VTY_NEWLINE);

  for (node = route_top (table); node; node = route_next (node))
    {
      info = (struct ospf6_route_info *) node->info;
      if (! info)
        continue;
      ospf6_route_entry_show (vty, (struct prefix_ipv6 *) &node->p, info);
    }
}

DEFUN (show_ipv6_route_ospf6_new,
       show_ipv6_route_ospf6_new_cmd,
       "show ipv6 route ospf6",
       SHOW_STR
       IP6_STR
       "Routing table\n"
       OSPF6_STR
       )
{

  OSPF6_CMD_CHECK_RUNNING ();

  ospf6_route_table_show_new (vty, ospf6->route_table);
  return CMD_SUCCESS;
}

DEFUN (show_ipv6_route_ospf6_prefix,
       show_ipv6_route_ospf6_prefix_cmd,
       "show ipv6 route ospf6 X::X",
       SHOW_STR
       IP6_STR
       "Routing table\n"
       OSPF6_STR
       "match IPv6 prefix\n"
       )
{
  int ret;
  struct route_node *node;
  struct ospf6_route_info *info;
  struct prefix p;

  OSPF6_CMD_CHECK_RUNNING ();


  ret = str2prefix_ipv6 (argv[0], (struct prefix_ipv6 *) &p);
  if (ret != 1)
    return CMD_ERR_NO_MATCH;

  node = route_node_match (ospf6->route_table, &p);
  if (! node || ! node->info)
    {
      vty_out (vty, "Route not found%s", VTY_NEWLINE);
      return CMD_SUCCESS;
    }

  vty_out (vty, "%3s %-43s %-9s %-8s %4s %-5s %-19s %-25s %4s%s",
           "   ", "Destination", "Options", "Area", "Cost", "Type2",
           "LS Origin", "Nexthop", "I/F", VTY_NEWLINE);

  info = (struct ospf6_route_info *) node->info;
  ospf6_route_entry_show (vty, (struct prefix_ipv6 *) &node->p, info);
  return CMD_SUCCESS;
}

void
ospf6_route_init ()
{
  install_element (VIEW_NODE, &show_ipv6_route_ospf6_new_cmd);
  install_element (VIEW_NODE, &show_ipv6_route_ospf6_prefix_cmd);
  install_element (ENABLE_NODE, &show_ipv6_route_ospf6_new_cmd);
  install_element (ENABLE_NODE, &show_ipv6_route_ospf6_prefix_cmd);
}

