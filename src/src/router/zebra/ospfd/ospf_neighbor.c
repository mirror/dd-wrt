/*
 * OSPF Neighbor functions.
 * Copyright (C) 1999, 2000 Toshiaki Takada
 *
 * This file is part of GNU Zebra.
 * 
 * GNU Zebra is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
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

#include <zebra.h>

#include "linklist.h"
#include "prefix.h"
#include "memory.h"
#include "command.h"
#include "thread.h"
#include "stream.h"
#include "table.h"
#include "log.h"

#include "ospfd/ospfd.h"
#include "ospfd/ospf_interface.h"
#include "ospfd/ospf_asbr.h"
#include "ospfd/ospf_lsa.h"
#include "ospfd/ospf_lsdb.h"
#include "ospfd/ospf_neighbor.h"
#include "ospfd/ospf_nsm.h"
#include "ospfd/ospf_packet.h"
#include "ospfd/ospf_network.h"
#include "ospfd/ospf_flood.h"

struct ospf_neighbor *
ospf_nbr_new (struct ospf_interface *oi)
{
  struct ospf_neighbor *nbr;

  /* Allcate new neighbor. */
  nbr = XMALLOC (MTYPE_OSPF_NEIGHBOR, sizeof (struct ospf_neighbor));
  memset (nbr, 0, sizeof (struct ospf_neighbor));

  /* Relate neighbor to the interface. */
  nbr->oi = oi;

  /* Set default values. */
  nbr->status = NSM_Down;

  /* Set inheritance values. */
  nbr->v_inactivity = OSPF_IF_PARAM (oi, v_wait);
  nbr->v_db_desc = OSPF_IF_PARAM (oi, retransmit_interval);
  nbr->v_ls_req = OSPF_IF_PARAM (oi, retransmit_interval);
  nbr->v_ls_upd = OSPF_IF_PARAM (oi, retransmit_interval);
  nbr->priority = -1;

  /* DD flags. */
  nbr->dd_flags = OSPF_DD_FLAG_MS|OSPF_DD_FLAG_M|OSPF_DD_FLAG_I;

  /* Last received and sent DD. */
  nbr->last_send = NULL;

  nbr->nbr_static = NULL;

  ospf_lsdb_init (&nbr->db_sum);
  ospf_lsdb_init (&nbr->ls_rxmt);
  ospf_lsdb_init (&nbr->ls_req);

  nbr->crypt_seqnum = 0;

  return nbr;
}

void
ospf_nbr_free (struct ospf_neighbor *nbr)
{
  /* Free DB summary list. */
  if (ospf_db_summary_count (nbr))
    ospf_db_summary_clear (nbr);
    /* ospf_db_summary_delete_all (nbr); */

  /* Free ls request list. */
  if (ospf_ls_request_count (nbr))
    ospf_ls_request_delete_all (nbr);

  /* Free retransmit list. */
  if (ospf_ls_retransmit_count (nbr))
    ospf_ls_retransmit_clear (nbr);

  /* Cleanup LSDBs. */
  ospf_lsdb_cleanup (&nbr->db_sum);
  ospf_lsdb_cleanup (&nbr->ls_req);
  ospf_lsdb_cleanup (&nbr->ls_rxmt);
  
  /* Clear last send packet. */
  if (nbr->last_send)
    ospf_packet_free (nbr->last_send);

  if (nbr->nbr_static)
    {
      nbr->nbr_static->neighbor = NULL;
      nbr->nbr_static = NULL;
    }

  /* Cancel all timers. */
  OSPF_NSM_TIMER_OFF (nbr->t_inactivity);
  OSPF_NSM_TIMER_OFF (nbr->t_db_desc);
  OSPF_NSM_TIMER_OFF (nbr->t_ls_req);
  OSPF_NSM_TIMER_OFF (nbr->t_ls_upd);

  XFREE (MTYPE_OSPF_NEIGHBOR, nbr);
}

/* Delete specified OSPF neighbor from interface. */
void
ospf_nbr_delete (struct ospf_neighbor *nbr)
{
  struct ospf_interface *oi;
  struct route_node *rn;
  struct prefix p;

  oi = nbr->oi;

  /* Unlink ospf neighbor from the interface. */
  p.family = AF_INET;
  p.prefixlen = IPV4_MAX_BITLEN;
  p.u.prefix4 = nbr->src;

  rn = route_node_lookup (oi->nbrs, &p);
  if (rn)
    {
      if (rn->info)
	{
	  rn->info = NULL;
	  route_unlock_node (rn);
	}
      else
	zlog_info ("Can't find neighbor %s in the interface %s",
		   inet_ntoa (nbr->src), IF_NAME (oi));

      route_unlock_node (rn);
    }

  /* Free ospf_neighbor structure. */
  ospf_nbr_free (nbr);
}

/* Check myself is in the neighbor list. */
int
ospf_nbr_bidirectional (struct in_addr *router_id,
			struct in_addr *neighbors, int size)
{
  int i;
  int max;

  max = size / sizeof (struct in_addr);

  for (i = 0; i < max; i ++)
    if (IPV4_ADDR_SAME (router_id, &neighbors[i]))
      return 1;

  return 0;
}

/* Add self to nbr list. */
void
ospf_nbr_add_self (struct ospf_interface *oi)
{
  struct ospf_neighbor *nbr;
  struct prefix p;
  struct route_node *rn;

  p.family = AF_INET;
  p.prefixlen = 32;
  p.u.prefix4 = oi->address->u.prefix4;

  rn = route_node_get (oi->nbrs, &p);
  if (rn->info)
    {
      /* There is already pseudo neighbor. */
      nbr = rn->info;
      route_unlock_node (rn);
    }
  else
    rn->info = oi->nbr_self;
}

/* Get neighbor count by status.
   Specify status = 0, get all neighbor other than myself. */
int
ospf_nbr_count (struct route_table *nbrs, int status)
{
  struct route_node *rn;
  struct ospf_neighbor *nbr;
  int count = 0;

  /* Sanity check. */
  if (nbrs == NULL)
    return 0;

  for (rn = route_top (nbrs); rn; rn = route_next (rn))
    if ((nbr = rn->info) != NULL)
      /* Ignore myself. */
      if (!IPV4_ADDR_SAME (&nbr->router_id, &ospf_top->router_id))
	if (status == 0 || nbr->status == status)
	  count++;

  return count;
}

struct ospf_neighbor *
ospf_nbr_lookup_by_addr (struct route_table *nbrs,
			 struct in_addr *addr)
{
  struct prefix p;
  struct route_node *rn;
  struct ospf_neighbor *nbr;

  p.family = AF_INET;
  p.prefixlen = IPV4_MAX_BITLEN;
  p.u.prefix4 = *addr;

  rn = route_node_lookup (nbrs, &p);

  if (rn == NULL)
    return NULL;

  if (rn->info == NULL)
    {
      route_unlock_node (rn);
      return NULL;
    }

  nbr = (struct ospf_neighbor *) rn->info;
  route_unlock_node (rn);

  return nbr;
}

struct ospf_neighbor *
ospf_nbr_lookup_by_routerid (struct route_table *nbrs,
			     struct in_addr *id)
{
  struct route_node *rn;
  struct ospf_neighbor *nbr;

  for (rn = route_top (nbrs); rn; rn = route_next (rn))
    if ((nbr = rn->info) != NULL)
      if (IPV4_ADDR_SAME (&nbr->router_id, id))
	{
	  route_unlock_node(rn);
	  return nbr;
	}

  return NULL;
}

