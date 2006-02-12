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

#include "ospf6d.h"

#include "ospf6_interface.h"
#include "ospf6_redistribute.h"

/* information about zebra. */
struct zclient *zclient = NULL;

/* redistribute function */
void
ospf6_zebra_redistribute (int type)
{
  if (zclient->redist[type])
    return;

  zclient->redist[type] = 1;

  if (zclient->sock > 0)
    zebra_redistribute_send (ZEBRA_REDISTRIBUTE_ADD, zclient->sock, type);
}

void
ospf6_zebra_no_redistribute (int type)
{
  if (!zclient->redist[type])
    return;

  zclient->redist[type] = 0;

  if (zclient->sock > 0)
    zebra_redistribute_send (ZEBRA_REDISTRIBUTE_DELETE, zclient->sock, type);
}

int
ospf6_zebra_is_redistribute (int type)
{
  return zclient->redist[type];
}


/* Inteface addition message from zebra. */
int
ospf6_zebra_if_add (int command, struct zclient *zclient, zebra_size_t length)
{
  struct interface *ifp;

  ifp = zebra_interface_add_read (zclient->ibuf);

  /* log */
  if (IS_OSPF6_DUMP_ZEBRA)
#if 0
    zlog_info ("Zebra I/F add: %s index %d flags %d metric %d mtu %d",
               ifp->name, ifp->ifindex, ifp->flags, ifp->metric, ifp->mtu);
#else
    zlog_info ("Zebra I/F add: %s index %d mtu %d",
               ifp->name, ifp->ifindex, ifp->mtu);
#endif

  ospf6_interface_if_add (ifp);

  return 0;
}

int
ospf6_zebra_if_del (int command, struct zclient *zclient, zebra_size_t length)
{
#if 0
  struct interface *ifp;

  ifp = zebra_interface_delete_read (zclient->ibuf);

  /* log */
  if (IS_OSPF6_DUMP_ZEBRA)
#if 0
    zlog_info ("Zebra I/F delete: %s index %d flags %d metric %d mtu %d",
               ifp->name, ifp->ifindex, ifp->flags, ifp->metric, ifp->mtu);
#else
    zlog_info ("Zebra I/F delete: %s index %d mtu %d",
               ifp->name, ifp->ifindex, ifp->mtu);
#endif

  ospf6_interface_if_del (ifp);
#endif

  return 0;
}

int
ospf6_zebra_if_state_update (int command, struct zclient *zclient,
                             zebra_size_t length)
{
  struct interface *ifp;

  ifp = zebra_interface_state_read (zclient->ibuf);

  /* log */
  if (IS_OSPF6_DUMP_ZEBRA)
    zlog_info ("Zebra I/F %s state change: index %d flags %ld metric %d mtu %d",
               ifp->name, ifp->ifindex, ifp->flags, ifp->metric, ifp->mtu);

  ospf6_interface_state_update (ifp);
  return 0;
}

int
ospf6_zebra_if_address_update_add (int command, struct zclient *zclient,
                               zebra_size_t length)
{
  struct connected *c;
  char buf[128];

  c = zebra_interface_address_add_read (zclient->ibuf);
  if (c == NULL)
    return 0;

  if (IS_OSPF6_DUMP_ZEBRA)
    {
      zlog_info ("Zebra I/F %s address change: %s %s/%d",
                 c->ifp->name, prefix_family_str (c->address),
                 inet_ntop (c->address->family, &c->address->u.prefix,
                            buf, sizeof (buf)), c->address->prefixlen);
    }

  if (c->address->family == AF_INET6)
    ospf6_interface_address_update (c->ifp);

  return 0;
}

int
ospf6_zebra_if_address_update_delete (int command, struct zclient *zclient,
                               zebra_size_t length)
{
  struct connected *c;
  char buf[128];

  c = zebra_interface_address_delete_read (zclient->ibuf);
  if (c == NULL)
    return 0;

  if (IS_OSPF6_DUMP_ZEBRA)
    {
      zlog_info ("Zebra I/F %s address change: %s %s/%d",
                 c->ifp->name, prefix_family_str (c->address),
                 inet_ntop (c->address->family, &c->address->u.prefix,
                            buf, sizeof (buf)), c->address->prefixlen);
    }

  if (c->address->family == AF_INET6)
    ospf6_interface_address_update (c->ifp);

  return 0;
}



const char *ospf6_external_route_type[] =
{
  "System",
  "Kernel",
  "Connect",
  "Static",
  "RIP",
  "RIPng",
  "OSPF",
  "OSPF6",
  "BGP",
  NULL,
};

int
ospf6_zebra_read_ipv6 (int command, struct zclient *zclient,
                       zebra_size_t length)
{
  struct stream *s;
  struct zapi_ipv6 api;
  unsigned long ifindex;
  struct in6_addr nexthop;
  struct prefix_ipv6 p;
  char prefixstr[128], nexthopstr[128];

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

  /* log */
  if (IS_OSPF6_DUMP_ZEBRA)
    {
      prefix2str ((struct prefix *)&p, prefixstr, sizeof (prefixstr));
      inet_ntop (AF_INET6, &nexthop, nexthopstr, sizeof (nexthopstr));

      if (command == ZEBRA_IPV6_ROUTE_ADD)
	zlog_info ("Zebra Receive add %s route: %s nexthop:%s ifindex:%ld",
		   ospf6_external_route_type [api.type], prefixstr,
		   nexthopstr, ifindex);
      else
	zlog_info ("Zebra Receive remove %s route: %s nexthop:%s ifindex:%ld",
		   ospf6_external_route_type [api.type], prefixstr,
		   nexthopstr, ifindex);
    }
  
  if (command == ZEBRA_IPV6_ROUTE_ADD)
    ospf6_redistribute_route_add (api.type, ifindex, &p);
  else
    ospf6_redistribute_route_remove (api.type, ifindex, &p);

  return 0;
}


DEFUN (show_zebra,
       show_zebra_cmd,
       "show zebra",
       SHOW_STR
       "Zebra information\n")
{
  int i;
  if (!zclient)
    vty_out (vty, "Not connected to zebra%s", VTY_NEWLINE);

  vty_out (vty, "Zebra Infomation%s", VTY_NEWLINE);
  vty_out (vty, "  enable: %d%s", zclient->enable, VTY_NEWLINE);
  vty_out (vty, "  fail: %d%s", zclient->fail, VTY_NEWLINE);
  vty_out (vty, "  redistribute default: %d%s", zclient->redist_default,
           VTY_NEWLINE);
  for (i = 0; i < ZEBRA_ROUTE_MAX; i++)
    vty_out (vty, "    RouteType: %s - %s%s", ospf6_external_route_type[i],
             zclient->redist[i] ? "redistributed" : "not redistributed",
             VTY_NEWLINE);
  return CMD_SUCCESS;
}

DEFUN (router_zebra,
       router_zebra_cmd,
       "router zebra",
       "Enable a routing process\n"
       "Make connection to zebra daemon\n")
{
  if (IS_OSPF6_DUMP_CONFIG)
    zlog_info ("Config: router zebra");

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
  if (IS_OSPF6_DUMP_CONFIG)
    zlog_info ("no router zebra");

  zclient->enable = 0;
  zclient_stop (zclient);
  return CMD_SUCCESS;
}

/* Zebra configuration write function. */
int
ospf6_zebra_config_write (struct vty *vty)
{
  if (! zclient->enable)
    {
      vty_out (vty, "no router zebra%s", VTY_NEWLINE);
      return 1;
    }
  else if (! zclient->redist[ZEBRA_ROUTE_OSPF6])
    {
      vty_out (vty, "router zebra%s", VTY_NEWLINE);
      vty_out (vty, " no redistribute ospf6%s", VTY_NEWLINE);
      return 1;
    }
  return 0;
}

/* Zebra node structure. */
struct cmd_node zebra_node =
{
  ZEBRA_NODE,
  "%s(config-zebra)# ",
};

static void
ospf6_zebra_route_update_add (struct prefix_ipv6 *p,
                              struct ospf6_route_info *ri)
{
  struct zapi_ipv6 api;
  listnode node;
  struct ospf6_nexthop *nexthop;
  struct in6_addr **nexthop_list;
  unsigned int *ifindex_list;
  char buf[96], dstring[96], nstring[96], ifname[8];
  int size, i;

  if (zclient->sock < 0)
    return;

  if (CHECK_FLAG (ri->flag, OSPF6_ROUTE_FLAG_ZEBRA_SYNC))
    return;

  api.type = ZEBRA_ROUTE_OSPF6;
  api.flags = 0;
  api.message = 0;
  SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
  SET_FLAG (api.message, ZAPI_MESSAGE_IFINDEX);
  api.nexthop_num = listcount (ri->nexthop_list);
  api.ifindex_num = listcount (ri->nexthop_list);

  /* allocate memory for nexthop_list */
  size = sizeof (struct ospf6_nexthop *) * listcount (ri->nexthop_list);
  nexthop_list = (struct in6_addr **) XMALLOC (MTYPE_OSPF6_OTHER, size);
  if (! nexthop_list)
    {
      zlog_err ("Can't add zebra route: nexthop_list malloc failed");
      return;
    }

  /* allocate memory for ifindex_list */
  size = sizeof (unsigned int) * listcount (ri->nexthop_list);
  ifindex_list = (unsigned int *) XMALLOC (MTYPE_OSPF6_OTHER, size);
  if (! ifindex_list)
    {
      zlog_err ("Can't add zebra route: ifindex_list malloc failed");
      XFREE (MTYPE_OSPF6_OTHER, nexthop_list);
      return;
    }

  /* for each nexthop */
  i = 0;
  for (node = listhead (ri->nexthop_list); node; nextnode (node))
    {
      nexthop = (struct ospf6_nexthop *) getdata (node);

      prefix2str ((struct prefix *) p, dstring, sizeof (dstring));
      inet_ntop (AF_INET6, &nexthop->ipaddr, buf, sizeof (buf));
      if (! if_indextoname (nexthop->ifindex, ifname))
        snprintf (ifname, sizeof (ifname), "%d", nexthop->ifindex);
      snprintf (nstring, sizeof (nstring), "%s%%%s", buf, ifname);

      if (! IN6_IS_ADDR_LINKLOCAL (&nexthop->ipaddr) &&
          ! IN6_IS_ADDR_UNSPECIFIED (&nexthop->ipaddr))
        {
          api.nexthop_num--;
          api.ifindex_num--;
          if (IS_OSPF6_DUMP_ZEBRA)
            zlog_info ("ZEBRA: Don't send add route: %s %s",
                       dstring, nstring);
          continue;
        }

      /* log */
      if (IS_OSPF6_DUMP_ZEBRA)
        zlog_info ("ZEBRA: Send add route: %s %s", dstring, nstring);

      nexthop_list[i] = &nexthop->ipaddr;
      ifindex_list[i] = nexthop->ifindex;
      i++;
    }

  api.nexthop = nexthop_list;
  api.ifindex = ifindex_list;

  if (api.nexthop_num && api.ifindex_num)
    {
      zapi_ipv6_add (zclient, p, &api);
      SET_FLAG (ri->flag, OSPF6_ROUTE_FLAG_ZEBRA_SYNC);
    }

  XFREE (MTYPE_OSPF6_OTHER, nexthop_list);
  XFREE (MTYPE_OSPF6_OTHER, ifindex_list);
}

static void
ospf6_zebra_route_update_remove (struct prefix_ipv6 *p,
                                 struct ospf6_route_info *ri)
{
  struct zapi_ipv6 api;
  listnode node;
  struct ospf6_nexthop *nexthop;
  struct in6_addr **nexthop_list;
  unsigned int *ifindex_list;
  char buf[96], dstring[96], nstring[96], ifname[8];
  int i, size;

  if (zclient->sock < 0)
    return;

  if (! CHECK_FLAG (ri->flag, OSPF6_ROUTE_FLAG_ZEBRA_SYNC))
    return;

  api.type = ZEBRA_ROUTE_OSPF6;
  api.flags = 0;
  api.message = 0;
  SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
  SET_FLAG (api.message, ZAPI_MESSAGE_IFINDEX);
  api.nexthop_num = listcount (ri->nexthop_list);
  api.ifindex_num = listcount (ri->nexthop_list);

  /* alloc memory for nexthop_list */
  size = sizeof (struct ospf6_nexthop *) * listcount (ri->nexthop_list);
  nexthop_list = (struct in6_addr **) XMALLOC (MTYPE_OSPF6_OTHER, size);
  if (! nexthop_list)
    {
      zlog_err ("Can't remove zebra route: nexthop_list malloc failed");
      return;
    }

  /* alloc memory for ifindex_list */
  size = sizeof (unsigned int) * listcount (ri->nexthop_list);
  ifindex_list = (unsigned int *) XMALLOC (MTYPE_OSPF6_OTHER, size);
  if (! ifindex_list)
    {
      XFREE (MTYPE_OSPF6_OTHER, nexthop_list);
      zlog_err ("Can't remove zebra route: ifindex_list malloc failed");
      return;
    }

  /* for each nexthop */
  i = 0;
  for (node = listhead (ri->nexthop_list); node; nextnode (node))
    {
      nexthop = (struct ospf6_nexthop *) getdata (node);

      prefix2str ((struct prefix *) p, dstring, sizeof (dstring));
      inet_ntop (AF_INET6, &nexthop->ipaddr, buf, sizeof (buf));
      if (! if_indextoname (nexthop->ifindex, ifname))
        snprintf (ifname, sizeof (ifname), "%d", nexthop->ifindex);
      snprintf (nstring, sizeof (nstring), "%s%%%s", buf, ifname);

      if (! IN6_IS_ADDR_LINKLOCAL (&nexthop->ipaddr) &&
          ! IN6_IS_ADDR_UNSPECIFIED (&nexthop->ipaddr))
        {
          api.nexthop_num--;
          api.ifindex_num--;
          if (IS_OSPF6_DUMP_ZEBRA)
            zlog_info ("ZEBRA: Don't send remove route: %s %s",
                       dstring, nstring);
          continue;
        }

      /* log */
      if (IS_OSPF6_DUMP_ZEBRA)
        zlog_info ("ZEBRA: Send remove route: %s %s", dstring, nstring);

      nexthop_list[i] = &nexthop->ipaddr;
      ifindex_list[i] = nexthop->ifindex;
      i++;
    }

  api.nexthop = nexthop_list;
  api.ifindex = ifindex_list;

  if (api.nexthop_num && api.ifindex_num)
    {
      zapi_ipv6_delete (zclient, p, &api);
      UNSET_FLAG (ri->flag, OSPF6_ROUTE_FLAG_ZEBRA_SYNC);
    }

  XFREE (MTYPE_OSPF6_OTHER, nexthop_list);
  XFREE (MTYPE_OSPF6_OTHER, ifindex_list);
}

void
ospf6_zebra_route_update (struct prefix_ipv6 *p,
                          struct ospf6_route_info *ri)
{
  if (zclient->sock < 0)
    return;

  if (! zclient->redist[ZEBRA_ROUTE_OSPF6])
    return;

  if (CHECK_FLAG (ri->flag, OSPF6_ROUTE_FLAG_ACTIVE))
    ospf6_zebra_route_update_add (p, ri);
  else
    ospf6_zebra_route_update_remove (p, ri);
}

static void
ospf6_zebra_redistribute_ospf6 ()
{
  struct route_node *node;
  struct prefix_ipv6 *p;
  struct ospf6_route_info *ri = NULL;

  for (node = route_top (ospf6->route_table); node; node = route_next (node))
    {
      p = (struct prefix_ipv6 *) &node->p;
      if (! node || ! node->info)
        continue;

      if (CHECK_FLAG (ri->flag, OSPF6_ROUTE_FLAG_ZEBRA_SYNC))
        continue;

      ri = (struct ospf6_route_info *) node->info;
      ospf6_zebra_route_update_add (p, ri);
      SET_FLAG (ri->flag, OSPF6_ROUTE_FLAG_ZEBRA_SYNC);
    }
}

static void
ospf6_zebra_no_redistribute_ospf6 ()
{
  struct route_node *node;
  struct prefix_ipv6 *p;
  struct ospf6_route_info *ri;

  for (node = route_top (ospf6->route_table); node; node = route_next (node))
    {
      p = (struct prefix_ipv6 *) &node->p;
      if (! node || ! node->info)
        continue;

      ri = (struct ospf6_route_info *) node->info;
      ospf6_zebra_route_update_remove (p, ri);
      UNSET_FLAG (ri->flag, OSPF6_ROUTE_FLAG_ZEBRA_SYNC);
    }
}


DEFUN (redistribute_ospf6,
       redistribute_ospf6_cmd,
       "redistribute ospf6",
       "Redistribute control\n"
       "OSPF6 route\n")
{
  /* log */
  if (IS_OSPF6_DUMP_CONFIG)
    zlog_info ("Config: redistribute ospf6");

  zclient->redist[ZEBRA_ROUTE_OSPF6] = 1;

  /* set zebra route table */
  ospf6_zebra_redistribute_ospf6 ();

  return CMD_SUCCESS;
}

DEFUN (no_redistribute_ospf6,
       no_redistribute_ospf6_cmd,
       "no redistribute ospf6",
       NO_STR
       "Redistribute control\n"
       "OSPF6 route\n")
{
  /* log */
  if (IS_OSPF6_DUMP_CONFIG)
    zlog_info ("Config: no redistribute ospf6");

  zclient->redist[ZEBRA_ROUTE_OSPF6] = 0;

  /* clean up zebra route table */
  ospf6_zebra_no_redistribute_ospf6 ();

  return CMD_SUCCESS;
}

void
ospf6_zebra_init ()
{
  /* Allocate zebra structure. */
  zclient = zclient_new ();
  zclient_init (zclient, ZEBRA_ROUTE_OSPF6);
  zclient->interface_add = ospf6_zebra_if_add;
  zclient->interface_delete = ospf6_zebra_if_del;
  zclient->interface_up = ospf6_zebra_if_state_update;
  zclient->interface_down = ospf6_zebra_if_state_update;
  zclient->interface_address_add = ospf6_zebra_if_address_update_add;
  zclient->interface_address_delete = ospf6_zebra_if_address_update_delete;
  zclient->ipv4_route_add = NULL;
  zclient->ipv4_route_delete = NULL;
  zclient->ipv6_route_add = ospf6_zebra_read_ipv6;
  zclient->ipv6_route_delete = ospf6_zebra_read_ipv6;

  /* redistribute connected route by default */
  /* ospf6_zebra_redistribute (ZEBRA_ROUTE_CONNECT); */

  /* Install zebra node. */
  install_node (&zebra_node, ospf6_zebra_config_write);

  /* Install command element for zebra node. */
  install_element (VIEW_NODE, &show_zebra_cmd);
  install_element (ENABLE_NODE, &show_zebra_cmd);
  install_element (CONFIG_NODE, &router_zebra_cmd);
  install_element (CONFIG_NODE, &no_router_zebra_cmd);
  install_default (ZEBRA_NODE);
  install_element (ZEBRA_NODE, &redistribute_ospf6_cmd);
  install_element (ZEBRA_NODE, &no_redistribute_ospf6_cmd);

  return;
}

void
ospf6_zebra_finish ()
{
  zclient_stop (zclient);
  zclient_free (zclient);
  zclient = (struct zclient *) NULL;
}

