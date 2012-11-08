/* 
   BGP Pretty Good BGP
   Copyright (C) 2008 University of New Mexico (Josh Karlin)

This file is part of GNU Zebra.

GNU Zebra is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

GNU Zebra is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Zebra; see the file COPYING.  If not, write to the Free
Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA. 
*/

/*
  Quagga based Pretty Good BGP:

  Summary 
  ------- 
  Pretty Good BGP (PGBGP) is a soft security enhancement to BGP.
  It uses independently collected (therefore completely distributed)
  historical routing information to determine network topology and
  prefix ownership.  Abberations to the historical database are considered
  anomalous and avoided when possible.

  What PGBGP can protect against: prefix hijacks, sub-prefix hijacks, and
  spoofed edges.

  Further reading is available at http://cs.unm.edu/~karlinjf/pgbgp/

  Route updates are forwarded to PGBGP, which determines if the route
  is anomalous.  Anomalous routes are flagged as suspicious and
  avoided where feasible for 24 hours.  If the anomalous
  characteristic is still in the RIB after 24 hours, consider it valid
  and enter it into the normal database.

  Cases for anomalous routes
  --------------------------
  case 1) New origin AS - prefix pair (one not recently seen in the RIB):
     response) label the route with BGP_INFO_SUSPICIOUS_O and avoid for 24 hours if possible

  case 2) New edge in path (one not recently seen in the RIB): 
     response) label the route with BGP_INFO_SUSPICIOUS_E and avoid for 24 hours
               if possible

  case 3) New prefix that is a sub-prefix of a prefix in recent history 
          and that path differs from the current less-specific's path
     response) label the sub-prefix routes with BGP_INFO_IGNORED_P and 
               prevent it from entering FIB for 24 hours
     response) label the super-net routes from the same next-hop as BGP_INFO_SUSPICIOUS_P 
               and try to avoid it for 24 hours if possible
     response) while no super-net route is selected, remove the BGP_INFO_IGNORED_P flags
  

  Normal Database (history)
  -------------------------
  Recently Seen) A route characteristic (edge, prefix/origin pair, prefix) 
                 that has resided within the RIB within the last X hours 
		 where X is user defined for each characteristic.
  Storage) Prefix and Origin history are stored in bgp_node structs with the
           "hist" pointer. 
	   Edge information is stored in a separate hash table, where the edge
	   is the key to the hash.
  Updates) The history's primary function is the keep track of when each route 
           characteristic was last seen.  For each route announcement, update 
           the history's 'last seen' time.  Periodically run the garbage collector 
	   which updates 'last seen' times for objects currently in the RIB.
  
  Garbage Collection
  ------------------
  Periodically the garbage collector (gc) is called to remove stale history 
  information and update the lastSeen time of objects that reside in the RIB 
  at the time of collection.  This is relatively expensive as it walks
  the RIB as well as the list of AS paths.

  What is removed) Objects that have not been seen in the RIB within a user-defined
                   time.
		   Suspicious objcets that are 24 hours old that have not been in the RIB
		   since the last collection.
  
  Reuse Priority Queue
  --------------------
  After 24 hours, routes that are flagged as suspicious have the flags removed.  
  This is not run on a timer.  Instead, for each update that PGBGP is informed of,
  it checks the reuse queue to determine if any routes need to be updated.

*/


/*
  Things that must be ensured:
  . GC updates lastSeen so it must be called at least twice as often as the lowest BUFFER_TIME
  . GC should be called at least twice per day
  . Delay times must be shorter than history window lengths
*/


/*
  Changes made to original PGBGP thinking
  . Don't check for things in the RIB all of the time, periodically 
    update the lastSeen values and just use lastSeen
*/

/*
  Changes made to original protocol
  . sub-prefixes are only ignored while the super-net has a selected 
    route and it's non-anomalous (not to a neighbor that announced 
    the sub-prefix)
  
  . At point of reuse, don't delete the item if it's not in the RIB. 
    delete it if it hasn't been in the RIB since the last storage.  
    This saves a lot of processing time for new edges

  . Changed heuristic from "if new sub-prefix and trusted AS on path 
    then it's okay" to  "if new sub-prefix and same path is used to reach 
    super-prefix, then it's okay".  Might be better to change to "if old 
    path is prefix of new path, then okay"
*/

#include <zebra.h>
#include <math.h>

#include "prefix.h"
#include "memory.h"
#include "command.h"
#include "log.h"
#include "pqueue.h"
#include "table.h"
#include "hash.h"
#include "str.h"

#include "bgpd/bgpd.h"
#include "bgpd/bgp_aspath.h"
#include "bgpd/bgp_pgbgp.h"
#include "bgpd/bgp_table.h"
#include "bgpd/bgp_route.h"
#include "bgpd/bgp_attr.h"
#include "bgpd/bgp_advertise.h"


#define true 1
#define false 0

struct hash * ashash;

static void *edge_hash_alloc (void *arg);
static unsigned int edge_key_make (void *p);
static int edge_cmp (const void *arg1, const void *args);

// Helper Functions
static struct bgp_pgbgp_pathSet bgp_pgbgp_pathOrigin (struct aspath *);
static int bgp_pgbgp_pathLength (struct aspath *asp);
static int bgp_pgbgp_gc (struct bgp_table *);
static int bgp_pgbgp_clean (struct bgp_table *);
static int bgp_pgbgp_reuse (time_t);
static struct bgp_node *findSuper (struct bgp_table *table, struct prefix *p,
                            time_t t_now);
static int bgp_pgbgp_store (struct bgp_table *table);
static int bgp_pgbgp_restore (void);
static struct bgp_info *bgp_pgbgp_selected (struct bgp_node *node);
static int originInRIB (struct bgp_node *node, struct bgp_pgbgp_origin *origin);
static int prefixInRIB (struct bgp_node *node, struct bgp_pgbgp_prefix *prefix);
static int edgeInRIB (struct bgp_pgbgp_edge *e);

// MOAS Functions
static void bgp_pgbgp_logOriginAnomaly (as_t asn, struct bgp_node *rn,
                                 struct attr *);
static int bgp_pgbgp_reuseOrigin (struct bgp_pgbgp_r_origin);
static void bgp_pgbgp_cleanHistTable (struct bgp_table *);
static int bgp_pgbgp_garbageCollectHistTable (struct bgp_table *);
static void bgp_pgbgp_storeHistTable (struct bgp_table *table, FILE * file);
static int bgp_pgbgp_updateOrigin (struct bgp_pgbgp_hist *, struct bgp_info *,
                            struct attr *, struct bgp_node *, time_t, int);


// Sub-Prefix Hijack Detector Functions
static int bgp_pgbgp_shouldIgnore (struct bgp_node *super, struct bgp_info *selected);
static void bgp_pgbgp_logSubprefixAnomaly (as_t asn, struct bgp_node *rn,
                                    struct attr *, struct bgp_node *super);
static int bgp_pgbgp_reusePrefix (struct bgp_pgbgp_r_prefix);
static int bgp_pgbgp_updatePrefix (struct bgp_pgbgp_hist *hist, struct bgp_node *,
                            struct bgp_info *, struct attr *,
                            struct bgp_node *, time_t, int);


// Spoofed Edge Detector Functions
static void bgp_pgbgp_cleanEdges (void);
static void bgp_pgbgp_logEdgeAnomaly (struct bgp_node *rn, struct attr *,
                               struct edge *edge);
static int bgp_pgbgp_reuseEdge (struct bgp_pgbgp_r_edge);
static void bgp_pgbgp_storeEdges (struct bgp_table *, FILE *);
static int bgp_pgbgp_garbageCollectEdges (struct bgp_table *);
static int bgp_pgbgp_updateEdge (struct bgp_pgbgp_hist *hist, struct bgp_info *,
                          struct attr *, struct bgp_node *, time_t, int);
static int bgp_pgbgp_restoreEdge (FILE * file);
static void bgp_pgbgp_storeEdges (struct bgp_table *table, FILE * file);



// New Peer Detector Functions
static int bgp_pgbgp_updatePeer (struct bgp_info *binfo, time_t now);


/* --------------- Global Variables ------------------ */
struct bgp_pgbgp_config bgp_pgbgp_cfg;
struct bgp_pgbgp_config *pgbgp = &bgp_pgbgp_cfg;
/*! --------------- Global Variables ------------------ !*/

/* --------------- VTY (others exist in bgp_route.c)  ------------------ */

struct nsearch
{
  struct vty *pvty;
  time_t time;
  as_t asn;
};

static void
edge_neighbor_iterator (struct hash_backet *backet, struct nsearch *pns)
{
  struct bgp_pgbgp_edge *hedge = backet->data;
  if ((!pns->asn || hedge->e.a == pns->asn || hedge->e.b == pns->asn)
      && hedge->e.a != hedge->e.b)
    {
      struct vty *vty = pns->pvty;
      if (hedge->deprefUntil > pns->time)
        vty_out (pns->pvty, "Untrusted: %d -- %d%s", hedge->e.a, hedge->e.b,
                 VTY_NEWLINE);
      else
        vty_out (pns->pvty, "Trusted: %d -- %d%s", hedge->e.a, hedge->e.b,
                 VTY_NEWLINE);
    }
}

static int
bgp_pgbgp_stats_neighbors (struct vty *vty, afi_t afi, safi_t safi, as_t asn)
{
  struct nsearch ns;
  ns.pvty = vty;
  ns.time = time (NULL);
  ns.asn = asn;

  hash_iterate (pgbgp->edgeT,
                (void (*)(struct hash_backet *, void *))
                edge_neighbor_iterator, &ns);
  return CMD_SUCCESS;
}

static void
bgp_pgbgp_stats_origin_one (struct vty *vty, struct bgp_node *rn,
                            time_t t_now)
{
  char str[INET6_BUFSIZ];
  
  if (!rn->hist)
    return;
  
  prefix2str (&rn->p, str, sizeof(str));
  vty_out (vty, "%s%s", str, VTY_NEWLINE);
  
  for (struct bgp_pgbgp_origin * cur = rn->hist->o; cur != NULL;
       cur = cur->next)
    {
      if (cur->deprefUntil > t_now)
        vty_out (vty, "Untrusted Origin AS: %d%s", cur->originAS,
                 VTY_NEWLINE);
      else
        vty_out (vty, "Trusted Origin AS: %d%s", cur->originAS,
                 VTY_NEWLINE);
    } 
}

static int
bgp_pgbgp_stats_origins (struct vty *vty, afi_t afi, safi_t safi,
                         const char *prefix)
{
  struct bgp *bgp;
  struct bgp_table *table;
  struct bgp_node *rn;
  time_t t_now = time (NULL);
  
  bgp = bgp_get_default ();
  if (bgp == NULL)
    return CMD_WARNING;
  if (bgp->rib == NULL)
    return CMD_WARNING;
  table = bgp->rib[afi][safi];
  if (table == NULL)
    return CMD_WARNING;
  
  if (prefix)
    {
      struct prefix p;
      str2prefix (prefix, &p);
      rn = bgp_node_match (table, &p);
      if (rn->hist)
        bgp_pgbgp_stats_origin_one (vty, rn, t_now);
      bgp_unlock_node (rn);
      return CMD_SUCCESS;
    }
  
  for (rn = bgp_table_top (table); rn; rn = bgp_route_next (rn))
    if (rn->hist)
      bgp_pgbgp_stats_origin_one (vty, rn, t_now);

  return CMD_SUCCESS;
}

static int
bgp_pgbgp_stats (struct vty *vty, afi_t afi, safi_t safi)
{
  struct bgp *bgp;
  struct bgp_table *table;


  bgp = bgp_get_default ();
  if (bgp == NULL)
    return CMD_WARNING;
  if (bgp->rib == NULL)
    return CMD_WARNING;
  table = bgp->rib[afi][safi];
  if (table == NULL)
    return CMD_WARNING;

  //    bgp_pgbgp_store(table);

  // Print out the number of anomalous routes
  int anomalous = 0;
  int routes = 0;
  int num_selected = 0;
  int num_origin = 0;
  int num_super = 0;
  int num_ignored = 0;
  int num_edge = 0;

  for (struct bgp_node * rn = bgp_table_top (table); rn;
       rn = bgp_route_next (rn))
    {
      for (struct bgp_info * ri = rn->info; ri; ri = ri->next)
        {
          routes += 1;
          if (ANOMALOUS (ri->flags))
            {
              anomalous += 1;
              if (CHECK_FLAG (ri->flags, BGP_INFO_SELECTED))
                num_selected += 1;

              if (CHECK_FLAG (ri->flags, BGP_INFO_SUSPICIOUS_O))
                num_origin += 1;
              if (CHECK_FLAG (ri->flags, BGP_INFO_SUSPICIOUS_E))
                num_edge += 1;
              if (CHECK_FLAG (ri->flags, BGP_INFO_SUSPICIOUS_P))
                num_super += 1;
              if (CHECK_FLAG (ri->flags, BGP_INFO_IGNORED_P))
                num_ignored += 1;
            }
        }
    }

  vty_out (vty, "%-30s: %10d%s", "Routes in the RIB", routes, VTY_NEWLINE);
  vty_out (vty, "%-30s: %10d%s", "Anomalous routes in RIB", anomalous,
           VTY_NEWLINE);
  vty_out (vty, "%-30s: %10d%s", "Selected anomalous routes", num_selected,
           VTY_NEWLINE);
  vty_out (vty, "-----------------------------%s", VTY_NEWLINE);
  vty_out (vty, "%-30s: %10d%s", "Routes with anomalous origins", num_origin,
           VTY_NEWLINE);
  vty_out (vty, "%-30s: %10d%s", "Routes with anomalous edges", num_edge,
           VTY_NEWLINE);
  vty_out (vty, "%-30s: %10d%s", "Routes ignored for sub-prefix", num_ignored,
           VTY_NEWLINE);
  vty_out (vty, "%-30s: %10d%s", "Less specific routes to avoid", num_super,
           VTY_NEWLINE);
  /*
     vty_out (vty, "There are %d routes in the RIB.%s", routes, VTY_NEWLINE); 
     vty_out (vty, "%d are anomalous.%s", anomalous, VTY_NEWLINE);
     vty_out (vty, "%d anomalous routes are selected.%s", num_selected, VTY_NEWLINE);
     vty_out (vty, "%s", VTY_NEWLINE);
     vty_out (vty, "Anomaly breakdown:%s", VTY_NEWLINE);
     vty_out (vty, "%d contain anomalous origins%s", num_origin, VTY_NEWLINE);
     vty_out (vty, "%d contain anomalous edges.%s", num_edge, VTY_NEWLINE);
     vty_out (vty, "%d are for ignored sub-prefixes.%s", num_ignored, VTY_NEWLINE);
     vty_out (vty, "%d are super-net routes through peers that announced anomalous sub-prefixes.%s", num_super, VTY_NEWLINE);
   */
  return CMD_SUCCESS;
}


DEFUN (show_ip_bgp_pgbgp,
       show_ip_bgp_pgbgp_cmd,
       "show ip bgp pgbgp",
       SHOW_STR IP_STR BGP_STR "Pretty-Good BGP statistics\n")
{
  return bgp_pgbgp_stats (vty, AFI_IP, SAFI_UNICAST);
}

DEFUN (show_ip_bgp_pgbgp_neighbors,
       show_ip_bgp_pgbgp_neighbors_cmd,
       "show ip bgp pgbgp neighbors WORD",
       SHOW_STR IP_STR BGP_STR
       "Pretty-Good BGP statistics\n"
       "PG-BGP neighbor information\n"
       "AS to show neighbors of\n")
{
  return bgp_pgbgp_stats_neighbors (vty, AFI_IP, SAFI_UNICAST,
                                    argc == 1 ? atoi (argv[0]) : 0);
}

ALIAS (show_ip_bgp_pgbgp_neighbors,
       show_ip_bgp_pgbgp_neighbors_all_cmd,
       "show ip bgp pgbgp neighbors",
       SHOW_STR
       IP_STR
       BGP_STR
       "Pretty-Good BGP statistics\n"
       "PG-BGP neighbors information\n")

DEFUN (show_ip_bgp_pgbgp_origins,
       show_ip_bgp_pgbgp_origins_cmd,
       "show ip bgp pgbgp origins A.B.C.D/M",
       SHOW_STR
       IP_STR
       BGP_STR
       "Pretty-Good BGP statistics\n"
       "PG-BGP prefix origin information\n"
       "Prefix to look up origin ASes of\n")
{
  return bgp_pgbgp_stats_origins (vty, AFI_IP, SAFI_UNICAST,
                                  argc == 1 ? argv[0] : NULL);
}

ALIAS (show_ip_bgp_pgbgp_origins,
       show_ip_bgp_pgbgp_origins_all_cmd,
       "show ip bgp pgbgp origins",
       SHOW_STR
       IP_STR
       BGP_STR
       "Pretty-Good BGP statistics\n"
       "PG-BGP prefixes origin information")


/*! --------------- VTY (others exist in bgp_route.c)  ------------------ !*/







/* --------------- Helper Functions ------------------ */
/*
  If the origin hasn't been seen/verified lately, look for it in the RIB
*/
int
originInRIB (struct bgp_node *node, struct bgp_pgbgp_origin *origin)
{
  for (struct bgp_info * ri = node->info; ri; ri = ri->next)
    {
      struct bgp_pgbgp_pathSet pathOrigins;
      pathOrigins = bgp_pgbgp_pathOrigin (ri->attr->aspath);
      for (int i = 0; i < pathOrigins.length; ++i)
        {
          if (pathOrigins.ases[i] == origin->originAS)
            {
              return true;
            }
        }
    }
  return false;
}


/*
  If the prefix hasn't been seen/verified lately, look for it in the RIB
*/
int
prefixInRIB (struct bgp_node *node, struct bgp_pgbgp_prefix *prefix)
{
  if (node->info)
    return true;
  return false;
}

static int
edge_inRIB_iterator (struct hash_backet *backet, struct bgp_pgbgp_edge *hedge)
{
  struct aspath *p = backet->data;
  char first = true;
  struct edge curEdge;
  curEdge.a = 0;
  curEdge.b = 0;

  struct assegment *seg;

  for (seg = p->segments; seg; seg = seg->next)
    {
      for (int i = 0; i < seg->length; i++)
        {
          curEdge.a = curEdge.b;
          curEdge.b = seg->as[i];
          if (first)
            {
              first = false;
              continue;
            }
          // Is this the edge we're looking for?
          if (curEdge.a == hedge->e.a && curEdge.b == hedge->e.b)
            {
              hedge->lastSeen = time (NULL);
              return false;
            }
        }
    }

  return true;
}

/*
  If the edge hasn't been seen/verified lately, look for it in the AS path list
  This function is expensive, use sparingly
*/
int
edgeInRIB (struct bgp_pgbgp_edge *e)
{
  int completed;
  completed = hash_iterate_until (ashash,
                                  (int (*)(struct hash_backet *, void *))
                                  edge_inRIB_iterator, e);
  if (completed)
    return false;

  return true;
}



/*
  Return the selected route for the given route node
 */

struct bgp_info *
bgp_pgbgp_selected (struct bgp_node *node)
{
  for (struct bgp_info * ri = node->info; ri; ri = ri->next)
    {
      if (CHECK_FLAG (ri->flags, BGP_INFO_SELECTED))
        return ri;
    }
  return NULL;
}

static int
reuse_cmp (void *node1, void *node2)
{
  struct bgp_pgbgp_reuse *a;
  struct bgp_pgbgp_reuse *b;
  a = (struct bgp_pgbgp_reuse *) node1;
  b = (struct bgp_pgbgp_reuse *) node2;
  return a->deprefUntil - b->deprefUntil;
}

int
bgp_pgbgp_pathLength (struct aspath *asp)
{
  struct assegment *seg;
  if ((asp == NULL) || (asp->segments == NULL))
    return 0;
  int count = 0;
  seg = asp->segments;
  while (seg->next != NULL)
    {
      count += seg->length;
      seg = seg->next;
    }
  return count;
}



/* Find the origin(s) of the path
   All ASes in the final set are considered origins */
static struct bgp_pgbgp_pathSet
bgp_pgbgp_pathOrigin (struct aspath *asp)
{
  struct assegment *seg, *last;
  struct bgp_pgbgp_pathSet tmp;
  tmp.length = 0;
  tmp.ases = NULL;

  assert (asp != NULL && asp->segments != NULL);

  /*    if ( (asp == NULL) || (asp->segments == NULL) )
     return tmp;
   */
  seg = asp->segments;
  last = NULL;
  while (seg->next != NULL)
    {
      if (seg->type != AS_SET && seg->type != AS_CONFED_SET)
        last = seg;
      seg = seg->next;
    }

  if (seg->type == AS_SET || seg->type == AS_CONFED_SET)
    seg = last;

  assert (seg);
  tmp.length = 1;
  tmp.ases = &seg->as[seg->length - 1];

  /*
     if (seg->type == AS_SET || seg->type == AS_CONFED_SET)
     {
     tmp.length = seg->length;
     tmp.ases = seg->as;
     }
     else
     {
     tmp.length = 1;
     tmp.ases = &seg->as[seg->length - 1];
     }
   */
  assert (tmp.length >= 1);
  return tmp;
  //    return seg->as[seg->length-1];
}

int
bgp_pgbgp_reuse (time_t t_now)
{

  struct bgp_pgbgp_reuse *cur = NULL;

  while (pgbgp->rq_size > 0)
    {
      cur = pqueue_dequeue (pgbgp->reuse_q);
      pgbgp->rq_size -= 1;

      // Is the next item ready to be reused?
      if (t_now < cur->deprefUntil)
        {
          pqueue_enqueue (cur, pgbgp->reuse_q);
          pgbgp->rq_size += 1;
          break;
        }

      // Okay, it needs to be reused now
      if (cur->type == PGBGP_REUSE_ORIGIN)
        bgp_pgbgp_reuseOrigin (cur->data.origin);

      else if (cur->type == PGBGP_REUSE_PREFIX)
        bgp_pgbgp_reusePrefix (cur->data.prefix);

      else if (cur->type == PGBGP_REUSE_EDGE)
        bgp_pgbgp_reuseEdge (cur->data.edge);


      XFREE (MTYPE_BGP_PGBGP_REUSE, cur);
    }
  return 0;
}

/* Check bit of the prefix. */
static int
check_bit (u_char * prefix, u_char prefixlen)
{
  int offset;
  int shift;
  u_char *p = (u_char *) prefix;

  assert (prefixlen <= 128);

  offset = prefixlen / 8;
  shift = 7 - (prefixlen % 8);

  return (p[offset] >> shift & 1);
}

/*
  Find a super-net in the tree that's not currently anomalous if one exists
*/
struct bgp_node *
findSuper (struct bgp_table *table, struct prefix *p, time_t t_now)
{
  struct bgp_node *node;
  struct bgp_node *matched;

  matched = NULL;
  node = table->top;

  while (node && node->p.prefixlen < p->prefixlen &&
         prefix_match (&node->p, p))
    {
      // Node may not yet have its info set when reading in from pgbgp log files
      if (node->hist && node->p.prefixlen >= 8)
        {
          if (node->hist->p != NULL && node->hist->p->ignoreUntil < t_now)
            //if (node->hist->p != NULL && prefixInRIB (node, NULL))
            //if (node->hist->p != NULL)
            matched = node;
        }
      node = node->link[check_bit (&p->u.prefix, node->p.prefixlen)];
    }
  if (matched)
    return bgp_lock_node (matched);
  return NULL;
}





/*! --------------- Helper Functions ------------------ !*/







/* --------------- Public PGBGP Interface ------------------ */
int
bgp_pgbgp_enable (struct bgp *bgp, afi_t afi, safi_t safi,
                  int ost, int est, int sst, int oht, int pht, int eht,
                  const char *file, const char *anoms)
{

  if (CHECK_FLAG (bgp->af_flags[afi][safi], BGP_CONFIG_PGBGP))
    {
      if (pgbgp->storage && pgbgp->anomalies)
        {
          if (pgbgp->origin_sus_time == ost
              && pgbgp->edge_sus_time == est
              && pgbgp->sub_sus_time == sst
              && pgbgp->origin_hist_time == oht
              && pgbgp->prefix_hist_time == pht
              && pgbgp->edge_hist_time == eht
              && strcmp (pgbgp->storage, file) == 0
              && strcmp (pgbgp->anomalies, anoms) == 0)

            return 0;
        }
    }

  SET_FLAG (bgp->af_flags[afi][safi], BGP_CONFIG_PGBGP);

#ifndef PGBGP_DEBUG
  time_t hour = 3600;
  time_t day = 86400;
#endif
#ifdef PGBGP_DEBUG
  time_t hour = 2;
  time_t day = 5;
#endif

  pgbgp->origin_sus_time = ost * hour;
  pgbgp->edge_sus_time = est * hour;
  pgbgp->sub_sus_time = sst * hour;
  pgbgp->origin_hist_time = oht * day;
  pgbgp->prefix_hist_time = pht * day;
  pgbgp->edge_hist_time = eht * day;
  pgbgp->peer_hist_time = DEFAULT_ORIGIN_HIST;

  if (file != NULL)
    pgbgp->storage = strdup (file);
  else
    pgbgp->storage = NULL;

  if (anoms != NULL)
    pgbgp->anomalies = strdup (anoms);
  else
    pgbgp->anomalies = NULL;


  pgbgp->reuse_q = pqueue_create ();
  pgbgp->reuse_q->cmp = reuse_cmp;
  pgbgp->rq_size = 0;
  pgbgp->lastgc = time (NULL);
  pgbgp->lastStore = time (NULL);
  pgbgp->startTime = time (NULL);
  install_element (RESTRICTED_NODE, &show_ip_bgp_pgbgp_cmd);
  install_element (RESTRICTED_NODE, &show_ip_bgp_pgbgp_neighbors_cmd);
  install_element (RESTRICTED_NODE, &show_ip_bgp_pgbgp_origins_cmd);
  install_element (VIEW_NODE, &show_ip_bgp_pgbgp_cmd);
  install_element (VIEW_NODE, &show_ip_bgp_pgbgp_neighbors_cmd);
  install_element (VIEW_NODE, &show_ip_bgp_pgbgp_origins_cmd);
  install_element (VIEW_NODE, &show_ip_bgp_pgbgp_neighbors_all_cmd);
  install_element (VIEW_NODE, &show_ip_bgp_pgbgp_origins_all_cmd);
  install_element (ENABLE_NODE, &show_ip_bgp_pgbgp_cmd);
  install_element (ENABLE_NODE, &show_ip_bgp_pgbgp_neighbors_cmd);
  install_element (ENABLE_NODE, &show_ip_bgp_pgbgp_origins_cmd);
  install_element (ENABLE_NODE, &show_ip_bgp_pgbgp_neighbors_all_cmd);
  install_element (ENABLE_NODE, &show_ip_bgp_pgbgp_origins_all_cmd);
  pgbgp->edgeT = hash_create_size (131072, edge_key_make, edge_cmp);
  bgp_pgbgp_restore ();
  return 0;
}

int
bgp_pgbgp_disable (struct bgp *bgp, afi_t afi, safi_t safi)
{
  UNSET_FLAG (bgp->af_flags[afi][safi], BGP_CONFIG_PGBGP);

  // Clean the tables
  if (bgp->rib[afi][safi] != NULL)
    bgp_pgbgp_clean (bgp->rib[afi][safi]);

  bgp_pgbgp_cleanEdges ();

  if (pgbgp->storage != NULL)
    free (pgbgp->storage);

  if (pgbgp->anomalies != NULL)
    free (pgbgp->anomalies);

  struct bgp_pgbgp_peerTime *pr = pgbgp->peerLast;
  while (pr)
    {
      struct bgp_pgbgp_peerTime *cur = pr;
      pr = pr->next;
      XFREE (MTYPE_BGP_PGBGP_PEER, cur);
    }

  return 0;
}

int
bgp_pgbgp_clean (struct bgp_table *table)
{
  struct bgp_pgbgp_reuse *rnode = NULL;

  while (pgbgp->rq_size > 0)
    {
      rnode = (struct bgp_pgbgp_reuse *) pqueue_dequeue (pgbgp->reuse_q);
      pgbgp->rq_size -= 1;
      XFREE (MTYPE_BGP_PGBGP_REUSE, rnode);
    }
  pqueue_delete (pgbgp->reuse_q);

  if (table == NULL)
    return 0;

  // Clean the detectors
  bgp_pgbgp_cleanHistTable (table);

  bgp_pgbgp_cleanEdges ();


  // Clean up the RIB nodes
  for (struct bgp_node * rn = bgp_table_top (table); rn;
       rn = bgp_route_next (rn))
    {
      int changed = 0;
      for (struct bgp_info * ri = rn->info; ri; ri = ri->next)
        {
          if (CHECK_FLAG (ri->flags, BGP_INFO_SUSPICIOUS_O
                          | BGP_INFO_SUSPICIOUS_P | BGP_INFO_SUSPICIOUS_E
                          | BGP_INFO_IGNORED_P))
            {
              changed = 1;
              UNSET_FLAG (ri->flags, BGP_INFO_SUSPICIOUS_O
                          | BGP_INFO_SUSPICIOUS_P | BGP_INFO_SUSPICIOUS_E
                          | BGP_INFO_IGNORED_P);
            }
        }
      if (changed && rn->info)
        {
          struct bgp_info *ri = rn->info;
          bgp_process (ri->peer->bgp, rn, rn->table->afi, rn->table->safi);
        }
    }

  hash_free (pgbgp->edgeT);
  return 0;
}


int
bgp_pgbgp_gc (struct bgp_table *table)
{
  struct bgp *bgp = bgp_get_default ();
  if (!bgp)
    return 0;

  // Collect each AFI/SAFI RIB
  for (afi_t afi = AFI_IP; afi < AFI_MAX; afi++)
    for (safi_t safi = SAFI_UNICAST; safi < SAFI_MAX; safi++)
      {
        if (!CHECK_FLAG (bgp->af_flags[afi][safi], BGP_CONFIG_PGBGP))
          continue;
        struct bgp_table *curTable = bgp->rib[afi][safi];
        if (!curTable)
          continue;
        bgp_pgbgp_garbageCollectHistTable (curTable);
      }

  bgp_pgbgp_garbageCollectEdges (table);

  return 0;
}

int
bgp_pgbgp_restore (void)
{

  if (pgbgp->storage == NULL)
    return 0;
  FILE *file = fopen (pgbgp->storage, "r");
  if (!file)
    return 0;

  int type = 0;
  struct prefix p;
  struct bgp *bgp = bgp_get_default ();
  struct bgp_node *curNode = NULL;

  // Get the log store time
  long long int writetime;
  fscanf (file, "%lld", &writetime);
  time_t swtime = writetime;

  // If it's too old (more than 1 week old), start fresh
  if (time (NULL) - swtime > 86400 * 7)
    {
      fclose (file);
      return 0;
    }


  // Get the PGBGP init time
  long long int stime;
  fscanf (file, "%lld", &stime);
  pgbgp->startTime = stime;

  while (fscanf (file, "%d", &type) != EOF)
    {

      if (type == PREFIX_ID)
        {
          char pre[128];
          unsigned int afi;
          unsigned int safi;
          long long int time;
          fscanf (file, "%s %u %u %lld", pre, &afi, &safi, &time);
          str2prefix (pre, &p);
          struct bgp_table *curTable = bgp->rib[afi][safi];
          assert (curTable != NULL);

          // Create and lock the node
          curNode = bgp_node_get (curTable, &p);
          assert (curNode->hist == NULL);

          //              bgp_lock_node(curNode);

          curNode->hist =
            XCALLOC (MTYPE_BGP_PGBGP_HIST, sizeof (struct bgp_pgbgp_hist));
          assert (curNode->hist != NULL);

          curNode->hist->p =
            XCALLOC (MTYPE_BGP_PGBGP_PREFIX,
                     sizeof (struct bgp_pgbgp_prefix));
          assert (curNode->hist->p != NULL);

          curNode->hist->p->lastSeen = time;
        }
      else if (type == ORIGIN_ID)
        {
          unsigned int ASN;
          long long int time;
          fscanf (file, "%u %lld", &ASN, &time);
          struct bgp_pgbgp_origin *or = XCALLOC (MTYPE_BGP_PGBGP_ORIGIN,
                                                 sizeof (struct
                                                         bgp_pgbgp_origin));
          or->lastSeen = time;
          or->originAS = ASN;
          or->next = curNode->hist->o;
          curNode->hist->o = or;
        }
      else if (type == EDGE_ID)
        {
          bgp_pgbgp_restoreEdge (file);
        }
      else if (type == PEER_ID)
        {
          struct bgp_pgbgp_peerTime *pr;
          long long int time;
          union sockunion su;
          char szsu[128];
          fscanf (file, "%s %lld", szsu, &time);
          str2sockunion (szsu, &su);
          pr =
            XCALLOC (MTYPE_BGP_PGBGP_PEER,
                     sizeof (struct bgp_pgbgp_peerTime));
          pr->su = su;
          pr->lastSeen = time;
          pr->next = pgbgp->peerLast;
          pgbgp->peerLast = pr;
        }
    }

  fclose (file);
  return 0;
}

int
bgp_pgbgp_store (struct bgp_table *table)
{
  if (pgbgp->storage == NULL)
    return 0;
  char *tmpname = malloc (sizeof (char) * (1 + 4 + strlen (pgbgp->storage)));
  strcpy (tmpname, pgbgp->storage);
  strcat (tmpname, ".tmp");
  FILE *file = fopen (tmpname, "w");

  if (!file)
    {
      free (tmpname);
      return 0;
    }

  // Store the current time
  fprintf (file, "%lld\n", (long long int) time (NULL));

  // Store the init time
  fprintf (file, "%lld\n", (long long int) pgbgp->startTime);

  // Store the peer times
  for (struct bgp_pgbgp_peerTime * pr = pgbgp->peerLast; pr; pr = pr->next)
    {
      char strSock[128];
      sockunion2str (&pr->su, strSock, sizeof (strSock));

      if (pr->deprefUntil < time (NULL))
        {
          fprintf (file, "%d %s %lld\n", PEER_ID, strSock,
                   (long long int) pr->lastSeen);
        }
    }

  // Store the tables
  bgp_pgbgp_storeHistTable (table, file);
  bgp_pgbgp_storeEdges (table, file);

  fclose (file);

  rename (tmpname, pgbgp->storage);

  free (tmpname);
  return 0;
}

/*
  Check to see if we've seen the peer recently
  If not, then we need to return true and not delay routes
  for awhile
*/
int
bgp_pgbgp_updatePeer (struct bgp_info *binfo, time_t now)
{
  int status = false;
  // Find the peer
  struct bgp_pgbgp_peerTime *pr = pgbgp->peerLast;
  for (; pr; pr = pr->next)
    if (sockunion_same (&pr->su, &binfo->peer->su))
      break;

  // If this is a new peer, create it
  if (pr == NULL)
    {
      pr = XCALLOC (MTYPE_BGP_PGBGP_PEER, sizeof (struct bgp_pgbgp_peerTime));
      pr->su = binfo->peer->su;
      pr->next = pgbgp->peerLast;
      pgbgp->peerLast = pr;

    }
  // Is it currently marked as new?
  if (pr->deprefUntil > now)
    goto UPPEER_DEPREF;

  // Have we seen the peer recently?
  if (pr->lastSeen + pgbgp->peer_hist_time > now)
    goto UPPEER_CLEAN;

  // It must not have been seen lately, depref it
  pr->deprefUntil = now + PGBGP_PEER_GRACE;


UPPEER_DEPREF:
  status = true;

UPPEER_CLEAN:
  pr->lastSeen = now;

  return status;
}


/*
  Returns whether or not the sub-prefix should be ignored
*/
int
bgp_pgbgp_shouldIgnore (struct bgp_node *super, struct bgp_info *selected)
{
  if (!selected || CHECK_FLAG (selected->flags, BGP_INFO_SUSPICIOUS_P))
    return false;
  return true;
}

/*
  This is a special case function for smoothly handling sub-prefix hijacks.

  It handles the following 2 events:

  Event 1: The super-prefix of an anomalous prefix has a route through a non-anomalous

  Event 1: An anomalous sub-prefix is ignored, but no best route for the super-prefix exists
  Response: Announce the sub-prefix until the super-prefix comes back

  Event 2: A super-prefix comes back to the RIB and its anomalous sub-prefix is in use
  Response: Ignore the sub-prefix again
 */


int
bgp_pgbgp_rib_updated (struct bgp_node *rn, struct bgp_info *old_best,
                       struct bgp_info *new_best)
{
  //  return 0;
  struct bgp_pgbgp_hist *hist = rn->hist;
  if (!hist)
    return 0;
  if (!hist->p)
    return 0;
  time_t t_now = time (NULL);

  /*
     If we can't avoid the sub-prefix by routing to the super-prefix,
     then route as normal to the sub-prefix
   */
  if (!bgp_pgbgp_shouldIgnore (rn, new_best))
    {
      for (struct bgp_pgbgp_avoid * cur = hist->p->avoid; cur;
           cur = cur->next)
        {
          if (cur->avoidUntil > t_now)
            {
              int changed = false;
              for (struct bgp_info * ri = cur->sub->info; ri; ri = ri->next)
                {
                  if (CHECK_FLAG (ri->flags, BGP_INFO_IGNORED_P))
                    {
                      changed = true;
                      UNSET_FLAG (ri->flags, BGP_INFO_IGNORED_P);
                    }
                }
              if (changed)
                {
                  struct bgp_info *ri = cur->sub->info;
                  if (ri && ri->peer && ri->peer->bgp)
                    bgp_process (ri->peer->bgp, cur->sub,
                                 cur->sub->table->afi, cur->sub->table->safi);

                }

            }
        }
    }

  /* 
     If we can avoid the sub-prefix by routing to the super-prefix,
     then do so
   */

  else
    {
      for (struct bgp_pgbgp_avoid * cur = hist->p->avoid; cur;
           cur = cur->next)
        {
          if (cur->avoidUntil > t_now)
            {
              int changed = false;
              for (struct bgp_info * ri = cur->sub->info; ri; ri = ri->next)
                {
                  if (!CHECK_FLAG (ri->flags, BGP_INFO_IGNORED_P))
                    {
                      changed = true;
                      SET_FLAG (ri->flags, BGP_INFO_IGNORED_P);
                    }
                }
              if (changed)
                {
                  struct bgp_info *ri = cur->sub->info;
                  if (ri && ri->peer && ri->peer->bgp)
                    bgp_process (ri->peer->bgp, cur->sub,
                                 cur->sub->table->afi, cur->sub->table->safi);
                }
            }
        }
    }

  /*
     if (old_best && !new_best)
     {
     time_t t_now = time(NULL);
     for (struct bgp_pgbgp_avoid * cur = hist->p->avoid; cur;
     cur = cur->next)
     {
     if (cur->avoidUntil > t_now)
     {
     for (struct bgp_info * ri = cur->sub->info; ri; ri = ri->next)
     UNSET_FLAG (ri->flags, BGP_INFO_IGNORED_P);

     struct bgp_info *ri = cur->sub->info;
     if (ri && ri->peer && ri->peer->bgp)
     bgp_process (ri->peer->bgp, cur->sub, cur->sub->table->afi,
     cur->sub->table->safi);
     }
     }      
     }


     else if (!old_best && new_best)
     {
     time_t t_now = time(NULL);
     for (struct bgp_pgbgp_avoid * av = hist->p->avoid; av; av = av->next)
     {
     struct bgp_info * ri = av->sub->info;
     if (av->avoidUntil > t_now && ri && !CHECK_FLAG(ri->flags, BGP_INFO_IGNORED_P)) 
     {
     for (; ri; ri = ri->next)
     SET_FLAG (ri->flags, BGP_INFO_IGNORED_P);
     ri = av->sub->info;
     if (ri && ri->peer && ri->peer->bgp)
     bgp_process (ri->peer->bgp, av->sub,
     av->sub->table->afi, av->sub->table->safi);

     }
     }      
     }
   */
  return 0;
}

int
bgp_pgbgp_update (struct bgp_info *binfo, struct attr *at,
                  struct bgp_node *rn)
{
  time_t t_now = time (NULL);

  // Clean up the reuse list
  bgp_pgbgp_reuse (t_now);


  if (!rn->hist)
    {
      rn->hist =
        XCALLOC (MTYPE_BGP_PGBGP_HIST, sizeof (struct bgp_pgbgp_hist));
      // Get the PGBGP history lock on rn
      bgp_lock_node (rn);
    }

  struct bgp_node *superhn = NULL;

  // implicit lock from node_get
  superhn = findSuper (rn->table, &rn->p, t_now);

  int newPeer = bgp_pgbgp_updatePeer (binfo, t_now);
  bgp_pgbgp_updateOrigin (rn->hist, binfo, at, rn, t_now, newPeer);
  bgp_pgbgp_updatePrefix (rn->hist, superhn, binfo, at, rn, t_now, newPeer);
  bgp_pgbgp_updateEdge (rn->hist, binfo, at, rn, t_now, newPeer);

  if (superhn != NULL)
    bgp_unlock_node (superhn);



  // GC and storage must be last, as they update lastSeen values of objects
  // which would cause new routes to be recently seen, which is undesired behavior
  // Make sure you don't collect anything that might be in use!
  if (t_now >= pgbgp->lastgc + PGBGP_GC_DELTA)
    {
      bgp_pgbgp_gc (rn->table);
      pgbgp->lastgc = t_now;
    }

  if (t_now >= pgbgp->lastStore + PGBGP_STORE_DELTA)
    {
      bgp_pgbgp_store (rn->table);
      pgbgp->lastStore = t_now;
    }



  return 0;
}




/*! --------------- Public PGBGP Interface ------------------ !*/









/* --------------- MOAS Detection ------------------ */
void
bgp_pgbgp_storeHistTable (struct bgp_table *table, FILE * file)
{
  time_t t_now;
  t_now = time (NULL);

  struct bgp *bgp = bgp_get_default ();
  if (!bgp)
    return;

  // Store each AFI/SAFI RIB
  for (afi_t afi = AFI_IP; afi < AFI_MAX; afi++)
    for (safi_t safi = SAFI_UNICAST; safi < SAFI_MAX; safi++)
      {
        if (!CHECK_FLAG (bgp->af_flags[afi][safi], BGP_CONFIG_PGBGP))
          continue;
        struct bgp_table *curTable = bgp->rib[afi][safi];
        if (!curTable)
          continue;

        for (struct bgp_node * rn = bgp_table_top (curTable); rn;
             rn = bgp_route_next (rn))
          {
            struct bgp_pgbgp_hist *hist = rn->hist;
            if (hist == NULL)
              continue;
            char szPrefix[128];
            prefix2str (&rn->p, szPrefix, sizeof (szPrefix));


            struct bgp_pgbgp_prefix *pre = hist->p;
            if (pre && pre->ignoreUntil <= t_now)
              {
                if (pre->lastSeen + pgbgp->prefix_hist_time > t_now)
                  fprintf (file, "%d %s %u %u %lld\n", PREFIX_ID, szPrefix,
                           (unsigned int) afi, (unsigned int) safi,
                           (long long int) pre->lastSeen);
                else
                  continue;
              }
            /* Need a prefix in the file before the origins, 
               if no prefix.. skip origins */
            else
              continue;

            for (struct bgp_pgbgp_origin * cur = hist->o; cur;
                 cur = cur->next)
              {
                if (cur->deprefUntil > t_now)
                  continue;

                if (cur->lastSeen + pgbgp->origin_hist_time > t_now)
                  fprintf (file, "%d %u %lld\n", ORIGIN_ID, cur->originAS,
                           (long long int) cur->lastSeen);
              }

          }
      }
}


int
bgp_pgbgp_garbageCollectHistTable (struct bgp_table *table)
{
  time_t t_now;
  t_now = time (NULL);


  for (struct bgp_node * rn = bgp_table_top (table); rn;
       rn = bgp_route_next (rn))
    {
      int collect = false;
      struct bgp_pgbgp_hist *hist = rn->hist;
      if (hist == NULL)
        continue;

      struct bgp_pgbgp_origin *cur = hist->o;
      struct bgp_pgbgp_prefix *pre = hist->p;
      struct bgp_pgbgp_origin *parent = NULL;

      int used = false;
      if (cur != NULL || pre != NULL)
        used = true;

      while (cur != NULL)
        {
          // Update the lastSeen time w/ originInRIB
          if (originInRIB (rn, cur))
            cur->lastSeen = t_now;

          collect = false;

          // Collect if old
          if (cur->lastSeen + pgbgp->origin_hist_time <= t_now)
            collect = true;

          // Collect if anomaly just became okay but not seen since last collection
          if (cur->deprefUntil != 0 && cur->deprefUntil < t_now)
            {
              if (cur->lastSeen < pgbgp->lastgc)
                collect = true;
              cur->deprefUntil = 0;
            }

          if (collect)
            {
              if (parent == NULL)
                hist->o = cur->next;
              else
                parent->next = cur->next;

              // Delete cur, parent doesn't change
              struct bgp_pgbgp_origin *del = cur;
              cur = cur->next;
              XFREE (MTYPE_BGP_PGBGP_ORIGIN, del);
            }
          else
            {
              parent = cur;
              cur = cur->next;
            }
        }

      // Update the lastSeen time w/ prefixInRIB
      if (pre && prefixInRIB (rn, pre))
        pre->lastSeen = t_now;

      collect = false;

      // Collect if old
      if (pre && pre->lastSeen + pgbgp->prefix_hist_time <= t_now)
        collect = true;

      // Collect if anomaly just became okay but not seen since last collection
      if (pre && pre->ignoreUntil != 0 && pre->ignoreUntil < t_now)
        {
          if (pre->lastSeen < pgbgp->lastgc)
            collect = true;
          pre->ignoreUntil = 0;
        }

      if (collect)
        {
          for (struct bgp_pgbgp_avoid * av = pre->avoid; av;)
            {
              struct bgp_pgbgp_avoid *del = av;
              av = av->next;
              bgp_unlock_node (del->sub);
              XFREE (MTYPE_BGP_PGBGP_AVOID, del);
            }

          XFREE (MTYPE_BGP_PGBGP_PREFIX, pre);
          hist->p = NULL;
        }

      // If the node isn't in use, remove it
      if (used && hist->o == NULL && hist->p == NULL)
        {
          XFREE (MTYPE_BGP_PGBGP_HIST, hist);
          rn->hist = NULL;
          bgp_unlock_node (rn);
        }
    }

  return 0;
}

void
bgp_pgbgp_cleanHistTable (struct bgp_table *table)
{
  // Clean up the RIB nodes
  for (struct bgp_node * rn = bgp_table_top (table); rn;
       rn = bgp_route_next (rn))
    {
      struct bgp_pgbgp_hist *hist = rn->hist;
      if (hist == NULL)
        continue;

      if (hist->p)
        {
          for (struct bgp_pgbgp_avoid * av = hist->p->avoid; av;)
            {
              struct bgp_pgbgp_avoid *del = av;
              av = av->next;
              bgp_unlock_node (del->sub);
              XFREE (MTYPE_BGP_PGBGP_AVOID, del);
            }
          hist->p->avoid = NULL;
          XFREE (MTYPE_BGP_PGBGP_PREFIX, hist->p);
          hist->p = NULL;
        }

      for (struct bgp_pgbgp_origin * cur = hist->o; cur;)
        {
          struct bgp_pgbgp_origin *next = cur->next;
          XFREE (MTYPE_BGP_PGBGP_ORIGIN, cur);
          cur = next;
        }
      hist->o = NULL;
      XFREE (MTYPE_BGP_PGBGP_HIST, hist);
      rn->hist = NULL;
      bgp_unlock_node (rn);
    }
}

void
bgp_pgbgp_logOriginAnomaly (as_t asn, struct bgp_node *rn, struct attr *at)
{
  assert (pgbgp);
  if (!pgbgp->anomalies)
    return;
  FILE *file = fopen (pgbgp->anomalies, "a");
  if (!file)
    return;

  char pre[256];
  prefix2str (&rn->p, pre, sizeof (pre));

  // MOAS | TIME | NEXTHOP | PREFIX | SUSPICIOUS_ORIGIN | TRUSTED_ORIGINS | PATH
  fprintf (file, "%d|%lld|%s|%s|%d|", MOAS, (long long int) time (NULL),
           inet_ntoa (at->nexthop), pre, asn);


  // Print the trusted origins
  assert (rn->hist);
  assert (rn->hist->o);

  struct bgp_pgbgp_hist *hist = rn->hist;

  for (struct bgp_pgbgp_origin * cur = hist->o; cur != NULL; cur = cur->next)
    {
      if (cur->deprefUntil > time (NULL))
        continue;
      fprintf (file, "%d", cur->originAS);
      if (cur->next != NULL)
        fprintf (file, " ");
    }

  fprintf (file, " |%s\n", aspath_print (at->aspath));
  fclose (file);
}

int
bgp_pgbgp_updateOrigin (struct bgp_pgbgp_hist *hist, struct bgp_info *binfo,
                        struct attr *at, struct bgp_node *rn, time_t t_now,
                        int newPeer)
{
  struct bgp_pgbgp_pathSet pathOrigins;
  struct bgp_pgbgp_origin *pi = NULL;
  int status = 0;
  struct bgp_pgbgp_reuse *r;
  pathOrigins = bgp_pgbgp_pathOrigin (at->aspath);


  for (int i = 0; i < pathOrigins.length; i++)
    {
      as_t pathOrigin = pathOrigins.ases[i];

      /* Is the Origin AS in the history? */
      for (pi = hist->o; pi; pi = pi->next)
        if (pi->originAS == pathOrigin)
          break;

      if (pi == NULL)
        {
          pi =
            XCALLOC (MTYPE_BGP_PGBGP_ORIGIN,
                     sizeof (struct bgp_pgbgp_origin));
          pi->next = hist->o;
          pi->originAS = pathOrigin;
          hist->o = pi;
        }

      // If this is our first origin for the prefix, let the sub-prefix
      // check take care of it
      if (pi->next == NULL)
        goto UPO_CLEAN;

      /* Is the origin currently marked as suspicious? */
      if (pi->deprefUntil > t_now)
        goto UPO_DEPREF;

      /* Have we seen the origin recently? */
      if (pi->lastSeen + pgbgp->origin_hist_time > t_now)
        goto UPO_CLEAN;

#ifndef PGBGP_DEBUG
      /* Are we within the initial grace period? */
      if (newPeer)
        goto UPO_CLEAN;
#endif

      /* It must not be in recent history, depref origin for first time */
      pi->deprefUntil = t_now + pgbgp->origin_sus_time;
      bgp_pgbgp_logOriginAnomaly (pathOrigin, rn, at);

      r = XCALLOC (MTYPE_BGP_PGBGP_REUSE, sizeof (struct bgp_pgbgp_reuse));
      r->type = PGBGP_REUSE_ORIGIN;
      r->deprefUntil = pi->deprefUntil;
      r->data.origin.originAS = pathOrigin;
      r->data.origin.rn = rn;
      bgp_lock_node (rn);
      pqueue_enqueue (r, pgbgp->reuse_q);
      pgbgp->rq_size += 1;


    UPO_DEPREF:
      SET_FLAG (binfo->flags, BGP_INFO_SUSPICIOUS_O);
      status = BGP_INFO_SUSPICIOUS_O;

    UPO_CLEAN:
      pi->lastSeen = t_now;
    }
  return status;
}

int
bgp_pgbgp_reuseOrigin (struct bgp_pgbgp_r_origin data)
{
  struct bgp_info *ri;
  int numChanged = 0;
  time_t t_now = time (NULL);
  assert (data.rn->hist != NULL);

  // Repreference paths for this prefix that are now okay
  for (ri = data.rn->info; ri; ri = ri->next)
    {
      if (CHECK_FLAG (ri->flags, BGP_INFO_SUSPICIOUS_O))
        {
          struct bgp_pgbgp_pathSet pathOrigins;
          pathOrigins = bgp_pgbgp_pathOrigin (ri->attr->aspath);
          int numOkay = 0;
          for (int i = 0; i < pathOrigins.length; i++)
            {
              as_t pathOrigin = pathOrigins.ases[i];
              // Find the origin
              struct bgp_pgbgp_origin *o = NULL;
              for (o = data.rn->hist->o; o != NULL; o = o->next)
                if (o->originAS == pathOrigin)
                  break;
              /*
                 if (o == NULL) {
                 for(struct bgp_pgbgp_origin * z = data.rn->hist->o; z != NULL; z = z->next)
                 printf("Known origin: %d\n", z->originAS);
                 char pre[128];
                 prefix2str(&data.rn->p, pre, 128);
                 printf("%s : %s : %d\n", pre, ri->attr->aspath->str, pathOrigin);
                 }
               */
              assert (o != NULL);

              if (o->deprefUntil <= t_now)
                numOkay += 1;
            }
          if (numOkay == pathOrigins.length)
            {
              UNSET_FLAG (ri->flags, BGP_INFO_SUSPICIOUS_O);
              numChanged += 1;
            }
        }
    }

  ri = data.rn->info;

  // Rerun the decision process?
  if (numChanged > 0)
    bgp_process (ri->peer->bgp, data.rn, data.rn->table->afi,
                 data.rn->table->safi);


  /*
     // Remove this (origin,prefix) pair from the normal database
     // if it's not still in the RIB
     struct bgp_pgbgp_hist *hist = rn->hist;
     struct bgp_pgbgp_origin * cur = hist->o;
     struct bgp_pgbgp_origin * parent = NULL;

     // Find the origin AS node
     while(cur != NULL)
     {
     if (cur->originAS == data.originAS)
     {
     // Delete the node if it hasn't been seen
     // since the last storage run
     if (cur->lastSeen < pgbgp->lastStore) {
     // Delete this node
     if (parent == NULL)
     hist->o = cur->next;
     else
     parent->next = cur->next;

     XFREE(MTYPE_BGP_PGBGP_ORIGIN, cur);
     }
     break;
     }      
     parent = cur;
     cur = cur->next;
     }
   */

  bgp_unlock_node (data.rn);
  return 0;
}

/*! --------------- MOAS Detection ------------------ !*/


/* --------------- Sub-Prefix Detection ------------------ */





void
bgp_pgbgp_logSubprefixAnomaly (as_t asn, struct bgp_node *rn, struct attr *at,
                               struct bgp_node *super)
{
  assert (pgbgp);
  if (!pgbgp->anomalies)
    return;
  FILE *file = fopen (pgbgp->anomalies, "a");
  if (!file)
    return;

  char pre[256];
  prefix2str (&rn->p, pre, sizeof (pre));

  char superpre[256];
  prefix2str (&super->p, superpre, sizeof (superpre));

  // SUBPREFIX | TIME | NEXTHOP | PREFIX | SUPER-PREFIX | SUSPICIOUS_ORIGIN | TRUSTED_ORIGINS | PATH
  fprintf (file, "%d|%lld|%s|%s|%s|%d|", SUBPREFIX,
           (long long int) time (NULL), inet_ntoa (at->nexthop), pre,
           superpre, asn);

  // Print the trusted origins
  assert (super->hist);
  assert (super->hist->o);

  struct bgp_pgbgp_hist *hist = super->hist;

  for (struct bgp_pgbgp_origin * cur = hist->o; cur != NULL; cur = cur->next)
    {
      if (cur->deprefUntil > time (NULL))
        continue;
      fprintf (file, "%d", cur->originAS);
      if (cur->next != NULL)
        fprintf (file, " ");
    }

  fprintf (file, " |%s\n", aspath_print (at->aspath));
  fclose (file);
}

/*
  If the first path is a prefix of the second, then return true  
 */

static int
bgp_pgbgp_pathIsPrefix(struct aspath *trusted, struct aspath * new)
{
  if (trusted == new)
    return true;
  
  struct assegment *seg1 = trusted->segments;
  struct assegment *seg2 = new->segments;
  
  while (seg1 || seg2)
    {
      if ((!seg1 && seg2) || (seg1 && !seg2))
	return false;
      if (seg1->type != seg2->type)
	return false;
      
      if (seg1->length > seg2->length)
	return false;
	  
      for(int i = 0; i < seg1->length; i++)
	if (seg1->as[i] != seg2->as[i])
	  return false;

      seg1 = seg1->next;
      seg2 = seg2->next;
    }  

  return true;
}

int
bgp_pgbgp_updatePrefix (struct bgp_pgbgp_hist *hist,
                        struct bgp_node *supernode, struct bgp_info *binfo,
                        struct attr *at, struct bgp_node *rn, time_t t_now,
                        int newPeer)
{
  struct bgp_pgbgp_prefix *pre = NULL;
  struct bgp_pgbgp_reuse *r = NULL;
  int status = 0;
  int changed = false;

  pre = hist->p;


  /* Do we have this prefix? */
  if (pre == NULL)
    {
      pre =
        XCALLOC (MTYPE_BGP_PGBGP_PREFIX, sizeof (struct bgp_pgbgp_prefix));
      hist->p = pre;
    }

  /* Is the prefix currently marked as suspicious? */
  if (pre->ignoreUntil > t_now)
    {
      goto UPP_IGNORE;
    }

  /* Should this neighbor be avoided for this prefix because it
     sent us info. about a suspicious sub-prefix? */
  for (struct bgp_pgbgp_avoid * av = hist->p->avoid; av; av = av->next)
    {
      if (binfo->peer->as == av->peerASN && av->avoidUntil > t_now)
        {
          SET_FLAG (binfo->flags, BGP_INFO_SUSPICIOUS_P);
          status = BGP_INFO_SUSPICIOUS_P;
          goto UPP_DONE;
        }
    }

  /* Have we seen the prefix recently? */
  if (pre->lastSeen + pgbgp->prefix_hist_time > t_now)
    goto UPP_DONE;

#ifndef PGBGP_DEBUG
  /* Are we within the initial grace period? */
  if (newPeer)
    goto UPP_DONE;
#endif

  /* Is there a less specific *in recent history* that this could be hijacking? */
  if (supernode == NULL)
    goto UPP_DONE;

  /* Does this path the super-net's non-anomalous path from this peer?  If so it's okay */
  int found = false;
  for (struct bgp_info * ri = supernode->info; ri; ri = ri->next)
    {
      if (ri->peer->as == binfo->peer->as)
	{
	  if (!ANOMALOUS(ri->flags) && bgp_pgbgp_pathIsPrefix(ri->attr->aspath, at->aspath))
	      found = true;
	  break;
	}
    }

  if (found)
    goto UPP_DONE;

  /* 
     It's not in recent history, and there is a less specific currently in use
     Response:
     . Ignore this prefix
     . Make the less specific's route for this neighbor suspicious
   */


  pre->ignoreUntil = t_now + pgbgp->sub_sus_time;

  struct bgp_pgbgp_pathSet pathOrigins;
  pathOrigins = bgp_pgbgp_pathOrigin (at->aspath);
  for (int i = 0; i < pathOrigins.length; i++)
    bgp_pgbgp_logSubprefixAnomaly (pathOrigins.ases[i], rn, at, supernode);



  r = XCALLOC (MTYPE_BGP_PGBGP_REUSE, sizeof (struct bgp_pgbgp_reuse));
  r->type = PGBGP_REUSE_PREFIX;
  r->deprefUntil = pre->ignoreUntil;
  r->data.prefix.rn = rn;
  r->data.prefix.rnsuper = supernode;
  bgp_lock_node (rn);
  bgp_lock_node (supernode);
  pqueue_enqueue (r, pgbgp->reuse_q);
  pgbgp->rq_size += 1;

UPP_IGNORE:
  // Sanity check
  if (supernode == NULL)
    goto UPP_DONE;
    
  /* Set the less specific's route from this peer to suspicious */
  changed = false;

  for (struct bgp_info * ri = supernode->info; ri; ri = ri->next)
    {
      if (ri->peer->as == binfo->peer->as)
        {
          if (!CHECK_FLAG (ri->flags, BGP_INFO_SUSPICIOUS_P))
            {
              SET_FLAG (ri->flags, BGP_INFO_SUSPICIOUS_P);
              changed = true;
            }
          break;
        }
    }

  // Make note of it in the less specific's history information
  found = false;
  struct bgp_pgbgp_hist *superhist = supernode->hist;

  if (superhist && superhist->p)
    {
      for (struct bgp_pgbgp_avoid * av = superhist->p->avoid; av;
           av = av->next)
        {
          if (av->peerASN == binfo->peer->as)
            {
              if (av->avoidUntil < pre->ignoreUntil)
                av->avoidUntil = pre->ignoreUntil;
              found = true;
              break;
            }
        }
      if (!found)
        {
          struct bgp_pgbgp_avoid *newavoid =
            XCALLOC (MTYPE_BGP_PGBGP_AVOID, sizeof (struct bgp_pgbgp_avoid));
          newavoid->peerASN = binfo->peer->as;
          newavoid->avoidUntil = pre->ignoreUntil;
          newavoid->next = superhist->p->avoid;
          newavoid->sub = rn;
          bgp_lock_node (rn);
          superhist->p->avoid = newavoid;
        }
    }
  /* 
     ignore this route unless the supernet's node
     is only a placeholder from loaded pgbgp data
   */
  if (bgp_pgbgp_shouldIgnore (supernode, bgp_pgbgp_selected (supernode)))
    {
      SET_FLAG (binfo->flags, BGP_INFO_IGNORED_P);
      status = BGP_INFO_IGNORED_P;
    }
  if (changed)
    {
      struct bgp_info *ri = supernode->info;
      bgp_process (ri->peer->bgp, supernode, supernode->table->afi,
                   supernode->table->safi);
    }

UPP_DONE:
  pre->lastSeen = t_now;

  return status;
}

int
bgp_pgbgp_reusePrefix (struct bgp_pgbgp_r_prefix data)
{
  struct bgp_info *ri = NULL;

  time_t t_now = time (NULL);

  // Repreference all routes for this node
  for (ri = data.rn->info; ri; ri = ri->next)
    UNSET_FLAG (ri->flags, BGP_INFO_IGNORED_P);
  ri = data.rn->info;

  // Rerun the decision process
  if (ri != NULL)
    bgp_process (ri->peer->bgp, data.rn, data.rn->table->afi,
                 data.rn->table->safi);


  // Remove the avoid nodes from the super
  struct bgp_pgbgp_hist *superhist = data.rnsuper->hist;
  if (superhist != NULL && superhist->p != NULL)
    {
      struct bgp_pgbgp_avoid *parent = NULL;
      for (struct bgp_pgbgp_avoid * av = superhist->p->avoid; av;)
        {
          int numChanged = 0;
          if (av->avoidUntil <= t_now)
            {
              struct bgp_pgbgp_avoid *del = av;
              av = av->next;
              if (parent == NULL)
                superhist->p->avoid = av;
              else
                parent->next = av;

              // Repreference any routes
              for (ri = data.rnsuper->info; ri; ri = ri->next)
                {
                  if (ri->peer->as == del->peerASN)
                    {
                      UNSET_FLAG (ri->flags, BGP_INFO_SUSPICIOUS_P);
                      numChanged += 1;
                      break;
                    }
                }
              ri = data.rnsuper->info;

              if (numChanged > 0 && ri != NULL)
                bgp_process (ri->peer->bgp, data.rnsuper,
                             data.rnsuper->table->afi,
                             data.rnsuper->table->safi);
              bgp_unlock_node (del->sub);
              XFREE (MTYPE_BGP_PGBGP_AVOID, del);
            }
          else
            {
              parent = av;
              av = av->next;
            }
        }
    }

  // Remove this prefix from the normal database
  // if it hasn't been seen in the RIB since the last
  // storage run
  /*
     struct bgp_pgbgp_hist *hist = rn->hist;
     struct bgp_pgbgp_prefix * pre = hist->p;

     if (pre && pre->lastSeen < pgbgp->lastStore)
     {
     // Delete this node
     for(struct bgp_pgbgp_avoid * av = hist->p->avoid; av;)
     {
     struct bgp_pgbgp_avoid *del = av;
     av = av->next;
     bgp_unlock_node(del->sub);
     XFREE (MTYPE_BGP_PGBGP_AVOID, del);
     }
     XFREE(MTYPE_BGP_PGBGP_PREFIX, pre);
     hist->p = NULL;      
     }
   */
  bgp_unlock_node (data.rn);
  bgp_unlock_node (data.rnsuper);
  return 0;
}

/*! --------------- Sub-Prefix Detection ------------------ !*/





/* --------------- Edge Detection ------------------ */

static void
edge_store_clear_iterator (struct hash_backet *backet, void *file)
{
  struct bgp_pgbgp_edge *hedge = backet->data;
}

static void
edge_store_iterator (struct hash_backet *backet, FILE * file)
{
  struct bgp_pgbgp_edge *hedge = backet->data;
  time_t t_now = time (NULL);
  if (hedge->deprefUntil > t_now)
    return;
  if (hedge->lastSeen + pgbgp->edge_hist_time > t_now)
    {
      fprintf (file, "%d %u %u %lld\n", EDGE_ID, hedge->e.a, hedge->e.b,
               (long long int) hedge->lastSeen);
    }
}


void
bgp_pgbgp_storeEdges (struct bgp_table *table, FILE * file)
{
  hash_iterate (pgbgp->edgeT,
                (void (*)(struct hash_backet *, void *))
                edge_store_iterator, file);
  return;
}


int
bgp_pgbgp_restoreEdge (FILE * file)
{
  unsigned int a, b;
  long long int lastSeen;
  fscanf (file, "%u %u %lld", &a, &b, &lastSeen);
  struct bgp_pgbgp_edge finder;
  finder.e.a = a;
  finder.e.b = b;
  finder.lastSeen = lastSeen;
  struct bgp_pgbgp_edge *hedge =
    hash_get (pgbgp->edgeT, &finder, edge_hash_alloc);
  hedge->lastSeen = finder.lastSeen;
  return 0;
}

unsigned int
edge_key_make (void *p)
{
  struct bgp_pgbgp_edge *pe = p;
  struct edge *e = &pe->e;
  return (e->a << 16) + e->b;
}

static int
edge_cmp (const void *arg1, const void *arg2)
{

  const struct edge *e1 = &((const struct bgp_pgbgp_edge *) arg1)->e;
  const struct edge *e2 = &((const struct bgp_pgbgp_edge *) arg2)->e;
  if (e1->a == e2->a && e1->b == e2->b)
    return 1;
  return 0;
}

static void *
edge_hash_alloc (void *arg)
{
  struct bgp_pgbgp_edge *hedge =
    XCALLOC (MTYPE_BGP_PGBGP_EDGE, sizeof (struct bgp_pgbgp_edge));
  struct bgp_pgbgp_edge *lookup = arg;
  if (hedge == NULL)
    return NULL;
  hedge->e = lookup->e;
  return hedge;
}


static void
edge_gc_iterator (struct hash_backet *backet, time_t * time)
{
  time_t t_now = *time;
  struct bgp_pgbgp_edge *hedge = backet->data;

  int collect = false;

  // Collect if we haven't seen it in awhile
  if (hedge->lastSeen + pgbgp->edge_hist_time <= t_now)
    collect = true;

  // Collect if it has just gotten out of anomaly stage
  // but hasn't been in the RIB since the last GC
  if (hedge->deprefUntil != 0 && hedge->deprefUntil < t_now)
    {
      if (hedge->lastSeen < pgbgp->lastgc)
        collect = true;
      hedge->deprefUntil = 0;
    }

  if (collect)
    {
      struct bgp_pgbgp_edge *ret = hash_release (pgbgp->edgeT, hedge);
      assert (ret != NULL);
      XFREE (MTYPE_BGP_PGBGP_EDGE, hedge);
    }
}



static void
edge_update_iterator (struct hash_backet *backet, void *v)
{
  struct aspath *p = backet->data;
  time_t t_now = time (NULL);
  int first = true;

  struct edge cur;
  cur.a = 0;
  cur.b = 0;
  struct assegment *seg;
  struct bgp_pgbgp_edge *hedge = NULL;
  for (seg = p->segments; seg; seg = seg->next)
    {
      for (int i = 0; i < seg->length; i++)
        {
          cur.a = cur.b;
          cur.b = seg->as[i];
          if (first)
            {
              first = false;
              continue;
            }
          if (cur.a == cur.b)
            continue;
          //              printf("%d -- %d\n", cur.a, cur.b);
          struct bgp_pgbgp_edge finder;
          finder.e = cur;
          hedge = hash_lookup (pgbgp->edgeT, &finder);

          if (!hedge)
            continue;
          hedge->lastSeen = t_now;
        }
    }
}

int
bgp_pgbgp_garbageCollectEdges (struct bgp_table *table)
{
  // Update the timings
  hash_iterate (ashash,
                (void (*)(struct hash_backet *, void *))
                edge_update_iterator, NULL);

  // Perform the collection
  time_t t_now = time (NULL);
  hash_iterate (pgbgp->edgeT,
                (void (*)(struct hash_backet *, void *))
                edge_gc_iterator, &t_now);
  return 0;
}

static void
edge_clean_iterator (struct hash_backet *backet, void *a1)
{
  struct bgp_pgbgp_edge *hedge = backet->data;
  struct bgp_pgbgp_edge *ret = hash_release (pgbgp->edgeT, hedge);
  assert (ret != NULL);
  XFREE (MTYPE_BGP_PGBGP_EDGE, hedge);
}

static void
bgp_pgbgp_cleanEdges (void)
{
  if (pgbgp->edgeT != NULL)
    {
      hash_iterate (pgbgp->edgeT,
                    (void (*)(struct hash_backet *, void *))
                    edge_clean_iterator, NULL);
      hash_free (pgbgp->edgeT);
    }
  return;
}

void
bgp_pgbgp_logEdgeAnomaly (struct bgp_node *rn, struct attr *at,
                          struct edge *edge)
{
  assert (pgbgp);
  if (!pgbgp->anomalies)
    return;
  FILE *file = fopen (pgbgp->anomalies, "a");
  if (!file)
    return;

  char pre[256];
  prefix2str (&rn->p, pre, sizeof (pre));

  // EDGE | TIME | NEXTHOP | PREFIX | PATH | Edge.a | Edge.b

  fprintf (file, "%d|%lld|%s|%s|%s|%d|%d\n", EDGE,
           (long long int) time (NULL), inet_ntoa (at->nexthop), pre,
           aspath_print (at->aspath), edge->a, edge->b);

  fclose (file);
}


int
bgp_pgbgp_updateEdge (struct bgp_pgbgp_hist *hist, struct bgp_info *binfo,
                      struct attr *at, struct bgp_node *rn, time_t t_now,
                      int newPeer)
{

  char first = true;
  struct edge curEdge;
  curEdge.a = 0;
  curEdge.b = 0;


  if (at->aspath == NULL)
    return 0;
  struct assegment *seg = at->aspath->segments;
  if (seg == NULL)
    return 0;
  time_t max_depref = 0;
  for (seg = at->aspath->segments; seg; seg = seg->next)
    {
      for (int i = 0; i < seg->length; i++)
        {
          curEdge.a = curEdge.b;
          curEdge.b = seg->as[i];
          if (first)
            {
              first = false;
              continue;
            }
          if (curEdge.a == curEdge.b)
            continue;

          // We have an edge to consider
          struct bgp_pgbgp_edge finder;
          finder.e = curEdge;
          struct bgp_pgbgp_edge *hedge =
            hash_get (pgbgp->edgeT, &finder, edge_hash_alloc);

          // Is this edge marked as suspicious?
          if (hedge->deprefUntil > t_now)
            goto UPE_DEPREF;

          // Have we seen the edge recently?
          if (hedge->lastSeen + pgbgp->edge_hist_time > t_now)
            goto UPE_CLEAN;
#ifndef PGBGP_DEBUG
          /* Are we within the initial grace period? */
          if (newPeer)
            goto UPE_CLEAN;
#endif
          // It must not be in recent history, depref edge for first time
          hedge->deprefUntil = t_now + pgbgp->edge_sus_time;
          bgp_pgbgp_logEdgeAnomaly (rn, at, &curEdge);


        UPE_DEPREF:
          if (hedge->deprefUntil > max_depref)
            max_depref = hedge->deprefUntil;
        UPE_CLEAN:
          hedge->lastSeen = t_now;
        }
    }
  if (max_depref)
    {
      SET_FLAG (binfo->flags, BGP_INFO_SUSPICIOUS_E);
      if (!hist->pEdgeReuse)
        {
          struct bgp_pgbgp_reuse *r;
          r =
            XCALLOC (MTYPE_BGP_PGBGP_REUSE, sizeof (struct bgp_pgbgp_reuse));
          r->type = PGBGP_REUSE_EDGE;
          r->deprefUntil = max_depref;
          r->data.edge.rn = rn;
          bgp_lock_node (rn);
          pqueue_enqueue (r, pgbgp->reuse_q);
          pgbgp->rq_size += 1;
          hist->pEdgeReuse = r;
        }
      return BGP_INFO_SUSPICIOUS_E;
    }

  return 0;
}

int
bgp_pgbgp_reuseEdge (struct bgp_pgbgp_r_edge data)
{

  // Okay, go through all of the paths for the prefix
  // and find the path that needs to be updated next and
  // enqueue it
  time_t minMax = 0;
  int numChanged = 0;
  time_t t_now = time (NULL);

  for (struct bgp_info * ri = data.rn->info; ri; ri = ri->next)
    {
      char first = true;
      struct edge curEdge = { 0, 0 };
      struct assegment *seg;
      time_t max_depref = 0;

      for (seg = ri->attr->aspath->segments; seg; seg = seg->next)
        {
          for (int i = 0; i < seg->length; i++)
            {
              curEdge.a = curEdge.b;
              curEdge.b = seg->as[i];
              if (first)
                {
                  first = false;
                  continue;
                }
              struct bgp_pgbgp_edge finder;
              finder.e = curEdge;
              struct bgp_pgbgp_edge *hedge =
                hash_lookup (pgbgp->edgeT, &finder);
              if (!hedge)
                continue;
              // Is this edge suspicious?
              if (hedge->deprefUntil > t_now
                  && hedge->deprefUntil > max_depref)
                max_depref = hedge->deprefUntil;
            }
        }

      if (max_depref)
        {
          if (!minMax || max_depref < minMax)
            minMax = max_depref;
        }
      else
        {
          if (CHECK_FLAG (ri->flags, BGP_INFO_SUSPICIOUS_E))
            {
              UNSET_FLAG (ri->flags, BGP_INFO_SUSPICIOUS_E);
              numChanged += 1;
            }
        }
    }
  struct bgp_info *ri = data.rn->info;
  if (numChanged > 0 && ri)
    bgp_process (ri->peer->bgp, data.rn, data.rn->table->afi,
                 data.rn->table->safi);

  struct bgp_pgbgp_hist *hist = data.rn->hist;
  hist->pEdgeReuse = NULL;

  if (minMax)
    {
      struct bgp_pgbgp_reuse *r;
      r = XCALLOC (MTYPE_BGP_PGBGP_REUSE, sizeof (struct bgp_pgbgp_reuse));
      r->type = PGBGP_REUSE_EDGE;
      r->deprefUntil = minMax;
      r->data.edge.rn = data.rn;
      pqueue_enqueue (r, pgbgp->reuse_q);
      pgbgp->rq_size += 1;
      hist->pEdgeReuse = r;
    }
  else
    {
      bgp_unlock_node (data.rn);
    }

  return 0;
}
