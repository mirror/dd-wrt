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

/* global ospf6d variable */
int  ospf6_sock;
struct ospf6 *ospf6;
list iflist;
list nexthoplist = NULL;
struct sockaddr_in6 allspfrouters6;
struct sockaddr_in6 alldrouters6;
char *recent_reason; /* set by ospf6_lsa_check_recent () */
int proctitle_mode = 0;

char ospf6_daemon_version[] = OSPF6_DAEMON_VERSION;


/* timeval calculation */
void
ospf6_timeval_add (const struct timeval *t1, const struct timeval *t2,
                   struct timeval *result)
{
  long moveup = 0;

  result->tv_usec = t1->tv_usec + t2->tv_usec;
  while (result->tv_usec > 1000000)
    {
      result->tv_usec -= 1000000;
      moveup ++;
    }

  result->tv_sec = t1->tv_sec + t2->tv_sec + moveup;
}

void
ospf6_timeval_sub (const struct timeval *t1, const struct timeval *t2,
                   struct timeval *result)
{
  long usec, movedown = 0;

  if (t1->tv_sec < t2->tv_sec ||
      (t1->tv_sec == t2->tv_sec && t1->tv_usec < t2->tv_usec))
    {
      result->tv_sec = 0;
      result->tv_usec = 0;
      return;
    }

  if (t1->tv_usec < t2->tv_usec)
    {
      usec = t1->tv_usec + 1000000;
      movedown++;
    }
  else
    usec = t1->tv_usec;
  result->tv_usec = usec - t2->tv_usec;

  result->tv_sec = t1->tv_sec - t2->tv_sec - movedown;
}

void
ospf6_timeval_div (const struct timeval *t1, u_int by,
                   struct timeval *result)
{
  long movedown;

  if (by == 0)
    {
      result->tv_sec = 0;
      result->tv_usec = 0;
      return;
    }

  movedown = t1->tv_sec % by;
  result->tv_sec = t1->tv_sec / by;
  result->tv_usec = (t1->tv_usec + movedown * 1000000) / by;
}

/* return -1 if t1 greater, return 1 if t2 greater.
   return 0 if t1 == t2 */
int
ospf6_timeval_cmp (const struct timeval *t1, const struct timeval *t2)
{
  if (t1->tv_sec > t2->tv_sec)
    return -1;
  if (t1->tv_sec < t2->tv_sec)
    return 1;
  if (t1->tv_usec > t2->tv_usec)
    return -1;
  if (t1->tv_usec < t2->tv_usec)
    return 1;
  return 0;
}

void
ospf6_timeval_add_equal (const struct timeval *t, struct timeval *result)
{
  struct timeval tmp;
  ospf6_timeval_add (t, result, &tmp);
  result->tv_sec = tmp.tv_sec;
  result->tv_usec = tmp.tv_usec;
}

void
ospf6_timeval_sub_equal (const struct timeval *t, struct timeval *result)
{
  struct timeval tmp;
  ospf6_timeval_sub (result, t, &tmp);
  result->tv_sec = tmp.tv_sec;
  result->tv_usec = tmp.tv_usec;
}

void
ospf6_timeval_decode (const struct timeval *t, long *dayp, long *hourp,
                      long *minp, long *secp, long *msecp, long *usecp)
{
  long day, hour, min, sec, msec, usec, left;

  left = t->tv_sec;
  day = left / 86400; left -= day * 86400;
  hour = left / 3600; left -= hour * 3600;
  min = left / 60; left -= min * 60;
  sec = left;
  left = t->tv_usec;
  msec = left / 1000; left -= msec * 1000;
  usec = left;

  if (dayp) *dayp = day;
  if (hourp) *hourp = hour;
  if (minp) *minp = min;
  if (secp) *secp = sec;
  if (msecp) *msecp = msec;
  if (usecp) *usecp = usec;
}

void
ospf6_timeval_string (struct timeval *tv, char *buf, int size)
{
  char days[16], hours[16], mins[16], secs[16], msecs[16], usecs[16];
  long day, hour, min, sec, msec, usec;

  ospf6_timeval_decode (tv, &day, &hour, &min, &sec, &msec, &usec);
  snprintf (days, sizeof (days), "%ld days ", day);
  snprintf (hours, sizeof (hours), "%ld hours ", hour);
  snprintf (mins, sizeof (mins), "%ld mins ", min);
  snprintf (secs, sizeof (secs), "%ld secs ", sec);
  snprintf (msecs, sizeof (msecs), "%ld msecs ", msec);
  snprintf (usecs, sizeof (usecs), "%ld usecs ", usec);

  snprintf (buf, size, "%s%s%s%s%s%s",
            (day ? days : ""), (hour ? hours : ""),
            (min ? mins : ""), (sec ? secs : ""),
            (msec ? msecs : ""), (usec ? usecs : ""));
}

/* foreach function */
void
ospf6_count_state (void *arg, int val, void *obj)
{
  int *count = (int *) arg;
  u_char state = val;
  struct ospf6_neighbor *nei = (struct ospf6_neighbor *) obj;

  if (nei->state == state)
    (*count)++;
}


/* vty commands */
/* for reload */
extern void _reload ();
DEFUN (reload,
       reload_cmd,
       "reload",
       "Reloads\n")
{
  _reload ();
  return CMD_SUCCESS;
}

DEFUN (garbage_collection,
       garbage_collection_cmd,
       "ipv6 ospf6 garbage collect",
       IPV6_STR
       OSPF6_STR
       "garbage collection by hand\n"
       "Remove Maxages if possible and recalculate routes\n")
{
  ospf6_maxage_remover ();
  ospf6_route_calculation_schedule ();
  return CMD_SUCCESS;
}

/* Show version. */
DEFUN (show_version_ospf6,
       show_version_ospf6_cmd,
       "show version ospf6",
       SHOW_STR
       "Displays ospf6d version\n")
{
  vty_out (vty, "Zebra OSPF6d Version: %s%s",
           ospf6_daemon_version, VTY_NEWLINE);

  return CMD_SUCCESS;
}

/* start ospf6 */
DEFUN (router_ospf6,
       router_ospf6_cmd,
       "router ospf6",
       OSPF6_ROUTER_STR
       OSPF6_STR
       )
{
  if (ospf6 == NULL)
    ospf6_start ();

  /* set current ospf point. */
  vty->node = OSPF6_NODE;
  vty->index = ospf6;

  return CMD_SUCCESS;
}

/* stop ospf6 */
DEFUN (no_router_ospf6,
       no_router_ospf6_cmd,
       "no router ospf6",
       NO_STR
       OSPF6_ROUTER_STR
       )
{
  if (!ospf6)
    vty_out (vty, "OSPFv3 is not running%s", VTY_NEWLINE);
  else
    ospf6_stop ();

  /* return to config node . */
  vty->node = CONFIG_NODE;
  vty->index = NULL;

  return CMD_SUCCESS;
}

/* show top level structures */
DEFUN (show_ipv6_ospf6,
       show_ipv6_ospf6_cmd,
       "show ipv6 ospf6",
       SHOW_STR
       IP6_STR
       OSPF6_STR
       )
{
  OSPF6_CMD_CHECK_RUNNING ();

  ospf6_show (vty);
  return CMD_SUCCESS;
}

DEFUN (show_ipv6_ospf6_nexthoplist,
       show_ipv6_ospf6_nexthoplist_cmd,
       "show ipv6 ospf6 nexthop-list",
       SHOW_STR
       IP6_STR
       OSPF6_STR
       "List of nexthop\n")
{
#if 0
  listnode i;
  struct ospf6_nexthop *nh;
  char buf[128];
  for (i = listhead (nexthoplist); i; nextnode (i))
    {
      nh = (struct ospf6_nexthop *) getdata (i);
      nexthop_str (nh, buf, sizeof (buf));
      vty_out (vty, "%s%s", buf,
	       VTY_NEWLINE);
    }
#endif
  return CMD_SUCCESS;
}

DEFUN (show_ipv6_ospf6_statistics,
       show_ipv6_ospf6_statistics_cmd,
       "show ipv6 ospf6 statistics",
       SHOW_STR
       IP6_STR
       OSPF6_STR
       "Statistics\n")
{
  OSPF6_CMD_CHECK_RUNNING ();

  ospf6_statistics_show (vty, ospf6);
  return CMD_SUCCESS;
}

/* change Router_ID commands. */
DEFUN (router_id,
       router_id_cmd,
       "router-id ROUTER_ID",
       "Configure ospf Router-ID.\n"
       V4NOTATION_STR)
{
  int ret;
  u_int32_t router_id;

  ret = inet_pton (AF_INET, argv[0], &router_id);
  if (!ret)
    {
      vty_out (vty, "malformed ospf router identifier%s", VTY_NEWLINE);
      vty_out (vty, "%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  ospf6->router_id = router_id;

  return CMD_SUCCESS;
}

DEFUN (interface_area,
       interface_area_cmd,
       "interface IFNAME area A.B.C.D",
       "Enable routing on an IPv6 interface\n"
       IFNAME_STR
       "Set the OSPF6 area ID\n"
       "OSPF6 area ID in IPv4 address notation\n"
       )
{
  struct ospf6 *o6;
  struct interface *ifp;
  struct ospf6_interface *o6i;
  struct ospf6_area *o6a;
  u_int32_t area_id;

  o6 = (struct ospf6 *) vty->index;

  ifp = if_get_by_name (argv[0]);

  /* find/create ospf6 interface */
  o6i = (struct ospf6_interface *) ifp->info;
  if (!o6i)
    o6i = ospf6_interface_create (ifp);

  /* parse Area-ID */
  if (inet_pton (AF_INET, argv[1], &area_id) != 1)
    {
      vty_out (vty, "Invalid Area-ID: %s%s", argv[1], VTY_NEWLINE);
      return CMD_ERR_AMBIGUOUS;
    }

  /* find/create ospf6 area */
  o6a = ospf6_area_lookup (area_id, o6);
  if (!o6a)
    {
      o6a = ospf6_area_create (area_id);
      o6a->ospf6 = o6;
      listnode_add (o6->area_list, o6a);
    }

  if (o6i && o6i->area)
    {
      if (o6i->area != o6a)
        vty_out (vty, "Aready attached to area %s%s",
                 o6i->area->str, VTY_NEWLINE);
      return CMD_ERR_NOTHING_TODO;
    }

  listnode_add (o6a->if_list, o6i);
  o6i->area = o6a;

  /* must check if got already interface info from zebra */
  if (if_is_up (ifp))
    thread_add_event (master, interface_up, o6i, 0);

  return CMD_SUCCESS;
}

DEFUN (no_interface,
       no_interface_cmd,
       "no interface IFNAME",
       NO_STR
       "Disable routing on an IPv6 interface\n"
       IFNAME_STR
       )
{
  struct interface *ifp;
  struct ospf6_interface *o6i;
  struct ospf6 *o6;

  o6 = (struct ospf6 *) vty->index;

  ifp = if_lookup_by_name (argv[0]);
  if (!ifp)
    return CMD_ERR_NO_MATCH;

  o6i = (struct ospf6_interface *) ifp->info;
  if (!o6i)
    return CMD_SUCCESS;

  if (o6i->area)
    thread_execute (master, interface_down, o6i, 0);

  listnode_delete (o6i->area->if_list, o6i);
  o6i->area = (struct ospf6_area *) NULL;

  ospf6_interface_delete (o6i);

  return CMD_SUCCESS;
}

DEFUN (area_range,
       area_range_cmd,
       "area A.B.C.D range X:X::X:X/M",
       "OSPFv3 area parameters\n"
       "OSPFv3 area ID in IPv4 address format\n"
       "Summarize routes matching address/mask (border routers only)\n"
       "IPv6 address range\n")
{
  struct ospf6 *o6;
  struct ospf6_area *o6a;
  u_int32_t area_id;
  int ret;

  o6 = (struct ospf6 *) vty->index;
  inet_pton (AF_INET, argv[0], &area_id);
  o6a = ospf6_area_lookup (area_id, o6);
  if (! o6a)
    {
      vty_out (vty, "No such area%s", VTY_NEWLINE);
      return CMD_ERR_NO_MATCH;
    }

  ret = str2prefix_ipv6 (argv[1], &o6a->area_range);
  if (ret <= 0)
    {
      vty_out (vty, "Malformed IPv6 address%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  return CMD_SUCCESS;
}

DEFUN (passive_interface,
       passive_interface_cmd,
       "passive-interface IFNAME",
       "Suppress routing updates on an interface\n"
       IFNAME_STR
       )
{
  struct interface *ifp;
  struct ospf6_interface *o6i;

  ifp = if_get_by_name (argv[0]);
  if (ifp->info)
    o6i = (struct ospf6_interface *) ifp->info;
  else
    o6i = ospf6_interface_create (ifp);

  o6i->is_passive = 1;
  if (o6i->thread_send_hello)
    {
      thread_cancel (o6i->thread_send_hello);
      o6i->thread_send_hello = (struct thread *) NULL;
    }

  return CMD_SUCCESS;
}

DEFUN (no_passive_interface,
       no_passive_interface_cmd,
       "no passive-interface IFNAME",
       NO_STR
       "Suppress routing updates on an interface\n"
       IFNAME_STR
       )
{
  struct interface *ifp;
  struct ospf6_interface *o6i;

  ifp = if_lookup_by_name (argv[0]);
  if (! ifp)
    return CMD_ERR_NO_MATCH;

  o6i = (struct ospf6_interface *) ifp->info;
  o6i->is_passive = 0;
  if (o6i->thread_send_hello == NULL)
    thread_add_event (master, ospf6_send_hello, o6i, 0);

  return CMD_SUCCESS;
}

#ifdef HAVE_SETPROCTITLE
extern int _argc;
extern char **_argv;

DEFUN (set_proctitle,
       set_proctitle_cmd,
       "set proctitle (version|normal|none)",
       "Set command\n"
       "Process title\n"
       "Version information\n"
       "Normal command-line options\n"
       "Just program name\n"
       )
{
  int i;
  char buf[64], tmp[64];

  if (strncmp (argv[0], "v", 1) == 0)
    {
      proctitle_mode = 1;
      setproctitle ("%s Zebra: %s", OSPF6_DAEMON_VERSION, ZEBRA_VERSION);
    }
  else if (strncmp (argv[0], "nor", 3) == 0)
    {
      proctitle_mode = 0;
      memset (tmp, 0, sizeof (tmp));
      memset (buf, 0, sizeof (buf));
      for (i = 0; i < _argc; i++)
        {
          snprintf (buf, sizeof (buf), "%s%s ", tmp, _argv[i]);
          memcpy (&tmp, &buf, sizeof (tmp));
        }
      setproctitle (buf);
    }
  else if (strncmp (argv[0], "non", 3) == 0)
    {
      proctitle_mode = -1;
      setproctitle (NULL);
    }
  else
    return CMD_ERR_NO_MATCH;

  return CMD_SUCCESS;
}
#endif /* HAVE_SETPROCTITLE */

/* OSPF configuration write function. */
int
ospf6_config_write (struct vty *vty)
{
  listnode j, k;
  char buf[64];
  struct ospf6_area *area;
  struct ospf6_interface *ospf6_interface;

  if (proctitle_mode == 1)
    vty_out (vty, "set proctitle version%s", VTY_NEWLINE);
  else if (proctitle_mode == -1)
    vty_out (vty, "set proctitle none%s", VTY_NEWLINE);

  vty_out (vty, "!%s", VTY_NEWLINE);

  if (! ospf6)
    return 0;

  /* OSPFv6 configuration. */
  if (!ospf6)
    return CMD_SUCCESS;

  inet_ntop (AF_INET, &ospf6->router_id, buf, sizeof (buf));
  vty_out (vty, "router ospf6%s", VTY_NEWLINE);
  vty_out (vty, " router-id %s%s", buf, VTY_NEWLINE);

  ospf6_redistribute_config_write (vty);

  for (j = listhead (ospf6->area_list); j; nextnode (j))
    {
      area = (struct ospf6_area *)getdata (j);
      for (k = listhead (area->if_list); k; nextnode (k))
        {
          ospf6_interface = (struct ospf6_interface *)getdata (k);
          vty_out (vty, " interface %s area %s%s",
                   ospf6_interface->interface->name, area->str,
                   VTY_NEWLINE);
          if (ospf6_interface->is_passive)
            vty_out (vty, " passive-interface %s%s",
                     ospf6_interface->interface->name,
                     VTY_NEWLINE);
        }
    }
  vty_out (vty, "!%s", VTY_NEWLINE);
  return 0;
}

/* OSPF6 node structure. */
struct cmd_node ospf6_node =
{
  OSPF6_NODE,
  "%s(config-ospf6)# ",
};

/* Install ospf related commands. */
void
ospf6_init ()
{
  /* Install ospf6 top node. */
  install_node (&ospf6_node, ospf6_config_write);

  install_element (VIEW_NODE, &show_ipv6_ospf6_cmd);
  install_element (VIEW_NODE, &show_version_ospf6_cmd);

#if 0
  install_element (VIEW_NODE, &show_ipv6_ospf6_statistics_cmd);
#endif

  install_element (ENABLE_NODE, &show_ipv6_ospf6_cmd);
  install_element (ENABLE_NODE, &show_version_ospf6_cmd);


#if 0
  install_element (ENABLE_NODE, &show_ipv6_ospf6_statistics_cmd);
#endif

  install_element (ENABLE_NODE, &reload_cmd);

#if 0
  install_element (ENABLE_NODE, &garbage_collection_cmd);
#endif

  install_element (CONFIG_NODE, &router_ospf6_cmd);
  install_element (CONFIG_NODE, &interface_cmd);

#ifdef HAVE_SETPROCTITLE
  install_element (CONFIG_NODE, &set_proctitle_cmd);
#endif /* HAVE_SETPROCTITLE */

  install_default (OSPF6_NODE);
  install_element (OSPF6_NODE, &router_id_cmd);
  install_element (OSPF6_NODE, &interface_area_cmd);
  install_element (OSPF6_NODE, &no_interface_cmd);
  install_element (OSPF6_NODE, &passive_interface_cmd);
  install_element (OSPF6_NODE, &no_passive_interface_cmd);
  install_element (OSPF6_NODE, &area_range_cmd);

  /* Make empty list of top list. */
  if_init ();

  ospf6_interface_init ();
  ospf6_neighbor_init ();
  ospf6_zebra_init ();
  ospf6_debug_init ();

  /* Install access list */
  access_list_init ();
#if 0
  access_list_add_hook (xxx);
  access_list_delete_hook (xxx);
#endif

  /* Install prefix list */
  prefix_list_init ();
#if 0
  prefix_list_add_hook (xxx);
  prefix_list_delete_hook (xxx);
#endif

  ospf6_routemap_init ();
  ospf6_lsdb_init ();

  ospf6_spf_init ();
  ospf6_route_init ();
}

void
ospf6_terminate ()
{
  /* stop ospf6 */
  ospf6_stop ();

  /* log */
  zlog (NULL, LOG_INFO, "OSPF6d terminated");
}

void
ospf6_maxage_remover ()
{
  if (IS_OSPF6_DUMP_LSDB)
    zlog_info ("MaxAge Remover");

  ospf6_top_schedule_maxage_remover (NULL, 0, ospf6);
  (*ospf6->foreach_area) (ospf6, NULL, 0,
                          ospf6_area_schedule_maxage_remover);
  (*ospf6->foreach_if) (ospf6, NULL, 0,
                        ospf6_interface_schedule_maxage_remover);
}

