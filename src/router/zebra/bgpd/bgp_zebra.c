/* zebra client
 * Copyright (C) 1997, 98, 99 Kunihiro Ishiguro
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

#include "command.h"
#include "stream.h"
#include "network.h"
#include "prefix.h"
#include "log.h"
#include "sockunion.h"
#include "zclient.h"
#include "routemap.h"

#include "bgpd/bgpd.h"
#include "bgpd/bgp_route.h"
#include "bgpd/bgp_attr.h"
#include "bgpd/bgp_nexthop.h"

int bgp_interface_add (int, struct zclient *, zebra_size_t);
int bgp_interface_delete (int, struct zclient *, zebra_size_t);
int bgp_interface_address_add (int, struct zclient *, zebra_size_t);
int bgp_interface_address_delete (int, struct zclient *, zebra_size_t);

/* All information about zebra. */
static struct zclient *zclient = NULL;

/* Update default router id. */
int
bgp_if_update (struct interface *ifp)
{
  struct bgp *bgp;
  listnode cn;
  struct listnode *nn;
  struct listnode *nm;
  struct peer_conf *conf;

  for (cn = listhead (ifp->connected); cn; nextnode (cn))
    {
      struct connected *co;
      struct in_addr addr;

      co = getdata (cn);

      if (co->address->family == AF_INET)
	{
	  addr = co->address->u.prefix4;

	  /* Ignore NET127. */
	  if (IPV4_NET127 (ntohl (addr.s_addr)))
	    continue;

	  LIST_LOOP (bgp_list, bgp, nn)
	    {
	      /* Respect configured router id */
	      if (! (bgp->config & BGP_CONFIG_ROUTER_ID))
		if (ntohl (bgp->id.s_addr) < ntohl (addr.s_addr))
		  {
		    bgp->id = addr;
		    LIST_LOOP (bgp->peer_conf, conf, nm)
		      {
			conf->peer->local_id = addr;
		      }
		  }
	    }
	}
    }
  return 0;
}

int
bgp_if_update_all ()
{
  listnode node;
  struct interface *ifp;

  for (node = listhead (iflist); node; node = nextnode (node))
    {
      ifp = getdata (node);
      bgp_if_update (ifp);
    }
  return 0;
}

/* Inteface addition message from zebra. */
int
bgp_interface_add (int command, struct zclient *zclient, zebra_size_t length)
{
  struct interface *ifp;

  ifp = zebra_interface_add_read (zclient->ibuf);

#if 0
  if (IS_BGP_DEBUG_ZEBRA)
    zlog_info ("BGP interface add %s index %d flags %d metric %d mtu %d",
	       ifp->name, ifp->ifindex, ifp->flags, ifp->metric, ifp->mtu);
#endif /* 0 */  

  bgp_if_update (ifp);

  return 0;
}

int
bgp_interface_delete (int command, struct zclient *zclient,
		      zebra_size_t length)
{
  struct stream *s;
  struct interface *ifp;

  s = zclient->ibuf;
  ifp = zebra_interface_state_read (s);

  return 0;
}

int
bgp_interface_up (int command, struct zclient *zclient, zebra_size_t length)
{
  struct stream *s;
  struct interface *ifp;
  struct connected *c;
  listnode node;

  s = zclient->ibuf;
  ifp = zebra_interface_state_read (s);

  if (! ifp)
    return 0;

  for (node = listhead (ifp->connected); node; nextnode (node))
    {
      c = getdata (node);
      bgp_connected_add (c);
    }

  return 0;
}

int
bgp_interface_down (int command, struct zclient *zclient, zebra_size_t length)
{
  struct stream *s;
  struct interface *ifp;
  struct connected *c;
  listnode node;

  s = zclient->ibuf;
  ifp = zebra_interface_state_read (s);
  if (! ifp)
    return 0;

  for (node = listhead (ifp->connected); node; nextnode (node))
    {
      c = getdata (node);
      bgp_connected_delete (c);
    }

  return 0;
}

int
bgp_interface_address_add (int command, struct zclient *zclient,
			   zebra_size_t length)
{
  struct connected *ifc;

  ifc = zebra_interface_address_add_read (zclient->ibuf);

  if (ifc == NULL)
    return 0;

#if 0
  if (IS_BGP_DEBUG_ZEBRA)
    {
      struct prefix *p;
      char buf[INET6_ADDRSTRLEN];

      p = c->address;
      if (p->family == AF_INET6)
	zlog_info ("BGP connected address %s/%d", 
		   inet_ntop (AF_INET6, &p->u.prefix6, buf, INET6_ADDRSTRLEN),
		   p->prefixlen);
    }
#endif /* 0 */

  bgp_if_update (ifc->ifp);

  if (if_is_up (ifc->ifp))
    bgp_connected_add (ifc);

  return 0;
}

int
bgp_interface_address_delete (int command, struct zclient *zclient,
			      zebra_size_t length)
{
  struct connected *ifc;

  ifc = zebra_interface_address_delete_read (zclient->ibuf);

  if (ifc == NULL)
    return 0;

  bgp_if_update (ifc->ifp);

  if (if_is_up (ifc->ifp))
    bgp_connected_delete (ifc);

  connected_free (ifc);

  return 0;
}

/* Zebra route add and delete treatment. */
int
zebra_read_ipv4 (int command, struct zclient *zclient, zebra_size_t length)
{
  struct stream *s;
  struct zapi_ipv4 api;
  unsigned long ifindex;
  struct in_addr nexthop;
  struct prefix_ipv4 p;

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
  else
    api.metric = 0;

  if (command == ZEBRA_IPV4_ROUTE_ADD)
    bgp_redistribute_add ((struct prefix *)&p, &nexthop, api.metric, api.type);
  else
    bgp_redistribute_delete ((struct prefix *)&p, api.type);

  return 0;
}

#ifdef HAVE_IPV6
/* Zebra route add and delete treatment. */
int
zebra_read_ipv6 (int command, struct zclient *zclient, zebra_size_t length)
{
  struct stream *s;
  struct zapi_ipv6 api;
  unsigned long ifindex;
  struct in6_addr nexthop;
  struct prefix_ipv6 p;

  s = zclient->ibuf;
  ifindex = 0;
  memset (&nexthop, 0, sizeof (struct in6_addr));

  /* Type, flags, message. */
  api.type = stream_getc (s);
  api.flags = stream_getc (s);
  api.message = stream_getc (s);

  /* IPv6 prefix. */
  memset (&p, 0, sizeof (struct prefix_ipv6));
  p.family = AF_INET6;
  p.prefixlen = stream_getc (s);
  stream_get (&p.prefix, s, PSIZE (p.prefixlen));

  /* Nexthop, ifindex, distance, metric. */
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP))
    {
      api.nexthop_num = stream_getc (s);
      stream_get (&nexthop, s, 16);
    }
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_IFINDEX))
    {
      api.ifindex_num = stream_getc (s);
      ifindex = stream_getl (s);
    }
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_DISTANCE))
    api.distance = stream_getc (s);
  else
    api.distance = 0;
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_METRIC))
    api.metric = stream_getl (s);
  else
    api.metric = 0;

  /* Simply ignore link-local address. */
  if (IN6_IS_ADDR_LINKLOCAL (&p.prefix))
    return 0;

  if (command == ZEBRA_IPV6_ROUTE_ADD)
    bgp_redistribute_add ((struct prefix *)&p, NULL, api.metric, api.type);
  else
    bgp_redistribute_delete ((struct prefix *) &p, api.type);
  
  return 0;
}
#endif /* HAVE_IPV6 */

/* Other routes redistribution into BGP. */
void
bgp_redistribute_set (struct bgp *bgp, afi_t afi, int type)
{
  /* Set flag to BGP instance. */
  bgp->redist[afi][type] = 1;

  /* Return if already redistribute flag is set. */
  if (zclient->redist[type])
    return;

  zclient->redist[type] = 1;

  /* Return if zebra connection is not established. */
  if (zclient->sock < 0)
    return;
    
  /* Send distribute add message to zebra. */
  zebra_redistribute_send (ZEBRA_REDISTRIBUTE_ADD, zclient->sock, type);
}

/* Redistribute with route-map specification. */
void
bgp_redistribute_routemap_set (struct bgp *bgp, afi_t afi, int type,
			       char *name)
{
  bgp_redistribute_set (bgp, afi, type);

  if (bgp->rmap[afi][type].name)
    free (bgp->rmap[afi][type].name);

  bgp->rmap[afi][type].name = strdup (name);
  bgp->rmap[afi][type].map = route_map_lookup_by_name (name);
}

/* Unset redistribution. */
void
bgp_redistribute_unset (struct bgp *bgp, afi_t afi, int type)
{
  /* Unset flag from BGP instance. */
  bgp->redist[afi][type] = 0;

  /* Unset route-map. */
  if (bgp->rmap[afi][type].name)
    free (bgp->rmap[afi][type].name);
  bgp->rmap[afi][type].name = NULL;
  bgp->rmap[afi][type].map = NULL;

  /* Return if zebra connection is disabled. */
  if (! zclient->redist[type])
    return;
  zclient->redist[type] = 0;

  if (bgp->redist[AFI_IP][type] == 0 
      && bgp->redist[AFI_IP6][type] == 0 
      && zclient->sock >= 0)
    /* Send distribute delete message to zebra. */
    zebra_redistribute_send (ZEBRA_REDISTRIBUTE_DELETE, zclient->sock, type);
  
  /* Withdraw redistributed routes from current BGP's routing table. */
  bgp_redistribute_withdraw (bgp, afi, type);
}

DEFUN (bgp_redistribute_kernel,
       bgp_redistribute_kernel_cmd,
       "redistribute kernel",
       "Redistribute information from another routing protocol\n"
       "Kernel routes\n")
{
  bgp_redistribute_set (vty->index, AFI_IP, ZEBRA_ROUTE_KERNEL);
  return CMD_SUCCESS;
}

DEFUN (bgp_redistribute_kernel_routemap,
       bgp_redistribute_kernel_routemap_cmd,
       "redistribute kernel route-map WORD",
       "Redistribute information from another routing protocol\n"
       "Kernel routes\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  bgp_redistribute_routemap_set (vty->index, AFI_IP, ZEBRA_ROUTE_KERNEL, argv[0]);
  return CMD_SUCCESS;
}

DEFUN (no_bgp_redistribute_kernel,
       no_bgp_redistribute_kernel_cmd,
       "no redistribute kernel",
       NO_STR
       "Redistribute information from another routing protocol\n"
       "Kernel routes\n")
{
  bgp_redistribute_unset (vty->index, AFI_IP, ZEBRA_ROUTE_KERNEL);
  return CMD_SUCCESS;
}

DEFUN (no_bgp_redistribute_kernel_routemap,
       no_bgp_redistribute_kernel_routemap_cmd,
       "no redistribute kernel route-map WORD",
       NO_STR
       "Redistribute information from another routing protocol\n"
       "Kernel routes\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  bgp_redistribute_unset (vty->index, AFI_IP, ZEBRA_ROUTE_KERNEL);
  return CMD_SUCCESS;
}

DEFUN (bgp_redistribute_static,
       bgp_redistribute_static_cmd,
       "redistribute static",
       "Redistribute information from another routing protocol\n"
       "Static routes\n")
{
  bgp_redistribute_set (vty->index, AFI_IP, ZEBRA_ROUTE_STATIC);
  return CMD_SUCCESS;
}

DEFUN (bgp_redistribute_static_routemap,
       bgp_redistribute_static_routemap_cmd,
       "redistribute static route-map WORD",
       "Redistribute information from another routing protocol\n"
       "Static routes\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  bgp_redistribute_routemap_set (vty->index, AFI_IP, ZEBRA_ROUTE_STATIC, argv[0]);
  return CMD_SUCCESS;
}

DEFUN (no_bgp_redistribute_static,
       no_bgp_redistribute_static_cmd,
       "no redistribute static",
       NO_STR
       "Redistribute information from another routing protocol\n"
       "Static routes\n")
{
  bgp_redistribute_unset (vty->index, AFI_IP, ZEBRA_ROUTE_STATIC);
  return CMD_SUCCESS;
}

DEFUN (no_bgp_redistribute_static_routemap,
       no_bgp_redistribute_static_routemap_cmd,
       "no redistribute static route-map WORD",
       NO_STR
       "Redistribute information from another routing protocol\n"
       "Static routes\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  bgp_redistribute_unset (vty->index, AFI_IP, ZEBRA_ROUTE_STATIC);
  return CMD_SUCCESS;
}

DEFUN (bgp_redistribute_connected,
       bgp_redistribute_connected_cmd,
       "redistribute connected",
       "Redistribute information from another routing protocol\n"
       "Connected\n")
{
  bgp_redistribute_set (vty->index, AFI_IP, ZEBRA_ROUTE_CONNECT);
  return CMD_SUCCESS;
}

DEFUN (bgp_redistribute_connected_routemap,
       bgp_redistribute_connected_routemap_cmd,
       "redistribute connected route-map WORD",
       "Redistribute information from another routing protocol\n"
       "Connected\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  bgp_redistribute_routemap_set (vty->index, AFI_IP, ZEBRA_ROUTE_CONNECT, argv[0]);
  return CMD_SUCCESS;
}

DEFUN (no_bgp_redistribute_connected,
       no_bgp_redistribute_connected_cmd,
       "no redistribute connected",
       NO_STR
       "Redistribute information from another routing protocol\n"
       "Connected\n")
{
  bgp_redistribute_unset (vty->index, AFI_IP, ZEBRA_ROUTE_CONNECT);
  return CMD_SUCCESS;
}

DEFUN (no_bgp_redistribute_connected_routemap,
       no_bgp_redistribute_connected_routemap_cmd,
       "no redistribute connected route-map WORD",
       NO_STR
       "Redistribute information from another routing protocol\n"
       "Connected\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  bgp_redistribute_unset (vty->index, AFI_IP, ZEBRA_ROUTE_CONNECT);
  return CMD_SUCCESS;
}

DEFUN (bgp_redistribute_rip,
       bgp_redistribute_rip_cmd,
       "redistribute rip",
       "Redistribute information from another routing protocol\n"
       "Routing Information Protocol (RIP)\n")
{
  bgp_redistribute_set (vty->index, AFI_IP, ZEBRA_ROUTE_RIP);
  return CMD_SUCCESS;
}

DEFUN (bgp_redistribute_rip_routemap,
       bgp_redistribute_rip_routemap_cmd,
       "redistribute rip route-map WORD",
       "Redistribute information from another routing protocol\n"
       "Routing Information Protocol (RIP)\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  bgp_redistribute_routemap_set (vty->index, AFI_IP, ZEBRA_ROUTE_RIP, argv[0]);
  return CMD_SUCCESS;
}

DEFUN (no_bgp_redistribute_rip,
       no_bgp_redistribute_rip_cmd,
       "no redistribute rip",
       NO_STR
       "Redistribute information from another routing protocol\n"
       "Routing Information Protocol (RIP)\n")
{
  bgp_redistribute_unset (vty->index, AFI_IP, ZEBRA_ROUTE_RIP);
  return CMD_SUCCESS;
}

DEFUN (no_bgp_redistribute_rip_routemap,
       no_bgp_redistribute_rip_routemap_cmd,
       "no redistribute rip route-map WORD",
       NO_STR
       "Redistribute information from another routing protocol\n"
       "Routing Information Protocol (RIP)\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  bgp_redistribute_unset (vty->index, AFI_IP, ZEBRA_ROUTE_RIP);
  return CMD_SUCCESS;
}

DEFUN (bgp_redistribute_ospf,
       bgp_redistribute_ospf_cmd,
       "redistribute ospf",
       "Redistribute information from another routing protocol\n"
       "Open Shortest Path First (OSPF)\n")
{
  bgp_redistribute_set (vty->index, AFI_IP, ZEBRA_ROUTE_OSPF);
  return CMD_SUCCESS;
}

DEFUN (bgp_redistribute_ospf_routemap,
       bgp_redistribute_ospf_routemap_cmd,
       "redistribute ospf route-map WORD",
       "Redistribute information from another routing protocol\n"
       "Open Shortest Path First (OSPF)\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  bgp_redistribute_routemap_set (vty->index, AFI_IP, ZEBRA_ROUTE_OSPF, argv[0]);
  return CMD_SUCCESS;
}

DEFUN (no_bgp_redistribute_ospf,
       no_bgp_redistribute_ospf_cmd,
       "no redistribute ospf",
       NO_STR
       "Redistribute information from another routing protocol\n"
       "Open Shortest Path First (OSPF)\n")
{
  bgp_redistribute_unset (vty->index, AFI_IP, ZEBRA_ROUTE_OSPF);
  return CMD_SUCCESS;
}

DEFUN (no_bgp_redistribute_ospf_routemap,
       no_bgp_redistribute_ospf_routemap_cmd,
       "no redistribute ospf route-map WORD",
       NO_STR
       "Redistribute information from another routing protocol\n"
       "Open Shortest Path First (OSPF)\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  bgp_redistribute_unset (vty->index, AFI_IP, ZEBRA_ROUTE_OSPF);
  return CMD_SUCCESS;
}

#ifdef HAVE_IPV6
DEFUN (ipv6_bgp_redistribute_kernel,
       ipv6_bgp_redistribute_kernel_cmd,
       "redistribute kernel",
       "Redistribute information from another routing protocol\n"
       "Kernel routes\n")
{
  bgp_redistribute_set (vty->index, AFI_IP6, ZEBRA_ROUTE_KERNEL);
  return CMD_SUCCESS;
}

DEFUN (ipv6_bgp_redistribute_kernel_routemap,
       ipv6_bgp_redistribute_kernel_routemap_cmd,
       "redistribute kernel route-map WORD",
       "Redistribute information from another routing protocol\n"
       "Kernel routes\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  bgp_redistribute_routemap_set (vty->index, AFI_IP6, ZEBRA_ROUTE_KERNEL, argv[0]);
  return CMD_SUCCESS;
}

DEFUN (no_ipv6_bgp_redistribute_kernel,
       no_ipv6_bgp_redistribute_kernel_cmd,
       "no redistribute kernel",
       NO_STR
       "Redistribute information from another routing protocol\n"
       "Kernel routes\n")
{
  bgp_redistribute_unset (vty->index, AFI_IP6, ZEBRA_ROUTE_KERNEL);
  return CMD_SUCCESS;
}

DEFUN (no_ipv6_bgp_redistribute_kernel_routemap,
       no_ipv6_bgp_redistribute_kernel_routemap_cmd,
       "no redistribute kernel route-map WORD",
       NO_STR
       "Redistribute information from another routing protocol\n"
       "Kernel routes\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  bgp_redistribute_unset (vty->index, AFI_IP6, ZEBRA_ROUTE_KERNEL);
  return CMD_SUCCESS;
}

DEFUN (ipv6_bgp_redistribute_static,
       ipv6_bgp_redistribute_static_cmd,
       "redistribute static",
       "Redistribute information from another routing protocol\n"
       "Static routes\n")
{
  bgp_redistribute_set (vty->index, AFI_IP6, ZEBRA_ROUTE_STATIC);
  return CMD_SUCCESS;
}

DEFUN (ipv6_bgp_redistribute_static_routemap,
       ipv6_bgp_redistribute_static_routemap_cmd,
       "redistribute static route-map WORD",
       "Redistribute information from another routing protocol\n"
       "Static routes\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  bgp_redistribute_routemap_set (vty->index, AFI_IP6, ZEBRA_ROUTE_STATIC, argv[0]);
  return CMD_SUCCESS;
}

DEFUN (no_ipv6_bgp_redistribute_static,
       no_ipv6_bgp_redistribute_static_cmd,
       "no redistribute static",
       NO_STR
       "Redistribute information from another routing protocol\n"
       "Static routes\n")
{
  bgp_redistribute_unset (vty->index, AFI_IP6, ZEBRA_ROUTE_STATIC);
  return CMD_SUCCESS;
}

DEFUN (no_ipv6_bgp_redistribute_static_routemap,
       no_ipv6_bgp_redistribute_static_routemap_cmd,
       "no redistribute static route-map WORD",
       NO_STR
       "Redistribute information from another routing protocol\n"
       "Static routes\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  bgp_redistribute_unset (vty->index, AFI_IP6, ZEBRA_ROUTE_STATIC);
  return CMD_SUCCESS;
}

DEFUN (ipv6_bgp_redistribute_connected,
       ipv6_bgp_redistribute_connected_cmd,
       "redistribute connected",
       "Redistribute information from another routing protocol\n"
       "Connected\n")
{
  bgp_redistribute_set (vty->index, AFI_IP6, ZEBRA_ROUTE_CONNECT);
  return CMD_SUCCESS;
}

DEFUN (ipv6_bgp_redistribute_connected_routemap,
       ipv6_bgp_redistribute_connected_routemap_cmd,
       "redistribute connected route-map WORD",
       "Redistribute information from another routing protocol\n"
       "Connected\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  bgp_redistribute_routemap_set (vty->index, AFI_IP6, ZEBRA_ROUTE_CONNECT, argv[0]);
  return CMD_SUCCESS;
}

DEFUN (no_ipv6_bgp_redistribute_connected,
       no_ipv6_bgp_redistribute_connected_cmd,
       "no redistribute connected",
       NO_STR
       "Redistribute information from another routing protocol\n"
       "Connected\n")
{
  bgp_redistribute_unset (vty->index, AFI_IP6, ZEBRA_ROUTE_CONNECT);
  return CMD_SUCCESS;
}

DEFUN (no_ipv6_bgp_redistribute_connected_routemap,
       no_ipv6_bgp_redistribute_connected_routemap_cmd,
       "no redistribute connected route-map WORD",
       NO_STR
       "Redistribute information from another routing protocol\n"
       "Connected\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  bgp_redistribute_unset (vty->index, AFI_IP6, ZEBRA_ROUTE_CONNECT);
  return CMD_SUCCESS;
}

DEFUN (ipv6_bgp_redistribute_ripng,
       ipv6_bgp_redistribute_ripng_cmd,
       "redistribute ripng",
       "Redistribute information from another routing protocol\n"
       "IPv6 Routing Information Protocol (RIPng)\n")
{
  bgp_redistribute_set (vty->index, AFI_IP6, ZEBRA_ROUTE_RIPNG);
  return CMD_SUCCESS;
}

DEFUN (ipv6_bgp_redistribute_ripng_routemap,
       ipv6_bgp_redistribute_ripng_routemap_cmd,
       "redistribute ripng route-map WORD",
       "Redistribute information from another routing protocol\n"
       "IPv6 Routing Information Protocol (RIPng)\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  bgp_redistribute_routemap_set (vty->index, AFI_IP6, ZEBRA_ROUTE_RIPNG, argv[0]);
  return CMD_SUCCESS;
}

DEFUN (no_ipv6_bgp_redistribute_ripng,
       no_ipv6_bgp_redistribute_ripng_cmd,
       "no redistribute ripng",
       NO_STR
       "Redistribute information from another routing protocol\n"
       "IPv6 Routing Information Protocol (RIPng)\n")
{
  bgp_redistribute_unset (vty->index, AFI_IP6, ZEBRA_ROUTE_RIPNG);
  return CMD_SUCCESS;
}

DEFUN (no_ipv6_bgp_redistribute_ripng_routemap,
       no_ipv6_bgp_redistribute_ripng_routemap_cmd,
       "no redistribute ripng route-map WORD",
       NO_STR
       "Redistribute information from another routing protocol\n"
       "IPv6 Routing Information Protocol (RIPng)\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  bgp_redistribute_unset (vty->index, AFI_IP6, ZEBRA_ROUTE_RIPNG);
  return CMD_SUCCESS;
}

DEFUN (ipv6_bgp_redistribute_ospf6,
       ipv6_bgp_redistribute_ospf6_cmd,
       "redistribute ospf6",
       "Redistribute information from another routing protocol\n"
       "IPv6 Open Shortest Path First (OSPFv3)\n")
{
  bgp_redistribute_set (vty->index, AFI_IP6, ZEBRA_ROUTE_OSPF6);
  return CMD_SUCCESS;
}

DEFUN (ipv6_bgp_redistribute_ospf6_routemap,
       ipv6_bgp_redistribute_ospf6_routemap_cmd,
       "redistribute ospf6 route-map WORD",
       "Redistribute information from another routing protocol\n"
       "IPv6 Open Shortest Path First (OSPFv3)\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  bgp_redistribute_routemap_set (vty->index, AFI_IP6, ZEBRA_ROUTE_OSPF6, argv[0]);
  return CMD_SUCCESS;
}

DEFUN (no_ipv6_bgp_redistribute_ospf6,
       no_ipv6_bgp_redistribute_ospf6_cmd,
       "no redistribute ospf6",
       NO_STR
       "Redistribute information from another routing protocol\n"
       "IPv6 Open Shortest Path First (OSPFv3)\n")
{
  bgp_redistribute_unset (vty->index, AFI_IP6, ZEBRA_ROUTE_OSPF6);
  return CMD_SUCCESS;
}

DEFUN (no_ipv6_bgp_redistribute_ospf6_routemap,
       no_ipv6_bgp_redistribute_ospf6_routemap_cmd,
       "no redistribute ospf6 route-map WORD",
       NO_STR
       "Redistribute information from another routing protocol\n"
       "IPv6 Open Shortest Path First (OSPFv3)\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  bgp_redistribute_unset (vty->index, AFI_IP6, ZEBRA_ROUTE_OSPF6);
  return CMD_SUCCESS;
}

/* Old config.  */
ALIAS (ipv6_bgp_redistribute_kernel,
       old_ipv6_bgp_redistribute_kernel_cmd,
       "ipv6 bgp redistribute kernel",
       IPV6_STR
       BGP_STR
       "Redistribute information from another routing protocol\n"
       "Kernel routes\n")

ALIAS (ipv6_bgp_redistribute_kernel_routemap,
       old_ipv6_bgp_redistribute_kernel_routemap_cmd,
       "ipv6 bgp redistribute kernel route-map WORD",
       IPV6_STR
       BGP_STR
       "Redistribute information from another routing protocol\n"
       "Kernel routes\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

ALIAS (no_ipv6_bgp_redistribute_kernel,
       old_no_ipv6_bgp_redistribute_kernel_cmd,
       "no ipv6 bgp redistribute kernel",
       NO_STR
       IPV6_STR
       BGP_STR
       "Redistribute information from another routing protocol\n"
       "Kernel routes\n")

ALIAS (no_ipv6_bgp_redistribute_kernel_routemap,
       old_no_ipv6_bgp_redistribute_kernel_routemap_cmd,
       "no ipv6 bgp redistribute kernel route-map WORD",
       NO_STR
       IPV6_STR
       BGP_STR
       "Redistribute information from another routing protocol\n"
       "Kernel routes\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

ALIAS (ipv6_bgp_redistribute_static,
       old_ipv6_bgp_redistribute_static_cmd,
       "ipv6 bgp redistribute static",
       IPV6_STR
       BGP_STR
       "Redistribute information from another routing protocol\n"
       "Static routes\n")

ALIAS (ipv6_bgp_redistribute_static_routemap,
       old_ipv6_bgp_redistribute_static_routemap_cmd,
       "ipv6 bgp redistribute static route-map WORD",
       IPV6_STR
       BGP_STR
       "Redistribute information from another routing protocol\n"
       "Static routes\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

ALIAS (no_ipv6_bgp_redistribute_static,
       old_no_ipv6_bgp_redistribute_static_cmd,
       "no ipv6 bgp redistribute static",
       NO_STR
       IPV6_STR
       BGP_STR
       "Redistribute information from another routing protocol\n"
       "Static routes\n")

ALIAS (no_ipv6_bgp_redistribute_static_routemap,
       old_no_ipv6_bgp_redistribute_static_routemap_cmd,
       "no ipv6 bgp redistribute static route-map WORD",
       NO_STR
       IPV6_STR
       BGP_STR
       "Redistribute information from another routing protocol\n"
       "Static routes\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

ALIAS (ipv6_bgp_redistribute_connected,
       old_ipv6_bgp_redistribute_connected_cmd,
       "ipv6 bgp redistribute connected",
       IPV6_STR
       BGP_STR
       "Redistribute information from another routing protocol\n"
       "Connected\n")

ALIAS (ipv6_bgp_redistribute_connected_routemap,
       old_ipv6_bgp_redistribute_connected_routemap_cmd,
       "ipv6 bgp redistribute connected route-map WORD",
       IPV6_STR
       BGP_STR
       "Redistribute information from another routing protocol\n"
       "Connected\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

ALIAS (no_ipv6_bgp_redistribute_connected,
       old_no_ipv6_bgp_redistribute_connected_cmd,
       "no ipv6 bgp redistribute connected",
       NO_STR
       IPV6_STR
       BGP_STR
       "Redistribute information from another routing protocol\n"
       "Connected\n")

ALIAS (no_ipv6_bgp_redistribute_connected_routemap,
       old_no_ipv6_bgp_redistribute_connected_routemap_cmd,
       "no ipv6 bgp redistribute connected route-map WORD",
       NO_STR
       IPV6_STR
       BGP_STR
       "Redistribute information from another routing protocol\n"
       "Connected\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

ALIAS (ipv6_bgp_redistribute_ripng,
       old_ipv6_bgp_redistribute_ripng_cmd,
       "ipv6 bgp redistribute ripng",
       IPV6_STR
       BGP_STR
       "Redistribute information from another routing protocol\n"
       "IPv6 Routing Information Protocol (RIPng)\n")

ALIAS (ipv6_bgp_redistribute_ripng_routemap,
       old_ipv6_bgp_redistribute_ripng_routemap_cmd,
       "ipv6 bgp redistribute ripng route-map WORD",
       IPV6_STR
       BGP_STR
       "Redistribute information from another routing protocol\n"
       "IPv6 Routing Information Protocol (RIPng)\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

ALIAS (no_ipv6_bgp_redistribute_ripng,
       old_no_ipv6_bgp_redistribute_ripng_cmd,
       "no ipv6 bgp redistribute ripng",
       NO_STR
       IPV6_STR
       BGP_STR
       "Redistribute information from another routing protocol\n"
       "IPv6 Routing Information Protocol (RIPng)\n")

ALIAS (no_ipv6_bgp_redistribute_ripng_routemap,
       old_no_ipv6_bgp_redistribute_ripng_routemap_cmd,
       "no ipv6 bgp redistribute ripng route-map WORD",
       NO_STR
       IPV6_STR
       BGP_STR
       "Redistribute information from another routing protocol\n"
       "IPv6 Routing Information Protocol (RIPng)\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

ALIAS (ipv6_bgp_redistribute_ospf6,
       old_ipv6_bgp_redistribute_ospf6_cmd,
       "ipv6 bgp redistribute ospf6",
       IPV6_STR
       BGP_STR
       "Redistribute information from another routing protocol\n"
       "IPv6 Open Shortest Path First (OSPFv3)\n")

ALIAS (ipv6_bgp_redistribute_ospf6_routemap,
       old_ipv6_bgp_redistribute_ospf6_routemap_cmd,
       "ipv6 bgp redistribute ospf6 route-map WORD",
       IPV6_STR
       BGP_STR
       "Redistribute information from another routing protocol\n"
       "IPv6 Open Shortest Path First (OSPFv3)\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")

ALIAS (no_ipv6_bgp_redistribute_ospf6,
       old_no_ipv6_bgp_redistribute_ospf6_cmd,
       "no ipv6 bgp redistribute ospf6",
       NO_STR
       IPV6_STR
       BGP_STR
       "Redistribute information from another routing protocol\n"
       "IPv6 Open Shortest Path First (OSPFv3)\n")

ALIAS (no_ipv6_bgp_redistribute_ospf6_routemap,
       old_no_ipv6_bgp_redistribute_ospf6_routemap_cmd,
       "no ipv6 bgp redistribute ospf6 route-map WORD",
       NO_STR
       IPV6_STR
       BGP_STR
       "Redistribute information from another routing protocol\n"
       "IPv6 Open Shortest Path First (OSPFv3)\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
#endif /* HAVE_IPV6 */

struct interface *
if_lookup_by_ipv4 (struct in_addr *addr)
{
  listnode ifnode;
  listnode cnode;
  struct interface *ifp;
  struct connected *connected;
  struct prefix_ipv4 p;
  struct prefix *cp; 
  
  p.family = AF_INET;
  p.prefix = *addr;
  p.prefixlen = IPV4_MAX_BITLEN;

  for (ifnode = listhead (iflist); ifnode; nextnode (ifnode))
    {
      ifp = getdata (ifnode);

      for (cnode = listhead (ifp->connected); cnode; nextnode (cnode))
	{
	  connected = getdata (cnode);
	  cp = connected->address;
	    
	  if (cp->family == AF_INET)
	    if (prefix_match (cp, (struct prefix *)&p))
	      return ifp;
	}
    }
  return NULL;
}

#ifdef HAVE_IPV6
struct interface *
if_lookup_by_ipv6 (struct in6_addr *addr)
{
  listnode ifnode;
  listnode cnode;
  struct interface *ifp;
  struct connected *connected;
  struct prefix_ipv6 p;
  struct prefix *cp; 
  
  p.family = AF_INET6;
  p.prefix = *addr;
  p.prefixlen = IPV6_MAX_BITLEN;

  for (ifnode = listhead (iflist); ifnode; nextnode (ifnode))
    {
      ifp = getdata (ifnode);

      for (cnode = listhead (ifp->connected); cnode; nextnode (cnode))
	{
	  connected = getdata (cnode);
	  cp = connected->address;
	    
	  if (cp->family == AF_INET6)
	    if (prefix_match (cp, (struct prefix *)&p))
	      return ifp;
	}
    }
  return NULL;
}
#endif /* HAVE_IPV6 */

#ifdef HAVE_IPV6
int
if_get_ipv6_global (struct interface *ifp, struct in6_addr *addr)
{
  listnode cnode;
  struct connected *connected;
  struct prefix *cp; 
  
  for (cnode = listhead (ifp->connected); cnode; nextnode (cnode))
    {
      connected = getdata (cnode);
      cp = connected->address;
	    
      if (cp->family == AF_INET6)
	if (! IN6_IS_ADDR_LINKLOCAL (&cp->u.prefix6))
	  {
	    memcpy (addr, &cp->u.prefix6, IPV6_MAX_BYTELEN);
	    return 1;
	  }
    }
  return 0;
}

int
if_get_ipv6_local (struct interface *ifp, struct in6_addr *addr)
{
  listnode cnode;
  struct connected *connected;
  struct prefix *cp; 
  
  for (cnode = listhead (ifp->connected); cnode; nextnode (cnode))
    {
      connected = getdata (cnode);
      cp = connected->address;
	    
      if (cp->family == AF_INET6)
	if (IN6_IS_ADDR_LINKLOCAL (&cp->u.prefix6))
	  {
	    memcpy (addr, &cp->u.prefix6, IPV6_MAX_BYTELEN);
	    return 1;
	  }
    }
  return 0;
}
#endif /* HAVE_IPV6 */

int
bgp_nexthop_set (union sockunion *local, union sockunion *remote, 
		 struct bgp_nexthop *nexthop, struct peer *peer)
{
  int ret = 0;
  struct interface *ifp = NULL;

  memset (nexthop, 0, sizeof (struct bgp_nexthop));

  if (!local)
    return -1;
  if (!remote)
    return -1;

  if (local->sa.sa_family == AF_INET)
    {
      nexthop->v4 = local->sin.sin_addr;
      ifp = if_lookup_by_ipv4 (&local->sin.sin_addr);
    }
#ifdef HAVE_IPV6
  if (local->sa.sa_family == AF_INET6)
    {
      if (IN6_IS_ADDR_LINKLOCAL (&local->sin6.sin6_addr))
	{
	  if (peer->ifname)
	    ifp = if_lookup_by_index (if_nametoindex (peer->ifname));
	}
      else
	ifp = if_lookup_by_ipv6 (&local->sin6.sin6_addr);
    }
#endif /* HAVE_IPV6 */

  if (!ifp)
    return -1;

  nexthop->ifp = ifp;

  /* IPv4 connection. */
  if (local->sa.sa_family == AF_INET)
    {
#ifdef HAVE_IPV6
      /* IPv6 nexthop*/
      ret = if_get_ipv6_global (ifp, &nexthop->v6_global);

      /* There is no global nexthop. */
      if (!ret)
	if_get_ipv6_local (ifp, &nexthop->v6_global);
      else
	if_get_ipv6_local (ifp, &nexthop->v6_local);
#endif /* HAVE_IPV6 */
    }

#ifdef HAVE_IPV6
  /* IPv6 connection. */
  if (local->sa.sa_family == AF_INET6)
    {
      struct interface *direct = NULL;

      /* IPv4 nexthop.  I don't care about it. */
      if (peer->local_id.s_addr)
	nexthop->v4 = peer->local_id;

      /* Global address*/
      if (! IN6_IS_ADDR_LINKLOCAL (&local->sin6.sin6_addr))
	{
	  memcpy (&nexthop->v6_global, &local->sin6.sin6_addr, 
		  IPV6_MAX_BYTELEN);

	  /* If directory connected set link-local address. */
	  direct = if_lookup_by_ipv6 (&remote->sin6.sin6_addr);
	  if (direct)
	    if_get_ipv6_local (ifp, &nexthop->v6_local);
	}
      else
	/* Link-local address. */
	{
	  ret = if_get_ipv6_global (ifp, &nexthop->v6_global);

	  /* If there is no global address.  Set link-local address as
             global.  I know this break RFC specification... */
	  if (!ret)
	    memcpy (&nexthop->v6_global, &local->sin6.sin6_addr, 
		    IPV6_MAX_BYTELEN);
	  else
	    memcpy (&nexthop->v6_local, &local->sin6.sin6_addr, 
		    IPV6_MAX_BYTELEN);
	}
    }

  if (IN6_IS_ADDR_LINKLOCAL (&local->sin6.sin6_addr) ||
      if_lookup_by_ipv6 (&remote->sin6.sin6_addr))
    peer->shared_network = 1;
  else
    peer->shared_network = 0;

  /* KAME stack specific treatment.  */
#ifdef KAME
  if (IN6_IS_ADDR_LINKLOCAL (&nexthop->v6_global)
      && IN6_LINKLOCAL_IFINDEX (nexthop->v6_global))
    {
      SET_IN6_LINKLOCAL_IFINDEX (nexthop->v6_global, 0);
    }
  if (IN6_IS_ADDR_LINKLOCAL (&nexthop->v6_local)
      && IN6_LINKLOCAL_IFINDEX (nexthop->v6_local))
    {
      SET_IN6_LINKLOCAL_IFINDEX (nexthop->v6_local, 0);
    }
#endif /* KAME */
#endif /* HAVE_IPV6 */
  return ret;
}

#ifdef HAVE_IPV6
unsigned int
bgp_ifindex_by_nexthop (struct in6_addr *addr)
{
  listnode ifnode;
  listnode cnode;
  struct interface *ifp;
  struct connected *connected;
  struct prefix_ipv6 p;
  
  p.family = AF_INET6;
  p.prefix = *addr;
  p.prefixlen = IPV6_MAX_BITLEN;

  for (ifnode = listhead (iflist); ifnode; nextnode (ifnode))
    {
      ifp = getdata (ifnode);

      for (cnode = listhead (ifp->connected); cnode; nextnode (cnode))
	{
	  struct prefix *cp; 

	  connected = getdata (cnode);
	  cp = connected->address;
	    
	  if (cp->family == AF_INET6)
	    {
	      if (prefix_match (cp, (struct prefix *)&p))
		return ifp->ifindex;
	    }
	}
    }
  return 0;
}
#endif /* HAVE_IPV6 */

void
bgp_zebra_announce (struct prefix *p, struct bgp_info *info, struct bgp *bgp)
{
  int flags;
  u_char distance;
  struct peer *peer;

  if (zclient->sock < 0)
    return;

  if (! zclient->redist[ZEBRA_ROUTE_BGP])
    return;

  flags = 0;
  peer = info->peer;

  if (peer_sort (peer) == BGP_PEER_IBGP || peer_sort (peer) == BGP_PEER_CONFED)
    {
      SET_FLAG (flags, ZEBRA_FLAG_IBGP);
      SET_FLAG (flags, ZEBRA_FLAG_INTERNAL);
    }

  if (peer_sort (peer) == BGP_PEER_EBGP && peer->ttl != 1)
    SET_FLAG (flags, ZEBRA_FLAG_INTERNAL);

  if (p->family == AF_INET)
    {
      struct zapi_ipv4 api;
      struct in_addr *nexthop;

      api.flags = flags;
      nexthop = &info->attr->nexthop;

      api.type = ZEBRA_ROUTE_BGP;
      api.message = 0;
      SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
      api.nexthop_num = 1;
      api.nexthop = &nexthop;
      api.ifindex_num = 0;
      SET_FLAG (api.message, ZAPI_MESSAGE_METRIC);
      api.metric = info->attr->med;

      distance = bgp_distance_apply (p, info, bgp);

      if (distance)
	{
	  SET_FLAG (api.message, ZAPI_MESSAGE_DISTANCE);
	  api.distance = distance;
	}
      zapi_ipv4_add (zclient, (struct prefix_ipv4 *) p, &api);
    }
#ifdef HAVE_IPV6
  /* We have to think about a IPv6 link-local address curse. */
  if (p->family == AF_INET6)
    {
      unsigned int ifindex;
      struct in6_addr *nexthop;
      struct zapi_ipv6 api;

      ifindex = 0;
      nexthop = NULL;

      /* Only global address nexthop exists. */
      if (info->attr->mp_nexthop_len == 16)
	nexthop = &info->attr->mp_nexthop_global;
      
      /* If both global and link-local address present. */
      if (info->attr->mp_nexthop_len == 32)
	{
	  /* Workaround for Cisco's nexthop bug.  */
	  if (IN6_IS_ADDR_UNSPECIFIED (&info->attr->mp_nexthop_global)
	      && peer->su_remote->sa.sa_family == AF_INET6)
	    nexthop = &peer->su_remote->sin6.sin6_addr;
	  else
	    nexthop = &info->attr->mp_nexthop_local;

	  if (info->peer->nexthop.ifp)
	    ifindex = info->peer->nexthop.ifp->ifindex;
	}

      if (nexthop == NULL)
	return;

      if (IN6_IS_ADDR_LINKLOCAL (nexthop) && ! ifindex)
	{
	  if (info->peer->ifname)
	    ifindex = if_nametoindex (info->peer->ifname);
	  else if (info->peer->nexthop.ifp)
	    ifindex = info->peer->nexthop.ifp->ifindex;
	}

      /* Make Zebra API structure. */
      api.flags = flags;
      api.type = ZEBRA_ROUTE_BGP;
      api.message = 0;
      SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
      api.nexthop_num = 1;
      api.nexthop = &nexthop;
      SET_FLAG (api.message, ZAPI_MESSAGE_IFINDEX);
      api.ifindex_num = 1;
      api.ifindex = &ifindex;
      SET_FLAG (api.message, ZAPI_MESSAGE_METRIC);
      api.metric = info->attr->med;

      zapi_ipv6_add (zclient, (struct prefix_ipv6 *) p, &api);
    }
#endif /* HAVE_IPV6 */
}

void
bgp_zebra_withdraw (struct prefix *p, struct bgp_info *info)
{
  int flags;
  struct peer *peer;

  if (zclient->sock < 0)
    return;

  if (! zclient->redist[ZEBRA_ROUTE_BGP])
    return;

  peer = info->peer;
  flags = 0;

  if (peer_sort (peer) == BGP_PEER_IBGP)
    {
      SET_FLAG (flags, ZEBRA_FLAG_INTERNAL);
      SET_FLAG (flags, ZEBRA_FLAG_IBGP);
    }

  if (peer_sort (peer) == BGP_PEER_EBGP && peer->ttl != 1)
    SET_FLAG (flags, ZEBRA_FLAG_INTERNAL);

  if (p->family == AF_INET)
    {
      struct zapi_ipv4 api;
      struct in_addr *nexthop;

      api.flags = flags;
      nexthop = &info->attr->nexthop;

      api.type = ZEBRA_ROUTE_BGP;
      api.message = 0;
      SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
      api.nexthop_num = 1;
      api.nexthop = &nexthop;
      api.ifindex_num = 0;
      SET_FLAG (api.message, ZAPI_MESSAGE_METRIC);
      api.metric = info->attr->med;

      zapi_ipv4_delete (zclient, (struct prefix_ipv4 *) p, &api);
    }
#ifdef HAVE_IPV6
  /* We have to think about a IPv6 link-local address curse. */
  if (p->family == AF_INET6)
    {
      struct zapi_ipv6 api;
      unsigned int ifindex;
      struct in6_addr *nexthop;

      ifindex = 0;
      nexthop = NULL;

      /* Only global address nexthop exists. */
      if (info->attr->mp_nexthop_len == 16)
	nexthop = &info->attr->mp_nexthop_global;

      /* If both global and link-local address present. */
      if (info->attr->mp_nexthop_len == 32)
	{
	  nexthop = &info->attr->mp_nexthop_local;
	  if (info->peer->nexthop.ifp)
	    ifindex = info->peer->nexthop.ifp->ifindex;
	}

      if (nexthop == NULL)
	return;

      if (IN6_IS_ADDR_LINKLOCAL (nexthop) && ! ifindex)
	if (info->peer->ifname)
	  ifindex = if_nametoindex (info->peer->ifname);

      api.flags = flags;
      api.type = ZEBRA_ROUTE_BGP;
      api.message = 0;
      SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
      api.nexthop_num = 1;
      api.nexthop = &nexthop;
      SET_FLAG (api.message, ZAPI_MESSAGE_IFINDEX);
      api.ifindex_num = 1;
      api.ifindex = &ifindex;
      SET_FLAG (api.message, ZAPI_MESSAGE_METRIC);
      api.metric = info->attr->med;

      zapi_ipv6_delete (zclient, (struct prefix_ipv6 *) p, &api);
    }
#endif /* HAVE_IPV6 */
}

DEFUN (router_zebra,
       router_zebra_cmd,
       "router zebra",
       "Enable a routing process\n"
       "Make connection to zebra daemon\n")
{
  vty->node = ZEBRA_NODE;
  zclient->enable = 1;
  zclient_start (zclient);
  return CMD_SUCCESS;
}

DEFUN (no_router_zebra,
       no_router_zebra_cmd,
       "no router zebra",
       NO_STR
       "Configure routing process\n"
       "Disable connection to zebra daemon\n")
{
  zclient->enable = 0;
  zclient_stop (zclient);
  return CMD_SUCCESS;
}

DEFUN (redistribute_bgp,
       redistribute_bgp_cmd,
       "redistribute bgp",
       "Redistribute control\n"
       "BGP route\n")
{
  zclient->redist[ZEBRA_ROUTE_BGP] = 1;
  return CMD_SUCCESS;
}

DEFUN (no_redistribute_bgp,
       no_redistribute_bgp_cmd,
       "no redistribute bgp",
       NO_STR
       "Redistribute control\n"
       "BGP route\n")
{
  zclient->redist[ZEBRA_ROUTE_BGP] = 0;
  return CMD_SUCCESS;
}

/* Zebra configuration write function. */
int
zebra_config_write (struct vty *vty)
{
  if (! zclient->enable)
    {
      vty_out (vty, "no router zebra%s", VTY_NEWLINE);
      return 1;
    }
  else if (! zclient->redist[ZEBRA_ROUTE_BGP])
    {
      vty_out (vty, "router zebra%s", VTY_NEWLINE);
      vty_out (vty, " no redistribute bgp%s", VTY_NEWLINE);
      return 1;
    }
  return 0;
}

/* Redistribute configuration. */
int
bgp_config_write_redistribute (struct vty *vty, struct bgp *bgp, afi_t afi,
			       safi_t safi, int *write)
{
  int i;
  char *str[] = { "system", "kernel", "connected", "static", "rip",
		  "ripng", "ospf", "ospf6", "bgp"};

  if (safi != SAFI_UNICAST)
    return 0;

  for (i = 0; i < ZEBRA_ROUTE_MAX; i++)
    {
      if (i != ZEBRA_ROUTE_BGP && bgp->redist[afi][i])
	{
	  /* "address-family" display.  */
	  bgp_config_write_family_header (vty, afi, safi, write);

	  if (bgp->rmap[afi][i].name)
	    vty_out (vty, "%sredistribute %s route-map %s%s",
		     afi == AFI_IP ? " " : "  ",
		     str[i], bgp->rmap[afi][i].name, VTY_NEWLINE);
	  else
	    vty_out (vty, "%sredistribute %s%s",
		     afi == AFI_IP ? " " : "  ",
		     str[i], VTY_NEWLINE);
	}
    }
  return *write;
}

/* Zebra node structure. */
struct cmd_node zebra_node =
{
  ZEBRA_NODE,
  "%s(config-router)# ",
};

void
bgp_zclient_reset ()
{
  zclient_reset (zclient);
}

void
zebra_init (int enable)
{
  /* Set default values. */
  zclient = zclient_new ();
  zclient_init (zclient, ZEBRA_ROUTE_BGP);
  zclient->interface_add = bgp_interface_add;
  zclient->interface_delete = bgp_interface_delete;
  zclient->interface_address_add = bgp_interface_address_add;
  zclient->interface_address_delete = bgp_interface_address_delete;
  zclient->ipv4_route_add = zebra_read_ipv4;
  zclient->ipv4_route_delete = zebra_read_ipv4;
  zclient->interface_up = bgp_interface_up;
  zclient->interface_down = bgp_interface_down;
#ifdef HAVE_IPV6
  zclient->ipv6_route_add = zebra_read_ipv6;
  zclient->ipv6_route_delete = zebra_read_ipv6;
#endif /* HAVE_IPV6 */

  /* Install zebra node. */
  install_node (&zebra_node, zebra_config_write);

  install_element (CONFIG_NODE, &router_zebra_cmd);
  install_element (CONFIG_NODE, &no_router_zebra_cmd);
  install_default (ZEBRA_NODE);
  install_element (ZEBRA_NODE, &redistribute_bgp_cmd);
  install_element (ZEBRA_NODE, &no_redistribute_bgp_cmd);

  install_element (BGP_NODE, &bgp_redistribute_kernel_cmd);
  install_element (BGP_NODE, &bgp_redistribute_kernel_routemap_cmd);
  install_element (BGP_NODE, &no_bgp_redistribute_kernel_cmd);
  install_element (BGP_NODE, &no_bgp_redistribute_kernel_routemap_cmd);
  install_element (BGP_NODE, &bgp_redistribute_static_cmd);
  install_element (BGP_NODE, &bgp_redistribute_static_routemap_cmd);
  install_element (BGP_NODE, &no_bgp_redistribute_static_cmd);
  install_element (BGP_NODE, &no_bgp_redistribute_static_routemap_cmd);
  install_element (BGP_NODE, &bgp_redistribute_connected_cmd);
  install_element (BGP_NODE, &bgp_redistribute_connected_routemap_cmd);
  install_element (BGP_NODE, &no_bgp_redistribute_connected_cmd);
  install_element (BGP_NODE, &no_bgp_redistribute_connected_routemap_cmd);
  install_element (BGP_NODE, &bgp_redistribute_rip_cmd);
  install_element (BGP_NODE, &bgp_redistribute_rip_routemap_cmd);
  install_element (BGP_NODE, &no_bgp_redistribute_rip_cmd);
  install_element (BGP_NODE, &no_bgp_redistribute_rip_routemap_cmd);
  install_element (BGP_NODE, &bgp_redistribute_ospf_cmd);
  install_element (BGP_NODE, &bgp_redistribute_ospf_routemap_cmd);
  install_element (BGP_NODE, &no_bgp_redistribute_ospf_cmd);
  install_element (BGP_NODE, &no_bgp_redistribute_ospf_routemap_cmd);

#ifdef HAVE_IPV6
  install_element (BGP_IPV6_NODE, &ipv6_bgp_redistribute_kernel_cmd);
  install_element (BGP_IPV6_NODE, &ipv6_bgp_redistribute_kernel_routemap_cmd);
  install_element (BGP_IPV6_NODE, &no_ipv6_bgp_redistribute_kernel_cmd);
  install_element (BGP_IPV6_NODE, &no_ipv6_bgp_redistribute_kernel_routemap_cmd);
  install_element (BGP_IPV6_NODE, &ipv6_bgp_redistribute_static_cmd);
  install_element (BGP_IPV6_NODE, &ipv6_bgp_redistribute_static_routemap_cmd);
  install_element (BGP_IPV6_NODE, &no_ipv6_bgp_redistribute_static_cmd);
  install_element (BGP_IPV6_NODE, &no_ipv6_bgp_redistribute_static_routemap_cmd);
  install_element (BGP_IPV6_NODE, &ipv6_bgp_redistribute_connected_cmd);
  install_element (BGP_IPV6_NODE, &ipv6_bgp_redistribute_connected_routemap_cmd);
  install_element (BGP_IPV6_NODE, &no_ipv6_bgp_redistribute_connected_cmd);
  install_element (BGP_IPV6_NODE, &no_ipv6_bgp_redistribute_connected_routemap_cmd);
  install_element (BGP_IPV6_NODE, &ipv6_bgp_redistribute_ripng_cmd);
  install_element (BGP_IPV6_NODE, &ipv6_bgp_redistribute_ripng_routemap_cmd);
  install_element (BGP_IPV6_NODE, &no_ipv6_bgp_redistribute_ripng_cmd);
  install_element (BGP_IPV6_NODE, &no_ipv6_bgp_redistribute_ripng_routemap_cmd);
  install_element (BGP_IPV6_NODE, &ipv6_bgp_redistribute_ospf6_cmd);
  install_element (BGP_IPV6_NODE, &ipv6_bgp_redistribute_ospf6_routemap_cmd);
  install_element (BGP_IPV6_NODE, &no_ipv6_bgp_redistribute_ospf6_cmd);
  install_element (BGP_IPV6_NODE, &no_ipv6_bgp_redistribute_ospf6_routemap_cmd);

  install_element (BGP_NODE, &old_ipv6_bgp_redistribute_kernel_cmd);
  install_element (BGP_NODE, &old_ipv6_bgp_redistribute_kernel_routemap_cmd);
  install_element (BGP_NODE, &old_no_ipv6_bgp_redistribute_kernel_cmd);
  install_element (BGP_NODE, &old_no_ipv6_bgp_redistribute_kernel_routemap_cmd);
  install_element (BGP_NODE, &old_ipv6_bgp_redistribute_static_cmd);
  install_element (BGP_NODE, &old_ipv6_bgp_redistribute_static_routemap_cmd);
  install_element (BGP_NODE, &old_no_ipv6_bgp_redistribute_static_cmd);
  install_element (BGP_NODE, &old_no_ipv6_bgp_redistribute_static_routemap_cmd);
  install_element (BGP_NODE, &old_ipv6_bgp_redistribute_connected_cmd);
  install_element (BGP_NODE, &old_ipv6_bgp_redistribute_connected_routemap_cmd);
  install_element (BGP_NODE, &old_no_ipv6_bgp_redistribute_connected_cmd);
  install_element (BGP_NODE, &old_no_ipv6_bgp_redistribute_connected_routemap_cmd);
  install_element (BGP_NODE, &old_ipv6_bgp_redistribute_ripng_cmd);
  install_element (BGP_NODE, &old_ipv6_bgp_redistribute_ripng_routemap_cmd);
  install_element (BGP_NODE, &old_no_ipv6_bgp_redistribute_ripng_cmd);
  install_element (BGP_NODE, &old_no_ipv6_bgp_redistribute_ripng_routemap_cmd);
  install_element (BGP_NODE, &old_ipv6_bgp_redistribute_ospf6_cmd);
  install_element (BGP_NODE, &old_ipv6_bgp_redistribute_ospf6_routemap_cmd);
  install_element (BGP_NODE, &old_no_ipv6_bgp_redistribute_ospf6_cmd);
  install_element (BGP_NODE, &old_no_ipv6_bgp_redistribute_ospf6_routemap_cmd);
#endif /* HAVE_IPV6 */

  /* Interface related init. */
  if_init ();
}
