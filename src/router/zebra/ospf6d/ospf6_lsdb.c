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

#include "linklist.h"
#include "ospf6_lsa.h"

struct ospf6_lsdb *
ospf6_lsdb_create ()
{
  struct ospf6_lsdb *lsdb;

  lsdb = XMALLOC (MTYPE_OSPF6_LSDB, sizeof (struct ospf6_lsdb));
  if (lsdb == NULL)
    {
      zlog_warn ("Can't malloc lsdb");
      return NULL;
    }

  memset (lsdb, 0, sizeof (struct ospf6_lsdb));
  return lsdb;
}

void
ospf6_lsdb_delete (struct ospf6_lsdb *lsdb)
{
  ospf6_lsdb_remove_all (lsdb);
  XFREE (MTYPE_OSPF6_LSDB, lsdb);
}

void
ospf6_lsdb_remove_maxage (struct ospf6_lsdb *lsdb)
{
  struct ospf6_lsdb_node *node;
  struct ospf6_lsa *lsa;

  for (node = ospf6_lsdb_head (lsdb); node; node = ospf6_lsdb_next (node))
    {
      lsa = node->lsa;

      /* contiue if it's not MaxAge */
      if (! ospf6_lsa_is_maxage (lsa))
        continue;

      /* continue if it's referenced by some retrans-lists */
      if (lsa->lock != 1)
        continue;

      if (IS_OSPF6_DUMP_LSDB)
        zlog_info ("Remove MaxAge LSA: %s", lsa->str);

      ospf6_lsdb_remove (lsa, lsdb);
      ospf6_lsa_unlock (lsa);
      ospf6_lsa_delete (lsa);
    }
}

struct ospf6_lsa *
ospf6_lsdb_lookup (u_int16_t type, u_int32_t id, u_int32_t adv_router)
{
  struct ospf6_interface *o6i;
  struct ospf6_area *o6a;
  listnode i, j;
  struct ospf6_lsa *lsa;

  if (OSPF6_LSA_IS_SCOPE_LINKLOCAL (ntohs (type)))
    {
      for (i = listhead (ospf6->area_list); i; nextnode (i))
        {
          o6a = getdata (i);
          for (j = listhead (o6a->if_list); j; nextnode (j))
            {
              o6i = getdata (j);
              lsa = ospf6_lsdb_lookup_lsdb (type, id, adv_router, o6i->lsdb);
              if (lsa)
                return lsa;
            }
        }
    }
  else if (OSPF6_LSA_IS_SCOPE_AREA (ntohs (type)))
    {
      for (i = listhead (ospf6->area_list); i; nextnode (i))
        {
          o6a = getdata (i);
          lsa = ospf6_lsdb_lookup_lsdb (type, id, adv_router, o6a->lsdb);
          if (lsa)
            return lsa;
        }
    }
  else if (OSPF6_LSA_IS_SCOPE_AS (ntohs (type)))
    {
      lsa = ospf6_lsdb_lookup_lsdb (type, id, adv_router, ospf6->lsdb);
      if (lsa)
        return lsa;
    }
  else
    zlog_warn ("Can't lookup lsdb: unknown scope: type %#x", ntohs (type));

  return NULL;
}

static void
ospf6_lsdb_stat_turnover (struct ospf6_lsa *old, struct ospf6_lsa *new)
{
  struct timeval now;
  u_long turnover_interval;

  /* update LSDB turnover statistics */
  gettimeofday (&now, (struct timezone *) NULL);
  turnover_interval = now.tv_sec - old->installed;
  new->turnover_total = old->turnover_total + turnover_interval;
  if (old->turnover_num)
    {
      if (old->turnover_min > turnover_interval)
        new->turnover_min = turnover_interval;
      else
        new->turnover_min = old->turnover_min;

      if (old->turnover_max < turnover_interval)
        new->turnover_max = turnover_interval;
      else
        new->turnover_max = old->turnover_max;
    }
  else
    {
      new->turnover_min = turnover_interval;
      new->turnover_max = turnover_interval;
    }
  new->turnover_num = old->turnover_num + 1;
}

static void
ospf6_lsdb_recalc_router (struct ospf6_lsa *lsa)
{
  struct ospf6_area *o6a = (struct ospf6_area *) lsa->scope;

  ospf6_spf_calculation_schedule (o6a->area_id);
  ospf6_route_calculation_schedule ();
}

static void
ospf6_lsdb_recalc_network (struct ospf6_lsa *lsa)
{
  struct ospf6_area *o6a = (struct ospf6_area *) lsa->scope;

  ospf6_spf_calculation_schedule (o6a->area_id);
  ospf6_route_calculation_schedule ();
}

static void
ospf6_lsdb_recalc_link (struct ospf6_lsa *lsa)
{
  struct ospf6_interface *o6i = (struct ospf6_interface *) lsa->scope;
  struct ospf6_area *o6a = (struct ospf6_area *) o6i->area;

  if (o6i->state == IFS_DR)
    ospf6_lsa_update_intra_prefix_transit (o6i->interface->name);

  ospf6_spf_calculation_schedule (o6a->area_id);
  ospf6_route_calculation_schedule ();
}

static void
ospf6_lsdb_recalc_intra_prefix (struct ospf6_lsa *lsa)
{
  ospf6_route_calculation_schedule ();
}

static void
ospf6_lsdb_recalc_as_external (struct ospf6_lsa *lsa)
{
  ospf6_route_external_incremental (lsa);
}

static void
ospf6_lsdb_recalc_unknown (struct ospf6_lsa *lsa)
{
  zlog_notice ("*** Unknown LSA Received");
}

static void
(*ospf6_lsdb_recalc[OSPF6_LSA_TYPE_MAX]) (struct ospf6_lsa *lsa) =
{
  ospf6_lsdb_recalc_unknown,
  ospf6_lsdb_recalc_router,
  ospf6_lsdb_recalc_network,
  ospf6_lsdb_recalc_unknown,
  ospf6_lsdb_recalc_unknown,
  ospf6_lsdb_recalc_as_external,
  ospf6_lsdb_recalc_unknown,
  ospf6_lsdb_recalc_unknown,
  ospf6_lsdb_recalc_link,
  ospf6_lsdb_recalc_intra_prefix
};

void
ospf6_lsdb_install (struct ospf6_lsa *new)
{
  struct ospf6_lsdb *lsdb;
  struct ospf6_lsa *old;
  int recalc = 0;

  struct ospf6 *as = NULL;
  struct ospf6_area *area = NULL;
  struct ospf6_interface *linklocal = NULL;

  if (OSPF6_LSA_IS_SCOPE_LINKLOCAL (ntohs (new->header->type)))
    {
      linklocal = (struct ospf6_interface *) new->scope;
      lsdb = linklocal->lsdb;
    }
  else if (OSPF6_LSA_IS_SCOPE_AREA (ntohs (new->header->type)))
    {
      area = (struct ospf6_area *) new->scope;
      lsdb = area->lsdb;
    }
  else if (OSPF6_LSA_IS_SCOPE_AS (ntohs (new->header->type)))
    {
      as = (struct ospf6 *) new->scope;
      lsdb = as->lsdb;
    }
  else
    {
      zlog_err ("Can't install lsdb: scope unknown: %s", new->str);
      return;
    }

  /* whether schedule calculation or not */
  old = ospf6_lsdb_lookup_lsdb (new->header->type, new->header->id,
                                new->header->adv_router, lsdb);
  if (old && ospf6_lsa_differ (old, new))
    recalc = 1;

  /* log */
  if (IS_OSPF6_DUMP_LSDB)
    {
      zlog_info ("LSA Install %s(%p): %s", new->str, new,
                 (recalc ? "recalc scheduled" : "recalc not scheduled"));
    }

  if (old)
    {
      ospf6_lsdb_stat_turnover (old, new);
      ospf6_lsdb_remove (old, lsdb);
    }

  ospf6_lsdb_add (new, lsdb);

  /* schedule SPF/Route calculation */
  if (recalc)
    (*ospf6_lsdb_recalc[OSPF6_LSA_TYPESW(new->header->type)]) (new);
}



/* vty functions */
static void
show_ipv6_ospf6_dbsummary (struct vty *vty, struct ospf6 *o6)
{
  char buf[32];
  listnode i, j;
  struct ospf6_area *o6a;
  struct ospf6_interface *o6i;
  struct ospf6_lsdb_node *node;

  u_int MaxAgeTotal, ActiveTotal, Total;
  u_int MaxAgeArea, ActiveArea, TotalArea;
  u_int MaxAgeRouter, ActiveRouter, TotalRouter;
  u_int MaxAgeNetwork, ActiveNetwork, TotalNetwork;
  u_int MaxAgeIntraPrefix, ActiveIntraPrefix, TotalIntraPrefix;
  u_int MaxAgeInterRouter, ActiveInterRouter, TotalInterRouter;
  u_int MaxAgeInterPrefix, ActiveInterPrefix, TotalInterPrefix;
  u_int MaxAgeASExternal, ActiveASExternal, TotalASExternal;
  u_int MaxAgeLink, ActiveLink, TotalLink;

  MaxAgeTotal = ActiveTotal = Total = MaxAgeASExternal
              = ActiveASExternal = TotalASExternal
              = 0;

  inet_ntop (AF_INET, &o6->router_id, buf, sizeof (buf));
  vty_out (vty, "%s", VTY_NEWLINE);
  vty_out (vty, "        OSPFv3 Router with ID (%s) (Process ID %ld)%s%s",
           buf, o6->process_id, VTY_NEWLINE, VTY_NEWLINE);

  for (node = ospf6_lsdb_head (o6->lsdb); node;
       node = ospf6_lsdb_next (node))
    {
      if (ntohs (node->lsa->header->type) != OSPF6_LSA_TYPE_AS_EXTERNAL)
        continue;

      if (ospf6_lsa_is_maxage (node->lsa))
        MaxAgeASExternal++;
      else
        ActiveASExternal++;
    }

  vty_out (vty, "AS:%s", VTY_NEWLINE);
  vty_out (vty, "%8s %11s%s",
           " ", "AS-External", VTY_NEWLINE);
  vty_out (vty, "%8s %11d%s",
           "Active", ActiveASExternal, VTY_NEWLINE);
  vty_out (vty, "%8s %11d%s",
           "MaxAge", MaxAgeASExternal, VTY_NEWLINE);

  MaxAgeTotal += MaxAgeASExternal;
  ActiveTotal += ActiveASExternal;

  for (i = listhead (o6->area_list); i; nextnode (i))
    {
      o6a = (struct ospf6_area *) getdata (i);

      MaxAgeArea = ActiveArea = TotalArea
      = MaxAgeRouter = ActiveRouter = TotalRouter
      = MaxAgeNetwork = ActiveNetwork = TotalNetwork
      = MaxAgeIntraPrefix = ActiveIntraPrefix = TotalIntraPrefix
      = MaxAgeInterRouter = ActiveInterRouter = TotalInterRouter
      = MaxAgeInterPrefix = ActiveInterPrefix = TotalInterPrefix
      = 0;

      for (node = ospf6_lsdb_head (o6a->lsdb); node;
           node = ospf6_lsdb_next (node))
        {
          if (ntohs (node->lsa->header->type) == OSPF6_LSA_TYPE_ROUTER)
            {
              if (ospf6_lsa_is_maxage (node->lsa))
                MaxAgeRouter++;
              else
                ActiveRouter++;
            }
          else if (ntohs (node->lsa->header->type) == OSPF6_LSA_TYPE_NETWORK)
            {
              if (ospf6_lsa_is_maxage (node->lsa))
                MaxAgeNetwork++;
              else
                ActiveNetwork++;
            }
          else if (ntohs (node->lsa->header->type) ==
                   OSPF6_LSA_TYPE_INTER_ROUTER)
            {
              if (ospf6_lsa_is_maxage (node->lsa))
                MaxAgeInterRouter++;
              else
                ActiveInterRouter++;
            }
          else if (ntohs (node->lsa->header->type) ==
                   OSPF6_LSA_TYPE_INTER_PREFIX)
            {
              if (ospf6_lsa_is_maxage (node->lsa))
                MaxAgeInterPrefix++;
              else
                ActiveInterPrefix++;
            }
          else if (ntohs (node->lsa->header->type) ==
                   OSPF6_LSA_TYPE_INTRA_PREFIX)
	    {
              if (ospf6_lsa_is_maxage (node->lsa))
                MaxAgeIntraPrefix++;
              else
                ActiveIntraPrefix++;
	    }
        }

      MaxAgeArea = MaxAgeRouter + MaxAgeNetwork + MaxAgeInterRouter
                   + MaxAgeInterPrefix + MaxAgeIntraPrefix;
      ActiveArea = ActiveRouter + ActiveNetwork + ActiveInterRouter
                   + ActiveInterPrefix + ActiveIntraPrefix;
      TotalArea = MaxAgeArea + ActiveArea;

      vty_out (vty, "Area ID: %s%s", o6a->str, VTY_NEWLINE);
      vty_out (vty, "%8s %6s %7s %11s %11s %11s  %8s%s",
               " ", "Router", "Network", "IntraPrefix", "InterRouter",
               "InterPrefix", "SubTotal", VTY_NEWLINE);
      vty_out (vty, "%8s %6d %7d %11d %11d %11d  %8d%s",
               "Active", ActiveRouter, ActiveNetwork, ActiveIntraPrefix,
               ActiveInterRouter, ActiveInterPrefix, ActiveArea, VTY_NEWLINE);
      vty_out (vty, "%8s %6d %7d %11d %11d %11d  %8d%s",
               "MaxAge", MaxAgeRouter, MaxAgeNetwork, MaxAgeIntraPrefix,
               MaxAgeInterRouter, MaxAgeInterPrefix, MaxAgeArea, VTY_NEWLINE);
      vty_out (vty, "%8s %6d %7d %11d %11d %11d  %8d%s",
               "SubTotal", MaxAgeRouter + ActiveRouter,
                           MaxAgeNetwork + ActiveNetwork,
                           MaxAgeIntraPrefix + ActiveIntraPrefix,
                           MaxAgeInterRouter + ActiveInterRouter,
                           MaxAgeInterPrefix + ActiveInterPrefix,
                           MaxAgeArea + ActiveArea, VTY_NEWLINE);

      MaxAgeTotal += MaxAgeArea;
      ActiveTotal += ActiveArea;

      for (j = listhead (o6a->if_list); j; nextnode (j))
        {
          o6i = (struct ospf6_interface *) getdata (j);

          MaxAgeLink = ActiveLink = TotalLink = 0;

          for (node = ospf6_lsdb_head (o6i->lsdb); node;
               node = ospf6_lsdb_next (node))
            {
              if (ntohs (node->lsa->header->type) == OSPF6_LSA_TYPE_LINK)
		{
                  if (ospf6_lsa_is_maxage (node->lsa))
                    MaxAgeLink++;
                  else
                    ActiveLink++;
		}
            }

          vty_out (vty, "INTERFACE: %s%s", o6i->interface->name, VTY_NEWLINE);
          vty_out (vty, "%8s %4s%s",
                   " ", "Link", VTY_NEWLINE);
          vty_out (vty, "%8s %4d%s",
                   "Active", ActiveLink, VTY_NEWLINE);
          vty_out (vty, "%8s %4d%s",
                   "MaxAge", MaxAgeLink, VTY_NEWLINE);

          MaxAgeTotal += MaxAgeLink;
          ActiveTotal += ActiveLink;
        }
    }

  vty_out (vty, "        Total: %d LSAs (%d MaxAge-LSAs)%s",
           MaxAgeTotal + ActiveTotal, MaxAgeTotal, VTY_NEWLINE);
}

DEFUN (show_ipv6_ospf6_database_dababase_summary,
       show_ipv6_ospf6_database_database_summary_cmd,
       "show ipv6 ospf6 database database-summary",
       SHOW_STR
       IP6_STR
       OSPF6_STR
       "Database Summary\n"
       "Summary of Database\n")
{
  OSPF6_CMD_CHECK_RUNNING ();
  show_ipv6_ospf6_dbsummary (vty, ospf6);
  return CMD_SUCCESS;
}

DEFUN (show_ipv6_ospf6_database_turnover,
       show_ipv6_ospf6_database_turnover_cmd,
       "show ipv6 ospf6 database turnover",
       SHOW_STR
       IP6_STR
       OSPF6_STR
       "Database summary\n"
       "Statistics of Database turn over\n"
       )
{
  listnode i, j;
  struct ospf6_area *o6a;
  struct ospf6_interface *o6i;
  char adv_router[15];
  struct ospf6_lsdb_node *node;

  OSPF6_CMD_CHECK_RUNNING ();

  vty_out (vty, "%-16s %-2s %-15s %4s %7s  %7s  %7s%s",
           "Type", "ID", "Adv-router", "Num", "Min", "Max", "Avg",
           VTY_NEWLINE);

  /* Linklocal scope */
  for (i = listhead (ospf6->area_list); i; nextnode (i))
    {
      o6a = (struct ospf6_area *) getdata (i);
      for (j = listhead (o6a->if_list); j; nextnode (j))
        {
          o6i = (struct ospf6_interface *) getdata (j);
          for (node = ospf6_lsdb_head (o6i->lsdb); node;
               node = ospf6_lsdb_next (node))
            {
              inet_ntop (AF_INET, &node->lsa->header->adv_router,
                         adv_router, sizeof (adv_router));
              vty_out (vty, "%-16s %-2d %-15s %4ld %7lds %7lds %7lds%s",
                       ospf6_lsa_type_string(node->lsa->header->type),
                       ntohl (node->lsa->header->id),
                       adv_router, node->lsa->turnover_num,
                       node->lsa->turnover_min, node->lsa->turnover_max,
                       (node->lsa->turnover_num ?
                        (node->lsa->turnover_total / node->lsa->turnover_num):
                        0),
                       VTY_NEWLINE);
            }
        }
    }

  /* Area scope */
  for (i = listhead (ospf6->area_list); i; nextnode (i))
    {
      o6a = (struct ospf6_area *) getdata (i);
      for (node = ospf6_lsdb_head (o6a->lsdb); node;
           node = ospf6_lsdb_next (node))
        {
          inet_ntop (AF_INET, &node->lsa->header->adv_router,
                     adv_router, sizeof (adv_router));
          vty_out (vty, "%-16s %-2d %-15s %4ld %7lds %7lds %7lds%s",
                   ospf6_lsa_type_string(node->lsa->header->type),
                   ntohl (node->lsa->header->id),
                   adv_router, node->lsa->turnover_num,
                   node->lsa->turnover_min, node->lsa->turnover_max,
                   (node->lsa->turnover_num ?
                    (node->lsa->turnover_total / node->lsa->turnover_num): 0),
                   VTY_NEWLINE);
        }
    }

  /* AS scope */
  for (node = ospf6_lsdb_head (ospf6->lsdb); node;
       node = ospf6_lsdb_next (node))
    {
      inet_ntop (AF_INET, &node->lsa->header->adv_router,
                 adv_router, sizeof (adv_router));
      vty_out (vty, "%-16s %-2d %-15s %4ld %7lds %7lds %7lds%s",
               ospf6_lsa_type_string (node->lsa->header->type),
               ntohl (node->lsa->header->id),
               adv_router, node->lsa->turnover_num,
               node->lsa->turnover_min, node->lsa->turnover_max,
               (node->lsa->turnover_num ?
                (node->lsa->turnover_total / node->lsa->turnover_num): 0),
               VTY_NEWLINE);
    }

  return CMD_SUCCESS;
}

DEFUN (show_ipv6_ospf6_database_turnover_summary,
       show_ipv6_ospf6_database_turnover_summary_cmd,
       "show ipv6 ospf6 database turnover-summary",
       SHOW_STR
       IP6_STR
       OSPF6_STR
       "Database summary\n"
       "Statistics summary of Database turn over\n"
       )
{
  listnode i, j;
  struct ospf6_area *o6a;
  struct ospf6_interface *o6i;
  struct ospf6_lsdb_node *node;
  u_long num_total[OSPF6_LSA_TYPE_MAX];
  u_long num_min[OSPF6_LSA_TYPE_MAX];
  u_long num_max[OSPF6_LSA_TYPE_MAX];
  u_long num_size[OSPF6_LSA_TYPE_MAX];
  u_long total[OSPF6_LSA_TYPE_MAX];
  u_long min[OSPF6_LSA_TYPE_MAX];
  u_long max[OSPF6_LSA_TYPE_MAX];
  u_long size[OSPF6_LSA_TYPE_MAX];
  int index;

  OSPF6_CMD_CHECK_RUNNING ();

  for (index = 0; index < OSPF6_LSA_TYPE_MAX; index++)
    {
      num_total[index] = num_min[index] = num_max[index] = num_size[index] = 0;
      total[index] = min[index] = max[index] = size[index] = 0;
    }

  /* Linklocal scope */
  for (i = listhead (ospf6->area_list); i; nextnode (i))
    {
      o6a = (struct ospf6_area *) getdata (i);
      for (j = listhead (o6a->if_list); j; nextnode (j))
        {
          o6i = (struct ospf6_interface *) getdata (j);
          for (node = ospf6_lsdb_head (o6i->lsdb); node;
               node = ospf6_lsdb_next (node))
            {
              index = OSPF6_LSA_TYPESW (node->lsa->header->type);

              if (num_min[index] == 0)
                num_min[index] = node->lsa->turnover_num;
              else if (num_min[index] > node->lsa->turnover_num)
                num_min[index] = node->lsa->turnover_num;
              if (num_max[index] < node->lsa->turnover_num)
                num_max[index] = node->lsa->turnover_num;
              num_total[index] += node->lsa->turnover_num;
              num_size[index]++;

              if (min[index] == 0)
                min[index] = node->lsa->turnover_min;
              else if (min[index] > node->lsa->turnover_min)
                min[index] = node->lsa->turnover_min;
              if (max[index] < node->lsa->turnover_max)
                max[index] = node->lsa->turnover_max;
              total[index] += node->lsa->turnover_total;
              size[index]++;
            }
        }
    }

  /* Area scope */
  for (i = listhead (ospf6->area_list); i; nextnode (i))
    {
      o6a = (struct ospf6_area *) getdata (i);
      for (node = ospf6_lsdb_head (o6a->lsdb); node;
           node = ospf6_lsdb_next (node))
        {
          index = OSPF6_LSA_TYPESW (node->lsa->header->type);

          if (num_min[index] == 0)
            num_min[index] = node->lsa->turnover_num;
          else if (num_min[index] > node->lsa->turnover_num)
            num_min[index] = node->lsa->turnover_num;
          if (num_max[index] < node->lsa->turnover_num)
            num_max[index] = node->lsa->turnover_num;
          num_total[index] += node->lsa->turnover_num;
          num_size[index]++;

          if (min[index] == 0)
            min[index] = node->lsa->turnover_min;
          else if (min[index] > node->lsa->turnover_min)
            min[index] = node->lsa->turnover_min;
          if (max[index] < node->lsa->turnover_max)
            max[index] = node->lsa->turnover_max;
          total[index] += node->lsa->turnover_total;
          size[index]++;
        }
    }

  /* AS scope */
  for (node = ospf6_lsdb_head (ospf6->lsdb); node;
       node = ospf6_lsdb_next (node))
    {
      index = OSPF6_LSA_TYPESW (node->lsa->header->type);

      if (num_min[index] == 0)
        num_min[index] = node->lsa->turnover_num;
      else if (num_min[index] > node->lsa->turnover_num)
        num_min[index] = node->lsa->turnover_num;
      if (num_max[index] < node->lsa->turnover_num)
        num_max[index] = node->lsa->turnover_num;
      num_total[index] += node->lsa->turnover_num;
      num_size[index]++;

      if (min[index] == 0)
        min[index] = node->lsa->turnover_min;
      else if (min[index] > node->lsa->turnover_min)
        min[index] = node->lsa->turnover_min;
      if (max[index] < node->lsa->turnover_max)
        max[index] = node->lsa->turnover_max;
      total[index] += node->lsa->turnover_total;
      size[index]++;
    }

  vty_out (vty, "%-16s %6s %6s %6s %6s  %6s  %6s%s",
           "Type", "MinNum", "MaxNum", "AvgNum", "Min", "Max", "Avg",
           VTY_NEWLINE);

  for (index = 1; index < OSPF6_LSA_TYPE_MAX; index++)
    {
      vty_out (vty, "%-16s %6ld %6ld %6ld %6lds %6lds %6lds%s",
               ospf6_lsa_type_strings[index],
               num_min[index], num_max[index],
               (num_size[index] ? (num_total[index] / num_size[index]) : 0),
               min[index], max[index],
               (num_total[index] ? total[index] / num_total[index] : 0),
               VTY_NEWLINE);
    }

  return CMD_SUCCESS;
}

#define OSPF6_LSDB_MATCH_TYPE        0x01
#define OSPF6_LSDB_MATCH_ID          0x02
#define OSPF6_LSDB_MATCH_ADV_ROUTER  0x04

static int
ospf6_lsdb_match (int flag, u_int16_t type, u_int32_t id,
                  u_int32_t adv_router, struct ospf6_lsa *lsa)
{
  if (CHECK_FLAG (flag, OSPF6_LSDB_MATCH_TYPE) &&
      lsa->header->type != type)
    return 0;

  if (CHECK_FLAG (flag, OSPF6_LSDB_MATCH_ID) &&
      lsa->header->id != id)
    return 0;

  if (CHECK_FLAG (flag, OSPF6_LSDB_MATCH_ADV_ROUTER) &&
      lsa->header->adv_router != adv_router)
    return 0;

  return 1;
}

DEFUN (show_ipv6_ospf6_database,
       show_ipv6_ospf6_database_cmd,
       "show ipv6 ospf6 database",
       SHOW_STR
       IP6_STR
       OSPF6_STR
       "LSA Database\n"
       )
{
  u_int flag;
  u_int16_t type = 0;
  u_int32_t id, adv_router;
  int ret;
  listnode i, j;
  struct ospf6_area *o6a;
  struct ospf6_interface *o6i;
  struct ospf6_lsdb_node *node;
  char invalid[32], *invalidp;

  flag = 0;
  memset (invalid, 0, sizeof (invalid));
  invalidp = invalid;

  if (argc > 0)
    {
      SET_FLAG (flag, OSPF6_LSDB_MATCH_TYPE);
      if (! strncmp (argv[0], "r", 1))
        type = htons (OSPF6_LSA_TYPE_ROUTER);
      if (! strncmp (argv[0], "n", 1))
        type = htons (OSPF6_LSA_TYPE_NETWORK);
      if (! strncmp (argv[0], "a", 1))
        type = htons (OSPF6_LSA_TYPE_AS_EXTERNAL);
      if (! strncmp (argv[0], "i", 1))
        type = htons (OSPF6_LSA_TYPE_INTRA_PREFIX);
      if (! strncmp (argv[0], "l", 1))
        type = htons (OSPF6_LSA_TYPE_LINK);
      if (! strncmp (argv[0], "*", 1))
        UNSET_FLAG (flag, OSPF6_LSDB_MATCH_TYPE);
    }

  if (argc > 1)
    {
      SET_FLAG (flag, OSPF6_LSDB_MATCH_ID);
      if (! strncmp (argv[1], "*", 1))
        UNSET_FLAG (flag, OSPF6_LSDB_MATCH_ID);
      else
        {
          ret = inet_pton (AF_INET, argv[1], &id);
          if (ret != 1)
            {
              id = htonl (strtoul (argv[1], &invalidp, 10));
              if (invalid[0] != '\0')
                {
                  vty_out (vty, "Link State ID is not parsable: %s%s",
                           argv[1], VTY_NEWLINE);
                  return CMD_SUCCESS;
                }
            }
        }
    }

  if (argc > 2)
    {
      SET_FLAG (flag, OSPF6_LSDB_MATCH_ADV_ROUTER);
      if (! strncmp (argv[2], "*", 1))
        UNSET_FLAG (flag, OSPF6_LSDB_MATCH_ADV_ROUTER);
      else
        {
          ret = inet_pton (AF_INET, argv[2], &adv_router);
          if (ret != 1)
            {
              adv_router = htonl (strtoul (argv[2], &invalidp, 10));
              if (invalid[0] != '\0')
                {
                  vty_out (vty, "Advertising Router is not parsable: %s%s",
                           argv[2], VTY_NEWLINE);
                  return CMD_SUCCESS;
                }
            }
        }
    }

  for (i = listhead (ospf6->area_list); i; nextnode (i))
    {
      o6a = (struct ospf6_area *) getdata (i);

      for (node = ospf6_lsdb_head (o6a->lsdb); node;
           node = ospf6_lsdb_next (node))
        {
          if (! ospf6_lsdb_match (flag, type, id, adv_router, node->lsa))
            continue;
          ospf6_lsa_show (vty, node->lsa);
        }
    }

  for (i = listhead (ospf6->area_list); i; nextnode (i))
    {
      o6a = (struct ospf6_area *) getdata (i);
      for (j = listhead (o6a->if_list); j; nextnode (j))
        {
          o6i = (struct ospf6_interface *) getdata (j);

          for (node = ospf6_lsdb_head (o6i->lsdb); node;
               node = ospf6_lsdb_next (node))
            {
              if (! ospf6_lsdb_match (flag, type, id, adv_router, node->lsa))
                continue;
              ospf6_lsa_show (vty, node->lsa);
            }
        }
    }

  for (node = ospf6_lsdb_head (ospf6->lsdb); node;
       node = ospf6_lsdb_next (node))
    {
      if (! ospf6_lsdb_match (flag, type, id, adv_router, node->lsa))
        continue;
      ospf6_lsa_show (vty, node->lsa);
    }

  return CMD_SUCCESS;
}

ALIAS (show_ipv6_ospf6_database,
       show_ipv6_ospf6_database_type_cmd,
       "show ipv6 ospf6 database (router|network|as-external|intra-prefix|link|*)",
       SHOW_STR
       IP6_STR
       OSPF6_STR
       "LSA Database\n"
       "Router-LSA\n"
       "Network-LSA\n"
       "AS-External-LSA\n"
       "Intra-Area-Prefix-LSA\n"
       "Link-LSA\n"
       "All LS Type\n"
       )

ALIAS (show_ipv6_ospf6_database,
       show_ipv6_ospf6_database_type_id_cmd,
       "show ipv6 ospf6 database (router|network|as-external|intra-prefix|link|*) (A.B.C.D|*)",
       SHOW_STR
       IP6_STR
       OSPF6_STR
       "LSA Database\n"
       "Router-LSA\n"
       "Network-LSA\n"
       "AS-External-LSA\n"
       "Intra-Area-Prefix-LSA\n"
       "Link-LSA\n"
       "All LS Type\n"
       "Link State ID\n"
       "All Link State ID\n"
       )

ALIAS (show_ipv6_ospf6_database,
       show_ipv6_ospf6_database_type_id_adv_router_cmd,
       "show ipv6 ospf6 database (router|network|as-external|intra-prefix|link|*) (A.B.C.D|*) (A.B.C.D|*)",
       SHOW_STR
       IP6_STR
       OSPF6_STR
       "LSA Database\n"
       "Router-LSA\n"
       "Network-LSA\n"
       "AS-External-LSA\n"
       "Intra-Area-Prefix-LSA\n"
       "Link-LSA\n"
       "All LS Type\n"
       "Link State ID\n"
       "All Link State ID\n"
       "Advertising Router\n"
       "All Advertising Router\n"
       )

void
ospf6_lsdb_init ()
{
  install_element (VIEW_NODE, &show_ipv6_ospf6_database_cmd);
  install_element (VIEW_NODE, &show_ipv6_ospf6_database_type_cmd);
  install_element (VIEW_NODE, &show_ipv6_ospf6_database_type_id_cmd);
  install_element (VIEW_NODE, &show_ipv6_ospf6_database_type_id_adv_router_cmd);
  install_element (VIEW_NODE, &show_ipv6_ospf6_database_database_summary_cmd);
  install_element (VIEW_NODE, &show_ipv6_ospf6_database_turnover_cmd);
  install_element (VIEW_NODE, &show_ipv6_ospf6_database_turnover_summary_cmd);

  install_element (ENABLE_NODE, &show_ipv6_ospf6_database_cmd);
  install_element (ENABLE_NODE, &show_ipv6_ospf6_database_type_cmd);
  install_element (ENABLE_NODE, &show_ipv6_ospf6_database_type_id_cmd);
  install_element (ENABLE_NODE, &show_ipv6_ospf6_database_type_id_adv_router_cmd);
  install_element (ENABLE_NODE, &show_ipv6_ospf6_database_database_summary_cmd);
  install_element (ENABLE_NODE, &show_ipv6_ospf6_database_turnover_cmd);
  install_element (ENABLE_NODE, &show_ipv6_ospf6_database_turnover_summary_cmd);
}

/***/

static struct ospf6_lsdb_node *
ospf6_lsdb_node_create ()
{
  struct ospf6_lsdb_node *node;

  node = XMALLOC (MTYPE_OSPF6_LSDB, sizeof (struct ospf6_lsdb_node));
  memset (node, 0, sizeof (struct ospf6_lsdb_node));

  return node;
}

static void
ospf6_lsdb_node_delete (struct ospf6_lsdb_node *node)
{
  XFREE (MTYPE_OSPF6_LSDB, node);
}

static void
ospf6_lsdb_node_lock (struct ospf6_lsdb_node *node)
{
  node->lock ++;
}

static void
ospf6_lsdb_node_unlock (struct ospf6_lsdb_node *node)
{
  node->lock --;
  if (node->lock == 0)
    {
      if (node->prev)
        node->prev->next = node->next;
      else
        node->lsdb->head = node->next;

      if (node->next)
        node->next->prev = node->prev;
      else
        node->lsdb->tail = node->prev;

      node->lsdb->count --;
      ospf6_lsa_unlock (node->lsa);
      ospf6_lsdb_node_delete (node);
    }
}

struct ospf6_lsdb_node *
ospf6_lsdb_head (struct ospf6_lsdb *lsdb)
{
  struct ospf6_lsdb_node *node;

  node = lsdb->head;
  if (node)
    ospf6_lsdb_node_lock (node);

  return node;
}

struct ospf6_lsdb_node *
ospf6_lsdb_next (struct ospf6_lsdb_node *node)
{
  struct ospf6_lsdb_node *next;

  next = node->next;
  if (next)
    ospf6_lsdb_node_lock (next);

  ospf6_lsdb_node_unlock (node);
  return next;
}

/* call this function when 'break'ing 'for' loop */
void
ospf6_lsdb_end (struct ospf6_lsdb_node *node)
{
  ospf6_lsdb_node_unlock (node);
}

static struct ospf6_lsdb_node *
ospf6_lsdb_node_lookup (u_int16_t type, u_int32_t id, u_int32_t adv_router,
                        struct ospf6_lsdb *lsdb)
{
  struct ospf6_lsdb_node *node;

  for (node = ospf6_lsdb_head (lsdb); node; node = ospf6_lsdb_next (node))
    {
      if (node->lsa->header->type != type)
        continue;
      if (node->lsa->header->id != id)
        continue;
      if (node->lsa->header->adv_router != adv_router)
        continue;

      ospf6_lsdb_end (node);
      return node;
    }
  return NULL;
}

void
ospf6_lsdb_add (struct ospf6_lsa *lsa, struct ospf6_lsdb *lsdb)
{
  struct ospf6_lsdb_node *node;

  if (ospf6_lsdb_node_lookup (lsa->header->type, lsa->header->id,
                              lsa->header->adv_router, lsdb))
    {
      zlog_err ("Can't add lsdb: already exists: %s", lsa->str);
      return;
    }

  node = ospf6_lsdb_node_create ();
  node->lsdb = lsdb;
  node->lsa = lsa;
  ospf6_lsa_lock (node->lsa);

  node->prev = lsdb->tail;
  if (lsdb->head == NULL)
    lsdb->head = node;
  else
    lsdb->tail->next = node;
  lsdb->tail = node;

  lsdb->count ++;
  ospf6_lsdb_node_lock (node);
}

void
ospf6_lsdb_remove (struct ospf6_lsa *lsa, struct ospf6_lsdb *lsdb)
{
  struct ospf6_lsdb_node *node;

  node = ospf6_lsdb_node_lookup (lsa->header->type, lsa->header->id,
                                 lsa->header->adv_router, lsdb);
  if (! node || node->lsa != lsa)
    {
      zlog_info ("Can't remove lsdb: no such instance: %s", lsa->str);
      return;
    }

  ospf6_lsdb_node_unlock (node);
}

void
ospf6_lsdb_remove_all (struct ospf6_lsdb *lsdb)
{
  struct ospf6_lsdb_node *node;
  for (node = ospf6_lsdb_head (lsdb); node; node = ospf6_lsdb_next (node))
    ospf6_lsdb_remove (node->lsa, lsdb);
}

struct ospf6_lsa *
ospf6_lsdb_lookup_lsdb (u_int16_t type, u_int32_t id, u_int32_t adv_router,
                        struct ospf6_lsdb *lsdb)
{
  struct ospf6_lsdb_node *node;
  node = ospf6_lsdb_node_lookup (type, id, adv_router, lsdb);
  if (node)
    return node->lsa;
  return NULL;
}


