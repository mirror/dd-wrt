/*
 * OSPF ABR functions.
 * Copyright (C) 1999 Alex Zinin
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

#ifndef _ZEBRA_OSPF_ABR_H
#define _ZEBRA_OSPF_ABR_H

#define OSPF_ABR_TASK_DELAY 	7

#define OSPF_RANGE_SUPPRESS	0x01
#define OSPF_RANGE_SUBST        0x02

struct ospf_area_range
{
  /* We do not include the range here, it will
     be the prefix in the RT. */
  struct route_node     *node;       /* Pointer to the RT node. */
  u_char    		flags;
  struct prefix_ipv4	substitute;
  int			specifics;   /* Number of more specific prefixes. */
  u_int32_t		cost;
};


struct ospf_area_range *ospf_some_area_range_match (struct prefix_ipv4 *p);
struct ospf_area_range *ospf_area_range_lookup (struct ospf_area *,
						struct in_addr *);
struct ospf_area_range *ospf_area_range_lookup_next (struct ospf_area *,
						     struct in_addr *, int);
int ospf_range_active (struct ospf_area_range *range);                       
int ospf_act_bb_connection ();

void ospf_check_abr_status ();
void ospf_abr_task ();
void ospf_schedule_abr_task ();

#endif /* _ZEBRA_OSPF_ABR_H */
