/*
 * Zebra connect library for OSPFd
 * Copyright (C) 1997, 98, 99, 2000 Kunihiro Ishiguro, Toshiaki Takada
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

#include <zebra.h>

#include "thread.h"
#include "command.h"
#include "network.h"
#include "prefix.h"
#include "routemap.h"
#include "table.h"
#include "stream.h"
#include "memory.h"
#include "zclient.h"
#include "filter.h"
#include "log.h"

#include "ospfd/ospfd.h"
#include "ospfd/ospf_interface.h"
#include "ospfd/ospf_ism.h"
#include "ospfd/ospf_asbr.h"
#include "ospfd/ospf_asbr.h"
#include "ospfd/ospf_abr.h"
#include "ospfd/ospf_lsa.h"
#include "ospfd/ospf_dump.h"
#include "ospfd/ospf_route.h"
#include "ospfd/ospf_zebra.h"
#ifdef HAVE_SNMP
#include "ospfd/ospf_snmp.h"
#endif /* HAVE_SNMP */

/* Zebra structure to hold current status. */
struct zclient *zclient = NULL;

/* For registering threads. */
extern struct thread_master *master;

/* Inteface addition message from zebra. */
int
ospf_interface_add (int command, struct zclient *zclient, zebra_size_t length)
{
  struct interface *ifp;

  ifp = zebra_interface_add_read (zclient->ibuf);

  if (IS_DEBUG_OSPF (zebra, ZEBRA_INTERFACE))
    zlog_info ("Zebra: interface add %s index %d flags %ld metric %d mtu %d",
	       ifp->name, ifp->ifindex, ifp->flags, ifp->metric, ifp->mtu);

  if (!OSPF_IF_PARAM_CONFIGURED (IF_DEF_PARAMS (ifp), type))
    {
      SET_IF_PARAM (IF_DEF_PARAMS (ifp), type);
      IF_DEF_PARAMS (ifp)->type = OSPF_IFTYPE_BROADCAST;
      
      if (if_is_broadcast (ifp))
	IF_DEF_PARAMS (ifp)->type = OSPF_IFTYPE_BROADCAST;
      else if (if_is_pointopoint (ifp))
	IF_DEF_PARAMS (ifp)->type = OSPF_IFTYPE_POINTOPOINT;
      else if (if_is_loopback (ifp))
	IF_DEF_PARAMS (ifp)->type = OSPF_IFTYPE_LOOPBACK;
    }

  ospf_if_update ();

#ifdef HAVE_SNMP
  ospf_snmp_if_update (ifp);
#endif /* HAVE_SNMP */

  return 0;
}

int
ospf_interface_delete (int command, struct zclient *zclient,
		       zebra_size_t length)
{
  struct interface *ifp;
  struct stream *s;
  struct route_node *rn;

  s = zclient->ibuf;  
  /* zebra_interface_state_read() updates interface structure in iflist */
  ifp = zebra_interface_state_read (s);

  if (ifp == NULL)
    return 0;

  if (if_is_up (ifp))
    zlog_warn ("Zebra: got delete of %s, but interface is still up",
	       ifp->name);
  
  if (IS_DEBUG_OSPF (zebra, ZEBRA_INTERFACE))
    zlog_info ("Zebra: interface delete %s index %d flags %ld metric %d mtu %d",
	       ifp->name, ifp->ifindex, ifp->flags, ifp->metric, ifp->mtu);  

#ifdef HAVE_SNMP
  ospf_snmp_if_delete (ifp);
#endif /* HAVE_SNMP */

  for (rn = route_top (IF_OIFS (ifp)); rn; rn = route_next (rn))
    if (rn->info)
      ospf_if_free ((struct ospf_interface *) rn->info);

  for (rn = route_top (IF_OIFS_PARAMS (ifp)); rn; rn = route_next (rn))
    if (rn->info)
      ospf_del_if_params (rn->info);
  
  if_delete (ifp);

  return 0;
}

struct interface *
zebra_interface_if_lookup (struct stream *s)
{
  struct interface *ifp;
  u_char ifname_tmp[INTERFACE_NAMSIZ];

  /* Read interface name. */
  stream_get (ifname_tmp, s, INTERFACE_NAMSIZ);

  /* Lookup this by interface index. */
  ifp = if_lookup_by_name (ifname_tmp);

  /* If such interface does not exist, indicate an error */
  if (!ifp)
    return NULL;

  return ifp;
}

void
zebra_interface_if_set_value (struct stream *s, struct interface *ifp)
{
  /* Read interface's index. */
  ifp->ifindex = stream_getl (s);

  /* Read interface's value. */
  ifp->flags = stream_getl (s);
  ifp->metric = stream_getl (s);
  ifp->mtu = stream_getl (s);
  ifp->bandwidth = stream_getl (s);
}

int
ospf_interface_state_up (int command, struct zclient *zclient,
			 zebra_size_t length)
{
  struct interface *ifp;
  struct interface if_tmp;
  struct ospf_interface *oi;
  struct route_node *rn;
  
  ifp = zebra_interface_if_lookup (zclient->ibuf);

  if (ifp == NULL)
    return 0;

  /* Interface is already up. */
  if (if_is_up (ifp))
    {
      /* Temporarily keep ifp values. */
      memcpy (&if_tmp, ifp, sizeof (struct interface));

      zebra_interface_if_set_value (zclient->ibuf, ifp);

      if (IS_DEBUG_OSPF (zebra, ZEBRA_INTERFACE))
	zlog_info ("Zebra: Interface[%s] state update.", ifp->name);

      if (if_tmp.bandwidth != ifp->bandwidth)
	{
	  if (IS_DEBUG_OSPF (zebra, ZEBRA_INTERFACE))
	    zlog_info ("Zebra: Interface[%s] bandwidth change %d -> %d.",
		       ifp->name, if_tmp.bandwidth, ifp->bandwidth);

	  ospf_if_recalculate_output_cost (ifp);
	}
      return 0;
    }
  
  zebra_interface_if_set_value (zclient->ibuf, ifp);
  
  if (IS_DEBUG_OSPF (zebra, ZEBRA_INTERFACE))
    zlog_info ("Zebra: Interface[%s] state change to up.", ifp->name);
  
  for (rn = route_top (IF_OIFS (ifp));rn; rn = route_next (rn))
    {
      if ( (oi = rn->info) == NULL)
	continue;
      
      ospf_if_up (oi);
    }
  
  return 0;
}

int
ospf_interface_state_down (int command, struct zclient *zclient,
			   zebra_size_t length)
{
  struct interface *ifp;
  struct ospf_interface *oi;
  struct route_node *node;

  ifp = zebra_interface_state_read (zclient->ibuf);

  if (ifp == NULL)
    return 0;

  if (IS_DEBUG_OSPF (zebra, ZEBRA_INTERFACE))
    zlog_info ("Zebra: Interface[%s] state change to down.", ifp->name);

  for (node = route_top (IF_OIFS (ifp));node; node = route_next (node))
    {
      if ( (oi = node->info) == NULL)
	continue;
      ospf_if_down (oi);
    }

  return 0;
}

int
ospf_interface_address_add (int command, struct zclient *zclient,
			    zebra_size_t length)
{
  struct connected *c;

  c = zebra_interface_address_add_read (zclient->ibuf);

  if (c == NULL)
    return 0;

#if 0
  if (IS_DEBUG_OSPF (zebra, ZEBRA_INTERFACE))
    {
      struct prefix *p;

      p = c->address;
      if (p->family == AF_INET)
	zlog_info (" connected address %s/%d", 
		   inet_atop (p->u.prefix4), p->prefixlen);
    }
#endif

  ospf_if_update ();

#ifdef HAVE_SNMP
  ospf_snmp_if_update (c->ifp);
#endif /* HAVE_SNMP */

  return 0;
}

int
ospf_interface_address_delete (int command, struct zclient *zclient,
			       zebra_size_t length)
{
  struct connected *c;
  struct interface *ifp;
  struct ospf_interface *oi;
  struct route_node *node;

  c = zebra_interface_address_delete_read (zclient->ibuf);

  if (c == NULL)
    return 0;

  ifp = c->ifp;
  node = route_node_lookup (IF_OIFS (ifp), c->address);

  if (!node)
    return 0;

  assert (node->info);
  oi = node->info;
  
  /* Call interface hook functions to clean up */
  ospf_if_free (oi);
  
#ifdef HAVE_SNMP
  ospf_snmp_if_update (c->ifp);
#endif /* HAVE_SNMP */

  connected_free (c);

  ospf_if_update();

  return 0;
}

void
ospf_zebra_add (struct prefix_ipv4 *p, struct ospf_route *or)
{
  u_char message;
  u_char distance;
  u_char flags;
  int psize;
  struct stream *s;
  struct ospf_path *path;
  listnode node;

  if (zclient->redist[ZEBRA_ROUTE_OSPF])
    {
      message = 0;
      flags = 0;

      /* OSPF pass nexthop and metric */
      SET_FLAG (message, ZAPI_MESSAGE_NEXTHOP);
      SET_FLAG (message, ZAPI_MESSAGE_METRIC);

      /* Distance value. */
      distance = ospf_distance_apply (p, or);
      if (distance)
	SET_FLAG (message, ZAPI_MESSAGE_DISTANCE);

      /* Make packet. */
      s = zclient->obuf;
      stream_reset (s);

      /* Length place holder. */
      stream_putw (s, 0);

      /* Put command, type, flags, message. */
      stream_putc (s, ZEBRA_IPV4_ROUTE_ADD);
      stream_putc (s, ZEBRA_ROUTE_OSPF);
      stream_putc (s, flags);
      stream_putc (s, message);
  
      /* Put prefix information. */
      psize = PSIZE (p->prefixlen);
      stream_putc (s, p->prefixlen);
      stream_write (s, (u_char *)&p->prefix, psize);

      /* Nexthop count. */
      stream_putc (s, or->path->count);

      /* Nexthop, ifindex, distance and metric information. */
      for (node = listhead (or->path); node; nextnode (node))
	{
	  path = getdata (node);

	  if (path->nexthop.s_addr != INADDR_ANY)
	    {
	      stream_putc (s, ZEBRA_NEXTHOP_IPV4);
	      stream_put_in_addr (s, &path->nexthop);
	    }
	  else
	    {
	      stream_putc (s, ZEBRA_NEXTHOP_IFINDEX);
	      if (path->oi)
		stream_putl (s, path->oi->ifp->ifindex);
	      else
		stream_putl (s, 0);
	    }
	}

      if (CHECK_FLAG (message, ZAPI_MESSAGE_DISTANCE))
	stream_putc (s, distance);
      if (CHECK_FLAG (message, ZAPI_MESSAGE_METRIC))
	stream_putl (s, or->cost);

      stream_putw_at (s, 0, stream_get_endp (s));

      writen (zclient->sock, s->data, stream_get_endp (s));

#if 0
      if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
	{
	  char *nexthop_str;

	  nexthop_str = strdup (inet_ntoa (*nexthop));
	  zlog_info ("Zebra: Route add %s/%d nexthop %s metric %d",
		     inet_ntoa (p->prefix), p->prefixlen, nexthop_str,
		     metric);
	  free (nexthop_str);
	}
#endif /* 0 */
    }
}

void
ospf_zebra_delete (struct prefix_ipv4 *p, struct ospf_route *or)
{
  struct zapi_ipv4 api;

  if (zclient->redist[ZEBRA_ROUTE_OSPF])
    {
      api.type = ZEBRA_ROUTE_OSPF;
      api.flags = 0;
      api.message = 0;
      /* Do not send nexthop information when delete route.
      SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
      api.nexthop_num = 1;
      api.nexthop = &nexthop;
      api.ifindex_num = 0;
      */
      /* Do not pass metric when delete route. 
	SET_FLAG (api.message, ZAPI_MESSAGE_METRIC);
	api.metric = metric;
      */
      zapi_ipv4_delete (zclient, p, &api);

#if 0
      if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
	{
	  char *nexthop_str;

	  nexthop_str = strdup (inet_ntoa (*nexthop));
	  zlog_info ("Zebra: Route delete %s/%d nexthop %s",
		     inet_ntoa (p->prefix), p->prefixlen, nexthop_str);
	  free (nexthop_str);
	}
#endif /* 0 */
    }
}

void
ospf_zebra_add_discard (struct prefix_ipv4 *p)
{
  struct in_addr lo_addr;
  struct in_addr *nexthop;
  struct zapi_ipv4 api;

  lo_addr.s_addr = htonl (INADDR_LOOPBACK);
  nexthop = &lo_addr;

  if (zclient->redist[ZEBRA_ROUTE_OSPF])
    {
      api.type = ZEBRA_ROUTE_OSPF;
      api.flags = ZEBRA_FLAG_BLACKHOLE;
      api.message = 0;
      SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
      api.nexthop_num = 1;
      api.nexthop = &nexthop;
      api.ifindex_num = 0;

      zapi_ipv4_add (zclient, p, &api);
    }
}

void
ospf_zebra_delete_discard (struct prefix_ipv4 *p)
{
  struct in_addr lo_addr;
  struct in_addr *nexthop;
  struct zapi_ipv4 api;

  lo_addr.s_addr = htonl (INADDR_LOOPBACK);
  nexthop = &lo_addr;

  if (zclient->redist[ZEBRA_ROUTE_OSPF])
    {
      api.type = ZEBRA_ROUTE_OSPF;
      api.flags = ZEBRA_FLAG_BLACKHOLE;
      api.message = 0;
      SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
      api.nexthop_num = 1;
      api.nexthop = &nexthop;
      api.ifindex_num = 0;

      zapi_ipv4_delete (zclient, p, &api);
    }
}

int
ospf_is_type_redistributed (int type)
{
  return (DEFAULT_ROUTE_TYPE (type)) ?
    zclient->default_information : zclient->redist[type];
}

int
ospf_redistribute_set (int type, int mtype, int mvalue)
{
  int force = 0;
  
  if (ospf_is_type_redistributed (type))
    {
      if (mtype != ospf_top->dmetric[type].type)
	{
	  ospf_top->dmetric[type].type = mtype;
	  force = LSA_REFRESH_FORCE;
	}
      if (mvalue != ospf_top->dmetric[type].value)
	{
	  ospf_top->dmetric[type].value = mvalue;
	  force = LSA_REFRESH_FORCE;
	}
	  
      ospf_external_lsa_refresh_type (type, force);
      
      if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
	zlog_info ("Redistribute[%s]: Refresh  Type[%d], Metric[%d]",
		   LOOKUP (ospf_redistributed_proto, type),
		   metric_type (type), metric_value (type));
      
      return CMD_SUCCESS;
    }

  ospf_top->dmetric[type].type = mtype;
  ospf_top->dmetric[type].value = mvalue;

  zclient_redistribute_set (zclient, type);

  if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
    zlog_info ("Redistribute[%s]: Start  Type[%d], Metric[%d]",
	       LOOKUP (ospf_redistributed_proto, type),
	       metric_type (type), metric_value (type));
  
  ospf_asbr_status_update (++ospf_top->redistribute);

  return CMD_SUCCESS;
}

int
ospf_redistribute_unset (int type)
{
  if (!ospf_is_type_redistributed (type))
    return CMD_SUCCESS;

  zclient_redistribute_unset (zclient, type);
  
  if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
    zlog_info ("Redistribute[%s]: Stop",
	       LOOKUP (ospf_redistributed_proto, type));

  ospf_top->dmetric[type].type = -1;
  ospf_top->dmetric[type].value = -1;

  /* Remove the routes from OSPF table. */
  ospf_redistribute_withdraw (type);

  ospf_asbr_status_update (--ospf_top->redistribute);

  return CMD_SUCCESS;
}

int
ospf_redistribute_default_set (int originate, int mtype, int mvalue)
{
  int force = 0;
  if (ospf_is_type_redistributed (DEFAULT_ROUTE))
    {
      if (mtype != ospf_top->dmetric[DEFAULT_ROUTE].type)
	{
	  ospf_top->dmetric[DEFAULT_ROUTE].type = mtype;
	  force = 1;
	}
      if (mvalue != ospf_top->dmetric[DEFAULT_ROUTE].value)
	{
	  force = 1;
	  ospf_top->dmetric[DEFAULT_ROUTE].value = mvalue;
	}
      
      ospf_external_lsa_refresh_default ();
      
      if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
	zlog_info ("Redistribute[%s]: Refresh  Type[%d], Metric[%d]",
		   LOOKUP (ospf_redistributed_proto, DEFAULT_ROUTE),
		   metric_type (DEFAULT_ROUTE),
		   metric_value (DEFAULT_ROUTE));
      return CMD_SUCCESS;
    }

  ospf_top->default_originate = originate;
  ospf_top->dmetric[DEFAULT_ROUTE].type = mtype;
  ospf_top->dmetric[DEFAULT_ROUTE].value = mvalue;

  zclient_redistribute_default_set (zclient);
  
  if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
    zlog_info ("Redistribute[DEFAULT]: Start  Type[%d], Metric[%d]",
	       metric_type (DEFAULT_ROUTE), metric_value (DEFAULT_ROUTE));


  if (ospf_top->router_id.s_addr == 0)
    ospf_top->external_origin |= (1 << DEFAULT_ROUTE);
  else
    thread_add_timer (master, ospf_default_originate_timer,
		      &ospf_top->default_originate, 1);

  ospf_asbr_status_update (++ospf_top->redistribute);

  return CMD_SUCCESS;
}

int
ospf_redistribute_default_unset ()
{
  if (!ospf_is_type_redistributed (DEFAULT_ROUTE))
    return CMD_SUCCESS;

  ospf_top->default_originate = DEFAULT_ORIGINATE_NONE;
  ospf_top->dmetric[DEFAULT_ROUTE].type = -1;
  ospf_top->dmetric[DEFAULT_ROUTE].value = -1;

  zclient_redistribute_default_unset (zclient);

  if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
    zlog_info ("Redistribute[DEFAULT]: Stop");
  
  ospf_asbr_status_update (--ospf_top->redistribute);

  return CMD_SUCCESS;
}

int
ospf_external_lsa_originate_check (struct external_info *ei)
{
  /* If prefix is multicast, then do not originate LSA. */
  if (IN_MULTICAST (htonl (ei->p.prefix.s_addr)))
    {
      zlog_info ("LSA[Type5:%s]: Not originate AS-external-LSA, "
		 "Prefix belongs multicast", inet_ntoa (ei->p.prefix));
      return 0;
    }

  /* Take care of default-originate. */
  if (is_prefix_default (&ei->p))
    if (ospf_top->default_originate == DEFAULT_ORIGINATE_NONE)
      {
	zlog_info ("LSA[Type5:0.0.0.0]: Not originate AS-exntenal-LSA "
		   "for default");
	return 0;
      }

  return 1;
}

/* If connected prefix is OSPF enable interface, then do not announce. */
int
ospf_distribute_check_connected (struct external_info *ei)
{
  struct route_node *rn;

  for (rn = route_top (ospf_top->networks); rn; rn = route_next (rn))
    if (rn->info != NULL)
      if (prefix_match (&rn->p, (struct prefix *)&ei->p))
	return 0;

  return 1;
}

/* return 1 if external LSA must be originated, 0 otherwise */
int
ospf_redistribute_check (struct external_info *ei, int *changed)
{
  struct route_map_set_values save_values;
  struct prefix_ipv4 *p = &ei->p;
  u_char type = is_prefix_default (&ei->p) ? DEFAULT_ROUTE : ei->type;
  
  if (changed)
    *changed = 0;

  if (!ospf_external_lsa_originate_check (ei))
    return 0;

  /* Take care connected route. */
  if (type == ZEBRA_ROUTE_CONNECT && !ospf_distribute_check_connected (ei))
    return 0;

  if (!DEFAULT_ROUTE_TYPE (type) && DISTRIBUTE_NAME (type))
    /* distirbute-list exists, but access-list may not? */
    if (DISTRIBUTE_LIST (type))
      if (access_list_apply (DISTRIBUTE_LIST (type), p) == FILTER_DENY)
	{
	  if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
	    zlog_info ("Redistribute[%s]: %s/%d filtered by ditribute-list.",
		       LOOKUP (ospf_redistributed_proto, type),
		       inet_ntoa (p->prefix), p->prefixlen);
	  return 0;
	}

  save_values = ei->route_map_set;
  ospf_reset_route_map_set_values (&ei->route_map_set);
  
  /* apply route-map if needed */
  if (ROUTEMAP_NAME (type))
    {
      int ret;

      ret = route_map_apply (ROUTEMAP (type), (struct prefix *)p,
			     RMAP_OSPF, ei);

      if (ret == RMAP_DENYMATCH)
	{
	  ei->route_map_set = save_values;
	  if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
	    zlog_info ("Redistribute[%s]: %s/%d filtered by route-map.",
		       LOOKUP (ospf_redistributed_proto, type),
		       inet_ntoa (p->prefix), p->prefixlen);
	  return 0;
	}
      
      /* check if 'route-map set' changed something */
      if (changed)
	*changed = !ospf_route_map_set_compare (&ei->route_map_set,
						&save_values);
    }

  return 1;
}

/* OSPF route-map set for redistribution */
void
ospf_routemap_set (int type, char *name)
{
  if (ROUTEMAP_NAME (type))
    free (ROUTEMAP_NAME (type));

  ROUTEMAP_NAME (type) = strdup (name);
  ROUTEMAP (type) = route_map_lookup_by_name (name);
}

void
ospf_routemap_unset (int type)
{
  if (ROUTEMAP_NAME (type))
    free (ROUTEMAP_NAME (type));

  ROUTEMAP_NAME (type) = NULL;
  ROUTEMAP (type) = NULL;
}

/* Zebra route add and delete treatment. */
int
ospf_zebra_read_ipv4 (int command, struct zclient *zclient,
		      zebra_size_t length)
{
  struct stream *s;
  struct zapi_ipv4 api;
  unsigned long ifindex;
  struct in_addr nexthop;
  struct prefix_ipv4 p;
  struct external_info *ei;

  s = zclient->ibuf;
  ifindex = 0;
  nexthop.s_addr = 0;

  /* Type, flags, message. */
  api.type = stream_getc (s);
  api.flags = stream_getc (s);
  api.message = stream_getc (s);

  /* IPv4 prefix. */
  memset (&p, 0, sizeof (struct prefix_ipv4));
  p.family = AF_INET;
  p.prefixlen = stream_getc (s);
  stream_get (&p.prefix, s, PSIZE (p.prefixlen));

  /* Nexthop, ifindex, distance, metric. */
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP))
    {
      api.nexthop_num = stream_getc (s);
      nexthop.s_addr = stream_get_ipv4 (s);
    }
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_IFINDEX))
    {
      api.ifindex_num = stream_getc (s);
      ifindex = stream_getl (s);
    }
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_DISTANCE))
    api.distance = stream_getc (s);
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_METRIC))
    api.metric = stream_getl (s);

  if (command == ZEBRA_IPV4_ROUTE_ADD)
    {
      ei = ospf_external_info_add (api.type, p, ifindex, nexthop);

      if (ospf_top->router_id.s_addr == 0)
	/* Set flags to generate AS-external-LSA originate event
	   for each redistributed protocols later. */
	ospf_top->external_origin |= (1 << api.type);
      else
	{
	  if (ei)
	    {
	      if (is_prefix_default (&p))
		ospf_external_lsa_refresh_default ();
	      else
		{
		  struct ospf_lsa *current;

		  current = ospf_external_info_find_lsa (&ei->p);
		  if (!current)
		    ospf_external_lsa_originate (ei);
		  else if (IS_LSA_MAXAGE (current))
		    ospf_external_lsa_refresh (current, ei, LSA_REFRESH_FORCE);
		  else
		    zlog_warn ("ospf_zebra_read_ipv4() : %s already exists",
			       inet_ntoa (p.prefix));
		}
	    }
	}
    }
  else /* if (command == ZEBRA_IPV4_ROUTE_DELETE) */
    {
      ospf_external_info_delete (api.type, p);
      if ( !is_prefix_default (&p))
	ospf_external_lsa_flush (api.type, &p, ifindex, nexthop);
      else
	ospf_external_lsa_refresh_default ();
    }

  return 0;
}


int
str2distribute_source (char *str, int *source)
{
  /* Sanity check. */
  if (str == NULL)
    return 0;

  if (strncmp (str, "k", 1) == 0)
    *source = ZEBRA_ROUTE_KERNEL;
  else if (strncmp (str, "c", 1) == 0)
    *source = ZEBRA_ROUTE_CONNECT;
  else if (strncmp (str, "s", 1) == 0)
    *source = ZEBRA_ROUTE_STATIC;
  else if (strncmp (str, "r", 1) == 0)
    *source = ZEBRA_ROUTE_RIP;
  else if (strncmp (str, "b", 1) == 0)
    *source = ZEBRA_ROUTE_BGP;
  else
    return 0;

  return 1;
}

int
str2metric (char *str, int *metric)
{
  /* Sanity check. */
  if (str == NULL)
    return 0;

  *metric = strtol (str, NULL, 10);
  if (*metric < 0 && *metric > 16777214)
    {
      /* vty_out (vty, "OSPF metric value is invalid%s", VTY_NEWLINE); */
      return 0;
    }

  return 1;
}

int
str2metric_type (char *str, int *metric_type)
{
  /* Sanity check. */
  if (str == NULL)
    return 0;

  if (strncmp (str, "1", 1) == 0)
    *metric_type = EXTERNAL_METRIC_TYPE_1;
  else if (strncmp (str, "2", 1) == 0)
    *metric_type = EXTERNAL_METRIC_TYPE_2;
  else
    return 0;

  return 1;
}

DEFUN (ospf_redistribute_source_metric_type,
       ospf_redistribute_source_metric_type_routemap_cmd,
       "redistribute (kernel|connected|static|rip|bgp) metric <0-16777214> metric-type (1|2) route-map WORD",
       "Redistribute information from another routing protocol\n"
       "Kernel routes\n"
       "Connected\n"
       "Static routes\n"
       "Routing Information Protocol (RIP)\n"
       "Border Gateway Protocol (BGP)\n"
       "Metric for redistributed routes\n"
       "OSPF default metric\n"
       "OSPF exterior metric type for redistributed routes\n"
       "Set OSPF External Type 1 metrics\n"
       "Set OSPF External Type 2 metrics\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  int source;
  int type = -1;
  int metric = -1;

  /* Get distribute source. */
  if (!str2distribute_source (argv[0], &source))
    return CMD_WARNING;

  /* Get metric value. */
  if (argc >= 2)
    if (!str2metric (argv[1], &metric))
      return CMD_WARNING;

  /* Get metric type. */
  if (argc >= 3)
    if (!str2metric_type (argv[2], &type))
      return CMD_WARNING;

  if (argc == 4)
    ospf_routemap_set (source, argv[3]);
  else
    ospf_routemap_unset (source);
  
  return ospf_redistribute_set (source, type, metric);
}

ALIAS (ospf_redistribute_source_metric_type,
       ospf_redistribute_source_metric_type_cmd,
       "redistribute (kernel|connected|static|rip|bgp) metric <0-16777214> metric-type (1|2)",
       "Redistribute information from another routing protocol\n"
       "Kernel routes\n"
       "Connected\n"
       "Static routes\n"
       "Routing Information Protocol (RIP)\n"
       "Border Gateway Protocol (BGP)\n"
       "Metric for redistributed routes\n"
       "OSPF default metric\n"
       "OSPF exterior metric type for redistributed routes\n"
       "Set OSPF External Type 1 metrics\n"
       "Set OSPF External Type 2 metrics\n")

ALIAS (ospf_redistribute_source_metric_type,
       ospf_redistribute_source_metric_cmd,
       "redistribute (kernel|connected|static|rip|bgp) metric <0-16777214>",
       "Redistribute information from another routing protocol\n"
       "Kernel routes\n"
       "Connected\n"
       "Static routes\n"
       "Routing Information Protocol (RIP)\n"
       "Border Gateway Protocol (BGP)\n"
       "Metric for redistributed routes\n"
       "OSPF default metric\n")

DEFUN (ospf_redistribute_source_type_metric,
       ospf_redistribute_source_type_metric_routemap_cmd,
       "redistribute (kernel|connected|static|rip|bgp) metric-type (1|2) metric <0-16777214> route-map WORD",
       "Redistribute information from another routing protocol\n"
       "Kernel routes\n"
       "Connected\n"
       "Static routes\n"
       "Routing Information Protocol (RIP)\n"
       "Border Gateway Protocol (BGP)\n"
       "OSPF exterior metric type for redistributed routes\n"
       "Set OSPF External Type 1 metrics\n"
       "Set OSPF External Type 2 metrics\n"
       "Metric for redistributed routes\n"
       "OSPF default metric\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  int source;
  int type = -1;
  int metric = -1;

  /* Get distribute source. */
  if (!str2distribute_source (argv[0], &source))
    return CMD_WARNING;

  /* Get metric value. */
  if (argc >= 2)
    if (!str2metric_type (argv[1], &type))
      return CMD_WARNING;

  /* Get metric type. */
  if (argc >= 3)
    if (!str2metric (argv[2], &metric))
      return CMD_WARNING;

  if (argc == 4)
    ospf_routemap_set (source, argv[3]);
  else
    ospf_routemap_unset (source);

  return ospf_redistribute_set (source, type, metric);
}

ALIAS (ospf_redistribute_source_type_metric,
       ospf_redistribute_source_type_metric_cmd,
       "redistribute (kernel|connected|static|rip|bgp) metric-type (1|2) metric <0-16777214>",
       "Redistribute information from another routing protocol\n"
       "Kernel routes\n"
       "Connected\n"
       "Static routes\n"
       "Routing Information Protocol (RIP)\n"
       "Border Gateway Protocol (BGP)\n"
       "OSPF exterior metric type for redistributed routes\n"
       "Set OSPF External Type 1 metrics\n"
       "Set OSPF External Type 2 metrics\n"
       "Metric for redistributed routes\n"
       "OSPF default metric\n")

ALIAS (ospf_redistribute_source_type_metric,
       ospf_redistribute_source_type_cmd,
       "redistribute (kernel|connected|static|rip|bgp) metric-type (1|2)",
       "Redistribute information from another routing protocol\n"
       "Kernel routes\n"
       "Connected\n"
       "Static routes\n"
       "Routing Information Protocol (RIP)\n"
       "Border Gateway Protocol (BGP)\n"
       "OSPF exterior metric type for redistributed routes\n"
       "Set OSPF External Type 1 metrics\n"
       "Set OSPF External Type 2 metrics\n")

ALIAS (ospf_redistribute_source_type_metric,
       ospf_redistribute_source_cmd,
       "redistribute (kernel|connected|static|rip|bgp)",
       "Redistribute information from another routing protocol\n"
       "Kernel routes\n"
       "Connected\n"
       "Static routes\n"
       "Routing Information Protocol (RIP)\n"
       "Border Gateway Protocol (BGP)\n")

DEFUN (ospf_redistribute_source_metric_routemap,
       ospf_redistribute_source_metric_routemap_cmd,
       "redistribute (kernel|connected|static|rip|bgp) metric <0-16777214> route-map WORD",
       "Redistribute information from another routing protocol\n"
       "Kernel routes\n"
       "Connected\n"
       "Static routes\n"
       "Routing Information Protocol (RIP)\n"
       "Border Gateway Protocol (BGP)\n"
       "Metric for redistributed routes\n"
       "OSPF default metric\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  int source;
  int metric = -1;

  /* Get distribute source. */
  if (!str2distribute_source (argv[0], &source))
    return CMD_WARNING;

  /* Get metric value. */
  if (argc >= 2)
    if (!str2metric (argv[1], &metric))
      return CMD_WARNING;

  if (argc == 3)
    ospf_routemap_set (source, argv[2]);
  else
    ospf_routemap_unset (source);
  
  return ospf_redistribute_set (source, -1, metric);
}

DEFUN (ospf_redistribute_source_type_routemap,
       ospf_redistribute_source_type_routemap_cmd,
       "redistribute (kernel|connected|static|rip|bgp) metric-type (1|2) route-map WORD",
       "Redistribute information from another routing protocol\n"
       "Kernel routes\n"
       "Connected\n"
       "Static routes\n"
       "Routing Information Protocol (RIP)\n"
       "Border Gateway Protocol (BGP)\n"
       "OSPF exterior metric type for redistributed routes\n"
       "Set OSPF External Type 1 metrics\n"
       "Set OSPF External Type 2 metrics\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  int source;
  int type = -1;

  /* Get distribute source. */
  if (!str2distribute_source (argv[0], &source))
    return CMD_WARNING;

  /* Get metric value. */
  if (argc >= 2)
    if (!str2metric_type (argv[1], &type))
      return CMD_WARNING;

  if (argc == 3)
    ospf_routemap_set (source, argv[2]);
  else
    ospf_routemap_unset (source);

  return ospf_redistribute_set (source, type, -1);
}

DEFUN (ospf_redistribute_source_routemap,
       ospf_redistribute_source_routemap_cmd,
       "redistribute (kernel|connected|static|rip|bgp) route-map WORD",
       "Redistribute information from another routing protocol\n"
       "Kernel routes\n"
       "Connected\n"
       "Static routes\n"
       "Routing Information Protocol (RIP)\n"
       "Border Gateway Protocol (BGP)\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  int source;

  /* Get distribute source. */
  if (!str2distribute_source (argv[0], &source))
    return CMD_WARNING;

  if (argc == 2)
    ospf_routemap_set (source, argv[1]);
  else
    ospf_routemap_unset (source);

  return ospf_redistribute_set (source, -1, -1);
}

DEFUN (no_ospf_redistribute_source,
       no_ospf_redistribute_source_cmd,
       "no redistribute (kernel|connected|static|rip|bgp)",
       NO_STR
       "Redistribute information from another routing protocol\n"
       "Kernel routes\n"
       "Connected\n"
       "Static routes\n"
       "Routing Information Protocol (RIP)\n"
       "Border Gateway Protocol (BGP)\n")
{
  int source;

  if (!str2distribute_source (argv[0], &source))
    return CMD_WARNING;

  ospf_routemap_unset (source);
  return ospf_redistribute_unset (source);
}


int
ospf_distribute_list_out_set (struct vty *vty, int type, char *name)
{
  /* Lookup access-list for distribute-list. */
  DISTRIBUTE_LIST (type) = access_list_lookup (AFI_IP, name);

  /* Clear previous distribute-name. */
  if (DISTRIBUTE_NAME (type))
    free (DISTRIBUTE_NAME (type));

  /* Set distribute-name. */
  DISTRIBUTE_NAME (type) = strdup (name);

  /* If access-list have been set, schedule update timer. */
  if (DISTRIBUTE_LIST (type))
    ospf_distribute_list_update (type);

  return CMD_SUCCESS;
}

int
ospf_distribute_list_out_unset (struct vty *vty, int type, char *name)
{
  /* Schedule update timer. */
  if (DISTRIBUTE_LIST (type))
    ospf_distribute_list_update (type);

  /* Unset distribute-list. */
  DISTRIBUTE_LIST (type) = NULL;

  /* Clear distribute-name. */
  if (DISTRIBUTE_NAME (type))
    free (DISTRIBUTE_NAME (type));
  
  DISTRIBUTE_NAME (type) = NULL;

  return CMD_SUCCESS;
}

DEFUN (ospf_distribute_list_out,
       ospf_distribute_list_out_cmd,
       "distribute-list WORD out (kernel|connected|static|rip|bgp)",
       "Filter networks in routing updates\n"
       "Access-list name\n"
       OUT_STR
       "Kernel routes\n"
       "Connected\n"
       "Static routes\n"
       "Routing Information Protocol (RIP)\n"
       "Border Gateway Protocol (BGP)\n")
{
  int source;

  /* Get distribute source. */
  if (!str2distribute_source (argv[1], &source))
    return CMD_WARNING;

  return ospf_distribute_list_out_set (vty, source, argv[0]);
}

DEFUN (no_ospf_distribute_list_out,
       no_ospf_distribute_list_out_cmd,
       "no distribute-list WORD out (kernel|connected|static|rip|bgp)",
       NO_STR
       "Filter networks in routing updates\n"
       "Access-list name\n"
       OUT_STR
       "Kernel routes\n"
       "Connected\n"
       "Static routes\n"
       "Routing Information Protocol (RIP)\n"
       "Border Gateway Protocol (BGP)\n")
{
  int source;

  if (!str2distribute_source (argv[1], &source))
    return CMD_WARNING;

  return ospf_distribute_list_out_unset (vty, source, argv[0]);
}

/* distribute-list update timer. */
int
ospf_distribute_list_update_timer (struct thread *thread)
{
  struct route_node *rn;
  struct external_info *ei;
  struct route_table *rt;
  struct ospf_lsa *lsa;
  u_char type;

  type = (int) THREAD_ARG (thread);
  rt = EXTERNAL_INFO (type);

  ospf_top->t_distribute_update = NULL;

  zlog_info ("Zebra[Redistribute]: distribute-list update timer fired!");

  /* foreach all external info. */
  if (rt)
    for (rn = route_top (rt); rn; rn = route_next (rn))
      if ((ei = rn->info) != NULL)
	{
	  if (is_prefix_default (&ei->p))
	    ospf_external_lsa_refresh_default ();
	  else if ((lsa = ospf_external_info_find_lsa (&ei->p)))
	    ospf_external_lsa_refresh (lsa, ei, LSA_REFRESH_IF_CHANGED);
	  else
	    ospf_external_lsa_originate (ei);
	}
  return 0;
}

#define OSPF_DISTRIBUTE_UPDATE_DELAY 5

/* Update distribute-list and set timer to apply access-list. */
void
ospf_distribute_list_update (int type)
{
  struct route_table *rt;
  
  zlog_info ("ospf_distribute_list_update(): start");

  /* External info does not exist. */
  if (!(rt = EXTERNAL_INFO (type)))
    return;

  /* If exists previously invoked thread, then cancel it. */
  if (ospf_top->t_distribute_update)
    OSPF_TIMER_OFF (ospf_top->t_distribute_update);

  /* Set timer. */
  ospf_top->t_distribute_update =
    thread_add_timer (master, ospf_distribute_list_update_timer,
		      (void *) type, OSPF_DISTRIBUTE_UPDATE_DELAY);

  zlog_info ("ospf_distribute_list_update(): stop");
}

/* If access-list is updated, apply some check. */
void
ospf_filter_update (struct access_list *access)
{
  int type;
  int abr_inv = 0;
  struct ospf_area *area;
  listnode node;

  /* If OSPF instatnce does not exist, return right now. */
  if (!ospf_top)
    return;


  /* Update distribute-list, and apply filter. */
  for (type = 0; type < ZEBRA_ROUTE_MAX; type++)
    {
      if (ROUTEMAP (type) != NULL)
	{
	  /* if route-map is not NULL it may be using this access list */
	  ospf_distribute_list_update (type);
	  continue;
	}
      

      if (DISTRIBUTE_NAME (type))
	{
	  /* Keep old access-list for distribute-list. */
	  struct access_list *old = DISTRIBUTE_LIST (type);
	  
	  /* Update access-list for distribute-list. */
	  DISTRIBUTE_LIST (type) =
	    access_list_lookup (AFI_IP, DISTRIBUTE_NAME (type));
	  
	  /* No update for this distribute type. */
	  if (old == NULL && DISTRIBUTE_LIST (type) == NULL)
	    continue;
	  
	  /* Schedule distribute-list update timer. */
	  if (DISTRIBUTE_LIST (type) == NULL ||
	      strcmp (DISTRIBUTE_NAME (type), access->name) == 0)
	    ospf_distribute_list_update (type);
	}
    }

  /* Update Area access-list. */
  for (node = listhead (ospf_top->areas); node; nextnode (node))
    if ((area = getdata (node)) != NULL)
      {
	if (EXPORT_NAME (area))
	  {
	    EXPORT_LIST (area) = NULL;
	    abr_inv++;
	  }

	if (IMPORT_NAME (area))
	  {
	    IMPORT_LIST (area) = NULL;
	    abr_inv++;
	  }
      }

  /* Schedule ABR tasks -- this will be changed -- takada. */
  if (OSPF_IS_ABR && abr_inv)
    ospf_schedule_abr_task ();
}

/* Default information originate. */
DEFUN (ospf_default_information_originate_metric_type_routemap,
       ospf_default_information_originate_metric_type_routemap_cmd,
       "default-information originate metric <0-16777214> metric-type (1|2) route-map WORD",
       "Control distribution of default information\n"
       "Distribute a default route\n"
       "OSPF default metric\n"
       "OSPF metric\n"
       "OSPF metric type for default routes\n"
       "Set OSPF External Type 1 metrics\n"
       "Set OSPF External Type 2 metrics\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  int type = -1;
  int metric = -1;

  /* Get metric value. */
  if (argc >= 1)
    if (!str2metric (argv[0], &metric))
      return CMD_WARNING;

  /* Get metric type. */
  if (argc >= 2)
    if (!str2metric_type (argv[1], &type))
      return CMD_WARNING;

  if (argc == 3)
    ospf_routemap_set (DEFAULT_ROUTE, argv[2]);
  else
    ospf_routemap_unset (DEFAULT_ROUTE);

  return ospf_redistribute_default_set (DEFAULT_ORIGINATE_ZEBRA, type, metric);
}

ALIAS (ospf_default_information_originate_metric_type_routemap,
       ospf_default_information_originate_metric_type_cmd,
       "default-information originate metric <0-16777214> metric-type (1|2)",
       "Control distribution of default information\n"
       "Distribute a default route\n"
       "OSPF default metric\n"
       "OSPF metric\n"
       "OSPF metric type for default routes\n"
       "Set OSPF External Type 1 metrics\n"
       "Set OSPF External Type 2 metrics\n")

ALIAS (ospf_default_information_originate_metric_type_routemap,
       ospf_default_information_originate_metric_cmd,
       "default-information originate metric <0-16777214>",
       "Control distribution of default information\n"
       "Distribute a default route\n"
       "OSPF default metric\n"
       "OSPF metric\n")

ALIAS (ospf_default_information_originate_metric_type_routemap,
       ospf_default_information_originate_cmd,
       "default-information originate",
       "Control distribution of default information\n"
       "Distribute a default route\n")

/* Default information originate. */
DEFUN (ospf_default_information_originate_metric_routemap,
       ospf_default_information_originate_metric_routemap_cmd,
       "default-information originate metric <0-16777214> route-map WORD",
       "Control distribution of default information\n"
       "Distribute a default route\n"
       "OSPF default metric\n"
       "OSPF metric\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  int metric = -1;

  /* Get metric value. */
  if (argc >= 1)
    if (!str2metric (argv[0], &metric))
      return CMD_WARNING;

  if (argc == 2)
    ospf_routemap_set (DEFAULT_ROUTE, argv[1]);
  else
    ospf_routemap_unset (DEFAULT_ROUTE);

  return ospf_redistribute_default_set (DEFAULT_ORIGINATE_ZEBRA, -1, metric);
}

/* Default information originate. */
DEFUN (ospf_default_information_originate_routemap,
       ospf_default_information_originate_routemap_cmd,
       "default-information originate route-map WORD",
       "Control distribution of default information\n"
       "Distribute a default route\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  if (argc == 1)
    ospf_routemap_set (DEFAULT_ROUTE, argv[0]);
  else
    ospf_routemap_unset (DEFAULT_ROUTE);

  return ospf_redistribute_default_set (DEFAULT_ORIGINATE_ZEBRA, -1, -1);
}

DEFUN (ospf_default_information_originate_type_metric_routemap,
       ospf_default_information_originate_type_metric_routemap_cmd,
       "default-information originate metric-type (1|2) metric <0-16777214> route-map WORD",
       "Control distribution of default information\n"
       "Distribute a default route\n"
       "OSPF metric type for default routes\n"
       "Set OSPF External Type 1 metrics\n"
       "Set OSPF External Type 2 metrics\n"
       "OSPF default metric\n"
       "OSPF metric\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  int type = -1;
  int metric = -1;

  /* Get metric type. */
  if (argc >= 1)
    if (!str2metric_type (argv[0], &type))
      return CMD_WARNING;

  /* Get metric value. */
  if (argc >= 2)
    if (!str2metric (argv[1], &metric))
      return CMD_WARNING;

  if (argc == 3)
    ospf_routemap_set (DEFAULT_ROUTE, argv[2]);
  else
    ospf_routemap_unset (DEFAULT_ROUTE);

  return ospf_redistribute_default_set (DEFAULT_ORIGINATE_ZEBRA, type, metric);
}

ALIAS (ospf_default_information_originate_type_metric_routemap,
       ospf_default_information_originate_type_metric_cmd,
       "default-information originate metric-type (1|2) metric <0-16777214>",
       "Control distribution of default information\n"
       "Distribute a default route\n"
       "OSPF metric type for default routes\n"
       "Set OSPF External Type 1 metrics\n"
       "Set OSPF External Type 2 metrics\n"
       "OSPF default metric\n"
       "OSPF metric\n")

ALIAS (ospf_default_information_originate_type_metric_routemap,
       ospf_default_information_originate_type_cmd,
       "default-information originate metric-type (1|2)",
       "Control distribution of default information\n"
       "Distribute a default route\n"
       "OSPF metric type for default routes\n"
       "Set OSPF External Type 1 metrics\n"
       "Set OSPF External Type 2 metrics\n")

DEFUN (ospf_default_information_originate_type_routemap,
       ospf_default_information_originate_type_routemap_cmd,
       "default-information originate metric-type (1|2) route-map WORD",
       "Control distribution of default information\n"
       "Distribute a default route\n"
       "OSPF metric type for default routes\n"
       "Set OSPF External Type 1 metrics\n"
       "Set OSPF External Type 2 metrics\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  int type = -1;

  /* Get metric type. */
  if (argc >= 1)
    if (!str2metric_type (argv[0], &type))
      return CMD_WARNING;

  if (argc == 2)
    ospf_routemap_set (DEFAULT_ROUTE, argv[1]);
  else
    ospf_routemap_unset (DEFAULT_ROUTE);

  return ospf_redistribute_default_set (DEFAULT_ORIGINATE_ZEBRA, type, -1);
}

DEFUN (ospf_default_information_originate_always_metric_type_routemap,
       ospf_default_information_originate_always_metric_type_routemap_cmd,
       "default-information originate always metric <0-16777214> metric-type (1|2) route-map WORD",
       "Control distribution of default information\n"
       "Distribute a default route\n"
       "Always advertise default route\n"
       "OSPF default metric\n"
       "OSPF metric\n"
       "OSPF metric type for default routes\n"
       "Set OSPF External Type 1 metrics\n"
       "Set OSPF External Type 2 metrics\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  int type = -1;
  int metric = -1;

  /* Get metric value. */
  if (argc >= 1)
    if (!str2metric (argv[0], &metric))
      return CMD_WARNING;

  /* Get metric type. */
  if (argc >= 2)
    if (!str2metric_type (argv[1], &type))
      return CMD_WARNING;

  if (argc == 3)
    ospf_routemap_set (DEFAULT_ROUTE, argv[2]);
  else
    ospf_routemap_unset (DEFAULT_ROUTE);

  return ospf_redistribute_default_set (DEFAULT_ORIGINATE_ALWAYS,
					type, metric);
}

ALIAS (ospf_default_information_originate_always_metric_type_routemap,
       ospf_default_information_originate_always_metric_type_cmd,
       "default-information originate always metric <0-16777214> metric-type (1|2)",
       "Control distribution of default information\n"
       "Distribute a default route\n"
       "Always advertise default route\n"
       "OSPF default metric\n"
       "OSPF metric\n"
       "OSPF metric type for default routes\n"
       "Set OSPF External Type 1 metrics\n"
       "Set OSPF External Type 2 metrics\n")

ALIAS (ospf_default_information_originate_always_metric_type_routemap,
       ospf_default_information_originate_always_metric_cmd,
       "default-information originate always metric <0-16777214>",
       "Control distribution of default information\n"
       "Distribute a default route\n"
       "Always advertise default route\n"
       "OSPF default metric\n"
       "OSPF metric\n"
       "OSPF metric type for default routes\n")

ALIAS (ospf_default_information_originate_always_metric_type_routemap,
       ospf_default_information_originate_always_cmd,
       "default-information originate always",
       "Control distribution of default information\n"
       "Distribute a default route\n"
       "Always advertise default route\n")

DEFUN (ospf_default_information_originate_always_metric_routemap,
       ospf_default_information_originate_always_metric_routemap_cmd,
       "default-information originate always metric <0-16777214> route-map WORD",
       "Control distribution of default information\n"
       "Distribute a default route\n"
       "Always advertise default route\n"
       "OSPF default metric\n"
       "OSPF metric\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  int metric = -1;

  /* Get metric value. */
  if (argc >= 1)
    if (!str2metric (argv[0], &metric))
      return CMD_WARNING;

  if (argc == 2)
    ospf_routemap_set (DEFAULT_ROUTE, argv[1]);
  else
    ospf_routemap_unset (DEFAULT_ROUTE);

  return ospf_redistribute_default_set (DEFAULT_ORIGINATE_ALWAYS, -1, metric);
}

DEFUN (ospf_default_information_originate_always_routemap,
       ospf_default_information_originate_always_routemap_cmd,
       "default-information originate always route-map WORD",
       "Control distribution of default information\n"
       "Distribute a default route\n"
       "Always advertise default route\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  if (argc == 1)
    ospf_routemap_set (DEFAULT_ROUTE, argv[0]);
  else
    ospf_routemap_unset (DEFAULT_ROUTE);

  return ospf_redistribute_default_set (DEFAULT_ORIGINATE_ALWAYS, -1, -1);
}

DEFUN (ospf_default_information_originate_always_type_metric_routemap,
       ospf_default_information_originate_always_type_metric_routemap_cmd,
       "default-information originate always metric-type (1|2) metric <0-16777214> route-map WORD",
       "Control distribution of default information\n"
       "Distribute a default route\n"
       "Always advertise default route\n"
       "OSPF metric type for default routes\n"
       "Set OSPF External Type 1 metrics\n"
       "Set OSPF External Type 2 metrics\n"
       "OSPF default metric\n"
       "OSPF metric\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  int type = -1;
  int metric = -1;

  /* Get metric type. */
  if (argc >= 1)
    if (!str2metric_type (argv[0], &type))
      return CMD_WARNING;

  /* Get metric value. */
  if (argc >= 2)
    if (!str2metric (argv[1], &metric))
      return CMD_WARNING;

  if (argc == 3)
    ospf_routemap_set (DEFAULT_ROUTE, argv[2]);
  else
    ospf_routemap_unset (DEFAULT_ROUTE);

  return ospf_redistribute_default_set (DEFAULT_ORIGINATE_ALWAYS,
					type, metric);
}

ALIAS (ospf_default_information_originate_always_type_metric_routemap,
       ospf_default_information_originate_always_type_metric_cmd,
       "default-information originate always metric-type (1|2) metric <0-16777214>",
       "Control distribution of default information\n"
       "Distribute a default route\n"
       "Always advertise default route\n"
       "OSPF metric type for default routes\n"
       "Set OSPF External Type 1 metrics\n"
       "Set OSPF External Type 2 metrics\n"
       "OSPF default metric\n"
       "OSPF metric\n")

ALIAS (ospf_default_information_originate_always_type_metric_routemap,
       ospf_default_information_originate_always_type_cmd,
       "default-information originate always metric-type (1|2)",
       "Control distribution of default information\n"
       "Distribute a default route\n"
       "Always advertise default route\n"
       "OSPF metric type for default routes\n"
       "Set OSPF External Type 1 metrics\n"
       "Set OSPF External Type 2 metrics\n")

DEFUN (ospf_default_information_originate_always_type_routemap,
       ospf_default_information_originate_always_type_routemap_cmd,
       "default-information originate always metric-type (1|2) route-map WORD",
       "Control distribution of default information\n"
       "Distribute a default route\n"
       "Always advertise default route\n"
       "OSPF metric type for default routes\n"
       "Set OSPF External Type 1 metrics\n"
       "Set OSPF External Type 2 metrics\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  int type = -1;

  /* Get metric type. */
  if (argc >= 1)
    if (!str2metric_type (argv[0], &type))
      return CMD_WARNING;

  if (argc == 2)
    ospf_routemap_set (DEFAULT_ROUTE, argv[1]);
  else
    ospf_routemap_unset (DEFAULT_ROUTE);

  return ospf_redistribute_default_set (DEFAULT_ORIGINATE_ALWAYS,
					type, -1);
}

DEFUN (no_ospf_default_information_originate,
       no_ospf_default_information_originate_cmd,
       "no default-information originate",
       NO_STR
       "Control distribution of default information\n"
       "Distribute a default route\n")
{
  struct prefix_ipv4 p;
  struct in_addr nexthop;
    
  p.family = AF_INET;
  p.prefix.s_addr = 0;
  p.prefixlen = 0;

  ospf_external_lsa_flush (DEFAULT_ROUTE, &p, 0, nexthop);

  if (EXTERNAL_INFO (DEFAULT_ROUTE)) {
    ospf_external_info_delete (DEFAULT_ROUTE, p);
    route_table_finish (EXTERNAL_INFO (DEFAULT_ROUTE));
    EXTERNAL_INFO (DEFAULT_ROUTE) = NULL;
  }

  ospf_routemap_unset (DEFAULT_ROUTE);
  return ospf_redistribute_default_unset ();
}

DEFUN (ospf_default_metric,
       ospf_default_metric_cmd,
       "default-metric <0-16777214>",
       "Set metric of redistributed routes\n"
       "Default metric\n")
{
  int metric = -1;

  if (!str2metric (argv[0], &metric))
    return CMD_WARNING;

  ospf_top->default_metric = metric;

  return CMD_SUCCESS;
}

DEFUN (no_ospf_default_metric,
       no_ospf_default_metric_cmd,
       "no default-metric",
       NO_STR
       "Set metric of redistributed routes\n")
{
  ospf_top->default_metric = -1;
  return CMD_SUCCESS;
}

ALIAS (no_ospf_default_metric,
       no_ospf_default_metric_val_cmd,
       "no default-metric <0-16777214>",
       NO_STR
       "Set metric of redistributed routes\n"
       "Default metric\n")

struct ospf_distance
{
  /* Distance value for the IP source prefix. */
  u_char distance;

  /* Name of the access-list to be matched. */
  char *access_list;
};

struct ospf_distance *
ospf_distance_new ()
{
  struct ospf_distance *new;
  new = XMALLOC (MTYPE_OSPF_DISTANCE, sizeof (struct ospf_distance));
  memset (new, 0, sizeof (struct ospf_distance));
  return new;
}

void
ospf_distance_free (struct ospf_distance *odistance)
{
  XFREE (MTYPE_OSPF_DISTANCE, odistance);
}

int
ospf_distance_set (struct vty *vty, char *distance_str, char *ip_str,
		  char *access_list_str)
{
  int ret;
  struct prefix_ipv4 p;
  u_char distance;
  struct route_node *rn;
  struct ospf_distance *odistance;

  ret = str2prefix_ipv4 (ip_str, &p);
  if (ret == 0)
    {
      vty_out (vty, "Malformed prefix%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  distance = atoi (distance_str);

  /* Get OSPF distance node. */
  rn = route_node_get (ospf_top->distance_table, (struct prefix *) &p);
  if (rn->info)
    {
      odistance = rn->info;
      route_unlock_node (rn);
    }
  else
    {
      odistance = ospf_distance_new ();
      rn->info = odistance;
    }

  /* Set distance value. */
  odistance->distance = distance;

  /* Reset access-list configuration. */
  if (odistance->access_list)
    {
      free (odistance->access_list);
      odistance->access_list = NULL;
    }
  if (access_list_str)
    odistance->access_list = strdup (access_list_str);

  return CMD_SUCCESS;
}

int
ospf_distance_unset (struct vty *vty, char *distance_str, char *ip_str,
		    char *access_list_str)
{
  int ret;
  struct prefix_ipv4 p;
  u_char distance;
  struct route_node *rn;
  struct ospf_distance *odistance;

  ret = str2prefix_ipv4 (ip_str, &p);
  if (ret == 0)
    {
      vty_out (vty, "Malformed prefix%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  distance = atoi (distance_str);

  rn = route_node_lookup (ospf_top->distance_table, (struct prefix *)&p);
  if (! rn)
    {
      vty_out (vty, "Can't find specified prefix%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  odistance = rn->info;

  if (odistance->access_list)
    free (odistance->access_list);
  ospf_distance_free (odistance);

  rn->info = NULL;
  route_unlock_node (rn);
  route_unlock_node (rn);

  return CMD_SUCCESS;
}

void
ospf_distance_reset ()
{
  struct route_node *rn;
  struct ospf_distance *odistance;

  for (rn = route_top (ospf_top->distance_table); rn; rn = route_next (rn))
    if ((odistance = rn->info) != NULL)
      {
	if (odistance->access_list)
	  free (odistance->access_list);
	ospf_distance_free (odistance);
	rn->info = NULL;
	route_unlock_node (rn);
      }
}

u_char
ospf_distance_apply (struct prefix_ipv4 *p, struct ospf_route *or)
{
#if 0
  struct route_node *rn;
  struct ospf_distance *odistance;
  struct access_list *alist;
  struct prefix_ipv4 q;

  memset (&q, 0, sizeof (struct prefix_ipv4));
  q.family = AF_INET;
  /* q.prefix =  */
  q.prefixlen = IPV4_MAX_BITLEN;
#endif /* 0 */

  if (! ospf_top)
    return 0;

#if 0
  rn = route_node_match (ospf_top->distance_table, (struct prefix *) &q);
  if (rn)
    {
      odistance = rn->info;
      route_unlock_node (rn);

      if (odistance->access_list)
	{
	  alist = access_list_lookup (AFI_IP, odistance->access_list);
	  if (alist == NULL)
	    return 0;
	  if (access_list_apply (alist, (struct prefix *) p) == FILTER_DENY)
	    return 0;

	  return odistance->distance;
	}
      else
	return odistance->distance;
    }
#endif /* 0 */

  if (ospf_top->distance_intra)
    if (or->path_type == OSPF_PATH_INTRA_AREA)
      return ospf_top->distance_intra;

  if (ospf_top->distance_inter)
    if (or->path_type == OSPF_PATH_INTER_AREA)
      return ospf_top->distance_inter;

  if (ospf_top->distance_external)
    if (or->path_type == OSPF_PATH_TYPE1_EXTERNAL
	|| or->path_type == OSPF_PATH_TYPE2_EXTERNAL)
      return ospf_top->distance_external;
  
  if (ospf_top->distance_all)
    return ospf_top->distance_all;

  return 0;
}

DEFUN (ospf_distance,
       ospf_distance_cmd,
       "distance <1-255>",
       "Define an administrative distance\n"
       "OSPF Administrative distance\n")
{
  ospf_top->distance_all = atoi (argv[0]);
  return CMD_SUCCESS;
}

DEFUN (no_ospf_distance,
       no_ospf_distance_cmd,
       "no distance <1-255>",
       NO_STR
       "Define an administrative distance\n"
       "OSPF Administrative distance\n")
{
  ospf_top->distance_all = 0;
  return CMD_SUCCESS;
}

DEFUN (no_ospf_distance_ospf,
       no_ospf_distance_ospf_cmd,
       "no distance ospf",
       NO_STR
       "Define an administrative distance\n"
       "OSPF Administrative distance\n"
       "OSPF Distance\n")
{
  ospf_top->distance_intra = 0;
  ospf_top->distance_inter = 0;
  ospf_top->distance_external = 0;
  return CMD_SUCCESS;
}

DEFUN (ospf_distance_ospf_intra,
       ospf_distance_ospf_intra_cmd,
       "distance ospf intra-area <1-255>",
       "Define an administrative distance\n"
       "OSPF Administrative distance\n"
       "Intra-area routes\n"
       "Distance for intra-area routes\n")
{
  ospf_top->distance_intra = atoi (argv[0]);
  return CMD_SUCCESS;
}

DEFUN (ospf_distance_ospf_intra_inter,
       ospf_distance_ospf_intra_inter_cmd,
       "distance ospf intra-area <1-255> inter-area <1-255>",
       "Define an administrative distance\n"
       "OSPF Administrative distance\n"
       "Intra-area routes\n"
       "Distance for intra-area routes\n"
       "Inter-area routes\n"
       "Distance for inter-area routes\n")
{
  ospf_top->distance_intra = atoi (argv[0]);
  ospf_top->distance_inter = atoi (argv[1]);
  return CMD_SUCCESS;
}

DEFUN (ospf_distance_ospf_intra_external,
       ospf_distance_ospf_intra_external_cmd,
       "distance ospf intra-area <1-255> external <1-255>",
       "Define an administrative distance\n"
       "OSPF Administrative distance\n"
       "Intra-area routes\n"
       "Distance for intra-area routes\n"
       "External routes\n"
       "Distance for external routes\n")
{
  ospf_top->distance_intra = atoi (argv[0]);
  ospf_top->distance_external = atoi (argv[1]);
  return CMD_SUCCESS;
}

DEFUN (ospf_distance_ospf_intra_inter_external,
       ospf_distance_ospf_intra_inter_external_cmd,
       "distance ospf intra-area <1-255> inter-area <1-255> external <1-255>",
       "Define an administrative distance\n"
       "OSPF Administrative distance\n"
       "Intra-area routes\n"
       "Distance for intra-area routes\n"
       "Inter-area routes\n"
       "Distance for inter-area routes\n"
       "External routes\n"
       "Distance for external routes\n")
{
  ospf_top->distance_intra = atoi (argv[0]);
  ospf_top->distance_inter = atoi (argv[1]);
  ospf_top->distance_external = atoi (argv[2]);
  return CMD_SUCCESS;
}

DEFUN (ospf_distance_ospf_intra_external_inter,
       ospf_distance_ospf_intra_external_inter_cmd,
       "distance ospf intra-area <1-255> external <1-255> inter-area <1-255>",
       "Define an administrative distance\n"
       "OSPF Administrative distance\n"
       "Intra-area routes\n"
       "Distance for intra-area routes\n"
       "External routes\n"
       "Distance for external routes\n"
       "Inter-area routes\n"
       "Distance for inter-area routes\n")
{
  ospf_top->distance_intra = atoi (argv[0]);
  ospf_top->distance_external = atoi (argv[1]);
  ospf_top->distance_inter = atoi (argv[2]);
  return CMD_SUCCESS;
}

DEFUN (ospf_distance_ospf_inter,
       ospf_distance_ospf_inter_cmd,
       "distance ospf inter-area <1-255>",
       "Define an administrative distance\n"
       "OSPF Administrative distance\n"
       "Inter-area routes\n"
       "Distance for inter-area routes\n")
{
  ospf_top->distance_inter = atoi (argv[0]);
  return CMD_SUCCESS;
}

DEFUN (ospf_distance_ospf_inter_intra,
       ospf_distance_ospf_inter_intra_cmd,
       "distance ospf inter-area <1-255> intra-area <1-255>",
       "Define an administrative distance\n"
       "OSPF Administrative distance\n"
       "Inter-area routes\n"
       "Distance for inter-area routes\n"
       "Intra-area routes\n"
       "Distance for intra-area routes\n")
{
  ospf_top->distance_inter = atoi (argv[0]);
  ospf_top->distance_intra = atoi (argv[1]);
  return CMD_SUCCESS;
}

DEFUN (ospf_distance_ospf_inter_external,
       ospf_distance_ospf_inter_external_cmd,
       "distance ospf inter-area <1-255> external <1-255>",
       "Define an administrative distance\n"
       "OSPF Administrative distance\n"
       "Inter-area routes\n"
       "Distance for inter-area routes\n"
       "External routes\n"
       "Distance for external routes\n")
{
  ospf_top->distance_inter = atoi (argv[0]);
  ospf_top->distance_external = atoi (argv[1]);
  return CMD_SUCCESS;
}

DEFUN (ospf_distance_ospf_inter_intra_external,
       ospf_distance_ospf_inter_intra_external_cmd,
       "distance ospf inter-area <1-255> intra-area <1-255> external <1-255>",
       "Define an administrative distance\n"
       "OSPF Administrative distance\n"
       "Inter-area routes\n"
       "Distance for inter-area routes\n"
       "Intra-area routes\n"
       "Distance for intra-area routes\n"
       "External routes\n"
       "Distance for external routes\n")
{
  ospf_top->distance_inter = atoi (argv[0]);
  ospf_top->distance_intra = atoi (argv[1]);
  ospf_top->distance_external = atoi (argv[2]);
  return CMD_SUCCESS;
}

DEFUN (ospf_distance_ospf_inter_external_intra,
       ospf_distance_ospf_inter_external_intra_cmd,
       "distance ospf inter-area <1-255> external <1-255> intra-area <1-255>",
       "Define an administrative distance\n"
       "OSPF Administrative distance\n"
       "Inter-area routes\n"
       "Distance for inter-area routes\n"
       "External routes\n"
       "Distance for external routes\n"
       "Intra-area routes\n"
       "Distance for intra-area routes\n")
{
  ospf_top->distance_inter = atoi (argv[0]);
  ospf_top->distance_external = atoi (argv[1]);
  ospf_top->distance_intra = atoi (argv[2]);
  return CMD_SUCCESS;
}

DEFUN (ospf_distance_ospf_external,
       ospf_distance_ospf_external_cmd,
       "distance ospf external <1-255>",
       "Define an administrative distance\n"
       "OSPF Administrative distance\n"
       "External routes\n"
       "Distance for external routes\n")
{
  ospf_top->distance_external = atoi (argv[0]);
  return CMD_SUCCESS;
}

DEFUN (ospf_distance_ospf_external_intra,
       ospf_distance_ospf_external_intra_cmd,
       "distance ospf external <1-255> intra-area <1-255>",
       "Define an administrative distance\n"
       "OSPF Administrative distance\n"
       "External routes\n"
       "Distance for external routes\n"
       "Intra-area routes\n"
       "Distance for intra-area routes\n")
{
  ospf_top->distance_external = atoi (argv[0]);
  ospf_top->distance_intra = atoi (argv[1]);
  return CMD_SUCCESS;
}

DEFUN (ospf_distance_ospf_external_inter,
       ospf_distance_ospf_external_inter_cmd,
       "distance ospf external <1-255> inter-area <1-255>",
       "Define an administrative distance\n"
       "OSPF Administrative distance\n"
       "External routes\n"
       "Distance for external routes\n"
       "Inter-area routes\n"
       "Distance for inter-area routes\n")
{
  ospf_top->distance_external = atoi (argv[0]);
  ospf_top->distance_inter = atoi (argv[1]);
  return CMD_SUCCESS;
}

DEFUN (ospf_distance_ospf_external_intra_inter,
       ospf_distance_ospf_external_intra_inter_cmd,
       "distance ospf external <1-255> intra-area <1-255> inter-area <1-255>",
       "Define an administrative distance\n"
       "OSPF Administrative distance\n"
       "External routes\n"
       "Distance for external routes\n"
       "Intra-area routes\n"
       "Distance for intra-area routes\n"
       "Inter-area routes\n"
       "Distance for inter-area routes\n")
{
  ospf_top->distance_external = atoi (argv[0]);
  ospf_top->distance_intra = atoi (argv[1]);
  ospf_top->distance_inter = atoi (argv[2]);
  return CMD_SUCCESS;
}

DEFUN (ospf_distance_ospf_external_inter_intra,
       ospf_distance_ospf_external_inter_intra_cmd,
       "distance ospf external <1-255> inter-area <1-255> intra-area <1-255>",
       "Define an administrative distance\n"
       "OSPF Administrative distance\n"
       "External routes\n"
       "Distance for external routes\n"
       "Inter-area routes\n"
       "Distance for inter-area routes\n"
       "Intra-area routes\n"
       "Distance for intra-area routes\n")
{
  ospf_top->distance_external = atoi (argv[0]);
  ospf_top->distance_inter = atoi (argv[1]);
  ospf_top->distance_intra = atoi (argv[2]);
  return CMD_SUCCESS;
}

DEFUN (ospf_distance_source,
       ospf_distance_source_cmd,
       "distance <1-255> A.B.C.D/M",
       "Administrative distance\n"
       "Distance value\n"
       "IP source prefix\n")
{
  ospf_distance_set (vty, argv[0], argv[1], NULL);
  return CMD_SUCCESS;
}

DEFUN (no_ospf_distance_source,
       no_ospf_distance_source_cmd,
       "no distance <1-255> A.B.C.D/M",
       NO_STR
       "Administrative distance\n"
       "Distance value\n"
       "IP source prefix\n")
{
  ospf_distance_unset (vty, argv[0], argv[1], NULL);
  return CMD_SUCCESS;
}

DEFUN (ospf_distance_source_access_list,
       ospf_distance_source_access_list_cmd,
       "distance <1-255> A.B.C.D/M WORD",
       "Administrative distance\n"
       "Distance value\n"
       "IP source prefix\n"
       "Access list name\n")
{
  ospf_distance_set (vty, argv[0], argv[1], argv[2]);
  return CMD_SUCCESS;
}

DEFUN (no_ospf_distance_source_access_list,
       no_ospf_distance_source_access_list_cmd,
       "no distance <1-255> A.B.C.D/M WORD",
       NO_STR
       "Administrative distance\n"
       "Distance value\n"
       "IP source prefix\n"
       "Access list name\n")
{
  ospf_distance_unset (vty, argv[0], argv[1], argv[2]);
  return CMD_SUCCESS;
}

char *distribute_str[] = { "system", "kernel", "connected", "static", "rip",
			   "ripng", "ospf", "ospf6", "bgp"};
int
config_write_ospf_redistribute (struct vty *vty)
{
  int type;

  /* redistribute print. */
  for (type = 0; type < ZEBRA_ROUTE_MAX; type++)
    if (type != zclient->redist_default && zclient->redist[type])
      {
        vty_out (vty, " redistribute %s", distribute_str[type]);
	if (ospf_top->dmetric[type].value >= 0)
	  vty_out (vty, " metric %d", ospf_top->dmetric[type].value);
	
        if (ospf_top->dmetric[type].type == EXTERNAL_METRIC_TYPE_1)
	  vty_out (vty, " metric-type 1");

	if (ROUTEMAP_NAME (type))
	  vty_out (vty, " route-map %s", ROUTEMAP_NAME (type));
	
        vty_out (vty, "%s", VTY_NEWLINE);
      }

  return 0;
}

int
config_write_ospf_default_metric (struct vty *vty)
{
  if (ospf_top->default_metric != -1)
    vty_out (vty, " default-metric %d%s", ospf_top->default_metric,
	     VTY_NEWLINE);
  return 0;
}

int
config_write_ospf_distribute (struct vty *vty)
{
  int type;

  if (ospf_top)
    {
      /* distribute-list print. */
      for (type = 0; type < ZEBRA_ROUTE_MAX; type++)
	if (ospf_top->dlist[type].name)
	  vty_out (vty, " distribute-list %s out %s%s", 
		   ospf_top->dlist[type].name,
		   distribute_str[type], VTY_NEWLINE);

      /* default-information print. */
      if (ospf_top->default_originate != DEFAULT_ORIGINATE_NONE)
	{
	  if (ospf_top->default_originate == DEFAULT_ORIGINATE_ZEBRA)
	    vty_out (vty, " default-information originate");
	  else
	    vty_out (vty, " default-information originate always");

	  if (ospf_top->dmetric[DEFAULT_ROUTE].value >= 0)
	    vty_out (vty, " metric %d",
		     ospf_top->dmetric[DEFAULT_ROUTE].value);
	  if (ospf_top->dmetric[DEFAULT_ROUTE].type == EXTERNAL_METRIC_TYPE_1)
	    vty_out (vty, " metric-type 1");

	  if (ROUTEMAP_NAME (DEFAULT_ROUTE))
	    vty_out (vty, " route-map %s", ROUTEMAP_NAME (DEFAULT_ROUTE));
	  
	  vty_out (vty, "%s", VTY_NEWLINE);
	}

    }

  return 0;
}

int
config_write_ospf_distance (struct vty *vty)
{
  struct route_node *rn;
  struct ospf_distance *odistance;

  if (ospf_top->distance_all)
    vty_out (vty, " distance %d%s", ospf_top->distance_all, VTY_NEWLINE);

  if (ospf_top->distance_intra 
      || ospf_top->distance_inter 
      || ospf_top->distance_external)
    {
      vty_out (vty, " distance ospf");

      if (ospf_top->distance_intra)
	vty_out (vty, " intra-area %d", ospf_top->distance_intra);
      if (ospf_top->distance_inter)
	vty_out (vty, " inter-area %d", ospf_top->distance_inter);
      if (ospf_top->distance_external)
	vty_out (vty, " external %d", ospf_top->distance_external);

      vty_out (vty, "%s", VTY_NEWLINE);
    }
  
  for (rn = route_top (ospf_top->distance_table); rn; rn = route_next (rn))
    if ((odistance = rn->info) != NULL)
      {
	vty_out (vty, " distance %d %s/%d %s%s", odistance->distance,
		 inet_ntoa (rn->p.u.prefix4), rn->p.prefixlen,
		 odistance->access_list ? odistance->access_list : "",
		 VTY_NEWLINE);
      }
  return 0;
}

/* Zebra node structure. */
struct cmd_node zebra_node =
{
  ZEBRA_NODE,
  "%s(config-router)#",
};

void
zebra_init ()
{
  /* Allocate zebra structure. */
  zclient = zclient_new ();
  zclient_init (zclient, ZEBRA_ROUTE_OSPF);
  zclient->interface_add = ospf_interface_add;
  zclient->interface_delete = ospf_interface_delete;
  zclient->interface_up = ospf_interface_state_up;
  zclient->interface_down = ospf_interface_state_down;
  zclient->interface_address_add = ospf_interface_address_add;
  zclient->interface_address_delete = ospf_interface_address_delete;
  zclient->ipv4_route_add = ospf_zebra_read_ipv4;
  zclient->ipv4_route_delete = ospf_zebra_read_ipv4;

  install_element (OSPF_NODE, &ospf_redistribute_source_type_metric_cmd);
  install_element (OSPF_NODE, &ospf_redistribute_source_metric_type_cmd);
  install_element (OSPF_NODE, &ospf_redistribute_source_type_cmd);
  install_element (OSPF_NODE, &ospf_redistribute_source_metric_cmd);
  install_element (OSPF_NODE, &ospf_redistribute_source_cmd);
  install_element (OSPF_NODE,
		   &ospf_redistribute_source_metric_type_routemap_cmd);
  install_element (OSPF_NODE,
		   &ospf_redistribute_source_type_metric_routemap_cmd);
  install_element (OSPF_NODE, &ospf_redistribute_source_metric_routemap_cmd);
  install_element (OSPF_NODE, &ospf_redistribute_source_type_routemap_cmd);
  install_element (OSPF_NODE, &ospf_redistribute_source_routemap_cmd);
  
  install_element (OSPF_NODE, &no_ospf_redistribute_source_cmd);

  install_element (OSPF_NODE, &ospf_distribute_list_out_cmd);
  install_element (OSPF_NODE, &no_ospf_distribute_list_out_cmd);

  install_element (OSPF_NODE,
		   &ospf_default_information_originate_metric_type_cmd);
  install_element (OSPF_NODE, &ospf_default_information_originate_metric_cmd);
  install_element (OSPF_NODE,
		   &ospf_default_information_originate_type_metric_cmd);
  install_element (OSPF_NODE, &ospf_default_information_originate_type_cmd);
  install_element (OSPF_NODE, &ospf_default_information_originate_cmd);
  install_element (OSPF_NODE,
		   &ospf_default_information_originate_always_metric_type_cmd);
  install_element (OSPF_NODE,
		   &ospf_default_information_originate_always_metric_cmd);
  install_element (OSPF_NODE,
		   &ospf_default_information_originate_always_cmd);
  install_element (OSPF_NODE,
		   &ospf_default_information_originate_always_type_metric_cmd);
  install_element (OSPF_NODE,
		   &ospf_default_information_originate_always_type_cmd);

  install_element (OSPF_NODE,
		   &ospf_default_information_originate_metric_type_routemap_cmd);
  install_element (OSPF_NODE,
		   &ospf_default_information_originate_metric_routemap_cmd);
  install_element (OSPF_NODE,
		   &ospf_default_information_originate_routemap_cmd);
  install_element (OSPF_NODE,
		   &ospf_default_information_originate_type_metric_routemap_cmd);
  install_element (OSPF_NODE,
		   &ospf_default_information_originate_type_routemap_cmd);
  install_element (OSPF_NODE,
		   &ospf_default_information_originate_always_metric_type_routemap_cmd);
  install_element (OSPF_NODE,
		   &ospf_default_information_originate_always_metric_routemap_cmd);
  install_element (OSPF_NODE,
		   &ospf_default_information_originate_always_routemap_cmd);
  install_element (OSPF_NODE,
		   &ospf_default_information_originate_always_type_metric_routemap_cmd);
  install_element (OSPF_NODE,
		   &ospf_default_information_originate_always_type_routemap_cmd);

  install_element (OSPF_NODE, &no_ospf_default_information_originate_cmd);

  install_element (OSPF_NODE, &ospf_default_metric_cmd);
  install_element (OSPF_NODE, &no_ospf_default_metric_cmd);
  install_element (OSPF_NODE, &no_ospf_default_metric_val_cmd);

  install_element (OSPF_NODE, &ospf_distance_cmd);
  install_element (OSPF_NODE, &no_ospf_distance_cmd);
  install_element (OSPF_NODE, &no_ospf_distance_ospf_cmd);
  install_element (OSPF_NODE, &ospf_distance_ospf_intra_cmd);
  install_element (OSPF_NODE, &ospf_distance_ospf_intra_inter_cmd);
  install_element (OSPF_NODE, &ospf_distance_ospf_intra_external_cmd);
  install_element (OSPF_NODE, &ospf_distance_ospf_intra_inter_external_cmd);
  install_element (OSPF_NODE, &ospf_distance_ospf_intra_external_inter_cmd);
  install_element (OSPF_NODE, &ospf_distance_ospf_inter_cmd);
  install_element (OSPF_NODE, &ospf_distance_ospf_inter_intra_cmd);
  install_element (OSPF_NODE, &ospf_distance_ospf_inter_external_cmd);
  install_element (OSPF_NODE, &ospf_distance_ospf_inter_intra_external_cmd);
  install_element (OSPF_NODE, &ospf_distance_ospf_inter_external_intra_cmd);
  install_element (OSPF_NODE, &ospf_distance_ospf_external_cmd);
  install_element (OSPF_NODE, &ospf_distance_ospf_external_intra_cmd);
  install_element (OSPF_NODE, &ospf_distance_ospf_external_inter_cmd);
  install_element (OSPF_NODE, &ospf_distance_ospf_external_intra_inter_cmd);
  install_element (OSPF_NODE, &ospf_distance_ospf_external_inter_intra_cmd);
#if 0
  install_element (OSPF_NODE, &ospf_distance_source_cmd);
  install_element (OSPF_NODE, &no_ospf_distance_source_cmd);
  install_element (OSPF_NODE, &ospf_distance_source_access_list_cmd);
  install_element (OSPF_NODE, &no_ospf_distance_source_access_list_cmd);
#endif /* 0 */

  access_list_add_hook (ospf_filter_update);
  access_list_delete_hook (ospf_filter_update);
}
