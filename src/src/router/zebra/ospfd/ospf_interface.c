/*
 * OSPF Interface functions.
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

#include "thread.h"
#include "linklist.h"
#include "prefix.h"
#include "if.h"
#include "table.h"
#include "memory.h"
#include "command.h"
#include "stream.h"
#include "log.h"

#include "ospfd/ospf_spf.h"
#include "ospfd/ospf_interface.h"
#include "ospfd/ospf_ism.h"
#include "ospfd/ospf_asbr.h"
#include "ospfd/ospf_lsa.h"
#include "ospfd/ospf_lsdb.h"
#include "ospfd/ospf_neighbor.h"
#include "ospfd/ospf_nsm.h"
#include "ospfd/ospf_packet.h"
#include "ospfd/ospf_abr.h"
#include "ospfd/ospfd.h"
#include "ospfd/ospf_network.h"
#include "ospfd/ospf_dump.h"
#ifdef HAVE_SNMP
#include "ospfd/ospf_snmp.h"
#endif /* HAVE_SNMP */

char *
ospf_if_name (struct ospf_interface *oi)
{
  static char buf[40];
  u_int32_t addr;
  
  if (oi->type == OSPF_IFTYPE_VIRTUALLINK)
    return oi->ifp->name;

  addr = ntohl (oi->address->u.prefix4.s_addr);
  snprintf (buf, 40, "%s:%d.%d.%d.%d", oi->ifp->name,
	    (addr&0xff000000) >> 24,
	    (addr&0x00ff0000) >> 16,
	    (addr&0x0000ff00) >> 8,
	    addr&0x000000ff);
  return buf;
}


int
ospf_if_get_output_cost (struct ospf_interface *oi)
{
  /* If all else fails, use default OSPF cost */
  u_int32_t cost;
  u_int32_t bw, refbw;

  bw = oi->ifp->bandwidth ? oi->ifp->bandwidth : OSPF_DEFAULT_BANDWIDTH;
  refbw = ospf_top ? ospf_top->ref_bandwidth : OSPF_DEFAULT_REF_BANDWIDTH;

  /* A specifed ip ospf cost overrides a calculated one. */
  if (OSPF_IF_PARAM_CONFIGURED (IF_DEF_PARAMS (oi->ifp), output_cost_cmd) ||
      OSPF_IF_PARAM_CONFIGURED (oi->params, output_cost_cmd))
    cost = OSPF_IF_PARAM (oi, output_cost_cmd);
  /* See if a cost can be calculated from the zebra processes
     interface bandwidth field. */
  else
    {
      cost = (u_int32_t) ((double)refbw / (double)bw + (double)0.5);
      if (cost < 1)
	cost = 1;
      else if (cost > 65535)
	cost = 65535;
    }

  return cost;
}

void
ospf_if_recalculate_output_cost (struct interface *ifp)
{
  u_int32_t newcost;
  struct route_node *rn;
  
  for (rn = route_top (IF_OIFS (ifp)); rn; rn = route_next (rn))
    {
      struct ospf_interface *oi;
      
      if ( (oi = rn->info) == NULL)
	continue;

      newcost = ospf_if_get_output_cost (oi);

      /* Is actual output cost changed? */
      if (oi->output_cost != newcost)
	{
	  oi->output_cost = newcost;
	  ospf_router_lsa_timer_add (oi->area);
	}
    }
}

void
ospf_if_reset_variables (struct ospf_interface *oi)
{
  /* Set default values. */
  /* don't clear this flag.  oi->flag = OSPF_IF_DISABLE; */

  if (oi->vl_data)
    oi->type = OSPF_IFTYPE_VIRTUALLINK;
  else 
  /* preserve network-type */
  if (oi->type != OSPF_IFTYPE_NBMA)
    oi->type = OSPF_IFTYPE_BROADCAST;

  oi->status = ISM_Down;

  oi->crypt_seqnum = 0;

  /* This must be short, (less than RxmtInterval) 
     - RFC 2328 Section 13.5 para 3.  Set to 1 second to avoid Acks being
       held back for too long - MAG */
  oi->v_ls_ack = 1;  
}

void
ospf_add_to_if (struct interface *ifp, struct ospf_interface *oi)
{
  struct route_node *node;
  
  node = route_node_get (IF_OIFS (ifp), oi->address);
  assert (!node->info);
  node->info = oi;
}

struct ospf_interface *
ospf_if_new (struct interface *ifp, struct prefix *p)
{
  struct ospf_interface *oi;

  oi = XMALLOC (MTYPE_OSPF_IF, sizeof (struct ospf_interface));
  bzero (oi, sizeof (struct ospf_interface));

  /* Set zebra interface pointer. */
  oi->ifp = ifp;
  oi->address = p;
  
  ospf_add_to_if (ifp, oi);
  listnode_add (ospf_top->oiflist, oi);
  
  /* Clear self-originated network-LSA. */
  oi->network_lsa_self = NULL;

  /* Initialize neighbor list. */
  oi->nbrs = route_table_init ();

  /* Initialize static neighbor list. */
  oi->nbr_static = list_new ();

  /* Initialize Link State Acknowledgment list. */
  oi->ls_ack = list_new ();
  oi->ls_ack_direct.ls_ack = list_new ();

  /* Set default values. */
  ospf_if_reset_variables (oi);

  /* Add pseudo neighbor. */
  oi->nbr_self = ospf_nbr_new (oi);
  oi->nbr_self->status = NSM_TwoWay;
  /*  oi->nbr_self->router_id = ospf_top->router_id; */
  oi->nbr_self->priority = OSPF_IF_PARAM (oi, priority);
  oi->nbr_self->options = OSPF_OPTION_E;

  oi->ls_upd_queue = route_table_init ();
  oi->t_ls_upd_event = NULL;
  oi->t_ls_ack_direct = NULL;

  oi->ospf = ospf_top;
  
  return oi;
}

/* Restore an interface to its pre UP state
   Used from ism_interface_down only */
void
ospf_if_cleanup (struct ospf_interface *oi)
{
  struct route_node *rn;
  listnode node;
  struct ospf_neighbor *nbr;

  /* oi->nbrs and oi->nbr_static should be deletete on InterafceDown event */
  /* delete all static neighbors attached to this interface */
  for (node = listhead (oi->nbr_static); node; )
    {
      struct ospf_nbr_static *nbr_static;

      nbr_static = getdata (node);
      nextnode (node);

      OSPF_POLL_TIMER_OFF (nbr_static->t_poll);

      if (nbr_static->neighbor)
	{
	  nbr_static->neighbor->nbr_static = NULL;
	  nbr_static->neighbor = NULL;
	}

      nbr_static->oi = NULL;
      
      listnode_delete (oi->nbr_static, nbr_static);
    }

  /* send Neighbor event KillNbr to all associated neighbors. */
  for (rn = route_top (oi->nbrs); rn; rn = route_next (rn))
    if ((nbr = rn->info) != NULL)
      if (nbr != oi->nbr_self)
	OSPF_NSM_EVENT_EXECUTE (nbr, NSM_KillNbr);

  /* Cleanup Link State Acknowlegdment list. */
  for (node = listhead (oi->ls_ack); node; nextnode (node))
    ospf_lsa_unlock (node->data);
  list_delete_all_node (oi->ls_ack);

  oi->crypt_seqnum = 0;
  
  /* Empty link state update queue */
  ospf_ls_upd_queue_empty (oi);
  
 /* Handle pseudo neighbor. */
  ospf_nbr_delete (oi->nbr_self);
  oi->nbr_self = ospf_nbr_new (oi);
  oi->nbr_self->status = NSM_TwoWay;
  oi->nbr_self->priority = OSPF_IF_PARAM (oi, priority);
  oi->nbr_self->options = OSPF_OPTION_E;

  ospf_lsa_unlock (oi->network_lsa_self);
  oi->network_lsa_self = NULL;
  OSPF_TIMER_OFF (oi->t_network_lsa_self);
}

void
ospf_if_free (struct ospf_interface *oi)
{
  struct route_node *rn;

  ospf_if_down (oi);

  assert (oi->status == ISM_Down);

  /* Free Pseudo Neighbour */
  ospf_nbr_delete (oi->nbr_self);
  
  route_table_finish (oi->nbrs);
  route_table_finish (oi->ls_upd_queue);
  
  /* Free any lists that should be freed */
  list_free (oi->nbr_static);
  
  list_free (oi->ls_ack);
  list_free (oi->ls_ack_direct.ls_ack);
  
  rn = route_node_lookup (IF_OIFS (oi->ifp), oi->address);
  assert (rn);
  assert (rn->info);
  
  rn->info = NULL;
  
  route_unlock_node (rn);
  route_unlock_node (rn);

  listnode_delete (ospf_top->oiflist, oi);
  listnode_delete (oi->area->oiflist, oi);

  memset (oi, 0, sizeof (*oi));
  XFREE (MTYPE_OSPF_IF, oi);
}


/*
*  check if interface with given address is configured and
*  return it if yes.
*/
struct ospf_interface *
ospf_if_is_configured (struct in_addr *address)
{
  listnode node;
  struct ospf_interface *oi;
  struct prefix *addr;
  
  for (node = listhead (ospf_top->oiflist); node; nextnode (node))
    if ((oi = getdata (node)) != NULL && oi->type != OSPF_IFTYPE_VIRTUALLINK)
      {
	if (oi->type == OSPF_IFTYPE_POINTOPOINT)
	  addr = oi->connected->destination;
	else
	  addr = oi->address;
	
	if (IPV4_ADDR_SAME (address, &addr->u.prefix4))
	  return oi;
      }

  return NULL;
}

struct ospf_interface *
ospf_if_lookup_by_local_addr (struct interface *ifp, struct in_addr address)
{
  listnode node;
  struct ospf_interface *oi;
  
  for (node = listhead (ospf_top->oiflist); node; nextnode (node))
    if ((oi = getdata (node)) != NULL && oi->type != OSPF_IFTYPE_VIRTUALLINK)
      {
	if (ifp && oi->ifp != ifp)
	  continue;
	
	if (IPV4_ADDR_SAME (&address, &oi->address->u.prefix4))
	  return oi;
      }

  return NULL;
}

struct ospf_interface *
ospf_if_lookup_by_prefix (struct prefix_ipv4 *p)
{
  listnode node;
  struct ospf_interface *oi;
  struct prefix ptmp;
  
  /* Check each Interface. */
  for (node = listhead (ospf_top->oiflist); node; nextnode (node)) {
    if ((oi = getdata (node)) != NULL && oi->type != OSPF_IFTYPE_VIRTUALLINK)
      {
	if (oi->type == OSPF_IFTYPE_POINTOPOINT) {
	  prefix_copy (&ptmp, oi->connected->destination);
	  ptmp.prefixlen = IPV4_MAX_BITLEN;
	}
	else
	  prefix_copy (&ptmp, oi->address);
	
	apply_mask (&ptmp);
	if (prefix_same (&ptmp, (struct prefix *) p))
	  return oi;
      }
  }
  return NULL;
}

/* determine receiving interface by source of packet */
struct ospf_interface *
ospf_if_lookup_recv_interface (struct in_addr src)
{
  listnode node;
  struct prefix_ipv4 addr;
  struct ospf_interface *oi, *match;

  addr.family = AF_INET;
  addr.prefix = src;
  addr.prefixlen = IPV4_MAX_BITLEN;

  match = NULL;

  for (node = listhead (ospf_top->oiflist); node; nextnode (node))
    {
      oi = getdata (node);
      
      if (oi->type == OSPF_IFTYPE_VIRTUALLINK)
	continue;
      
      if (oi->type == OSPF_IFTYPE_POINTOPOINT)
	{
	  if (IPV4_ADDR_SAME (&oi->connected->destination->u.prefix4, &src))
	    return oi;
	}
      else
	{
	  if (prefix_match (oi->address, (struct prefix *) &addr))
	    match = oi;
	}
    }

  return match;
}

void
ospf_if_stream_set (struct ospf_interface *oi)
{
  /* set output fifo queue. */
  if (oi->obuf == NULL) 
    oi->obuf = ospf_fifo_new ();
}

void
ospf_if_stream_unset (struct ospf_interface *oi)
{
  if (oi->obuf)
    {
     ospf_fifo_free (oi->obuf);
     oi->obuf = NULL;

     if (oi->on_write_q)
       {
	 listnode_delete (ospf_top->oi_write_q, oi);
	 oi->on_write_q = 0;
       }
    }
}

struct ospf_if_params *
ospf_new_if_params ()
{
  struct ospf_if_params *oip;

  oip = XMALLOC (MTYPE_OSPF_IF_PARAMS, sizeof (struct ospf_if_params));

  if (!oip)
    return NULL;

  memset (oip, 0, sizeof (struct ospf_if_params));

  UNSET_IF_PARAM (oip, output_cost_cmd);
  UNSET_IF_PARAM (oip, transmit_delay);
  UNSET_IF_PARAM (oip, retransmit_interval);
  UNSET_IF_PARAM (oip, passive_interface);
  UNSET_IF_PARAM (oip, v_hello);
  UNSET_IF_PARAM (oip, v_wait);
  UNSET_IF_PARAM (oip, priority);
  UNSET_IF_PARAM (oip, type);
  UNSET_IF_PARAM (oip, auth_simple);
  UNSET_IF_PARAM (oip, auth_crypt);
  UNSET_IF_PARAM (oip, auth_type);
  
  oip->auth_crypt = list_new ();

  return oip;
}

void
ospf_del_if_params (struct ospf_if_params *oip)
{
  list_delete (oip->auth_crypt);
  XFREE (MTYPE_OSPF_IF_PARAMS, oip);
}

void
ospf_free_if_params (struct interface *ifp, struct in_addr addr)
{
  struct ospf_if_params *oip;
  struct prefix_ipv4 p;
  struct route_node *rn;
  p.prefixlen = IPV4_MAX_PREFIXLEN;
  p.prefix = addr;
  rn = route_node_lookup (IF_OIFS_PARAMS (ifp), (struct prefix*)&p);
  if (!rn || !rn->info)
    return;

  oip = rn->info;
  route_unlock_node (rn);
  
  if (!OSPF_IF_PARAM_CONFIGURED (oip, output_cost_cmd) &&
      !OSPF_IF_PARAM_CONFIGURED (oip, transmit_delay) &&
      !OSPF_IF_PARAM_CONFIGURED (oip, retransmit_interval) &&
      !OSPF_IF_PARAM_CONFIGURED (oip, passive_interface) &&
      !OSPF_IF_PARAM_CONFIGURED (oip, v_hello) &&
      !OSPF_IF_PARAM_CONFIGURED (oip, v_wait) &&
      !OSPF_IF_PARAM_CONFIGURED (oip, priority) &&
      !OSPF_IF_PARAM_CONFIGURED (oip, type) &&
      !OSPF_IF_PARAM_CONFIGURED (oip, auth_simple) &&
      !OSPF_IF_PARAM_CONFIGURED (oip, auth_type) &&
      listcount (oip->auth_crypt) == 0)
    {
      ospf_del_if_params (oip);
      rn->info = NULL;
      route_unlock_node (rn);
    }
}

struct ospf_if_params *
ospf_lookup_if_params (struct interface *ifp, struct in_addr addr)
{
  struct prefix_ipv4 p;
  struct route_node *rn;

  p.prefixlen = IPV4_MAX_PREFIXLEN;
  p.prefix = addr;

  rn = route_node_lookup (IF_OIFS_PARAMS (ifp), (struct prefix*)&p);
  
  if (rn)
    {
      route_unlock_node (rn);
      return rn->info;
    }

  return NULL;
}

struct ospf_if_params *
ospf_get_if_params (struct interface *ifp, struct in_addr addr)
{
  struct prefix_ipv4 p;
  struct route_node *rn;

  p.family = AF_INET;
  p.prefixlen = IPV4_MAX_PREFIXLEN;
  p.prefix = addr;

  rn = route_node_get (IF_OIFS_PARAMS (ifp), (struct prefix*)&p);
  
  if (rn->info == NULL)
    rn->info = ospf_new_if_params ();
  else
    route_unlock_node (rn);
  
  return rn->info;
}

void
ospf_if_update_params (struct interface *ifp, struct in_addr addr)
{
  struct route_node *rn;
  struct ospf_interface *oi;
  
  for (rn = route_top (IF_OIFS (ifp)); rn; rn = route_next (rn))
    {
      if ((oi = rn->info) == NULL)
	continue;

      if (IPV4_ADDR_SAME (&oi->address->u.prefix4, &addr))
	oi->params = ospf_lookup_if_params (ifp, oi->address->u.prefix4);
    }
}

int
ospf_if_new_hook (struct interface *ifp)
{
  ifp->info = XMALLOC (MTYPE_OSPF_IF_INFO, sizeof (struct ospf_if_info));
  memset (ifp->info, 0, sizeof (struct ospf_if_info));
  
  IF_OIFS (ifp) = route_table_init ();
  IF_OIFS_PARAMS (ifp) = route_table_init ();
  
  IF_DEF_PARAMS (ifp) = ospf_new_if_params ();
  
  SET_IF_PARAM (IF_DEF_PARAMS (ifp), transmit_delay);
  IF_DEF_PARAMS (ifp)->transmit_delay = OSPF_TRANSMIT_DELAY_DEFAULT;
  
  SET_IF_PARAM (IF_DEF_PARAMS (ifp), retransmit_interval);
  IF_DEF_PARAMS (ifp)->retransmit_interval = OSPF_RETRANSMIT_INTERVAL_DEFAULT;

  SET_IF_PARAM (IF_DEF_PARAMS (ifp), priority);
  IF_DEF_PARAMS (ifp)->priority = OSPF_ROUTER_PRIORITY_DEFAULT;

  SET_IF_PARAM (IF_DEF_PARAMS (ifp), passive_interface);
  IF_DEF_PARAMS (ifp)->passive_interface = OSPF_IF_ACTIVE;

  SET_IF_PARAM (IF_DEF_PARAMS (ifp), v_hello);
  IF_DEF_PARAMS (ifp)->v_hello = OSPF_HELLO_INTERVAL_DEFAULT;

  SET_IF_PARAM (IF_DEF_PARAMS (ifp), v_wait);
  IF_DEF_PARAMS (ifp)->v_wait = OSPF_ROUTER_DEAD_INTERVAL_DEFAULT;

  SET_IF_PARAM (IF_DEF_PARAMS (ifp), auth_simple);
  memset (IF_DEF_PARAMS (ifp)->auth_simple, 0, OSPF_AUTH_SIMPLE_SIZE);
  
  SET_IF_PARAM (IF_DEF_PARAMS (ifp), auth_crypt);
  IF_DEF_PARAMS (ifp)->auth_crypt = list_new ();

  SET_IF_PARAM (IF_DEF_PARAMS (ifp), auth_type);
  IF_DEF_PARAMS (ifp)->auth_type = OSPF_AUTH_NOTSET;
  
  return 0;
}

int
ospf_if_delete_hook (struct interface *ifp)
{
  route_table_finish (IF_OIFS (ifp));
  route_table_finish (IF_OIFS_PARAMS (ifp));
  XFREE (MTYPE_OSPF_IF_INFO, ifp->info);
  ifp->info = NULL;
  return 0;
}

int
ospf_if_is_enable (struct ospf_interface *oi)
{
  if (!if_is_loopback (oi->ifp))
    if (if_is_up (oi->ifp))
	return 1;

  return 0;
}

int
ospf_if_up (struct ospf_interface *oi)
{
  if (oi == NULL)
    return 0;

  if (oi->type == OSPF_IFTYPE_LOOPBACK)
    OSPF_ISM_EVENT_SCHEDULE (oi, ISM_LoopInd);
  else
    {
      if (oi->type != OSPF_IFTYPE_VIRTUALLINK)
	ospf_if_add_allspfrouters (ospf_top, oi->address, oi->ifp->ifindex);
      ospf_if_stream_set (oi);
      OSPF_ISM_EVENT_SCHEDULE (oi, ISM_InterfaceUp);
    }

  return 1;
}

int
ospf_if_down (struct ospf_interface *oi)
{
  if (oi == NULL)
    return 0;

  OSPF_ISM_EVENT_EXECUTE (oi, ISM_InterfaceDown);
  /* Shutdown packet reception and sending */
  ospf_if_stream_unset (oi);
  if (oi->type != OSPF_IFTYPE_VIRTUALLINK)
    ospf_if_drop_allspfrouters (ospf_top, oi->address, oi->ifp->ifindex);


  return 1;
}


/* Virtual Link related functions. */

struct ospf_vl_data *
ospf_vl_data_new (struct ospf_area *area, struct in_addr vl_peer)
{
  struct ospf_vl_data *vl_data;

  vl_data = XMALLOC (MTYPE_OSPF_VL_DATA, sizeof (struct ospf_vl_data));
  bzero (vl_data, sizeof (struct ospf_vl_data));

  vl_data->vl_peer.s_addr = vl_peer.s_addr;
  vl_data->vl_area_id = area->area_id;
  vl_data->format = area->format;

  return vl_data;
}

void
ospf_vl_data_free (struct ospf_vl_data *vl_data)
{
  XFREE (MTYPE_OSPF_VL_DATA, vl_data);
}

u_int vlink_count = 0;

struct ospf_interface * 
ospf_vl_new (struct ospf_vl_data *vl_data)
{
  struct ospf_interface * voi;
  struct interface * vi;
  char   ifname[INTERFACE_NAMSIZ + 1];
  struct ospf_area *area;
  struct in_addr area_id;
  struct connected *co;
  struct prefix_ipv4 *p;
  
  if (IS_DEBUG_OSPF_EVENT)
    zlog_info ("ospf_vl_new(): Start");
  if (vlink_count == OSPF_VL_MAX_COUNT)
    {
      if (IS_DEBUG_OSPF_EVENT)
	zlog_info ("ospf_vl_new(): Alarm: "
		   "cannot create more than OSPF_MAX_VL_COUNT virtual links");
      return NULL;
    }

  if (IS_DEBUG_OSPF_EVENT)
    zlog_info ("ospf_vl_new(): creating pseudo zebra interface");

  vi = if_create ();
  co = connected_new ();
  co->ifp = vi;
  listnode_add (vi->connected, co);

  p = prefix_ipv4_new ();
  p->family = AF_INET;
  p->prefix.s_addr = 0;
  p->prefixlen = 0;
 
  co->address = (struct prefix *)p;
  
  voi = ospf_if_new (vi, co->address);
  if (voi == NULL)
    {
      if (IS_DEBUG_OSPF_EVENT)
	zlog_info ("ospf_vl_new(): Alarm: OSPF int structure is not created");
      return NULL;
    }
  voi->connected = co;
  voi->vl_data = vl_data;
  voi->ifp->mtu = OSPF_VL_MTU;
  voi->type = OSPF_IFTYPE_VIRTUALLINK;

  sprintf (ifname, "VLINK%d", vlink_count++);
  if (IS_DEBUG_OSPF_EVENT)
    zlog_info ("ospf_vl_new(): Created name: %s", ifname);
  strncpy (vi->name, ifname, IFNAMSIZ);
  if (IS_DEBUG_OSPF_EVENT)
    zlog_info ("ospf_vl_new(): set if->name to %s", vi->name);

  area_id.s_addr = 0;
  area = ospf_area_get (area_id, OSPF_AREA_ID_FORMAT_ADDRESS);
  voi->area = area;

  if (IS_DEBUG_OSPF_EVENT)
    zlog_info ("ospf_vl_new(): set associated area to the backbone");

  ospf_area_add_if (voi->area, voi);

  ospf_if_stream_set (voi);

  if (IS_DEBUG_OSPF_EVENT)
    zlog_info ("ospf_vl_new(): Stop");
  return voi;
}

void
ospf_vl_if_delete (struct ospf_vl_data *vl_data)
{
  struct interface *ifp = vl_data->vl_oi->ifp;
  vl_data->vl_oi->address->u.prefix4.s_addr = 0;
  vl_data->vl_oi->address->prefixlen = 0;
  ospf_if_free (vl_data->vl_oi);
  if_delete (ifp);
  vlink_count--;
}

struct ospf_vl_data *
ospf_vl_lookup (struct ospf_area *area, struct in_addr vl_peer)
{
  struct ospf_vl_data *vl_data;
  listnode node;

  for (node = listhead (ospf_top->vlinks); node; nextnode (node))
    if ((vl_data = getdata (node)) != NULL)
      if (vl_data->vl_peer.s_addr == vl_peer.s_addr &&
          IPV4_ADDR_SAME (&vl_data->vl_area_id, &area->area_id))
        return vl_data;

  return NULL;
}

void 
ospf_vl_shutdown (struct ospf_vl_data *vl_data)
{
  struct ospf_interface *oi;

  if ((oi = vl_data->vl_oi) == NULL)
    return;

  oi->address->u.prefix4.s_addr = 0;
  oi->address->prefixlen = 0;

  UNSET_FLAG (oi->ifp->flags, IFF_UP);
  /* OSPF_ISM_EVENT_SCHEDULE (oi, ISM_InterfaceDown); */
  OSPF_ISM_EVENT_EXECUTE (oi, ISM_InterfaceDown);
}

void
ospf_vl_add (struct ospf_vl_data *vl_data)
{
  listnode_add (ospf_top->vlinks, vl_data);
#ifdef HAVE_SNMP
  ospf_snmp_vl_add (vl_data);
#endif /* HAVE_SNMP */
}

void
ospf_vl_delete (struct ospf_vl_data *vl_data)
{
  ospf_vl_shutdown (vl_data);
  ospf_vl_if_delete (vl_data);

#ifdef HAVE_SNMP
  ospf_snmp_vl_delete (vl_data);
#endif /* HAVE_SNMP */
  listnode_delete (ospf_top->vlinks, vl_data);

  ospf_vl_data_free (vl_data);
}

void
ospf_vl_set_params (struct ospf_vl_data *vl_data, struct vertex *v)
{
  int changed = 0;
  struct ospf_interface *voi;
  listnode node;
  struct vertex_nexthop *nh;
  int i;
  struct router_lsa *rl;

  voi = vl_data->vl_oi;

  if (voi->output_cost != v->distance)
    {
      voi->output_cost = v->distance;
      changed = 1;
    }

  for (node = listhead (v->nexthop); node; nextnode (node))
    if ((nh = getdata (node)) != NULL)
      {
	vl_data->out_oi = (struct ospf_interface *) nh->oi;

	voi->address->u.prefix4 = vl_data->out_oi->address->u.prefix4;
	voi->address->prefixlen = vl_data->out_oi->address->prefixlen;

	break; /* We take the first interface. */
      }

  rl = (struct router_lsa *)v->lsa;
  
  for (i = 0; i < ntohs (rl->links); i++)
    {
      switch (rl->link[i].type)
	{
	case LSA_LINK_TYPE_VIRTUALLINK:
	  if (IS_DEBUG_OSPF_EVENT)
	    zlog_info ("found back link through VL");
	case LSA_LINK_TYPE_TRANSIT:
	case LSA_LINK_TYPE_POINTOPOINT:
	  vl_data->peer_addr = rl->link[i].link_data;
	  if (IS_DEBUG_OSPF_EVENT)
	    zlog_info ("%s peer address is %s\n",
		       vl_data->vl_oi->ifp->name, inet_ntoa(vl_data->peer_addr));
	  return;
	}
    }
}


void
ospf_vl_up_check (struct ospf_area * area, struct in_addr rid,
                  struct vertex *v)
{
  listnode node;
  struct ospf_vl_data *vl_data;
  struct ospf_interface *oi;

  if (IS_DEBUG_OSPF_EVENT)
    {
      zlog_info ("ospf_vl_up_check(): Start");
      zlog_info ("ospf_vl_up_check(): Router ID is %s", inet_ntoa (rid));
      zlog_info ("ospf_vl_up_check(): Area is %s", inet_ntoa (area->area_id));
    }

  for (node = listhead (ospf_top->vlinks); node; nextnode (node))
    {
      if ((vl_data = getdata (node)) == NULL)
        continue;
  
      if (IS_DEBUG_OSPF_EVENT)
	{
	  zlog_info ("ospf_vl_up_check(): considering VL, name: %s", 
		     vl_data->vl_oi->ifp->name);
	  zlog_info ("ospf_vl_up_check(): VL area: %s, peer ID: %s", 
		     inet_ntoa (vl_data->vl_area_id),
		     inet_ntoa (vl_data->vl_peer));
	}

      if (IPV4_ADDR_SAME (&vl_data->vl_peer, &rid) &&
          IPV4_ADDR_SAME (&vl_data->vl_area_id, &area->area_id))
        {
          oi = vl_data->vl_oi;
          SET_FLAG (vl_data->flags, OSPF_VL_FLAG_APPROVED);

	  if (IS_DEBUG_OSPF_EVENT)
	    zlog_info ("ospf_vl_up_check(): this VL matched");

          if (oi->status == ISM_Down)
            {
	      if (IS_DEBUG_OSPF_EVENT)
		zlog_info ("ospf_vl_up_check(): VL is down, waking it up");
              SET_FLAG (oi->ifp->flags, IFF_UP);
              OSPF_ISM_EVENT_SCHEDULE (oi, ISM_InterfaceUp);
            }

          ospf_vl_set_params (vl_data, v);
        }
    }
}

void
ospf_vl_unapprove ()
{
  listnode node;
  struct ospf_vl_data *vl_data;

  for (node = listhead (ospf_top->vlinks); node; nextnode (node))
    if ((vl_data = getdata (node)) != NULL)
      UNSET_FLAG (vl_data->flags, OSPF_VL_FLAG_APPROVED);
}

void
ospf_vl_shut_unapproved ()
{
  listnode node;
  struct ospf_vl_data *vl_data;

  for (node = listhead (ospf_top->vlinks); node; nextnode (node))
    if ((vl_data = getdata (node)) != NULL)
      if (!CHECK_FLAG (vl_data->flags, OSPF_VL_FLAG_APPROVED))
        ospf_vl_shutdown (vl_data);
}

int
ospf_full_virtual_nbrs (struct ospf_area *area)
{
  if (IS_DEBUG_OSPF_EVENT)
    {
      zlog_info ("counting fully adjacent virtual neighbors in area %s",
		 inet_ntoa (area->area_id));
      zlog_info ("there are %d of them", area->full_vls);
    }

  return area->full_vls;
}

int
ospf_vls_in_area (struct ospf_area *area)
{
  listnode node;
  struct ospf_vl_data *vl_data;
  int c = 0;

  for (node = listhead (ospf_top->vlinks); node; nextnode (node))
    if ((vl_data = getdata (node)) != NULL)
      if (IPV4_ADDR_SAME (&vl_data->vl_area_id, &area->area_id))
        c++;

  return c;
}


struct crypt_key *
ospf_crypt_key_new ()
{
  struct crypt_key *ck;

  ck = XMALLOC (MTYPE_OSPF_CRYPT_KEY, sizeof (struct crypt_key));
  bzero (ck, sizeof (struct crypt_key));

  return ck;
}

void
ospf_crypt_key_add (list crypt, struct crypt_key *ck)
{
  listnode_add (crypt, ck);
}

struct crypt_key *
ospf_crypt_key_lookup (list auth_crypt, u_char key_id)
{
  listnode node;
  struct crypt_key *ck;

  for (node = listhead (auth_crypt); node; nextnode (node))
    {
      ck = getdata (node);
      if (ck->key_id == key_id)
        return ck;
    }

  return NULL;
}

int
ospf_crypt_key_delete (list auth_crypt, u_char key_id)
{
  listnode node;
  struct crypt_key *ck;

  for (node = listhead (auth_crypt); node; nextnode (node))
    {
      ck = getdata (node);
      if (ck->key_id == key_id)
        {
          listnode_delete (auth_crypt, ck);
          return 1;
        }
    }

  return 0;
}


char *ospf_int_type_str[] = 
{
  "unknown",               /*should never be used*/
  "point-to-point",
  "broadcast",
  "non-broadcast",
  "point-to-multipoint",
  "virtual-link",           /*should never be used*/
  "loopback"
};

/* Configuration write function for ospfd. */
int
interface_config_write (struct vty *vty)
{
  listnode n1, n2;
  struct interface *ifp;
  struct crypt_key *ck;
  int write = 0;
  struct route_node *rn = NULL;
  struct ospf_if_params *params;

  for (n1 = listhead (iflist); n1; nextnode (n1))
    {
      ifp = getdata (n1);

      if (memcmp (ifp->name, "VLINK", 5) == 0)
	continue;

      vty_out (vty, "!%s", VTY_NEWLINE);
      vty_out (vty, "interface %s%s", ifp->name,
               VTY_NEWLINE);
      if (ifp->desc)
        vty_out (vty, " description %s%s", ifp->desc,
               VTY_NEWLINE);

      write++;

      params = IF_DEF_PARAMS (ifp);
      
      do {
	/* Interface Network print. */
	if (OSPF_IF_PARAM_CONFIGURED (params, type) &&
	    params->type != OSPF_IFTYPE_BROADCAST &&
	    params->type != OSPF_IFTYPE_LOOPBACK)
	  {
	    vty_out (vty, " ip ospf network %s",
		     ospf_int_type_str[params->type]);
	    if (params != IF_DEF_PARAMS (ifp))
	      vty_out (vty, " %s", inet_ntoa (rn->p.u.prefix4));
	    vty_out (vty, "%s", VTY_NEWLINE);
	  }
	
	/* OSPF interface authentication print */
	if (OSPF_IF_PARAM_CONFIGURED (params, auth_type) &&
	    params->auth_type != OSPF_AUTH_NOTSET)
	  {
	    char *auth_str;
	    
	    /* Translation tables are not that much help here due to syntax
	       of the simple option */
	    switch (params->auth_type)
	      {
		
	      case OSPF_AUTH_NULL:
		auth_str = " null";
		break;
		
	      case OSPF_AUTH_SIMPLE:
		auth_str = "";
		break;
		
	      case OSPF_AUTH_CRYPTOGRAPHIC:
		auth_str = " message-digest";
		break;
		
	      default:
		auth_str = "";
		break;
	      }
	    
	    vty_out (vty, " ip ospf authentication%s", auth_str);
	    if (params != IF_DEF_PARAMS (ifp))
	      vty_out (vty, " %s", inet_ntoa (rn->p.u.prefix4));
	    vty_out (vty, "%s", VTY_NEWLINE);
	  }

	/* Simple Authentication Password print. */
	if (OSPF_IF_PARAM_CONFIGURED (params, auth_simple) &&
	    params->auth_simple[0] != '\0')
	  {
	    vty_out (vty, " ip ospf authentication-key %s",
		     params->auth_simple);
	    if (params != IF_DEF_PARAMS (ifp))
	      vty_out (vty, " %s", inet_ntoa (rn->p.u.prefix4));
	    vty_out (vty, "%s", VTY_NEWLINE);
	  }
	
	/* Cryptographic Authentication Key print. */
	for (n2 = listhead (params->auth_crypt); n2; nextnode (n2))
	  {
	    ck = getdata (n2);
	    vty_out (vty, " ip ospf message-digest-key %d md5 %s",
		     ck->key_id, ck->auth_key);
	    if (params != IF_DEF_PARAMS (ifp))
	      vty_out (vty, " %s", inet_ntoa (rn->p.u.prefix4));
	    vty_out (vty, "%s", VTY_NEWLINE);
	  }
	
	/* Interface Output Cost print. */
	if (OSPF_IF_PARAM_CONFIGURED (params, output_cost_cmd))
	  {
	    vty_out (vty, " ip ospf cost %u", params->output_cost_cmd);
	    if (params != IF_DEF_PARAMS (ifp))
	      vty_out (vty, " %s", inet_ntoa (rn->p.u.prefix4));
	    vty_out (vty, "%s", VTY_NEWLINE);
	  }
	
	/* Hello Interval print. */
	if (OSPF_IF_PARAM_CONFIGURED (params, v_hello) &&
	    params->v_hello != OSPF_HELLO_INTERVAL_DEFAULT)
	  {
	    vty_out (vty, " ip ospf hello-interval %u", params->v_hello);
	    if (params != IF_DEF_PARAMS (ifp))
	      vty_out (vty, " %s", inet_ntoa (rn->p.u.prefix4));
	    vty_out (vty, "%s", VTY_NEWLINE);
	  }
	
	
	/* Router Dead Interval print. */
	if (OSPF_IF_PARAM_CONFIGURED (params, v_wait) &&
	    params->v_wait != OSPF_ROUTER_DEAD_INTERVAL_DEFAULT)
	  {
	    vty_out (vty, " ip ospf dead-interval %u", params->v_wait);
	    if (params != IF_DEF_PARAMS (ifp))
	      vty_out (vty, " %s", inet_ntoa (rn->p.u.prefix4));
	    vty_out (vty, "%s", VTY_NEWLINE);
	  }
	
      /* Router Priority print. */
	if (OSPF_IF_PARAM_CONFIGURED (params, priority) &&
	    params->priority != OSPF_ROUTER_PRIORITY_DEFAULT)
	  {
	    vty_out (vty, " ip ospf priority %u", params->priority);
	    if (params != IF_DEF_PARAMS (ifp))
	      vty_out (vty, " %s", inet_ntoa (rn->p.u.prefix4));
	    vty_out (vty, "%s", VTY_NEWLINE);
	  }
	
	/* Retransmit Interval print. */
	if (OSPF_IF_PARAM_CONFIGURED (params, retransmit_interval) &&
	    params->retransmit_interval != OSPF_RETRANSMIT_INTERVAL_DEFAULT)
	  {
	    vty_out (vty, " ip ospf retransmit-interval %u",
		     params->retransmit_interval);
	    if (params != IF_DEF_PARAMS (ifp))
	      vty_out (vty, " %s", inet_ntoa (rn->p.u.prefix4));
	    vty_out (vty, "%s", VTY_NEWLINE);
	  }
	
	/* Transmit Delay print. */
	if (OSPF_IF_PARAM_CONFIGURED (params, transmit_delay) &&
	    params->transmit_delay != OSPF_TRANSMIT_DELAY_DEFAULT)
	  {
	    vty_out (vty, " ip ospf transmit-delay %u", params->transmit_delay);
	    if (params != IF_DEF_PARAMS (ifp))
	      vty_out (vty, " %s", inet_ntoa (rn->p.u.prefix4));
	    vty_out (vty, "%s", VTY_NEWLINE);
	  }

	while (1)
	  {
	    if (rn == NULL)
	      rn = route_top (IF_OIFS_PARAMS (ifp));
	    else
	      rn = route_next (rn);

	    if (rn == NULL)
	      break;
	    params = rn->info;
	    if (params != NULL)
	      break;
	  }
      } while (rn);
      
    }

  return write;
}


DEFUN (interface_ip_ospf_authentication_args,
       interface_ip_ospf_authentication_args_addr_cmd,
       "ip ospf authentication (null|message-digest) A.B.C.D",
       "IP Information\n"
       "OSPF interface commands\n"
       "Enable authentication on this interface\n"
       "Use null authentication\n"
       "Use message-digest authentication\n"
       "Address of interface")
{
  struct interface *ifp;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;
  
  ifp = vty->index;
  params = IF_DEF_PARAMS (ifp);

  if (argc == 2)
    {
      ret = inet_aton(argv[1], &addr);
      if (!ret)
	{
	  vty_out (vty, "Please specify interface address by A.B.C.D%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}

      params = ospf_get_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  /* Handle null authentication */
  if ( argv[0][0] == 'n' )
    {
      SET_IF_PARAM (params, auth_type);
      params->auth_type = OSPF_AUTH_NULL;
      return CMD_SUCCESS;
    }

  /* Handle message-digest authentication */
  if ( argv[0][0] == 'm' )
    {
      SET_IF_PARAM (params, auth_type);
      params->auth_type = OSPF_AUTH_CRYPTOGRAPHIC;
      return CMD_SUCCESS;
    }

  vty_out (vty, "You shouldn't get here!%s", VTY_NEWLINE);
  return CMD_WARNING;
}

ALIAS (interface_ip_ospf_authentication_args,
       interface_ip_ospf_authentication_args_cmd,
       "ip ospf authentication (null|message-digest)",
       "IP Information\n"
       "OSPF interface commands\n"
       "Enable authentication on this interface\n"
       "Use null authentication\n"
       "Use message-digest authentication\n")

DEFUN (interface_ip_ospf_authentication,
       interface_ip_ospf_authentication_addr_cmd,
       "ip ospf authentication A.B.C.D",
       "IP Information\n"
       "OSPF interface commands\n"
       "Enable authentication on this interface\n"
       "Address of interface")
{
  struct interface *ifp;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;
  
  ifp = vty->index;
  params = IF_DEF_PARAMS (ifp);

  if (argc == 1)
    {
      ret = inet_aton(argv[1], &addr);
      if (!ret)
	{
	  vty_out (vty, "Please specify interface address by A.B.C.D%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}

      params = ospf_get_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }
  
  SET_IF_PARAM (params, auth_type);
  params->auth_type = OSPF_AUTH_SIMPLE;

  return CMD_SUCCESS;
}

ALIAS (interface_ip_ospf_authentication,
       interface_ip_ospf_authentication_cmd,
       "ip ospf authentication",
       "IP Information\n"
       "OSPF interface commands\n"
       "Enable authentication on this interface\n")

DEFUN (no_interface_ip_ospf_authentication,
       no_interface_ip_ospf_authentication_addr_cmd,
       "no ip ospf authentication A.B.C.D",
       NO_STR
       "IP Information\n"
       "OSPF interface commands\n"
       "Enable authentication on this interface\n"
       "Address of interface")
{
  struct interface *ifp;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;
  
  ifp = vty->index;
  params = IF_DEF_PARAMS (ifp);

  if (argc == 1)
    {
      ret = inet_aton(argv[1], &addr);
      if (!ret)
	{
	  vty_out (vty, "Please specify interface address by A.B.C.D%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}

      params = ospf_lookup_if_params (ifp, addr);
      if (params == NULL)
	return CMD_SUCCESS;
    }

  params->auth_type = OSPF_AUTH_NOTSET;
  UNSET_IF_PARAM (params, auth_type);
  
  if (params != IF_DEF_PARAMS (ifp))
    {
      ospf_free_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }
  
  return CMD_SUCCESS;
}

ALIAS (no_interface_ip_ospf_authentication,
       no_interface_ip_ospf_authentication_cmd,
       "no ip ospf authentication",
       NO_STR
       "IP Information\n"
       "OSPF interface commands\n"
       "Enable authentication on this interface\n")

DEFUN (ip_ospf_authentication_key,
       ip_ospf_authentication_key_addr_cmd,
       "ip ospf authentication-key AUTH_KEY A.B.C.D",
       "IP Information\n"
       "OSPF interface commands\n"
       "Authentication password (key)\n"
       "The OSPF password (key)\n"
       "Address of interface")
{
  struct interface *ifp;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;
  
  ifp = vty->index;
  params = IF_DEF_PARAMS (ifp);

  if (argc == 2)
    {
      ret = inet_aton(argv[1], &addr);
      if (!ret)
	{
	  vty_out (vty, "Please specify interface address by A.B.C.D%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}

      params = ospf_get_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }


  memset (params->auth_simple, 0, OSPF_AUTH_SIMPLE_SIZE + 1);
  strncpy (params->auth_simple, argv[0], OSPF_AUTH_SIMPLE_SIZE);
  SET_IF_PARAM (params, auth_simple);

  return CMD_SUCCESS;
}

ALIAS (ip_ospf_authentication_key,
       ip_ospf_authentication_key_cmd,
       "ip ospf authentication-key AUTH_KEY",
       "IP Information\n"
       "OSPF interface commands\n"
       "Authentication password (key)\n"
       "The OSPF password (key)")

ALIAS (ip_ospf_authentication_key,
       ospf_authentication_key_cmd,
       "ospf authentication-key AUTH_KEY",
       "OSPF interface commands\n"
       "Authentication password (key)\n"
       "The OSPF password (key)")

DEFUN (no_ip_ospf_authentication_key,
       no_ip_ospf_authentication_key_addr_cmd,
       "no ip ospf authentication-key A.B.C.D",
       NO_STR
       "IP Information\n"
       "OSPF interface commands\n"
       "Authentication password (key)\n"
       "Address of interface")
{
  struct interface *ifp;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;
  
  ifp = vty->index;
  params = IF_DEF_PARAMS (ifp);

  if (argc == 2)
    {
      ret = inet_aton(argv[1], &addr);
      if (!ret)
	{
	  vty_out (vty, "Please specify interface address by A.B.C.D%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}

      params = ospf_lookup_if_params (ifp, addr);
      if (params == NULL)
	return CMD_SUCCESS;
    }

  memset (params->auth_simple, 0, OSPF_AUTH_SIMPLE_SIZE);
  UNSET_IF_PARAM (params, auth_simple);
  
  if (params != IF_DEF_PARAMS (ifp))
    {
      ospf_free_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }
  
  return CMD_SUCCESS;
}

ALIAS (no_ip_ospf_authentication_key,
       no_ip_ospf_authentication_key_cmd,
       "no ip ospf authentication-key",
       NO_STR
       "IP Information\n"
       "OSPF interface commands\n"
       "Authentication password (key)\n")

ALIAS (no_ip_ospf_authentication_key,
       no_ospf_authentication_key_cmd,
       "no ospf authentication-key",
       NO_STR
       "OSPF interface commands\n"
       "Authentication password (key)\n")

DEFUN (ip_ospf_message_digest_key,
       ip_ospf_message_digest_key_addr_cmd,
       "ip ospf message-digest-key <1-255> md5 KEY A.B.C.D",
       "IP Information\n"
       "OSPF interface commands\n"
       "Message digest authentication password (key)\n"
       "Key ID\n"
       "Use MD5 algorithm\n"
       "The OSPF password (key)"
       "Address of interface")
{
  struct interface *ifp;
  struct crypt_key *ck;
  u_char key_id;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;
  
  ifp = vty->index;
  params = IF_DEF_PARAMS (ifp);

  if (argc == 3)
    {
      ret = inet_aton(argv[2], &addr);
      if (!ret)
	{
	  vty_out (vty, "Please specify interface address by A.B.C.D%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}

      params = ospf_get_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  key_id = strtol (argv[0], NULL, 10);
  if (ospf_crypt_key_lookup (params->auth_crypt, key_id) != NULL)
    {
      vty_out (vty, "OSPF: Key %d already exists%s", key_id, VTY_NEWLINE);
      return CMD_WARNING;
    }

  ck = ospf_crypt_key_new ();
  ck->key_id = (u_char) key_id;
  memset (ck->auth_key, 0, OSPF_AUTH_MD5_SIZE+1);
  strncpy (ck->auth_key, argv[1], OSPF_AUTH_MD5_SIZE);

  ospf_crypt_key_add (params->auth_crypt, ck);
  SET_IF_PARAM (params, auth_crypt);
  
  return CMD_SUCCESS;
}

ALIAS (ip_ospf_message_digest_key,
       ip_ospf_message_digest_key_cmd,
       "ip ospf message-digest-key <1-255> md5 KEY",
       "IP Information\n"
       "OSPF interface commands\n"
       "Message digest authentication password (key)\n"
       "Key ID\n"
       "Use MD5 algorithm\n"
       "The OSPF password (key)")

ALIAS (ip_ospf_message_digest_key,
       ospf_message_digest_key_cmd,
       "ospf message-digest-key <1-255> md5 KEY",
       "OSPF interface commands\n"
       "Message digest authentication password (key)\n"
       "Key ID\n"
       "Use MD5 algorithm\n"
       "The OSPF password (key)")

DEFUN (no_ip_ospf_message_digest_key,
       no_ip_ospf_message_digest_key_addr_cmd,
       "no ip ospf message-digest-key <1-255> A.B.C.D",
       NO_STR
       "IP Information\n"
       "OSPF interface commands\n"
       "Message digest authentication password (key)\n"
       "Key ID\n"
       "Address of interface")
{
  struct interface *ifp;
  struct crypt_key *ck;
  int key_id;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;
  
  ifp = vty->index;
  params = IF_DEF_PARAMS (ifp);

  if (argc == 2)
    {
      ret = inet_aton(argv[1], &addr);
      if (!ret)
	{
	  vty_out (vty, "Please specify interface address by A.B.C.D%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}

      params = ospf_lookup_if_params (ifp, addr);
      if (params == NULL)
	return CMD_SUCCESS;
    }

  key_id = strtol (argv[0], NULL, 10);
  ck = ospf_crypt_key_lookup (params->auth_crypt, key_id);
  if (ck == NULL)
    {
      vty_out (vty, "OSPF: Key %d does not exist%s", key_id, VTY_NEWLINE);
      return CMD_WARNING;
    }

  ospf_crypt_key_delete (params->auth_crypt, key_id);

  if (params != IF_DEF_PARAMS (ifp))
    {
      ospf_free_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }
  
  return CMD_SUCCESS;
}

ALIAS (no_ip_ospf_message_digest_key,
       no_ip_ospf_message_digest_key_cmd,
       "no ip ospf message-digest-key <1-255>",
       NO_STR
       "IP Information\n"
       "OSPF interface commands\n"
       "Message digest authentication password (key)\n"
       "Key ID\n")
     
ALIAS (no_ip_ospf_message_digest_key,
       no_ospf_message_digest_key_cmd,
       "no ospf message-digest-key <1-255>",
       NO_STR
       "OSPF interface commands\n"
       "Message digest authentication password (key)\n"
       "Key ID\n")

DEFUN (ip_ospf_cost,
       ip_ospf_cost_addr_cmd,
       "ip ospf cost <1-65535> A.B.C.D",
       "IP Information\n"
       "OSPF interface commands\n"
       "Interface cost\n"
       "Cost\n"
       "Address of interface")
{
  struct interface *ifp = vty->index;
  u_int32_t cost;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;
      
  params = IF_DEF_PARAMS (ifp);

  cost = strtol (argv[0], NULL, 10);

  /* cost range is <1-65535>. */
  if (cost < 1 || cost > 65535)
    {
      vty_out (vty, "Interface output cost is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (argc == 2)
    {
      ret = inet_aton(argv[1], &addr);
      if (!ret)
	{
	  vty_out (vty, "Please specify interface address by A.B.C.D%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}

      params = ospf_get_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  SET_IF_PARAM (params, output_cost_cmd);
  params->output_cost_cmd = cost;

  ospf_if_recalculate_output_cost (ifp);
    
  return CMD_SUCCESS;
}

ALIAS (ip_ospf_cost,
       ip_ospf_cost_cmd,
       "ip ospf cost <1-65535>",
       "IP Information\n"
       "OSPF interface commands\n"
       "Interface cost\n"
       "Cost")

ALIAS (ip_ospf_cost,
       ospf_cost_cmd,
       "ospf cost <1-65535>",
       "OSPF interface commands\n"
       "Interface cost\n"
       "Cost")

DEFUN (no_ip_ospf_cost,
       no_ip_ospf_cost_addr_cmd,
       "no ip ospf cost A.B.C.D",
       NO_STR
       "IP Information\n"
       "OSPF interface commands\n"
       "Interface cost\n"
       "Address of interface")
{
  struct interface *ifp = vty->index;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;
  
  ifp = vty->index;
  params = IF_DEF_PARAMS (ifp);

  if (argc == 1)
    {
      ret = inet_aton(argv[0], &addr);
      if (!ret)
	{
	  vty_out (vty, "Please specify interface address by A.B.C.D%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}

      params = ospf_lookup_if_params (ifp, addr);
      if (params == NULL)
	return CMD_SUCCESS;
    }

  UNSET_IF_PARAM (params, output_cost_cmd);

  if (params != IF_DEF_PARAMS (ifp))
    {
      ospf_free_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  ospf_if_recalculate_output_cost (ifp);
  
  return CMD_SUCCESS;
}

ALIAS (no_ip_ospf_cost,
       no_ip_ospf_cost_cmd,
       "no ip ospf cost",
       NO_STR
       "IP Information\n"
       "OSPF interface commands\n"
       "Interface cost\n")

ALIAS (no_ip_ospf_cost,
       no_ospf_cost_cmd,
       "no ospf cost",
       NO_STR
       "OSPF interface commands\n"
       "Interface cost\n")

DEFUN (ip_ospf_dead_interval,
       ip_ospf_dead_interval_addr_cmd,
       "ip ospf dead-interval <1-65535> A.B.C.D",
       "IP Information\n"
       "OSPF interface commands\n"
       "Interval after which a neighbor is declared dead\n"
       "Seconds\n"
       "Address of interface")
{
  struct interface *ifp = vty->index;
  u_int32_t seconds;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;
      
  params = IF_DEF_PARAMS (ifp);

  seconds = strtol (argv[0], NULL, 10);

  /* dead_interval range is <1-65535>. */
  if (seconds < 1 || seconds > 65535)
    {
      vty_out (vty, "Router Dead Interval is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }


  if (argc == 2)
    {
      ret = inet_aton(argv[1], &addr);
      if (!ret)
	{
	  vty_out (vty, "Please specify interface address by A.B.C.D%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}

      params = ospf_get_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  SET_IF_PARAM (params, v_wait);
  params->v_wait = seconds;
  
  return CMD_SUCCESS;
}

ALIAS (ip_ospf_dead_interval,
       ip_ospf_dead_interval_cmd,
       "ip ospf dead-interval <1-65535>",
       "IP Information\n"
       "OSPF interface commands\n"
       "Interval after which a neighbor is declared dead\n"
       "Seconds\n")

ALIAS (ip_ospf_dead_interval,
       ospf_dead_interval_cmd,
       "ospf dead-interval <1-65535>",
       "OSPF interface commands\n"
       "Interval after which a neighbor is declared dead\n"
       "Seconds\n")

DEFUN (no_ip_ospf_dead_interval,
       no_ip_ospf_dead_interval_addr_cmd,
       "no ip ospf dead-interval A.B.C.D",
       NO_STR
       "IP Information\n"
       "OSPF interface commands\n"
       "Interval after which a neighbor is declared dead\n"
       "Address of interface")
{
  struct interface *ifp = vty->index;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;
  
  ifp = vty->index;
  params = IF_DEF_PARAMS (ifp);

  if (argc == 1)
    {
      ret = inet_aton(argv[0], &addr);
      if (!ret)
	{
	  vty_out (vty, "Please specify interface address by A.B.C.D%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}

      params = ospf_lookup_if_params (ifp, addr);
      if (params == NULL)
	return CMD_SUCCESS;
    }

  UNSET_IF_PARAM (params, v_wait);
  params->v_wait = OSPF_ROUTER_DEAD_INTERVAL_DEFAULT;

  if (params != IF_DEF_PARAMS (ifp))
    {
      ospf_free_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  return CMD_SUCCESS;
}

ALIAS (no_ip_ospf_dead_interval,
       no_ip_ospf_dead_interval_cmd,
       "no ip ospf dead-interval",
       NO_STR
       "IP Information\n"
       "OSPF interface commands\n"
       "Interval after which a neighbor is declared dead\n")

ALIAS (no_ip_ospf_dead_interval,
       no_ospf_dead_interval_cmd,
       "no ospf dead-interval",
       NO_STR
       "OSPF interface commands\n"
       "Interval after which a neighbor is declared dead\n")

DEFUN (ip_ospf_hello_interval,
       ip_ospf_hello_interval_addr_cmd,
       "ip ospf hello-interval <1-65535> A.B.C.D",
       "IP Information\n"
       "OSPF interface commands\n"
       "Time between HELLO packets\n"
       "Seconds\n"
       "Address of interface")
{
  struct interface *ifp = vty->index;
  u_int32_t seconds;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;
      
  params = IF_DEF_PARAMS (ifp);

  seconds = strtol (argv[0], NULL, 10);
  
  /* HelloInterval range is <1-65535>. */
  if (seconds < 1 || seconds > 65535)
    {
      vty_out (vty, "Hello Interval is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (argc == 2)
    {
      ret = inet_aton(argv[1], &addr);
      if (!ret)
	{
	  vty_out (vty, "Please specify interface address by A.B.C.D%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}

      params = ospf_get_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  SET_IF_PARAM (params, v_hello); 
  params->v_hello = seconds;

  return CMD_SUCCESS;
}

ALIAS (ip_ospf_hello_interval,
       ip_ospf_hello_interval_cmd,
       "ip ospf hello-interval <1-65535>",
       "IP Information\n"
       "OSPF interface commands\n"
       "Time between HELLO packets\n"
       "Seconds\n")

ALIAS (ip_ospf_hello_interval,
       ospf_hello_interval_cmd,
       "ospf hello-interval <1-65535>",
       "OSPF interface commands\n"
       "Time between HELLO packets\n"
       "Seconds\n")

DEFUN (no_ip_ospf_hello_interval,
       no_ip_ospf_hello_interval_addr_cmd,
       "no ip ospf hello-interval A.B.C.D",
       NO_STR
       "IP Information\n"
       "OSPF interface commands\n"
       "Time between HELLO packets\n"
       "Address of interface")
{
  struct interface *ifp = vty->index;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;
  
  ifp = vty->index;
  params = IF_DEF_PARAMS (ifp);

  if (argc == 1)
    {
      ret = inet_aton(argv[0], &addr);
      if (!ret)
	{
	  vty_out (vty, "Please specify interface address by A.B.C.D%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}

      params = ospf_lookup_if_params (ifp, addr);
      if (params == NULL)
	return CMD_SUCCESS;
    }

  UNSET_IF_PARAM (params, v_hello);
  params->v_hello = OSPF_ROUTER_DEAD_INTERVAL_DEFAULT;

  if (params != IF_DEF_PARAMS (ifp))
    {
      ospf_free_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  return CMD_SUCCESS;
}

ALIAS (no_ip_ospf_hello_interval,
       no_ip_ospf_hello_interval_cmd,
       "no ip ospf hello-interval",
       NO_STR
       "IP Information\n"
       "OSPF interface commands\n"
       "Time between HELLO packets\n")

ALIAS (no_ip_ospf_hello_interval,
       no_ospf_hello_interval_cmd,
       "no ospf hello-interval",
       NO_STR
       "OSPF interface commands\n"
       "Time between HELLO packets\n")

DEFUN (ip_ospf_network,
       ip_ospf_network_cmd,
       "ip ospf network (broadcast|non-broadcast|point-to-multipoint|point-to-point)",
       "IP Information\n"
       "OSPF interface commands\n"
       "Network type\n"
       "Specify OSPF broadcast multi-access network\n"
       "Specify OSPF NBMA network\n"
       "Specify OSPF point-to-multipoint network\n"
       "Specify OSPF point-to-point network\n")
{
  struct interface *ifp = vty->index;
  int old_type = IF_DEF_PARAMS (ifp)->type;
  struct route_node *rn;
  
  if (strncmp (argv[0], "b", 1) == 0)
    IF_DEF_PARAMS (ifp)->type = OSPF_IFTYPE_BROADCAST;
  else if (strncmp (argv[0], "n", 1) == 0)
    IF_DEF_PARAMS (ifp)->type = OSPF_IFTYPE_NBMA;
  else if (strncmp (argv[0], "point-to-m", 10) == 0)
    IF_DEF_PARAMS (ifp)->type = OSPF_IFTYPE_POINTOMULTIPOINT;
  else if (strncmp (argv[0], "point-to-p", 10) == 0)
    IF_DEF_PARAMS (ifp)->type = OSPF_IFTYPE_POINTOPOINT;

  if (IF_DEF_PARAMS (ifp)->type == old_type)
    return CMD_SUCCESS;

  SET_IF_PARAM (IF_DEF_PARAMS (ifp), type);

  for (rn = route_top (IF_OIFS (ifp)); rn; rn = route_next (rn))
    {
      struct ospf_interface *oi = rn->info;

      if (!oi)
	continue;
      
      oi->type = IF_DEF_PARAMS (ifp)->type;
      
      if (oi->status > ISM_Down)
	{
	  OSPF_ISM_EVENT_EXECUTE (oi, ISM_InterfaceDown);
	  OSPF_ISM_EVENT_EXECUTE (oi, ISM_InterfaceUp);
	}
    }

  return CMD_SUCCESS;
}

ALIAS (ip_ospf_network,
       ospf_network_cmd,
       "ospf network (broadcast|non-broadcast|point-to-multipoint|point-to-point)",
       "OSPF interface commands\n"
       "Network type\n"
       "Specify OSPF broadcast multi-access network\n"
       "Specify OSPF NBMA network\n"
       "Specify OSPF point-to-multipoint network\n"
       "Specify OSPF point-to-point network\n")

DEFUN (no_ip_ospf_network,
       no_ip_ospf_network_cmd,
       "no ip ospf network",
       NO_STR
       "IP Information\n"
       "OSPF interface commands\n"
       "Network type\n")
{
  struct interface *ifp = vty->index;
  int old_type = IF_DEF_PARAMS (ifp)->type;
  struct route_node *rn;

  IF_DEF_PARAMS (ifp)->type = OSPF_IFTYPE_BROADCAST;

  if (IF_DEF_PARAMS (ifp)->type == old_type)
    return CMD_SUCCESS;

  for (rn = route_top (IF_OIFS (ifp)); rn; rn = route_next (rn))
    {
      struct ospf_interface *oi = rn->info;

      if (!oi)
	continue;
      
      oi->type = IF_DEF_PARAMS (ifp)->type;
      
      if (oi->status > ISM_Down)
	{
	  OSPF_ISM_EVENT_EXECUTE (oi, ISM_InterfaceDown);
	  OSPF_ISM_EVENT_EXECUTE (oi, ISM_InterfaceUp);
	}
    }

  return CMD_SUCCESS;
}

ALIAS (no_ip_ospf_network,
       no_ospf_network_cmd,
       "no ospf network",
       NO_STR
       "OSPF interface commands\n"
       "Network type\n")

DEFUN (ip_ospf_priority,
       ip_ospf_priority_addr_cmd,
       "ip ospf priority <0-255> A.B.C.D",
       "IP Information\n"
       "OSPF interface commands\n"
       "Router priority\n"
       "Priority\n"
       "Address of interface")
{
  struct interface *ifp = vty->index;
  u_int32_t priority;
  struct route_node *rn;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;
      
  params = IF_DEF_PARAMS (ifp);

  priority = strtol (argv[0], NULL, 10);
  
  /* Router Priority range is <0-255>. */
  if (priority < 0 || priority > 255)
    {
      vty_out (vty, "Router Priority is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (argc == 2)
    {
      ret = inet_aton(argv[1], &addr);
      if (!ret)
	{
	  vty_out (vty, "Please specify interface address by A.B.C.D%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}

      params = ospf_get_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }
  
  SET_IF_PARAM (params, priority);
  params->priority = priority;

  for (rn = route_top (IF_OIFS (ifp)); rn; rn = route_next (rn))
    {
      struct ospf_interface *oi = rn->info;
      
      if (!oi)
	continue;
      

      if (PRIORITY (oi) != OSPF_IF_PARAM (oi, priority))
	{
	  PRIORITY (oi) = OSPF_IF_PARAM (oi, priority);
	  OSPF_ISM_EVENT_SCHEDULE (oi, ISM_NeighborChange);
	}
    }
  
  return CMD_SUCCESS;
}

ALIAS (ip_ospf_priority,
       ip_ospf_priority_cmd,
       "ip ospf priority <0-255>",
       "IP Information\n"
       "OSPF interface commands\n"
       "Router priority\n"
       "Priority\n")

ALIAS (ip_ospf_priority,
       ospf_priority_cmd,
       "ospf priority <0-255>",
       "OSPF interface commands\n"
       "Router priority\n"
       "Priority\n")

DEFUN (no_ip_ospf_priority,
       no_ip_ospf_priority_addr_cmd,
       "no ip ospf priority A.B.C.D",
       NO_STR
       "IP Information\n"
       "OSPF interface commands\n"
       "Router priority\n"
       "Address of interface")
{
  struct interface *ifp = vty->index;
  struct route_node *rn;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;
  
  ifp = vty->index;
  params = IF_DEF_PARAMS (ifp);

  if (argc == 1)
    {
      ret = inet_aton(argv[0], &addr);
      if (!ret)
	{
	  vty_out (vty, "Please specify interface address by A.B.C.D%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}

      params = ospf_lookup_if_params (ifp, addr);
      if (params == NULL)
	return CMD_SUCCESS;
    }

  UNSET_IF_PARAM (params, priority);
  params->priority = OSPF_ROUTER_PRIORITY_DEFAULT;

  if (params != IF_DEF_PARAMS (ifp))
    {
      ospf_free_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }
  
  for (rn = route_top (IF_OIFS (ifp)); rn; rn = route_next (rn))
    {
      struct ospf_interface *oi = rn->info;
      
      if (!oi)
	continue;
      
      
      if (PRIORITY (oi) != OSPF_IF_PARAM (oi, priority))
	{
	  PRIORITY (oi) = OSPF_IF_PARAM (oi, priority);
	  OSPF_ISM_EVENT_SCHEDULE (oi, ISM_NeighborChange);
	}
    }
  
  return CMD_SUCCESS;
}

ALIAS (no_ip_ospf_priority,
       no_ip_ospf_priority_cmd,
       "no ip ospf priority",
       NO_STR
       "IP Information\n"
       "OSPF interface commands\n"
       "Router priority\n")

ALIAS (no_ip_ospf_priority,
       no_ospf_priority_cmd,
       "no ospf priority",
       NO_STR
       "OSPF interface commands\n"
       "Router priority\n")

DEFUN (ip_ospf_retransmit_interval,
       ip_ospf_retransmit_interval_addr_cmd,
       "ip ospf retransmit-interval <3-65535> A.B.C.D",
       "IP Information\n"
       "OSPF interface commands\n"
       "Time between retransmitting lost link state advertisements\n"
       "Seconds\n"
       "Address of interface")
{
  struct interface *ifp = vty->index;
  u_int32_t seconds;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;
      
  params = IF_DEF_PARAMS (ifp);
  seconds = strtol (argv[0], NULL, 10);

  /* Retransmit Interval range is <3-65535>. */
  if (seconds < 3 || seconds > 65535)
    {
      vty_out (vty, "Retransmit Interval is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }


  if (argc == 2)
    {
      ret = inet_aton(argv[1], &addr);
      if (!ret)
	{
	  vty_out (vty, "Please specify interface address by A.B.C.D%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}

      params = ospf_get_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  SET_IF_PARAM (params, retransmit_interval);
  params->retransmit_interval = seconds;

  return CMD_SUCCESS;
}

ALIAS (ip_ospf_retransmit_interval,
       ip_ospf_retransmit_interval_cmd,
       "ip ospf retransmit-interval <3-65535>",
       "IP Information\n"
       "OSPF interface commands\n"
       "Time between retransmitting lost link state advertisements\n"
       "Seconds\n")

ALIAS (ip_ospf_retransmit_interval,
       ospf_retransmit_interval_cmd,
       "ospf retransmit-interval <3-65535>",
       "OSPF interface commands\n"
       "Time between retransmitting lost link state advertisements\n"
       "Seconds\n")

DEFUN (no_ip_ospf_retransmit_interval,
       no_ip_ospf_retransmit_interval_addr_cmd,
       "no ip ospf retransmit-interval A.B.C.D",
       NO_STR
       "IP Information\n"
       "OSPF interface commands\n"
       "Time between retransmitting lost link state advertisements\n"
       "Address of interface")
{
  struct interface *ifp = vty->index;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;
  
  ifp = vty->index;
  params = IF_DEF_PARAMS (ifp);

  if (argc == 1)
    {
      ret = inet_aton(argv[0], &addr);
      if (!ret)
	{
	  vty_out (vty, "Please specify interface address by A.B.C.D%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}

      params = ospf_lookup_if_params (ifp, addr);
      if (params == NULL)
	return CMD_SUCCESS;
    }

  UNSET_IF_PARAM (params, retransmit_interval);
  params->retransmit_interval = OSPF_RETRANSMIT_INTERVAL_DEFAULT;

  if (params != IF_DEF_PARAMS (ifp))
    {
      ospf_free_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  return CMD_SUCCESS;
}

ALIAS (no_ip_ospf_retransmit_interval,
       no_ip_ospf_retransmit_interval_cmd,
       "no ip ospf retransmit-interval",
       NO_STR
       "IP Information\n"
       "OSPF interface commands\n"
       "Time between retransmitting lost link state advertisements\n")

ALIAS (no_ip_ospf_retransmit_interval,
       no_ospf_retransmit_interval_cmd,
       "no ospf retransmit-interval",
       NO_STR
       "OSPF interface commands\n"
       "Time between retransmitting lost link state advertisements\n")

DEFUN (ip_ospf_transmit_delay,
       ip_ospf_transmit_delay_addr_cmd,
       "ip ospf transmit-delay <1-65535> A.B.C.D",
       "IP Information\n"
       "OSPF interface commands\n"
       "Link state transmit delay\n"
       "Seconds\n"
       "Address of interface")
{
  struct interface *ifp = vty->index;
  u_int32_t seconds;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;
      
  params = IF_DEF_PARAMS (ifp);
  seconds = strtol (argv[0], NULL, 10);

  /* Transmit Delay range is <1-65535>. */
  if (seconds < 1 || seconds > 65535)
    {
      vty_out (vty, "Transmit Delay is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (argc == 2)
    {
      ret = inet_aton(argv[1], &addr);
      if (!ret)
	{
	  vty_out (vty, "Please specify interface address by A.B.C.D%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}

      params = ospf_get_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  SET_IF_PARAM (params, transmit_delay); 
  params->transmit_delay = seconds;

  return CMD_SUCCESS;
}

ALIAS (ip_ospf_transmit_delay,
       ip_ospf_transmit_delay_cmd,
       "ip ospf transmit-delay <1-65535>",
       "IP Information\n"
       "OSPF interface commands\n"
       "Link state transmit delay\n"
       "Seconds\n")

ALIAS (ip_ospf_transmit_delay,
       ospf_transmit_delay_cmd,
       "ospf transmit-delay <1-65535>",
       "OSPF interface commands\n"
       "Link state transmit delay\n"
       "Seconds\n")

DEFUN (no_ip_ospf_transmit_delay,
       no_ip_ospf_transmit_delay_addr_cmd,
       "no ip ospf transmit-delay A.B.C.D",
       NO_STR
       "IP Information\n"
       "OSPF interface commands\n"
       "Link state transmit delay\n"
       "Address of interface")
{
  struct interface *ifp = vty->index;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;
  
  ifp = vty->index;
  params = IF_DEF_PARAMS (ifp);

  if (argc == 1)
    {
      ret = inet_aton(argv[0], &addr);
      if (!ret)
	{
	  vty_out (vty, "Please specify interface address by A.B.C.D%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}

      params = ospf_lookup_if_params (ifp, addr);
      if (params == NULL)
	return CMD_SUCCESS;
    }

  UNSET_IF_PARAM (params, transmit_delay);
  params->transmit_delay = OSPF_TRANSMIT_DELAY_DEFAULT;

  if (params != IF_DEF_PARAMS (ifp))
    {
      ospf_free_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  return CMD_SUCCESS;
}

ALIAS (no_ip_ospf_transmit_delay,
       no_ip_ospf_transmit_delay_cmd,
       "no ip ospf transmit-delay",
       NO_STR
       "IP Information\n"
       "OSPF interface commands\n"
       "Link state transmit delay\n")

ALIAS (no_ip_ospf_transmit_delay,
       no_ospf_transmit_delay_cmd,
       "no ospf transmit-delay",
       NO_STR
       "OSPF interface commands\n"
       "Link state transmit delay\n")


/* ospfd's interface node. */
struct cmd_node interface_node =
{
  INTERFACE_NODE,
  "%s(config-if)# ",
  1
};

/* Initialization of OSPF interface. */
void
ospf_if_init ()
{
  /* Initialize Zebra interface data structure. */
  if_init ();
  if_add_hook (IF_NEW_HOOK, ospf_if_new_hook);
  if_add_hook (IF_DELETE_HOOK, ospf_if_delete_hook);

  /* Install interface node. */
  install_node (&interface_node, interface_config_write);

  install_element (CONFIG_NODE, &interface_cmd);
  install_default (INTERFACE_NODE);

  install_element (INTERFACE_NODE, &interface_desc_cmd);
  install_element (INTERFACE_NODE, &no_interface_desc_cmd);
  install_element (INTERFACE_NODE, &interface_ip_ospf_authentication_args_addr_cmd);
  install_element (INTERFACE_NODE, &interface_ip_ospf_authentication_args_cmd);
  install_element (INTERFACE_NODE, &interface_ip_ospf_authentication_addr_cmd);
  install_element (INTERFACE_NODE, &interface_ip_ospf_authentication_cmd);
  install_element (INTERFACE_NODE, &no_interface_ip_ospf_authentication_addr_cmd);
  install_element (INTERFACE_NODE, &no_interface_ip_ospf_authentication_cmd);
  install_element (INTERFACE_NODE, &ip_ospf_authentication_key_addr_cmd);
  install_element (INTERFACE_NODE, &ip_ospf_authentication_key_cmd);
  install_element (INTERFACE_NODE, &no_ip_ospf_authentication_key_addr_cmd);
  install_element (INTERFACE_NODE, &no_ip_ospf_authentication_key_cmd);
  install_element (INTERFACE_NODE, &ip_ospf_message_digest_key_addr_cmd);
  install_element (INTERFACE_NODE, &ip_ospf_message_digest_key_cmd);
  install_element (INTERFACE_NODE, &no_ip_ospf_message_digest_key_addr_cmd);
  install_element (INTERFACE_NODE, &no_ip_ospf_message_digest_key_cmd);
  install_element (INTERFACE_NODE, &ip_ospf_cost_addr_cmd);
  install_element (INTERFACE_NODE, &ip_ospf_cost_cmd);
  install_element (INTERFACE_NODE, &no_ip_ospf_cost_addr_cmd);
  install_element (INTERFACE_NODE, &no_ip_ospf_cost_cmd);
  install_element (INTERFACE_NODE, &ip_ospf_dead_interval_addr_cmd);
  install_element (INTERFACE_NODE, &ip_ospf_dead_interval_cmd);
  install_element (INTERFACE_NODE, &no_ip_ospf_dead_interval_addr_cmd);
  install_element (INTERFACE_NODE, &no_ip_ospf_dead_interval_cmd);
  install_element (INTERFACE_NODE, &ip_ospf_hello_interval_addr_cmd);
  install_element (INTERFACE_NODE, &ip_ospf_hello_interval_cmd);
  install_element (INTERFACE_NODE, &no_ip_ospf_hello_interval_addr_cmd);
  install_element (INTERFACE_NODE, &no_ip_ospf_hello_interval_cmd);
  install_element (INTERFACE_NODE, &ip_ospf_network_cmd);
  install_element (INTERFACE_NODE, &no_ip_ospf_network_cmd);
  install_element (INTERFACE_NODE, &ip_ospf_priority_addr_cmd);
  install_element (INTERFACE_NODE, &ip_ospf_priority_cmd);
  install_element (INTERFACE_NODE, &no_ip_ospf_priority_addr_cmd);
  install_element (INTERFACE_NODE, &no_ip_ospf_priority_cmd);
  install_element (INTERFACE_NODE, &ip_ospf_retransmit_interval_addr_cmd);
  install_element (INTERFACE_NODE, &ip_ospf_retransmit_interval_cmd);
  install_element (INTERFACE_NODE, &no_ip_ospf_retransmit_interval_addr_cmd);
  install_element (INTERFACE_NODE, &no_ip_ospf_retransmit_interval_cmd);
  install_element (INTERFACE_NODE, &ip_ospf_transmit_delay_addr_cmd);
  install_element (INTERFACE_NODE, &ip_ospf_transmit_delay_cmd);
  install_element (INTERFACE_NODE, &no_ip_ospf_transmit_delay_addr_cmd);
  install_element (INTERFACE_NODE, &no_ip_ospf_transmit_delay_cmd);

  /* These commands are compatibitliy for previous version. */
  install_element (INTERFACE_NODE, &ospf_authentication_key_cmd);
  install_element (INTERFACE_NODE, &no_ospf_authentication_key_cmd);
  install_element (INTERFACE_NODE, &ospf_message_digest_key_cmd);
  install_element (INTERFACE_NODE, &no_ospf_message_digest_key_cmd);
  install_element (INTERFACE_NODE, &ospf_cost_cmd);
  install_element (INTERFACE_NODE, &no_ospf_cost_cmd);
  install_element (INTERFACE_NODE, &ospf_dead_interval_cmd);
  install_element (INTERFACE_NODE, &no_ospf_dead_interval_cmd);
  install_element (INTERFACE_NODE, &ospf_hello_interval_cmd);
  install_element (INTERFACE_NODE, &no_ospf_hello_interval_cmd);
  install_element (INTERFACE_NODE, &ospf_network_cmd);
  install_element (INTERFACE_NODE, &no_ospf_network_cmd);
  install_element (INTERFACE_NODE, &ospf_priority_cmd);
  install_element (INTERFACE_NODE, &no_ospf_priority_cmd);
  install_element (INTERFACE_NODE, &ospf_retransmit_interval_cmd);
  install_element (INTERFACE_NODE, &no_ospf_retransmit_interval_cmd);
  install_element (INTERFACE_NODE, &ospf_transmit_delay_cmd);
  install_element (INTERFACE_NODE, &no_ospf_transmit_delay_cmd);
}
