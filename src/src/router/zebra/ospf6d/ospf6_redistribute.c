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

#include <zebra.h>

#include "log.h"
#include "memory.h"
#include "vty.h"
#include "prefix.h"
#include "table.h"
#include "linklist.h"
#include "routemap.h"
#include "command.h"

#include "ospf6_top.h"
#include "ospf6_redistribute.h"
#include "ospf6_dump.h"
#include "ospf6_prefix.h"
#include "ospf6_lsa.h"
#include "ospf6_lsdb.h"

#include "ospf6_route.h"
#include "ospf6_zebra.h"

#include "ospf6_message.h"
#include "ospf6_neighbor.h"
#include "ospf6_interface.h"

/* xxx */
extern struct ospf6 *ospf6;

void
ospf6_redistribute_routemap_set (struct ospf6 *o6, int type, char *mapname)
{
  if (o6->rmap[type].name)
    free (o6->rmap[type].name);

  o6->rmap[type].name = strdup (mapname);
  o6->rmap[type].map = route_map_lookup_by_name (mapname);
}

void
ospf6_redistribute_routemap_update ()
{
  struct ospf6 *o6 = ospf6;
  int i;

  for (i = 0; i < ZEBRA_ROUTE_MAX; i++)
    {
      if (o6->rmap[i].name)
        o6->rmap[i].map = route_map_lookup_by_name (o6->rmap[i].name);
      else
        o6->rmap[i].map = NULL;
    }
}

void
ospf6_redistribute_routemap_unset (struct ospf6 *o6, int type)
{
  if (o6->rmap[type].name)
    free (o6->rmap[type].name);

  o6->rmap[type].name = NULL;
  o6->rmap[type].map = NULL;
}

static u_int32_t
ospf6_redistribute_get_id (struct prefix_ipv6 *p)
{
  u_int32_t id;
  struct ospf6_lsa *lsa;
  struct ospf6_as_external_lsa *external_lsa;
  struct prefix_ipv6 prefix6;

  /* Start redistributing from LSID 1 */
  id = 1;

  while (1)
    {
      lsa = ospf6_lsdb_lookup (htons (OSPF6_LSA_TYPE_AS_EXTERNAL),
                               htonl (id), ospf6->router_id);

      if (! lsa)
        break;

      external_lsa = (struct ospf6_as_external_lsa *) (lsa->lsa_hdr + 1);
      prefix6.family = AF_INET6;
      prefix6.prefixlen = external_lsa->ospf6_prefix.prefix_length;
      ospf6_prefix_in6_addr (&external_lsa->ospf6_prefix, &prefix6.prefix);

      if (prefix_same ((struct prefix *) &prefix6, (struct prefix *) p))
        break;

      id ++;
    }

  return id;
}

void
ospf6_redistribute_route_add (int type, int ifindex, struct prefix_ipv6 *p)
{
  int ret;
  char buf[128];
  struct route_node *rn;
  struct ospf6_redistribute_info *ri;

  /* for log */
  inet_ntop (AF_INET6, &p->prefix, buf, sizeof (buf));

  /* Ignore Connected prefix of OSPF enabled Interface */
  if (type == ZEBRA_ROUTE_CONNECT)
    {
      if (ospf6_interface_is_enabled (ifindex))
        {
          if (IS_OSPF6_DUMP_REDISTRIBUTE)
            zlog_info ("Redistribute: add: ignore connect route of enabled "
                       "interface: prefix: %s/%d ifindex: %d",
                       buf, p->prefixlen, ifindex);
          return;
        }
    }

  /* set redistribute info */
  ri = XMALLOC (MTYPE_OSPF6_EXTERNAL_INFO,
                sizeof (struct ospf6_redistribute_info));
  if (!ri)
    {
      zlog_err ("Redistribute: add: Can't allocate memory for external info");
      return;
    }

  memset (ri, 0, sizeof (struct ospf6_redistribute_info));
  ri->metric_type = OSPF6_REDISTRIBUTE_DEFAULT_TYPE;
  ri->metric = OSPF6_REDISTRIBUTE_DEFAULT_METRIC;
  ri->type = type;
  ri->ifindex = ifindex;
  ri->id = ospf6_redistribute_get_id (p);

  /* test applying deny match */
  if (ospf6->rmap[type].map)
    {
      ret = route_map_apply (ospf6->rmap[type].map, (struct prefix *)p,
                             RMAP_OSPF6, ri);
      if (ret == RMAP_DENYMATCH)
        {
          XFREE (MTYPE_OSPF6_EXTERNAL_INFO, ri);
          if (IS_OSPF6_DUMP_REDISTRIBUTE)
            zlog_info ("Redistribute: add: applied deny match: "
                       "prefix: %s/%d ifindex: %d", buf, p->prefixlen, ifindex);
          return;
        }
    }

  /* log */
  if (IS_OSPF6_DUMP_REDISTRIBUTE)
    zlog_info ("Redistribute: add: type: %d index: %d prefix: %s/%d id: %d",
               type, ifindex, buf, p->prefixlen, ri->id);

  /* install new external info */
  rn = route_node_get (ospf6->external_table, (struct prefix *) p);
  if (rn->info)
    XFREE (MTYPE_OSPF6_EXTERNAL_INFO, rn->info);
  rn->info = ri;

  /* update AS-external LSA */
  ospf6_lsa_update_as_external (ri->id);
}

void
ospf6_redistribute_route_remove (int type, int ifindex, struct prefix_ipv6 *p)
{
  char buf[128];
  struct ospf6_lsa *lsa;
  struct route_node *rn;
  struct ospf6_redistribute_info *ri;

  /* for log */
  inet_ntop (AF_INET6, &p->prefix, buf, sizeof (buf));

  rn = route_node_lookup (ospf6->external_table, (struct prefix *) p);
  if (! rn)
    {
      zlog_warn ("Redistribute: remove: no such route: prefix: %s/%d",
                 buf, p->prefixlen);
      return;
    }

  ri = rn->info;
  if (! ri)
    {
      zlog_warn ("Redistribute: remove: no external info: prefix: %s/%d",
                 buf, p->prefixlen);
      return;
    }

  if (ri->type != type || ri->ifindex != ifindex)
    {
      zlog_warn ("Redistribute: remove: type or ifindex mismatch: "
                 "prefix: %s/%d", buf, p->prefixlen);
      return;
    }

  /* log */
  if (IS_OSPF6_DUMP_REDISTRIBUTE)
    zlog_info ("Redistribute: remove: type: %d index: %d prefix: %s/%d",
               type, ifindex, buf, p->prefixlen);

  lsa = ospf6_lsdb_lookup (htons (OSPF6_LSA_TYPE_AS_EXTERNAL),
                           htonl (ri->id), ospf6->router_id);

  if (lsa)
    ospf6_lsa_premature_aging (lsa);

  XFREE (MTYPE_OSPF6_EXTERNAL_INFO, ri);
  rn->info = NULL;
}

DEFUN (show_ipv6_route_ospf6_external,
       show_ipv6_route_ospf6_external_cmd,
       "show ipv6 route ospf6 external",
       SHOW_STR
       IP6_STR
       ROUTE_STR
       OSPF6_STR
       "redistributing External information\n"
       )
{
  char buf[96], pstring[96], istring[16], rstring[32], ostring[32];
  struct ospf6 *o6 = ospf6;
  struct route_node *rn;
  struct ospf6_redistribute_info *ri;

  static char *type_name[ZEBRA_ROUTE_MAX] =
    { "X", "K", "C", "S", "r", "R", "o", "O", "B" };

  vty_out (vty, "Codes: K - kernel, C - connected, S - static,"
                "       R - RIPng,  B - BGP4+ route.%s", VTY_NEWLINE);

  for (rn = route_top (o6->external_table); rn; rn = route_next (rn))
    {
      if (! rn || ! rn->info)
        continue;

      ri = (struct ospf6_redistribute_info *) rn->info;

      inet_ntop (AF_INET, &o6->router_id, rstring, sizeof (rstring));
      snprintf (ostring, sizeof (ostring), "%s[%d]", rstring, ri->id);
      inet_ntop (AF_INET6, &rn->p.u.prefix6, buf, sizeof (buf));
      snprintf (pstring, sizeof (pstring), "%s/%d", buf, rn->p.prefixlen);
      if (! if_indextoname (ri->ifindex, istring))
        snprintf (istring, sizeof (istring), "%d", ri->ifindex);

      vty_out (vty, "%s %-43s %-19s Type-%d %d %s%s",
               type_name[ri->type], pstring, ostring,
               ri->metric_type, ri->metric, istring, VTY_NEWLINE);
    }

  return CMD_SUCCESS;
}

DEFUN (ospf6_redistribute_static,
       ospf6_redistribute_static_cmd,
       "redistribute static",
       "Redistribute\n"
       "Static route\n")
{
  ospf6_zebra_redistribute (ZEBRA_ROUTE_STATIC);
  ospf6_redistribute_routemap_unset (ospf6, ZEBRA_ROUTE_STATIC);
  return CMD_SUCCESS;
}

DEFUN (ospf6_redistribute_static_routemap,
       ospf6_redistribute_static_routemap_cmd,
       "redistribute static route-map WORD",
       "Redistribute\n"
       "Static routes\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  ospf6_zebra_redistribute (ZEBRA_ROUTE_STATIC);
  ospf6_redistribute_routemap_set (ospf6, ZEBRA_ROUTE_STATIC, argv[0]);
  return CMD_SUCCESS;
}

DEFUN (no_ospf6_redistribute_static,
       no_ospf6_redistribute_static_cmd,
       "no redistribute static",
       NO_STR
       "Redistribute\n"
       "Static route\n")
{
  ospf6_zebra_no_redistribute (ZEBRA_ROUTE_STATIC);
  ospf6_redistribute_routemap_unset (ospf6, ZEBRA_ROUTE_STATIC);
  return CMD_SUCCESS;
}

DEFUN (ospf6_redistribute_kernel,
       ospf6_redistribute_kernel_cmd,
       "redistribute kernel",
       "Redistribute\n"
       "Static route\n")
{
  ospf6_zebra_redistribute (ZEBRA_ROUTE_KERNEL);
  ospf6_redistribute_routemap_unset (ospf6, ZEBRA_ROUTE_KERNEL);
  return CMD_SUCCESS;
}

DEFUN (ospf6_redistribute_kernel_routemap,
       ospf6_redistribute_kernel_routemap_cmd,
       "redistribute kernel route-map WORD",
       "Redistribute\n"
       "Static routes\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  ospf6_zebra_redistribute (ZEBRA_ROUTE_KERNEL);
  ospf6_redistribute_routemap_set (ospf6, ZEBRA_ROUTE_KERNEL, argv[0]);
  return CMD_SUCCESS;
}

DEFUN (no_ospf6_redistribute_kernel,
       no_ospf6_redistribute_kernel_cmd,
       "no redistribute kernel",
       NO_STR
       "Redistribute\n"
       "Static route\n")
{
  ospf6_zebra_no_redistribute (ZEBRA_ROUTE_KERNEL);
  ospf6_redistribute_routemap_unset (ospf6, ZEBRA_ROUTE_KERNEL);
  return CMD_SUCCESS;
}

DEFUN (ospf6_redistribute_connected,
       ospf6_redistribute_connected_cmd,
       "redistribute connected",
       "Redistribute\n"
       "Connected route\n")
{
  ospf6_zebra_redistribute (ZEBRA_ROUTE_CONNECT);
  ospf6_redistribute_routemap_unset (ospf6, ZEBRA_ROUTE_CONNECT);
  return CMD_SUCCESS;
}

DEFUN (ospf6_redistribute_connected_routemap,
       ospf6_redistribute_connected_routemap_cmd,
       "redistribute connected route-map WORD",
       "Redistribute\n"
       "Connected routes\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  ospf6_zebra_redistribute (ZEBRA_ROUTE_CONNECT);
  ospf6_redistribute_routemap_set (ospf6, ZEBRA_ROUTE_CONNECT, argv[0]);
  return CMD_SUCCESS;
}

DEFUN (no_ospf6_redistribute_connected,
       no_ospf6_redistribute_connected_cmd,
       "no redistribute connected",
       NO_STR
       "Redistribute\n"
       "Connected route\n")
{
  ospf6_zebra_no_redistribute (ZEBRA_ROUTE_CONNECT);
  ospf6_redistribute_routemap_unset (ospf6, ZEBRA_ROUTE_CONNECT);
  return CMD_SUCCESS;
}

DEFUN (ospf6_redistribute_ripng,
       ospf6_redistribute_ripng_cmd,
       "redistribute ripng",
       "Redistribute\n"
       "RIPng route\n")
{
  ospf6_zebra_redistribute (ZEBRA_ROUTE_RIPNG);
  ospf6_redistribute_routemap_unset (ospf6, ZEBRA_ROUTE_RIPNG);
  return CMD_SUCCESS;
}

DEFUN (ospf6_redistribute_ripng_routemap,
       ospf6_redistribute_ripng_routemap_cmd,
       "redistribute ripng route-map WORD",
       "Redistribute\n"
       "RIPng routes\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  ospf6_zebra_redistribute (ZEBRA_ROUTE_RIPNG);
  ospf6_redistribute_routemap_set (ospf6, ZEBRA_ROUTE_RIPNG, argv[0]);
  return CMD_SUCCESS;
}

DEFUN (no_ospf6_redistribute_ripng,
       no_ospf6_redistribute_ripng_cmd,
       "no redistribute ripng",
       NO_STR
       "Redistribute\n"
       "RIPng route\n")
{
  ospf6_zebra_no_redistribute (ZEBRA_ROUTE_RIPNG);
  ospf6_redistribute_routemap_unset (ospf6, ZEBRA_ROUTE_RIPNG);
  return CMD_SUCCESS;
}

DEFUN (ospf6_redistribute_bgp,
       ospf6_redistribute_bgp_cmd,
       "redistribute bgp",
       "Redistribute\n"
       "RIPng route\n")
{
  ospf6_zebra_redistribute (ZEBRA_ROUTE_BGP);
  ospf6_redistribute_routemap_unset (ospf6, ZEBRA_ROUTE_BGP);
  return CMD_SUCCESS;
}

DEFUN (ospf6_redistribute_bgp_routemap,
       ospf6_redistribute_bgp_routemap_cmd,
       "redistribute bgp route-map WORD",
       "Redistribute\n"
       "BGP routes\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  ospf6_zebra_redistribute (ZEBRA_ROUTE_BGP);
  ospf6_redistribute_routemap_set (ospf6, ZEBRA_ROUTE_BGP, argv[0]);
  return CMD_SUCCESS;
}

DEFUN (no_ospf6_redistribute_bgp,
       no_ospf6_redistribute_bgp_cmd,
       "no redistribute bgp",
       NO_STR
       "Redistribute\n"
       "RIPng route\n")
{
  ospf6_zebra_no_redistribute (ZEBRA_ROUTE_BGP);
  ospf6_redistribute_routemap_unset (ospf6, ZEBRA_ROUTE_BGP);
  return CMD_SUCCESS;
}

char *zebra_route_string[] = { "system", "kernel", "connected", "static",
                               "rip", "ripng", "ospf", "ospf6", "bgp" };
int
ospf6_redistribute_config_write (struct vty *vty)
{
  int i;

  for (i = 0; i < ZEBRA_ROUTE_MAX; i++)
    {
      if (i == ZEBRA_ROUTE_OSPF6)
        continue;

      if (ospf6_zebra_is_redistribute (i) == 0)
        continue;

      if (ospf6->rmap[i].map)
        vty_out (vty, " redistribute %s route-map %s%s",
                 zebra_route_string[i], ospf6->rmap[i].name, VTY_NEWLINE);
      else
        vty_out (vty, " redistribute %s%s",
                 zebra_route_string[i], VTY_NEWLINE);
    }

  return 0;
}

void
ospf6_redistribute_show_config (struct vty *vty, struct ospf6 *o6)
{
  int i;

  if (ospf6_zebra_is_redistribute(ZEBRA_ROUTE_SYSTEM) ||
      ospf6_zebra_is_redistribute(ZEBRA_ROUTE_KERNEL) ||
      ospf6_zebra_is_redistribute(ZEBRA_ROUTE_STATIC) ||
      ospf6_zebra_is_redistribute(ZEBRA_ROUTE_RIPNG) ||
      ospf6_zebra_is_redistribute(ZEBRA_ROUTE_BGP))
    vty_out (vty, " Redistributing External Routes from,%s", VTY_NEWLINE);
  else
    return;

  for (i = 0; i < ZEBRA_ROUTE_MAX; i++)
    {
      if (i == ZEBRA_ROUTE_OSPF6)
        continue;

      if (ospf6_zebra_is_redistribute (i))
        {
          if (o6->rmap[i].map)
            vty_out (vty, "    %s with route-map %s%s",
                     zebra_route_string[i], o6->rmap[i].name,
                     VTY_NEWLINE);
          else
            vty_out (vty, "    %s%s", zebra_route_string[i], VTY_NEWLINE);
        }
    }
}

void
ospf6_redistribute_init (struct ospf6 *o6)
{
  install_element (VIEW_NODE, &show_ipv6_route_ospf6_external_cmd);
  install_element (ENABLE_NODE, &show_ipv6_route_ospf6_external_cmd);

  install_element (OSPF6_NODE, &ospf6_redistribute_static_cmd);
  install_element (OSPF6_NODE, &ospf6_redistribute_static_routemap_cmd);
  install_element (OSPF6_NODE, &no_ospf6_redistribute_static_cmd);
  install_element (OSPF6_NODE, &ospf6_redistribute_kernel_cmd);
  install_element (OSPF6_NODE, &ospf6_redistribute_kernel_routemap_cmd);
  install_element (OSPF6_NODE, &no_ospf6_redistribute_kernel_cmd);
  install_element (OSPF6_NODE, &ospf6_redistribute_connected_cmd);
  install_element (OSPF6_NODE, &ospf6_redistribute_connected_routemap_cmd);
  install_element (OSPF6_NODE, &no_ospf6_redistribute_connected_cmd);
  install_element (OSPF6_NODE, &ospf6_redistribute_ripng_cmd);
  install_element (OSPF6_NODE, &ospf6_redistribute_ripng_routemap_cmd);
  install_element (OSPF6_NODE, &no_ospf6_redistribute_ripng_cmd);
  install_element (OSPF6_NODE, &ospf6_redistribute_bgp_cmd);
  install_element (OSPF6_NODE, &ospf6_redistribute_bgp_routemap_cmd);
  install_element (OSPF6_NODE, &no_ospf6_redistribute_bgp_cmd);
}

void
ospf6_redistribute_finish (struct ospf6 *o6)
{
  struct route_node *rn;
  struct ospf6_redistribute_info *ri;

  for (rn = route_top (o6->external_table); rn; rn = route_next (rn))
    {
      ri = rn->info;
      if (ri)
        ospf6_redistribute_route_remove (ri->type, ri->ifindex,
                                         (struct prefix_ipv6 *) &rn->p);
    }

  route_table_finish (o6->external_table);
}

