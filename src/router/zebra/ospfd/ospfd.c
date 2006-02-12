/*
 * OSPF version 2 daemon program.
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

#include <zebra.h>

#include "thread.h"
#include "vty.h"
#include "command.h"
#include "linklist.h"
#include "prefix.h"
#include "table.h"
#include "if.h"
#include "memory.h"
#include "stream.h"
#include "log.h"
#include "sockunion.h"          /* for inet_aton () */
#include "zclient.h"

#include "ospfd/ospfd.h"
#include "ospfd/ospf_network.h"
#include "ospfd/ospf_interface.h"
#include "ospfd/ospf_ism.h"
#include "ospfd/ospf_asbr.h"
#include "ospfd/ospf_lsa.h"
#include "ospfd/ospf_lsdb.h"
#include "ospfd/ospf_neighbor.h"
#include "ospfd/ospf_nsm.h"
#include "ospfd/ospf_spf.h"
#include "ospfd/ospf_packet.h"
#include "ospfd/ospf_dump.h"
#include "ospfd/ospf_zebra.h"
#include "ospfd/ospf_abr.h"
#include "ospfd/ospf_flood.h"
#include "ospfd/ospf_route.h"
#include "ospfd/ospf_ase.h"

/* OSPF instance top. */
struct ospf *ospf_top;

extern struct zclient *zclient;

static char *ospf_network_type_str[] =
{
  "Null",
  "POINTOPOINT",
  "BROADCAST",
  "NBMA",
  "POINTOMULTIPOINT",
  "VIRTUALLINK"
};

/* Temporary Area Format Routine AREA-ID and TYPE to ASCII */
char*
ait_ntoa (struct in_addr inaddr, int type)
{

#ifdef HAVE_NSSA

	static char	buf[50];
	static char typ[10];

	typ[0]=0;

	if (type == OSPF_AREA_NSSA) strcpy (typ, "NSSA");
	if (type == OSPF_AREA_STUB) strcpy (typ, "Stub");

	if (typ[0]==0)
	sprintf (buf, "(%s)", inet_ntoa(inaddr));
	else
	sprintf (buf, "(%s [%s])", inet_ntoa(inaddr), typ);

return buf;

#else /* ! HAVE_NSSA */

return (inet_ntoa(inaddr));

#endif /* HAVE_NSSA */

}



/* Get Router ID from ospf interface list. */
struct in_addr
ospf_router_id_get (list if_list)
{
  listnode node;
  struct in_addr router_id;

  bzero (&router_id, sizeof (struct in_addr));

  for (node = listhead (if_list); node; nextnode (node))
    {
      struct ospf_interface *oi = getdata (node);

      if (!if_is_up (oi->ifp) ||
	  OSPF_IF_PARAM (oi, passive_interface) == OSPF_IF_PASSIVE)
	continue;
      
      /* Ignore virtual link interface. */
      if (oi->type != OSPF_IFTYPE_VIRTUALLINK &&
	  oi->type != OSPF_IFTYPE_LOOPBACK) 
	if (IPV4_ADDR_CMP (&router_id, &oi->address->u.prefix4) < 0)
	  router_id = oi->address->u.prefix4;
    }

  return router_id;
}

#define OSPF_EXTERNAL_LSA_ORIGINATE_DELAY 1

void
ospf_router_id_update ()
{
  listnode node;
  struct in_addr router_id, router_id_old;

  if (IS_DEBUG_OSPF_EVENT)
    zlog_info ("Router-ID[OLD:%s]: Update",inet_ntoa (ospf_top->router_id));

  router_id_old = ospf_top->router_id;

  if (ospf_top->router_id_static.s_addr != 0)
    router_id = ospf_top->router_id_static;
  else
    router_id = ospf_router_id_get (ospf_top->oiflist);

  ospf_top->router_id = router_id;
  
  if (IS_DEBUG_OSPF_EVENT)
    zlog_info ("Router-ID[NEW:%s]: Update", inet_ntoa (ospf_top->router_id));

  if (!IPV4_ADDR_SAME (&router_id_old, &router_id))
    {
      for (node = listhead (ospf_top->oiflist); node; nextnode (node))
        {
	  struct ospf_interface *oi = getdata (node);
          /* Is interface OSPF enable? */
          /* if (!ospf_if_is_enable (ifp))
             continue; */

          /* Update self-neighbor's router_id. */
          oi->nbr_self->router_id = router_id;
        }

      /* If AS-external-LSA is queued, then flush those LSAs. */
      if (router_id_old.s_addr == 0 && ospf_top->external_origin)
	{
	  int type;
	  /* Originate each redistributed external route. */
	  for (type = 0; type < ZEBRA_ROUTE_MAX; type++)
	    if (ospf_top->external_origin & (1 << type))
	      thread_add_event (master, ospf_external_lsa_originate_timer,
				NULL, type);
	  /* Originate Deafult. */
	  if (ospf_top->external_origin & (1 << ZEBRA_ROUTE_MAX))
	    thread_add_event (master, ospf_default_originate_timer,
			      &ospf_top->default_originate, 0);

	  ospf_top->external_origin = 0;
	}

      OSPF_TIMER_ON (ospf_top->t_router_lsa_update,
		     ospf_router_lsa_update_timer, OSPF_LSA_UPDATE_DELAY);
    }
}

int
ospf_router_id_update_timer (struct thread *thread)
{
  if (IS_DEBUG_OSPF_EVENT)
    zlog_info ("Router-ID: Update timer fired!");

  ospf_top->t_router_id_update = NULL;
  ospf_router_id_update ();

  return 0;
}

/* For OSPF area sort by area id. */
int
ospf_area_id_cmp (struct ospf_area *a1, struct ospf_area *a2)
{
  if (ntohl (a1->area_id.s_addr) > ntohl (a2->area_id.s_addr))
    return 1;
  if (ntohl (a1->area_id.s_addr) < ntohl (a2->area_id.s_addr))
    return -1;
  return 0;
}

/* For OSPF neighbor sort by neighbor address. */
int
ospf_nbr_static_cmp (struct ospf_nbr_static *n1, struct ospf_nbr_static *n2)
{
  if (ntohl (n1->addr.s_addr) > ntohl (n2->addr.s_addr))
    return 1;
  if (ntohl (n1->addr.s_addr) < ntohl (n2->addr.s_addr))
    return -1;
  return 0;
}

/* Allocate new ospf structure. */
struct ospf *
ospf_new ()
{
  int i;

  struct ospf *new = XMALLOC (MTYPE_OSPF_TOP, sizeof (struct ospf));
  bzero (new, sizeof (struct ospf));

  new->router_id.s_addr = htonl (0);
  new->router_id_static.s_addr = htonl (0);

  new->abr_type = OSPF_ABR_STAND;
  new->iflist = iflist;
  new->oiflist = list_new ();
  new->vlinks = list_new ();
  new->areas = list_new ();
  new->areas->cmp = (int (*)(void *, void *)) ospf_area_id_cmp;
  new->networks = (struct route_table *) route_table_init ();
  new->nbr_static = list_new ();
  new->nbr_static->cmp = (int (*)(void *, void *)) ospf_nbr_static_cmp;

  new->lsdb = ospf_lsdb_new ();

  new->default_originate = DEFAULT_ORIGINATE_NONE;

  new->new_external_route = route_table_init ();
  new->old_external_route = route_table_init ();
  new->external_lsas = route_table_init ();

  /* Distribute parameter init. */
  for (i = 0; i <= ZEBRA_ROUTE_MAX; i++)
    {
      new->dmetric[i].type = -1;
      new->dmetric[i].value = -1;
    }
  new->default_metric = -1;
  new->ref_bandwidth = OSPF_DEFAULT_REF_BANDWIDTH;

  /* SPF timer value init. */
  new->spf_delay = OSPF_SPF_DELAY_DEFAULT;
  new->spf_holdtime = OSPF_SPF_HOLDTIME_DEFAULT;

  /* MaxAge init. */
  new->maxage_lsa = list_new ();
  new->t_maxage_walker =
    thread_add_timer (master, ospf_lsa_maxage_walker,
                      NULL, OSPF_LSA_MAXAGE_CHECK_INTERVAL);

  /* Distance table init. */
  new->distance_table = route_table_init ();

  new->lsa_refresh_queue.index = 0;
  new->lsa_refresh_interval = OSPF_LSA_REFRESH_INTERVAL_DEFAULT;
  new->t_lsa_refresher = thread_add_timer (master, ospf_lsa_refresh_walker,
					   new, new->lsa_refresh_interval);
  new->lsa_refresher_started = time (NULL);

  new->fd = ospf_sock_init ();
  new->t_read = thread_add_read (master, ospf_read, new, new->fd);
  new->oi_write_q = list_new ();
  
  return new;
}


/* allocate new OSPF Area object */
struct ospf_area *
ospf_area_new (struct in_addr area_id)
{
  struct ospf_area *new;

  /* Allocate new config_network. */
  new = XMALLOC (MTYPE_OSPF_AREA, sizeof (struct ospf_area));
  bzero (new, sizeof (struct ospf_area));

  new->top = ospf_top;

  new->area_id = area_id;

  new->external_routing = OSPF_AREA_DEFAULT;
  new->default_cost = 1;
  new->auth_type = OSPF_AUTH_NULL;

  /* New LSDB init. */
  new->lsdb = ospf_lsdb_new ();

  /* Self-originated LSAs initialize. */
  new->router_lsa_self = NULL;
  /* new->summary_lsa_self = route_table_init(); */
  /* new->summary_lsa_asbr_self = route_table_init(); */

  new->oiflist = list_new ();
  new->ranges = route_table_init ();

  if (area_id.s_addr == OSPF_AREA_BACKBONE)
    ospf_top->backbone = new;

  return new;
}

void
ospf_area_free (struct ospf_area *area)
{
  /* Free LSDBs. */
  foreach_lsa (ROUTER_LSDB (area), area->lsdb, 0, ospf_lsa_discard_callback);
  foreach_lsa (NETWORK_LSDB (area), area->lsdb, 0, ospf_lsa_discard_callback);
  foreach_lsa (SUMMARY_LSDB (area), area->lsdb, 0, ospf_lsa_discard_callback);
  foreach_lsa (SUMMARY_ASBR_LSDB (area), area->lsdb, 0,
	       ospf_lsa_discard_callback);

#ifdef HAVE_NSSA
  foreach_lsa (NSSA_LSDB (area), area->lsdb, 0, ospf_lsa_discard_callback);
#endif /* HAVE_NSSA */

  ospf_lsdb_delete_all (area->lsdb);
  ospf_lsdb_free (area->lsdb);

  ospf_lsa_unlock (area->router_lsa_self);
  
  route_table_finish (area->ranges);
  list_delete (area->oiflist);

  if (EXPORT_NAME (area))
    free (EXPORT_NAME (area));

  if (IMPORT_NAME (area))
    free (IMPORT_NAME (area));

  /* Cancel timer. */
  OSPF_TIMER_OFF (area->t_router_lsa_self);

  if (OSPF_IS_AREA_BACKBONE (area))
    ospf_top->backbone = NULL;

  XFREE (MTYPE_OSPF_AREA, area);
}

void
ospf_area_check_free (struct in_addr area_id)
{
  struct ospf_area *area;

  area = ospf_area_lookup_by_area_id (area_id);
  if (area &&
      listcount (area->oiflist) == 0 &&
      area->ranges->top == NULL &&
      area->shortcut_configured == OSPF_SHORTCUT_DEFAULT &&
      area->external_routing == OSPF_AREA_DEFAULT &&
      area->no_summary == 0 &&
      area->default_cost == 1 &&
      EXPORT_NAME (area) == NULL &&
      IMPORT_NAME (area) == NULL &&
      area->auth_type == OSPF_AUTH_NULL)
    {
      listnode_delete (ospf_top->areas, area);
      ospf_area_free (area);
    }
}

struct ospf_area *
ospf_area_get (struct in_addr area_id, int format)
{
  struct ospf_area *area;
  
  area = ospf_area_lookup_by_area_id (area_id);
  if (!area)
    {
      area = ospf_area_new (area_id);
      area->format = format;
      listnode_add_sort (ospf_top->areas, area);
      ospf_check_abr_status ();  
    }

  return area;
}

struct ospf_area *
ospf_area_lookup_by_area_id (struct in_addr area_id)
{
  struct ospf_area *area;
  listnode node;

  for (node = listhead (ospf_top->areas); node; nextnode (node))
    {
      area = getdata (node);

      if (IPV4_ADDR_SAME (&area->area_id, &area_id))
        return area;
    }

  return NULL;
}

void
ospf_area_add_if (struct ospf_area *area, struct ospf_interface *oi)
{
  listnode_add (area->oiflist, oi);
}

void
ospf_area_del_if (struct ospf_area *area, struct ospf_interface *oi)
{
  listnode_delete (area->oiflist, oi);
}


/* Config network statement related functions. */
struct ospf_network *
ospf_network_new (struct in_addr area_id, int format)
{
  struct ospf_network *new;
  new = XMALLOC (MTYPE_OSPF_NETWORK, sizeof (struct ospf_network));
  bzero (new, sizeof (struct ospf_network));

  new->area_id = area_id;
  new->format = format;
  
  return new;
}

void
ospf_network_free (struct ospf_network *network)
{
  ospf_area_check_free (network->area_id);
  ospf_schedule_abr_task ();
  XFREE (MTYPE_OSPF_NETWORK, network);
}


void
ospf_network_run (struct ospf *ospf, struct prefix *p,
                    struct ospf_area *area)
{
  struct interface *ifp;
  listnode node;

  /* Get target interface. */
  for (node = listhead (ospf->iflist); node; nextnode (node))
    {
      listnode cn;
      
      if ((ifp = getdata (node)) == NULL)
	continue;

      if (memcmp (ifp->name, "VLINK", 5) == 0)
	continue;
	
      /* if interface prefix is match specified prefix,
	 then create socket and join multicast group. */
      for (cn = listhead (ifp->connected); cn; nextnode (cn))
	{
	  struct connected *co = getdata (cn);
	  struct prefix *addr;

	  if (if_is_pointopoint (ifp))
	    addr = co->destination;
	  else 
	    addr = co->address;

	  if (p->family == co->address->family &&
	      !ospf_if_is_configured (&(addr->u.prefix4)))
	    if ((if_is_pointopoint (ifp) &&
		 IPV4_ADDR_SAME (&(addr->u.prefix4), &(p->u.prefix4))) ||
		 prefix_match (p, addr)) 
	    {
	        struct ospf_interface *oi;
		
		oi = ospf_if_new (ifp, co->address);
		oi->connected = co;
		
		oi->nbr_self->address = *oi->address;

		area->act_ints++;
		oi->area = area;

		oi->params = ospf_lookup_if_params (ifp, oi->address->u.prefix4);
		oi->output_cost = ospf_if_get_output_cost (oi);
		
		if (area->external_routing != OSPF_AREA_DEFAULT)
		  UNSET_FLAG (oi->nbr_self->options, OSPF_OPTION_E);
		oi->nbr_self->priority = OSPF_IF_PARAM (oi, priority);
		
		/* Add pseudo neighbor. */
		ospf_nbr_add_self (oi);

		/* Make sure pseudo neighbor's router_id. */
		oi->nbr_self->router_id = ospf_top->router_id;
		oi->nbr_self->src = oi->address->u.prefix4;
		
		/* Relate ospf interface to ospf instance. */
		oi->ospf = ospf_top;

		/* update network type as interface flag */
		/* If network type is specified previously,
		   skip network type setting. */
		oi->type = IF_DEF_PARAMS (ifp)->type;
		
		/* Set area flag. */
		switch (area->external_routing)
		  {
		  case OSPF_AREA_DEFAULT:
		    SET_FLAG (oi->nbr_self->options, OSPF_OPTION_E);
		    break;
		  case OSPF_AREA_STUB:
		    UNSET_FLAG (oi->nbr_self->options, OSPF_OPTION_E);
		    break;
#ifdef HAVE_NSSA
		  case OSPF_AREA_NSSA:
		    UNSET_FLAG (oi->nbr_self->options, OSPF_OPTION_E);
		    SET_FLAG (oi->nbr_self->options, OSPF_OPTION_NP);
		    break;
#endif /* HAVE_NSSA */
		  }

		ospf_area_add_if (oi->area, oi);

		if (if_is_up (ifp)) 
		  ospf_if_up (oi);

		break;
	      }
	}
    }
}

void
ospf_ls_upd_queue_empty (struct ospf_interface *oi)
{
  struct route_node *rn;
  listnode node;
  list lst;
  struct ospf_lsa *lsa;

  /* empty ls update queue */
  for (rn = route_top (oi->ls_upd_queue); rn;
       rn = route_next (rn))
    if ((lst = (list) rn->info))
      {
	for (node = listhead (lst); node; nextnode (node))
	  if ((lsa = getdata (node)))
	    ospf_lsa_unlock (lsa);
	list_free (lst);
	rn->info = NULL;
      }
  
  /* remove update event */
  if (oi->t_ls_upd_event)
    {
      thread_cancel (oi->t_ls_upd_event);
      oi->t_ls_upd_event = NULL;
    }
}

void
ospf_if_update ()
{
  struct route_node *rn;
  listnode node;
  struct ospf_network *network;
  struct ospf_area *area;

  if (ospf_top != NULL)
    {
      /* Update Router ID scheduled. */
      if (ospf_top->router_id_static.s_addr == 0)
        if (ospf_top->t_router_id_update == NULL)
          {
            ospf_top->t_router_id_update =
              thread_add_timer (master, ospf_router_id_update_timer, NULL,
                                OSPF_ROUTER_ID_UPDATE_DELAY);
          }

      /* find interfaces that not configured already */
      for (node = listhead (ospf_top->oiflist); node; nextnode (node))
	{
	  int found = 0;
	  struct ospf_interface *oi = getdata (node);
	  struct connected *co = oi->connected;
	  
	  if (oi->type == OSPF_IFTYPE_VIRTUALLINK)
	    continue;
	  
	  for (rn = route_top (ospf_top->networks); rn; rn = route_next (rn))
	    {
	      if (rn->info == NULL)
		continue;
	      
	      if ((oi->type == OSPF_IFTYPE_POINTOPOINT &&
		   IPV4_ADDR_SAME (&(co->destination->u.prefix4),
				   &(rn->p.u.prefix4))) ||
		  prefix_match (&(rn->p), co->address))
		{
		  found = 1;
		  route_unlock_node (rn);
		  break;
		}
	    }

	  if (found == 0)
	    ospf_if_free (oi);
	}
	
      /* Run each interface. */
      for (rn = route_top (ospf_top->networks); rn; rn = route_next (rn))
	if (rn->info != NULL)
	  {
	    network = (struct ospf_network *) rn->info;
	    area = ospf_area_get (network->area_id, network->format);
	    ospf_network_run (ospf_top, &rn->p, area);
	  }
    }
}

int
ospf_str2area_id (char *str, struct in_addr *area_id)
{
  int ret;
  int area_id_dec;
  int format;

  if (strchr (str, '.') != NULL)
    {
      ret = inet_aton (str, area_id);
      if (!ret)
        return 0;
      format = OSPF_AREA_ID_FORMAT_ADDRESS;
    }
  else
    {
      area_id_dec = strtol (str, NULL, 10);
      if (area_id_dec < 0)
        return 0;
      area_id->s_addr = htonl (area_id_dec);
      format = OSPF_AREA_ID_FORMAT_DECIMAL;
    }

  return format;
}


/* router ospf command */
DEFUN (router_ospf,
       router_ospf_cmd,
       "router ospf",
       "Enable a routing process\n"
       "Start OSPF configuration\n")
{
#ifdef HAVE_NSSA
  zlog_info ("ROUTER OSPF:   NSSA Enabled by --enable-nssa");
#endif /* HAVE_NSSA */

  /* There is already active ospf instance. */
  if (ospf_top != NULL)
    {
      vty->node = OSPF_NODE;
      vty->index = ospf_top;
      return CMD_SUCCESS;
    }

  /* Make new ospf instance. */
  ospf_top = ospf_new ();
 
 /* Set current ospf point. */
  vty->node = OSPF_NODE;
  vty->index = ospf_top;

  if (ospf_top->router_id_static.s_addr == 0)
    ospf_router_id_update ();

  return CMD_SUCCESS;
}

void ospf_remove_vls_through_area (struct ospf_area *area);

DEFUN (no_router_ospf,
       no_router_ospf_cmd,
       "no router ospf",
       NO_STR
       "Enable a routing process\n"
       "Start OSPF configuration\n")
{
  struct route_node *rn;
  listnode node;
  int i;
  
  if (ospf_top == NULL)
    {
      vty_out (vty, "There isn't active ospf instance.%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Unredister redistribution */
  for (i = 0; i < ZEBRA_ROUTE_MAX; i++)
    ospf_redistribute_unset (i);

  for (node = listhead (ospf_top->areas); node;)
    {
      struct ospf_area *area = getdata (node);
      nextnode (node);
      
      ospf_remove_vls_through_area (area);
    }
  
  for (node = listhead (ospf_top->vlinks); node; )
    {
      struct ospf_vl_data *vl_data = node->data;
      nextnode (node);
      
      ospf_vl_delete (vl_data);
    }
  
  list_delete (ospf_top->vlinks);

  /* Reset interface. */
  for (node = listhead (ospf_top->oiflist); node;)
    {
      struct ospf_interface *oi = getdata (node);
      nextnode (node);
      
      if (oi)
	ospf_if_free (oi);
    }

  /* Clear static neighbors */
  for (node = listhead (ospf_top->nbr_static); node; nextnode (node))
    {
      struct ospf_nbr_static *nbr_static = getdata(node);


      OSPF_POLL_TIMER_OFF (nbr_static->t_poll);

      if (nbr_static->neighbor)
	{
	  nbr_static->neighbor->nbr_static = NULL;
	  nbr_static->neighbor = NULL;
	}

      if (nbr_static->oi)
	{
	  listnode_delete (nbr_static->oi->nbr_static, nbr_static);
	  nbr_static->oi = NULL;
	}

      XFREE (MTYPE_OSPF_NEIGHBOR_STATIC, nbr_static);
    }
  list_delete (ospf_top->nbr_static);

  /* Clear networks and Areas. */
  for (rn = route_top (ospf_top->networks); rn; rn = route_next (rn))
    {
      struct ospf_network *network;

      if ((network = rn->info) != NULL)
	{
	  ospf_network_free (network);
	  rn->info = NULL;
	  route_unlock_node (rn);
	}
    }

  for (node = listhead (ospf_top->areas); node;)
    {
      struct ospf_area *area = getdata (node);
      nextnode (node);
      
      listnode_delete (ospf_top->areas, area);
      ospf_area_free (area);
    }

  /* Cancel all timers. */
  OSPF_TIMER_OFF (ospf_top->t_external_lsa);
  OSPF_TIMER_OFF (ospf_top->t_router_id_update);
  OSPF_TIMER_OFF (ospf_top->t_router_lsa_update);
  OSPF_TIMER_OFF (ospf_top->t_spf_calc);
  OSPF_TIMER_OFF (ospf_top->t_ase_calc);
  OSPF_TIMER_OFF (ospf_top->t_maxage);
  OSPF_TIMER_OFF (ospf_top->t_maxage_walker);
  OSPF_TIMER_OFF (ospf_top->t_abr_task);
  OSPF_TIMER_OFF (ospf_top->t_distribute_update);
  OSPF_TIMER_OFF (ospf_top->t_lsa_refresher);
  OSPF_TIMER_OFF (ospf_top->t_read);
  OSPF_TIMER_OFF (ospf_top->t_write);

  close (ospf_top->fd);
   
  foreach_lsa (EXTERNAL_LSDB (ospf_top), ospf_top->lsdb, 0,
	       ospf_lsa_discard_callback);
  ospf_lsdb_delete_all (ospf_top->lsdb);
  ospf_lsdb_free (ospf_top->lsdb);

  for (node = listhead (ospf_top->maxage_lsa); node; nextnode (node))
    ospf_lsa_unlock (getdata (node));

  list_delete (ospf_top->maxage_lsa);

  if (ospf_top->old_table)
    ospf_route_table_free (ospf_top->old_table);
  if (ospf_top->new_table)
    {
      ospf_route_delete (ospf_top->new_table);
      ospf_route_table_free (ospf_top->new_table);
    }
  if (ospf_top->old_rtrs)
    ospf_rtrs_free (ospf_top->old_rtrs);
  if (ospf_top->new_rtrs)
    ospf_rtrs_free (ospf_top->new_rtrs);
  if (ospf_top->new_external_route)
    {
      ospf_route_delete (ospf_top->new_external_route);
      ospf_route_table_free (ospf_top->new_external_route);
    }
  if (ospf_top->old_external_route)
    {
      ospf_route_delete (ospf_top->old_external_route);
      ospf_route_table_free (ospf_top->old_external_route);
    }
  if (ospf_top->external_lsas)
    {
      ospf_ase_external_lsas_finish (ospf_top->external_lsas);
    }

  list_delete (ospf_top->areas);
  
  for (i = ZEBRA_ROUTE_SYSTEM; i <= ZEBRA_ROUTE_MAX; i++)
    if (EXTERNAL_INFO (i) != NULL)
      for (rn = route_top (EXTERNAL_INFO (i)); rn; rn = route_next (rn))
	{
	  if (rn->info == NULL)
	    continue;
	  
	  XFREE (MTYPE_OSPF_EXTERNAL_INFO, rn->info);
	  rn->info = NULL;
	  route_unlock_node (rn);
	}

  ospf_distance_reset ();
  route_table_finish (ospf_top->distance_table);

  XFREE (MTYPE_OSPF_TOP, ospf_top);

  ospf_top = NULL;

  return CMD_SUCCESS;
}

DEFUN (ospf_router_id,
       ospf_router_id_cmd,
       "ospf router-id A.B.C.D",
       "OSPF specific commands\n"
       "router-id for the OSPF process\n"
       "OSPF router-id in IP address format\n")
{
  int ret;
  struct in_addr router_id;

  ret = inet_aton (argv[0], &router_id);
  if (!ret)
    {
      vty_out (vty, "Please specify Router ID by A.B.C.D%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* ospf_top->router_id = router_id; */
  ospf_top->router_id_static = router_id;

  if (ospf_top->t_router_id_update == NULL)
    ospf_top->t_router_id_update =
      thread_add_timer (master, ospf_router_id_update_timer, NULL,
			OSPF_ROUTER_ID_UPDATE_DELAY);

  return CMD_SUCCESS;
}

ALIAS (ospf_router_id,
       router_id_cmd,
       "router-id A.B.C.D",
       "router-id for the OSPF process\n"
       "OSPF router-id in IP address format\n")

DEFUN (no_ospf_router_id,
       no_ospf_router_id_cmd,
       "no ospf router-id",
       NO_STR
       "OSPF specific commands\n"
       "router-id for the OSPF process\n")
{
  ospf_top->router_id_static.s_addr = 0;

  ospf_router_id_update ();

  return CMD_SUCCESS;
}

ALIAS (no_ospf_router_id,
       no_router_id_cmd,
       "no router-id A.B.C.D",
       NO_STR
       "router-id for the OSPF process\n"
       "OSPF router-id in IP address format\n")

DEFUN (passive_interface,
       passive_interface_addr_cmd,
       "passive-interface IFNAME A.B.C.D",
       "Suppress routing updates on an interface\n"
       "Interface's name\n")
{
 struct interface *ifp;
 struct in_addr addr;
 int ret;
 struct ospf_if_params *params;

 ifp = if_lookup_by_name (argv[0]);
 
 if (ifp == NULL)
   {
     vty_out (vty, "Please specify an existing interface%s", VTY_NEWLINE);
     return CMD_WARNING;
   }

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

  SET_IF_PARAM (params, passive_interface);
  params->passive_interface = OSPF_IF_PASSIVE;
 
 return CMD_SUCCESS;
}

ALIAS (passive_interface,
       passive_interface_cmd,
       "passive-interface IFNAME",
       "Suppress routing updates on an interface\n"
       "Interface's name\n")

DEFUN (no_passive_interface,
       no_passive_interface_addr_cmd,
       "no passive-interface IFNAME A.B.C.D",
       NO_STR
       "Allow routing updates on an interface\n"
       "Interface's name\n")
{
  struct interface *ifp;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;
    
  ifp = if_lookup_by_name (argv[0]);
  
  if (ifp == NULL)
    {
      vty_out (vty, "Please specify an existing interface%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

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

  UNSET_IF_PARAM (params, passive_interface);
  params->passive_interface = OSPF_IF_ACTIVE;
  
  if (params != IF_DEF_PARAMS (ifp))
    {
      ospf_free_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }
  
  return CMD_SUCCESS;
}

ALIAS (no_passive_interface,
       no_passive_interface_cmd,
       "no passive-interface IFNAME",
       NO_STR
       "Allow routing updates on an interface\n"
       "Interface's name\n")

DEFUN (network_area,
       network_area_cmd,
       "network A.B.C.D/M area (A.B.C.D|<0-4294967295>)",
       "Enable routing on an IP network\n"
       "OSPF network prefix\n"
       "Set the OSPF area ID\n"
       "OSPF area ID in IP address format\n"
       "OSPF area ID as a decimal value\n")
{
  int ret;
  struct prefix p;
  struct in_addr area_id;
  struct ospf *ospf;
  struct ospf_network *network;
  struct ospf_area *area;
  struct route_node *rn;
  struct external_info *ei;

  ospf = vty->index;

  /* get network prefix. */
  ret = str2prefix_ipv4 (argv[0], (struct prefix_ipv4 *) &p);
  if (ret <= 0)
    {
      vty_out (vty, "Please specify address by a.b.c.d/mask%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Make sure mask is applied. */
  apply_mask (&p);

  /* get Area ID. */
  ret = ospf_str2area_id (argv[1], &area_id);
  if (!ret)
    {
      vty_out (vty, "OSPF area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  rn = route_node_get (ospf->networks, &p);
  if (rn->info)
    {
      vty_out (vty, "There is already same network statement.%s", VTY_NEWLINE);
      route_unlock_node (rn);
      return CMD_WARNING;
    }

  network = ospf_network_new (area_id, ret);
  
  rn->info = network;

  area = ospf_area_get (area_id, ret);

  /* Run network config now. */
  ospf_network_run (ospf, &p, area);

  /* Update connected redistribute. */
  if (ospf_is_type_redistributed (ZEBRA_ROUTE_CONNECT))
    if (EXTERNAL_INFO (ZEBRA_ROUTE_CONNECT))
      for (rn = route_top (EXTERNAL_INFO (ZEBRA_ROUTE_CONNECT));
	   rn; rn = route_next (rn))
	if ((ei = rn->info) != NULL)
	  if (ospf_external_info_find_lsa (&ei->p))
	    if (!ospf_distribute_check_connected (ei))
	      ospf_external_lsa_flush (ei->type, &ei->p,
				       ei->ifindex, ei->nexthop);

  ospf_area_check_free (area_id);
  return CMD_SUCCESS;
}

void
ospf_remove_vls_through_area (struct ospf_area *area)
{
  listnode node, next;
  struct ospf_vl_data *vl_data;

  for (node = listhead (ospf_top->vlinks); node; node = next)
    {
      next = node->next;
      if ((vl_data = getdata (node)) != NULL)
	if (IPV4_ADDR_SAME (&vl_data->vl_area_id, &area->area_id))
	  ospf_vl_delete (vl_data);
    }
}


DEFUN (no_network_area,
       no_network_area_cmd,
       "no network A.B.C.D/M area A.B.C.D",
       NO_STR
       "Enable routing on an IP network\n"
       "OSPF network prefix\n"
       "Set the OSPF area ID\n"
       "OSPF area ID in IP address format\n")
{
  int ret;
  struct ospf *ospf;
  struct prefix_ipv4 p;
  struct in_addr area_id;
  struct route_node *rn;
  struct ospf_network *network;
  struct external_info *ei;

  ospf = (struct ospf *) vty->index;

  ret = str2prefix_ipv4 (argv[0], &p);
  if (ret <= 0)
    {
      vty_out (vty, "Please specify address by a.b.c.d/mask%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (!ospf_str2area_id (argv[1], &area_id))
    {
      vty_out (vty, "OSPF area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  apply_mask_ipv4 (&p);

  rn = route_node_get (ospf->networks, (struct prefix *) &p);
  if (!rn->info)
    {
      vty_out (vty, "Can't find specified network area configuration.%s",
               VTY_NEWLINE);
      route_unlock_node (rn);
      return CMD_WARNING;
    }

  network = rn->info;
  if (!IPV4_ADDR_SAME (&area_id, &network->area_id))
    {
      zlog_warn ("Area ID not match, do nothing");
      return CMD_WARNING;
    }

  ospf_network_free (rn->info);
  rn->info = NULL;
  route_unlock_node (rn);

  ospf_if_update ();
  
  /* Update connected redistribute. */
  if (ospf_is_type_redistributed (ZEBRA_ROUTE_CONNECT))
    if (EXTERNAL_INFO (ZEBRA_ROUTE_CONNECT))
      for (rn = route_top (EXTERNAL_INFO (ZEBRA_ROUTE_CONNECT));
	   rn; rn = route_next (rn))
	if ((ei = rn->info) != NULL)
	  if (!ospf_external_info_find_lsa (&ei->p))
	    if (ospf_distribute_check_connected (ei))
	      ospf_external_lsa_originate (ei);

  return CMD_SUCCESS;
}

ALIAS (no_network_area,
       no_network_area_decimal_cmd,
       "no network A.B.C.D/M area <0-4294967295>",
       NO_STR
       "Enable routing on an IP network\n"
       "OSPF network prefix\n"
       "Set the OSPF area ID\n"
       "OSPF area ID as a decimal value\n")

/* OSPF area range allocation.  */
struct ospf_area_range *
ospf_new_area_range (struct ospf_area * area,
                     struct prefix_ipv4 *p)
{
  struct ospf_area_range *range;
  struct route_node *node;

  node = route_node_get (area->ranges, (struct prefix *) p);
  if (node->info)
    {
      route_unlock_node (node);
      return node->info;
    }

  range = XMALLOC (MTYPE_OSPF_AREA_RANGE, sizeof (struct ospf_area_range));
  bzero (range, sizeof (struct ospf_area_range));
  range->node = node;
  node->info = range;

  return range;
}


int
ospf_area_range_set (struct vty *vty, int argc, char **argv, int not_advertise)
{
  struct ospf_area *area;
  struct in_addr area_id;
  struct prefix_ipv4 p;
  int ret;
  struct ospf_area_range *range;

  ret = ospf_str2area_id (argv[0], &area_id);
  if (!ret)
    {
      vty_out (vty, "OSPF Area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  area = ospf_area_get (area_id, ret);

  ret = str2prefix_ipv4 (argv[1], &p);
  if (ret <= 0)
    {
      vty_out (vty, "Please specify area range as a.b.c.d/mask%s",
               VTY_NEWLINE);
      return CMD_WARNING;
    }

  range = ospf_new_area_range (area, &p); /* may get new or old range */

  if (not_advertise) /* set suppress */
    SET_FLAG (range->flags, OSPF_RANGE_SUPPRESS);
  else
    UNSET_FLAG (range->flags, OSPF_RANGE_SUPPRESS);

  ospf_schedule_abr_task ();
  return CMD_SUCCESS;
}

DEFUN (area_range,
       area_range_cmd,
       "area A.B.C.D range A.B.C.D/M",
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "Configure OSPF area range for route summarization\n"
       "area range prefix\n")
{
  return ospf_area_range_set (vty, argc, argv, 0); /* Advertise=0 */
}

ALIAS (area_range,
       area_range_decimal_cmd,
       "area <0-4294967295> range A.B.C.D/M",
       "OSPF area parameters\n"
       "OSPF area ID as a decimal value\n"
       "Configure OSPF area range for route summarization\n"
       "area range prefix\n")

ALIAS (area_range,
       area_range_advertise_cmd,
       "area A.B.C.D range A.B.C.D/M advertise",
       "OSPF area parameters\n"
       "OSPF area ID\n"
       "OSPF area range for route advertise (default)\n"
       "area range prefix\n"
       "advertise this range\n")

ALIAS (area_range,
       area_range_advertise_decimal_cmd,
       "area <0-4294967295> range A.B.C.D/M advertise",
       "OSPF area parameters\n"
       "OSPF area ID\n"
       "Configure OSPF DECIMAL area range for route summarization\n"
       "area range prefix\n"
       "advertise this range\n")

/* Change syntax from suppress to not-advertise.  */
DEFUN (area_range_notadvertise,
       area_range_notadvertise_cmd,
       "area A.B.C.D range A.B.C.D/M not-advertise",
       "OSPF area parameters\n"
       "OSPF area ID\n"
       "OSPF area range for route DoNotAdvertise\n"
       "area range prefix\n"
       "do not advertise this range\n")
{
  return ospf_area_range_set (vty, argc, argv, 1); /* DoNotAdvertise=1 */
}

ALIAS (area_range_notadvertise,
       area_range_notadvertise_decimal_cmd,
       "area <0-4294967295> range A.B.C.D/M not-advertise",
       "OSPF area parameters\n"
       "OSPF area ID\n"
       "Configure OSPF DECIMAL area range for DoNotAdvertise\n"
       "area range prefix\n"
       "do not advertise this range\n")

int
ospf_area_range_unset (struct vty *vty, int argc, char **argv)
{
  struct ospf_area *area;
  struct in_addr area_id;
  struct prefix_ipv4 p;
  struct route_node *node;
  int ret;

  ret = ospf_str2area_id (argv[0], &area_id);
  if (!ret)
    {
      vty_out (vty, "OSPF area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  area = ospf_area_lookup_by_area_id (area_id);
  if (!area)
    {
      vty_out (vty, "Area does not exist.%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  ret = str2prefix_ipv4 (argv[1], &p);
  if (ret <= 0)
    {
      vty_out (vty, "Please specify area range as a.b.c.d/mask%s",
               VTY_NEWLINE);
      return CMD_WARNING;
    }

  node = route_node_lookup (area->ranges, (struct prefix*) &p);
  if (node == NULL)
    {
      vty_out (vty, "Specified area range was not configured%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  XFREE (MTYPE_OSPF_AREA_RANGE, node->info);
  node->info = NULL;

  route_unlock_node (node);
  ospf_area_check_free (area_id);
  ospf_schedule_abr_task ();

  return CMD_SUCCESS;
}

DEFUN (no_area_range,
       no_area_range_cmd,
       "no area A.B.C.D range A.B.C.D/M",
       NO_STR
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "Deconfigure OSPF area range for route summarization\n"
       "area range prefix\n")
{
  return ospf_area_range_unset (vty, argc, argv);
}

ALIAS (no_area_range,
       no_area_range_decimal_cmd,
       "no area <0-4294967295> range A.B.C.D/M",
       NO_STR
       "OSPF area parameters\n"
       "OSPF area ID as a decimal value\n"
       "Deconfigure OSPF area range for route summarization\n"
       "area range prefix\n")

ALIAS (no_area_range,
       no_area_range_advertise_cmd,
       "no area A.B.C.D range A.B.C.D/M advertise",
       NO_STR
       "OSPF area parameters\n"
       "OSPF area ID\n"
       "OSPF area range for route advertise (default)\n"
       "area range prefix\n"
       "advertise this range\n")

ALIAS (no_area_range,
       no_area_range_advertise_decimal_cmd,
       "no area <0-4294967295> range A.B.C.D/M advertise",
       NO_STR
       "OSPF area parameters\n"
       "OSPF area ID\n"
       "Configure OSPF DECIMAL area range for route summarization\n"
       "area range prefix\n"
       "advertise this range\n")

ALIAS (no_area_range,
       no_area_range_notadvertise_cmd,
       "no area A.B.C.D range A.B.C.D/M not-advertise",
       NO_STR
       "OSPF area parameters\n"
       "OSPF area ID\n"
       "OSPF area range for route DoNotAdvertise\n"
       "area range prefix\n"
       "do not advertise this range\n")

ALIAS (no_area_range,
       no_area_range_notadvertise_decimal_cmd,
       "no area <0-4294967295> range A.B.C.D/M not-advertise",
       NO_STR
       "OSPF area parameters\n"
       "OSPF area ID\n"
       "Configure OSPF DECIMAL area range for DoNotAdvertise\n"
       "area range prefix\n"
       "do not advertise this range\n")

DEFUN (area_range_subst,
       area_range_subst_cmd,
       "area A.B.C.D range IPV4_PREFIX substitute IPV4_PREFIX",
       "OSPF area parameters\n"
       "OSPF area ID\n"
       "configure OSPF area range for route summarization\n"
       "area range prefix\n"
       "announce area range as another prefix\n"
       "network prefix to be announced instead of range\n")
{
  struct ospf_area *area;
  struct ospf_area_range *range;
  struct in_addr area_id;
  struct prefix_ipv4 p, subst;
  int ret;

  ret = ospf_str2area_id (argv[0], &area_id);
  if (!ret)
    {
      vty_out (vty, "OSPF Area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  area = ospf_area_get (area_id, ret);

  ret = str2prefix_ipv4 (argv[1], &p);
  if (ret <= 0)
    {
      vty_out (vty, "Please specify area range as a.b.c.d/mask%s",
               VTY_NEWLINE);
      return CMD_WARNING;
    }

  ret = str2prefix_ipv4 (argv[2], &subst);
  if (ret <= 0)
    {
      vty_out (vty, "Please specify network prefix as a.b.c.d/mask%s",
               VTY_NEWLINE);
      return CMD_WARNING;
    }

  range = ospf_new_area_range (area, &p);

  if (CHECK_FLAG (range->flags, OSPF_RANGE_SUPPRESS))
    {
      vty_out (vty, "The same area range is configured as suppress%s",
               VTY_NEWLINE);
      return CMD_WARNING;
    }
  
  SET_FLAG (range->flags, OSPF_RANGE_SUBST);
  range->substitute = subst;

  ospf_schedule_abr_task ();
  return CMD_SUCCESS;
}


DEFUN (no_area_range_subst,
       no_area_range_subst_cmd,
       "no area A.B.C.D range IPV4_PREFIX substitute IPV4_PREFIX",
       NO_STR
       "OSPF area parameters\n"
       "OSPF area ID\n"
       "Deconfigure OSPF area range for route summarization\n"
       "area range prefix\n"
       "Do not advertise this range\n"
       "Announce area range as another prefix\n"
       "Network prefix to be announced instead of range\n")
{
  struct ospf_area *area;
  struct ospf_area_range *range;
  struct in_addr area_id;
  struct prefix_ipv4 p, subst;
  struct route_node *node;
  int ret;

  ret = ospf_str2area_id (argv[0], &area_id);
  if (!ret)
    {
      vty_out (vty, "OSPF Area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  area = ospf_area_lookup_by_area_id (area_id);
  if (!area)
    {
      vty_out (vty, "Area does not exist%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  ret = str2prefix_ipv4 (argv[1], &p);
  if (ret <= 0)
    {
      vty_out (vty, "Please specify area range as a.b.c.d/mask%s",
               VTY_NEWLINE);
      return CMD_WARNING;
    }

  node = route_node_lookup (area->ranges, (struct prefix *) &p);
  if (node == NULL)
    {
      vty_out (vty, "Specified area range was not configured%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  range = (struct ospf_area_range *) node->info;

  ret = str2prefix_ipv4 (argv[2], &subst);
  if (ret <= 0)
    {
      vty_out (vty, "Please specify network prefix as a.b.c.d/mask%s",
               VTY_NEWLINE);
      return CMD_WARNING;
    }
  
  UNSET_FLAG (range->flags, OSPF_RANGE_SUBST);

  ospf_schedule_abr_task ();

  return CMD_SUCCESS;
}

#if OLD_VLINK_CMD /* This is a train wreck and not extensible */

/* THIS is a MESS!!!!   Cleaning up - MAG !!*/


struct ospf_vl_data *
ospf_find_vl_data (struct in_addr area_id, int format,
		   struct in_addr vl_peer, struct vty *vty)
{
  struct ospf_area *area;
  struct ospf_vl_data *vl_data;

  if (area_id.s_addr == OSPF_AREA_BACKBONE)
    {
      vty_out (vty, "Configuring VLs over the backbone is not allowed%s",
               VTY_NEWLINE);
      return NULL;
    }
  area = ospf_area_get (area_id, format);

  if (area->external_routing != OSPF_AREA_DEFAULT)
    {
      if (format == OSPF_AREA_ID_FORMAT_ADDRESS)
	vty_out (vty, "Area %s is %s%s",
		 inet_ntoa (area_id),
#ifdef HAVE_NSSA
		 area->external_routing == OSPF_AREA_NSSA?"nssa":"stub",
#else
		 "stub",
#endif /* HAVE_NSSA */		 
		 VTY_NEWLINE);
      else
	vty_out (vty, "Area %d is %s%s",
		 ntohl (area_id.s_addr),
#ifdef HAVE_NSSA
		 area->external_routing == OSPF_AREA_NSSA?"nssa":"stub",
#else
		 "stub",
#endif /* HAVE_NSSA */		 
		 VTY_NEWLINE);	
      return NULL;
    }
  
  if ((vl_data = ospf_vl_lookup (area, vl_peer)) == NULL)
    {
      vl_data = ospf_vl_data_new (area, vl_peer);
      if (vl_data->vl_oi == NULL)
	{
	  vl_data->vl_oi = ospf_vl_new (vl_data);
	  ospf_vl_add (vl_data);
	  ospf_spf_calculate_schedule ();
	}
    }
  return vl_data;
}


int
ospf_vl_set_security (struct in_addr area_id, int format,
		      struct in_addr vl_peer,  char *key, u_char key_id,
		     char *md5_key, struct vty *vty)
{
  struct crypt_key *ck;
  struct ospf_vl_data *vl_data;

  vl_data = ospf_find_vl_data (area_id, format, vl_peer, vty);
  if (!vl_data)
    return CMD_WARNING;
  
  if (key)
    strncpy (IF_DEF_PARAMS (vl_data->vl_oi->ifp)->auth_simple, key, OSPF_AUTH_SIMPLE_SIZE);
  else if (md5_key)
    {
      if (ospf_crypt_key_lookup (IF_DEF_PARAMS (vl_data->vl_oi->ifp)->auth_crypt,
				 key_id) != NULL)
	{
	  vty_out (vty, "OSPF: Key %d already exists%s",
		   key_id, VTY_NEWLINE);
	  return CMD_WARNING;
	}
      ck = ospf_crypt_key_new ();
      ck->key_id = key_id;
      strncpy (ck->auth_key, md5_key, OSPF_AUTH_MD5_SIZE);
      
      ospf_crypt_key_add (IF_DEF_PARAMS (vl_data->vl_oi->ifp)->auth_crypt, ck);
    }
  
  return CMD_SUCCESS;
}



int
ospf_vl_set_timers (struct in_addr area_id, int format,
		     struct in_addr vl_peer, u_int16_t hello_interval,
		     u_int16_t retransmit_interval, u_int16_t transmit_delay,
		     u_int16_t dead_interval, struct vty *vty)
{
  struct ospf_vl_data *vl_data;
  struct interface *ifp;
  
  vl_data = ospf_find_vl_data (area_id, format, vl_peer, vty);
  if (!vl_data)
    return CMD_WARNING;

  ifp = vl_data->vl_oi->ifp;
  
  SET_IF_PARAM (IF_DEF_PARAMS (ifp), transmit_delay);
  IF_DEF_PARAMS (ifp)->transmit_delay = transmit_delay;
  
  SET_IF_PARAM (IF_DEF_PARAMS (ifp), retransmit_interval);
  IF_DEF_PARAMS (ifp)->retransmit_interval = retransmit_interval;

  SET_IF_PARAM (IF_DEF_PARAMS (ifp), v_hello);
  IF_DEF_PARAMS (ifp)->v_hello = hello_interval;

  SET_IF_PARAM (IF_DEF_PARAMS (ifp), v_wait);
  IF_DEF_PARAMS (ifp)->v_wait = dead_interval;

  return CMD_SUCCESS;
}

DEFUN (area_vlink,
       area_vlink_cmd,
       "area A.B.C.D virtual-link A.B.C.D",
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "Configure a virtual link\n"
       "Router ID of the remote ABR\n")
{
  struct in_addr area_id, vl_peer;
  int ret, format;

  format = ospf_str2area_id (argv[0], &area_id);
  if (!format)
    {
      vty_out (vty, "OSPF area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  ret = inet_aton (argv[1], &vl_peer);
  if (! ret)
    {
      vty_out (vty, "Please specify valid Router ID as a.b.c.d%s",
               VTY_NEWLINE);
      return CMD_WARNING;
    }

  return ospf_vl_set_timers (area_id, format, vl_peer,
			   OSPF_HELLO_INTERVAL_DEFAULT,
			   OSPF_RETRANSMIT_INTERVAL_DEFAULT,
			   OSPF_TRANSMIT_DELAY_DEFAULT,
			   OSPF_ROUTER_DEAD_INTERVAL_DEFAULT, vty);
}

ALIAS (area_vlink,
       area_vlink_decimal_cmd,
       "area <0-4294967295> virtual-link A.B.C.D",
       "OSPF area parameters\n"
       "OSPF area ID as a decimal value\n"
       "Configure a virtual link\n"
       "Router ID of the remote ABR\n")

DEFUN (area_vlink_param,
       area_vlink_param_cmd,
       "area A.B.C.D virtual-link A.B.C.D hello-interval <1-65535> retransmit-interval <3-65535> transmit-delay <1-65535> dead-interval <1-65535>",
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "Configure a virtual link\n"
       "Router ID of the remote ABR\n"
       "Time between HELLO packets\n"
       "Seconds\n"
       "Time between retransmitting lost link state advertisements\n"
       "Seconds\n"       
       "Link state transmit delay\n"
       "Seconds\n"
       "Interval after which a neighbor is declared dead\n"
       "Seconds\n")
{
  struct in_addr area_id, vl_peer;
  int ret, format;
  int hello, retransmit, transmit, dead;

  format = ospf_str2area_id (argv[0], &area_id);
  if (!format)
    {
      vty_out (vty, "OSPF area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  ret = inet_aton (argv[1], &vl_peer);
  if (! ret)
    {
      vty_out (vty, "Please specify valid Router ID as a.b.c.d%s",
               VTY_NEWLINE);
      return CMD_WARNING;
    }

  hello = strtol (argv[2], NULL, 10);
  if (hello < 0)
    return CMD_WARNING;
  retransmit = strtol (argv[3], NULL, 10);
  if (retransmit < 0)
    return CMD_WARNING;
  transmit = strtol (argv[4], NULL, 10);
  if (transmit < 0)
    return CMD_WARNING;
  dead = strtol (argv[5], NULL, 10);
  if (dead < 0)
    return CMD_WARNING;

  return ospf_vl_set_timers (area_id, format, vl_peer, hello, retransmit,
			     transmit, dead, vty);
}      

ALIAS (area_vlink_param,
       area_vlink_param_decimal_cmd,
       "area <0-4294967295> virtual-link A.B.C.D hello-interval <1-65535> retransmit-interval <3-65535> transmit-delay <1-65535> dead-interval <1-65535>",
       "OSPF area parameters\n"
       "OSPF area ID as a decimal value\n"
       "Configure a virtual link\n"
       "Router ID of the remote ABR\n"
       "Time between HELLO packets\n"
       "Seconds\n"
       "Time between retransmitting lost link state advertisements\n"
       "Seconds\n"       
       "Link state transmit delay\n"
       "Seconds\n"
       "Interval after which a neighbor is declared dead\n"
       "Seconds\n")



DEFUN (area_vlink_param_auth,
       area_vlink_param_auth_cmd,
       "area A.B.C.D virtual-link A.B.C.D hello-interval <1-65535> retransmit-interval <3-65535> transmit-delay <1-65535> dead-interval <1-65535> authentication-key AUTH_KEY",
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "Configure a virtual link\n"
       "Router ID of the remote ABR\n"
       "Time between HELLO packets\n"
       "Seconds\n"
       "Time between retransmitting lost link state advertisements\n"
       "Seconds\n"       
       "Link state transmit delay\n"
       "Seconds\n"
       "Interval after which a neighbor is declared dead\n"
       "Seconds\n"
       "Authentication password (key)\n"
       "The OSPF password (key)")
{
  struct in_addr area_id, vl_peer;
  int ret, format;
  int hello, retransmit, transmit, dead;
  char key[OSPF_AUTH_SIMPLE_SIZE+1];

  format = ospf_str2area_id (argv[0], &area_id);
  if (!format)
    {
      vty_out (vty, "OSPF area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  ret = inet_aton (argv[1], &vl_peer);
  if (! ret)
    {
      vty_out (vty, "Please specify valid Router ID as a.b.c.d%s",
               VTY_NEWLINE);
      return CMD_WARNING;
    }

  hello = strtol (argv[2], NULL, 10);
  if (hello < 0)
    return CMD_WARNING;
  retransmit = strtol (argv[3], NULL, 10);
  if (retransmit < 0)
    return CMD_WARNING;
  transmit = strtol (argv[4], NULL, 10);
  if (transmit < 0)
    return CMD_WARNING;
  dead = strtol (argv[5], NULL, 10);
  if (dead < 0)
    return CMD_WARNING;

  bzero (key, OSPF_AUTH_SIMPLE_SIZE + 1);
  strncpy (key, argv[6], OSPF_AUTH_SIMPLE_SIZE);

  ret = ospf_vl_set_timers (area_id, format, vl_peer, hello, retransmit,
			   transmit, dead, vty);
  if (ret != CMD_SUCCESS)
    return ret;
  return ospf_vl_set_security (area_id, format, vl_peer, key, 0, NULL, vty);
}      

ALIAS (area_vlink_param_auth,
       area_vlink_param_auth_decimal_cmd,
       "area <0-4294967295> virtual-link A.B.C.D hello-interval <1-65535> retransmit-interval <3-65535> transmit-delay <1-65535> dead-interval <1-65535> authentication-key AUTH_KEY",
       "OSPF area parameters\n"
       "OSPF area ID as a decimal value\n"
       "Configure a virtual link\n"
       "Router ID of the remote ABR\n"
       "Time between HELLO packets\n"
       "Seconds\n"
       "Time between retransmitting lost link state advertisements\n"
       "Seconds\n"       
       "Link state transmit delay\n"
       "Seconds\n"
       "Interval after which a neighbor is declared dead\n"
       "Seconds\n"
       "Authentication password (key)\n"
       "The OSPF password (key)")
     

DEFUN (area_vlink_param_md5,
       area_vlink_param_md5_cmd,
       "area A.B.C.D virtual-link A.B.C.D hello-interval <1-65535> retransmit-interval <3-65535> transmit-delay <1-65535> dead-interval <1-65535> message-digest-key <1-255> md5 KEY",
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "Configure a virtual link\n"
       "Router ID of the remote ABR\n"
       "Time between HELLO packets\n"
       "Seconds\n"
       "Time between retransmitting lost link state advertisements\n"
       "Seconds\n"       
       "Link state transmit delay\n"
       "Seconds\n"
       "Interval after which a neighbor is declared dead\n"
       "Seconds\n"
       "Message digest authentication password (key)\n"
       "Key ID\n"
       "Use MD5 algorithm\n"
       "The OSPF password (key)")
{
  struct in_addr area_id, vl_peer;
  int ret, format, key_id;
  int hello, retransmit, transmit, dead;
  char key[OSPF_AUTH_MD5_SIZE+1];

  format = ospf_str2area_id (argv[0], &area_id);
  if (!format)
    {
      vty_out (vty, "OSPF area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  ret = inet_aton (argv[1], &vl_peer);
  if (! ret)
    {
      vty_out (vty, "Please specify valid Router ID as a.b.c.d%s",
               VTY_NEWLINE);
      return CMD_WARNING;
    }

  hello = strtol (argv[2], NULL, 10);
  if (hello < 0)
    return CMD_WARNING;
  retransmit = strtol (argv[3], NULL, 10);
  if (retransmit < 0)
    return CMD_WARNING;
  transmit = strtol (argv[4], NULL, 10);
  if (transmit < 0)
    return CMD_WARNING;
  dead = strtol (argv[5], NULL, 10);
  if (dead < 0)
    return CMD_WARNING;

  key_id = strtol (argv[6], NULL, 10);
  if (key_id < 0)
    return CMD_WARNING;
  bzero (key, OSPF_AUTH_MD5_SIZE + 1);
  strncpy (key, argv[7], OSPF_AUTH_MD5_SIZE);

  ret = ospf_vl_set_timers (area_id, format, vl_peer, hello, retransmit,
			   transmit, dead, vty);
  if (ret != CMD_SUCCESS)
    return ret;
  return ospf_vl_set_security (area_id, format, vl_peer, NULL,
			       key_id, key, vty);
}      

ALIAS (area_vlink_param_md5,
       area_vlink_param_md5_decimal_cmd,
       "area <0-4294967295> virtual-link A.B.C.D hello-interval <1-65535> retransmit-interval <3-65535> transmit-delay <1-65535> dead-interval <1-65535> message-digest-key <1-255> md5 KEY",
       "OSPF area parameters\n"
       "OSPF area ID as a decimal value\n"
       "Configure a virtual link\n"
       "Router ID of the remote ABR\n"
       "Time between HELLO packets\n"
       "Seconds\n"
       "Time between retransmitting lost link state advertisements\n"
       "Seconds\n"       
       "Link state transmit delay\n"
       "Seconds\n"
       "Interval after which a neighbor is declared dead\n"
       "Seconds\n"
       "Message digest authentication password (key)\n"
       "Key ID\n"
       "Use MD5 algorithm\n"
       "The OSPF password (key)")


DEFUN (area_vlink_md5,
       area_vlink_md5_cmd,
       "area A.B.C.D virtual-link A.B.C.D message-digest-key <1-255> md5 KEY",
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "Configure a virtual link\n"
       "Router ID of the remote ABR\n"
       "Message digest authentication password (key)\n"
       "Key ID\n"
       "Use MD5 algorithm\n"
       "The OSPF password (key)")
{
  struct in_addr area_id, vl_peer;
  int ret, format, key_id;
  char key[OSPF_AUTH_MD5_SIZE+1];

  format = ospf_str2area_id (argv[0], &area_id);
  if (!format)
    {
      vty_out (vty, "OSPF area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  ret = inet_aton (argv[1], &vl_peer);
  if (! ret)
    {
      vty_out (vty, "Please specify valid Router ID as a.b.c.d%s",
               VTY_NEWLINE);
      return CMD_WARNING;
    }

  key_id = strtol (argv[2], NULL, 10);
  if (key_id < 0)
    return CMD_WARNING;
  bzero (key, OSPF_AUTH_MD5_SIZE + 1);
  strncpy (key, argv[3], OSPF_AUTH_MD5_SIZE);

  return ospf_vl_set_security (area_id, format, vl_peer,
			       NULL, key_id, key, vty);
}

ALIAS (area_vlink_md5,
       area_vlink_md5_decimal_cmd,
       "area <0-4294967295> virtual-link A.B.C.D message-digest-key <1-255> md5 KEY",
       "OSPF area parameters\n"
       "OSPF area ID as a decimal value\n"
       "Configure a virtual link\n"
       "Router ID of the remote ABR\n"
       "Message digest authentication password (key)\n"
       "Key ID\n"
       "Use MD5 algorithm\n"
       "The OSPF password (key)")

DEFUN (area_vlink_auth,
       area_vlink_auth_cmd,
       "area A.B.C.D virtual-link A.B.C.D authentication-key AUTH_KEY",
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "Configure a virtual link\n"
       "Router ID of the remote ABR\n"
       "Authentication password (key)\n"
       "The OSPF password (key)")
{
  struct in_addr area_id, vl_peer;
  int ret, format;
  char key[OSPF_AUTH_SIMPLE_SIZE+1];

  format = ospf_str2area_id (argv[0], &area_id);
  if (!format)
    {
      vty_out (vty, "OSPF area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  ret = inet_aton (argv[1], &vl_peer);
  if (! ret)
    {
      vty_out (vty, "Please specify valid Router ID as a.b.c.d%s",
               VTY_NEWLINE);
      return CMD_WARNING;
    }

  bzero (key, OSPF_AUTH_SIMPLE_SIZE + 1);
  strncpy (key, argv[2], OSPF_AUTH_SIMPLE_SIZE);

  return ospf_vl_set_security (area_id, format, vl_peer,
			       key, 0, NULL, vty);
}

ALIAS (area_vlink_auth,
       area_vlink_auth_decimal_cmd,
       "area <0-4294967295> virtual-link A.B.C.D authentication-key AUTH_KEY",
       "OSPF area parameters\n"
       "OSPF area ID as a decimal value\n"
       "Configure a virtual link\n"
       "Router ID of the remote ABR\n"
       "Authentication password (key)\n"
       "The OSPF password (key)")

     

DEFUN (no_area_vlink,
       no_area_vlink_cmd,
       "no area A.B.C.D virtual-link A.B.C.D",
       NO_STR
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "Configure a virtual link\n"
       "Router ID of the remote ABR\n")
{
  struct ospf_area *area;
  struct in_addr area_id, vl_peer;
  struct ospf_vl_data *vl_data = NULL;
  int ret;

  ret = ospf_str2area_id (argv[0], &area_id);
  if (!ret)
    {
      vty_out (vty, "OSPF area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  area = ospf_area_lookup_by_area_id (area_id);
  if (!area)
    {
      vty_out (vty, "Area does not exist", VTY_NEWLINE);
      return CMD_WARNING;
    }

  ret = inet_aton (argv[1], &vl_peer);
  if (! ret)
    {
      vty_out (vty, "Please specify valid Router ID as a.b.c.d%s",
               VTY_NEWLINE);
      return CMD_WARNING;
    }

  if ((vl_data = ospf_vl_lookup (area, vl_peer)))
    ospf_vl_delete (vl_data);

  ospf_area_check_free (area_id);
  
  return CMD_SUCCESS;
}

ALIAS (no_area_vlink,
       no_area_vlink_decimal_cmd,
       "no area <0-4294967295> virtual-link A.B.C.D",
       NO_STR
       "OSPF area parameters\n"
       "OSPF area ID as a decimal value\n"
       "Configure a virtual link\n"
       "Router ID of the remote ABR\n")

#else /* OLD_VLINK_CMD */

     /* Command Handler Logic in VLink stuff is delicate!!

	ALTER AT YOUR OWN RISK!!!!

	Various dummy values are used to represent 'NoChange' state for
	VLink configuration NOT being changed by a VLink command, and
	special syntax is used within the command strings so that the
	typed in command verbs can be seen in the configuration command
	bacckend handler.  This is to drastically reduce the verbeage
	required to coe up with a reasonably compatible Cisco VLink command

	- Matthew Grant <grantma@anathoth.gen.nz> 
	Wed, 21 Feb 2001 15:13:52 +1300
     */


/* Configuration data for virtual links 
 */ 
struct ospf_vl_config_data {
  struct vty *vty;		/* vty stuff */
  struct in_addr area_id;	/* area ID from command line */
  int format;			/* command line area ID format */
  struct in_addr vl_peer;	/* command line vl_peer */
  int auth_type;		/* Authehntication type, if given */
  char *auth_key;		/* simple password if present */
  int crypto_key_id;		/* Cryptographic key ID */
  char *md5_key;		/* MD5 authentication key */
  int hello_interval;	        /* Obvious what these are... */
  int retransmit_interval; 
  int transmit_delay;
  int dead_interval;
};

void
ospf_vl_config_data_init (struct ospf_vl_config_data *vl_config, 
			  struct vty *vty)
{
  memset (vl_config, 0, sizeof (struct ospf_vl_config_data));
  vl_config->auth_type = OSPF_AUTH_CMD_NOTSEEN;
  vl_config->vty = vty;
}

struct ospf_vl_data *
ospf_find_vl_data (struct ospf_vl_config_data *vl_config)
{
  struct ospf_area *area;
  struct ospf_vl_data *vl_data;
  struct vty *vty;
  struct in_addr area_id;

  vty = vl_config->vty;
  area_id = vl_config->area_id;

  if (area_id.s_addr == OSPF_AREA_BACKBONE)
    {
      vty_out (vty, 
	       "Configuring VLs over the backbone is not allowed%s",
               VTY_NEWLINE);
      return NULL;
    }
  area = ospf_area_get (area_id, vl_config->format);

  if (area->external_routing != OSPF_AREA_DEFAULT)
    {
      if (vl_config->format == OSPF_AREA_ID_FORMAT_ADDRESS)
	vty_out (vty, "Area %s is %s%s",
		 inet_ntoa (area_id),
#ifdef HAVE_NSSA
		 area->external_routing == OSPF_AREA_NSSA?"nssa":"stub",
#else
		 "stub",
#endif /* HAVE_NSSA */		 
		 VTY_NEWLINE);
      else
	vty_out (vty, "Area %d is %s%s",
		 ntohl (area_id.s_addr),
#ifdef HAVE_NSSA
		 area->external_routing == OSPF_AREA_NSSA?"nssa":"stub",
#else
		 "stub",
#endif /* HAVE_NSSA */		 
		 VTY_NEWLINE);	
      return NULL;
    }
  
  if ((vl_data = ospf_vl_lookup (area, vl_config->vl_peer)) == NULL)
    {
      vl_data = ospf_vl_data_new (area, vl_config->vl_peer);
      if (vl_data->vl_oi == NULL)
	{
	  vl_data->vl_oi = ospf_vl_new (vl_data);
	  ospf_vl_add (vl_data);
	  ospf_spf_calculate_schedule ();
	}
    }
  return vl_data;
}


int
ospf_vl_set_security (struct ospf_vl_data *vl_data,
		      struct ospf_vl_config_data *vl_config)
{
  struct crypt_key *ck;
  struct vty *vty;
  struct interface *ifp = vl_data->vl_oi->ifp;

  vty = vl_config->vty;

  if (vl_config->auth_type != OSPF_AUTH_CMD_NOTSEEN)
    {
      SET_IF_PARAM (IF_DEF_PARAMS (ifp), auth_type);
      IF_DEF_PARAMS (ifp)->auth_type = vl_config->auth_type;
    }

  if (vl_config->auth_key)
    {
      memset(IF_DEF_PARAMS (ifp)->auth_simple, 0, OSPF_AUTH_SIMPLE_SIZE+1);
      strncpy (IF_DEF_PARAMS (ifp)->auth_simple, vl_config->auth_key, 
	       OSPF_AUTH_SIMPLE_SIZE);
    }
  else if (vl_config->md5_key)
    {
      if (ospf_crypt_key_lookup (IF_DEF_PARAMS (ifp)->auth_crypt, vl_config->crypto_key_id) 
	  != NULL)
	{
	  vty_out (vty, "OSPF: Key %d already exists%s",
		   vl_config->crypto_key_id, VTY_NEWLINE);
	  return CMD_WARNING;
	}
      ck = ospf_crypt_key_new ();
      ck->key_id = vl_config->crypto_key_id;
      memset(ck->auth_key, 0, OSPF_AUTH_MD5_SIZE+1);
      strncpy (ck->auth_key, vl_config->md5_key, OSPF_AUTH_MD5_SIZE);
      
      ospf_crypt_key_add (IF_DEF_PARAMS (ifp)->auth_crypt, ck);
    }
  else if (vl_config->crypto_key_id != 0)
    {
      /* Delete a key */

      if (ospf_crypt_key_lookup (IF_DEF_PARAMS (ifp)->auth_crypt, 
				 vl_config->crypto_key_id) == NULL)
	{
	  vty_out (vty, "OSPF: Key %d does not exist%s", 
		   vl_config->crypto_key_id, VTY_NEWLINE);
	  return CMD_WARNING;
	}
      
      ospf_crypt_key_delete (IF_DEF_PARAMS (ifp)->auth_crypt, vl_config->crypto_key_id);

    }
  
  return CMD_SUCCESS;
}



int
ospf_vl_set_timers (struct ospf_vl_data *vl_data,
		    struct ospf_vl_config_data *vl_config)
{
  struct interface *ifp = ifp = vl_data->vl_oi->ifp;
  /* Virtual Link data initialised to defaults, so only set
     if a value given */
  if (vl_config->hello_interval)
    {
      SET_IF_PARAM (IF_DEF_PARAMS (ifp), v_hello);
      IF_DEF_PARAMS (ifp)->v_hello = vl_config->hello_interval;
    }

  if (vl_config->dead_interval)
    {
      SET_IF_PARAM (IF_DEF_PARAMS (ifp), v_wait);
      IF_DEF_PARAMS (ifp)->v_wait = vl_config->dead_interval;
    }

  if (vl_config->retransmit_interval)
    {
      SET_IF_PARAM (IF_DEF_PARAMS (ifp), retransmit_interval);
      IF_DEF_PARAMS (ifp)->retransmit_interval = vl_config->retransmit_interval;
    }
  
  if (vl_config->transmit_delay)
    {
      SET_IF_PARAM (IF_DEF_PARAMS (ifp), transmit_delay);
      IF_DEF_PARAMS (ifp)->transmit_delay = vl_config->transmit_delay;
    }
  
  return CMD_SUCCESS;
}



/* The business end of all of the above */
int
ospf_vl_set (struct ospf_vl_config_data *vl_config)
{
  struct ospf_vl_data *vl_data;
  int ret;

  vl_data = ospf_find_vl_data (vl_config);
  if (!vl_data)
    return CMD_WARNING;
  
  /* Process this one first as it can have a fatal result, which can
     only logically occur if the virtual link exists already
     Thus a command error does not result in a change to the
     running configuration such as unexpectedly altered timer 
     values etc.*/
  ret = ospf_vl_set_security (vl_data, vl_config);
  if (ret != CMD_SUCCESS)
    return ret;

  /* Set any time based parameters, these area already range checked */

  ret = ospf_vl_set_timers (vl_data, vl_config);
  if (ret != CMD_SUCCESS)
    return ret;

  return CMD_SUCCESS;

}

/* This stuff exists to make specifying all the alias commands A LOT simpler
 */
#define VLINK_HELPSTR_IPADDR \
       "OSPF area parameters\n" \
       "OSPF area ID in IP address format\n" \
       "Configure a virtual link\n" \
       "Router ID of the remote ABR\n"

#define VLINK_HELPSTR_DECIMAL \
       "OSPF area parameters\n" \
       "OSPF area ID in IP address format\n" \
       "Configure a virtual link\n" \
       "Router ID of the remote ABR\n"

#define VLINK_HELPSTR_AUTHTYPE_SIMPLE \
       "Enable authentication on this virtual link\n" \
       "dummy string \n" 

#define VLINK_HELPSTR_AUTHTYPE_ALL \
       VLINK_HELPSTR_AUTHTYPE_SIMPLE \
       "Use null authentication\n" \
       "Use message-digest authentication\n"


#define VLINK_HELPSTR_TIME_PARAM_NOSECS \
       "Time between HELLO packets\n" \
       "Time between retransmitting lost link state advertisements\n" \
       "Link state transmit delay\n" \
       "Interval after which a neighbor is declared dead\n"

#define VLINK_HELPSTR_TIME_PARAM \
       VLINK_HELPSTR_TIME_PARAM_NOSECS \
       "Seconds\n"

#define VLINK_HELPSTR_AUTH_SIMPLE \
       "Authentication password (key)\n" \
       "The OSPF password (key)"

#define VLINK_HELPSTR_AUTH_MD5 \
       "Message digest authentication password (key)\n" \
       "dummy string \n" \
       "Key ID\n" \
       "Use MD5 algorithm\n" \
       "The OSPF password (key)"

DEFUN (area_vlink,
       area_vlink_cmd,
       "area A.B.C.D virtual-link A.B.C.D",
       VLINK_HELPSTR_IPADDR)
{
  struct ospf_vl_config_data vl_config;
  char auth_key[OSPF_AUTH_SIMPLE_SIZE+1];
  char md5_key[OSPF_AUTH_MD5_SIZE+1]; 
  int i;
  int ret;
  
  ospf_vl_config_data_init(&vl_config, vty);

  /* Read off first 2 parameters and check them */
  vl_config.format = ospf_str2area_id (argv[0], &vl_config.area_id);
  if (!vl_config.format)
    {
      vty_out (vty, "OSPF area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  ret = inet_aton (argv[1], &vl_config.vl_peer);
  if (! ret)
    {
      vty_out (vty, "Please specify valid Router ID as a.b.c.d%s",
               VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (argc <=2)
    {
      /* Thats all folks! - BUGS B. strikes again!!!*/

      return  ospf_vl_set (&vl_config);
    }


  /* Deal with other parameters */
  for (i=2; i < argc; i++)
    {

      /* vty_out (vty, "argv[%d] - %s%s", i, argv[i], VTY_NEWLINE); */

      switch (argv[i][0])
	{

	case 'a':
	  if (i > 2 || strncmp (argv[i], "authentication-", 15) == 0)
	    {
	      /* authentication-key - this option can occur anywhere on 
		                      command line.  At start of command line
				      must check for authentication option. */
		 memset (auth_key, 0, OSPF_AUTH_SIMPLE_SIZE + 1);
		 strncpy (auth_key, argv[i+1], OSPF_AUTH_SIMPLE_SIZE);
		vl_config.auth_key = auth_key;
		i++;
	    }
	  else if (strncmp (argv[i], "authentication", 14) == 0)
	    {
	      /* authentication  - this option can only occur at start
		                   of command line */
	      vl_config.auth_type = OSPF_AUTH_SIMPLE;
	      if ((i+1) < argc)
		{
		  if (strncmp (argv[i+1], "n", 1) == 0)
		    {
		      /* "authentication null" */
		      vl_config.auth_type = OSPF_AUTH_NULL;
		      i++;
		    }
		  else if (strncmp (argv[i+1], "m", 1) == 0
			   && strcmp (argv[i+1], "message-digest-") != 0)
		    {
		      /* "authentication message-digest" */ 
		      vl_config.auth_type = OSPF_AUTH_CRYPTOGRAPHIC;
		      i++;
		    }
		}
	    }
	  break;

	case 'm':
	  /* message-digest-key */
	  i++;
	  vl_config.crypto_key_id = strtol (argv[i], NULL, 10);
	  if (vl_config.crypto_key_id < 0)
	    return CMD_WARNING;
	  i++;
	  memset(md5_key, 0, OSPF_AUTH_MD5_SIZE+1);
	  strncpy (md5_key, argv[i], OSPF_AUTH_MD5_SIZE);
	  vl_config.md5_key = md5_key; 
	  break;

	case 'h':
	  /* Hello interval */
	  i++;
	  vl_config.hello_interval = strtol (argv[i], NULL, 10);
	  if (vl_config.hello_interval < 0) 
	    return CMD_WARNING;
	  break;

	case 'r':
	  /* Retransmit Interval */
	  i++;
	  vl_config.retransmit_interval = strtol (argv[i], NULL, 10);
	  if (vl_config.retransmit_interval < 0)
	    return CMD_WARNING;
	  break;

	case 't':
	  /* Transmit Delay */
	  i++;
	  vl_config.transmit_delay = strtol (argv[i], NULL, 10);
	  if (vl_config.transmit_delay < 0)
	    return CMD_WARNING;
	  break;

	case 'd':
	  /* Dead Interval */
	  i++;
	  vl_config.dead_interval = strtol (argv[i], NULL, 10);
	  if (vl_config.dead_interval < 0)
	    return CMD_WARNING;
	  break;
	}
    }


  /* Action configuration */

  return ospf_vl_set (&vl_config);

}

ALIAS (area_vlink,
       area_vlink_decimal_cmd,
       "area <0-4294967295> virtual-link A.B.C.D",
       VLINK_HELPSTR_DECIMAL)

DEFUN (no_area_vlink,
       no_area_vlink_cmd,
       "no area A.B.C.D virtual-link A.B.C.D",
       NO_STR
       VLINK_HELPSTR_IPADDR)
{
  struct ospf_area *area;
  struct ospf_vl_config_data vl_config;
  struct ospf_vl_data *vl_data = NULL;
  char auth_key[OSPF_AUTH_SIMPLE_SIZE+1];
  int i;
  int ret;

  ospf_vl_config_data_init(&vl_config, vty);

  ret = ospf_str2area_id (argv[0], &vl_config.area_id);
  if (!ret)
    {
      vty_out (vty, "OSPF area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  area = ospf_area_lookup_by_area_id (vl_config.area_id);
  if (!area)
    {
      vty_out (vty, "Area does not exist%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  ret = inet_aton (argv[1], &vl_config.vl_peer);
  if (! ret)
    {
      vty_out (vty, "Please specify valid Router ID as a.b.c.d%s",
               VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (argc <=2)
    {
      /* Basic VLink no command */
      /* Thats all folks! - BUGS B. strikes again!!!*/
      if ((vl_data = ospf_vl_lookup (area, vl_config.vl_peer)))
	ospf_vl_delete (vl_data);

      ospf_area_check_free (vl_config.area_id);
      
      return CMD_SUCCESS;
    }

  /* If we are down here, we are reseting parameters */

  /* Deal with other parameters */
  for (i=2; i < argc; i++)
    {

      /* vty_out (vty, "argv[%d] - %s%s", i, argv[i], VTY_NEWLINE); */

      switch (argv[i][0])
	{

	case 'a':
	  if (i > 2 || strncmp (argv[i], "authentication-", 15) == 0)
	    {
	      /* authentication-key - this option can occur anywhere on 
		                      command line.  At start of command line
				      must check for authentication option. */
	      memset (auth_key, 0, OSPF_AUTH_SIMPLE_SIZE + 1);
	      vl_config.auth_key = auth_key;
	    }
	  else if (strncmp (argv[i], "authentication", 14) == 0)
	    {
	      /* authentication  - this option can only occur at start
		                   of command line */
	      vl_config.auth_type = OSPF_AUTH_NOTSET;
	    }
	  break;

	case 'm':
	  /* message-digest-key */
	  /* Delete one key */
	  i++;
	  vl_config.crypto_key_id = strtol (argv[i], NULL, 10);
	  if (vl_config.crypto_key_id < 0)
	    return CMD_WARNING;
	  vl_config.md5_key = NULL; 
	  break;

	case 'h':
	  /* Hello interval */
	  vl_config.hello_interval = OSPF_HELLO_INTERVAL_DEFAULT;
	  break;

	case 'r':
	  /* Retransmit Interval */
	  vl_config.retransmit_interval = OSPF_RETRANSMIT_INTERVAL_DEFAULT;
	  break;

	case 't':
	  /* Transmit Delay */
	  vl_config.transmit_delay = OSPF_TRANSMIT_DELAY_DEFAULT;
	  break;

	case 'd':
	  /* Dead Interval */
	  i++;
	  vl_config.dead_interval = OSPF_ROUTER_DEAD_INTERVAL_DEFAULT;
	  break;
	}
    }


  /* Action configuration */

  return ospf_vl_set (&vl_config);
}

ALIAS (no_area_vlink,
       no_area_vlink_decimal_cmd,
       "no area <0-4294967295> virtual-link A.B.C.D",
       NO_STR
       VLINK_HELPSTR_DECIMAL)


ALIAS (area_vlink,
       area_vlink_param1_cmd,
       "area A.B.C.D virtual-link A.B.C.D "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535>",
       VLINK_HELPSTR_IPADDR
       VLINK_HELPSTR_TIME_PARAM)

ALIAS (area_vlink,
       area_vlink_param1_decimal_cmd,
       "area <0-4294967295> virtual-link A.B.C.D "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535>",
       VLINK_HELPSTR_DECIMAL
       VLINK_HELPSTR_TIME_PARAM)

ALIAS (no_area_vlink,
       no_area_vlink_param1_cmd,
       "no area A.B.C.D virtual-link A.B.C.D "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval)",
       NO_STR
       VLINK_HELPSTR_IPADDR
       VLINK_HELPSTR_TIME_PARAM)

ALIAS (no_area_vlink,
       no_area_vlink_param1_decimal_cmd,
       "no area <0-4294967295> virtual-link A.B.C.D "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval)",
       NO_STR
       VLINK_HELPSTR_DECIMAL
       VLINK_HELPSTR_TIME_PARAM)

ALIAS (area_vlink,
       area_vlink_param2_cmd,
       "area A.B.C.D virtual-link A.B.C.D "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535> "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535>",
       VLINK_HELPSTR_IPADDR
       VLINK_HELPSTR_TIME_PARAM
       VLINK_HELPSTR_TIME_PARAM)

ALIAS (area_vlink,
       area_vlink_param2_decimal_cmd,
       "area <0-4294967295> virtual-link A.B.C.D "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535> "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535>",
       VLINK_HELPSTR_DECIMAL
       VLINK_HELPSTR_TIME_PARAM
       VLINK_HELPSTR_TIME_PARAM)

ALIAS (no_area_vlink,
       no_area_vlink_param2_cmd,
       "no area A.B.C.D virtual-link A.B.C.D "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval)",
       NO_STR
       VLINK_HELPSTR_IPADDR
       VLINK_HELPSTR_TIME_PARAM
       VLINK_HELPSTR_TIME_PARAM)

ALIAS (no_area_vlink,
       no_area_vlink_param2_decimal_cmd,
       "no area <0-4294967295> virtual-link A.B.C.D "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval)",
       NO_STR
       VLINK_HELPSTR_DECIMAL
       VLINK_HELPSTR_TIME_PARAM
       VLINK_HELPSTR_TIME_PARAM)

ALIAS (area_vlink,
       area_vlink_param3_cmd,
       "area A.B.C.D virtual-link A.B.C.D "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535> "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535> "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535>",
       VLINK_HELPSTR_IPADDR
       VLINK_HELPSTR_TIME_PARAM
       VLINK_HELPSTR_TIME_PARAM
       VLINK_HELPSTR_TIME_PARAM)

ALIAS (area_vlink,
       area_vlink_param3_decimal_cmd,
       "area <0-4294967295> virtual-link A.B.C.D "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535> "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535> "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535>",
       VLINK_HELPSTR_DECIMAL
       VLINK_HELPSTR_TIME_PARAM
       VLINK_HELPSTR_TIME_PARAM
       VLINK_HELPSTR_TIME_PARAM)

ALIAS (no_area_vlink,
       no_area_vlink_param3_cmd,
       "no area A.B.C.D virtual-link A.B.C.D "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval)",
       NO_STR
       VLINK_HELPSTR_IPADDR
       VLINK_HELPSTR_TIME_PARAM
       VLINK_HELPSTR_TIME_PARAM
       VLINK_HELPSTR_TIME_PARAM)

ALIAS (no_area_vlink,
       no_area_vlink_param3_decimal_cmd,
       "no area <0-4294967295> virtual-link A.B.C.D "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval)",
       NO_STR
       VLINK_HELPSTR_DECIMAL
       VLINK_HELPSTR_TIME_PARAM
       VLINK_HELPSTR_TIME_PARAM
       VLINK_HELPSTR_TIME_PARAM)

ALIAS (area_vlink,
       area_vlink_param4_cmd,
       "area A.B.C.D virtual-link A.B.C.D "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535> "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535> "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535> "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535>",
       VLINK_HELPSTR_IPADDR
       VLINK_HELPSTR_TIME_PARAM
       VLINK_HELPSTR_TIME_PARAM
       VLINK_HELPSTR_TIME_PARAM
       VLINK_HELPSTR_TIME_PARAM)

ALIAS (area_vlink,
       area_vlink_param4_decimal_cmd,
       "area <0-4294967295> virtual-link A.B.C.D "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535> "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535> "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535> "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535>",
       VLINK_HELPSTR_DECIMAL
       VLINK_HELPSTR_TIME_PARAM
       VLINK_HELPSTR_TIME_PARAM
       VLINK_HELPSTR_TIME_PARAM
       VLINK_HELPSTR_TIME_PARAM)

ALIAS (no_area_vlink,
       no_area_vlink_param4_cmd,
       "no area A.B.C.D virtual-link A.B.C.D "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval)",
       NO_STR
       VLINK_HELPSTR_IPADDR
       VLINK_HELPSTR_TIME_PARAM
       VLINK_HELPSTR_TIME_PARAM
       VLINK_HELPSTR_TIME_PARAM
       VLINK_HELPSTR_TIME_PARAM)

ALIAS (no_area_vlink,
       no_area_vlink_param4_decimal_cmd,
       "no area <0-4294967295> virtual-link A.B.C.D "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval)",
       NO_STR
       VLINK_HELPSTR_DECIMAL
       VLINK_HELPSTR_TIME_PARAM
       VLINK_HELPSTR_TIME_PARAM
       VLINK_HELPSTR_TIME_PARAM
       VLINK_HELPSTR_TIME_PARAM)

ALIAS (area_vlink,
       area_vlink_authtype_args_cmd,
       "area A.B.C.D virtual-link A.B.C.D "
       "(authentication|) (message-digest|null)",
       VLINK_HELPSTR_IPADDR
       VLINK_HELPSTR_AUTHTYPE_ALL)

ALIAS (area_vlink,
       area_vlink_authtype_args_decimal_cmd,
       "area <0-4294967295> virtual-link A.B.C.D "
       "(authentication|) (message-digest|null)",
       VLINK_HELPSTR_DECIMAL
       VLINK_HELPSTR_AUTHTYPE_ALL)

ALIAS (area_vlink,
       area_vlink_authtype_cmd,
       "area A.B.C.D virtual-link A.B.C.D "
       "(authentication|)",
       VLINK_HELPSTR_IPADDR
       VLINK_HELPSTR_AUTHTYPE_SIMPLE)

ALIAS (area_vlink,
       area_vlink_authtype_decimal_cmd,
       "area <0-4294967295> virtual-link A.B.C.D "
       "(authentication|)",
       VLINK_HELPSTR_DECIMAL
       VLINK_HELPSTR_AUTHTYPE_SIMPLE)

ALIAS (no_area_vlink,
       no_area_vlink_authtype_cmd,
       "no area A.B.C.D virtual-link A.B.C.D "
       "(authentication|)",
       NO_STR
       VLINK_HELPSTR_IPADDR
       VLINK_HELPSTR_AUTHTYPE_SIMPLE)

ALIAS (no_area_vlink,
       no_area_vlink_authtype_decimal_cmd,
       "no area <0-4294967295> virtual-link A.B.C.D "
       "(authentication|)",
       NO_STR
       VLINK_HELPSTR_DECIMAL
       VLINK_HELPSTR_AUTHTYPE_SIMPLE)

ALIAS (area_vlink,
       area_vlink_md5_cmd,
       "area A.B.C.D virtual-link A.B.C.D "
       "(message-digest-key|) <1-255> md5 KEY",
       VLINK_HELPSTR_IPADDR
       VLINK_HELPSTR_AUTH_MD5)

ALIAS (area_vlink,
       area_vlink_md5_decimal_cmd,
       "area <0-4294967295> virtual-link A.B.C.D "
       "(message-digest-key|) <1-255> md5 KEY",
       VLINK_HELPSTR_DECIMAL
       VLINK_HELPSTR_AUTH_MD5)

ALIAS (no_area_vlink,
       no_area_vlink_md5_cmd,
       "no area A.B.C.D virtual-link A.B.C.D "
       "(message-digest-key|) <1-255>",
       NO_STR
       VLINK_HELPSTR_IPADDR
       VLINK_HELPSTR_AUTH_MD5)

ALIAS (no_area_vlink,
       no_area_vlink_md5_decimal_cmd,
       "no area <0-4294967295> virtual-link A.B.C.D "
       "(message-digest-key|) <1-255>",
       NO_STR
       VLINK_HELPSTR_DECIMAL
       VLINK_HELPSTR_AUTH_MD5)

ALIAS (area_vlink,
       area_vlink_authkey_cmd,
       "area A.B.C.D virtual-link A.B.C.D "
       "(authentication-key|) AUTH_KEY",
       VLINK_HELPSTR_IPADDR
       VLINK_HELPSTR_AUTH_SIMPLE)

ALIAS (area_vlink,
       area_vlink_authkey_decimal_cmd,
       "area <0-4294967295> virtual-link A.B.C.D "
       "(authentication-key|) AUTH_KEY",
       VLINK_HELPSTR_DECIMAL
       VLINK_HELPSTR_AUTH_SIMPLE)

ALIAS (no_area_vlink,
       no_area_vlink_authkey_cmd,
       "no area A.B.C.D virtual-link A.B.C.D "
       "(authentication-key|)",
       NO_STR
       VLINK_HELPSTR_IPADDR
       VLINK_HELPSTR_AUTH_SIMPLE)

ALIAS (no_area_vlink,
       no_area_vlink_authkey_decimal_cmd,
       "no area <0-4294967295> virtual-link A.B.C.D "
       "(authentication-key|)",
       NO_STR
       VLINK_HELPSTR_DECIMAL
       VLINK_HELPSTR_AUTH_SIMPLE)


ALIAS (area_vlink,
       area_vlink_authtype_args_authkey_cmd,
       "area A.B.C.D virtual-link A.B.C.D "
       "(authentication|) (message-digest|null) "
       "(authentication-key|) AUTH_KEY",
       VLINK_HELPSTR_IPADDR
       VLINK_HELPSTR_AUTHTYPE_ALL
       VLINK_HELPSTR_AUTH_SIMPLE)

ALIAS (area_vlink,
       area_vlink_authtype_args_authkey_decimal_cmd,
       "area <0-4294967295> virtual-link A.B.C.D "
       "(authentication|) (message-digest|null) "
       "(authentication-key|) AUTH_KEY",
       VLINK_HELPSTR_DECIMAL
       VLINK_HELPSTR_AUTHTYPE_ALL
       VLINK_HELPSTR_AUTH_SIMPLE)

ALIAS (area_vlink,
       area_vlink_authtype_authkey_cmd,
       "area A.B.C.D virtual-link A.B.C.D "
       "(authentication|) "
       "(authentication-key|) AUTH_KEY",
       VLINK_HELPSTR_IPADDR
       VLINK_HELPSTR_AUTHTYPE_SIMPLE
       VLINK_HELPSTR_AUTH_SIMPLE)

ALIAS (area_vlink,
       area_vlink_authtype_authkey_decimal_cmd,
       "area <0-4294967295> virtual-link A.B.C.D "
       "(authentication|) "
       "(authentication-key|) AUTH_KEY",
       VLINK_HELPSTR_DECIMAL
       VLINK_HELPSTR_AUTHTYPE_SIMPLE
       VLINK_HELPSTR_AUTH_SIMPLE)

ALIAS (no_area_vlink,
       no_area_vlink_authtype_authkey_cmd,
       "no area A.B.C.D virtual-link A.B.C.D "
       "(authentication|) "
       "(authentication-key|)",
       NO_STR
       VLINK_HELPSTR_IPADDR
       VLINK_HELPSTR_AUTHTYPE_SIMPLE
       VLINK_HELPSTR_AUTH_SIMPLE)

ALIAS (no_area_vlink,
       no_area_vlink_authtype_authkey_decimal_cmd,
       "no area <0-4294967295> virtual-link A.B.C.D "
       "(authentication|) "
       "(authentication-key|)",
       NO_STR
       VLINK_HELPSTR_DECIMAL
       VLINK_HELPSTR_AUTHTYPE_SIMPLE
       VLINK_HELPSTR_AUTH_SIMPLE)

ALIAS (area_vlink,
       area_vlink_authtype_args_md5_cmd,
       "area A.B.C.D virtual-link A.B.C.D "
       "(authentication|) (message-digest|null) "
       "(message-digest-key|) <1-255> md5 KEY",
       VLINK_HELPSTR_IPADDR
       VLINK_HELPSTR_AUTHTYPE_ALL
       VLINK_HELPSTR_AUTH_MD5)

ALIAS (area_vlink,
       area_vlink_authtype_args_md5_decimal_cmd,
       "area <0-4294967295> virtual-link A.B.C.D "
       "(authentication|) (message-digest|null) "
       "(message-digest-key|) <1-255> md5 KEY",
       VLINK_HELPSTR_DECIMAL
       VLINK_HELPSTR_AUTHTYPE_ALL
       VLINK_HELPSTR_AUTH_MD5)

ALIAS (area_vlink,
       area_vlink_authtype_md5_cmd,
       "area A.B.C.D virtual-link A.B.C.D "
       "(authentication|) "
       "(message-digest-key|) <1-255> md5 KEY",
       VLINK_HELPSTR_IPADDR
       VLINK_HELPSTR_AUTHTYPE_SIMPLE
       VLINK_HELPSTR_AUTH_MD5)

ALIAS (area_vlink,
       area_vlink_authtype_md5_decimal_cmd,
       "area <0-4294967295> virtual-link A.B.C.D "
       "(authentication|) "
       "(message-digest-key|) <1-255> md5 KEY",
       VLINK_HELPSTR_DECIMAL
       VLINK_HELPSTR_AUTHTYPE_SIMPLE
       VLINK_HELPSTR_AUTH_MD5)

ALIAS (no_area_vlink,
       no_area_vlink_authtype_md5_cmd,
       "no area A.B.C.D virtual-link A.B.C.D "
       "(authentication|) "
       "(message-digest-key|)",
       NO_STR
       VLINK_HELPSTR_IPADDR
       VLINK_HELPSTR_AUTHTYPE_SIMPLE
       VLINK_HELPSTR_AUTH_MD5)

ALIAS (no_area_vlink,
       no_area_vlink_authtype_md5_decimal_cmd,
       "no area <0-4294967295> virtual-link A.B.C.D "
       "(authentication|) "
       "(message-digest-key|)",
       NO_STR
       VLINK_HELPSTR_DECIMAL
       VLINK_HELPSTR_AUTHTYPE_SIMPLE
       VLINK_HELPSTR_AUTH_MD5)


#endif /* OLD_VLINK_CMD */

DEFUN (area_shortcut,
       area_shortcut_cmd,
       "area A.B.C.D shortcut (default|enable|disable)",
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "Configure the area's shortcutting mode\n"
       "Set default shortcutting behavior\n"
       "Enable shortcutting through the area\n"
       "Disable shortcutting through the area\n")
{
  struct ospf_area *area;
  struct in_addr area_id;
  int mode;
  int ret;

  ret = ospf_str2area_id (argv[0], &area_id);
  if (!ret)
    {
      vty_out (vty, "OSPF area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (area_id.s_addr == OSPF_AREA_BACKBONE)
    {
      vty_out (vty, "You cannot configure backbone area as shortcut%s",
               VTY_NEWLINE);
      return CMD_WARNING;
    }

  area = ospf_area_get (area_id, ret);

  if (strncmp (argv[1], "de", 2) == 0)
    mode = OSPF_SHORTCUT_DEFAULT;
  else if (strncmp (argv[1], "di", 2) == 0)
    mode = OSPF_SHORTCUT_DISABLE;
  else if (strncmp (argv[1], "e", 1) == 0)
    mode = OSPF_SHORTCUT_ENABLE;
  else
    return CMD_WARNING;

  if (area->shortcut_configured != mode)
    {
      area->shortcut_configured = mode;
      if (ospf_top->abr_type != OSPF_ABR_SHORTCUT)
        vty_out (vty, "Shortcut area setting will take effect "
                 "only when the router is configured as "
                 "Shortcut ABR%s", VTY_NEWLINE);
      ospf_router_lsa_timer_add (area);
      ospf_schedule_abr_task ();
    }

  ospf_area_check_free (area_id);
  return CMD_SUCCESS;
}

ALIAS (area_shortcut,
       area_shortcut_decimal_cmd,
       "area <0-4294967295> shortcut (default|enable|disable)",
       "OSPF area parameters\n"
       "OSPF area ID as a decimal value\n"
       "Configure the area's shortcutting mode\n"
       "Set default shortcutting behavior\n"
       "Enable shortcutting through the area\n"
       "Disable shortcutting through the area\n")

DEFUN (no_area_shortcut,
       no_area_shortcut_cmd,
       "no area A.B.C.D shortcut (enable|disable)",
       NO_STR
       "OSPF area parameters\n"
       "OSPF area ID\n"
       "Deconfigure the area's shortcutting mode\n"
       "Deconfigure enabled shortcutting through the area\n"
       "Deconfigure disabled shortcutting through the area\n")
{
  struct ospf_area *area;
  struct in_addr area_id;
  int mode;
  int ret;

  ret = ospf_str2area_id (argv[0], &area_id);
  if (!ret)
    {
      vty_out (vty, "OSPF area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (area_id.s_addr == OSPF_AREA_BACKBONE)
     return CMD_SUCCESS;

  area = ospf_area_lookup_by_area_id (area_id);
  if (!area)
    {
      vty_out (vty, "Area does not exist%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (strncmp (argv[1], "di", 2) == 0)
    mode = OSPF_SHORTCUT_DISABLE;
  else if (strncmp (argv[1], "e", 1) == 0)
    mode = OSPF_SHORTCUT_ENABLE;
  else
    return CMD_WARNING;

  if (area->shortcut_configured == mode)
    {
      ospf_area_check_free (area_id);
      area->shortcut_configured = OSPF_SHORTCUT_DEFAULT;
      ospf_router_lsa_timer_add (area);
      ospf_schedule_abr_task ();
    }

  return CMD_SUCCESS;
}

ALIAS (no_area_shortcut,
       no_area_shortcut_decimal_cmd,
       "no area <0-4294967295> shortcut (enable|disable)",
       NO_STR
       "OSPF area parameters\n"
       "OSPF area ID as a decimal value\n"
       "Deconfigure the area's shortcutting mode\n"
       "Deconfigure enabled shortcutting through the area\n"
       "Deconfigure disabled shortcutting through the area\n")

struct message ospf_area_type_msg[] =
{
  { OSPF_AREA_DEFAULT,	"Default" },
  { OSPF_AREA_STUB,     "Stub" },
  { OSPF_AREA_NSSA,     "NSSA" },
};
int ospf_area_type_msg_max = OSPF_AREA_TYPE_MAX;

void
ospf_area_type_set (struct ospf_area *area, int type)
{
  listnode node;
  struct ospf_interface *oi;

  if (area->external_routing == type)
    {
      if (IS_DEBUG_OSPF_EVENT)
	zlog_info ("Area[%s]: Types are the same, ignored.",
		   inet_ntoa (area->area_id));
      return;
    }

  area->external_routing = type;

  if (IS_DEBUG_OSPF_EVENT)
    zlog_info ("Area[%s]: Configured as %s", inet_ntoa (area->area_id),
	       LOOKUP (ospf_area_type_msg, type));

  switch (area->external_routing)
    {
    case OSPF_AREA_DEFAULT:
      for (node = listhead (area->oiflist); node; nextnode (node))
	if ((oi = getdata (node)) != NULL)
	  if (oi->nbr_self != NULL)
	    SET_FLAG (oi->nbr_self->options, OSPF_OPTION_E);
      break;
    case OSPF_AREA_STUB:
      for (node = listhead (area->oiflist); node; nextnode (node))
	if ((oi = getdata (node)) != NULL)
	  if (oi->nbr_self != NULL)
	    {
	      if (IS_DEBUG_OSPF_EVENT)
		zlog_info ("setting options on %s accordingly", IF_NAME (oi));
	      UNSET_FLAG (oi->nbr_self->options, OSPF_OPTION_E);
	      if (IS_DEBUG_OSPF_EVENT)
		zlog_info ("options set on %s: %x",
			   IF_NAME (oi), OPTIONS (oi));
	    }
      break;
    case OSPF_AREA_NSSA:
#ifdef HAVE_NSSA
      if (IS_DEBUG_OSPF_EVENT)
	zlog_info ("Scanning all NSSA interfaces for area %ld, start %ld",
		   area, listhead (area->oiflist) );

      for (node = listhead (area->oiflist); node; nextnode (node))
	if ((oi = getdata (node)) != NULL)
	  if (oi->nbr_self != NULL)
	    {
	      zlog_info ("setting nssa options on %s accordingly", IF_NAME (oi));
	      UNSET_FLAG (oi->nbr_self->options, OSPF_OPTION_E);
	      SET_FLAG (oi->nbr_self->options, OSPF_OPTION_NP);
	      zlog_info ("options set on %s: %x", IF_NAME (oi), OPTIONS (oi));
	    }
#endif /* HAVE_NSSA */
      break;
    default:
      break;
    }

  ospf_router_lsa_timer_add (area);
  ospf_schedule_abr_task ();
}

int
ospf_area_stub_cmd (struct vty *vty, int argc, char **argv, int no_summary,
		    int xlate)
{
  struct ospf_area *area;
  struct in_addr area_id;
  int ret;

#ifdef HAVE_NSSA
  int nssa = 0;

  zlog_info ("OSPF Area ID set for STUB/NSSA Code = %d", no_summary);

/*  Special translation of no_summary into nssa/no_summary  */
/*  0, 1 = no nssa	    2, 3 = nssa */
/*  0, 2 = no_summary off   1, 3 = no_summary on  */

  if (no_summary >1)
	{
	  nssa = 1;       /* set nssa */
	  no_summary = no_summary - 2; /* translate 2 into 0, 3 into 1 */
	}
#endif /* HAVE_NSSA */



  ret = ospf_str2area_id (argv[0], &area_id);
  if (!ret)
    {
      vty_out (vty, "OSPF Area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (area_id.s_addr == OSPF_AREA_BACKBONE)
    {
      vty_out (vty, "You cannot configure backbone area as stub%s",
               VTY_NEWLINE);
      return CMD_WARNING;
    }

  area = ospf_area_get (area_id, ret);

  if (ospf_vls_in_area (area))
    {
      vty_out (vty, "First deconfigure all VLs through this area%s",
               VTY_NEWLINE);
      return CMD_WARNING;
    }

#ifdef HAVE_NSSA
  /* Set transllator Role to zero or other. */
  area->NSSATranslatorRole = xlate;
  area->NSSATranslator = xlate;

  if (nssa)
    {
      if (area->external_routing != OSPF_AREA_NSSA)
	ospf_top->anyNSSA++;
      ospf_area_type_set (area, OSPF_AREA_NSSA);
      zlog_info ("OSPF_AREA_NSSA");
    }
  else
    {
      ospf_area_type_set (area, OSPF_AREA_STUB);
      zlog_info ("OSPF_AREA_STUB");
    }
#else /* ! HAVE_NSSA */
  ospf_area_type_set (area, OSPF_AREA_STUB);
  zlog_info ("OSPF_AREA_STUB");
#endif /* HAVE_NSSA */

  area->no_summary = no_summary;

  return CMD_SUCCESS;
}

/*********************************************************************************/

DEFUN (area_stub,
       area_stub_cmd,
       "area A.B.C.D stub",
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "Configure OSPF area as stub\n")
{
  return ospf_area_stub_cmd (vty, argc, argv, 0, 0);
}

ALIAS (area_stub,
       area_stub_decimal_cmd,
       "area <0-4294967295> stub",
       "OSPF area parameters\n"
       "OSPF area ID as a decimal value\n"
       "Configure OSPF area as stub\n")

#ifdef HAVE_NSSA
DEFUN (area_nssa,
       area_nssa_cmd,
       "area A.B.C.D nssa",
       "OSPF nssa parameters\n"
       "OSPF area ID in IP address format\n"
       "Configure OSPF area as nssa\n")
{
  return ospf_area_stub_cmd (vty, argc, argv, 2, 0);
}

ALIAS (area_nssa,
       area_nssa_decimal_cmd,
       "area <0-4294967295> nssa",
       "OSPF nssa parameters\n"
       "OSPF area ID as a decimal value\n"
       "Configure OSPF area as nssa\n")
/*****************************************************************************
**/

DEFUN (area_nssa_never,
       area_nssa_t_never_cmd,
       "area A.B.C.D nssa translate-never",
       "OSPF nssa parameters\n"
       "OSPF area ID in IP address format\n"
       "Configure NSSA-ABR to never translate\n")
{
  return ospf_area_stub_cmd (vty, argc, argv, 2, OSPF_NSSA_ROLE_NEVER);
}

ALIAS (area_nssa_never,
       area_nssa_t_never_decimal_cmd,
       "area <0-4294967295> nssa translate-never",
       "OSPF nssa parameters\n"
       "OSPF area ID as a decimal value\n"
       "Configure NSSA-ABR to never translate\n")
/*****************************************************************************
**/

DEFUN (area_nssa_candidate,
       area_nssa_t_candidate_cmd,
       "area A.B.C.D nssa translate-candidate",
       "OSPF nssa parameters\n"
       "OSPF area ID in IP address format\n"
       "Configure NSSA-ABR for translate election\n")
{
  return ospf_area_stub_cmd (vty, argc, argv, 2, OSPF_NSSA_ROLE_CANDIDATE);
}

ALIAS (area_nssa_candidate,
       area_nssa_t_candidate_decimal_cmd,
       "area <0-4294967295> nssa translate-candidate",
       "OSPF nssa parameters\n"
       "OSPF area ID as a decimal value\n"
       "Configure NSSA-ABR for translate election\n")
/*****************************************************************************
**/

DEFUN (area_nssa_always,
       area_nssa_t_always_cmd,
       "area A.B.C.D nssa translate-always",
       "OSPF nssa parameters\n"
       "OSPF area ID in IP address format\n"
       "Configure NSSA-ABR to always translate\n")
{
  return ospf_area_stub_cmd (vty, argc, argv, 2, OSPF_NSSA_ROLE_ALWAYS);
}

ALIAS (area_nssa_always,
       area_nssa_t_always_decimal_cmd,
       "area <0-4294967295> nssa translate-always",
       "OSPF nssa parameters\n"
       "OSPF area ID as a decimal value\n"
       "Configure NSSA-ABR to always translate\n")

/*************************    NSSA no-summary ********************************
***/

DEFUN (area_nssa_nosum,
       area_nssa_nosum_cmd,
       "area A.B.C.D nssa no-summary",
       "OSPF nssa parameters\n"
       "OSPF area ID in IP address format\n"
       "Configure OSPF area as nssa\n"
       "Do not inject inter-area routes into nssa\n")
{
  return ospf_area_stub_cmd (vty, argc, argv, 3, 0);
}

ALIAS (area_nssa_nosum,
       area_nssa_nosum_decimal_cmd,
       "area <0-4294967295> nssa no-summary",
       "OSPF nssa parameters\n"
       "OSPF area ID as a decimal value\n"
       "Configure OSPF area as nssa\n"
       "Do not inject inter-area routes into nssa\n")

/*****************************************************************************
**/

DEFUN (area_nssa_nosum_never,
       area_nssa_nosum_t_never_cmd,
       "area A.B.C.D nssa no-summary translate-never",
       "OSPF nssa parameters\n"
       "OSPF area ID in IP address format\n"
       "Configure OSPF area as nssa\n"
       "No inter-area routes into nssa, nor translation\n")
{
  return ospf_area_stub_cmd (vty, argc, argv, 3, OSPF_NSSA_ROLE_NEVER);
}

ALIAS (area_nssa_nosum_never,
       area_nssa_nosum_t_never_decimal_cmd,
       "area <0-4294967295> nssa no-summary translate-never",
       "OSPF nssa parameters\n"
       "OSPF area ID as a decimal value\n"
       "Configure OSPF area as nssa\n"
       "No inter-area routes into nssa, nor translation\n")

/*****************************************************************************
**/

DEFUN (area_nssa_nosum_candidate,
       area_nssa_nosum_t_candidate_cmd,
       "area A.B.C.D nssa no-summary translate-candidate",
       "OSPF nssa parameters\n"
       "OSPF area ID in IP address format\n"
       "Configure OSPF area as nssa\n"
       "No inter-area routes into nssa, translation election\n")
{
  return ospf_area_stub_cmd (vty, argc, argv, 3, OSPF_NSSA_ROLE_CANDIDATE);
}

ALIAS (area_nssa_nosum_candidate,
       area_nssa_nosum_t_candidate_decimal_cmd,
       "area <0-4294967295> nssa no-summary translate-candidate",
       "OSPF nssa parameters\n"
       "OSPF area ID as a decimal value\n"
       "Configure OSPF area as nssa\n"
       "No inter-area routes into nssa, translation election\n")

/*****************************************************************************
**/

DEFUN (area_nssa_nosum_always,
       area_nssa_nosum_t_always_cmd,
       "area A.B.C.D nssa no-summary translate-always",
       "OSPF nssa parameters\n"
       "OSPF area ID in IP address format\n"
       "Configure OSPF area as nssa\n"
       "No inter-area routes into nssa, always translate\n")
{
  return ospf_area_stub_cmd (vty, argc, argv, 3, OSPF_NSSA_ROLE_ALWAYS);
}

ALIAS (area_nssa_nosum_always,
       area_nssa_nosum_t_always_decimal_cmd,
       "area <0-4294967295> nssa no-summary translate-always",
       "OSPF nssa parameters\n"
       "OSPF area ID as a decimal value\n"
       "Configure OSPF area as nssa\n"
       "No inter-area routes into nssa, always translate\n")

#endif /* HAVE_NSSA */
/*********************************************************************************/

DEFUN (area_stub_nosum,
       area_stub_nosum_cmd,
       "area A.B.C.D stub no-summary",
       "OSPF stub parameters\n"
       "OSPF area ID in IP address format\n"
       "Configure OSPF area as stub\n"
       "Do not inject inter-area routes into stub\n")
{
  return ospf_area_stub_cmd (vty, argc, argv, 1, 0);
}

ALIAS (area_stub_nosum,
       area_stub_nosum_decimal_cmd,
       "area <0-4294967295> stub no-summary",
       "OSPF stub parameters\n"
       "OSPF area ID as a decimal value\n"
       "Configure OSPF area as stub\n"
       "Do not inject inter-area routes into stub\n")

/*********************************************************************************/

int
ospf_no_area_stub_cmd (struct vty *vty, int argc, char **argv, int no_summary)
{
  struct ospf_area *area;
  struct in_addr area_id;
  int ret;


#ifdef HAVE_NSSA
  int nssa = 0;

 vty_out (vty, "\nOSPF No-Area ID set for STUB/NSSA Code = %d\n\n", no_summary);


/*  Special translation of no_summary into nssa/no_summary  */
/*  0, 1 = no nssa	    2, 3 = nssa */
/*  0, 2 = no_summary off   1, 3 = no_summary on  */

  if (no_summary >1)
	{
	  nssa = 1;       /* set nssa */
	  no_summary = no_summary - 2; /* translate 2 into 0, 3 into 1 */
	}
#endif /* HAVE_NSSA */



  ret = ospf_str2area_id (argv[0], &area_id);
  if (!ret)
    {
      vty_out (vty, "OSPF Area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (area_id.s_addr == OSPF_AREA_BACKBONE)
     return CMD_SUCCESS;

  area = ospf_area_lookup_by_area_id (area_id);
  if (!area)
    {
      vty_out (vty, "Area is not yet configured%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (no_summary)
    {
      area->no_summary = 0;
      ospf_area_check_free (area_id);
      return CMD_SUCCESS;
    }


#ifdef HAVE_NSSA

  if (area->external_routing == OSPF_AREA_NSSA)
  {
    ospf_area_type_set (area, OSPF_AREA_DEFAULT);

    ospf_area_check_free (area_id);

    if (ospf_top->anyNSSA >0)
      ospf_top->anyNSSA--;

    return CMD_SUCCESS;
  }
#endif /* HAVE_NSSA */


  if (area->external_routing == OSPF_AREA_STUB)
    ospf_area_type_set (area, OSPF_AREA_DEFAULT);
  else
    {
      vty_out (vty, "Area is not stub nor nssa%s", VTY_NEWLINE);
      return CMD_WARNING;
    }


  ospf_area_check_free (area_id);
  return CMD_SUCCESS;
}

/*********************************************************************************/

DEFUN (no_area_stub,
       no_area_stub_cmd,
       "no area A.B.C.D stub",
       NO_STR
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "Configure OSPF area as stub\n")
{
  return ospf_no_area_stub_cmd (vty, argc, argv, 0);
}

ALIAS (no_area_stub,
       no_area_stub_decimal_cmd,
       "no area <0-4294967295> stub",
       NO_STR
       "OSPF area parameters\n"
       "OSPF area ID as a decimal value\n"
       "Configure OSPF area as stub\n")

#ifdef HAVE_NSSA
/*********************************************************************************/

DEFUN (no_area_nssa,
       no_area_nssa_cmd,
       "no area A.B.C.D nssa",
       NO_STR
       "OSPF nssa parameters\n"
       "OSPF area ID in IP address format\n"
       "Configure OSPF area as nssa\n")
{
  return ospf_no_area_stub_cmd (vty, argc, argv, 2);
}

ALIAS (no_area_nssa,
       no_area_nssa_decimal_cmd,
       "no area <0-4294967295> nssa",
       NO_STR
       "OSPF nssa parameters\n"
       "OSPF area ID as a decimal value\n"
       "Configure OSPF area as nssa\n")

/*********************************************************************************/
#endif /* HAVE_NSSA */

DEFUN (no_area_stub_nosum,
       no_area_stub_nosum_cmd,
       "no area A.B.C.D stub no-summary",
       NO_STR
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "Configure OSPF area as stub\n"
       "Do not inject inter-area routes into area\n")
{
  return ospf_no_area_stub_cmd (vty, argc, argv, 1);
}

ALIAS (no_area_stub_nosum,
       no_area_stub_nosum_decimal_cmd,
       "no area <0-4294967295> stub no-summary",
       NO_STR
       "OSPF area parameters\n"
       "OSPF area ID as a decimal value\n"
       "Configure OSPF area as stub\n"
       "Do not inject inter-area routes into area\n")

#ifdef HAVE_NSSA
/*********************************************************************************/

DEFUN (no_area_nssa_nosum,
       no_area_nssa_nosum_cmd,
       "no area A.B.C.D nssa no-summary",
       NO_STR
       "OSPF nssa parameters\n"
       "OSPF area ID in IP address format\n"
       "Configure OSPF area as nssa\n"
       "Do not inject inter-area routes into nssa\n")
{
  return ospf_no_area_stub_cmd (vty, argc, argv, 3);
}

ALIAS (no_area_nssa_nosum,
       no_area_nssa_nosum_decimal_cmd,
       "no area <0-4294967295> nssa no-summary",
       NO_STR
       "OSPF nssa parameters\n"
       "OSPF area ID as a decimal value\n"
       "Configure OSPF area as nssa\n"
       "Do not inject inter-area routes into nssa\n")

/*********************************************************************************/
#endif /* HAVE_NSSA */

DEFUN (area_default_cost,
       area_default_cost_cmd,
       "area A.B.C.D default-cost <0-16777215>",
       "OSPF area parameters\n"
       "OSPF area ID\n"
       "Set the summary-default cost of a NSSA or stub area\n"
       "Stub's advertised default summary cost\n")
{
  struct ospf_area *area;
  struct in_addr area_id;
  u_int32_t cost;
  int ret;

  ret = ospf_str2area_id (argv[0], &area_id);
  if (!ret)
    {
      vty_out (vty, "OSPF Area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (area_id.s_addr == OSPF_AREA_BACKBONE)
    {
      vty_out (vty, "You cannot configure backbone area as shortcut%s",
               VTY_NEWLINE);
      return CMD_WARNING;
    }

  area = ospf_area_get (area_id, ret);

  if (area->external_routing == OSPF_AREA_DEFAULT)
    {
      vty_out (vty, "The area is neither stub, nor NSSA%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  cost = atol (argv[1]);

  if (cost > 16777215)
    {
      vty_out (vty, "Invalid cost value, expected <0-16777215>%s",
               VTY_NEWLINE);
      return CMD_WARNING;
    }

  area->default_cost = cost;

  return CMD_SUCCESS;
}

ALIAS (area_default_cost,
       area_default_cost_decimal_cmd,
       "area <0-4294967295> default-cost NAME",
       NO_STR
       "OSPF area parameters\n"
       "OSPF area ID as a decimal value\n"
       "Set the summary-default cost of a NSSA or stub area\n"
       "Stub's advertised default summary cost\n")

DEFUN (no_area_default_cost,
       no_area_default_cost_cmd,
       "no area A.B.C.D default-cost <0-16777215>",
       NO_STR
       "OSPF area parameters\n"
       "OSPF area ID\n"
       "Set the summary-default cost of a NSSA or stub area\n"
       "Stub's advertised default summary cost\n")
{
  struct ospf_area *area;
  struct in_addr area_id;
  u_int32_t cost;
  int ret;

  ret = ospf_str2area_id (argv[0], &area_id);
  if (!ret)
    {
      vty_out (vty, "OSPF Area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (area_id.s_addr == OSPF_AREA_BACKBONE)
    {
      vty_out (vty, "You cannot configure backbone area as shortcut%s",
               VTY_NEWLINE);
      return CMD_WARNING;
    }

  area = ospf_area_lookup_by_area_id (area_id);
  if (!area)
    {
      vty_out (vty, "Area is not yet configured%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (area->external_routing == OSPF_AREA_DEFAULT)
    {
      vty_out (vty, "The area is neither stub, nor NSSA%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  cost = atol (argv[1]);

  if (cost > 16777215)
    {
      vty_out (vty, "Invalid cost value, expected <0-16777215>%s",
               VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (cost != area->default_cost)
    {
      vty_out (vty, "Specified cost value is not equal to the configured one%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  area->default_cost = 1;

  ospf_area_check_free (area_id);
  return CMD_SUCCESS;
}

ALIAS (no_area_default_cost,
       no_area_default_cost_decimal_cmd,
       "no area <0-4294967295> default-cost NAME",
       NO_STR
       "OSPF area parameters\n"
       "OSPF area ID as a decimal value\n"
       "Set the summary-default cost of a NSSA or stub area\n"
       "Stub's advertised default summary cost\n")

int
ospf_set_area_export_list (struct ospf_area * area, char * list_name)
{
  struct access_list *list;
  list = access_list_lookup (AFI_IP, list_name);

  EXPORT_LIST (area) = list;

  if (EXPORT_NAME (area))
    free (EXPORT_NAME (area));

  EXPORT_NAME (area) = strdup (list_name);
  ospf_schedule_abr_task ();

  return CMD_SUCCESS;
}

int
ospf_unset_area_export_list (struct ospf_area * area)
{

  EXPORT_LIST (area) = 0;

  if (EXPORT_NAME (area))
    free (EXPORT_NAME (area));

  EXPORT_NAME (area) = NULL;

  ospf_area_check_free (area->area_id);
  
  ospf_schedule_abr_task ();
  return CMD_SUCCESS;
}

DEFUN (area_export_list,
       area_export_list_cmd,
       "area A.B.C.D export-list NAME",
       "OSPF area parameters\n"
       "OSPF area ID\n"
       "Set the filter for networks announced to other areas\n"
       "Name of the access-list\n")
{
  struct ospf_area *area;
  struct in_addr area_id;
  int ret;

  ret = ospf_str2area_id (argv[0], &area_id);
  if (!ret)
    {
      vty_out (vty, "OSPF Area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  area = ospf_area_get (area_id, ret);

  return ospf_set_area_export_list (area, argv[1]);
}

ALIAS (area_export_list,
       area_export_list_decimal_cmd,
       "area <0-4294967295> export-list NAME",
       "OSPF area parameters\n"
       "OSPF area ID as a decimal value\n"
       "Set the filter for networks announced to other areas\n"
       "Name of the access-list\n")

DEFUN (no_area_export_list,
       no_area_export_list_cmd,
       "no area A.B.C.D export-list NAME",
       NO_STR
       "OSPF area parameters\n"
       "OSPF area ID\n"
       "Unset the filter for networks announced to other areas\n"
       "Name of the access-list\n")
{
  struct ospf_area *area;
  struct in_addr area_id;
  int ret;

  ret = ospf_str2area_id (argv[0], &area_id);
  if (!ret)
    {
      vty_out (vty, "OSPF Area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  area = ospf_area_lookup_by_area_id (area_id);
  if (!area)
    {
      vty_out (vty, "Area is not yet configured%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  return ospf_unset_area_export_list(area);
}

ALIAS (no_area_export_list,
       no_area_export_list_decimal_cmd,
       "no area <0-4294967295> export-list NAME",
       NO_STR
       "OSPF area parameters\n"
       "OSPF area ID as a decimal value\n"
       "Unset the filter for networks announced to other areas\n"
       "Name of the access-list\n")

int
ospf_set_area_import_list (struct ospf_area *area, char *name)
{
  struct access_list *list;
  list = access_list_lookup (AFI_IP, name);

  IMPORT_LIST (area) = list;

  if (IMPORT_NAME (area))
    free (IMPORT_NAME (area));

  IMPORT_NAME (area) = strdup (name);
  ospf_schedule_abr_task ();

  return CMD_SUCCESS;
}

int
ospf_unset_area_import_list (struct ospf_area * area)
{

  IMPORT_LIST (area) = 0;

  if (IMPORT_NAME (area))
    free (IMPORT_NAME (area));

  IMPORT_NAME (area) = NULL;
  ospf_area_check_free (area->area_id);

  ospf_schedule_abr_task ();

  return CMD_SUCCESS;
}


DEFUN (area_import_list,
       area_import_list_cmd,
       "area A.B.C.D import-list NAME",
       "OSPF area parameters\n"
       "OSPF area ID\n"
       "Set the filter for networks from other areas announced to the specified one\n"
       "Name of the access-list\n")
{
  struct ospf_area *area;
  struct in_addr area_id;
  int ret;

  ret = ospf_str2area_id (argv[0], &area_id);
  if (!ret)
    {
      vty_out (vty, "OSPF Area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  area = ospf_area_get (area_id, ret);
  
  return ospf_set_area_import_list (area, argv[1]);
}

ALIAS (area_import_list,
       area_import_list_decimal_cmd,
       "area <0-4294967295> import-list NAME",
       "OSPF area parameters\n"
       "OSPF area ID as a decimal value\n"
       "Set the filter for networks from other areas announced to the specified one\n"
       "Name of the access-list\n")

DEFUN (no_area_import_list,
       no_area_import_list_cmd,
       "no area A.B.C.D import-list NAME",
       NO_STR
       "OSPF area parameters\n"
       "OSPF area ID\n"
       "Unset the filter for networks announced to other areas\n"
       "Name of the access-list\n")
{
  struct ospf_area *area;
  struct in_addr area_id;
  int ret;

  ret = ospf_str2area_id (argv[0], &area_id);
  if (!ret)
    {
      vty_out (vty, "OSPF Area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  area = ospf_area_lookup_by_area_id (area_id);
  if (!area)
    {
      vty_out (vty, "Area is not yet configured%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  return ospf_unset_area_import_list (area);
}

ALIAS (no_area_import_list,
       no_area_import_list_decimal_cmd,
       "no area <0-4294967295> import-list NAME",
       NO_STR
       "OSPF area parameters\n"
       "OSPF area ID as a decimal value\n"
       "Unset the filter for networks announced to other areas\n"
       "Name of the access-list\n")

DEFUN (area_authentication_message_digest,
       area_authentication_message_digest_cmd,
       "area A.B.C.D authentication message-digest",
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "Enable authentication\n"
       "Use message-digest authentication\n")
{
  struct ospf_area *area;
  struct in_addr area_id;
  int ret;

  ret = ospf_str2area_id (argv[0], &area_id);
  if (!ret)
    {
      vty_out (vty, "OSPF area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  area = ospf_area_get (area_id, ret);

  area->auth_type = OSPF_AUTH_CRYPTOGRAPHIC;

  return CMD_SUCCESS;
}

ALIAS (area_authentication_message_digest,
       area_authentication_message_digest_decimal_cmd,
       "area <0-4294967295> authentication message-digest",
       "OSPF area parameters\n"
       "OSPF area ID as a decimal value\n"
       "Enable authentication\n"
       "Use message-digest authentication\n")

DEFUN (area_authentication,
       area_authentication_cmd,
       "area A.B.C.D authentication",
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "Enable authentication\n")
{
  struct ospf_area *area;
  struct in_addr area_id;
  int ret;

  ret = ospf_str2area_id (argv[0], &area_id);
  if (!ret)
    {
      vty_out (vty, "OSPF area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  area = ospf_area_get (area_id, ret);

  area->auth_type = OSPF_AUTH_SIMPLE;

  return CMD_SUCCESS;
}

ALIAS (area_authentication,
       area_authentication_decimal_cmd,
       "area <0-4294967295> authentication",
       "OSPF area parameters\n"
       "OSPF area ID as a decimal value\n"
       "Enable authentication\n")

DEFUN (no_area_authentication,
       no_area_authentication_cmd,
       "no area A.B.C.D authentication",
       NO_STR
       "OSPF area parameters\n"
       "OSPF area ID in IP address format\n"
       "Enable authentication\n")
{
  struct ospf_area *area;
  struct in_addr area_id;

  if (!ospf_str2area_id (argv[0], &area_id))
    {
      vty_out (vty, "OSPF area ID is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  area = ospf_area_lookup_by_area_id (area_id);
  if (!area)
    {
      vty_out (vty, "Area ID %s is not declared%s", inet_ntoa (area_id),
               VTY_NEWLINE);
      return CMD_WARNING;
    }

  area->auth_type = OSPF_AUTH_NULL;

  ospf_area_check_free (area_id);
  
  return CMD_SUCCESS;
}

ALIAS (no_area_authentication,
       no_area_authentication_decimal_cmd,
       "no area <0-4294967295> authentication",
       NO_STR
       "OSPF area parameters\n"
       "OSPF area ID as a decimal value\n"
       "Enable authentication\n")


DEFUN (ospf_abr_type,
       ospf_abr_type_cmd,
       "ospf abr-type (cisco|ibm|shortcut|standard)",
       "OSPF specific commands\n"
       "Set OSPF ABR type\n"
       "Alternative ABR, cisco implementation\n"
       "Alternative ABR, IBM implementation\n"
       "Shortcut ABR\n"
       "Standard behavior (RFC2328)\n")
{
  u_char abr_type = OSPF_ABR_UNKNOWN;

  if (strncmp (argv[0], "c", 1) == 0)
    abr_type = OSPF_ABR_CISCO;
  else if (strncmp (argv[0], "i", 1) == 0)
    abr_type = OSPF_ABR_IBM;
  else if (strncmp (argv[0], "sh", 2) == 0)
    abr_type = OSPF_ABR_SHORTCUT;
  else if (strncmp (argv[0], "st", 2) == 0)
    abr_type = OSPF_ABR_STAND;
  else
    return CMD_WARNING;

  /* If ABR type value is changed, schedule ABR task. */
  if (ospf_top->abr_type != abr_type)
    {
      ospf_top->abr_type = abr_type;
      ospf_schedule_abr_task ();
    }

  return CMD_SUCCESS;
}

DEFUN (no_ospf_abr_type,
       no_ospf_abr_type_cmd,
       "no ospf abr-type (cisco|ibm|shortcut)",
       NO_STR
       "OSPF specific commands\n"
       "Set OSPF ABR type\n"
       "Alternative ABR, cisco implementation\n"
       "Alternative ABR, IBM implementation\n"
       "Shortcut ABR\n")
{
  u_char abr_type = OSPF_ABR_UNKNOWN;

  if (strncmp (argv[0], "c", 1) == 0)
    abr_type = OSPF_ABR_CISCO;
  else if (strncmp (argv[0], "i", 1) == 0)
    abr_type = OSPF_ABR_IBM;
  else if (strncmp (argv[0], "s", 1) == 0)
    abr_type = OSPF_ABR_SHORTCUT;
  else
    return CMD_WARNING;

  /* If ABR type value is changed, schedule ABR task. */
  if (ospf_top->abr_type == abr_type)
    {
      ospf_top->abr_type = OSPF_ABR_STAND;
      ospf_schedule_abr_task ();
    }

  return CMD_SUCCESS;
}

DEFUN (ospf_compatible_rfc1583,
       ospf_compatible_rfc1583_cmd,
       "compatible rfc1583",
       "OSPF compatibility list\n"
       "compatible with RFC 1583\n")
{
  if (ospf_top->RFC1583Compat == 0)
    {
      ospf_top->RFC1583Compat = 1;
      ospf_spf_calculate_schedule ();
    }
  return CMD_SUCCESS;
}

DEFUN (no_ospf_compatible_rfc1583,
       no_ospf_compatible_rfc1583_cmd,
       "no compatible rfc1583",
       NO_STR
       "OSPF compatibility list\n"
       "compatible with RFC 1583\n")
{
  if (ospf_top->RFC1583Compat == 1)
    {
      ospf_top->RFC1583Compat = 0;
      ospf_spf_calculate_schedule ();
    }
  return CMD_SUCCESS;
}

ALIAS (ospf_compatible_rfc1583,
       ospf_rfc1583_flag_cmd,
       "ospf rfc1583compatibility",
       "OSPF specific commands\n"
       "Enable the RFC1583Compatibility flag\n")

ALIAS (no_ospf_compatible_rfc1583,
       no_ospf_rfc1583_flag_cmd,
       "no ospf rfc1583compatibility",
       NO_STR
       "OSPF specific commands\n"
       "Disable the RFC1583Compatibility flag\n")

char *ospf_abr_type_descr_str[] = 
{
  "Unknown",
  "Standard (RFC2328)",
  "Alternative IBM",
  "Alternative Cisco",
  "Alternative Shortcut"
};

char *ospf_shortcut_mode_descr_str[] = 
{
  "Default",
  "Enabled",
  "Disabled"
};



void
show_ip_ospf_area (struct vty *vty, struct ospf_area *area)
{
  /* Show Area ID. */
  vty_out (vty, " Area ID: %s", inet_ntoa (area->area_id));

  /* Show Area type/mode. */
  if (OSPF_IS_AREA_BACKBONE (area))
    vty_out (vty, " (Backbone)%s", VTY_NEWLINE);
  else
    {
      if (area->external_routing == OSPF_AREA_STUB)
	vty_out (vty, " (Stub%s%s)",
		 area->no_summary ? ", no summary" : "",
		 area->shortcut_configured ? "; " : "");

#ifdef HAVE_NSSA

      else
      if (area->external_routing == OSPF_AREA_NSSA)
	vty_out (vty, " (NSSA%s%s)",
		 area->no_summary ? ", no summary" : "",
		 area->shortcut_configured ? "; " : "");
#endif /* HAVE_NSSA */

      vty_out (vty, "%s", VTY_NEWLINE);
      vty_out (vty, "   Shortcutting mode: %s",
	       ospf_shortcut_mode_descr_str[area->shortcut_configured]);
      vty_out (vty, ", S-bit consensus: %s%s",
	       area->shortcut_capability ? "ok" : "no", VTY_NEWLINE);
    }

  /* Show number of interfaces. */
  vty_out (vty, "   Number of interfaces in this area: Total: %d, "
	   "Active: %d%s", listcount (area->oiflist),
	   area->act_ints, VTY_NEWLINE);

#ifdef HAVE_NSSA
  if (area->external_routing == OSPF_AREA_NSSA)
    {
      vty_out (vty, "   It is an NSSA configuration. %s   Elected NSSA/ABR performs type-7/type-5 LSA translation. %s", VTY_NEWLINE, VTY_NEWLINE);
      if (! OSPF_IS_ABR)
	vty_out (vty, "   It is not ABR, therefore not Translator. %s",
		 VTY_NEWLINE);
      else
	{
	  if (area->NSSATranslator)
	    vty_out (vty, "   We are an ABR and the NSSA Elected Translator. %s", VTY_NEWLINE);
	  else
	    vty_out (vty, "   We are an ABR, but not the NSSA Elected Translator. %s", VTY_NEWLINE);
	}
    }
#endif /* HAVE_NSSA */

  /* Show number of fully adjacent neighbors. */
  vty_out (vty, "   Number of fully adjacent neighbors in this area:"
	   " %d%s", area->full_nbrs, VTY_NEWLINE);

  /* Show authentication type. */
  vty_out (vty, "   Area has ");
  if (area->auth_type == OSPF_AUTH_NULL)
    vty_out (vty, "no authentication%s", VTY_NEWLINE);
  else if (area->auth_type == OSPF_AUTH_SIMPLE)
    vty_out (vty, "simple password authentication%s", VTY_NEWLINE);
  else if (area->auth_type == OSPF_AUTH_CRYPTOGRAPHIC)
    vty_out (vty, "message digest authentication%s", VTY_NEWLINE);

  if (!OSPF_IS_AREA_BACKBONE (area))
    vty_out (vty, "   Number of full virtual adjacencies going through"
	     " this area: %d%s", area->full_vls, VTY_NEWLINE);

  /* Show SPF calculation times. */
  vty_out (vty, "   SPF algorithm executed %d times%s",
	   area->spf_calculation, VTY_NEWLINE);

  /* Show number of LSA. */
  vty_out (vty, "   Number of LSA %ld%s", area->lsdb->total, VTY_NEWLINE);

  vty_out (vty, "%s", VTY_NEWLINE);
}

DEFUN (show_ip_ospf,
       show_ip_ospf_cmd,
       "show ip ospf",
       SHOW_STR
       IP_STR
       "OSPF information\n")
{
  listnode node;
  struct ospf_area * area;

  /* Check OSPF is enable. */
  if (ospf_top == NULL)
    {
      vty_out (vty, " OSPF Routing Process not enabled%s", VTY_NEWLINE);
      return CMD_SUCCESS;
    }

  /* Show Router ID. */
  vty_out (vty, " OSPF Routing Process, Router ID: %s%s",
           inet_ntoa (ospf_top->router_id),
           VTY_NEWLINE);

  /* Show capability. */
  vty_out (vty, " Supports only single TOS (TOS0) routes%s", VTY_NEWLINE);
  vty_out (vty, " This implementation conforms to RFC2328%s", VTY_NEWLINE);
  vty_out (vty, " RFC1583Compatibility flag is %s%s",
	   ospf_top->RFC1583Compat ? "enabled" : "disabled", VTY_NEWLINE);

  /* Show SPF timers. */
  vty_out (vty, " SPF schedule delay %d secs, Hold time between two SPFs %d secs%s",
	   ospf_top->spf_delay, ospf_top->spf_holdtime, VTY_NEWLINE);

  /* Show refresh parameters. */
  vty_out (vty, " Refresh timer %d secs%s",
	   ospf_top->lsa_refresh_interval, VTY_NEWLINE);
	   
  /* Show ABR/ASBR flags. */
  if (CHECK_FLAG (ospf_top->flags, OSPF_FLAG_ABR))
    vty_out (vty, " This router is an ABR, ABR type is: %s%s",
             ospf_abr_type_descr_str[ospf_top->abr_type], VTY_NEWLINE);

  if (CHECK_FLAG (ospf_top->flags, OSPF_FLAG_ASBR))
    vty_out (vty, " This router is an ASBR "
             "(injecting external routing information)%s", VTY_NEWLINE);

  /* Show Number of AS-external-LSAs. */
  vty_out (vty, " Number of external LSA %ld%s",
	   ospf_lsdb_count_all (ospf_top->lsdb), VTY_NEWLINE);

  /* Show number of areas attached. */
  vty_out (vty, " Number of areas attached to this router: %d%s%s",
           listcount (ospf_top->areas), VTY_NEWLINE, VTY_NEWLINE);

  /* Show each area status. */
  for (node = listhead (ospf_top->areas); node; nextnode (node))
    if ((area = getdata (node)) != NULL)
      show_ip_ospf_area (vty, area);

  return CMD_SUCCESS;
}

int
ospf_oi_count (struct interface *ifp)
{
  int i = 0;
  struct route_node *rn;

  for (rn = route_top (IF_OIFS (ifp)); rn; rn = route_next (rn))
    if (rn->info)
      i++;

  return i;
}

void
show_ip_ospf_interface_sub (struct vty *vty, struct interface *ifp)
{
  struct ospf_neighbor *nbr;
  int oi_count;
  struct route_node *rn;
  char buf[9];

  oi_count = ospf_oi_count (ifp);
  
  /* Is interface up? */
  if (if_is_up (ifp))
    vty_out (vty, "%s is up, line protocol is up%s", ifp->name, VTY_NEWLINE);
  else
    {
      vty_out (vty, "%s is down, line protocol is down%s", ifp->name,
	       VTY_NEWLINE);

      
      if (oi_count == 0)
	vty_out (vty, "  OSPF not enabled on this interface%s", VTY_NEWLINE);
      else
	vty_out (vty, "  OSPF is enabled, but not running on this interface%s",
		 VTY_NEWLINE);
      return;
    }

  /* Is interface OSPF enabled? */
  if (oi_count == 0)
    {
      vty_out (vty, "  OSPF not enabled on this interface%s", VTY_NEWLINE);
      return;
    }
  
  for (rn = route_top (IF_OIFS (ifp)); rn; rn = route_next (rn))
    {
      struct ospf_interface *oi = rn->info;
      
      if (oi == NULL)
	continue;
      
      /* Show OSPF interface information. */
      vty_out (vty, "%s  Internet Address %s/%d,", VTY_NEWLINE,
	       inet_ntoa (oi->address->u.prefix4), oi->address->prefixlen);

      vty_out (vty, " Area %s%s", ait_ntoa (oi->area->area_id, oi->area->external_routing), 
	       VTY_NEWLINE);

      vty_out (vty, "  Router ID %s, Network Type %s, Cost: %d%s",
	       inet_ntoa (ospf_top->router_id), ospf_network_type_str[oi->type],
	       oi->output_cost, VTY_NEWLINE);

      vty_out (vty, "  Transmit Delay is %d sec, State %s, Priority %d%s",
	       OSPF_IF_PARAM (oi,transmit_delay), LOOKUP (ospf_ism_status_msg, oi->status),
	       PRIORITY (oi), VTY_NEWLINE);

  /* Show DR information. */
      if (DR (oi).s_addr == 0)
	vty_out (vty, "  No designated router on this network%s", VTY_NEWLINE);
      else
	{
	  nbr = ospf_nbr_lookup_by_addr (oi->nbrs, &DR (oi));
	  if (nbr == NULL)
	    vty_out (vty, "  No designated router on this network%s", VTY_NEWLINE);
	  else
	    {
	      vty_out (vty, "  Designated Router (ID) %s,",
		       inet_ntoa (nbr->router_id));
	      vty_out (vty, " Interface Address %s%s",
		       inet_ntoa (nbr->address.u.prefix4), VTY_NEWLINE);
	    }
	}

      /* Show BDR information. */
      if (BDR (oi).s_addr == 0)
	vty_out (vty, "  No backup designated router on this network%s",
		 VTY_NEWLINE);
      else
	{
	  nbr = ospf_nbr_lookup_by_addr (oi->nbrs, &BDR (oi));
	  if (nbr == NULL)
	    vty_out (vty, "  No backup designated router on this network%s",
		     VTY_NEWLINE);
	  else
	    {
	      vty_out (vty, "  Backup Designated Router (ID) %s,",
		       inet_ntoa (nbr->router_id));
	      vty_out (vty, " Interface Address %s%s",
		       inet_ntoa (nbr->address.u.prefix4), VTY_NEWLINE);
	    }
	}
      vty_out (vty, "  Timer intarvals configured,");
      vty_out (vty, " Hello %d, Dead %d, Wait %d, Retransmit %d%s",
	       OSPF_IF_PARAM (oi, v_hello), OSPF_IF_PARAM (oi, v_wait),
	       OSPF_IF_PARAM (oi, v_wait),
	       OSPF_IF_PARAM (oi, retransmit_interval),
	       VTY_NEWLINE);
      
      if (OSPF_IF_PARAM (oi, passive_interface) == OSPF_IF_ACTIVE)
	vty_out (vty, "    Hello due in %s%s",
		 ospf_timer_dump (oi->t_hello, buf, 9), VTY_NEWLINE);
      else /* OSPF_IF_PASSIVE is set */
	vty_out (vty, "    No Hellos (Passive interface)%s", VTY_NEWLINE);
      
      vty_out (vty, "  Neighbor Count is %d, Adjacent neighbor count is %d%s",
	       ospf_nbr_count (oi->nbrs, 0), ospf_nbr_count (oi->nbrs, NSM_Full),
	       VTY_NEWLINE);
    }
}

DEFUN (show_ip_ospf_interface,
       show_ip_ospf_interface_cmd,
       "show ip ospf interface [INTERFACE]",
       SHOW_STR
       IP_STR
       "OSPF information\n"
       "Interface information\n"
       "Interface name\n")
{
  struct interface *ifp;
  listnode node;

  /* Show All Interfaces. */
  if (argc == 0)
    for (node = listhead (iflist); node; nextnode (node))
      show_ip_ospf_interface_sub (vty, node->data);
  /* Interface name is specified. */
  else
    {
      if ((ifp = if_lookup_by_name (argv[0])) == NULL)
        vty_out (vty, "No such interface name%s", VTY_NEWLINE);
      else
        show_ip_ospf_interface_sub (vty, ifp);
    }

  return CMD_SUCCESS;
}

void
show_ip_ospf_neighbor_sub (struct vty *vty, struct ospf_interface *oi)
{
  struct route_node *rn;
  struct ospf_neighbor *nbr;
  char msgbuf[16];
  char timebuf[9];

  for (rn = route_top (oi->nbrs); rn; rn = route_next (rn))
    if ((nbr = rn->info))
      /* Do not show myself. */
      if (nbr != oi->nbr_self)
	/* Down state is not shown. */
	if (nbr->status != NSM_Down)
	  {
	    ospf_nbr_state_message (nbr, msgbuf, 16);

	    if (nbr->status == NSM_Attempt && nbr->router_id.s_addr == 0)
	    vty_out (vty, "%-15s %3d   %-15s %8s    ",
		     "-", nbr->priority,
		     msgbuf, ospf_timer_dump (nbr->t_inactivity, timebuf, 9));
	    else
	    vty_out (vty, "%-15s %3d   %-15s %8s    ",
		     inet_ntoa (nbr->router_id), nbr->priority,
		     msgbuf, ospf_timer_dump (nbr->t_inactivity, timebuf, 9));
	    vty_out (vty, "%-15s ", inet_ntoa (nbr->src));
	    vty_out (vty, "%-15s %5ld %5ld %5d%s",
		     IF_NAME (oi), ospf_ls_retransmit_count (nbr),
		     ospf_ls_request_count (nbr), ospf_db_summary_count (nbr),
		     VTY_NEWLINE);
	  }
}

DEFUN (show_ip_ospf_neighbor,
       show_ip_ospf_neighbor_cmd,
       "show ip ospf neighbor",
       SHOW_STR
       IP_STR
       "OSPF information\n"
       "Neighbor list\n")
{
  listnode node;

  if (!ospf_top)
    {
      vty_out (vty, " OSPF Routing Process not enabled%s", VTY_NEWLINE);
      return CMD_SUCCESS;
    }

  /* Show All neighbors. */
  vty_out (vty, "%sNeighbor ID     Pri   State           Dead "
           "Time   Address         Interface           RXmtL "
           "RqstL DBsmL%s", VTY_NEWLINE, VTY_NEWLINE);

  for (node = listhead (ospf_top->oiflist); node; nextnode (node))
      show_ip_ospf_neighbor_sub (vty, getdata (node));

  return CMD_SUCCESS;
}

DEFUN (show_ip_ospf_neighbor_all,
       show_ip_ospf_neighbor_all_cmd,
       "show ip ospf neighbor all",
       SHOW_STR
       IP_STR
       "OSPF information\n"
       "Neighbor list\n"
       "include down status neighbor\n")
{
  listnode node;

  if (!ospf_top)
    {
      vty_out (vty, " OSPF Routing Process not enabled%s", VTY_NEWLINE);
      return CMD_SUCCESS;
    }

  /* Show All neighbors. */
  vty_out (vty, "%sNeighbor ID     Pri   State           Dead "
           "Time   Address         Interface           RXmtL "
           "RqstL DBsmL%s", VTY_NEWLINE, VTY_NEWLINE);

  for (node = listhead (ospf_top->oiflist); node; nextnode (node))
    {
      struct ospf_interface *oi = getdata (node);
      listnode nbr_node;

      show_ip_ospf_neighbor_sub (vty, oi);

    /* print Down neighbor status */
    for (nbr_node = listhead (oi->nbr_static); nbr_node; nextnode (nbr_node))
      {
	struct ospf_nbr_static *nbr_static;

	nbr_static = getdata (nbr_node);

	if (nbr_static->neighbor == NULL
	    || nbr_static->neighbor->status == NSM_Down)
	  {
	    vty_out (vty, "%-15s %3d   %-15s %8s    ",
		     "-", nbr_static->priority, "Down", "-");
	    vty_out (vty, "%-15s %-15s %5d %5d %5d%s", 
		     inet_ntoa (nbr_static->addr), IF_NAME (oi),
		     0, 0, 0, VTY_NEWLINE);
	  }
      }
    }

  return CMD_SUCCESS;
}

DEFUN (show_ip_ospf_neighbor_int,
       show_ip_ospf_neighbor_int_cmd,
       "show ip ospf neighbor A.B.C.D",
       SHOW_STR
       IP_STR
       "OSPF information\n"
       "Neighbor list\n"
       "Interface name\n")
{
  struct ospf_interface *oi;
  struct in_addr addr;
  int ret;
  
  if (!ospf_top)
    {
      vty_out (vty, " OSPF Routing Process not enabled%s", VTY_NEWLINE);
      return CMD_SUCCESS;
    }

  ret = inet_aton (argv[0], &addr);
  if (!ret)
    {
      vty_out (vty, "Please specify interface address by A.B.C.D%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  if ((oi = ospf_if_is_configured (&addr)) == NULL)
    vty_out (vty, "No such interface address%s", VTY_NEWLINE);
  else
    {
      vty_out (vty, "%sNeighbor ID     Pri   State           Dead "
               "Time   Address         Interface           RXmtL "
               "RqstL DBsmL%s", VTY_NEWLINE, VTY_NEWLINE);
      show_ip_ospf_neighbor_sub (vty, oi);
    }

  return CMD_SUCCESS;
}

void
show_ip_ospf_nbr_static_detail_sub (struct vty *vty, struct ospf_interface *oi,
				  struct ospf_nbr_static *nbr_static)
{
  char timebuf[9];

  /* Show neighbor ID. */
  vty_out (vty, " Neighbor %s,", "-");

  /* Show interface address. */
  vty_out (vty, " interface address %s%s",
	   inet_ntoa (nbr_static->addr), VTY_NEWLINE);
  /* Show Area ID. */
  vty_out (vty, "    In the area %s via interface %s%s",
	   ait_ntoa (oi->area->area_id, oi->area->external_routing), IF_NAME (oi), VTY_NEWLINE);
  /* Show neighbor priority and state. */
  vty_out (vty, "    Neighbor priority is %d, State is %s,",
	   nbr_static->priority, "Down");
  /* Show state changes. */
  vty_out (vty, " %d state changes%s", nbr_static->state_change, VTY_NEWLINE);

  /* Show PollInterval */
  vty_out (vty, "    Poll interval %d%s", nbr_static->v_poll, VTY_NEWLINE);

  /* Show poll-interval timer. */
  vty_out (vty, "    Poll timer due in %s%s",
	   ospf_timer_dump (nbr_static->t_poll, timebuf, 9), VTY_NEWLINE);

  /* Show poll-interval timer thread. */
  vty_out (vty, "    Thread Poll Timer %s%s", 
	   nbr_static->t_poll != NULL ? "on" : "off", VTY_NEWLINE);
}

void
show_ip_ospf_neighbor_detail_sub (struct vty *vty, struct ospf_interface *oi,
				  struct ospf_neighbor *nbr)
{
  char optbuf[24];
  char timebuf[9];

  /* Show neighbor ID. */
  if (nbr->status == NSM_Attempt && nbr->router_id.s_addr == 0)
    vty_out (vty, " Neighbor %s,", "-");
  else
  vty_out (vty, " Neighbor %s,", inet_ntoa (nbr->router_id));

  /* Show interface address. */
  vty_out (vty, " interface address %s%s",
	   inet_ntoa (nbr->address.u.prefix4), VTY_NEWLINE);
  /* Show Area ID. */
  vty_out (vty, "    In the area %s via interface %s%s",
	   ait_ntoa (oi->area->area_id, oi->area->external_routing), oi->ifp->name, VTY_NEWLINE);
  /* Show neighbor priority and state. */
  vty_out (vty, "    Neighbor priority is %d, State is %s,",
	   nbr->priority, LOOKUP (ospf_nsm_status_msg, nbr->status));
  /* Show state changes. */
  vty_out (vty, " %d state changes%s", nbr->state_change, VTY_NEWLINE);

  /* Show Designated Rotuer ID. */
  vty_out (vty, "    DR is %s,", inet_ntoa (nbr->d_router));
  /* Show Backup Designated Rotuer ID. */
  vty_out (vty, " BDR is %s%s", inet_ntoa (nbr->bd_router), VTY_NEWLINE);
  /* Show options. */
  vty_out (vty, "    Options %d %s%s", nbr->options,
	   ospf_option_dump (nbr->options, optbuf, 24), VTY_NEWLINE);
  /* Show Router Dead interval timer. */
  vty_out (vty, "    Dead timer due in %s%s",
	   ospf_timer_dump (nbr->t_inactivity, timebuf, 9), VTY_NEWLINE);
  /* Show Database Summary list. */
  vty_out (vty, "    Database Summary List %d%s",
	   ospf_db_summary_count (nbr), VTY_NEWLINE);
  /* Show Link State Request list. */
  vty_out (vty, "    Link State Request List %ld%s",
	   ospf_ls_request_count (nbr), VTY_NEWLINE);
  /* Show Link State Retransmission list. */
  vty_out (vty, "    Link State Retransmission List %ld%s",
	   ospf_ls_retransmit_count (nbr), VTY_NEWLINE);
  /* Show inactivity timer thread. */
  vty_out (vty, "    Thread Inactivity Timer %s%s", 
	   nbr->t_inactivity != NULL ? "on" : "off", VTY_NEWLINE);
  /* Show Database Description retransmission thread. */
  vty_out (vty, "    Thread Database Description Retransmision %s%s",
	   nbr->t_db_desc != NULL ? "on" : "off", VTY_NEWLINE);
  /* Show Link State Request Retransmission thread. */
  vty_out (vty, "    Thread Link State Request Retransmission %s%s",
	   nbr->t_ls_req != NULL ? "on" : "off", VTY_NEWLINE);
  /* Show Link State Update Retransmission thread. */
  vty_out (vty, "    Thread Link State Update Retransmission %s%s%s",
	   nbr->t_ls_upd != NULL ? "on" : "off", VTY_NEWLINE, VTY_NEWLINE);
}

DEFUN (show_ip_ospf_neighbor_id,
       show_ip_ospf_neighbor_id_cmd,
       "show ip ospf neighbor A.B.C.D",
       SHOW_STR
       IP_STR
       "OSPF information\n"
       "Neighbor list\n"
       "Neighbor ID\n")
{
  listnode node;
  struct ospf_neighbor *nbr;
  struct in_addr router_id;
  int ret;

  ret = inet_aton (argv[0], &router_id);
  if (!ret)
    {
      vty_out (vty, "Please specify Neighbor ID by A.B.C.D%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  for (node = listhead (ospf_top->oiflist); node; nextnode (node))
    {
      struct ospf_interface *oi = getdata (node);

      if ((nbr = ospf_nbr_lookup_by_routerid (oi->nbrs, &router_id)))
	{
	  show_ip_ospf_neighbor_detail_sub (vty, oi, nbr);
	  return CMD_SUCCESS;
	}
    }

  /* Nothing to show. */
  return CMD_SUCCESS;
}

DEFUN (show_ip_ospf_neighbor_detail,
       show_ip_ospf_neighbor_detail_cmd,
       "show ip ospf neighbor detail",
       SHOW_STR
       IP_STR
       "OSPF information\n"
       "Neighbor list\n"
       "detail of all neighbors\n")
{
  listnode node;

  if (!ospf_top)
    return CMD_SUCCESS;

  for (node = listhead (ospf_top->oiflist); node; nextnode (node))
    {
      struct ospf_interface *oi = getdata (node);
      struct route_node *rn;
      struct ospf_neighbor *nbr;

      for (rn = route_top (oi->nbrs); rn; rn = route_next (rn))
	if ((nbr = rn->info))
	  if (nbr != oi->nbr_self)
	    if (nbr->status != NSM_Down)
	      show_ip_ospf_neighbor_detail_sub (vty, oi, nbr);
    }

  return CMD_SUCCESS;
}

DEFUN (show_ip_ospf_neighbor_detail_all,
       show_ip_ospf_neighbor_detail_all_cmd,
       "show ip ospf neighbor detail all",
       SHOW_STR
       IP_STR
       "OSPF information\n"
       "Neighbor list\n"
       "detail of all neighbors\n"
       "include down status neighbor\n")
{
  listnode node;

  if (!ospf_top)
    return CMD_SUCCESS;

  for (node = listhead (ospf_top->oiflist); node; nextnode (node))
    {
      struct ospf_interface *oi = getdata (node);
      struct route_node *rn;
      struct ospf_neighbor *nbr;

      for (rn = route_top (oi->nbrs); rn; rn = route_next (rn))
	if ((nbr = rn->info))
	  if (nbr != oi->nbr_self)
	    if (oi->type == OSPF_IFTYPE_NBMA && nbr->status != NSM_Down)
	      show_ip_ospf_neighbor_detail_sub (vty, oi, rn->info);

      if (oi->type == OSPF_IFTYPE_NBMA)
	{
	  listnode nd;

	  for (nd = listhead (oi->nbr_static); nd; nextnode (nd))
	    {
	      struct ospf_nbr_static *nbr_static = getdata (nd);
	      if (nbr_static->neighbor == NULL
		  || nbr_static->neighbor->status == NSM_Down)
		show_ip_ospf_nbr_static_detail_sub (vty, oi, nbr_static);
	    }
	}
    }

  return CMD_SUCCESS;
}

DEFUN (show_ip_ospf_neighbor_int_detail,
       show_ip_ospf_neighbor_int_detail_cmd,
       "show ip ospf neighbor A.B.C.D detail",
       SHOW_STR
       IP_STR
       "OSPF information\n"
       "Neighbor list\n"
       "Interface address\n"
       "detail of all neighbors")
{
  struct ospf_interface *oi;
  struct in_addr addr;
  int ret;
  
  ret = inet_aton (argv[0], &addr);
  if (!ret)
    {
      vty_out (vty, "Please specify interface address by A.B.C.D%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  if ((oi = ospf_if_is_configured (&addr)) == NULL)
    vty_out (vty, "No such interface address%s", VTY_NEWLINE);
  else
    {
      struct route_node *rn;
      struct ospf_neighbor *nbr;

      for (rn = route_top (oi->nbrs); rn; rn = route_next (rn))
	if ((nbr = rn->info))
	  if (nbr != oi->nbr_self)
	    if (nbr->status != NSM_Down)
	      show_ip_ospf_neighbor_detail_sub (vty, oi, nbr);
    }

  return CMD_SUCCESS;
}

DEFUN (timers_spf,
       timers_spf_cmd,
       "timers spf <0-4294967295> <0-4294967295>",
       "Adjust routing timers\n"
       "OSPF SPF timers\n"
       "Delay between receiving a change to SPF calculation\n"
       "Hold time between consecutive SPF calculations\n")
{
  u_int32_t delay, hold;

  delay = strtoul (argv[0], NULL, 10);
  hold = strtoul (argv[1], NULL, 10);

  if (delay < 0 || delay > 0xFFFFFFFF)
    {
      vty_out (vty, "SPF delay timer value is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (hold < 0 || hold > 0xFFFFFFFF)
    {
      vty_out (vty, "SPF hold timer value is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  ospf_top->spf_delay = delay;
  ospf_top->spf_holdtime = hold;

  return CMD_SUCCESS;
}

DEFUN (no_timers_spf,
       no_timers_spf_cmd,
       "no timers spf",
       NO_STR
       "Adjust routing timers\n"
       "OSPF SPF timers\n")
{
  ospf_top->spf_delay = OSPF_SPF_DELAY_DEFAULT;
  ospf_top->spf_holdtime = OSPF_SPF_HOLDTIME_DEFAULT;

  return CMD_SUCCESS;
}


void
ospf_nbr_static_add (struct ospf_nbr_static *nbr_static,
		     struct ospf_interface *oi)
{
  struct ospf_neighbor *nbr;
  struct route_node *rn;
  struct prefix key;

  assert (oi);

  if (oi->type != OSPF_IFTYPE_NBMA)
    return;

  if (nbr_static->neighbor != NULL)
    return;

  if (IPV4_ADDR_SAME(&oi->nbr_self->address.u.prefix4, &nbr_static->addr))
    return;
      
  nbr_static->oi = oi;
  listnode_add (oi->nbr_static, nbr_static);

  /* Get neighbor information from table. */
  key.family = AF_INET;
  key.prefixlen = IPV4_MAX_BITLEN;
  key.u.prefix4.s_addr = nbr_static->addr.s_addr;

  rn = route_node_get (oi->nbrs, &key);

  if (rn->info)
    {
      nbr = rn->info;

      nbr->nbr_static = nbr_static;
      nbr_static->neighbor = nbr;

      route_unlock_node (rn);
    }
  else
    {
      nbr = ospf_nbr_new (oi);
      nbr->status = NSM_Down;
      nbr->src.s_addr = nbr_static->addr.s_addr;
      nbr->nbr_static = nbr_static;
      nbr->priority = nbr_static->priority;
      nbr->address = key;

      rn->info = nbr;

      nbr_static->neighbor = nbr;

      OSPF_NSM_EVENT_EXECUTE (nbr, NSM_Start);
    }
}

void
ospf_nbr_static_if_update (struct ospf_interface *oi)
{
  struct ospf_nbr_static *nbr_static;
  listnode node;

  assert(oi);

  if (oi->type != OSPF_IFTYPE_NBMA)
    return;

  for (node = listhead (ospf_top->nbr_static); node; nextnode (node))
    {
      struct prefix p;

      nbr_static = getdata (node);
      assert (nbr_static);

      if (nbr_static->oi != NULL)
	continue;

      if (nbr_static->neighbor != NULL)
	continue;

      p.family = AF_INET;
      p.prefixlen = IPV4_MAX_BITLEN;
      p.u.prefix4 = nbr_static->addr;

      if (prefix_match(oi->address, &p))
	ospf_nbr_static_add (nbr_static, oi);
    }
}

struct ospf_nbr_static *
ospf_nbr_static_lookup_by_addr (struct in_addr addr)
{
  listnode node;
  struct ospf_nbr_static *nbr_static;

  if (! ospf_top)
    return NULL;

  for (node = listhead (ospf_top->nbr_static); node; nextnode (node))
    {
      nbr_static = getdata (node);

      if (IPV4_ADDR_SAME(&nbr_static->addr, &addr))
	return nbr_static;
    }
  return NULL;
}

struct ospf_nbr_static *
ospf_nbr_static_lookup_next (struct in_addr *addr, int first)
{
  listnode node;
  struct ospf_nbr_static *nbr_static;

  if (! ospf_top)
    return NULL;

  for (node = listhead (ospf_top->nbr_static); node; nextnode (node))
    {
      nbr_static = getdata (node);

      if (first)
	{
	  *addr = nbr_static->addr;
	  return nbr_static;
	}
      else if (ntohl (nbr_static->addr.s_addr) > ntohl (addr->s_addr))
	{
	  *addr = nbr_static->addr;
	  return nbr_static;
	}
    }
  return NULL;
}

int
ospf_nbr_static_new (char *nbr_addr, int priority, int poll_interval,
		     struct vty *vty)
{
  struct ospf_nbr_static *nbr_static;
  listnode node;
  int ret;
  struct in_addr addr;
  struct prefix p;

  ret = inet_aton (nbr_addr, &addr);
  if (!ret)
    {
      vty_out (vty, "Please specify Neighbor address by A.B.C.D%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  for (node = listhead (ospf_top->nbr_static); node; nextnode (node))
    {
      nbr_static = getdata (node);

      /* this neighbor is already registered */
      if (IPV4_ADDR_SAME(&nbr_static->addr, &addr))
	{
	  if (nbr_static->v_poll != poll_interval)
	    {
	      nbr_static->v_poll = poll_interval;

	      if (nbr_static->t_poll)
		{
		  OSPF_POLL_TIMER_OFF (nbr_static->t_poll);
		  OSPF_POLL_TIMER_ON (nbr_static->t_poll, ospf_poll_timer,
				      nbr_static->v_poll);
		}
	    }

	  if (nbr_static->priority != priority)
	    nbr_static->priority = priority;

	  return CMD_SUCCESS;
	}
    }

  p.family = AF_INET;
  p.prefixlen = IPV4_MAX_BITLEN;
  p.u.prefix4.s_addr = addr.s_addr;

  /* make new static neighbor object */
  nbr_static = XMALLOC (MTYPE_OSPF_NEIGHBOR_STATIC,
			sizeof (struct ospf_nbr_static));
  bzero (nbr_static, sizeof (struct ospf_nbr_static));

  nbr_static->addr = addr;
  nbr_static->oi = NULL;
  nbr_static->neighbor = NULL;
  nbr_static->priority = priority;
  nbr_static->v_poll = poll_interval;
  nbr_static->t_poll = NULL;

  listnode_add_sort (ospf_top->nbr_static, nbr_static);

  for (node = listhead (ospf_top->oiflist); node; nextnode (node))
    {
      struct ospf_interface *oi = getdata (node);

      assert(oi);

      if (oi->type != OSPF_IFTYPE_NBMA)
	continue;

      if (!prefix_match(oi->address, &p))
	continue;

      ospf_nbr_static_add (nbr_static, oi);

      return CMD_SUCCESS;
    }

  return CMD_SUCCESS;
}

DEFUN (neighbor_priority_pollinterval,
       neighbor_priority_pollinterval_cmd,
       "neighbor A.B.C.D priority <0-255> poll-interval <1-65535>",
       NEIGHBOR_STR
       "Neighbor IP address\n"
       "Neighbor Priority\n"
       "Priority\n"
       "Dead Neighbor Polling interval\n"
       "Seconds\n")
{
  return ospf_nbr_static_new (argv[0], atoi(argv[1]), atoi(argv[2]), vty);
}

DEFUN (neighbor_priority,
       neighbor_priority_cmd,
       "neighbor A.B.C.D priority <0-255>",
       NEIGHBOR_STR
       "Neighbor IP address\n"
       "Neighbor Priority\n"
       "Seconds\n")
{
  return ospf_nbr_static_new (argv[0], atoi(argv[1]),
			      OSPF_POLL_INTERVAL_DEFAULT, vty);
}

DEFUN (neighbor_pollinterval,
       neighbor_pollinterval_cmd,
       "neighbor A.B.C.D poll-interval <1-65535>",
       NEIGHBOR_STR
       "Neighbor IP address\n"
       "Dead Neighbor Polling interval\n"
       "Seconds\n")
{
  return ospf_nbr_static_new (argv[0], OSPF_NEIGHBOR_PRIORITY_DEFAULT,
			      atoi(argv[1]), vty);
}

DEFUN (neighbor,
       neighbor_cmd,
       "neighbor A.B.C.D",
       NEIGHBOR_STR
       "Neighbor IP address\n")
{
  return ospf_nbr_static_new (argv[0], OSPF_NEIGHBOR_PRIORITY_DEFAULT,
			      OSPF_POLL_INTERVAL_DEFAULT, vty);
}


DEFUN (no_neighbor,
       no_neighbor_cmd,
       "no neighbor A.B.C.D",
       NO_STR
       NEIGHBOR_STR
       "Neighbor IP address\n")
{
  int ret;
  listnode node;
  struct ospf_nbr_static *nbr_static = NULL;
  struct in_addr addr;

  ret = inet_aton(argv[0], &addr);
  if (!ret)
    {
      vty_out (vty, "Please specify Neighbor address by A.B.C.D%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  for (node = listhead (ospf_top->nbr_static); node; nextnode (node))
    {
      nbr_static = getdata(node);

      if (IPV4_ADDR_SAME(&nbr_static->addr, &addr))
	break;
    }

  if (node == NULL) 
    {
      vty_out (vty, "There is no such Neighbor address %s%s",
	       inet_ntoa(addr), VTY_NEWLINE);
      return CMD_WARNING;
    }

  list_delete_node (ospf_top->nbr_static, node);

  OSPF_POLL_TIMER_OFF (nbr_static->t_poll);

  if (nbr_static->neighbor)
    {
      nbr_static->neighbor->nbr_static = NULL;
      OSPF_NSM_EVENT_SCHEDULE (nbr_static->neighbor, NSM_KillNbr);

      nbr_static->neighbor = NULL;
    }

  if (nbr_static->oi)
    {
      listnode_delete (nbr_static->oi->nbr_static, nbr_static);
      nbr_static->oi = NULL;
    }

  XFREE (MTYPE_OSPF_NEIGHBOR_STATIC, nbr_static);

  return CMD_SUCCESS;
}

ALIAS (no_neighbor,
       no_neighbor_priority_cmd,
       "no neighbor A.B.C.D priority <0-255>",
       NO_STR
       NEIGHBOR_STR
       "Neighbor IP address\n"
       "Neighbor Priority\n"
       "Priority\n")

ALIAS (no_neighbor,
       no_neighbor_pollinterval_cmd,
       "no neighbor A.B.C.D poll-interval <1-65535>",
       NO_STR
       NEIGHBOR_STR
       "Neighbor IP address\n"
       "Dead Neighbor Polling interval\n"
       "Seconds\n")

ALIAS (no_neighbor,
       no_neighbor_priority_pollinterval_cmd,
       "no neighbor A.B.C.D priority <0-255> poll-interval <1-65535>",
       NO_STR
       NEIGHBOR_STR
       "Neighbor IP address\n"
       "Neighbor Priority\n"
       "Priority\n"
       "Dead Neighbor Polling interval\n"
       "Seconds\n")

DEFUN (clear_ip_ospf_neighbor,
       clear_ip_ospf_neighbor_cmd,
       "clear ip ospf neighbor A.B.C.D",
       "Reset functions\n"
       "IP\n"
       "Clear OSPF\n"
       "Neighbor list\n"
       "Neighbor ID\n")
{
  listnode node;
  struct ospf_neighbor *nbr;
  struct in_addr router_id;
  int ret;

  ret = inet_aton (argv[0], &router_id);
  if (!ret)
    {
      vty_out (vty, "Please specify Neighbor ID by A.B.C.D%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  for (node = listhead (ospf_top->oiflist); node; nextnode (node))
    {
      struct ospf_interface *oi = getdata (node);

      nbr = ospf_nbr_lookup_by_routerid (oi->nbrs, &router_id);

      if (nbr)
	{
	  OSPF_NSM_EVENT_SCHEDULE (nbr, NSM_SeqNumberMismatch);
	  vty_out (vty, "clear neighbor %s%s", argv[0], VTY_NEWLINE);
	  break;
	}
    }

  return CMD_SUCCESS;
}

DEFUN (refresh_timer, refresh_timer_cmd,
       "refresh timer <10-1800>",
       "Adjust refresh parameters\n"
       "Set refresh timer\n"
       "Timer value in seconds\n")
{
  int interval = (atoi (argv[0])/10)*10;
  int time_left;
  
  if (ospf_top->lsa_refresh_interval == interval)
    return CMD_SUCCESS;

  time_left = ospf_top->lsa_refresh_interval -
    (time (NULL) - ospf_top->lsa_refresher_started);
  
  if (time_left > interval)
    {
      OSPF_TIMER_OFF (ospf_top->t_lsa_refresher);
      ospf_top->t_lsa_refresher =
	thread_add_timer (master, ospf_lsa_refresh_walker, ospf_top,
			  interval);
    }
  ospf_top->lsa_refresh_interval = interval;  
  return CMD_SUCCESS;
}

DEFUN (no_refresh_timer, no_refresh_timer_val_cmd,
       "no refresh timer <10-1800>",
       "Adjust refresh parameters\n"
       "Unset refresh timer\n"
       "Timer value in seconds\n")
{
  int interval;
  int time_left;

  if (argc == 1)
    {
      interval = (atoi (argv[0])/10)*10;
  
      if (ospf_top->lsa_refresh_interval != interval ||
	  interval == OSPF_LSA_REFRESH_INTERVAL_DEFAULT)
	return CMD_SUCCESS;
    }

  time_left = ospf_top->lsa_refresh_interval -
    (time (NULL) - ospf_top->lsa_refresher_started);

  if (time_left > OSPF_LSA_REFRESH_INTERVAL_DEFAULT)
    {
      OSPF_TIMER_OFF (ospf_top->t_lsa_refresher);
      ospf_top->t_lsa_refresher =
	thread_add_timer (master, ospf_lsa_refresh_walker, ospf_top,
			  OSPF_LSA_REFRESH_INTERVAL_DEFAULT);
    }

  ospf_top->lsa_refresh_interval = OSPF_LSA_REFRESH_INTERVAL_DEFAULT;
  
  return CMD_SUCCESS;
}

ALIAS (no_refresh_timer, no_refresh_timer_cmd,
       "no refresh timer",
       "Adjust refresh parameters\n"
       "Unset refresh timer\n")

DEFUN (auto_cost_reference_bandwidth,
       auto_cost_reference_bandwidth_cmd,
       "auto-cost reference-bandwidth <1-4294967>",
       "Calculate OSPF interface cost according to bandwidth\n"
       "Use reference bandwidth method to assign OSPF cost\n"
       "The reference bandwidth in terms of Mbits per second\n")
{
  u_int32_t refbw;
  listnode node;

  refbw = strtol (argv[0], NULL, 10);
  if (refbw < 1 || refbw > 4294967)
    {
      vty_out (vty, "reference-bandwidth value is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* If reference bandwidth is changed. */
  if ((refbw * 1000) == ospf_top->ref_bandwidth)
    return CMD_SUCCESS;
  
  ospf_top->ref_bandwidth = refbw * 1000;
  vty_out (vty, "%% OSPF: Reference bandwidth is changed.%s", VTY_NEWLINE);
  vty_out (vty, "        Please ensure reference bandwidth is consistent across all routers%s", VTY_NEWLINE);
      
  for (node = listhead (ospf_top->iflist); node; nextnode (node))
      ospf_if_recalculate_output_cost (getdata (node));
  
  return CMD_SUCCESS;
}

DEFUN (no_auto_cost_reference_bandwidth,
       no_auto_cost_reference_bandwidth_cmd,
       "no auto-cost reference-bandwidth",
       NO_STR
       "Calculate OSPF interface cost according to bandwidth\n"
       "Use reference bandwidth method to assign OSPF cost\n")
{
  listnode node;

  if (ospf_top->ref_bandwidth == OSPF_DEFAULT_REF_BANDWIDTH)
    return CMD_SUCCESS;
  
  ospf_top->ref_bandwidth = OSPF_DEFAULT_REF_BANDWIDTH;
  vty_out (vty, "%% OSPF: Reference bandwidth is changed.%s", VTY_NEWLINE);
  vty_out (vty, "        Please ensure reference bandwidth is consistent across all routers%s", VTY_NEWLINE);

  
    for (node = listhead (ospf_top->iflist); node; nextnode (node))
      ospf_if_recalculate_output_cost (getdata (node));
      
  return CMD_SUCCESS;
}

#ifdef HAVE_NSSA
struct in_addr ospf_loop_target; /* = htonl (OSPF_LOOP_TARGET);    injector target */
char stopack='0';
char stopacksnd = '0';

u_int32_t
lsa_seqnum_decrement (struct ospf_lsa *lsa, int decr)
{
  u_int32_t seqnum;

  seqnum = ntohl (lsa->data->ls_seqnum) + decr;

  return htonl (seqnum);
}


DEFUN (stopack_fct, stopack_cmd,
       "stopack <0-1> <0-1>",
       "Stop Acknowledgements RCV SND\n"
       "Declare 0=FALSE 1=TRUE\n"
       "then watch count activity\n")
{

    stopack=argv[0][0];
    stopacksnd=argv[1][0];

  return CMD_SUCCESS;

}


int
ospf_inj_cmd (struct vty *vty, int special, int pos, int ltype)
{		   
  int incr;
  listnode node;
  struct ospf_area *area;
  struct ospf_interface *oi = NULL;
  struct ospf_neighbor *nbr = NULL;
  struct ospf_lsa *lsa = NULL;
  struct ospf_lsa *dup = NULL;
  struct route_node *rn;
  int got1st=0;
  char tbuf[30];

    strncpy(tbuf, inet_ntoa(ospf_loop_target), 29);


  incr = 0;

  if (special==1) incr=1;
  if (special==2) incr=-1;

while(1)
  {
  zlog_info ("Extract CMD= %d, Pos = %d, LSA-Type=%d", special, pos, ltype);

    for (node = listhead (ospf_top->areas); node; nextnode (node))
    {
      area = getdata (node);

      for (rn = route_top (ROUTER_LSDB(area) ); rn; rn = route_next (rn))
	 if ((lsa = rn->info) != NULL)
	 {
	     got1st++; /* 1st -> 1 */
	     if (got1st>=pos)
		break; /* we Got LSA */
	 }

      /*   if (lsa != NULL)
		break;  we got LSA */

      /* else go onto next area */
      
    }
    if (lsa == NULL)
		break; /* after all that we got no LSA's */
    
    zlog_info ("Got LSA-SEQ %x, Extract %d Neighbor from eth0 interface;", lsa->data->ls_seqnum, pos);

  for (node = listhead (ospf_top->oiflist); node; nextnode (node))
  {
    oi = node->data;
    if (oi->ifp->name[0] == 'e') break;
  }


  for (rn = route_top (oi->nbrs); rn; rn = route_next (rn))
    if ((nbr = rn->info))
    {
      zlog_info ("Got rn %x on %s",rn, IF_NAME (oi));
      /* Do not do myself. */
      if (nbr != oi->nbr_self)
	  break;
    }

  if (nbr == NULL)
	    break;  /* send nothing */

    zlog_info ("Send Copy LSA-%d to %s w/SEQNUM+%d",
	    ltype, tbuf, incr);

    dup = ospf_lsa_dup(lsa); /* lock = 1 */

    if (incr != 0) /* bump seq by -1 or + 1 */
	dup->data->ls_seqnum = lsa_seqnum_decrement (dup, incr);

    switch (special)
    {

    case 1:

	zlog_info ("Send Copy LSA-%d to %s w/SEQNUM+%d", ltype, tbuf, incr);
	break;


    case 2:

	zlog_info ("Send Copy LSA-%d to %s w/SEQNUM+%d", ltype, tbuf, incr);
	break;


    case 3:
	dup->data->ls_seqnum = htonl(0x70000001);
	zlog_info ("Send Copy LSA-%d to %s w/SEQNUM = %d", ltype, tbuf, dup->data->ls_seqnum);
	break;

    case 4:
	dup->data->ls_seqnum = htonl(0x8FFFFFFE);
	zlog_info ("Send Copy LSA-%d to %s w/SEQNUM = %d", ltype, tbuf, dup->data->ls_seqnum);
	break;

    case 0:
    default:

	zlog_info ("Default SEND LSA-%d to %s w/SEQNUM+%d", ltype, tbuf, incr);
	break;


    }

    ospf_lsa_checksum (dup->data);

    sh_fct (dup->data->type, vty, dup);

    ospf_ls_upd_send_lsa (nbr, dup, OSPF_SEND_PACKET_LOOP);

   /* ospf_lsa_discard(dup); */

    break;

  }


  return CMD_SUCCESS;


}


DEFUN (set_injection, set_injection_cmd,
       "setinject  A.B.C.D",
       "Target Ip for Injected Data onto line\n"
       "Declare IP  <target ip>\n"
       "then inj abc.\n")
{
int ret;
struct in_addr addr;


  ret = inet_aton (argv[0], &addr);
  if (!ret)
    {
      vty_out (vty, "Please specify Injector address by A.B.C.D%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }
    else
    {
     ospf_loop_target.s_addr = addr.s_addr;

     vty_out (vty, "Injector address set to %s %s",
	      inet_ntoa(ospf_loop_target) , VTY_NEWLINE);

    }
  return CMD_SUCCESS;

}

DEFUN (data_injection, data_injection_cmd,
       "inject-data <1-900>",
       "Inject Data onto line\n"
       "Declare number <1-900>\n"
       "then watch logs.\n")
{
  char gotch =  argv[0][0];
  int source;
  listnode node;
  struct ospf_area *area;
  struct ospf_interface *oi = NULL;
  struct ospf_neighbor *nbr = NULL;
  struct ospf_lsa *lsa = NULL;
  struct ospf_lsa *dup = NULL;
  struct route_node *rn;
  int got1st=0;
  int gotnum, gotspec, gotpos, gotlsa;
  
  gotlsa=(int) (gotch-0x30);
  
  gotspec=0;
  gotpos=0;
  
  gotnum = atoi(argv[0]);
  
  if (ospf_loop_target.s_addr == 0)
    ospf_loop_target.s_addr = htonl (OSPF_LOOP_TARGET);
  


  if (gotnum>9)
    {
      
      gotpos = (int) (argv[0][1] - 0x30);
      
      if (gotnum>99)
	{
	  gotspec= (int) (argv[0][0] - 0x30);
	  gotlsa= (int) (argv[0][2] - 0x30);
	}
      
      return ( ospf_inj_cmd (vty, gotspec, gotpos, gotlsa) );
      
    }
  
  else
    
    switch (gotch)
      {

      case '0':

	assert (0); /* test crash message */
	break;

  case '5': /* send Type-5 LSA */

    zlog_info ("Send Copy LSA-5 to Target");
    dup = ospf_external_lsa_test (5);

 /* lock = 1 */

    if (dup == NULL)
	  {
		zlog_info ("New LSA-5 Incomplete");
		break; /* we have no LSA */
	  }

    sh_fct (dup->data->type, vty, dup);
    zlog_info ("Got LSA %x, Extract 1st Neighbor from eth0 interface;", lsa);

    for (node = listhead (ospf_top->oiflist); node; nextnode (node))
	{
	oi = node->data;
	if (oi->ifp->name[0] == 'e') break;
	}

    for (rn = route_top (oi->nbrs); rn; rn = route_next (rn))
	if ((nbr = rn->info))
	    {
		zlog_info ("Got rn %x on %s",rn, oi->ifp->name);
		/* Do not do myself. */
		if (nbr != oi->nbr_self)
		    break;
	    }

      if (nbr == NULL)
	    break;

    ospf_ls_upd_send_lsa (nbr, dup, OSPF_SEND_PACKET_LOOP);

    ospf_lsa_discard(dup); 

    break;

  case '7': /* send Type-7 LSA */

    zlog_info ("Send Copy LSA-7 to Target");
    dup = ospf_external_lsa_test (7);

 /* lock = 1 */

    if (dup == NULL)
	  {
		zlog_info ("New LSA-7 Incomplete");
		break; /* we have no LSA */
	  }

    sh_fct (dup->data->type, vty, dup);
    zlog_info ("Got LSA %x, Extract 1st Neighbor from eth0 interface;", lsa);

    for (node = listhead (ospf_top->oiflist); node; nextnode (node))
	{
	oi = node->data;
	if (oi->ifp->name[0] == 'e') break;
	}

    for (rn = route_top (oi->nbrs); rn; rn = route_next (rn))
	if ((nbr = rn->info))
	    {
		zlog_info ("Got rn %x on %s",rn, oi->ifp->name);
		/* Do not do myself. */
		if (nbr != oi->nbr_self)
		    break;
	    }

      if (nbr == NULL)
	    break;

    dup->data->type = OSPF_AS_NSSA_LSA;

    ospf_ls_upd_send_lsa (nbr, dup, OSPF_SEND_PACKET_LOOP);

    ospf_lsa_discard(dup); 

    break;


  case '1':  /* send Type-1 LSA */


    zlog_info ("Extract 1st Type-1 LSA: Start");



  for (node = listhead (ospf_top->areas); node; nextnode (node))
    {
      area = getdata (node);

      for (rn = route_top (ROUTER_LSDB(area) ); rn; rn = route_next (rn))
	 if ((lsa = rn->info) != NULL)
		break; /* we Got LSA */

      if (lsa != NULL)
		break; /* we got LSA */

      /* else go onto next area */
      
    }
    if (lsa == NULL)
		break; /* after all that we got no LSA's */

   
    
    zlog_info ("Got LSA-SEQ %x, Extract 1st Neighbor from eth0 interface;", lsa->data->ls_seqnum);
 
  /*  show_as_external_lsa_detail (vty, dup); */
	    sh_fct (lsa->data->type, vty, lsa);


  for (node = listhead (ospf_top->oiflist); node; nextnode (node))
  {
    oi = node->data;
    if (oi->ifp->name[0] == 'e') break;
  }

  for (rn = route_top (oi->nbrs); rn; rn = route_next (rn))
    if ((nbr = rn->info))
    {
      zlog_info ("Got rn %x on %s",rn, oi->ifp->name);
      /* Do not do myself. */
      if (nbr != oi->nbr_self)
	  break;
    }

      if (nbr == NULL)
	    break;

    zlog_info ("Send Copy LSA-1 to Target");

    /* dup = ospf_lsa_dup(lsa);  lock = 1 */

     /*dup->data->type = OSPF_AS_NSSA_LSA; */

    ospf_ls_upd_send_lsa (nbr, lsa, OSPF_SEND_PACKET_LOOP);

   /* ospf_lsa_discard(dup); */

    break;

  case '8':  /* send Type-1 LSA */


    zlog_info ("Extract 2nd Type-1 LSA: Start");



  for (node = listhead (ospf_top->areas); node; nextnode (node))
    {
      area = getdata (node);

      for (rn = route_top (ROUTER_LSDB(area) ); rn; rn = route_next (rn))
	 if ((lsa = rn->info) != NULL)
	 {
	     got1st++; /* 1st -> 1 */
	     if (got1st>=2)
		break; /* we Got LSA */
	 }

   /*   if (lsa != NULL)
		break;  we got LSA */

      /* else go onto next area */
      
    }
    if (lsa == NULL)
		break; /* after all that we got no LSA's */

   
    
    zlog_info ("Got LSA-SEQ %x, Extract 2nd Neighbor from eth0 interface;", lsa->data->ls_seqnum);
 
  /*  show_as_external_lsa_detail (vty, dup); */
  /*	    sh_fct (lsa->data->type, vty, lsa); */


  for (node = listhead (ospf_top->oiflist); node; nextnode (node))
  {
    oi = node->data;
    if (oi->ifp->name[0] == 'e') break;
  }


  for (rn = route_top (oi->nbrs); rn; rn = route_next (rn))
    if ((nbr = rn->info))
    {
      zlog_info ("Got rn %x on %s",rn, oi->ifp->name);
      /* Do not do myself. */
      if (nbr != oi->nbr_self)
	  break;
    }

      if (nbr == NULL)
	    break;

    zlog_info ("Send Copy LSA-1 to Target w/SEQNUM+1");

     dup = ospf_lsa_dup(lsa); /* lock = 1 */

    dup->data->ls_seqnum = lsa_seqnum_increment (lsa);

    ospf_lsa_checksum (dup->data);

	    sh_fct (dup->data->type, vty, dup);

    ospf_ls_upd_send_lsa (nbr, dup, OSPF_SEND_PACKET_LOOP);

   /* ospf_lsa_discard(dup); */

    break;

  case '9':
/*	Future ospf_force_NSSA_ external (); */

    zlog_info ("Send Type-9 Not Implemented Yet");

    break;

  /* Get distribute source. */
  if (!str2distribute_source ("static", &source))
    return CMD_WARNING;

   zlog_info ("5-Force External Self-Origination \n");
   ospf_distribute_list_out_set (vty, source, "eth0");

	break;
  default:
      zlog_info ("No such Injection \n");
      break;
  }

  return CMD_SUCCESS;
}
#endif /* HAVE NSSA */

char *ospf_abr_type_str[] = 
{
  "unknown",
  "standard",
  "ibm",
  "cisco",
  "shortcut"
};

char *ospf_shortcut_mode_str[] = 
{
  "default",
  "enable",
  "disable"
};


void
area_id2str (char *buf, int length, struct ospf_area *area)
{
  bzero (buf, length);

  if (area->format == OSPF_AREA_ID_FORMAT_ADDRESS)
    strncpy (buf, inet_ntoa (area->area_id), length);
  else
    sprintf (buf, "%lu", (unsigned long) ntohl (area->area_id.s_addr));
}

int
config_write_network_area (struct vty *vty)
{
  struct route_node *rn;
  u_char buf[INET_ADDRSTRLEN];

  /* `network area' print. */
  for (rn = route_top (ospf_top->networks); rn; rn = route_next (rn))
    if (rn->info)
      {
	struct ospf_network *n = rn->info;

	bzero (buf, INET_ADDRSTRLEN);

	/* Create Area ID string by specified Area ID format. */
	if (n->format == OSPF_AREA_ID_FORMAT_ADDRESS)
	  strncpy (buf, inet_ntoa (n->area_id), INET_ADDRSTRLEN);
	else
	  sprintf (buf, "%lu", 
		   (unsigned long int) ntohl (n->area_id.s_addr));

	/* Network print. */
	vty_out (vty, " network %s/%d area %s%s",
		 inet_ntoa (rn->p.u.prefix4), rn->p.prefixlen,
		 buf, VTY_NEWLINE);
      }

  return 0;
}

int
config_write_ospf_area (struct vty *vty)
{
  listnode node;
  u_char buf[INET_ADDRSTRLEN];

  /* Area configuration print. */
  for (node = listhead (ospf_top->areas); node; nextnode (node))
    {
      struct ospf_area *area = getdata (node);
      struct route_node *rn1;

      area_id2str (buf, INET_ADDRSTRLEN, area);

      if (area->auth_type != OSPF_AUTH_NULL)
	{
	  if (area->auth_type == OSPF_AUTH_SIMPLE)
	    vty_out (vty, " area %s authentication%s", buf, VTY_NEWLINE);
	  else
	    vty_out (vty, " area %s authentication message-digest%s",
		     buf, VTY_NEWLINE);
	}

      if (area->shortcut_configured != OSPF_SHORTCUT_DEFAULT)
	vty_out (vty, " area %s shortcut %s%s", buf,
		 ospf_shortcut_mode_str[area->shortcut_configured],
		 VTY_NEWLINE);

      if (
	     (area->external_routing == OSPF_AREA_STUB)
#ifdef HAVE_NSSA
		|| (area->external_routing == OSPF_AREA_NSSA)
#endif /* HAVE_NSSA */
	 )
		{

#ifdef HAVE_NSSA
      if (area->external_routing == OSPF_AREA_NSSA)
	  vty_out (vty, " area %s nssa", buf);
      else
#endif /* HAVE_NSSA */
	  vty_out (vty, " area %s stub", buf);

	  if (area->no_summary)
	    vty_out (vty, " no-summary");

	  vty_out (vty, "%s", VTY_NEWLINE);

	  if (area->default_cost != 1)
	    vty_out (vty, " area %s default-cost %d%s", buf, 
		     area->default_cost, VTY_NEWLINE);
		}

      for (rn1 = route_top (area->ranges); rn1; rn1 = route_next (rn1))
	if (rn1->info)
	  {
	    struct ospf_area_range *range = rn1->info;

	    vty_out (vty, " area %s range %s/%d", buf,
		     inet_ntoa (rn1->p.u.prefix4), rn1->p.prefixlen);

	    if (CHECK_FLAG (range->flags, OSPF_RANGE_SUPPRESS))
	      vty_out (vty, " not-advertise");

	    if (CHECK_FLAG (range->flags, OSPF_RANGE_SUBST))
	      vty_out (vty, " substitute %s/%d",
		       inet_ntoa (range->substitute.prefix), 
		       range->substitute.prefixlen);

	    vty_out (vty, "%s", VTY_NEWLINE);
	  }

      if (EXPORT_NAME (area))
	vty_out (vty, " area %s export-list %s%s", buf,
		 EXPORT_NAME (area), VTY_NEWLINE);

      if (IMPORT_NAME (area))
	vty_out (vty, " area %s import-list %s%s", buf,
		 IMPORT_NAME (area), VTY_NEWLINE);
    }

  return 0;
}

int
config_write_ospf_nbr_static (struct vty *vty)
{
  listnode node;
  struct ospf_nbr_static *nbr;

  /* Static Neighbor configuration print. */
  for (node = listhead (ospf_top->nbr_static); node; nextnode (node))
    {
      nbr = getdata(node);

      vty_out (vty, " neighbor %s", inet_ntoa(nbr->addr));

      if (nbr->priority != OSPF_NEIGHBOR_PRIORITY_DEFAULT)
	vty_out (vty, " priority %d", nbr->priority);

      if (nbr->v_poll != OSPF_POLL_INTERVAL_DEFAULT)
	vty_out (vty, " poll-interval %d", nbr->v_poll);

      vty_out (vty, "%s", VTY_NEWLINE);
    }

  return 0;
}

int
config_write_virtual_link (struct vty *vty)
{
  listnode node;
  u_char buf[INET_ADDRSTRLEN];

  /* Virtual-Link print */
  for (node = listhead (ospf_top->vlinks); node; nextnode (node))
    {
      listnode n2;
      struct crypt_key *ck;
      struct ospf_vl_data *vl_data = getdata (node);
      struct ospf_interface *oi;

      if (vl_data != NULL)
	{
	  bzero (buf, INET_ADDRSTRLEN);
	  
	  if (vl_data->format == OSPF_AREA_ID_FORMAT_ADDRESS)
	    strncpy (buf, inet_ntoa (vl_data->vl_area_id), INET_ADDRSTRLEN);
	  else
	    sprintf (buf, "%lu", 
		     (unsigned long int) ntohl (vl_data->vl_area_id.s_addr));
	  oi = vl_data->vl_oi;

	  /* timers */
	  if (OSPF_IF_PARAM (oi, v_hello) != OSPF_HELLO_INTERVAL_DEFAULT ||
	      OSPF_IF_PARAM (oi, v_wait) != OSPF_ROUTER_DEAD_INTERVAL_DEFAULT ||
	      OSPF_IF_PARAM (oi, retransmit_interval) != OSPF_RETRANSMIT_INTERVAL_DEFAULT ||
	      OSPF_IF_PARAM (oi, transmit_delay) != OSPF_TRANSMIT_DELAY_DEFAULT)
	    vty_out (vty, " area %s virtual-link %s hello-interval %d retransmit-interval %d transmit-delay %d dead-interval %d%s",
		     buf,
		     inet_ntoa (vl_data->vl_peer), 
		     OSPF_IF_PARAM (oi, v_hello),
		     OSPF_IF_PARAM (oi, retransmit_interval),
		     OSPF_IF_PARAM (oi, transmit_delay),
		     OSPF_IF_PARAM (oi, v_wait),
		     VTY_NEWLINE);
	  else
	    vty_out (vty, " area %s virtual-link %s%s", buf,
		     inet_ntoa (vl_data->vl_peer), VTY_NEWLINE);
	  /* Auth key */
	  if (IF_DEF_PARAMS (vl_data->vl_oi->ifp)->auth_simple[0] != '\0')
	    vty_out (vty, " area %s virtual-link %s authentication-key %s%s",
		     buf,
		     inet_ntoa (vl_data->vl_peer),
		     IF_DEF_PARAMS (vl_data->vl_oi->ifp)->auth_simple,
		     VTY_NEWLINE);
	  /* md5 keys */
	  for (n2 = listhead (IF_DEF_PARAMS (vl_data->vl_oi->ifp)->auth_crypt); n2; nextnode (n2))
	    {
	      ck = getdata (n2);
	      vty_out (vty, " area %s virtual-link %s message-digest-key %d md5 %s%s",
		       buf,
		       inet_ntoa (vl_data->vl_peer),
		       ck->key_id, ck->auth_key, VTY_NEWLINE);
	    }
	 
	}
    }

  return 0;
}

/* OSPF configuration write function. */
int
ospf_config_write (struct vty *vty)
{
  listnode node;
  int write = 0;

  if (ospf_top != NULL)
    {
      /* `router ospf' print. */
      vty_out (vty, "router ospf%s", VTY_NEWLINE);

      write++;

      if (!ospf_top->networks)
        return write;

      /* Router ID print. */
      if (ospf_top->router_id_static.s_addr != 0)
        vty_out (vty, " ospf router-id %s%s",
                 inet_ntoa (ospf_top->router_id_static), VTY_NEWLINE);

      /* ABR type print. */
      if (ospf_top->abr_type != OSPF_ABR_STAND)
        vty_out (vty, " ospf abr-type %s%s", 
                 ospf_abr_type_str[ospf_top->abr_type], VTY_NEWLINE);

      /* RFC1583 compatibility flag print. */
      /* Compatible with CISCO 12.1. */
      if (ospf_top->RFC1583Compat)
	vty_out (vty, " compatible rfc1583%s", VTY_NEWLINE);
      /*
      if (ospf_top->RFC1583Compat)
	vty_out (vty, " ospf rfc1583compatibility%s", VTY_NEWLINE);
      */

      /* auto-cost reference-bandwidth configuration.  */
      if (ospf_top->ref_bandwidth != OSPF_DEFAULT_REF_BANDWIDTH)
	vty_out (vty, " auto-cost reference-bandwidth %d%s",
		 ospf_top->ref_bandwidth / 1000, VTY_NEWLINE);

      /* SPF timers print. */
      if (ospf_top->spf_delay != OSPF_SPF_DELAY_DEFAULT ||
	  ospf_top->spf_holdtime != OSPF_SPF_HOLDTIME_DEFAULT)
	vty_out (vty, " timers spf %d %d%s",
		 ospf_top->spf_delay, ospf_top->spf_holdtime, VTY_NEWLINE);

      /* SPF refresh parameters print. */
      if (ospf_top->lsa_refresh_interval != OSPF_LSA_REFRESH_INTERVAL_DEFAULT)
	vty_out (vty, " refresh timer %d%s",
		 ospf_top->lsa_refresh_interval, VTY_NEWLINE);

      /* Redistribute information print. */
      config_write_ospf_redistribute (vty);

      /* passive-interface print. */
      for (node = listhead (ospf_top->iflist); node; nextnode (node))
        {
          struct interface *ifp = getdata (node);

	  if (!ifp)
	    continue;
	  if (IF_DEF_PARAMS (ifp)->passive_interface == OSPF_IF_PASSIVE)
	    vty_out (vty, " passive-interface %s%s",
		     ifp->name, VTY_NEWLINE);
        }

      for (node = listhead (ospf_top->oiflist); node; nextnode (node))
        {
          struct ospf_interface *oi = getdata (node);

	  if (OSPF_IF_PARAM_CONFIGURED (oi->params, passive_interface) &&
	      oi->params->passive_interface == OSPF_IF_PASSIVE)
	    vty_out (vty, " passive-interface %s%s",
		     inet_ntoa (oi->address->u.prefix4), VTY_NEWLINE);
        }

      
      /* Network area print. */
      config_write_network_area (vty);

      /* Area config print. */
      config_write_ospf_area (vty);

      /* static neighbor print. */
      config_write_ospf_nbr_static (vty);

      /* Virtual-Link print. */
      config_write_virtual_link (vty);

      /* Default metric configuration.  */
      config_write_ospf_default_metric (vty);

      /* Distribute-list and default-information print. */
      config_write_ospf_distribute (vty);

      /* Distance configuration. */
      config_write_ospf_distance (vty);
    }

  return write;
}

struct cmd_node ospf_node =
{
  OSPF_NODE,
  "%s(config-router)# ",
  1
};

/* Install OSPF related commands. */
void
ospf_init ()
{


  /* Install ospf top node. */
  install_node (&ospf_node, ospf_config_write);

  /* Install ospf commands. */
  install_element (VIEW_NODE, &show_ip_ospf_interface_cmd);
  install_element (VIEW_NODE, &show_ip_ospf_neighbor_int_detail_cmd);
  install_element (VIEW_NODE, &show_ip_ospf_neighbor_int_cmd);
  install_element (VIEW_NODE, &show_ip_ospf_neighbor_id_cmd);
  install_element (VIEW_NODE, &show_ip_ospf_neighbor_detail_all_cmd);
  install_element (VIEW_NODE, &show_ip_ospf_neighbor_detail_cmd);
  install_element (VIEW_NODE, &show_ip_ospf_neighbor_cmd);
  install_element (VIEW_NODE, &show_ip_ospf_neighbor_all_cmd);
  install_element (ENABLE_NODE, &show_ip_ospf_interface_cmd);
  install_element (ENABLE_NODE, &show_ip_ospf_neighbor_int_detail_cmd);
  install_element (ENABLE_NODE, &show_ip_ospf_neighbor_int_cmd);
  install_element (ENABLE_NODE, &show_ip_ospf_neighbor_id_cmd);
  install_element (ENABLE_NODE, &show_ip_ospf_neighbor_detail_all_cmd);
  install_element (ENABLE_NODE, &show_ip_ospf_neighbor_detail_cmd);
  install_element (ENABLE_NODE, &show_ip_ospf_neighbor_cmd);
  install_element (ENABLE_NODE, &show_ip_ospf_neighbor_all_cmd);
#if 0
  install_element (ENABLE_NODE, &clear_ip_ospf_neighbor_cmd);
#endif
  install_element (CONFIG_NODE, &router_ospf_cmd);
  install_element (CONFIG_NODE, &no_router_ospf_cmd);

  install_default (OSPF_NODE);
  install_element (OSPF_NODE, &ospf_router_id_cmd);
  install_element (OSPF_NODE, &no_ospf_router_id_cmd);
  install_element (OSPF_NODE, &router_id_cmd);
  install_element (OSPF_NODE, &no_router_id_cmd);
  install_element (OSPF_NODE, &passive_interface_addr_cmd);
  install_element (OSPF_NODE, &passive_interface_cmd);
  install_element (OSPF_NODE, &no_passive_interface_addr_cmd);
  install_element (OSPF_NODE, &no_passive_interface_cmd);

  install_element (OSPF_NODE, &ospf_abr_type_cmd);
  install_element (OSPF_NODE, &no_ospf_abr_type_cmd);
  install_element (OSPF_NODE, &ospf_rfc1583_flag_cmd);
  install_element (OSPF_NODE, &no_ospf_rfc1583_flag_cmd);
  install_element (OSPF_NODE, &ospf_compatible_rfc1583_cmd);
  install_element (OSPF_NODE, &no_ospf_compatible_rfc1583_cmd);

  /*  install_element (OSPF_NODE, &network_area_decimal_cmd); */
  install_element (OSPF_NODE, &network_area_cmd);
  install_element (OSPF_NODE, &no_network_area_decimal_cmd);
  install_element (OSPF_NODE, &no_network_area_cmd);

  install_element (OSPF_NODE, &area_authentication_message_digest_decimal_cmd);
  install_element (OSPF_NODE, &area_authentication_message_digest_cmd);

  install_element (OSPF_NODE, &area_authentication_decimal_cmd);
  install_element (OSPF_NODE, &area_authentication_cmd);
  install_element (OSPF_NODE, &no_area_authentication_decimal_cmd);
  install_element (OSPF_NODE, &no_area_authentication_cmd);

  /* OSPF area range commands.  */
  install_element (OSPF_NODE, &area_range_cmd);
  install_element (OSPF_NODE, &area_range_decimal_cmd);
  install_element (OSPF_NODE, &area_range_advertise_cmd);
  install_element (OSPF_NODE, &area_range_advertise_decimal_cmd);
  install_element (OSPF_NODE, &area_range_notadvertise_cmd);
  install_element (OSPF_NODE, &area_range_notadvertise_decimal_cmd);
  install_element (OSPF_NODE, &no_area_range_cmd);
  install_element (OSPF_NODE, &no_area_range_decimal_cmd);
  install_element (OSPF_NODE, &no_area_range_advertise_cmd);
  install_element (OSPF_NODE, &no_area_range_advertise_decimal_cmd);
  install_element (OSPF_NODE, &no_area_range_notadvertise_cmd);
  install_element (OSPF_NODE, &no_area_range_notadvertise_decimal_cmd);

#ifdef HAVE_NSSA
  install_element (ENABLE_NODE, &data_injection_cmd);
  install_element (ENABLE_NODE, &set_injection_cmd);
  install_element (ENABLE_NODE, &stopack_cmd);
#endif /* HAVE_NSSA */

  install_element (OSPF_NODE, &area_range_subst_cmd);
  install_element (OSPF_NODE, &no_area_range_subst_cmd);

#if OLD_VLINK_CMD  /* Old Vlink commands */
  install_element (OSPF_NODE, &area_vlink_auth_decimal_cmd);
  install_element (OSPF_NODE, &area_vlink_auth_cmd);
  install_element (OSPF_NODE, &area_vlink_md5_decimal_cmd);
  install_element (OSPF_NODE, &area_vlink_md5_cmd);
  install_element (OSPF_NODE, &area_vlink_param_md5_decimal_cmd);
  install_element (OSPF_NODE, &area_vlink_param_md5_cmd);
  install_element (OSPF_NODE, &area_vlink_param_auth_decimal_cmd);
  install_element (OSPF_NODE, &area_vlink_param_auth_cmd);
  install_element (OSPF_NODE, &area_vlink_param_decimal_cmd);
  install_element (OSPF_NODE, &area_vlink_param_cmd);
  install_element (OSPF_NODE, &area_vlink_decimal_cmd);
  install_element (OSPF_NODE, &area_vlink_cmd);
  install_element (OSPF_NODE, &no_area_vlink_decimal_cmd);
  install_element (OSPF_NODE, &no_area_vlink_cmd);
#else /* OLD_VLINK_CMD */
  install_element (OSPF_NODE, &area_vlink_decimal_cmd);
  install_element (OSPF_NODE, &area_vlink_cmd);
  install_element (OSPF_NODE, &no_area_vlink_decimal_cmd);
  install_element (OSPF_NODE, &no_area_vlink_cmd);

  install_element (OSPF_NODE, &area_vlink_param1_decimal_cmd);
  install_element (OSPF_NODE, &area_vlink_param1_cmd);
  install_element (OSPF_NODE, &no_area_vlink_param1_decimal_cmd);
  install_element (OSPF_NODE, &no_area_vlink_param1_cmd);

  install_element (OSPF_NODE, &area_vlink_param2_decimal_cmd);
  install_element (OSPF_NODE, &area_vlink_param2_cmd);
  install_element (OSPF_NODE, &no_area_vlink_param2_decimal_cmd);
  install_element (OSPF_NODE, &no_area_vlink_param2_cmd);

  install_element (OSPF_NODE, &area_vlink_param3_decimal_cmd);
  install_element (OSPF_NODE, &area_vlink_param3_cmd);
  install_element (OSPF_NODE, &no_area_vlink_param3_decimal_cmd);
  install_element (OSPF_NODE, &no_area_vlink_param3_cmd);

  install_element (OSPF_NODE, &area_vlink_param4_decimal_cmd);
  install_element (OSPF_NODE, &area_vlink_param4_cmd);
  install_element (OSPF_NODE, &no_area_vlink_param4_decimal_cmd);
  install_element (OSPF_NODE, &no_area_vlink_param4_cmd);

  install_element (OSPF_NODE, &area_vlink_authtype_args_decimal_cmd);
  install_element (OSPF_NODE, &area_vlink_authtype_args_cmd);
  install_element (OSPF_NODE, &area_vlink_authtype_decimal_cmd);
  install_element (OSPF_NODE, &area_vlink_authtype_cmd);
  install_element (OSPF_NODE, &no_area_vlink_authtype_decimal_cmd);
  install_element (OSPF_NODE, &no_area_vlink_authtype_cmd);

  install_element (OSPF_NODE, &area_vlink_md5_decimal_cmd);
  install_element (OSPF_NODE, &area_vlink_md5_cmd);
  install_element (OSPF_NODE, &no_area_vlink_md5_decimal_cmd);
  install_element (OSPF_NODE, &no_area_vlink_md5_cmd);

  install_element (OSPF_NODE, &area_vlink_authkey_decimal_cmd);
  install_element (OSPF_NODE, &area_vlink_authkey_cmd);
  install_element (OSPF_NODE, &no_area_vlink_authkey_decimal_cmd);
  install_element (OSPF_NODE, &no_area_vlink_authkey_cmd);

  install_element (OSPF_NODE, &area_vlink_authtype_args_authkey_decimal_cmd);
  install_element (OSPF_NODE, &area_vlink_authtype_args_authkey_cmd);
  install_element (OSPF_NODE, &area_vlink_authtype_authkey_decimal_cmd);
  install_element (OSPF_NODE, &area_vlink_authtype_authkey_cmd);
  install_element (OSPF_NODE, &no_area_vlink_authtype_authkey_decimal_cmd);
  install_element (OSPF_NODE, &no_area_vlink_authtype_authkey_cmd);

  install_element (OSPF_NODE, &area_vlink_authtype_args_md5_decimal_cmd);
  install_element (OSPF_NODE, &area_vlink_authtype_args_md5_cmd);
  install_element (OSPF_NODE, &area_vlink_authtype_md5_decimal_cmd);
  install_element (OSPF_NODE, &area_vlink_authtype_md5_cmd);
  install_element (OSPF_NODE, &no_area_vlink_authtype_md5_decimal_cmd);
  install_element (OSPF_NODE, &no_area_vlink_authtype_md5_cmd);

#endif /* OLD_VLINK_CMD */

  install_element (OSPF_NODE, &area_stub_nosum_cmd);
  install_element (OSPF_NODE, &area_stub_nosum_decimal_cmd);
  install_element (OSPF_NODE, &area_stub_cmd);
  install_element (OSPF_NODE, &area_stub_decimal_cmd);
  install_element (OSPF_NODE, &no_area_stub_nosum_cmd);
  install_element (OSPF_NODE, &no_area_stub_nosum_decimal_cmd);
  install_element (OSPF_NODE, &no_area_stub_cmd);
  install_element (OSPF_NODE, &no_area_stub_decimal_cmd);

#ifdef HAVE_NSSA
  install_element (OSPF_NODE, &area_nssa_nosum_cmd);
  install_element (OSPF_NODE, &area_nssa_nosum_decimal_cmd);
  install_element (OSPF_NODE, &area_nssa_cmd);
  install_element (OSPF_NODE, &area_nssa_decimal_cmd);

  install_element (OSPF_NODE, &area_nssa_nosum_t_never_cmd);
  install_element (OSPF_NODE, &area_nssa_nosum_t_never_decimal_cmd);
  install_element (OSPF_NODE, &area_nssa_t_never_cmd);
  install_element (OSPF_NODE, &area_nssa_t_never_decimal_cmd);

  install_element (OSPF_NODE, &area_nssa_nosum_t_candidate_cmd);
  install_element (OSPF_NODE, &area_nssa_nosum_t_candidate_decimal_cmd);
  install_element (OSPF_NODE, &area_nssa_t_candidate_cmd);
  install_element (OSPF_NODE, &area_nssa_t_candidate_decimal_cmd);

  install_element (OSPF_NODE, &area_nssa_nosum_t_always_cmd);
  install_element (OSPF_NODE, &area_nssa_nosum_t_always_decimal_cmd);
  install_element (OSPF_NODE, &area_nssa_t_always_cmd);
  install_element (OSPF_NODE, &area_nssa_t_always_decimal_cmd);

  install_element (OSPF_NODE, &no_area_nssa_nosum_cmd);
  install_element (OSPF_NODE, &no_area_nssa_nosum_decimal_cmd);
  install_element (OSPF_NODE, &no_area_nssa_cmd);
  install_element (OSPF_NODE, &no_area_nssa_decimal_cmd);
#endif /* HAVE_NSSA */

  install_element (OSPF_NODE, &area_default_cost_cmd);
  install_element (OSPF_NODE, &area_default_cost_decimal_cmd);
  install_element (OSPF_NODE, &no_area_default_cost_cmd);
  install_element (OSPF_NODE, &no_area_default_cost_decimal_cmd);

  install_element (OSPF_NODE, &area_shortcut_decimal_cmd);
  install_element (OSPF_NODE, &area_shortcut_cmd);
  install_element (OSPF_NODE, &no_area_shortcut_decimal_cmd);
  install_element (OSPF_NODE, &no_area_shortcut_cmd);

  install_element (OSPF_NODE, &area_export_list_cmd);
  install_element (OSPF_NODE, &area_export_list_decimal_cmd);
  install_element (OSPF_NODE, &no_area_export_list_cmd);
  install_element (OSPF_NODE, &no_area_export_list_decimal_cmd);

  install_element (OSPF_NODE, &area_import_list_cmd);
  install_element (OSPF_NODE, &area_import_list_decimal_cmd);
  install_element (OSPF_NODE, &no_area_import_list_cmd);
  install_element (OSPF_NODE, &no_area_import_list_decimal_cmd);

  install_element (OSPF_NODE, &timers_spf_cmd);
  install_element (OSPF_NODE, &no_timers_spf_cmd);

  install_element (OSPF_NODE, &refresh_timer_cmd);
  install_element (OSPF_NODE, &no_refresh_timer_val_cmd);
  install_element (OSPF_NODE, &no_refresh_timer_cmd);
  
  install_element (OSPF_NODE, &auto_cost_reference_bandwidth_cmd);
  install_element (OSPF_NODE, &no_auto_cost_reference_bandwidth_cmd);

  install_element (OSPF_NODE, &neighbor_cmd);
  install_element (OSPF_NODE, &no_neighbor_cmd);

  install_element (OSPF_NODE, &neighbor_priority_cmd);
  install_element (OSPF_NODE, &no_neighbor_priority_cmd);

  install_element (OSPF_NODE, &neighbor_pollinterval_cmd);
  install_element (OSPF_NODE, &no_neighbor_pollinterval_cmd);

  install_element (OSPF_NODE, &neighbor_priority_pollinterval_cmd);
  install_element (OSPF_NODE, &no_neighbor_priority_pollinterval_cmd);

  install_element (VIEW_NODE, &show_ip_ospf_cmd);
  install_element (ENABLE_NODE, &show_ip_ospf_cmd);

  /* Make empty list of ospf list. */
  ospf_top = NULL;

  zebra_init ();
}

