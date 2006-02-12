/*
 * OSPF routing table.
 * Copyright (C) 1999, 2000 Toshiaki Takada
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
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef _ZEBRA_OSPF_ROUTE_H
#define _ZEBRA_OSPF_ROUTE_H

#define OSPF_DESTINATION_ROUTER		1
#define OSPF_DESTINATION_NETWORK	2
#define OSPF_DESTINATION_DISCARD	3

#define OSPF_PATH_MIN			0
#define OSPF_PATH_INTRA_AREA		1
#define OSPF_PATH_INTER_AREA		2
#define OSPF_PATH_TYPE1_EXTERNAL	3
#define OSPF_PATH_TYPE2_EXTERNAL	4
#define OSPF_PATH_MAX			5

#ifndef NEW_OSPF_ROUTE
/* OSPF Path. */
struct ospf_path
{
  struct in_addr nexthop;
  struct in_addr adv_router;
  struct ospf_interface *oi;
};

struct ospf_router_route
{
  list routes;			/* list of (struct ospf_route *) */
  struct ospf_route *best;	/* the best route */
};

/* Below is the structure linked to every
   route node. Note that for Network routing
   entries a single ospf_route is kept, while
   for ABRs and ASBRs (Router routing entries),
   we link an instance of ospf_router_route
   where a list of paths is maintained, so

   nr->info is a (struct ospf_route *) for OSPF_DESTINATION_NETWORK
   but
   nr->info is a (struct ospf_router_route *) for OSPF_DESTINATION_ROUTER
*/

struct route_standard
{
  /* Link Sate Origin. */
  struct lsa_header *origin;

  /* Associated Area. */
  struct in_addr area_id;	/* The area the route belongs to */

#ifdef HAVE_NSSA
  /*  Area Type */
  int external_routing;
#endif /* HAVE_NSSA */

  /* Optional Capability. */
  u_char options;		/* Get from LSA header. */

  /*  */
  u_char flags; 		/* From router-LSA */
};

struct route_external
{
  /* Link State Origin. */
  struct ospf_lsa *origin;

  /* Link State Cost Type2. */
  u_int32_t type2_cost;

  /* Tag value. */
  u_int32_t tag;

  /* ASBR route. */
  struct ospf_route *asbr;
};

struct ospf_route
{
  /* Create time. */
  time_t ctime;

  /* Modified time. */
  time_t mtime;

  /* Destination Type. */
  u_char type;

  /* Destination ID. */		/* i.e. Link State ID. */
  struct in_addr id;

  /* Address Mask. */
  struct in_addr mask;		/* Only valid for networks. */

  /* Path Type. */
  u_char path_type;

  /* List of Paths. */
  list path;

  /* Link State Cost. */
  u_int32_t cost;		/* i.e. metric. */

  /* Route specific info. */
  union
  {
    struct route_standard std;
    struct route_external ext;
  } u;
};

#else
/* OSPF netxhop. */
struct ospf_nexthop
{
  /* Type of NextHop. */
  u_char type;
#define OSPF_NEXTHOP_ADDRESS;
#define OSPF_NEXTHOP_INTERFACE;

  /* Nexthop Address or Interface. */
  union
  {
    struct in_addr address;
    struct ospf_interface *oi;
  } nh;

  /* Advertising Router. */
  struct in_addr adv_router;

  /* Pointer to ABR/ASBR route. */
  struct ospf_route *router;

  /* Tag value. */ 
  u_int32_t tag;
};

/* OSPF Route. */
struct ospf_path
{
  /* Cost. */
  u_int32_t cost;

  /* Path-Type specific info. */
  union
  {
    struct ospf_area *area;    /* OSPF Area for Intra/Inter-Area route. */
    u_int32_t type2_cost;      /* Type-2 cost for External route. */
  } u;

  /* Nexthops. */
  struct ospf_nexthop *nexthop;
  /* list nexthop; */
#if 0
  /* Link State Origin. */
  struct ospf_lsa *origin;
#endif  
};

struct network_route
{
  /* Create time. */
  time_t ctime;

  /* Destination Type. */
  /* u_char dtype; */

  /* Destination ID. */
  struct in_addr id;

  /* Address Mask. */
  struct in_addr mask;

  /* Store each type of paths. */
  struct ospf_path *path[OSPF_PATH_MAX];
};

typedef struct network_route ospf_route;

struct router_route
{
  /* Create time. */
  time_t ctime;

  /* Destination Type. */
  /* u_char dtype; */

  /* Destination ID. */
  struct in_addr id;

  /* Optional Capability. */
  u_char options;

  /* Store list of paths. */
  list path;
};

#endif

struct ospf_path *ospf_path_new ();
void ospf_path_free (struct ospf_path *op);
struct ospf_path *ospf_path_lookup (list, struct ospf_path *);
struct ospf_route *ospf_route_new ();
void ospf_route_free (struct ospf_route *or);
void ospf_route_delete (struct route_table *rt);
void ospf_route_table_free (struct route_table *rt);

void ospf_route_install (struct route_table *);
void ospf_route_table_dump (struct route_table *);
void ospf_intra_route_add (struct route_table *, struct vertex *, 
			   struct ospf_area *);

void ospf_intra_add_router (struct route_table *, struct vertex *,
			    struct ospf_area *);

void ospf_intra_add_transit (struct route_table *, struct vertex *,
			     struct ospf_area *);

void ospf_intra_add_stub (struct route_table *, struct router_lsa_link *,
 		          struct vertex *, struct ospf_area *);

int ospf_route_cmp (struct ospf_route *, struct ospf_route *);
void ospf_route_copy_nexthops (struct ospf_route *, list);
void ospf_route_copy_nexthops_from_vertex (struct ospf_route *,
					   struct vertex * );

void ospf_route_subst (struct route_node *, struct ospf_route *,
		       struct ospf_route *);
void ospf_route_add (struct route_table *, struct prefix_ipv4 *,
		     struct ospf_route *, struct ospf_route *);

void ospf_route_subst_nexthops (struct ospf_route *, list);
void ospf_prune_unreachable_networks (struct route_table *);
void ospf_prune_unreachable_routers (struct route_table *);
int ospf_add_discard_route (struct route_table *, struct ospf_area *, 
			    struct prefix_ipv4 *);
void ospf_delete_discard_route (struct prefix_ipv4 *);
int ospf_route_match_same (struct route_table *, struct prefix_ipv4 *,
			   struct ospf_route *);

#endif /* _ZEBRA_OSPF_ROUTE_H */
