/* BGP Pretty Good BGP
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
02111-1307, USA.  */

#ifndef _QUAGGA_BGP_PGBGP_H
#define _QUAGGA_BGP_PGBGP_H

#include "bgpd.h"
#include "bgp_route.h"
#include "table.h"

#define MOAS 0
#define SUBPREFIX 1
#define EDGE 2

/* Global PGBGP data */
struct bgp_pgbgp_config
{
  /* Depref time for a new origin AS */
  time_t origin_sus_time;

  /* Depref time for a new edge */
  time_t edge_sus_time;

  /* Depref time for a new sub-prefix */
  time_t sub_sus_time;

  /* Origin AS Mapping History Length */
  time_t origin_hist_time;

  /* Prefix Mapping History Length */
  time_t prefix_hist_time;

  /* Edge Mapping History Length */
  time_t edge_hist_time;

  /* Peer Mapping History Length */
  time_t peer_hist_time;

  /* The list of depreferenced routes */
  struct pqueue *reuse_q;
  int rq_size;

  /* Time that the last garbage collection (gc) took place */
  time_t lastgc;

  /* History table */
  //    struct route_table *histT;

  /* Edge Hash Table */
  struct hash *edgeT;

  /* File path for history storage */
  char *storage;

  /* File path for dump of anomalous routes */
  char *anomalies;

  /* The time that we last stored to disk */
  time_t lastStore;

  /* The time that PGBGP started counting */
  time_t startTime;

  /* Last time each peer was seen */
  struct bgp_pgbgp_peerTime *peerLast;

};


struct bgp_pgbgp_peerTime
{
  struct bgp_pgbgp_peerTime *next;
  time_t lastSeen;
  union sockunion su;
  time_t deprefUntil;
};

struct edge
{
  as_t a;
  as_t b;
};

/*
  Avoid the neighbors for the less specific that told you about
  the more specific
 */
struct bgp_pgbgp_avoid
{
  struct bgp_pgbgp_avoid *next;
  time_t avoidUntil;
  as_t peerASN;
  struct bgp_node *sub;
};

/* A list of origin ASes for a path
   Usually it's only one but if the last AS
   in the path is an AS set, then the whole
   set must be returned 
*/
struct bgp_pgbgp_pathSet
{
  int length;
  as_t *ases;
};

/*
  Avoid paths with suspicious origins
 */
struct bgp_pgbgp_origin
{
  struct bgp_pgbgp_origin *next;
  time_t lastSeen;
  time_t deprefUntil;
  as_t originAS;
};

/*
  Ignore routes for this prefix
 */
struct bgp_pgbgp_prefix
{
  time_t lastSeen;
  time_t ignoreUntil;
  struct bgp_pgbgp_avoid *avoid;
};

struct bgp_pgbgp_edge
{
  time_t lastSeen;
  time_t deprefUntil;
  struct edge e;
};

struct bgp_pgbgp_hist
{
  struct bgp_pgbgp_origin *o;
  struct bgp_pgbgp_prefix *p;
  struct bgp_pgbgp_reuse *pEdgeReuse;
};

struct bgp_pgbgp_r_origin
{
  as_t originAS;
  struct bgp_node *rn;
};

struct bgp_pgbgp_r_prefix
{
  struct bgp_node *rn;
  struct bgp_node *rnsuper;
};

/*
  This node contained a route with a bad edge, check 
  it again for bad edges in 24 hours
*/
struct bgp_pgbgp_r_edge
{
  struct bgp_node *rn;
};


union reuseTypes
{
  struct bgp_pgbgp_r_origin origin;
  struct bgp_pgbgp_r_prefix prefix;
  struct bgp_pgbgp_r_edge edge;
};

struct bgp_pgbgp_reuse
{
  union reuseTypes data;
  short type;
  time_t deprefUntil;
};

#define ANOMALOUS(V) \
(CHECK_FLAG(V, BGP_INFO_SUSPICIOUS_O | BGP_INFO_SUSPICIOUS_P \
	    | BGP_INFO_SUSPICIOUS_E | BGP_INFO_IGNORED_P))

#define PGBGP_REUSE_ORIGIN 0
#define PGBGP_REUSE_PREFIX 1
#define PGBGP_REUSE_EDGE 2

#define BGP_PGBGP_NONE      0
#define BGP_PGBGP_DEPREFFED 1

// For storage
#define ORIGIN_ID 0
#define PREFIX_ID 1
#define EDGE_ID 2
#define PEER_ID 3

/* Default timing values */
#define DEFAULT_ORIGIN_SUS       (86400 * 1)
#define DEFAULT_EDGE_SUS         (86400 * 1)
#define DEFAULT_SUB_SUS          (86400 * 1)
#define DEFAULT_ORIGIN_HIST      (86400 * 30)
#define DEFAULT_PREFIX_HIST      (86400 * 10)
#define DEFAULT_EDGE_HIST        (86400 * 60)
// Time between garbage collections
#define PGBGP_GC_DELTA           (3600)
// Time between file stores
#define PGBGP_STORE_DELTA        (28800)
// Time that a new peer's routes are not considered suspicious
#define PGBGP_PEER_GRACE         (86400 * 1)



///////// PUBLIC PGBGP FUNCTIONS /////////

/*
  bgp_pgbgp_enable:
  Enable PGBGP depreferencing / history tracking for this afi/safi
  
  Arguments:
  . ost: Depref. time of new prefix origins (in hours)
  . est: Depref. time of new edges (in hours)
  . sst: Depref. time of new sub-prefixes (in hours)
  . oht: Storage time of known origins for prefixes (in days)
  . pht: Storage time of known prefixes (in days)
  . eht: Storage time of known edges (in days)
  . storage: File to periodically store history in (can be /dev/null)
  . anoms: File to store history of depreferenced routes (can be /dev/null)

  Caution:
  It is important that the storage times are longer than the depreference times
*/
extern int bgp_pgbgp_enable (struct bgp *, afi_t afi, safi_t safi, int ost,
                             int est, int sst, int oht, int pht, int eht,
                             const char *storage, const char *anoms);
extern int bgp_pgbgp_disable (struct bgp *, afi_t afi, safi_t safi);

/*
  bgp_pgbgp_update:
  Call on the event of an announcement update
  
  Arguments:
  bgp_info: The route
  at: The new route's attributes
*/
extern int bgp_pgbgp_update (struct bgp_info *, struct attr *at,
                             struct bgp_node *);

/*
  bgp_pgbgp_rib_updated:
  Call upon discovery of a new best path (or lack thereof)

  This is a special case function for smoothly handling sub-prefix hijacks.

  It handles the following 2 events:

  Event 1: An anomalous sub-prefix is ignored, but no best route for the super-prefix exists
  Response: Announce the sub-prefix until the super-prefix comes back

  Event 2: A super-prefix comes back to the RIB and its anomalous sub-prefix is in use
  Response: Ignore the sub-prefix again

  Arguments:
  rn: The route node that a new best path was found for
  old_best: The old best route (NULL if one did not exist)
  new_best: The current best route (NULL if one does not exist)
 */
extern int
bgp_pgbgp_rib_updated (struct bgp_node *rn, struct bgp_info *old_best,
                       struct bgp_info *new_best);

#endif
