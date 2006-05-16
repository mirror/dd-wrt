/*
 * OSPFv3 Redistribute
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

#ifndef OSPF6_REDISTRIBUTE_H
#define OSPF6_REDISTRIBUTE_H


#define OSPF6_REDISTRIBUTE_DEFAULT_TYPE    1
#define OSPF6_REDISTRIBUTE_DEFAULT_METRIC  100

struct ospf6_redistribute_info
{
  /* protocol type */
  int type;

  /* if index */
  int ifindex;

  /* ASE LS ID */
  u_int32_t id;

  /* ASE Metric-type */
  int metric_type;

  /* ASE Metric */
  u_int32_t metric;

  /* PrefixOptions */
  u_char prefix_options;

  /* Forwarding Address */
  struct in6_addr forward;
};

/* prototypes */
void ospf6_redistribute_routemap_update ();
u_int32_t ospf6_redistribute_ls_id_lookup (int , struct prefix_ipv6 *,
                                           struct ospf6 *);
void ospf6_redistribute_route_add (int, int, struct prefix_ipv6 *);
void ospf6_redistribute_route_remove (int, int, struct prefix_ipv6 *);
int ospf6_redistribute_config_write (struct vty *);
void ospf6_redistribute_show_config (struct vty *, struct ospf6 *);
void ospf6_redistribute_init (struct ospf6 *);
void ospf6_redistribute_finish (struct ospf6 *);

#endif /* OSPF6_REDISTRIBUTE_H */

