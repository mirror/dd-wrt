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

#ifndef OSPF6_ROUTE_H
#define OSPF6_ROUTE_H

/* Next Hop */
struct ospf6_nexthop
{
  /* Interface index */
  unsigned int ifindex;

  /* IP address, if any */
  struct in6_addr ipaddr;

  /* advertising router */
  u_int32_t adv_router;

  /* reference count of this nexthop */
  unsigned int lock;
};


#define OSPF6_ROUTE_PATH_TYPE_INTRA      1
#define OSPF6_ROUTE_PATH_TYPE_INTER      2
#define OSPF6_ROUTE_PATH_TYPE_EXTERNAL1  3
#define OSPF6_ROUTE_PATH_TYPE_EXTERNAL2  4

#define OSPF6_ROUTE_FLAG_NONE         0x00
#define OSPF6_ROUTE_FLAG_ZEBRA_SYNC   0x01
#define OSPF6_ROUTE_FLAG_ACTIVE       0x02

struct ospf6_route_info
{
  /* Flags */
  u_char flag;

  /* Destination type */
  u_char capability_bits;

  /* Optional Capabilities */
  u_char opt_capability[3];

  /* Associated Area */
  u_int32_t area_id;

  /* Path-type */
  u_char path_type;

  /* Cost */
  u_int32_t cost;

  /* Type 2 cost */
  u_int32_t cost_e2;

  /* Link State Origin */
  u_int32_t origin_id;
  u_int32_t origin_adv_router;

  /* Nexthop List */
  list nexthop_list;
};

struct ospf6_nexthop *
ospf6_nexthop_create (unsigned int ifindex, struct in6_addr *ipaddr,
                      u_int32_t adv_router);
void
ospf6_nexthop_delete (struct ospf6_nexthop *nexthop);

struct ospf6_route_info *
ospf6_route_create (struct prefix *destination, char *string,
                    u_char opt_capability[3], u_char capability_bits,
                    u_int32_t area_id, u_char path_type,
                    u_int32_t cost, u_int32_t cost_e2,
                    u_int32_t origin_id, u_int32_t origin_adv_router,
                    list nexthop_list, struct route_table *table);
void
ospf6_route_delete (struct prefix *destination,
                        struct route_table *table);
void
ospf6_route_delete_all (struct route_table *table);

void
ospf6_route_zebra_update ();

void
ospf6_route_statistics_show (struct vty *vty, struct route_table *table);
void
ospf6_route_calculation_schedule ();
void
ospf6_route_external_incremental (struct ospf6_lsa *lsa);
void
ospf6_route_init ();

#endif /* OSPF6_ROUTE_H */

