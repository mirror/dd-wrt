/* BGP nexthop scan
 * Copyright (C) 2000 Kunihiro Ishiguro
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

#include "command.h"
#include "thread.h"
#include "prefix.h"
#include "table.h"
#include "zclient.h"
#include "stream.h"
#include "network.h"
#include "log.h"
#include "memory.h"

#include "bgpd/bgpd.h"
#include "bgpd/bgp_route.h"
#include "bgpd/bgp_attr.h"
#include "bgpd/bgp_nexthop.h"
#include "bgpd/bgp_debug.h"
#include "zebra/rib.h"
#include "zebra/zserv.h"	/* For ZEBRA_SERV_PATH. */

struct bgp_nexthop_cache *zlookup_query (struct in_addr);
#ifdef HAVE_IPV6
struct bgp_nexthop_cache *zlookup_query_ipv6 (struct in6_addr *);
#endif /* HAVE_IPV6 */

/* Only one BGP scan thread are activated at the same time. */
struct thread *bgp_scan_thread = NULL;

/* BGP import thread */
struct thread *bgp_import_thread = NULL;

/* BGP scan interval. */
int bgp_scan_interval;

/* BGP import interval. */
int bgp_import_interval;

/* Route table for next-hop lookup cache. */
struct route_table *bgp_nexthop_cache_ipv4;
struct route_table *cache1;
struct route_table *cache2;

/* Route table for next-hop lookup cache. */
struct route_table *bgp_nexthop_cache_ipv6;
struct route_table *cache6_1;
struct route_table *cache6_2;

/* Route table for connected route. */
struct route_table *bgp_connected_ipv4;

/* Route table for connected route. */
struct route_table *bgp_connected_ipv6;

/* BGP nexthop lookup query client. */
static struct zclient *zlookup = NULL;

/* BGP process function. */
int bgp_process (struct bgp *, struct route_node *, afi_t, safi_t,
		 struct bgp_info *, struct prefix_rd *, u_char *);

/* Add nexthop to the end of the list.  */
void
bnc_nexthop_add (struct bgp_nexthop_cache *bnc, struct nexthop *nexthop)
{
  struct nexthop *last;

  for (last = bnc->nexthop; last && last->next; last = last->next)
    ;
  if (last)
    last->next = nexthop;
  else
    bnc->nexthop = nexthop;
  nexthop->prev = last;
}

void
bnc_nexthop_free (struct bgp_nexthop_cache *bnc)
{
  struct nexthop *nexthop;
  struct nexthop *next = NULL;

  for (nexthop = bnc->nexthop; nexthop; nexthop = next)
    {
      nexthop = nexthop->next;
      XFREE (MTYPE_NEXTHOP, nexthop);
    }
}

struct bgp_nexthop_cache *
bnc_new ()
{
  struct bgp_nexthop_cache *new;

  new = XMALLOC (MTYPE_BGP_NEXTHOP_CACHE, sizeof (struct bgp_nexthop_cache));
  memset (new, 0, sizeof (struct bgp_nexthop_cache));
  return new;
}

void
bnc_free (struct bgp_nexthop_cache *bnc)
{
  bnc_nexthop_free (bnc);
  XFREE (MTYPE_BGP_NEXTHOP_CACHE, bnc);
}

int
bgp_nexthop_same (struct nexthop *next1, struct nexthop *next2)
{
  if (next1->type != next2->type)
    return 0;

  switch (next1->type)
    {
    case ZEBRA_NEXTHOP_IPV4:
      if (! IPV4_ADDR_SAME (&next1->gate.ipv4, &next2->gate.ipv4))
	return 0;
      break;
    case ZEBRA_NEXTHOP_IFINDEX:
    case ZEBRA_NEXTHOP_IFNAME:
      if (next1->ifindex != next2->ifindex)
	return 0;
      break;
#ifdef HAVE_IPV6
    case ZEBRA_NEXTHOP_IPV6:
      if (! IPV6_ADDR_SAME (&next1->gate.ipv6, &next2->gate.ipv6))
	return 0;
      break;
    case ZEBRA_NEXTHOP_IPV6_IFINDEX:
    case ZEBRA_NEXTHOP_IPV6_IFNAME:
      if (! IPV6_ADDR_SAME (&next1->gate.ipv6, &next2->gate.ipv6))
	return 0;
      if (next1->ifindex != next2->ifindex)
	return 0;
      break;
#endif /* HAVE_IPV6 */
    }
  return 1;
}

int
bgp_nexthop_cache_changed (struct bgp_nexthop_cache *bnc1,
			   struct bgp_nexthop_cache *bnc2)
{
  int i;
  struct nexthop *next1, *next2;

  if (bnc1->nexthop_num != bnc2->nexthop_num)
    return 1;

  next1 = bnc1->nexthop;
  next2 = bnc2->nexthop;

  for (i = 0; i < bnc1->nexthop_num; i++)
    {
      if (! bgp_nexthop_same (next1, next2))
	return 1;

      next1 = next1->next;
      next2 = next2->next;
    }
  return 0;
}

/* If nexthop exists on connected network return 1. */
int
bgp_nexthop_check_ebgp (afi_t afi, struct attr *attr)
{
  struct route_node *rn;

  /* If zebra is not enabled return */
  if (zlookup->sock < 0)
    return 1;

  /* Lookup the address is onlink or not. */
  if (afi == AFI_IP)
    {
      rn = route_node_match_ipv4 (bgp_connected_ipv4, &attr->nexthop);
      if (rn)
	{
	  route_unlock_node (rn);
	  return 1;
	}
    }
#ifdef HAVE_IPV6
  else if (afi == AFI_IP6)
    {
      if (attr->mp_nexthop_len == 32)
	return 1;
      else if (attr->mp_nexthop_len == 16)
	{
	  if (IN6_IS_ADDR_LINKLOCAL (&attr->mp_nexthop_global))
	    return 1;

	  rn = route_node_match_ipv6 (bgp_connected_ipv6,
				      &attr->mp_nexthop_global);
	  if (rn)
	    {
	      route_unlock_node (rn);
	      return 1;
	    }
	}
    }
#endif /* HAVE_IPV6 */
  return 0;
}

#ifdef HAVE_IPV6
/* Check specified next-hop is reachable or not. */
int
bgp_nexthop_lookup_ipv6 (struct peer *peer, struct bgp_info *ri, int *changed,
			 int *metricchanged)
{
  struct route_node *rn;
  struct prefix p;
  struct bgp_nexthop_cache *bnc;
  struct attr *attr;

  /* If lookup is not enabled, return valid. */
  if (zlookup->sock < 0)
    {
      ri->igpmetric = 0;
      return 1;
    }

  /* Only check IPv6 global address only nexthop. */
  attr = ri->attr;

  if (attr->mp_nexthop_len != 16 
      || IN6_IS_ADDR_LINKLOCAL (&attr->mp_nexthop_global))
    return 1;

  memset (&p, 0, sizeof (struct prefix));
  p.family = AF_INET6;
  p.prefixlen = IPV6_MAX_BITLEN;
  p.u.prefix6 = attr->mp_nexthop_global;

  /* IBGP or ebgp-multihop */
  rn = route_node_get (bgp_nexthop_cache_ipv6, &p);

  if (rn->info)
    {
      bnc = rn->info;
      route_unlock_node (rn);
    }
  else
    {
      bnc = zlookup_query_ipv6 (&attr->mp_nexthop_global);
      if (bnc)
	{
	  struct route_table *old;
	  struct route_node *oldrn;
	  struct bgp_nexthop_cache *oldbnc;

	  if (changed)
	    {
	      if (bgp_nexthop_cache_ipv6 == cache6_1)
		old = cache6_2;
	      else
		old = cache6_1;

	      oldrn = route_node_lookup (old, &p);
	      if (oldrn)
		{
		  oldbnc = oldrn->info;

		  bnc->changed = bgp_nexthop_cache_changed (bnc, oldbnc);

		  if (bnc->metric != oldbnc->metric)
		    bnc->metricchanged = 1;
		}
	    }
	}
      else
	{
	  bnc = bnc_new ();
	  bnc->valid = 0;
	}
      rn->info = bnc;
    }

  if (changed)
    *changed = bnc->changed;

  if (metricchanged)
    *metricchanged = bnc->metricchanged;

  if (bnc->valid)
    ri->igpmetric = bnc->metric;
  else
    ri->igpmetric = 0;

  return bnc->valid;
}
#endif /* HAVE_IPV6 */

/* Check specified next-hop is reachable or not. */
int
bgp_nexthop_lookup (afi_t afi, struct peer *peer, struct bgp_info *ri,
		    int *changed, int *metricchanged)
{
  struct route_node *rn;
  struct prefix p;
  struct bgp_nexthop_cache *bnc;
  struct in_addr addr;

  /* If lookup is not enabled, return valid. */
  if (zlookup->sock < 0)
    {
      ri->igpmetric = 0;
      return 1;
    }

#ifdef HAVE_IPV6
  if (afi == AFI_IP6)
    return bgp_nexthop_lookup_ipv6 (peer, ri, changed, metricchanged);
#endif /* HAVE_IPV6 */

  addr = ri->attr->nexthop;

  memset (&p, 0, sizeof (struct prefix));
  p.family = AF_INET;
  p.prefixlen = IPV4_MAX_BITLEN;
  p.u.prefix4 = addr;

  /* IBGP or ebgp-multihop */
  rn = route_node_get (bgp_nexthop_cache_ipv4, &p);

  if (rn->info)
    {
      bnc = rn->info;
      route_unlock_node (rn);
    }
  else
    {
      bnc = zlookup_query (addr);
      if (bnc)
	{
	  struct route_table *old;
	  struct route_node *oldrn;
	  struct bgp_nexthop_cache *oldbnc;

	  if (changed)
	    {
	      if (bgp_nexthop_cache_ipv4 == cache1)
		old = cache2;
	      else
		old = cache1;

	      oldrn = route_node_lookup (old, &p);
	      if (oldrn)
		{
		  oldbnc = oldrn->info;

		  bnc->changed = bgp_nexthop_cache_changed (bnc, oldbnc);

		  if (bnc->metric != oldbnc->metric)
		    bnc->metricchanged = 1;
		}
	    }
	}
      else
	{
	  bnc = bnc_new ();
	  bnc->valid = 0;
	}
      rn->info = bnc;
    }

  if (changed)
    *changed = bnc->changed;

  if (metricchanged)
    *metricchanged = bnc->metricchanged;

  if (bnc->valid)
    ri->igpmetric = bnc->metric;
  else
    ri->igpmetric = 0;

  return bnc->valid;
}

/* Reset and free all BGP nexthop cache. */
void
bgp_nexthop_cache_reset (struct route_table *table)
{
  struct route_node *rn;
  struct bgp_nexthop_cache *bnc;

  for (rn = route_top (table); rn; rn = route_next (rn))
    if ((bnc = rn->info) != NULL)
      {
	bnc_free (bnc);
	rn->info = NULL;
	route_unlock_node (rn);
      }
}

void
bgp_scan_ipv4 ()
{
  struct route_node *rn;
  struct bgp *bgp;
  struct bgp_info *bi;
  struct peer_conf *conf;
  struct listnode *nn;
  int valid;
  int current;
  int changed;
  int metricchanged;

  /* Change cache. */
  if (bgp_nexthop_cache_ipv4 == cache1)
    bgp_nexthop_cache_ipv4 = cache2;
  else
    bgp_nexthop_cache_ipv4 = cache1;

  /* Get default bgp. */
  bgp = bgp_get_default ();
  if (bgp == NULL)
    return;

  /* Maximum prefix check */
  LIST_LOOP (bgp->peer_conf, conf, nn)
    {
      if (conf->peer->status != Established)
	continue;

      if (conf->peer->afc[AFI_IP][SAFI_UNICAST])
	bgp_maximum_prefix_overflow (conf, AFI_IP, SAFI_UNICAST);
      if (conf->peer->afc[AFI_IP][SAFI_MULTICAST])
	bgp_maximum_prefix_overflow (conf, AFI_IP, SAFI_MULTICAST);
      if (conf->peer->afc[AFI_IP][SAFI_MPLS_VPN])
	bgp_maximum_prefix_overflow (conf, AFI_IP, SAFI_MPLS_VPN);
    }

  for (rn = route_top (bgp->rib[AFI_IP][SAFI_UNICAST]); rn;
       rn = route_next (rn))
    {
      for (bi = rn->info; bi; bi = bi->next)
	{
	  if (bi->type == ZEBRA_ROUTE_BGP && bi->sub_type == BGP_ROUTE_NORMAL)
	    {
	      changed = 0;
	      metricchanged = 0;

	      if (peer_sort (bi->peer) == BGP_PEER_EBGP && bi->peer->ttl == 1)
		valid = bgp_nexthop_check_ebgp (AFI_IP, bi->attr);
	      else
		valid = bgp_nexthop_lookup (AFI_IP, bi->peer, bi,
					    &changed, &metricchanged);

	      current = CHECK_FLAG (bi->flags, BGP_INFO_VALID) ? 1 : 0;

	      if (changed)
		SET_FLAG (bi->flags, BGP_INFO_IGP_CHANGED);
	      else
		UNSET_FLAG (bi->flags, BGP_INFO_IGP_CHANGED);

	      if (valid != current)
		{
		  if (CHECK_FLAG (bi->flags, BGP_INFO_VALID))
		    {
		      bgp_aggregate_decrement (bgp, &rn->p, bi, AFI_IP,
					       SAFI_UNICAST);
		      UNSET_FLAG (bi->flags, BGP_INFO_VALID);
		    }
		  else
		    {
		      SET_FLAG (bi->flags, BGP_INFO_VALID);
		      bgp_aggregate_increment (bgp, &rn->p, bi, AFI_IP,
					       SAFI_UNICAST);
		    }
		}
	    }
	}
      bgp_process (bgp, rn, AFI_IP, SAFI_UNICAST, NULL, NULL, NULL);
    }

  /* Flash old cache. */
  if (bgp_nexthop_cache_ipv4 == cache1)
    bgp_nexthop_cache_reset (cache2);
  else
    bgp_nexthop_cache_reset (cache1);
}

#ifdef HAVE_IPV6
void
bgp_scan_ipv6 ()
{
  struct route_node *rn;
  struct bgp *bgp;
  struct bgp_info *bi;
  struct peer_conf *conf;
  struct listnode *nn;
  int valid;
  int current;
  int changed;
  int metricchanged;

  /* Change cache. */
  if (bgp_nexthop_cache_ipv6 == cache6_1)
    bgp_nexthop_cache_ipv6 = cache6_2;
  else
    bgp_nexthop_cache_ipv6 = cache6_1;

  /* Get default bgp. */
  bgp = bgp_get_default ();
  if (bgp == NULL)
    return;

  /* Maximum prefix check */
  LIST_LOOP (bgp->peer_conf, conf, nn)
    {
      if (conf->peer->status != Established)
	continue;

      if (conf->peer->afc[AFI_IP6][SAFI_UNICAST])
	bgp_maximum_prefix_overflow (conf, AFI_IP6, SAFI_UNICAST);
      if (conf->peer->afc[AFI_IP6][SAFI_MULTICAST])
	bgp_maximum_prefix_overflow (conf, AFI_IP6, SAFI_MULTICAST);
    }

  for (rn = route_top (bgp->rib[AFI_IP6][SAFI_UNICAST]); rn;
       rn = route_next (rn))
    {
      for (bi = rn->info; bi; bi = bi->next)
	{
	  if (bi->type == ZEBRA_ROUTE_BGP && bi->sub_type == BGP_ROUTE_NORMAL)
	    {
	      changed = 0;
	      metricchanged = 0;

	      if (peer_sort (bi->peer) == BGP_PEER_EBGP && bi->peer->ttl == 1)
		valid = 1;
	      else
		valid = bgp_nexthop_lookup_ipv6 (bi->peer, bi,
						 &changed, &metricchanged);

	      current = CHECK_FLAG (bi->flags, BGP_INFO_VALID) ? 1 : 0;

	      if (changed)
		SET_FLAG (bi->flags, BGP_INFO_IGP_CHANGED);
	      else
		UNSET_FLAG (bi->flags, BGP_INFO_IGP_CHANGED);

	      if (valid != current)
		{
		  if (CHECK_FLAG (bi->flags, BGP_INFO_VALID))
		    {
		      bgp_aggregate_decrement (bgp, &rn->p, bi, AFI_IP6,
					       SAFI_UNICAST);
		      UNSET_FLAG (bi->flags, BGP_INFO_VALID);
		    }
		  else
		    {
		      SET_FLAG (bi->flags, BGP_INFO_VALID);
		      bgp_aggregate_increment (bgp, &rn->p, bi, AFI_IP6,
					       SAFI_UNICAST);
		    }
		}
	    }
	}
      bgp_process (bgp, rn, AFI_IP6, SAFI_UNICAST, NULL, NULL, NULL);
    }

  /* Flash old cache. */
  if (bgp_nexthop_cache_ipv6 == cache6_1)
    bgp_nexthop_cache_reset (cache6_2);
  else
    bgp_nexthop_cache_reset (cache6_1);
}
#endif /* HAVE_IPV6 */

/* BGP scan thread.  This thread check nexthop reachability. */
int
bgp_scan (struct thread *t)
{
  bgp_scan_thread =
    thread_add_timer (master, bgp_scan, NULL, bgp_scan_interval);

  if (BGP_DEBUG (normal, NORMAL))
    zlog_info ("Performing BGP general scanning");

  bgp_scan_ipv4 ();

#ifdef HAVE_IPV6
  bgp_scan_ipv6 ();
#endif /* HAVE_IPV6 */

  return 0;
}

struct bgp_connected
{
  unsigned int refcnt;
};

void
bgp_connected_add (struct connected *ifc)
{
  struct prefix p;
  struct prefix *addr;
  struct prefix *dest;
  struct interface *ifp;
  struct route_node *rn;
  struct bgp_connected *bc;

  ifp = ifc->ifp;

  if (! ifp)
    return;

  if (if_is_loopback (ifp))
    return;

  addr = ifc->address;
  dest = ifc->destination;

  if (addr->family == AF_INET)
    {
      memset (&p, 0, sizeof (struct prefix));
      p.family = AF_INET;
      p.prefixlen = addr->prefixlen;

      if (if_is_pointopoint (ifp))
	p.u.prefix4 = dest->u.prefix4;
      else
	p.u.prefix4 = addr->u.prefix4;

      apply_mask_ipv4 ((struct prefix_ipv4 *) &p);

      if (prefix_ipv4_any ((struct prefix_ipv4 *) &p))
	return;

      rn = route_node_get (bgp_connected_ipv4, (struct prefix *) &p);
      if (rn->info)
	{
	  bc = rn->info;
	  bc->refcnt++;
	}
      else
	{
	  bc = XMALLOC (0, sizeof (struct bgp_connected));
	  memset (bc, 0, sizeof (struct bgp_connected));
	  bc->refcnt = 1;
	  rn->info = bc;
	}
    }
#ifdef HAVE_IPV6
  if (addr->family == AF_INET6)
    {
      memset (&p, 0, sizeof (struct prefix));
      p.family = AF_INET6;
      p.prefixlen = addr->prefixlen;

      if (if_is_pointopoint (ifp))
	p.u.prefix6 = dest->u.prefix6;
      else
	p.u.prefix6 = addr->u.prefix6;

      apply_mask_ipv6 ((struct prefix_ipv6 *) &p);

      if (IN6_IS_ADDR_UNSPECIFIED (&p.u.prefix6))
	return;

      if (IN6_IS_ADDR_LINKLOCAL (&p.u.prefix6))
	return;

      rn = route_node_get (bgp_connected_ipv6, (struct prefix *) &p);
      if (rn->info)
	{
	  bc = rn->info;
	  bc->refcnt++;
	}
      else
	{
	  bc = XMALLOC (0, sizeof (struct bgp_connected));
	  memset (bc, 0, sizeof (struct bgp_connected));
	  bc->refcnt = 1;
	  rn->info = bc;
	}
    }
#endif /* HAVE_IPV6 */
}

void
bgp_connected_delete (struct connected *ifc)
{
  struct prefix p;
  struct prefix *addr;
  struct prefix *dest;
  struct interface *ifp;
  struct route_node *rn;
  struct bgp_connected *bc;

  ifp = ifc->ifp;

  if (if_is_loopback (ifp))
    return;

  addr = ifc->address;
  dest = ifc->destination;

  if (addr->family == AF_INET)
    {
      memset (&p, 0, sizeof (struct prefix));
      p.family = AF_INET;
      p.prefixlen = addr->prefixlen;

      if (if_is_pointopoint (ifp))
	p.u.prefix4 = dest->u.prefix4;
      else
	p.u.prefix4 = addr->u.prefix4;

      apply_mask_ipv4 ((struct prefix_ipv4 *) &p);

      if (prefix_ipv4_any ((struct prefix_ipv4 *) &p))
	return;

      rn = route_node_lookup (bgp_connected_ipv4, &p);
      if (! rn)
	return;

      bc = rn->info;
      bc->refcnt--;
      if (bc->refcnt == 0)
	{
	  XFREE (0, bc);
	  rn->info = NULL;
	}
      route_unlock_node (rn);
      route_unlock_node (rn);
    }
#ifdef HAVE_IPV6
  else if (addr->family == AF_INET6)
    {
      memset (&p, 0, sizeof (struct prefix));
      p.family = AF_INET6;
      p.prefixlen = addr->prefixlen;

      if (if_is_pointopoint (ifp))
	p.u.prefix6 = dest->u.prefix6;
      else
	p.u.prefix6 = addr->u.prefix6;

      apply_mask_ipv6 ((struct prefix_ipv6 *) &p);

      if (IN6_IS_ADDR_UNSPECIFIED (&p.u.prefix6))
	return;

      if (IN6_IS_ADDR_LINKLOCAL (&p.u.prefix6))
	return;

      rn = route_node_lookup (bgp_connected_ipv6, (struct prefix *) &p);
      if (! rn)
	return;

      bc = rn->info;
      bc->refcnt--;
      if (bc->refcnt == 0)
	{
	  XFREE (0, bc);
	  rn->info = NULL;
	}
      route_unlock_node (rn);
      route_unlock_node (rn);
    }
#endif /* HAVE_IPV6 */
}

int
bgp_nexthop_self (afi_t afi, struct attr *attr)
{
  listnode node;
  listnode node2;
  struct interface *ifp;
  struct connected *ifc;
  struct prefix *p;

  for (node = listhead (iflist); node; nextnode (node))
    {
      ifp = getdata (node);

      for (node2 = listhead (ifp->connected); node2; nextnode (node2))
	{
	  ifc = getdata (node2);
	  p = ifc->address;

	  if (p && p->family == AF_INET 
	      && IPV4_ADDR_SAME (&p->u.prefix4, &attr->nexthop))
	    return 1;
	}
    }
  return 0;
}

struct bgp_nexthop_cache *
zlookup_read ()
{
  struct stream *s;
  u_int16_t length;
  u_char command;
  int nbytes;
  struct in_addr raddr;
  u_int32_t metric;
  int i;
  u_char nexthop_num;
  struct nexthop *nexthop;
  struct bgp_nexthop_cache *bnc;

  s = zlookup->ibuf;
  stream_reset (s);

  nbytes = stream_read (s, zlookup->sock, 2);
  length = stream_getw (s);

  nbytes = stream_read (s, zlookup->sock, length - 2);
  command = stream_getc (s);
  raddr.s_addr = stream_get_ipv4 (s);
  metric = stream_getl (s);
  nexthop_num = stream_getc (s);

  if (nexthop_num)
    {
      bnc = bnc_new ();
      bnc->valid = 1;
      bnc->metric = metric;
      bnc->nexthop_num = nexthop_num;

      for (i = 0; i < nexthop_num; i++)
	{
	  nexthop = XMALLOC (MTYPE_NEXTHOP, sizeof (struct nexthop));
	  memset (nexthop, 0, sizeof (struct nexthop));
	  nexthop->type = stream_getc (s);
	  switch (nexthop->type)
	    {
	    case ZEBRA_NEXTHOP_IPV4:
	      nexthop->gate.ipv4.s_addr = stream_get_ipv4 (s);
	      break;
	    case ZEBRA_NEXTHOP_IFINDEX:
	    case ZEBRA_NEXTHOP_IFNAME:
	      nexthop->ifindex = stream_getl (s);
	      break;
	    }
	  bnc_nexthop_add (bnc, nexthop);
	}
    }
  else
    return NULL;

  return bnc;
}

struct bgp_nexthop_cache *
zlookup_query (struct in_addr addr)
{
  int ret;
  struct stream *s;

  /* Check socket. */
  if (zlookup->sock < 0)
    return NULL;

  s = zlookup->obuf;
  stream_reset (s);
  stream_putw (s, 7);
  stream_putc (s, ZEBRA_IPV4_NEXTHOP_LOOKUP);
  stream_put_in_addr (s, &addr);

  ret = writen (zlookup->sock, s->data, 7);
  if (ret < 0)
    {
      zlog_err ("can't write to zlookup->sock");
      close (zlookup->sock);
      zlookup->sock = -1;
      return NULL;
    }
  if (ret == 0)
    {
      zlog_err ("zlookup->sock connection closed");
      close (zlookup->sock);
      zlookup->sock = -1;
      return NULL;
    }

  return zlookup_read ();
}

#ifdef HAVE_IPV6
struct bgp_nexthop_cache *
zlookup_read_ipv6 ()
{
  struct stream *s;
  u_int16_t length;
  u_char command;
  int nbytes;
  struct in6_addr raddr;
  u_int32_t metric;
  int i;
  u_char nexthop_num;
  struct nexthop *nexthop;
  struct bgp_nexthop_cache *bnc;

  s = zlookup->ibuf;
  stream_reset (s);

  nbytes = stream_read (s, zlookup->sock, 2);
  length = stream_getw (s);

  nbytes = stream_read (s, zlookup->sock, length - 2);
  command = stream_getc (s);

  stream_get (&raddr, s, 16);

  metric = stream_getl (s);
  nexthop_num = stream_getc (s);

  if (nexthop_num)
    {
      bnc = bnc_new ();
      bnc->valid = 1;
      bnc->metric = metric;
      bnc->nexthop_num = nexthop_num;

      for (i = 0; i < nexthop_num; i++)
	{
	  nexthop = XMALLOC (MTYPE_NEXTHOP, sizeof (struct nexthop));
	  memset (nexthop, 0, sizeof (struct nexthop));
	  nexthop->type = stream_getc (s);
	  switch (nexthop->type)
	    {
	    case ZEBRA_NEXTHOP_IPV6:
	      stream_get (&nexthop->gate.ipv6, s, 16);
	      break;
	    case ZEBRA_NEXTHOP_IPV6_IFINDEX:
	    case ZEBRA_NEXTHOP_IPV6_IFNAME:
	      stream_get (&nexthop->gate.ipv6, s, 16);
	      nexthop->ifindex = stream_getl (s);
	      break;
	    case ZEBRA_NEXTHOP_IFINDEX:
	    case ZEBRA_NEXTHOP_IFNAME:
	      nexthop->ifindex = stream_getl (s);
	      break;
	    }
	  bnc_nexthop_add (bnc, nexthop);
	}
    }
  else
    return NULL;

  return bnc;
}

struct bgp_nexthop_cache *
zlookup_query_ipv6 (struct in6_addr *addr)
{
  int ret;
  struct stream *s;

  /* Check socket. */
  if (zlookup->sock < 0)
    return NULL;

  s = zlookup->obuf;
  stream_reset (s);
  stream_putw (s, 19);
  stream_putc (s, ZEBRA_IPV6_NEXTHOP_LOOKUP);
  stream_put (s, addr, 16);

  ret = writen (zlookup->sock, s->data, 19);
  if (ret < 0)
    {
      zlog_err ("can't write to zlookup->sock");
      close (zlookup->sock);
      zlookup->sock = -1;
      return NULL;
    }
  if (ret == 0)
    {
      zlog_err ("zlookup->sock connection closed");
      close (zlookup->sock);
      zlookup->sock = -1;
      return NULL;
    }

  return zlookup_read_ipv6 ();
}
#endif /* HAVE_IPV6 */

int
bgp_import_check (struct prefix *p, u_int32_t *igpmetric)
{
  struct stream *s;
  int ret;
  u_int16_t length;
  u_char command;
  int nbytes;
  struct in_addr addr;
  u_int32_t metric = 0;
  u_char nexthop_num;

  /* If lookup connection is not available return valid. */
  if (zlookup->sock < 0)
    {
      if (igpmetric)
	*igpmetric = 0;
      return 1;
    }

  /* Send query to the lookup connection */
  s = zlookup->obuf;
  stream_reset (s);
  stream_putw (s, 8);
  stream_putc (s, ZEBRA_IPV4_IMPORT_LOOKUP);
  stream_putc (s, p->prefixlen);
  stream_put_in_addr (s, &p->u.prefix4);

  /* Write the packet. */
  ret = writen (zlookup->sock, s->data, 8);

  if (ret < 0)
    {
      zlog_err ("can't write to zlookup->sock");
      close (zlookup->sock);
      zlookup->sock = -1;
      return 1;
    }
  if (ret == 0)
    {
      zlog_err ("zlookup->sock connection closed");
      close (zlookup->sock);
      zlookup->sock = -1;
      return 1;
    }

  /* Get result. */
  stream_reset (s);

  /* Fetch length. */
  nbytes = stream_read (s, zlookup->sock, 2);
  length = stream_getw (s);

  /* Fetch whole data. */
  nbytes = stream_read (s, zlookup->sock, length - 2);
  command = stream_getc (s);
  addr.s_addr = stream_get_ipv4 (s);
  metric = stream_getl (s);
  nexthop_num = stream_getc (s);

  /* Set IGP metric value. */
  if (igpmetric)
    *igpmetric = metric;

  /* If there is nexthop then this is active route. */
  if (nexthop_num)
    return 1;
  else
    return 0;
}

/* Scan all configured BGP route then check the route exists in IGP or
   not. */
int
bgp_import (struct thread *t)
{
  struct bgp *bgp;
  struct route_node *rn;
  struct route_node *rm;
  struct bgp_static *bgp_static;
  struct bgp_info *bi;
  int valid;
  u_int32_t igpmetric;

  bgp_import_thread = 
    thread_add_timer (master, bgp_import, NULL, bgp_import_interval);

  bgp = bgp_get_default ();
  if (! bgp)
    return 0;

  for (rn = route_top (bgp->route[AFI_IP][SAFI_UNICAST]); rn;
       rn = route_next (rn))
    if ((bgp_static = rn->info) != NULL)
      {
	valid = bgp_static->valid;
	igpmetric = bgp_static->igpmetric;

	if (! CHECK_FLAG (bgp->config, BGP_CONFIG_IMPORT_CHECK))
	  bgp_static->valid = 1;
	else
	  bgp_static->valid
	    = bgp_import_check (&rn->p, &bgp_static->igpmetric);

	if (bgp_static->valid != valid)
	  {
	    rm = route_node_get (bgp->rib[AFI_IP][SAFI_UNICAST], &rn->p);

	    for (bi = rm->info; bi; bi = bi->next)
	      if (bi->type == ZEBRA_ROUTE_BGP
		  && bi->sub_type == BGP_ROUTE_STATIC)
	      {
		if (bgp_static->valid)
		  {
		    SET_FLAG (bi->flags, BGP_INFO_VALID);
		    bgp_aggregate_increment (bgp, &rn->p, bi,
					     AFI_IP, SAFI_UNICAST);
		  }
		else
		  {
		    bgp_aggregate_decrement (bgp, &rn->p, bi,
					     AFI_IP, SAFI_UNICAST);
		    UNSET_FLAG (bi->flags, BGP_INFO_VALID);
		  }
	      }
	    bgp_process (bgp, rm, AFI_IP, SAFI_UNICAST, NULL, NULL, NULL);
	    
	    route_unlock_node (rm);
	  }
      }
  return 0;
}

/* Connect to zebra for nexthop lookup. */
int
zlookup_connect (struct thread *t)
{
  struct zclient *zlookup;

  zlookup = THREAD_ARG (t);
  zlookup->t_connect = NULL;

  if (zlookup->sock != -1)
    return 0;

#ifdef HAVE_TCP_ZEBRA
  zlookup->sock = zclient_socket ();
#else
  zlookup->sock = zclient_socket_un (ZEBRA_SERV_PATH);
#endif /* HAVE_TCP_ZEBRA */
  if (zlookup->sock < 0)
    return -1;

  /* Make BGP scan thread. */
  bgp_scan_thread = 
    thread_add_timer (master, bgp_scan, NULL, 0);

  /* Make BGP import there. */
  bgp_import_thread = 
    thread_add_timer (master, bgp_import, NULL, 0);

  return 0;
}

/* Check specified multiaccess next-hop. */
int
bgp_multiaccess_check_v4 (struct in_addr nexthop, char *peer)
{
  struct route_node *rn1;
  struct route_node *rn2;
  struct prefix p1;
  struct prefix p2;
  struct in_addr addr;
  int ret;

  ret = inet_aton (peer, &addr);
  if (! ret)
    return 0;

  memset (&p1, 0, sizeof (struct prefix));
  p1.family = AF_INET;
  p1.prefixlen = IPV4_MAX_BITLEN;
  p1.u.prefix4 = nexthop;
  memset (&p2, 0, sizeof (struct prefix));
  p2.family = AF_INET;
  p2.prefixlen = IPV4_MAX_BITLEN;
  p2.u.prefix4 = addr;

  /* If bgp scan is not enabled, return invalid. */
  if (zlookup->sock < 0)
    return 0;

  rn1 = route_node_match (bgp_connected_ipv4, &p1);
  if (! rn1)
    return 0;
  
  rn2 = route_node_match (bgp_connected_ipv4, &p2);
  if (! rn2)
    return 0;

  if (rn1 == rn2)
    return 1;

  return 0;
}

DEFUN (bgp_scan_time,
       bgp_scan_time_cmd,
       "bgp scan-time <5-60>",
       "BGP specific commands\n"
       "Configure background scanner interval\n"
       "Scanner interval (seconds)\n")
{
  bgp_scan_interval = atoi (argv[0]);

  if (bgp_scan_thread)
    {
      thread_cancel (bgp_scan_thread);
      bgp_scan_thread = 
	thread_add_timer (master, bgp_scan, NULL, bgp_scan_interval);
    }

  return CMD_SUCCESS;
}

DEFUN (no_bgp_scan_time,
       no_bgp_scan_time_cmd,
       "no bgp scan-time",
       NO_STR
       "BGP specific commands\n"
       "Configure background scanner interval\n")
{
  bgp_scan_interval = BGP_SCAN_INTERVAL_DEFAULT;

  if (bgp_scan_thread)
    {
      thread_cancel (bgp_scan_thread);
      bgp_scan_thread = 
	thread_add_timer (master, bgp_scan, NULL, bgp_scan_interval);
    }

  return CMD_SUCCESS;
}

ALIAS (no_bgp_scan_time,
       no_bgp_scan_time_val_cmd,
       "no bgp scan-time <5-60>",
       NO_STR
       "BGP specific commands\n"
       "Configure background scanner interval\n"
       "Scanner interval (seconds)\n")

DEFUN (show_ip_bgp_scan,
       show_ip_bgp_scan_cmd,
       "show ip bgp scan",
       SHOW_STR
       IP_STR
       BGP_STR
       "BGP scan status\n")
{
  struct route_node *rn;
  struct bgp_nexthop_cache *bnc;

  if (bgp_scan_thread)
    vty_out (vty, "BGP scan is running%s", VTY_NEWLINE);
  else
    vty_out (vty, "BGP scan is not running%s", VTY_NEWLINE);
  vty_out (vty, "BGP scan interval is %d%s", bgp_scan_interval, VTY_NEWLINE);

  vty_out (vty, "Current BGP nexthop cache:%s", VTY_NEWLINE);
  for (rn = route_top (bgp_nexthop_cache_ipv4); rn; rn = route_next (rn))
    if ((bnc = rn->info) != NULL)
      {
	if (bnc->valid)
	  vty_out (vty, " %s valid [IGP metric %d]%s",
		   inet_ntoa (rn->p.u.prefix4), bnc->metric, VTY_NEWLINE);
	else
	  vty_out (vty, " %s invalid%s",
		   inet_ntoa (rn->p.u.prefix4), VTY_NEWLINE);
      }

#ifdef HAVE_IPV6
  {
    char buf[BUFSIZ];
    for (rn = route_top (bgp_nexthop_cache_ipv6); rn; rn = route_next (rn))
      if ((bnc = rn->info) != NULL)
	{
	  if (bnc->valid)
	    vty_out (vty, " %s valid [IGP metric %d]%s",
		     inet_ntop (AF_INET6, &rn->p.u.prefix6, buf, BUFSIZ),
		     bnc->metric, VTY_NEWLINE);
	  else
	    vty_out (vty, " %s invalid%s",
		     inet_ntop (AF_INET6, &rn->p.u.prefix6, buf, BUFSIZ),
		     VTY_NEWLINE);
	}
  }
#endif /* HAVE_IPV6 */

  vty_out (vty, "BGP connected route:%s", VTY_NEWLINE);
  for (rn = route_top (bgp_connected_ipv4); rn; rn = route_next (rn))
    if (rn->info != NULL)
      vty_out (vty, " %s/%d%s", inet_ntoa (rn->p.u.prefix4), rn->p.prefixlen,
	       VTY_NEWLINE);

#ifdef HAVE_IPV6
  {
    char buf[BUFSIZ];

    for (rn = route_top (bgp_connected_ipv6); rn; rn = route_next (rn))
      if (rn->info != NULL)
	vty_out (vty, " %s/%d%s",
		 inet_ntop (AF_INET6, &rn->p.u.prefix6, buf, BUFSIZ),
		 rn->p.prefixlen,
		 VTY_NEWLINE);
  }
#endif /* HAVE_IPV6 */

  return CMD_SUCCESS;
}

int
bgp_config_write_scan_time (struct vty *vty)
{
  if (bgp_scan_interval != BGP_SCAN_INTERVAL_DEFAULT)
    vty_out (vty, " bgp scan-time %d%s", bgp_scan_interval, VTY_NEWLINE);
  return CMD_SUCCESS;
}

void
bgp_scan_init ()
{
  zlookup = zclient_new ();
  zlookup->sock = -1;
  zlookup->ibuf = stream_new (ZEBRA_MAX_PACKET_SIZ);
  zlookup->obuf = stream_new (ZEBRA_MAX_PACKET_SIZ);
  zlookup->t_connect = thread_add_event (master, zlookup_connect, zlookup, 0);

  bgp_scan_interval = BGP_SCAN_INTERVAL_DEFAULT;
  bgp_import_interval = BGP_IMPORT_INTERVAL_DEFAULT;

  cache1 = route_table_init ();
  cache2 = route_table_init ();
  bgp_nexthop_cache_ipv4 = cache1;

  bgp_connected_ipv4 = route_table_init ();

#ifdef HAVE_IPV6
  cache6_1 = route_table_init ();
  cache6_2 = route_table_init ();
  bgp_nexthop_cache_ipv6 = cache6_1;
  bgp_connected_ipv6 = route_table_init ();
#endif /* HAVE_IPV6 */

  install_element (BGP_NODE, &bgp_scan_time_cmd);
  install_element (BGP_NODE, &no_bgp_scan_time_cmd);
  install_element (BGP_NODE, &no_bgp_scan_time_val_cmd);
  install_element (VIEW_NODE, &show_ip_bgp_scan_cmd);
  install_element (ENABLE_NODE, &show_ip_bgp_scan_cmd);
}
