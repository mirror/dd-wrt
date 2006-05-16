/*
 * OSPF Link State Advertisement
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

#include "linklist.h"
#include "prefix.h"
#include "if.h"
#include "table.h"
#include "memory.h"
#include "command.h"
#include "vty.h"
#include "stream.h"
#include "log.h"
#include "thread.h"
#include "hash.h"
#include "sockunion.h"		/* for inet_aton() */

#include "ospfd/ospfd.h"
#include "ospfd/ospf_interface.h"
#include "ospfd/ospf_ism.h"
#include "ospfd/ospf_asbr.h"
#include "ospfd/ospf_lsa.h"
#include "ospfd/ospf_lsdb.h"
#include "ospfd/ospf_neighbor.h"
#include "ospfd/ospf_nsm.h"
#include "ospfd/ospf_flood.h"
#include "ospfd/ospf_packet.h"
#include "ospfd/ospf_spf.h"
#include "ospfd/ospf_dump.h"
#include "ospfd/ospf_route.h"
#include "ospfd/ospf_ase.h"
#include "ospfd/ospf_zebra.h"

u_int32_t
get_metric (u_char *metric)
{
  u_int32_t m;
  m = metric[0];
  m = (m << 8) + metric[1];
  m = (m << 8) + metric[2];
  return m;
}


void
tv_debug (struct timeval a)
{
  printf ("%ld.%ld\n", a.tv_sec, a.tv_usec);
}

struct timeval
tv_adjust (struct timeval a)
{
  while (a.tv_usec >= 1000000)
    {
      a.tv_usec -= 1000000;
      a.tv_sec++;
    }

  while (a.tv_usec < 0)
    {
      a.tv_usec += 1000000;
      a.tv_sec--;
    }

  return a;
}

int
tv_ceil (struct timeval a)
{
  a = tv_adjust (a);

  return (a.tv_usec ? a.tv_sec + 1 : a.tv_sec);
}

int
tv_floor (struct timeval a)
{
  a = tv_adjust (a);

  return a.tv_sec;
}

struct timeval
int2tv (int a)
{
  struct timeval ret;

  ret.tv_sec = a;
  ret.tv_usec = 0;

  return ret;
}

struct timeval
tv_add (struct timeval a, struct timeval b)
{
  struct timeval ret;

  ret.tv_sec = a.tv_sec + b.tv_sec;
  ret.tv_usec = a.tv_usec + b.tv_usec;

  return tv_adjust (ret);
}

struct timeval
tv_sub (struct timeval a, struct timeval b)
{
  struct timeval ret;

  ret.tv_sec = a.tv_sec - b.tv_sec;
  ret.tv_usec = a.tv_usec - b.tv_usec;

  return tv_adjust (ret);
}

int
tv_cmp (struct timeval a, struct timeval b)
{
  return (a.tv_sec == b.tv_sec ?
	  a.tv_usec - b.tv_usec : a.tv_sec - b.tv_sec);
}


int
get_age (struct ospf_lsa *lsa)
{
  int age;
  struct timeval now;

  gettimeofday (&now, NULL);
  age = ntohs (lsa->data->ls_age) + tv_floor (tv_sub (now, lsa->tv_recv));

  return age;
}


/* Fletcher Checksum -- Refer to RFC1008. */
#define MODX                 4102
#define LSA_CHECKSUM_OFFSET    15

u_int16_t
ospf_lsa_checksum (struct lsa_header *lsa)
{
  u_char *sp, *ep, *p, *q;
  int c0 = 0, c1 = 0;
  int x, y;
  u_int16_t length;

  lsa->checksum = 0;
  length = ntohs (lsa->length) - 2;
  sp = (char *) &lsa->options;

  for (ep = sp + length; sp < ep; sp = q)
    {
      q = sp + MODX;
      if (q > ep)
        q = ep;
      for (p = sp; p < q; p++)
        {
          c0 += *p;
          c1 += c0;
        }
      c0 %= 255;
      c1 %= 255;
    }

  /* r = (c1 << 8) + c0; */
  x = ((length - LSA_CHECKSUM_OFFSET) * c0 - c1) % 255;
  if (x <= 0)
    x += 255;
  y = 510 - c0 - x;
  if (y > 255)
    y -= 255;

  /* take care endian issue. */
  lsa->checksum = htons ((x << 8) + y);

  return (lsa->checksum);
}

/* Create OSPF LSA. */
struct ospf_lsa *
ospf_lsa_new ()
{
  struct ospf_lsa *new;

  new = XMALLOC (MTYPE_OSPF_LSA, sizeof (struct ospf_lsa));
  assert (new);

  bzero (new, sizeof (struct ospf_lsa));

  new->flags = 0;
  new->lock = 1;
  new->retransmit_counter = 0;
  gettimeofday (&new->tv_recv, NULL);
  new->tv_orig = new->tv_recv;
  new->refresh_list = -1;
  
  return new;
}

/* Duplicate OSPF LSA. */
struct ospf_lsa *
ospf_lsa_dup (struct ospf_lsa *lsa)
{
  struct ospf_lsa *new;

  if (lsa == NULL)
    return NULL;

  new = XMALLOC (MTYPE_OSPF_LSA, sizeof (struct ospf_lsa));
  assert (new);
  bzero (new, sizeof (struct ospf_lsa));

  new->flags = lsa->flags;
  UNSET_FLAG (new->flags, OSPF_LSA_DISCARD);
  new->tv_recv = lsa->tv_recv;
  new->tv_orig = lsa->tv_orig;
  new->lock = 1;
  new->retransmit_counter = 0;

  new->data = ospf_lsa_data_dup (lsa->data);

  return new;
}

/* Free OSPF LSA. */
void
ospf_lsa_free (struct ospf_lsa *lsa)
{
  assert (lsa->lock == 0);
  
  if (IS_DEBUG_OSPF (lsa, LSA))
    zlog_info ("LSA: freed %p", lsa);

  /* Delete LSA data. */
  if (lsa->data != NULL)
    ospf_lsa_data_free (lsa->data);

  assert (lsa->refresh_list < 0);

  memset (lsa, 0, sizeof (struct ospf_lsa)); 
  XFREE (MTYPE_OSPF_LSA, lsa);
}

/* Lock LSA. */
struct ospf_lsa *
ospf_lsa_lock (struct ospf_lsa *lsa)
{
  lsa->lock++;
  return lsa;
}

/* Unlock LSA. */
void
ospf_lsa_unlock (struct ospf_lsa *lsa)
{
  /* This is sanity check. */
  if (!lsa)
    return;
  
  lsa->lock--;

#ifdef HAVE_NSSA
  if (lsa->lock < 0 )
    {
      zlog_info ("OOPS- We Would Have Crashed due to LSA-UNLOCK); Data now corrupt");
      return;
    }
#endif /* HAVE_NSSA */

  assert (lsa->lock >= 0);

  if (lsa->lock == 0)
    {
#ifdef HAVE_NSSA
      if (! CHECK_FLAG (lsa->flags, OSPF_LSA_DISCARD))
	{
	  zlog_info ("OOPS- We Would Have Crashed due to NO DISCARD); Data now corrupt");
	  return;
	}
#endif /* HAVE_NSSA */

      assert (CHECK_FLAG (lsa->flags, OSPF_LSA_DISCARD));
      ospf_lsa_free (lsa);
    }
}

/* Check discard flag. */
void
ospf_lsa_discard (struct ospf_lsa *lsa)
{
  if (!CHECK_FLAG (lsa->flags, OSPF_LSA_DISCARD))
    {
      SET_FLAG (lsa->flags, OSPF_LSA_DISCARD);
      ospf_lsa_unlock (lsa);
    }
}

/* Create LSA data. */
struct lsa_header *
ospf_lsa_data_new (size_t size)
{
  struct lsa_header *new;

  new = (struct lsa_header *) XMALLOC (MTYPE_OSPF_LSA_DATA, size);
  assert (new);
  bzero (new, size);

  return new;
}

/* Duplicate LSA data. */
struct lsa_header *
ospf_lsa_data_dup (struct lsa_header *lsah)
{
  struct lsa_header *new;

  new = ospf_lsa_data_new (ntohs (lsah->length));
  memcpy (new, lsah, ntohs (lsah->length));

  return new;
}

/* Free LSA data. */
void
ospf_lsa_data_free (struct lsa_header *lsah)
{
  if (IS_DEBUG_OSPF (lsa, LSA))
    zlog_info ("LSA[Type%d:%s]: data freed %p",
	       lsah->type, inet_ntoa (lsah->id), lsah);

  XFREE (MTYPE_OSPF_LSA_DATA, lsah);
}


/* LSA general functions. */

u_int32_t
lsa_seqnum_increment (struct ospf_lsa *lsa)
{
  u_int32_t seqnum;

  seqnum = ntohl (lsa->data->ls_seqnum) + 1;

  return htonl (seqnum);
}

void
lsa_header_set (struct stream *s, u_char options,
		u_char type, struct in_addr id)
{
  struct lsa_header *lsah;

  lsah = (struct lsa_header *) STREAM_DATA (s);

  lsah->ls_age = htons (0);
  lsah->options = options;
  lsah->type = type;
  lsah->id = id;
  lsah->adv_router = ospf_top->router_id;
  lsah->ls_seqnum = htonl (OSPF_INITIAL_SEQUENCE_NUMBER);

  ospf_output_forward (s, OSPF_LSA_HEADER_SIZE);
}

/* router-LSA related functions. */
/* Get router-LSA flags. */
u_char
router_lsa_flags (struct ospf_area *area)
{
  u_char flags;

  flags = ospf_top->flags;

  /* Set virtual link flag. */
  if (ospf_full_virtual_nbrs (area))
    SET_FLAG (flags, ROUTER_LSA_VIRTUAL);
  else
    /* Just sanity check */
    UNSET_FLAG (flags, ROUTER_LSA_VIRTUAL);

  /* Set Shortcut ABR behabiour flag. */
  UNSET_FLAG (flags, ROUTER_LSA_SHORTCUT);
  if (ospf_top->abr_type == OSPF_ABR_SHORTCUT)
    if (!OSPF_IS_AREA_BACKBONE (area))
      if ((area->shortcut_configured == OSPF_SHORTCUT_DEFAULT &&
	   !ospf_top->backbone) ||
	  area->shortcut_configured == OSPF_SHORTCUT_ENABLE)
	SET_FLAG (flags, ROUTER_LSA_SHORTCUT);

  /* ASBR can't exit in stub area. */
  if (area->external_routing == OSPF_AREA_STUB)
    UNSET_FLAG (flags, OSPF_FLAG_ASBR);

  return flags;
}

/* Lookup neighbor other than myself.
   And check neighbor count,
   Point-to-Point link must have only 1 neighbor. */
struct ospf_neighbor *
ospf_nbr_lookup_ptop (struct route_table *nbrs, struct in_addr router_id)
{
  struct route_node *rn;
  struct ospf_neighbor *nbr = NULL;

  /* Search neighbor, there must be one of two nbrs. */
  for (rn = route_top (nbrs); rn; rn = route_next (rn))
    if ((nbr = rn->info) != NULL)
      /* Ignore myself. */
      if (!IPV4_ADDR_SAME (&nbr->router_id, &ospf_top->router_id))
	if (nbr->status == NSM_Full)
	  break;

  /* PtoP link must have only 1 neighbor. */
  if (ospf_nbr_count (nbrs, 0) > 1)
    zlog_warn ("Point-to-Point link has more than 2 neighobrs.");

  return nbr;
}

/* Set a link information. */
void
link_info_set (struct stream *s, struct in_addr id,
	       struct in_addr data, u_char type, u_char tos, u_int16_t cost)
{
  /* TOS based routing is not supported. */
  stream_put_ipv4 (s, id.s_addr);		/* Link ID. */
  stream_put_ipv4 (s, data.s_addr);		/* Link Data. */
  stream_putc (s, type);			/* Link Type. */
  stream_putc (s, tos);				/* TOS = 0. */
  stream_putw (s, cost);			/* Link Cost. */
}

/* Describe Point-to-Point link. */
int
lsa_link_ptop_set (struct stream *s, struct ospf_interface *oi)
{
  int links = 0;
  struct ospf_neighbor *nbr;
  struct in_addr id, mask;

  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
    zlog_info ("LSA[Type1]: Set link Point-to-Point");

  if ((nbr = ospf_nbr_lookup_ptop (oi->nbrs, ospf_top->router_id)))
    if (nbr->status == NSM_Full)
      {
	/* For unnumbered point-to-point networks, the Link Data field
	   should specify the interface's MIB-II ifIndex value. */
	link_info_set (s, nbr->router_id, oi->address->u.prefix4,
		       LSA_LINK_TYPE_POINTOPOINT, 0, oi->output_cost);
	links++;
      }

  if (oi->connected->destination != NULL)
    {
      /* Option 1:
	 link_type = LSA_LINK_TYPE_STUB;
	 link_id = nbr->address.u.prefix4;
	 link_data.s_addr = 0xffffffff;
	 link_cost = o->output_cost; */
      
      id.s_addr = oi->connected->destination->u.prefix4.s_addr;
      mask.s_addr = 0xffffffff;
      link_info_set (s, id, mask, LSA_LINK_TYPE_STUB, 0, oi->output_cost);
    }
  else
    {
       /* Option 2:  We need to include link to a stub
	 network regardless of the state of the neighbor */
      masklen2ip (oi->address->prefixlen, &mask);
      id.s_addr = oi->address->u.prefix4.s_addr & mask.s_addr;
      link_info_set (s, id, mask, LSA_LINK_TYPE_STUB, 0, oi->output_cost);
    }
  links++;

  return links;
}

/* Describe Broadcast Link. */
int
lsa_link_broadcast_set (struct stream *s, struct ospf_interface *oi)
{
  struct ospf_neighbor *dr;
  struct in_addr id, mask;

  /* Describe Type 3 Link. */
  if (oi->status == ISM_Waiting)
    {
      masklen2ip (oi->address->prefixlen, &mask);
      id.s_addr = oi->address->u.prefix4.s_addr & mask.s_addr;
      link_info_set (s, id, mask, LSA_LINK_TYPE_STUB, 0, oi->output_cost);
      return 1;
    }

  dr = ospf_nbr_lookup_by_addr (oi->nbrs, &DR (oi));
  /* Describe Type 2 link. */
  if (dr && (dr->status == NSM_Full ||
	     IPV4_ADDR_SAME (&oi->address->u.prefix4, &DR (oi))) &&
      ospf_nbr_count (oi->nbrs, NSM_Full) > 0)
    {
      link_info_set (s, DR (oi), oi->address->u.prefix4,
		     LSA_LINK_TYPE_TRANSIT, 0, oi->output_cost);
    }
  /* Describe type 3 link. */
  else
    {
      masklen2ip (oi->address->prefixlen, &mask);
      id.s_addr = oi->address->u.prefix4.s_addr & mask.s_addr;
      link_info_set (s, id, mask, LSA_LINK_TYPE_STUB, 0, oi->output_cost);
    }
  return 1;
}

int
lsa_link_loopback_set (struct stream *s, struct ospf_interface *oi)
{
  struct in_addr id, mask;

  /* Describe Type 3 Link. */
  if (oi->status != ISM_Loopback)
    return 0;

  mask.s_addr = 0xffffffff;
  id.s_addr = oi->address->u.prefix4.s_addr;
  link_info_set (s, id, mask, LSA_LINK_TYPE_STUB, 0, oi->output_cost);
  return 1;
}

/* Describe Virtual Link. */
int
lsa_link_virtuallink_set (struct stream *s, struct ospf_interface *oi)
{
  struct ospf_neighbor *nbr;

  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
    zlog_info ("LSA[Type1]: Set link type VL, state %d", oi->status);

  if (oi->status == ISM_PointToPoint)
    if ((nbr = ospf_nbr_lookup_ptop (oi->nbrs, ospf_top->router_id)))
      if (nbr->status == NSM_Full)
	{
	  link_info_set (s, nbr->router_id, oi->address->u.prefix4,
			 LSA_LINK_TYPE_VIRTUALLINK, 0, oi->output_cost);
	  return 1;
	}

  return 0;
}

#define lsa_link_nbma_set(S,O)  lsa_link_broadcast_set (S, O)

/* Set router-LSA link information. */
int
router_lsa_link_set (struct stream *s, struct ospf_area *area)
{
  listnode node;
  int links = 0;

  for (node = listhead (area->oiflist); node; node = nextnode (node))
    {
      struct ospf_interface *oi = node->data;
      struct interface *ifp = oi->ifp;

      /* Check interface is up, OSPF is enable. */
      if (if_is_up (ifp))
	{
	  if (oi->status != ISM_Down)
	    {
	      /* Describe each link. */
	      switch (oi->type)
		{
		case OSPF_IFTYPE_POINTOPOINT:
		  links += lsa_link_ptop_set (s, oi);
		  break;
		case OSPF_IFTYPE_BROADCAST:
		  links += lsa_link_broadcast_set (s, oi);
		  break;
		case OSPF_IFTYPE_NBMA:
		  links += lsa_link_nbma_set (s, oi);
		  break;
		case OSPF_IFTYPE_POINTOMULTIPOINT:
		  /* Not supproted yet. */
		  break;
		case OSPF_IFTYPE_VIRTUALLINK:
		  links += lsa_link_virtuallink_set (s, oi);
		  break;
		case OSPF_IFTYPE_LOOPBACK:
		  links += lsa_link_loopback_set (s, oi); 
		}
	    }
	}
    }

  return links;
}

/* Set router-LSA body. */
void
ospf_router_lsa_body_set (struct stream *s, struct ospf_area *area)
{
  unsigned long putp;
  u_int16_t cnt;

  /* Set flags. */
  stream_putc (s, router_lsa_flags (area));

  /* Set Zero fields. */
  stream_putc (s, 0);

  /* Keep pointer to # links. */
  putp = s->putp;

  /* Forward word */
  stream_putw(s, 0);

  /* Set all link information. */
  cnt = router_lsa_link_set (s, area);

  /* Set # of links here. */
  stream_putw_at (s, putp, cnt);
}

/* Create new router-LSA. */
struct ospf_lsa *
ospf_router_lsa_new (struct ospf_area *area)
{
  struct stream *s;
  struct lsa_header *lsah;
  struct ospf_lsa *new;
  int length;

  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
    zlog_info ("LSA[Type1]: Create router-LSA instance");

  /* Create a stream for LSA. */
  s = stream_new (OSPF_MAX_LSA_SIZE);
  lsah = (struct lsa_header *) STREAM_DATA (s);

#define LSA_OPTIONS_GET(A) \
        (((A)->external_routing == OSPF_AREA_DEFAULT) ? OSPF_OPTION_E : 0)

#ifdef HAVE_NSSA
#define LSA_NSSA_GET(A) \
        (((A)->external_routing == OSPF_AREA_NSSA) ? area->NSSATranslator : 0)

  /* Set LSA common header fields. */
  lsa_header_set (s, LSA_OPTIONS_GET (area) | LSA_NSSA_GET (area),
		  OSPF_ROUTER_LSA, ospf_top->router_id);
#else /* ! HAVE_NSSA */
  /* Set LSA common header fields. */
  lsa_header_set (s, LSA_OPTIONS_GET (area),
		  OSPF_ROUTER_LSA, ospf_top->router_id);
#endif /* HAVE_NSSA */

  /* Set router-LSA body fields. */
  ospf_router_lsa_body_set (s, area);

  /* Set length. */
  length = stream_get_endp (s);
  lsah->length = htons (length);

  /* Now, create OSPF LSA instance. */
  new = ospf_lsa_new ();
  new->area = area;
  SET_FLAG (new->flags, OSPF_LSA_SELF);

  /* Copy LSA data to store, discard stream. */
  new->data = ospf_lsa_data_new (length);
  memcpy (new->data, lsah, length);
  stream_free (s);

  return new;
}

/* Originate Router-LSA. */
struct ospf_lsa *
ospf_router_lsa_originate (struct ospf_area *area)
{
  struct ospf_lsa *new;

  /* Create new router-LSA instance. */
  new = ospf_router_lsa_new (area);

  /* Sanity check. */
  if (new->data->adv_router.s_addr == 0)
    {
      if (IS_DEBUG_OSPF_EVENT)
	zlog_info ("LSA[Type1]: AdvRouter is 0, discard");
      ospf_lsa_discard (new);
      return NULL;
    }

  /* Install LSA to LSDB. */
  new = ospf_lsa_install (NULL, new);

  /* Update LSA origination count. */
  ospf_top->lsa_originate_count++;

  /* Flooding new LSA through area. */
  ospf_flood_through_area (area, NULL, new);

  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
    {
      zlog_info ("LSA[Type%d:%s]: Originate router-LSA %p",
		 new->data->type, inet_ntoa (new->data->id), new);
      ospf_lsa_header_dump (new->data);
    }

  return new;
}

/* Refresh router-LSA. */
struct ospf_lsa *
ospf_router_lsa_refresh (struct ospf_lsa *lsa)
{
  struct ospf_area *area = lsa->area;
  struct ospf_lsa *new;

  /* Sanity check. */
  assert (lsa->data);

  /* Delete LSA from neighbor retransmit-list. */
  ospf_ls_retransmit_delete_nbr_all (area, lsa);

  /* Create new router-LSA instance. */
  new = ospf_router_lsa_new (area);
  new->data->ls_seqnum = lsa_seqnum_increment (lsa);

  ospf_lsa_install (NULL, new);

  /* Flood LSA through area. */
  ospf_flood_through_area (area, NULL, new);

  /* Debug logging. */
  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
    {
      zlog_info ("LSA[Type%d:%s]: router-LSA refresh",
		 new->data->type, inet_ntoa (new->data->id));
      ospf_lsa_header_dump (new->data);
    }

  return NULL;
}

int
ospf_router_lsa_timer (struct thread *t)
{
  struct ospf_area *area;

  if (IS_DEBUG_OSPF_EVENT)
    zlog_info ("Timer[router-LSA]: (router-LSA Refresh expire)");

  area = THREAD_ARG (t);
  area->t_router_lsa_self = NULL;

  /* Now refresh router-LSA. */
  if (area->router_lsa_self)
    ospf_router_lsa_refresh (area->router_lsa_self);
  /* Newly originate router-LSA. */
  else
    ospf_router_lsa_originate (area);

  return 0;
}

void
ospf_router_lsa_timer_add (struct ospf_area *area)
{
  /* Keep area's self-originated router-LSA. */
  struct ospf_lsa *lsa = area->router_lsa_self;

  /* Cancel previously scheduled router-LSA timer. */
  if (area->t_router_lsa_self)
    if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
      zlog_info ("LSA[Type1]: Cancel previous router-LSA timer");
  OSPF_TIMER_OFF (area->t_router_lsa_self);

  /* If router-LSA is originated previously, check the interval time. */
  if (lsa)
    {
      struct timeval delta, now;

      gettimeofday (&now, NULL);
      delta = tv_sub (now, lsa->tv_orig);

      if (tv_cmp (delta, int2tv (OSPF_MIN_LS_INTERVAL)) < 0)
        {
	  int delay = tv_ceil (tv_sub (int2tv (OSPF_MIN_LS_INTERVAL), delta));

	  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
	    zlog_info ("LSA[Type%d:%s]: Refresh timer delay %d seconds",
		       lsa->data->type, inet_ntoa (lsa->data->id), delay);
	  OSPF_AREA_TIMER_ON (area->t_router_lsa_self,
			      ospf_router_lsa_timer, delay);

	  return;
        }
    }

  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
    zlog_info ("LSA[Type1]: Scheduling router-LSA origination right away");

  /* Immediately refresh router-LSA. */
  OSPF_AREA_TIMER_ON (area->t_router_lsa_self, ospf_router_lsa_timer, 0);
}

int
ospf_router_lsa_update_timer (struct thread *t)
{
  listnode node;

  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
    zlog_info ("Timer[router-LSA Update]: (timer expire)");

  ospf_top->t_router_lsa_update = NULL;

  for (node = listhead (ospf_top->areas); node; nextnode (node))
    {
      struct ospf_area *area = getdata (node);
      struct ospf_lsa *lsa = area->router_lsa_self;
      struct router_lsa *rl;
      char *area_str;

      /* Keep Area ID string. */
      area_str = strdup (inet_ntoa (area->area_id));

      /* If LSA not exist in this Area, originate new. */
      if (lsa == NULL)
        {
	  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
	    zlog_info("LSA[Type1]: Create router-LSA for Area %s", area_str);

	  ospf_router_lsa_originate (area);
        }
      /* If router-ID is changed, Link ID must change.
	 First flush old LSA, then originate new. */
      else if (!IPV4_ADDR_SAME (&lsa->data->id, &ospf_top->router_id))
	{
	  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
	    zlog_info("LSA[Type%d:%s]: Refresh router-LSA for Area %s",
		      lsa->data->type, inet_ntoa (lsa->data->id), area_str);
	  ospf_lsa_flush_area (lsa, area);
	  ospf_lsa_unlock (area->router_lsa_self);
	  area->router_lsa_self = NULL;

	  /* Refresh router-LSA, (not install) and flood through area. */
	  ospf_router_lsa_timer_add (area);
	}
      else
	{
	  rl = (struct router_lsa *) lsa->data;
	  /* Refresh router-LSA, (not install) and flood through area. */
	  if (rl->flags != ospf_top->flags)
	    ospf_router_lsa_timer_add (area);
	}

      free (area_str);
    }

  return 0;
}


/* network-LSA related functions. */
/* Originate Network-LSA. */
void
ospf_network_lsa_body_set (struct stream *s, struct ospf_interface *oi)
{
  struct in_addr mask;
  struct route_node *rn;
  struct ospf_neighbor *nbr;

  masklen2ip (oi->address->prefixlen, &mask);
  stream_put_ipv4 (s, mask.s_addr);

  /* The network-LSA lists those routers that are fully adjacent to
    the Designated Router; each fully adjacent router is identified by
    its OSPF Router ID.  The Designated Router includes itself in this
    list. RFC2328, Section 12.4.2 */

  for (rn = route_top (oi->nbrs); rn; rn = route_next (rn))
    if ((nbr = rn->info) != NULL)
      if (nbr->status == NSM_Full || nbr == oi->nbr_self)
	stream_put_ipv4 (s, nbr->router_id.s_addr);
}

struct ospf_lsa *
ospf_network_lsa_new (struct ospf_interface *oi)
{
  struct stream *s;
  struct ospf_lsa *new;
  struct lsa_header *lsah;
  int length;

  /* If there are no neighbours on this network (the net is stub),
     the router does not originate network-LSA (see RFC 12.4.2) */
  if (oi->full_nbrs == 0)
    return NULL;
  
  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
    zlog_info ("LSA[Type2]: Create network-LSA instance");

  /* Create new stream for LSA. */
  s = stream_new (OSPF_MAX_LSA_SIZE);
  lsah = (struct lsa_header *) STREAM_DATA (s);

  lsa_header_set (s, (OPTIONS (oi) | LSA_OPTIONS_GET (oi->area)),
		  OSPF_NETWORK_LSA, DR (oi));

  /* Set network-LSA body fields. */
  ospf_network_lsa_body_set (s, oi);

  /* Set length. */
  length = stream_get_endp (s);
  lsah->length = htons (length);

  /* Create OSPF LSA instance. */
  new = ospf_lsa_new ();
  new->area = oi->area;
  SET_FLAG (new->flags, OSPF_LSA_SELF);

  /* Copy LSA to store. */
  new->data = ospf_lsa_data_new (length);
  memcpy (new->data, lsah, length);
  stream_free (s);

  return new;
}

/* Originate network-LSA. */
struct ospf_lsa *
ospf_network_lsa_originate (struct ospf_interface *oi)
{
  struct ospf_lsa *new;

  /* Create new network-LSA instance. */
  new = ospf_network_lsa_new (oi);
  if (new == NULL)
    return NULL;

  /* Install LSA to LSDB. */
  new = ospf_lsa_install (oi, new);

  /* Update LSA origination count. */
  ospf_top->lsa_originate_count++;

  /* Flooding new LSA through area. */
  ospf_flood_through_area (oi->area, NULL, new);

  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
    {
      zlog_info ("LSA[Type%d:%s]: Originate network-LSA %p",
		 new->data->type, inet_ntoa (new->data->id), new);
      ospf_lsa_header_dump (new->data);
    }

  return new;
}

int
ospf_network_lsa_refresh (struct ospf_lsa *lsa, struct ospf_interface *oi)
{
  struct ospf_area *area = lsa->area;
  struct ospf_lsa *new;

  assert (lsa->data);

  /* Delete LSA from neighbor retransmit-list. */
  ospf_ls_retransmit_delete_nbr_all (area, lsa);

  /* Create new network-LSA instance. */
  new = ospf_network_lsa_new (oi);
  if (new == NULL)
    return -1;
  new->data->ls_seqnum = lsa_seqnum_increment (lsa);

  ospf_lsa_install (oi, new);

  /* Flood LSA through aera. */
  ospf_flood_through_area (area, NULL, new);

  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
    {
      zlog_info ("LSA[Type%d:%s]: network-LSA refresh",
		 new->data->type, inet_ntoa (new->data->id));
      ospf_lsa_header_dump (new->data);
    }

  return 0;
}

int
ospf_network_lsa_refresh_timer (struct thread *t)
{
  struct ospf_interface *oi;

  oi = THREAD_ARG (t);
  oi->t_network_lsa_self = NULL;

  if (oi->network_lsa_self)
    /* Now refresh network-LSA. */
    ospf_network_lsa_refresh (oi->network_lsa_self, oi);
  else
    /* Newly create network-LSA. */
    ospf_network_lsa_originate (oi);

  return 0;
}

void
ospf_network_lsa_timer_add (struct ospf_interface *oi)
{
  /* Keep interface's self-originated network-LSA. */
  struct ospf_lsa *lsa = oi->network_lsa_self;

  /* Cancel previously schedules network-LSA timer. */
  if (oi->t_network_lsa_self)
    if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
      zlog_info ("LSA[Type2]: Cancel previous network-LSA timer");
  OSPF_TIMER_OFF (oi->t_network_lsa_self);

  /* If network-LSA is originated previously, check the interval time. */
  if (lsa)
    {
      struct timeval delta, now;

      gettimeofday (&now, NULL);
      delta = tv_sub (now, lsa->tv_orig);

      if (tv_cmp (delta, int2tv (OSPF_MIN_LS_INTERVAL)) < 0)
        {
	  int delay = tv_ceil (tv_sub (int2tv (OSPF_MIN_LS_INTERVAL), delta));

	  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
	    zlog_info ("LSA[Type%d:%s]: Refresh timer delay %d seconds",
		       lsa->data->type, inet_ntoa (lsa->data->id), delay);

          oi->t_network_lsa_self =
            thread_add_timer (master, ospf_network_lsa_refresh_timer,
			      oi, delay);
          return;
        }
    }

  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
    zlog_info ("Scheduling network-LSA origination right away");
  /* Immediately refresh network-LSA. */
  oi->t_network_lsa_self =
    thread_add_event (master, ospf_network_lsa_refresh_timer, oi, 0);
}


void
stream_put_ospf_metric (struct stream *s, u_int32_t metric_value)
{
  u_int32_t metric;
  char *mp;

  /* Put 0 metric. TOS metric is not supported. */
  metric = htonl (metric_value);
  mp = (char *) &metric;
  mp++;
  stream_put (s, mp, 3);
}

/* summary-LSA related functions. */
void
ospf_summary_lsa_body_set (struct stream *s, struct prefix *p,
			   u_int32_t metric)
{
  struct in_addr mask;

  masklen2ip (p->prefixlen, &mask);

  /* Put Network Mask. */
  stream_put_ipv4 (s, mask.s_addr);

  /* Set # TOS. */
  stream_putc (s, (u_char) 0);

  /* Set metric. */
  stream_put_ospf_metric (s, metric);
}

struct ospf_lsa *
ospf_summary_lsa_new (struct ospf_area *area, struct prefix *p,
		      u_int32_t metric, struct in_addr id)
{
  struct stream *s;
  struct ospf_lsa *new;
  struct lsa_header *lsah;
  int length;

  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
    zlog_info ("LSA[Type3]: Create summary-LSA instance");

  /* Create new stream for LSA. */
  s = stream_new (OSPF_MAX_LSA_SIZE);
  lsah = (struct lsa_header *) STREAM_DATA (s);

  lsa_header_set (s, LSA_OPTIONS_GET (area), OSPF_SUMMARY_LSA, id);

  /* Set summary-LSA body fields. */
  ospf_summary_lsa_body_set (s, p, metric);

  /* Set length. */
  length = stream_get_endp (s);
  lsah->length = htons (length);

  /* Create OSPF LSA instance. */
  new = ospf_lsa_new ();
  new->area = area;
  SET_FLAG (new->flags, OSPF_LSA_SELF);

  /* Copy LSA to store. */
  new->data = ospf_lsa_data_new (length);
  memcpy (new->data, lsah, length);
  stream_free (s);

  return new;
}

/* Originate Summary-LSA. */
struct ospf_lsa *
ospf_summary_lsa_originate (struct prefix_ipv4 *p, u_int32_t metric, 
			    struct ospf_area *area)
{
  struct ospf_lsa *new;
  struct in_addr id;
  
  id = ospf_lsa_unique_id (area->lsdb, OSPF_SUMMARY_LSA, p);

  /* Create new summary-LSA instance. */
  new = ospf_summary_lsa_new (area, (struct prefix *) p, metric, id);

  /* Instlal LSA to LSDB. */
  new = ospf_lsa_install (NULL, new);

  /* Update LSA origination count. */
  ospf_top->lsa_originate_count++;

  /* Flooding new LSA through area. */
  ospf_flood_through_area (area, NULL, new);

  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
    {
      zlog_info ("LSA[Type%d:%s]: Originate summary-LSA %p",
		 new->data->type, inet_ntoa (new->data->id), new);
      ospf_lsa_header_dump (new->data);
    }

  return new;
}

struct ospf_lsa*
ospf_summary_lsa_refresh (struct ospf_lsa *lsa)
{
  struct ospf_lsa *new;
  struct summary_lsa *sl;
  struct prefix p;
  
  /* Sanity check. */
  assert (lsa->data);

  sl = (struct summary_lsa *)lsa->data;
  p.prefixlen = ip_masklen (sl->mask);
  new = ospf_summary_lsa_new (lsa->area, &p, GET_METRIC (sl->metric),
			      sl->header.id);

  new->data->ls_seqnum = lsa_seqnum_increment (lsa);
  
  /* Re-calculate checksum. */
  ospf_lsa_checksum (new->data);

  ospf_lsa_install (NULL, new);
  
  /* Flood LSA through AS. */
  ospf_flood_through_area (new->area, NULL, new);

  /* Debug logging. */
  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
    {
      zlog_info ("LSA[Type%d:%s]: summary-LSA refresh",
		 new->data->type, inet_ntoa (new->data->id));
      ospf_lsa_header_dump (new->data);
    }
  
  return new;
}


/* summary-ASBR-LSA related functions. */
void
ospf_summary_asbr_lsa_body_set (struct stream *s, struct prefix *p,
				u_int32_t metric)
{
  struct in_addr mask;

  masklen2ip (p->prefixlen, &mask);

  /* Put Network Mask. */
  stream_put_ipv4 (s, mask.s_addr);

  /* Set # TOS. */
  stream_putc (s, (u_char) 0);

  /* Set metric. */
  stream_put_ospf_metric (s, metric);
}

struct ospf_lsa *
ospf_summary_asbr_lsa_new (struct ospf_area *area, struct prefix *p,
			   u_int32_t metric, struct in_addr id)
{
  struct stream *s;
  struct ospf_lsa *new;
  struct lsa_header *lsah;
  int length;

  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
    zlog_info ("LSA[Type3]: Create summary-LSA instance");

  /* Create new stream for LSA. */
  s = stream_new (OSPF_MAX_LSA_SIZE);
  lsah = (struct lsa_header *) STREAM_DATA (s);

  lsa_header_set (s, LSA_OPTIONS_GET (area), OSPF_SUMMARY_LSA_ASBR, id);

  /* Set summary-LSA body fields. */
  ospf_summary_asbr_lsa_body_set (s, p, metric);

  /* Set length. */
  length = stream_get_endp (s);
  lsah->length = htons (length);

  /* Create OSPF LSA instance. */
  new = ospf_lsa_new ();
  new->area = area;
  SET_FLAG (new->flags, OSPF_LSA_SELF);

  /* Copy LSA to store. */
  new->data = ospf_lsa_data_new (length);
  memcpy (new->data, lsah, length);
  stream_free (s);

  return new;
}

/* Originate summary-ASBR-LSA. */
struct ospf_lsa *
ospf_summary_asbr_lsa_originate (struct prefix_ipv4 *p, u_int32_t metric, 
				 struct ospf_area *area)
{
  struct ospf_lsa *new;
  struct in_addr id;
  
  id = ospf_lsa_unique_id (area->lsdb, OSPF_SUMMARY_LSA_ASBR, p);

  /* Create new summary-LSA instance. */
  new = ospf_summary_asbr_lsa_new (area, (struct prefix *) p, metric, id);

  /* Install LSA to LSDB. */
  new = ospf_lsa_install (NULL, new);
  
  /* Update LSA origination count. */
  ospf_top->lsa_originate_count++;

  /* Flooding new LSA through area. */
  ospf_flood_through_area (area, NULL, new);

  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
    {
      zlog_info ("LSA[Type%d:%s]: Originate summary-ASBR-LSA %p",
		 new->data->type, inet_ntoa (new->data->id), new);
      ospf_lsa_header_dump (new->data);
    }

  return new;
}

struct ospf_lsa*
ospf_summary_asbr_lsa_refresh (struct ospf_lsa *lsa)
{
  struct ospf_lsa *new;
  struct summary_lsa *sl;
  struct prefix p;

  /* Sanity check. */
  assert (lsa->data);

  sl = (struct summary_lsa *)lsa->data;
  p.prefixlen = ip_masklen (sl->mask);
  new = ospf_summary_asbr_lsa_new (lsa->area, &p, GET_METRIC (sl->metric),
				   sl->header.id);
  
  new->data->ls_seqnum = lsa_seqnum_increment (lsa);
  
  /* Re-calculate checksum. */
  ospf_lsa_checksum (new->data);

  ospf_lsa_install (NULL, new);
  
  /* Flood LSA through area. */
  ospf_flood_through_area (new->area, NULL, new);

  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
    {
      zlog_info ("LSA[Type%d:%s]: summary-ASBR-LSA refresh",
		 new->data->type, inet_ntoa (new->data->id));
      ospf_lsa_header_dump (new->data);
    }

  return new;
}

/* AS-external-LSA related functions. */

/* Get nexthop for AS-external-LSAs.  Return nexthop if its interface
   is connected, else 0*/
struct in_addr
ospf_external_lsa_nexthop_get (struct in_addr nexthop)
{
  struct in_addr fwd;
  struct prefix nh;
  /* struct route_node *rn; */
  listnode n1;

  fwd.s_addr = 0;

  if (!nexthop.s_addr)
    return fwd;

  /* Check whether nexthop is covered by OSPF network. */
  nh.family = AF_INET;
  nh.u.prefix4 = nexthop;
  nh.prefixlen = IPV4_MAX_BITLEN;

  for (n1 = listhead (ospf_top->oiflist); n1; nextnode (n1))
    {
      struct ospf_interface *oi = getdata (n1);

      if (if_is_up (oi->ifp))
	if (oi->address->family == AF_INET)
	  if (prefix_match (oi->address, &nh))
	    return nexthop;
    }

  return fwd;
}

#ifdef HAVE_NSSA
/* NSSA-external-LSA related functions. */

/* Get 1st IP connection for Forward Addr */
          
struct in_addr
ospf_get_ip_from_ifp (struct ospf_interface *oi)
{
  struct in_addr fwd;

  fwd.s_addr = 0;

  if (if_is_up (oi->ifp))
    return oi->address->u.prefix4;
  
  return fwd;
}

/* Get 1st IP connection for Forward Addr */
struct in_addr
ospf_get_nssa_ip (void)
{
  struct in_addr fwd;
  listnode n1;

  fwd.s_addr = 0;


  for (n1 = listhead (ospf_top->oiflist); n1; nextnode (n1))
    {
      struct ospf_interface *oi = getdata (n1);

      if (if_is_up (oi->ifp))
	if (oi->area->external_routing == OSPF_AREA_NSSA)
	  if (oi->address && oi->address->family == AF_INET)
	    return (oi->address->u.prefix4 );
    }

  return fwd;
}
#endif /* HAVE_NSSA */

#define DEFAULT_DEFAULT_METRIC	             20
#define DEFAULT_DEFAULT_ORIGINATE_METRIC     10
#define DEFAULT_DEFAULT_ALWAYS_METRIC	      1

#define DEFAULT_METRIC_TYPE		     EXTERNAL_METRIC_TYPE_2

int
metric_type (u_char src)
{
  return (ospf_top->dmetric[src].type < 0 ?
	  DEFAULT_METRIC_TYPE : ospf_top->dmetric[src].type);
}

int
metric_value (u_char src)
{
  if (ospf_top->dmetric[src].value < 0)
    {
      if (src == DEFAULT_ROUTE)
	{
	  if (ospf_top->default_originate == DEFAULT_ORIGINATE_ZEBRA)
	    return DEFAULT_DEFAULT_ORIGINATE_METRIC;
	  else
	    return DEFAULT_DEFAULT_ALWAYS_METRIC;
	}
      else if (ospf_top->default_metric < 0)
	return DEFAULT_DEFAULT_METRIC;
      else
	return ospf_top->default_metric;
    }

  return ospf_top->dmetric[src].value;
}

/* Set AS-external-LSA body. */
void
ospf_external_lsa_body_set (struct stream *s, struct external_info *ei)
{
  struct prefix_ipv4 *p = &ei->p;
  struct in_addr mask, fwd_addr;
  u_int32_t mvalue;
  int mtype;
  int type;

  /* Put Network Mask. */
  masklen2ip (p->prefixlen, &mask);
  stream_put_ipv4 (s, mask.s_addr);

  /* If prefix is default, specify DEFAULT_ROUTE. */
  type = is_prefix_default (&ei->p) ? DEFAULT_ROUTE : ei->type;
  
  mtype = (ROUTEMAP_METRIC_TYPE (ei) != -1) ?
    ROUTEMAP_METRIC_TYPE (ei) : metric_type (type);

  mvalue = (ROUTEMAP_METRIC (ei) != -1) ?
    ROUTEMAP_METRIC (ei) : metric_value (type);

  /* Put type of external metric. */
  stream_putc (s, (mtype == EXTERNAL_METRIC_TYPE_2 ? 0x80 : 0));

  /* Put 0 metric. TOS metric is not supported. */
  stream_put_ospf_metric (s, mvalue);
  
  /* Get forwarding address to nexthop if on the Connection List, else 0. */
  fwd_addr = ospf_external_lsa_nexthop_get (ei->nexthop);

  /* Put forwarding address. */
  stream_put_ipv4 (s, fwd_addr.s_addr);
  
  /* Put route tag -- This value should be introduced from configuration. */
  stream_putl (s, 0);
}

/* Create new external-LSA. */
struct ospf_lsa *
ospf_external_lsa_new (struct external_info *ei, struct in_addr *old_id)
{
  struct stream *s;
  struct lsa_header *lsah;
  struct ospf_lsa *new;
  struct in_addr id;
  int length;

  if (ei == NULL)
    {
      if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
	zlog_warn ("LSA[Type5]: External info is NULL, could not originated");
      return NULL;
    }

  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
    zlog_info ("LSA[Type5]: Originate AS-external-LSA instance");

  /* If old Link State ID is specified, refresh LSA with same ID. */
  if (old_id)
    id = *old_id;
  /* Get Link State with unique ID. */
  else
    {
      id = ospf_lsa_unique_id (ospf_top->lsdb, OSPF_AS_EXTERNAL_LSA, &ei->p);
      if (id.s_addr == 0xffffffff)
	{
	  /* Maybe Link State ID not available. */
	  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
	    zlog_info ("LSA[Type5]: Link ID not available, can't originate");
	  return NULL;
	}
    }

  /* Create new stream for LSA. */
  s = stream_new (OSPF_MAX_LSA_SIZE);
  lsah = (struct lsa_header *) STREAM_DATA (s);

  /* Set LSA common header fields. */
  lsa_header_set (s, OSPF_OPTION_E, OSPF_AS_EXTERNAL_LSA, id);

  /* Set AS-external-LSA body fields. */
  ospf_external_lsa_body_set (s, ei);

  /* Set length. */
  length = stream_get_endp (s);
  lsah->length = htons (length);

  /* Now, create OSPF LSA instance. */
  new = ospf_lsa_new ();
  new->area = NULL;
  SET_FLAG (new->flags, OSPF_LSA_SELF|OSPF_LSA_APPROVED);

  /* Copy LSA data to store, discard stream. */
  new->data = ospf_lsa_data_new (length);
  memcpy (new->data, lsah, length);
  stream_free (s);

  return new;
}

#ifdef HAVE_NSSA
/* Set AS-external-LSA body test. */
void
ospf_external_lsa_body_test (struct stream *s)
{
  struct in_addr mask, fwd_addr;
  u_int32_t mvalue = 0;
  /* int mtype;
     int type; */

  mask.s_addr = 0;
  fwd_addr.s_addr = 0;

  /* Put Network Mask. */
  /* masklen2ip (p->prefixlen, &mask); */
  stream_put_ipv4 (s, mask.s_addr);

  /* If prefix is default, specify DEFAULT_ROUTE. */
  /* type = is_prefix_default (&ei->p) ? DEFAULT_ROUTE : ei->type;
  
  mtype = (ROUTEMAP_METRIC_TYPE (ei) != -1) ?
  ROUTEMAP_METRIC_TYPE (ei) : metric_type (type);

  mvalue = (ROUTEMAP_METRIC (ei) != -1) ?
  ROUTEMAP_METRIC (ei) : metric_value (type); */

  /* Put type of external metric. */
  stream_putc (s,  0);

  /* Put 0 metric. TOS metric is not supported. */
  stream_put_ospf_metric (s, mvalue);
  
 
  /*  fwd_addr = ospf_top->router_id; */
       
  /* OLD == ospf_external_lsa_nexthop_get (ei->nexthop); */

  /* Put forwarding address. */
  /* stream_put_ipv4 (s, fwd_addr.s_addr); */
  stream_put_ipv4 (s, ospf_top->router_id.s_addr);
  
  /* Put route tag -- This value should be introduced from configuration. */
  stream_putl (s, 0);
}


/* Create test external-LSA. */
struct ospf_lsa *
ospf_external_lsa_test (int typ)
{
  struct stream *s;
  struct lsa_header *lsah;
  struct ospf_lsa *new;
  struct in_addr id;
  static int saddr = 0x05050505;
  int length;

  if (IS_DEBUG_OSPF_NSSA)
    zlog_info ("LSA[Type7]: Originate NSSA-external-LSA instance");

  id.s_addr = saddr--;
  if (id.s_addr == 0xffffffff)
    {
      /* Maybe Link State ID not available. */
      if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
	zlog_info ("LSA[Type5]: Link ID not available, can't originate");
      return NULL;
    }
 

  /* Create new stream for LSA. */
  s = stream_new (OSPF_MAX_LSA_SIZE);
  lsah = (struct lsa_header *) STREAM_DATA (s);


  /* Set LSA common header fields. */
  lsa_header_set (s, OSPF_OPTION_E, OSPF_AS_EXTERNAL_LSA, id);

  /* Set AS-external-LSA body fields. */
  ospf_external_lsa_body_test (s);


  /* Set length. */
  length = stream_get_endp (s);
  lsah->length = htons (length);

  /* Now, create OSPF LSA instance. */
  new = ospf_lsa_new ();
  new->area = NULL;
  SET_FLAG (new->flags, OSPF_LSA_SELF|OSPF_LSA_APPROVED);


  /* Copy LSA data to store, discard stream. */
  new->data = ospf_lsa_data_new (length);
  memcpy (new->data, lsah, length);
  stream_free (s);

  if (typ==7)
    new->data->type = OSPF_AS_NSSA_LSA; 

  ospf_lsa_checksum(new->data);

  /* return NULL; */

  return new;
}


/* As Type-7 */
void
ospf_install_flood_nssa (struct ospf_lsa *lsa, struct external_info *ei)
{
  struct ospf_lsa *new2;
  struct as_external_lsa *extlsa;

  /* NSSA Originate or Refresh (If anyNSSA)

  LSA is self-originated. And just installed as Type-5.
  Additionally, install as Type-7 LSDB for every attached NSSA.

  P-Bit controls which ABR performs translation to outside world; If
  we are an ABR....do not set the P-bit, because we send the Type-5,
  not as the ABR Translator, but as the ASBR owner within the AS!

  If we are NOT ABR, Flood through NSSA as Type-7 w/P-bit set.  The
  elected ABR Translator will see the P-bit, Translate, and re-flood.

  Later, ABR_TASK and P-bit will scan Type-7 LSDB and translate to
  Type-5's to non-NSSA Areas.  (it will also attempt a re-install) */

  /* make lsa duplicate, lock=1 */
  new2 = ospf_lsa_dup(lsa);

  /* make type-7 */
  new2->data->type  = OSPF_AS_NSSA_LSA;

  /* set P-bit if not ABR */
  if (! OSPF_IS_ABR)
    {
      SET_FLAG(new2->data->options, OSPF_OPTION_NP);

      /* set non-zero FWD ADDR 

      draft-ietf-ospf-nssa-update-09.txt

      if the network between the NSSA AS boundary router and the
      adjacent AS is advertised into OSPF as an internal OSPF route, 
      the forwarding address should be the next op address as is cu
      currently done with type-5 LSAs.  If the intervening network is 
      not adversited into OSPF as an internal OSPF route and the 
      type-7 LSA's P-bit is set a forwarding address should be 
      selected from one of the router's active OSPF inteface addresses
      which belong to the NSSA.  If no such addresses exist, then
      no type-7 LSA's with the P-bit set should originate from this
      router.   */

      extlsa = (struct as_external_lsa *)(lsa->data);

      if (extlsa->e[0].fwd_addr.s_addr == 0) 
	extlsa->e[0].fwd_addr = ospf_get_nssa_ip(); /* this NSSA area in ifp */

      if (IS_DEBUG_OSPF_NSSA)
	if (extlsa->e[0].fwd_addr.s_addr == 0) 
	  {
	    zlog_info ("LSA[Type-7]: Could not build FWD-ADDR");
	    ospf_lsa_discard(new2);
	    return;
	  }
    }

  /* Re-calculate checksum. */
  ospf_lsa_checksum (new2->data);

  /* install also as Type-7 */
  ospf_lsa_install (NULL, new2);   /* Remove Old, Lock New = 2 */

  /* will send each copy, lock=2+n */
  ospf_flood_through_as (NULL, new2); /* all attached NSSA's, no AS/STUBs */

  /* last send, lock=2 LSA is now permanent in Type-7 LSDB */
  /* It has the same ID as it's Type-5 Counter-Part */
 
}
#endif /* HAVE_NSSA */

int
is_prefix_default (struct prefix_ipv4 *p)
{
  struct prefix_ipv4 q;

  q.family = AF_INET;
  q.prefix.s_addr = 0;
  q.prefixlen = 0;

  return prefix_same ((struct prefix *) p, (struct prefix *) &q);
}

/* Originate an AS-external-LSA, install and flood. */
struct ospf_lsa *
ospf_external_lsa_originate (struct external_info *ei)
{
  struct ospf_lsa *new;

  /* Added for NSSA project....

       External LSAs are originated in ASBRs as usual, but for NSSA systems.
     there is the global Type-5 LSDB and a Type-7 LSDB installed for
     every area.  The Type-7's are flooded to every IR and every ABR; We
     install the Type-5 LSDB so that the normal "refresh" code operates
     as usual, and flag them as not used during ASE calculations.  The
     Type-7 LSDB is used for calculations.  Each Type-7 has a Forwarding
     Address of non-zero.

     If an ABR is the elected NSSA translator, following SPF and during
     the ABR task it will translate all the scanned Type-7's, with P-bit
     ON and not-self generated, and translate to Type-5's throughout the
     non-NSSA/STUB AS.

     A difference in operation depends whether this ASBR is an ABR
     or not.  If not an ABR, the P-bit is ON, to indicate that any
     elected NSSA-ABR can perform its translation.

     If an ABR, the P-bit is OFF;  No ABR will perform translation and
     this ASBR will flood the Type-5 LSA as usual.

     For the case where this ASBR is not an ABR, the ASE calculations
     are based on the Type-5 LSDB;  The Type-7 LSDB exists just to
     demonstrate to the user that there are LSA's that belong to any
     attached NSSA.

     Finally, it just so happens that when the ABR is translating every
     Type-7 into Type-5, it installs it into the Type-5 LSDB as an
     approved Type-5 (translated from Type-7);  at the end of translation
     if any Translated Type-5's remain unapproved, then they must be
     flushed from the AS.

     */
  
  /* Check the AS-external-LSA should be originated. */
  if (!ospf_redistribute_check (ei, NULL))
    return NULL;
  
  /* Create new AS-external-LSA instance. */
  if ((new = ospf_external_lsa_new (ei, NULL)) == NULL)
    {
      if (IS_DEBUG_OSPF_EVENT)
	zlog_info ("LSA[Type5:%s]: Could not originate AS-external-LSA",
		   inet_ntoa (ei->p.prefix));
      return NULL;
    }

  /* Install newly created LSA into Type-5 LSDB, lock = 1. */
  ospf_lsa_install (NULL, new);

  /* Update LSA origination count. */
  ospf_top->lsa_originate_count++;

  /* Flooding new LSA. only to AS (non-NSSA/STUB) */
  ospf_flood_through_as (NULL, new);

#ifdef HAVE_NSSA
  /* If there is any attached NSSA, do special handling */
  if (ospf_top->anyNSSA)
    ospf_install_flood_nssa (new, ei); /* Install/Flood Type-7 to all NSSAs */
#endif /* HAVE_NSSA */

  /* Debug logging. */
  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
    {
      zlog_info ("LSA[Type%d:%s]: Originate AS-external-LSA %p",
		 new->data->type, inet_ntoa (new->data->id), new);
      ospf_lsa_header_dump (new->data);
    }

  return new;
}

/* Originate AS-external-LSA from external info with initial flag. */
int
ospf_external_lsa_originate_timer (struct thread *t)
{
  struct route_node *rn;
  struct external_info *ei;
  struct route_table *rt;
  int type;

  ospf_top->t_external_lsa = NULL;
  type = THREAD_VAL (t);

  /* Originate As-external-LSA from all type of distribute source. */
  if ((rt = EXTERNAL_INFO (type)))
    for (rn = route_top (rt); rn; rn = route_next (rn))
      if ((ei = rn->info) != NULL)
	if (!is_prefix_default ((struct prefix_ipv4 *)&ei->p))
	  if (!ospf_external_lsa_originate (ei))
	    zlog_warn ("LSA: AS-external-LSA was not originated.");
  
  return 0;
}

struct external_info *
ospf_default_external_info ()
{
  int type;
  struct route_node *rn;
  struct prefix_ipv4 p;
  
  p.family = AF_INET;
  p.prefix.s_addr = 0;
  p.prefixlen = 0;

  /* First, lookup redistributed default route. */
  for (type = 0; type <= ZEBRA_ROUTE_MAX; type++)
    if (EXTERNAL_INFO (type) && type != ZEBRA_ROUTE_OSPF)
      {
	rn = route_node_lookup (EXTERNAL_INFO (type), (struct prefix *) &p);
	if (rn != NULL)
	  {
	    route_unlock_node (rn);
	    assert (rn->info);
	    if (ospf_redistribute_check (rn->info, NULL))
	      return rn->info;
	  }
      }

  return NULL;
}

int
ospf_default_originate_timer (struct thread *t)
{
  int *origin;
  struct prefix_ipv4 p;
  struct in_addr nexthop;
  struct external_info *ei;
  
  /* Get originate flags. */
  origin = THREAD_ARG (t);

  p.family = AF_INET;
  p.prefix.s_addr = 0;
  p.prefixlen = 0;

  if (*origin == DEFAULT_ORIGINATE_ALWAYS)
    {
      /* If there is no default route via redistribute,
	 then originate AS-external-LSA with nexthop 0 (self). */
      nexthop.s_addr = 0;
      ospf_external_info_add (DEFAULT_ROUTE, p, 0, nexthop);
    }

  if ((ei = ospf_default_external_info ()))
    ospf_external_lsa_originate (ei);
  
  return 0;
}

/* Flush an AS-external-LSA from LSDB and routing domain. */
void
ospf_external_lsa_flush (u_char type, struct prefix_ipv4 *p,
			 unsigned int ifindex, struct in_addr nexthop)
{
  struct ospf_lsa *lsa;

  if (IS_DEBUG_OSPF (lsa, LSA_FLOODING))
    zlog_info ("LSA: Flushing AS-external-LSA %s/%d",
	       inet_ntoa (p->prefix), p->prefixlen);

  /* First lookup LSA from LSDB. */
  if (!(lsa = ospf_external_info_find_lsa (p)))
    {
      if (IS_DEBUG_OSPF (lsa, LSA_FLOODING))
	zlog_warn ("LSA: There is no such AS-external-LSA %s/%d in LSDB",
		   inet_ntoa (p->prefix), p->prefixlen);
      return;
    }

  /* Sweep LSA from Link State Retransmit List. */
  ospf_ls_retransmit_delete_nbr_all (NULL, lsa);

  /* There must be no self-originated LSA in rtrs_external. */
#if 0
  /* Remove External route from Zebra. */
  ospf_zebra_delete ((struct prefix_ipv4 *) p, &nexthop);
#endif

  if (!IS_LSA_MAXAGE (lsa))
    {
      /* Unregister LSA from Refresh queue. */
      ospf_refresher_unregister_lsa (ospf_top, lsa);

      /* Flush AS-external-LSA through AS. */
      ospf_flush_through_as (lsa);
    }

  if (IS_DEBUG_OSPF (lsa, LSA_FLOODING))
    zlog_info ("ospf_external_lsa_flush(): stop");
}

void
ospf_external_lsa_refresh_default ()
{
  struct prefix_ipv4 p;
  struct external_info *ei;
  struct ospf_lsa *lsa;

  p.family = AF_INET;
  p.prefixlen = 0;
  p.prefix.s_addr = 0;

  ei = ospf_default_external_info ();
  lsa = ospf_external_info_find_lsa (&p);

  if (ei)
    {
      if (lsa)
	{
	  if (IS_DEBUG_OSPF_EVENT)
	    zlog_info ("LSA[Type5:0.0.0.0]: Refresh AS-external-LSA %p", lsa);
	  ospf_external_lsa_refresh (lsa, ei, LSA_REFRESH_FORCE);
	}
      else
	{
	  if (IS_DEBUG_OSPF_EVENT)
	    zlog_info ("LSA[Type5:0.0.0.0]: Originate AS-external-LSA");
	  ospf_external_lsa_originate (ei);
	}
    }
  else
    {
      if (lsa)
	{
	  if (IS_DEBUG_OSPF_EVENT)
	    zlog_info ("LSA[Type5:0.0.0.0]: Flush AS-external-LSA");
	  ospf_lsa_flush_as (lsa);
	}
    }
}

void
ospf_external_lsa_refresh_type (u_char type, int force)
{
  struct route_node *rn;
  struct external_info *ei;

  if (type != DEFAULT_ROUTE)
    if (EXTERNAL_INFO(type))
      /* Refresh each redistributed AS-external-LSAs. */
      for (rn = route_top (EXTERNAL_INFO (type)); rn; rn = route_next (rn))
	if ((ei = rn->info))
	  if (!is_prefix_default (&ei->p))
	    {
	      struct ospf_lsa *lsa;

	      if ((lsa = ospf_external_info_find_lsa (&ei->p)))
		ospf_external_lsa_refresh (lsa, ei, force);
	      else
		ospf_external_lsa_originate (ei);
	    }
}

/* Refresh AS-external-LSA. */
void
ospf_external_lsa_refresh (struct ospf_lsa *lsa,
			   struct external_info *ei, int force)
{
  struct ospf_lsa *new;
  int changed;
  
  /* Check the AS-external-LSA should be originated. */
  if (!ospf_redistribute_check (ei, &changed))
    {
      ospf_external_lsa_flush (ei->type, &ei->p, ei->ifindex, ei->nexthop);
      return;
    }

  if (!changed && !force)
    return;

  /* Delete LSA from neighbor retransmit-list. */
  ospf_ls_retransmit_delete_nbr_all (NULL, lsa);

  /* Unregister AS-external-LSA from refresh-list. */
  ospf_refresher_unregister_lsa (ospf_top, lsa);

  new = ospf_external_lsa_new (ei, &lsa->data->id);
  
  if (new == NULL)
    {
      if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
	zlog_warn ("LSA[Type%d:%s]: Could not be refreshed", lsa->data->type,
		   inet_ntoa (lsa->data->id));
      return;
    }
  
  new->data->ls_seqnum = lsa_seqnum_increment (lsa);

  /* Record timestamp. */
  gettimeofday (&new->tv_orig, NULL);

  /* Re-calculate checksum. */
  ospf_lsa_checksum (new->data);

  ospf_lsa_install (NULL, new);	/* As type-5. */

  /* Flood LSA through AS. */
  ospf_flood_through_as (NULL, new);

#ifdef HAVE_NSSA
  /* If any attached NSSA, install as Type-7, flood to all NSSA Areas */
  if (ospf_top->anyNSSA)
    ospf_install_flood_nssa (new, ei); /* Install/Flood per new rules */
#endif /* HAVE_NSSA */

  /* Register slef-originated LSA to refresh queue. */
  ospf_refresher_register_lsa (ospf_top, new);

  /* Debug logging. */
  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
    {
      zlog_info ("LSA[Type%d:%s]: AS-external-LSA refresh",
		 new->data->type, inet_ntoa (new->data->id));
      ospf_lsa_header_dump (new->data);
    }

  return;
}


/* LSA installation functions. */

/* Install router-LSA to an area. */
struct ospf_lsa *
ospf_router_lsa_install (struct ospf_lsa *new, int rt_recalc)
{
  struct ospf_area *area = new->area;

  /* RFC 2328 Section 13.2 Router-LSAs and network-LSAs
     The entire routing table must be recalculated, starting with
     the shortest path calculations for each area (not just the
     area whose link-state database has changed). 
  */
  if (rt_recalc)
    ospf_spf_calculate_schedule();

  if (IS_LSA_SELF (new))
    {
      /* Set router-LSA refresh timer. */
      OSPF_TIMER_OFF (area->t_router_lsa_self);
      OSPF_AREA_TIMER_ON (area->t_router_lsa_self,
			  ospf_router_lsa_timer, OSPF_LS_REFRESH_TIME);
      
      /* Set self-originated router-LSA. */
      ospf_lsa_unlock (area->router_lsa_self);
      area->router_lsa_self = ospf_lsa_lock (new);

      if (IS_DEBUG_OSPF (lsa, LSA_INSTALL))
	zlog_info("LSA[Type%d]: ID %s is self-originated",
		  new->data->type, inet_ntoa (new->data->id));
    }

  return new;
}

#define OSPF_INTERFACE_TIMER_ON(T,F,V) \
	if (!(T)) \
	  (T) = thread_add_timer (master, (F), oi, (V))

/* Install network-LSA to an area. */
struct ospf_lsa *
ospf_network_lsa_install (struct ospf_interface *oi, 
			  struct ospf_lsa *new,
			  int rt_recalc)
{

  /* RFC 2328 Section 13.2 Router-LSAs and network-LSAs
     The entire routing table must be recalculated, starting with
     the shortest path calculations for each area (not just the
     area whose link-state database has changed). 
  */
  if (rt_recalc)
    ospf_spf_calculate_schedule();

  /* We supposed that when LSA is originated by us, we pass the int
     for which it was originated. If LSA was received by flooding,
     the RECEIVED flag is set, so we do not link the LSA to the int. */
  if (IS_LSA_SELF (new) && !CHECK_FLAG (new->flags, OSPF_LSA_RECEIVED))
    {
      /* Set LSRefresh timer. */
      OSPF_TIMER_OFF (oi->t_network_lsa_self);

      OSPF_INTERFACE_TIMER_ON (oi->t_network_lsa_self,
			       ospf_network_lsa_refresh_timer,
			       OSPF_LS_REFRESH_TIME);

      ospf_lsa_unlock (oi->network_lsa_self);
      oi->network_lsa_self = ospf_lsa_lock (new);
    }

  return new;
}

/* Install summary-LSA to an area. */
struct ospf_lsa *
ospf_summary_lsa_install (struct ospf_lsa *new, int rt_recalc)
{

  if (rt_recalc && !IS_LSA_SELF (new))
    {
      /* RFC 2328 Section 13.2 Summary-LSAs
	 The best route to the destination described by the summary-
	 LSA must be recalculated (see Section 16.5).  If this
	 destination is an AS boundary router, it may also be
	 necessary to re-examine all the AS-external-LSAs.
      */

#if 0
      /* This doesn't exist yet... */
      ospf_summary_incremental_update(new); */
#else /* #if 0 */
      ospf_spf_calculate_schedule();
#endif /* #if 0 */
 
      if (IS_DEBUG_OSPF (lsa, LSA_INSTALL))
	zlog_info ("ospf_summary_lsa_install(): SPF scheduled");
    }

  if (IS_LSA_SELF (new))
    ospf_refresher_register_lsa (ospf_top, new);

  return new;
}

/* Install ASBR-summary-LSA to an area. */
struct ospf_lsa *
ospf_summary_asbr_lsa_install (struct ospf_lsa *new, int rt_recalc)
{
  if (rt_recalc && !IS_LSA_SELF (new))
    {
      /* RFC 2328 Section 13.2 Summary-LSAs
	 The best route to the destination described by the summary-
	 LSA must be recalculated (see Section 16.5).  If this
	 destination is an AS boundary router, it may also be
	 necessary to re-examine all the AS-external-LSAs.
      */
#if 0
      /* These don't exist yet... */
      ospf_summary_incremental_update(new);
      /* Isn't this done by the above call? 
	 - RFC 2328 Section 16.5 implies it should be */
      /* ospf_ase_calculate_schedule(); */
#else  /* #if 0 */
      ospf_spf_calculate_schedule();
#endif /* #if 0 */
    }

  /* register LSA to refresh-list. */
  if (IS_LSA_SELF (new))
    ospf_refresher_register_lsa (ospf_top, new);

  return new;
}

/* Install AS-external-LSA. */
struct ospf_lsa *
ospf_external_lsa_install (struct ospf_lsa *new, int rt_recalc)
{
  ospf_ase_register_external_lsa (new, ospf_top);
  /* If LSA is not self-originated, calculate an external route. */
  if (rt_recalc)
    {
      /* RFC 2328 Section 13.2 AS-external-LSAs
            The best route to the destination described by the AS-
            external-LSA must be recalculated (see Section 16.6).
      */

      if (!IS_LSA_SELF (new))
	ospf_ase_incremental_update (new, ospf_top);
    }

  /* Register self-originated LSA to refresh queue. */
  if (IS_LSA_SELF (new))
    ospf_refresher_register_lsa (ospf_top, new);

  return new;
}

void
ospf_discard_from_db (struct ospf_lsdb *lsdb, struct ospf_lsa *lsa)
{
  struct ospf_lsa *old;
  
  old = ospf_lsdb_lookup (lsdb, lsa);

  if (!old)
    return;

  if (old->refresh_list >= 0)
    ospf_refresher_unregister_lsa (ospf_top, old);

  ospf_ls_retransmit_delete_nbr_all (old->area, old);

  if (old->data->type == OSPF_AS_EXTERNAL_LSA)
    ospf_ase_unregister_external_lsa (old, ospf_top);
  ospf_lsa_maxage_delete (old);
  ospf_lsa_discard (old);
}

/* callback for foreach_lsa */
int
ospf_lsa_discard_callback (struct ospf_lsa *lsa, void *p, int i)
{
#ifdef HAVE_NSSA
  /* Removed: Stay away from any Local Translated Type-7 LSAs */
  /* if (CHECK_FLAG (lsa->flags, OSPF_LSA_LOCAL_XLT))
     return 0; */
#endif /* HAVE_NSSA */
  ospf_discard_from_db ((struct ospf_lsdb *)p, lsa);
  return 0;
}

struct ospf_lsa *
ospf_lsa_install (struct ospf_interface *oi, struct ospf_lsa *lsa)
{
  struct ospf_lsa *new = NULL;
  struct ospf_lsa *old = NULL;
  struct ospf_lsdb *lsdb = NULL;
  int rt_recalc;

  /* Set LSDB. */
  lsdb = lsa->data->type != OSPF_AS_EXTERNAL_LSA ?
    lsa->area->lsdb : ospf_top->lsdb;

#ifdef HAVE_NSSA
  if (IS_DEBUG_OSPF_NSSA)
    {
      zlog_info ("LSA[Installing]: Type-%d ", lsa->data->type);

      if  (lsa->data->type == OSPF_AS_NSSA_LSA )
	zlog_info ("NSSA LSA AREA = %s", inet_ntoa (lsa->area->area_id));
    }
#endif /* HAVE_NSSA */

  assert (lsdb);

  /*  RFC 2328 13.2.  Installing LSAs in the database

        Installing a new LSA in the database, either as the result of
        flooding or a newly self-originated LSA, may cause the OSPF
        routing table structure to be recalculated.  The contents of the
        new LSA should be compared to the old instance, if present.  If
        there is no difference, there is no need to recalculate the
        routing table. When comparing an LSA to its previous instance,
        the following are all considered to be differences in contents:

            o   The LSA's Options field has changed.

            o   One of the LSA instances has LS age set to MaxAge, and
                the other does not.

            o   The length field in the LSA header has changed.

            o   The body of the LSA (i.e., anything outside the 20-byte
                LSA header) has changed. Note that this excludes changes
                in LS Sequence Number and LS Checksum.

  */
  /* Look up old LSA and determine if any SPF calculation or incremental
     update is needed */
  old = ospf_lsdb_lookup (lsdb, lsa);

  /* Do comparision and record if recalc needed. */
  rt_recalc = 0;
  if (  old == NULL || ospf_lsa_different(old, lsa))
    rt_recalc = 1;

  /* discard old LSA from LSDB */
  ospf_discard_from_db (lsdb, lsa);
  /* Insert LSA to LSDB. */
  ospf_lsdb_add (lsdb, lsa);
  lsa->lsdb = lsdb;

  /* Calculate Checksum if self-originated?. */
  if (IS_LSA_SELF (lsa))
    ospf_lsa_checksum (lsa->data);

  /* Do LSA specific installation process. */
  switch (lsa->data->type)
    {
    case OSPF_ROUTER_LSA:
      new = ospf_router_lsa_install (lsa, rt_recalc);
      break;
    case OSPF_NETWORK_LSA:
      assert (oi);
      new = ospf_network_lsa_install (oi, lsa, rt_recalc);
      break;
    case OSPF_SUMMARY_LSA:
      new = ospf_summary_lsa_install (lsa, rt_recalc);
      break;
    case OSPF_SUMMARY_LSA_ASBR:
      new = ospf_summary_asbr_lsa_install (lsa, rt_recalc);
      break;
    case OSPF_AS_EXTERNAL_LSA:
      new = ospf_external_lsa_install (lsa, rt_recalc);
      break;
    default: /* NSSA, or type-6,8,9....nothing special */
#ifdef HAVE_NSSA
      new = ospf_external_lsa_install (lsa, rt_recalc);
#endif /* HAVE_NSSA */
      break;
    }

  /* Debug logs. */
  if (IS_DEBUG_OSPF (lsa, LSA_INSTALL))
    {
      char area_str[25];

      if (new->data->type != OSPF_AS_EXTERNAL_LSA)
	sprintf (area_str, " to Area %s", inet_ntoa (new->area->area_id));
      else
	area_str[0] = '\0';

      zlog_info ("LSA[Type%d:%s]: Install %s%s", new->data->type,
		 inet_ntoa (new->data->id),
		 LOOKUP (ospf_lsa_type_msg, new->data->type), area_str);
    }

  /* If received LSA' ls_age is MaxAge, set LSA on MaxAge LSA list. */
  if (IS_LSA_MAXAGE (new) && !IS_LSA_SELF (new))
    {
      if (IS_DEBUG_OSPF (lsa, LSA_FLOODING))
	zlog_info ("LSA[Type%d:%s]: Install LSA, MaxAge",
		   new->data->type, inet_ntoa (new->data->id));
      ospf_lsa_maxage (lsa);
    }

  return new;
}


int
ospf_check_nbr_status ()
{
  listnode node;

  for (node = listhead (ospf_top->oiflist); node; node = nextnode (node))
    {
      struct ospf_interface *oi = getdata (node);
      struct route_node *rn;
      struct ospf_neighbor *nbr;

      if (ospf_if_is_enable (oi))
	for (rn = route_top (oi->nbrs); rn; rn = route_next (rn))
          if ((nbr = rn->info) != NULL)
	    if (nbr->status == NSM_Exchange || nbr->status == NSM_Loading)
	      {
		route_unlock_node (rn);
		return 0;
	      }
    }

  return 1;
}


/* This function flood the maxaged LSA to DR. */
void
ospf_maxage_flood (struct ospf_lsa *lsa)
{
  switch (lsa->data->type)
    {
    case OSPF_ROUTER_LSA:
    case OSPF_NETWORK_LSA:
    case OSPF_SUMMARY_LSA:
    case OSPF_SUMMARY_LSA_ASBR:
#ifdef HAVE_NSSA
    case OSPF_AS_NSSA_LSA:
#endif /* HAVE_NSSA */
      ospf_flood_through_area (lsa->area, NULL, lsa);
      break;
    case OSPF_AS_EXTERNAL_LSA:
      ospf_flood_through_as (NULL, lsa);
      break;
    default:
      break;
    }
}

int
ospf_maxage_lsa_remover (struct thread *thread)
{
  listnode node;
  listnode next;
  int reschedule = 0;

  ospf_top->t_maxage = NULL;

  if (IS_DEBUG_OSPF (lsa, LSA_FLOODING))
    zlog_info ("LSA[MaxAge]: remover Start");

  reschedule = !ospf_check_nbr_status ();

  if (!reschedule)
    for (node = listhead (ospf_top->maxage_lsa); node; node = next)
      {
        struct ospf_lsa *lsa = getdata (node);
        next = node->next;

        if (lsa->retransmit_counter > 0)
          {
            reschedule = 1;
            continue;
          }

        /* Remove LSA from the LSDB */
        if (CHECK_FLAG (lsa->flags, OSPF_LSA_SELF))
          if (IS_DEBUG_OSPF (lsa, LSA_FLOODING))
            zlog_info ("LSA[Type%d:%s]: This LSA is self-oririnated: ",
                       lsa->data->type, inet_ntoa (lsa->data->id));

        if (IS_DEBUG_OSPF (lsa, LSA_FLOODING))
          zlog_info ("LSA[Type%d:%s]: MaxAge LSA removed from list",
                     lsa->data->type, inet_ntoa (lsa->data->id));

	/* Flood max age LSA. */
	ospf_maxage_flood (lsa);

	/* Remove from lsdb. */
        ospf_discard_from_db (lsa->lsdb, lsa);
        ospf_lsdb_delete (lsa->lsdb, lsa);
      }

  /*    A MaxAge LSA must be removed immediately from the router's link
        state database as soon as both a) it is no longer contained on any
        neighbor Link state retransmission lists and b) none of the router's
        neighbors are in states Exchange or Loading. */
  if (reschedule)
    OSPF_SCHEDULE_MAXAGE (ospf_top->t_maxage, ospf_maxage_lsa_remover);

  return 0;
}

int
ospf_lsa_maxage_exist (struct ospf_lsa *new)
{
  listnode node;

  for (node = listhead (ospf_top->maxage_lsa); node; nextnode (node))
    if (((struct ospf_lsa *) node->data) == new)
      return 1;

  return 0;
}

void
ospf_lsa_maxage_delete (struct ospf_lsa *lsa)
{
  listnode n;

  if ((n = listnode_lookup (ospf_top->maxage_lsa, lsa)))
    {
      list_delete_node (ospf_top->maxage_lsa, n);
      ospf_lsa_unlock (lsa);
    }
}

void
ospf_lsa_maxage (struct ospf_lsa *lsa)
{
  /* When we saw a MaxAge LSA flooded to us, we put it on the list
     and schedule the MaxAge LSA remover. */
  if (ospf_lsa_maxage_exist (lsa))
    {
      if (IS_DEBUG_OSPF (lsa, LSA_FLOODING))
	zlog_info ("LSA[Type%d:%s]: %p already exists on MaxAge LSA list",
		   lsa->data->type, inet_ntoa (lsa->data->id), lsa);
      return;
    }

  listnode_add (ospf_top->maxage_lsa, ospf_lsa_lock (lsa));

  if (IS_DEBUG_OSPF (lsa, LSA_FLOODING))
    zlog_info ("LSA[Type%d:%s]: MaxAge LSA remover scheduled.",
	       lsa->data->type, inet_ntoa (lsa->data->id));

  OSPF_SCHEDULE_MAXAGE (ospf_top->t_maxage, ospf_maxage_lsa_remover);
}

int
ospf_lsa_maxage_walker_remover (struct ospf_lsa *lsa, void *p_arg, int int_arg)
{
#ifdef HAVE_NSSA
  /* Stay away from any Local Translated Type-7 LSAs */
  if (CHECK_FLAG (lsa->flags, OSPF_LSA_LOCAL_XLT))
    return 0;
#endif /* HAVE_NSSA */

  if (IS_LSA_MAXAGE (lsa))
    /* Self-originated LSAs should NOT time-out instead,
       they're flushed and submitted to the max_age list explicitly. */
    if (!ospf_lsa_is_self_originated (lsa))
      {
	if (IS_DEBUG_OSPF (lsa, LSA_FLOODING))
	  zlog_info("LSA[Type%d:%s]: is MaxAge", lsa->data->type,
		    inet_ntoa (lsa->data->id));

	if (lsa->data->type == OSPF_AS_EXTERNAL_LSA)
	  ospf_ase_incremental_update (lsa, ospf_top);
	else
	  ospf_spf_calculate_schedule ();

	ospf_lsa_maxage (lsa);
      }

  return 0;
}

/* Periodical check of MaxAge LSA. */
int
ospf_lsa_maxage_walker (struct thread *t)
{
  listnode node;

  ospf_top->t_maxage_walker = NULL;

  for (node = listhead (ospf_top->areas); node; nextnode (node))
    {
      struct ospf_area *area = node->data;

      foreach_lsa (ROUTER_LSDB (area), NULL, 0,
		   ospf_lsa_maxage_walker_remover);
      foreach_lsa (NETWORK_LSDB (area), NULL, 0,
		   ospf_lsa_maxage_walker_remover);
      foreach_lsa (SUMMARY_LSDB (area), NULL, 0,
		   ospf_lsa_maxage_walker_remover);
      foreach_lsa (SUMMARY_ASBR_LSDB (area), NULL, 0,
		   ospf_lsa_maxage_walker_remover);
    }

  /* for AS-eternal-LSAs. */
  if (ospf_top->lsdb)
    foreach_lsa (EXTERNAL_LSDB (ospf_top), NULL, 0,
		 ospf_lsa_maxage_walker_remover);

  ospf_top->t_maxage_walker = 
    thread_add_timer (master, ospf_lsa_maxage_walker, NULL,
                      OSPF_LSA_MAXAGE_CHECK_INTERVAL);
  return 0;
}

int
find_summary (struct ospf_lsa *lsa, void * v, int i)
{
  struct prefix_ipv4 *p, pr;

  if ((p = (struct prefix_ipv4 *) v) != NULL)
    if (lsa != NULL)
      /* We're looking for self-originated one */
      if (ospf_lsa_is_self_originated (lsa))
	{
	  struct summary_lsa *sl = (struct summary_lsa *) lsa->data;

	  pr.family = AF_INET;
	  pr.prefix = sl->header.id;
	  pr.prefixlen = ip_masklen (sl->mask);
	  apply_mask_ipv4 (&pr);

	  if (prefix_same ((struct prefix*) &pr, (struct prefix*) p))
	    return 1;
	}

  return 0;
}

int
find_asbr_summary (struct ospf_lsa *lsa, void * v, int i)
{
  struct prefix_ipv4 *p;

  if ((p = (struct prefix_ipv4 *) v) != NULL)
    if (lsa != NULL)
      /* We're looking for self-originated one */
      if (ospf_lsa_is_self_originated (lsa))
	{
	  struct summary_lsa *sl = (struct summary_lsa *) lsa->data;

	  if (IPV4_ADDR_SAME (&p->prefix, &sl->header.id))
	    return 1;
	}

  return 0;
}

struct ospf_lsa *
ospf_lsa_lookup (struct ospf_area *area, u_int32_t type,
                 struct in_addr id, struct in_addr adv_router)
{
  switch (type)
    {
    case OSPF_ROUTER_LSA:
    case OSPF_NETWORK_LSA:
    case OSPF_SUMMARY_LSA:
    case OSPF_SUMMARY_LSA_ASBR:
#ifdef HAVE_NSSA
    case OSPF_AS_NSSA_LSA:
#endif /* HAVE_NSSA */
      return ospf_lsdb_lookup_by_id (area->lsdb, type, id, adv_router);
      break;
    case OSPF_AS_EXTERNAL_LSA:
      return ospf_lsdb_lookup_by_id (ospf_top->lsdb, type, id, adv_router);
      break;
    default:
      break;
    }

  return NULL;
}

struct ospf_lsa *
ospf_lsa_lookup_by_id (struct ospf_area *area, u_int32_t type, 
                       struct in_addr id)
{
  struct ospf_lsa *lsa;
  struct route_node *rn;

  switch (type)
    {
    case OSPF_ROUTER_LSA:
      return ospf_lsdb_lookup_by_id (area->lsdb, type, id, id);
      break;
    case OSPF_NETWORK_LSA:
      for (rn = route_top (NETWORK_LSDB (area)); rn; rn = route_next (rn))
	if ((lsa = rn->info))
	  if (IPV4_ADDR_SAME (&lsa->data->id, &id))
	    {
	      route_unlock_node (rn);
	      return lsa;
	    }
      break;
    case OSPF_SUMMARY_LSA:
    case OSPF_SUMMARY_LSA_ASBR:
      /* Currently not used. */
      assert (1);
      return ospf_lsdb_lookup_by_id (area->lsdb, type, id, id);
      break;
    case OSPF_AS_EXTERNAL_LSA:
    default:
      break;
    }

  return NULL;
}

struct ospf_lsa *
ospf_lsa_lookup_by_header (struct ospf_area *area, struct lsa_header *lsah)
{
  struct ospf_lsa *match;

  match = ospf_lsa_lookup (area, lsah->type, lsah->id, lsah->adv_router);

  if (match == NULL)
    if (IS_DEBUG_OSPF (lsa, LSA) == OSPF_DEBUG_LSA)
      zlog_info ("LSA[Type%d:%s]: Lookup by header, NO MATCH",
		 lsah->type, inet_ntoa (lsah->id));

  return match;
}

/* return +n, l1 is more recent.
   return -n, l2 is more recent.
   return 0, l1 and l2 is identical. */
int
ospf_lsa_more_recent (struct ospf_lsa *l1, struct ospf_lsa *l2)
{
  int r;
  int x, y;

  if (l1 == NULL && l2 == NULL)
    return 0;
  if (l1 == NULL)
    return -1;
  if (l2 == NULL)
    return 1;

  /* compare LS sequence number. */
  x = (int) ntohl (l1->data->ls_seqnum);
  y = (int) ntohl (l2->data->ls_seqnum);
  if (x > y)
    return 1;
  if (x < y)
    return -1;

  /* compare LS checksum. */
  r = ntohs (l1->data->checksum) - ntohs (l2->data->checksum);
  if (r)
    return r;

  /* compare LS age. */
  if (IS_LSA_MAXAGE (l1) && !IS_LSA_MAXAGE (l2))
    return 1;
  else if (!IS_LSA_MAXAGE (l1) && IS_LSA_MAXAGE (l2))
    return -1;

  /* compare LS age with MaxAgeDiff. */
  if (LS_AGE (l1) - LS_AGE (l2) > OSPF_LSA_MAXAGE_DIFF)
    return -1;
  else if (LS_AGE (l2) - LS_AGE (l1) > OSPF_LSA_MAXAGE_DIFF)
    return 1;

  /* LSAs are identical. */
  return 0;
}

/* If two LSAs are different, return 1, otherwise return 0. */
int
ospf_lsa_different (struct ospf_lsa *l1, struct ospf_lsa *l2)
{
  char *p1, *p2;
  assert (l1);
  assert (l2);
  assert (l1->data);
  assert (l2->data);

  if (l1->data->options != l2->data->options)
    return 1;

  if (IS_LSA_MAXAGE (l1) && !IS_LSA_MAXAGE (l2))
    return 1;

  if (IS_LSA_MAXAGE (l2) && !IS_LSA_MAXAGE (l1))
    return 1;

  if (l1->data->length != l2->data->length)
    return 1;

  if (l1->data->length ==  0)
    return 1;

  assert (l1->data->length > OSPF_LSA_HEADER_SIZE);

  p1 = (char *) l1->data;
  p2 = (char *) l2->data;

  if (memcmp (p1 + OSPF_LSA_HEADER_SIZE, p2 + OSPF_LSA_HEADER_SIZE,
              ntohs( l1->data->length ) - OSPF_LSA_HEADER_SIZE) != 0)
    return 1;

  return 0;
}

void
ospf_lsa_flush_self_originated (struct ospf_neighbor *nbr,
                                struct ospf_lsa *self,
                                struct ospf_lsa *new)
{
  u_int32_t seqnum;

  /* Adjust LS Sequence Number. */
  seqnum = ntohl (new->data->ls_seqnum) + 1;
  self->data->ls_seqnum = htonl (seqnum);

  /* Recalculate LSA checksum. */
  ospf_lsa_checksum (self->data);

  /* Reflooding LSA. */
  /*  RFC2328  Section 13.3
	    On non-broadcast networks, separate	Link State Update
	    packets must be sent, as unicasts, to each adjacent	neighbor
	    (i.e., those in state Exchange or greater).	 The destination
	    IP addresses for these packets are the neighbors' IP
	    addresses.   */
  if (nbr->oi->type == OSPF_IFTYPE_NBMA)
    {
      struct route_node *rn;
      struct ospf_neighbor *onbr;

      for (rn = route_top (nbr->oi->nbrs); rn; rn = route_next (rn))
	if ((onbr = rn->info) != NULL)
	  if (onbr != nbr->oi->nbr_self && onbr->status >= NSM_Exchange)
	    ospf_ls_upd_send_lsa (onbr, self, OSPF_SEND_PACKET_DIRECT);
    }
  else
  ospf_ls_upd_send_lsa (nbr, self, OSPF_SEND_PACKET_INDIRECT);

  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
    zlog_info ("LSA[Type%d:%s]: Flush self-originated LSA",
	       self->data->type, inet_ntoa (self->data->id));
}

/* If there is self-originated LSA, then return 1, otherwise return 0. */
/* An interface-independent version of ospf_lsa_is_self_originated */
int 
ospf_lsa_is_self_originated (struct ospf_lsa *lsa)
{
  listnode node;

  /* This LSA is already checked. */
  if (CHECK_FLAG (lsa->flags, OSPF_LSA_SELF_CHECKED))
    return CHECK_FLAG (lsa->flags, OSPF_LSA_SELF);

  /* Make sure LSA is self-checked. */
  SET_FLAG (lsa->flags, OSPF_LSA_SELF_CHECKED);

  /* AdvRouter and Router ID is the same. */
  if (IPV4_ADDR_SAME (&lsa->data->adv_router, &ospf_top->router_id))
    SET_FLAG (lsa->flags, OSPF_LSA_SELF);

  /* LSA is router-LSA. */
  else if (lsa->data->type == OSPF_ROUTER_LSA &&
      IPV4_ADDR_SAME (&lsa->data->id, &ospf_top->router_id))
    SET_FLAG (lsa->flags, OSPF_LSA_SELF);

  /* LSA is network-LSA.  Compare Link ID with all interfaces. */
  else if (lsa->data->type == OSPF_NETWORK_LSA)
    for (node = listhead (ospf_top->oiflist); node; nextnode (node))
      {
	struct ospf_interface *oi = getdata (node);

	/* Ignore virtual link. */
        if (oi->type != OSPF_IFTYPE_VIRTUALLINK)
	  if (oi->address->family == AF_INET)
	    if (IPV4_ADDR_SAME (&lsa->data->id, &oi->address->u.prefix4))
	      {
		/* to make it easier later */
		SET_FLAG (lsa->flags, OSPF_LSA_SELF);
		return CHECK_FLAG (lsa->flags, OSPF_LSA_SELF);
	      }
      }

  return CHECK_FLAG (lsa->flags, OSPF_LSA_SELF);
}

/* Get unique Link State ID. */
struct in_addr
ospf_lsa_unique_id (struct ospf_lsdb *lsdb, u_char type, struct prefix_ipv4 *p)
{
  struct ospf_lsa *lsa;
  struct in_addr mask, id;

  id = p->prefix;

  /* Check existence of LSA instance. */
  lsa = ospf_lsdb_lookup_by_id (lsdb, type, id, ospf_top->router_id);
  if (lsa)
    {
      struct as_external_lsa *al = (struct as_external_lsa *) lsa->data;
      if (ip_masklen (al->mask) == p->prefixlen)
	{
	  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
	    zlog_warn ("ospf_lsa_unique_id(): "
		       "Can't get Link State ID for %s/%d",
		       inet_ntoa (p->prefix), p->prefixlen);
	  /*	  id.s_addr = 0; */
	  id.s_addr = 0xffffffff;
	  return id;
	}
      /* Masklen differs, then apply wildcard mask to Link State ID. */
      else
	{
	  masklen2ip (p->prefixlen, &mask);

	  id.s_addr = p->prefix.s_addr | (~mask.s_addr);
	  lsa = ospf_lsdb_lookup_by_id (ospf_top->lsdb, type,
				       id, ospf_top->router_id);
	  if (lsa)
	    {
	      if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
		zlog_warn ("ospf_lsa_unique_id(): "
			   "Can't get Link State ID for %s/%d",
			   inet_ntoa (p->prefix), p->prefixlen);
	      /* 	      id.s_addr = 0; */
	      id.s_addr = 0xffffffff;
	      return id;
	    }
	}
    }

  return id;
}


#define LSA_ACTION_ORIGN_RTR  1
#define LSA_ACTION_ORIGN_NET  2
#define LSA_ACTION_FLOOD_AREA 3
#define LSA_ACTION_FLOOD_AS   4
#define LSA_ACTION_FLUSH_AREA 5
#define LSA_ACTION_FLUSH_AS   6

struct lsa_action
{
  u_char action;
  struct ospf_area *area;
  struct ospf_interface *oi; 
  struct ospf_lsa *lsa;
};

int
ospf_lsa_action (struct thread *t)
{
  struct lsa_action *data;

  data = THREAD_ARG (t);

  if (IS_DEBUG_OSPF (lsa, LSA) == OSPF_DEBUG_LSA)
    zlog_info ("LSA[Action]: Performing scheduled LSA action: %d",
	       data->action);

  switch (data->action)
    {
    case LSA_ACTION_ORIGN_RTR:
      ospf_router_lsa_refresh (data->area->router_lsa_self);
      break;
    case LSA_ACTION_ORIGN_NET:
      ospf_network_lsa_originate (data->oi);
      break;
    case LSA_ACTION_FLOOD_AREA:
      ospf_flood_through_area (data->area, NULL, data->lsa);
      break;
    case LSA_ACTION_FLOOD_AS:
      ospf_flood_through_as (NULL, data->lsa);
      break;
    case LSA_ACTION_FLUSH_AREA:
      ospf_lsa_flush_area (data->lsa, data->area);
      break;
    case LSA_ACTION_FLUSH_AS:
      ospf_lsa_flush_as (data->lsa);
      break;
    }

  ospf_lsa_unlock (data->lsa);
  XFREE (MTYPE_OSPF_MESSAGE, data);
  return 0;
}

void
ospf_schedule_lsa_flood_area (struct ospf_area *area, struct ospf_lsa *lsa)
{
  struct lsa_action *data;

  data = XMALLOC (MTYPE_OSPF_MESSAGE, sizeof (struct lsa_action));
  bzero (data, sizeof (struct lsa_action));

  data->action = LSA_ACTION_FLOOD_AREA;
  data->area = area;
  data->lsa  = ospf_lsa_lock (lsa);

  thread_add_event (master, ospf_lsa_action, data, 0);
}

void
ospf_schedule_lsa_flush_area (struct ospf_area *area, struct ospf_lsa *lsa)
{
  struct lsa_action *data;

  data = XMALLOC (MTYPE_OSPF_MESSAGE, sizeof (struct lsa_action));
  bzero (data, sizeof (struct lsa_action));

  data->action = LSA_ACTION_FLUSH_AREA;
  data->area = area;
  data->lsa  = ospf_lsa_lock (lsa);

  thread_add_event (master, ospf_lsa_action, data, 0);
}


/* LSA Refreshment functions. */
void
ospf_lsa_refresh (struct ospf_lsa *lsa)
{
  struct external_info *ei;
  assert (CHECK_FLAG (lsa->flags, OSPF_LSA_SELF));

  switch (lsa->data->type)
    {
      /* Router and Network LSAs are processed differently. */
    case OSPF_ROUTER_LSA:
    case OSPF_NETWORK_LSA: 
      break;
    case OSPF_SUMMARY_LSA:
      ospf_summary_lsa_refresh (lsa);
      break;
    case OSPF_SUMMARY_LSA_ASBR:
      ospf_summary_asbr_lsa_refresh (lsa);
      break;
    case OSPF_AS_EXTERNAL_LSA:
      ei = ospf_external_info_check (lsa);
      if (ei)
	ospf_external_lsa_refresh (lsa, ei, LSA_REFRESH_FORCE);
      else
	ospf_lsa_flush_as (lsa);
      break;
    }
}

void
ospf_refresher_register_lsa (struct ospf *top, struct ospf_lsa *lsa)
{
  u_int16_t index, current_index;
  
  assert (CHECK_FLAG (lsa->flags, OSPF_LSA_SELF));

  if (lsa->refresh_list < 0)
    {
      int delay;

      if (LS_AGE (lsa) == 0 &&
	  ntohl (lsa->data->ls_seqnum) == OSPF_INITIAL_SEQUENCE_NUMBER)
	/* Randomize first update by  OSPF_LS_REFRESH_SHIFT factor */ 
	delay = OSPF_LS_REFRESH_SHIFT + (random () % OSPF_LS_REFRESH_TIME);
      else
	/* Randomize another updates by +-OSPF_LS_REFRESH_JITTER factor */
	delay = OSPF_LS_REFRESH_TIME - LS_AGE (lsa) - OSPF_LS_REFRESH_JITTER
	  + (random () % (2*OSPF_LS_REFRESH_JITTER)); 

      if (delay < 0)
	delay = 0;

      current_index = top->lsa_refresh_queue.index +
	(time (NULL) - top->lsa_refresher_started)/OSPF_LSA_REFRESHER_GRANULARITY;
      
      index = (current_index + delay/OSPF_LSA_REFRESHER_GRANULARITY)
	% (OSPF_LSA_REFRESHER_SLOTS);

      if (IS_DEBUG_OSPF (lsa, LSA_REFRESH))
	zlog_info ("LSA[Refresh]: lsa with age %d added to index %d",
		   LS_AGE (lsa), index);
      if (!top->lsa_refresh_queue.qs[index])
	top->lsa_refresh_queue.qs[index] = list_new ();
      listnode_add (top->lsa_refresh_queue.qs[index], ospf_lsa_lock (lsa));
      lsa->refresh_list = index;
    }
}

void
ospf_refresher_unregister_lsa (struct ospf *top, struct ospf_lsa *lsa)
{
  assert (CHECK_FLAG (lsa->flags, OSPF_LSA_SELF));
  if (lsa->refresh_list >= 0)
    {
      list refresh_list = top->lsa_refresh_queue.qs[lsa->refresh_list];
      listnode_delete (refresh_list, lsa);
      if (!listcount (refresh_list))
	{
	  list_free (refresh_list);
	  top->lsa_refresh_queue.qs[lsa->refresh_list] = NULL;
	}
      ospf_lsa_unlock (lsa);
      lsa->refresh_list = -1;
    }
}

int
ospf_lsa_refresh_walker (struct thread *t)
{
  list refresh_list;
  listnode node;
  struct ospf *top = THREAD_ARG (t);
  int i;
  list lsa_to_refresh = list_new ();

  if (IS_DEBUG_OSPF (lsa, LSA_REFRESH))
    zlog_info ("LSA[Refresh]:ospf_lsa_refresh_walker(): start");

  
  i = top->lsa_refresh_queue.index;
  
  top->lsa_refresh_queue.index =
    (top->lsa_refresh_queue.index +
     (time (NULL) - top->lsa_refresher_started) / OSPF_LSA_REFRESHER_GRANULARITY)
    % OSPF_LSA_REFRESHER_SLOTS;

  if (IS_DEBUG_OSPF (lsa, LSA_REFRESH))
    zlog_info ("LSA[Refresh]: ospf_lsa_refresh_walker(): next index %d",
	       top->lsa_refresh_queue.index);

  for (;i != top->lsa_refresh_queue.index;
       i = (i + 1) % OSPF_LSA_REFRESHER_SLOTS)
    {
      if (IS_DEBUG_OSPF (lsa, LSA_REFRESH))
	zlog_info ("LSA[Refresh]: ospf_lsa_refresh_walker(): refresh index %d", i);

      refresh_list = top->lsa_refresh_queue.qs [i];
      
      top->lsa_refresh_queue.qs [i] = NULL;
      
      if (refresh_list)
	{
	  for (node = listhead (refresh_list); node;)
	    {
	      listnode next;
	      struct ospf_lsa *lsa = getdata (node);
	      next = node->next;
	      
	      if (IS_DEBUG_OSPF (lsa, LSA_REFRESH))
		zlog_info ("LSA[Refresh]: ospf_lsa_refresh_walker(): refresh lsa %p", lsa);
	      
	      list_delete_node (refresh_list, node);
	      ospf_lsa_unlock (lsa);
	      lsa->refresh_list = -1;
	      listnode_add (lsa_to_refresh, lsa);
	      node = next;
	    }
	  list_free (refresh_list);
	}
    }

  top->t_lsa_refresher = thread_add_timer (master, ospf_lsa_refresh_walker,
					   top, top->lsa_refresh_interval);
  top->lsa_refresher_started = time (NULL);

  for (node = listhead (lsa_to_refresh); node; nextnode (node))
    ospf_lsa_refresh (getdata (node));
  
  list_delete (lsa_to_refresh);
  
  if (IS_DEBUG_OSPF (lsa, LSA_REFRESH))
    zlog_info ("LSA[Refresh]: ospf_lsa_refresh_walker(): end");
  
  return 0;
}


/* Show functions */
int
show_lsa_summary (struct ospf_lsa *lsa, void *v, int self)
{
  struct vty *vty = (struct vty *) v;
  struct router_lsa *rl;
  struct summary_lsa *sl;
  struct as_external_lsa *asel;
  struct prefix_ipv4 p;

  if (lsa != NULL)
    /* If self option is set, check LSA self flag. */
    if (self == 0 || IS_LSA_SELF (lsa))
      {
	/* LSA common part show. */
	vty_out (vty, "%-15s ", inet_ntoa (lsa->data->id));
	vty_out (vty, "%-15s %4d 0x%08x 0x%04x",
		 inet_ntoa (lsa->data->adv_router), LS_AGE (lsa),
		 ntohl (lsa->data->ls_seqnum), ntohs (lsa->data->checksum));
	/* LSA specific part show. */
	switch (lsa->data->type)
	  {
	  case OSPF_ROUTER_LSA:
	    rl = (struct router_lsa *) lsa->data;
	    vty_out (vty, " %-d", ntohs (rl->links));
	    break;
	  case OSPF_SUMMARY_LSA:
	    sl = (struct summary_lsa *) lsa->data;

	    p.family = AF_INET;
	    p.prefix = sl->header.id;
	    p.prefixlen = ip_masklen (sl->mask);
	    apply_mask_ipv4 (&p);

	    vty_out (vty, " %s/%d", inet_ntoa (p.prefix), p.prefixlen);
	    break;
	  case OSPF_AS_EXTERNAL_LSA:
	    asel = (struct as_external_lsa *) lsa->data;

	    p.family = AF_INET;
	    p.prefix = asel->header.id;
	    p.prefixlen = ip_masklen (asel->mask);
	    apply_mask_ipv4 (&p);

	    vty_out (vty, " %s %s/%d [0x%x]",
		     IS_EXTERNAL_METRIC (asel->e[0].tos) ? "E2" : "E1",
		     inet_ntoa (p.prefix), p.prefixlen,
		     ntohl (asel->e[0].route_tag));
	    break;
	  case OSPF_NETWORK_LSA:
	  case OSPF_SUMMARY_LSA_ASBR:
	  default:
	    break;
	  }
	vty_out (vty, VTY_NEWLINE);
      }

  return 0;
}


char *show_database_desc[] =
{
  "unknown",
  "Router Link States",
  "Net Link States",
  "Summary Link States",
  "ASBR-Summary Link States",
  "AS External Link States",
#ifdef HAVE_NSSA
  "Group Membership LSA",
  "NSSA-external Link States",
#endif /* HAVE_NSSA */
};

#define SHOW_OSPF_COMMON_HEADER \
  "Link ID         ADV Router      Age  Seq#       CkSum"

char *show_database_header[] =
{
  "",
  "Link ID         ADV Router      Age  Seq#       CkSum  Link count",
  "Link ID         ADV Router      Age  Seq#       CkSum",
  "Link ID         ADV Router      Age  Seq#       CkSum  Route",
  "Link ID         ADV Router      Age  Seq#       CkSum",
  "Link ID         ADV Router      Age  Seq#       CkSum  Route",
#ifdef HAVE_NSSA
  " --- header for Group Member ----",
  "Link ID         ADV Router      Age  Seq#       CkSum  Route",
#endif /* HAVE_NSSA */
};

void
show_ip_ospf_database_header (struct vty *vty, struct ospf_lsa *lsa)
{
  struct router_lsa *rlsa = (struct router_lsa*) lsa->data;

  vty_out (vty, "  LS age: %d%s", LS_AGE (lsa), VTY_NEWLINE);
  vty_out (vty, "  Options: %d%s", lsa->data->options, VTY_NEWLINE);

  if (lsa->data->type == OSPF_ROUTER_LSA)
    {
      vty_out (vty, "  Flags: 0x%x" , rlsa->flags);

      if (rlsa->flags)
	vty_out (vty, " :%s%s%s%s",
		 IS_ROUTER_LSA_BORDER (rlsa) ? " ABR" : "",
		 IS_ROUTER_LSA_EXTERNAL (rlsa) ? " ASBR" : "",
		 IS_ROUTER_LSA_VIRTUAL (rlsa) ? " VL-endpoint" : "",
		 IS_ROUTER_LSA_SHORTCUT (rlsa) ? " Shortcut" : "");

      vty_out (vty, "%s", VTY_NEWLINE);
    }
  vty_out (vty, "  LS Type: %s%s",
           LOOKUP (ospf_lsa_type_msg, lsa->data->type), VTY_NEWLINE);
  vty_out (vty, "  Link State ID: %s %s%s", inet_ntoa (lsa->data->id),
           LOOKUP (ospf_link_state_id_type_msg, lsa->data->type), VTY_NEWLINE);
  vty_out (vty, "  Advertising Router: %s%s",
           inet_ntoa (lsa->data->adv_router), VTY_NEWLINE);
  vty_out (vty, "  LS Seq Number: %08x%s", ntohl (lsa->data->ls_seqnum),
           VTY_NEWLINE);
  vty_out (vty, "  Checksum: 0x%04x%s", ntohs (lsa->data->checksum),
           VTY_NEWLINE);
  vty_out (vty, "  Length: %d%s", ntohs (lsa->data->length), VTY_NEWLINE);
}

char *link_type_desc[] =
{
  "(null)",
  "another Router (point-to-point)",
  "a Transit Network",
  "Stub Network",
  "a Virtual Link",
};

char *link_id_desc[] =
{
  "(null)",
  "Neighboring Router ID",
  "Designated Router address",
  "Network/subnet number",
  "Neighboring Router ID",
};

char *link_data_desc[] =
{
  "(null)",
  "Router Interface address",
  "Router Interface address",
  "Network Mask",
  "Router Interface address",
};

/* Show router-LSA each Link information. */
void
show_ip_ospf_database_router_links (struct vty *vty,
                                    struct router_lsa *rl)
{
  int len, i, type;

  len = ntohs (rl->header.length) - 4;
  for (i = 0; i < ntohs (rl->links) && len > 0; len -= 12, i++)
    {
      type = rl->link[i].type;

      vty_out (vty, "    Link connected to: %s%s",
	       link_type_desc[type], VTY_NEWLINE);
      vty_out (vty, "     (Link ID) %s: %s%s", link_id_desc[type],
	       inet_ntoa (rl->link[i].link_id), VTY_NEWLINE);
      vty_out (vty, "     (Link Data) %s: %s%s", link_data_desc[type],
	       inet_ntoa (rl->link[i].link_data), VTY_NEWLINE);
      vty_out (vty, "      Number of TOS metrics: 0%s", VTY_NEWLINE);
      vty_out (vty, "       TOS 0 Metric: %d%s",
	       ntohs (rl->link[i].metric), VTY_NEWLINE);
      vty_out (vty, "%s", VTY_NEWLINE);
    }
}

/* Show router-LSA detail information. */
int
show_router_lsa_detail (struct vty *vty, struct ospf_lsa *lsa)
{
  if (lsa != NULL)
    {
      struct router_lsa *rl = (struct router_lsa *) lsa->data;

      show_ip_ospf_database_header (vty, lsa);
          
      vty_out (vty, "   Number of Links: %d%s%s", ntohs (rl->links),
	       VTY_NEWLINE, VTY_NEWLINE);

      show_ip_ospf_database_router_links (vty, rl);
    }

  return 0;
}

/* Show network-LSA detail information. */
int
show_network_lsa_detail (struct vty *vty, struct ospf_lsa *lsa)
{
  int length, i;

  if (lsa != NULL)
    {
      struct network_lsa *nl = (struct network_lsa *) lsa->data;

      show_ip_ospf_database_header (vty, lsa);

      vty_out (vty, "  Network Mask: /%d%s",
	       ip_masklen (nl->mask), VTY_NEWLINE);

      length = ntohs (lsa->data->length) - OSPF_LSA_HEADER_SIZE - 4;

      for (i = 0; length > 0; i++, length -= 4)
	vty_out (vty, "        Attached Router: %s%s",
		 inet_ntoa (nl->routers[i]), VTY_NEWLINE);

      vty_out (vty, "%s", VTY_NEWLINE);
    }

  return 0;
}

/* Show summary-LSA detail information. */
int
show_summary_lsa_detail (struct vty *vty, struct ospf_lsa *lsa)
{
  if (lsa != NULL)
    {
      struct summary_lsa *sl = (struct summary_lsa *) lsa->data;

      show_ip_ospf_database_header (vty, lsa);

      vty_out (vty, "  Network Mask: /%d%s", ip_masklen (sl->mask),
	       VTY_NEWLINE);
      vty_out (vty, "        TOS: 0  Metric: %d%s", GET_METRIC (sl->metric),
	       VTY_NEWLINE);
    }

  return 0;
}

/* Show summary-ASBR-LSA detail information. */
int
show_summary_asbr_lsa_detail (struct vty *vty, struct ospf_lsa *lsa)
{
  if (lsa != NULL)
    {
      struct summary_lsa *sl = (struct summary_lsa *) lsa->data;

      show_ip_ospf_database_header (vty, lsa);

      vty_out (vty, "  Network Mask: /%d%s",
	       ip_masklen (sl->mask), VTY_NEWLINE);
      vty_out (vty, "        TOS: 0  Metric: %d%s", GET_METRIC (sl->metric),
	       VTY_NEWLINE);
    }

  return 0;
}

/* Show AS-external-LSA detail information. */
int
show_as_external_lsa_detail (struct vty *vty, struct ospf_lsa *lsa)
{
  if (lsa != NULL)
    {
      struct as_external_lsa *al = (struct as_external_lsa *) lsa->data;

      show_ip_ospf_database_header (vty, lsa);

      vty_out (vty, "  Network Mask: /%d%s",
	       ip_masklen (al->mask), VTY_NEWLINE);
      vty_out (vty, "        Metric Type: %s%s",
	       IS_EXTERNAL_METRIC (al->e[0].tos) ?
	       "2 (Larger than any link state path)" : "1", VTY_NEWLINE);
      vty_out (vty, "        TOS: 0%s", VTY_NEWLINE);
      vty_out (vty, "        Metric: %d%s",
	       GET_METRIC (al->e[0].metric), VTY_NEWLINE);
      vty_out (vty, "        Forward Address: %s%s",
	       inet_ntoa (al->e[0].fwd_addr), VTY_NEWLINE);

      vty_out (vty, "        External Route Tag: %u%s%s",
	       ntohl (al->e[0].route_tag), VTY_NEWLINE, VTY_NEWLINE);
    }

  return 0;
}

#ifdef HAVE_NSSA
int
show_as_external_lsa_stdvty (struct ospf_lsa *lsa)
{
  struct as_external_lsa *al = (struct as_external_lsa *) lsa->data;

  /* show_ip_ospf_database_header (vty, lsa); */

  zlog_info( "  Network Mask: /%d%s",
	     ip_masklen (al->mask), "\n");
  zlog_info( "        Metric Type: %s%s",
	     IS_EXTERNAL_METRIC (al->e[0].tos) ?
	     "2 (Larger than any link state path)" : "1", "\n");
  zlog_info( "        TOS: 0%s", "\n");
  zlog_info( "        Metric: %d%s",
	     GET_METRIC (al->e[0].metric), "\n");
  zlog_info( "        Forward Address: %s%s",
	     inet_ntoa (al->e[0].fwd_addr), "\n");

  zlog_info( "        External Route Tag: %u%s%s",
	     ntohl (al->e[0].route_tag), "\n", "\n");

  return 0;
}

/* Show AS-NSSA-LSA detail information. */
int
show_as_nssa_lsa_detail (struct vty *vty, struct ospf_lsa *lsa)
{
  if (lsa != NULL)
    {
      struct as_external_lsa *al = (struct as_external_lsa *) lsa->data;

      show_ip_ospf_database_header (vty, lsa);

      vty_out (vty, "  Network Mask: /%d%s",
	       ip_masklen (al->mask), VTY_NEWLINE);
      vty_out (vty, "        Metric Type: %s%s",
	       IS_EXTERNAL_METRIC (al->e[0].tos) ?
	       "2 (Larger than any link state path)" : "1", VTY_NEWLINE);
      vty_out (vty, "        TOS: 0%s", VTY_NEWLINE);
      vty_out (vty, "        Metric: %d%s",
	       GET_METRIC (al->e[0].metric), VTY_NEWLINE);
      vty_out (vty, "        NSSA: Forward Address: %s%s",
	       inet_ntoa (al->e[0].fwd_addr), VTY_NEWLINE);

      vty_out (vty, "        External Route Tag: %u%s%s",
	       ntohl (al->e[0].route_tag), VTY_NEWLINE, VTY_NEWLINE);
    }

  return 0;
}

int dummy (struct vty *vty, struct ospf_lsa *lsa)
{
  return 0;
}
#endif /* HAVE_NSSA */

int (*show_function[])(struct vty *, struct ospf_lsa *) =
{
  NULL,
  show_router_lsa_detail,
  show_network_lsa_detail,
  show_summary_lsa_detail,
  show_summary_asbr_lsa_detail,
  show_as_external_lsa_detail,
#ifdef HAVE_NSSA
  dummy,
  show_as_nssa_lsa_detail,  /* almost same as external */
#endif /* HAVE_NSSA */
};

#ifdef HAVE_NSSA
int sh_fct (int type, struct vty *vty, struct ospf_lsa *lsa)
{
  if (type < OSPF_MIN_LSA || type >= OSPF_MAX_LSA)
    return 0;

  show_function[type] (vty, lsa);
  return 0;
}
#endif /* HAVE_NSSA */

void
show_lsa_prefix_set (struct vty *vty, struct prefix_ls *lp, struct in_addr *id,
		     struct in_addr *adv_router)
{
  memset (lp, 0, sizeof (struct prefix_ls));
  lp->family = 0;
  if (id == NULL)
    lp->prefixlen = 0;
  else if (adv_router == NULL)
    {
      lp->prefixlen = 32;
      lp->id = *id;
    }
  else
    {
      lp->prefixlen = 64;
      lp->id = *id;
      lp->adv_router = *adv_router;
    }
}

void
show_lsa_detail_proc (struct vty *vty, struct route_table *rt,
		      struct in_addr *id, struct in_addr *adv_router)
{
  struct prefix_ls lp;
  struct route_node *rn, *start;
  struct ospf_lsa *lsa;

  show_lsa_prefix_set (vty, &lp, id, adv_router);
  start = route_node_get (rt, (struct prefix *) &lp);
  if (start)
    {
      route_lock_node (start);
      for (rn = start; rn; rn = route_next_until (rn, start))
	if ((lsa = rn->info))
	  {
#ifdef HAVE_NSSA
	    /* Stay away from any Local Translated Type-7 LSAs */
	    if (CHECK_FLAG (lsa->flags, OSPF_LSA_LOCAL_XLT))
	      continue;
	    sh_fct (lsa->data->type, vty, lsa);
#else
	    show_function[lsa->data->type] (vty, lsa);
#endif /* HAVE_NSSA */
	  }
      route_unlock_node (start);
    }
}

/* Show detail LSA information
   -- if id is NULL then show all LSAs. */
void
show_lsa_detail (struct vty *vty, int type,
		 struct in_addr *id, struct in_addr *adv_router)
{
  listnode node;
  
  /* Show Type 1-4 LSAs. */
  if (type != OSPF_AS_EXTERNAL_LSA)
    {
      for (node = listhead (ospf_top->areas); node; nextnode (node))
	{
	  struct ospf_area *area = node->data;
	  vty_out (vty, "%s                %s (Area %s)%s%s",
		   VTY_NEWLINE, show_database_desc[type],
		   ait_ntoa (area->area_id, area->external_routing), VTY_NEWLINE, VTY_NEWLINE);
	  show_lsa_detail_proc (vty, AREA_LSDB (area, type), id, adv_router);
	}
    }
  /* Show AS-external-LSAs. */
  else
    {
      vty_out (vty, "                %s %s%s",
	       show_database_desc[OSPF_AS_EXTERNAL_LSA],
	       VTY_NEWLINE, VTY_NEWLINE);
      show_lsa_detail_proc (vty, EXTERNAL_LSDB (ospf_top), id, adv_router);
    }
}

void
show_lsa_detail_adv_router_proc (struct vty *vty, struct route_table *rt,
				 struct in_addr *adv_router)
{
  struct route_node *rn;
  struct ospf_lsa *lsa;

  for (rn = route_top (rt); rn; rn = route_next (rn))
    if ((lsa = rn->info))
      if (IPV4_ADDR_SAME (adv_router, &lsa->data->adv_router))
	{
#ifdef HAVE_NSSA
	  if (CHECK_FLAG (lsa->flags, OSPF_LSA_LOCAL_XLT))
	    continue;
	  sh_fct (lsa->data->type, vty, lsa);
#else
	  show_function[lsa->data->type] (vty, lsa);
#endif /* HAVE_NSSA */
	}
}

/* Show detail LSA information. */
void
show_lsa_detail_adv_router (struct vty *vty, int type,
			    struct in_addr *adv_router)
{
  listnode node;
  
  if (type != OSPF_AS_EXTERNAL_LSA)
    {
      for (node = listhead (ospf_top->areas); node; nextnode (node))
	{
	  struct ospf_area *area = node->data;
	  vty_out (vty, "%s                %s (Area %s)%s%s",
		   VTY_NEWLINE, show_database_desc[type],
		   ait_ntoa (area->area_id, area->external_routing), VTY_NEWLINE, VTY_NEWLINE);
	  show_lsa_detail_adv_router_proc (vty, AREA_LSDB (area, type),
					  adv_router);
	}
    }
  else
    {
      vty_out (vty, "                %s %s%s",
	       show_database_desc[OSPF_AS_EXTERNAL_LSA],
	       VTY_NEWLINE, VTY_NEWLINE);
      show_lsa_detail_adv_router_proc (vty, EXTERNAL_LSDB (ospf_top),
				       adv_router);
    }
  
}

void
show_ip_ospf_database_summary (struct vty *vty, int self)
{
  listnode node;
  int type;

  /* Show self-originated Type1-4 LSAs for each areas. */
  for (node = listhead (ospf_top->areas); node; nextnode (node))
    {
      struct ospf_area *area = node->data;

      for (type = OSPF_ROUTER_LSA; type <= OSPF_SUMMARY_LSA_ASBR; type++)
	if (ospf_lsdb_count_self (area->lsdb, type) > 0 ||
	    (!self && ospf_lsdb_count (area->lsdb, type) > 0))
	  {
	    vty_out (vty, "                %s (Area %s)%s%s",
		     show_database_desc[type], ait_ntoa (area->area_id, area->external_routing),
		     VTY_NEWLINE, VTY_NEWLINE);
	    vty_out (vty, "%s%s", show_database_header[type], VTY_NEWLINE);

	    foreach_lsa (AREA_LSDB (area, type), vty, self, show_lsa_summary);

	    vty_out (vty, "%s", VTY_NEWLINE);
	  }
    }

  /* Show self-originated AS-external-LSAs. */
  if (ospf_lsdb_count_self (ospf_top->lsdb, OSPF_AS_EXTERNAL_LSA) ||
      (!self && ospf_lsdb_count (ospf_top->lsdb, OSPF_AS_EXTERNAL_LSA)))
    {
      vty_out (vty, "                %s%s%s",
	       show_database_desc[OSPF_AS_EXTERNAL_LSA],
	       VTY_NEWLINE, VTY_NEWLINE);
      vty_out (vty, "%s%s", show_database_header[OSPF_AS_EXTERNAL_LSA],
	       VTY_NEWLINE);
      foreach_lsa (EXTERNAL_LSDB (ospf_top), vty, self, show_lsa_summary);
      vty_out (vty, "%s", VTY_NEWLINE);
    }

  vty_out (vty, "%s", VTY_NEWLINE);
}

void
show_ip_ospf_database_maxage (struct vty *vty)
{
  listnode node;
  struct ospf_lsa *lsa;

  vty_out (vty, "%s                MaxAge Link States:%s%s",
           VTY_NEWLINE, VTY_NEWLINE, VTY_NEWLINE);

  for (node = listhead (ospf_top->maxage_lsa); node; nextnode (node))
    if ((lsa = node->data) != NULL)
      {
	vty_out (vty, "Link type: %d%s", lsa->data->type, VTY_NEWLINE);
	vty_out (vty, "Link State ID: %s%s",
		 inet_ntoa (lsa->data->id), VTY_NEWLINE);
	vty_out (vty, "Advertising Router: %s%s",
		 inet_ntoa (lsa->data->adv_router), VTY_NEWLINE);
	vty_out (vty, "LSA lock count: %d%s", lsa->lock, VTY_NEWLINE);
	vty_out (vty, "%s", VTY_NEWLINE);
      }
}

DEFUN (show_ip_ospf_database,
       show_ip_ospf_database_cmd,
       "show ip ospf database",
       SHOW_STR
       IP_STR
       "OSPF information\n"
       "Database summary\n")
{
  int type, ret;
  struct in_addr id, adv_router;

  if (ospf_top == NULL)
    return CMD_SUCCESS;

  vty_out (vty, "%s       OSPF Router with ID (%s)%s%s", VTY_NEWLINE,
           inet_ntoa (ospf_top->router_id), VTY_NEWLINE, VTY_NEWLINE);

  /* Show all LSA. */
  if (argc == 0)
    {
      show_ip_ospf_database_summary (vty, 0);
      return CMD_SUCCESS;
    }

  /* Set database type to show. */
  if (strncmp (argv[0], "r", 1) == 0)
    type = OSPF_ROUTER_LSA;
  else if (strncmp (argv[0], "ne", 2) == 0)
    type = OSPF_NETWORK_LSA;
#ifdef HAVE_NSSA
  else if (strncmp (argv[0], "ns", 2) == 0)
    type = OSPF_AS_NSSA_LSA;
#endif /* HAVE_NSSA */
  else if (strncmp (argv[0], "su", 2) == 0)
    type = OSPF_SUMMARY_LSA;
  else if (strncmp (argv[0], "a", 1) == 0)
    type = OSPF_SUMMARY_LSA_ASBR;
  else if (strncmp (argv[0], "e", 1) == 0)
    type = OSPF_AS_EXTERNAL_LSA;
  else if (strncmp (argv[0], "se", 2) == 0)
    {
      show_ip_ospf_database_summary (vty, 1);
      return CMD_SUCCESS;
    }
  else if (strncmp (argv[0], "m", 1) == 0)
    {
      show_ip_ospf_database_maxage (vty);
      return CMD_SUCCESS;
    }
  else
    return CMD_WARNING;

  /* `show ip ospf database LSA'. */
  if (argc == 1)
    show_lsa_detail (vty, type, NULL, NULL);
  else if (argc >= 2)
    {
      ret = inet_aton (argv[1], &id);
      if (!ret)
	return CMD_WARNING;
      
      /* `show ip ospf database LSA ID'. */
      if (argc == 2)
	show_lsa_detail (vty, type, &id, NULL);
      /* `show ip ospf database LSA ID adv-router ADV_ROUTER'. */
      else if (argc == 3)
	{
	  if (strncmp (argv[2], "s", 1) == 0)
	    adv_router = ospf_top->router_id;
	  else
	    {
	      ret = inet_aton (argv[2], &adv_router);
	      if (!ret)
		return CMD_WARNING;
	    }
	  show_lsa_detail (vty, type, &id, &adv_router);
	}
    }

  return CMD_SUCCESS;
}


#ifdef HAVE_NSSA
ALIAS (show_ip_ospf_database,
       show_ip_ospf_database_type_cmd,
	   "show ip ospf database (nssa-external|asbr-summary|external|max-age|network|router|self-originate|summary)",
	   SHOW_STR
       IP_STR
       "OSPF information\n"
       "Database summary\n"
       "ASBR summary link states\n"
       "External link states\n"
       "LSAs in MaxAge list\n"
       "Network link states\n"
       "Router link states\n"
       "Self-originated link states\n"
       "Network summary link states\n")
#else /* ! HAVE_NSSA */
ALIAS (show_ip_ospf_database,
       show_ip_ospf_database_type_cmd,
	   "show ip ospf database (asbr-summary|external|max-age|network|router|self-originate|summary)",
	   SHOW_STR
       IP_STR
       "OSPF information\n"
       "Database summary\n"
       "ASBR summary link states\n"
       "External link states\n"
       "LSAs in MaxAge list\n"
       "Network link states\n"
       "Router link states\n"
       "Self-originated link states\n"
       "Network summary link states\n")
#endif /* HAVE_NSSA */

#ifdef HAVE_NSSA
ALIAS (show_ip_ospf_database,
       show_ip_ospf_database_type_id_cmd,
       "show ip ospf database (nssa-external|asbr-summary|external|network|router|summary) A.B.C.D",
       SHOW_STR
       IP_STR
       "OSPF information\n"
       "Database summary\n"
       "ASBR summary link states\n"
       "External link states\n"
       "Network link states\n"
       "Router link states\n"
       "Network summary link states\n"
       "Link State ID (as an IP address)\n")
#else /* ! HAVE_NSSA */
ALIAS (show_ip_ospf_database,
       show_ip_ospf_database_type_id_cmd,
       "show ip ospf database (asbr-summary|external|network|router|summary) A.B.C.D",
       SHOW_STR
       IP_STR
       "OSPF information\n"
       "Database summary\n"
       "ASBR summary link states\n"
       "External link states\n"
       "Network link states\n"
       "Router link states\n"
       "Network summary link states\n"
       "Link State ID (as an IP address)\n")
#endif /* HAVE_NSSA */

#ifdef HAVE_NSSA
ALIAS (show_ip_ospf_database,
       show_ip_ospf_database_type_id_adv_router_cmd,
       "show ip ospf database (nssa-external|asbr-summary|external|network|router|summary) A.B.C.D adv-router A.B.C.D",
       SHOW_STR
       IP_STR
       "OSPF information\n"
       "Database summary\n"
       "ASBR summary link states\n"
       "External link states\n"
       "Network link states\n"
       "Router link states\n"
       "Network summary link states\n"
       "Link State ID (as an IP address)\n"
       "Advertising Router link states\n"
       "Advertising Router (as an IP address)\n")
#else /* ! HAVE_NSSA */
ALIAS (show_ip_ospf_database,
       show_ip_ospf_database_type_id_adv_router_cmd,
       "show ip ospf database (asbr-summary|external|network|router|summary) A.B.C.D adv-router A.B.C.D",
       SHOW_STR
       IP_STR
       "OSPF information\n"
       "Database summary\n"
       "ASBR summary link states\n"
       "External link states\n"
       "Network link states\n"
       "Router link states\n"
       "Network summary link states\n"
       "Link State ID (as an IP address)\n"
       "Advertising Router link states\n"
       "Advertising Router (as an IP address)\n")
#endif /* HAVE_NSSA */

#ifdef HAVE_NSSA
ALIAS (show_ip_ospf_database,
       show_ip_ospf_database_type_id_self_cmd,
       "show ip ospf database (nssa-external|asbr-summary|external|network|router|summary) A.B.C.D (self-originate|)",
       SHOW_STR
       IP_STR
       "OSPF information\n"
       "Database summary\n"
       "ASBR summary link states\n"
       "External link states\n"
       "Network link states\n"
       "Router link states\n"
       "Network summary link states\n"
       "Link State ID (as an IP address)\n"
       "Self-originated link states\n")
#else /* ! HAVE_NSSA */
ALIAS (show_ip_ospf_database,
       show_ip_ospf_database_type_id_self_cmd,
       "show ip ospf database (asbr-summary|external|network|router|summary) A.B.C.D (self-originate|)",
       SHOW_STR
       IP_STR
       "OSPF information\n"
       "Database summary\n"
       "ASBR summary link states\n"
       "External link states\n"
       "Network link states\n"
       "Router link states\n"
       "Network summary link states\n"
       "Link State ID (as an IP address)\n"
       "Self-originated link states\n")
#endif /* HAVE_NSSA */

#ifdef HAVE_NSSA
DEFUN (show_ip_ospf_database_type_adv_router,
       show_ip_ospf_database_type_adv_router_cmd,
       "show ip ospf database (nssa-external|asbr-summary|external|network|router|summary) adv-router A.B.C.D",
       SHOW_STR
       IP_STR
       "OSPF information\n"
       "Database summary\n"
       "ASBR summary link states\n"
       "External link states\n"
       "Network link states\n"
       "Router link states\n"
       "Network summary link states\n"
       "Advertising Router link states\n"
       "Advertising Router (as an IP address)\n")
#else /* ! HAVE_NSSA */
DEFUN (show_ip_ospf_database_type_adv_router,
       show_ip_ospf_database_type_adv_router_cmd,
       "show ip ospf database (asbr-summary|external|network|router|summary) adv-router A.B.C.D",
       SHOW_STR
       IP_STR
       "OSPF information\n"
       "Database summary\n"
       "ASBR summary link states\n"
       "External link states\n"
       "Network link states\n"
       "Router link states\n"
       "Network summary link states\n"
       "Advertising Router link states\n"
       "Advertising Router (as an IP address)\n")
#endif /* HAVE_NSSA */

{
  int type, ret;
  struct in_addr adv_router;

  if (ospf_top == NULL)
    return CMD_SUCCESS;

  vty_out (vty, "%s       OSPF Router with ID (%s)%s%s", VTY_NEWLINE,
           inet_ntoa (ospf_top->router_id), VTY_NEWLINE, VTY_NEWLINE);

  if (argc != 2)
    return CMD_WARNING;

  /* Set database type to show. */
  if (strncmp (argv[0], "r", 1) == 0)
    type = OSPF_ROUTER_LSA;
  else if (strncmp (argv[0], "ne", 2) == 0)
    type = OSPF_NETWORK_LSA;
#ifdef HAVE_NSSA
  else if (strncmp (argv[0], "ns", 2) == 0)
    type = OSPF_AS_NSSA_LSA;
#endif /* HAVE_NSSA */
  else if (strncmp (argv[0], "s", 1) == 0)
    type = OSPF_SUMMARY_LSA;
  else if (strncmp (argv[0], "a", 1) == 0)
    type = OSPF_SUMMARY_LSA_ASBR;
  else if (strncmp (argv[0], "e", 1) == 0)
    type = OSPF_AS_EXTERNAL_LSA;
  else
    return CMD_WARNING;

  /* `show ip ospf database LSA adv-router ADV_ROUTER'. */
  if (strncmp (argv[1], "s", 1) == 0)
    adv_router = ospf_top->router_id;
  else
    {
      ret = inet_aton (argv[1], &adv_router);
      if (!ret)
	return CMD_WARNING;
    }

  show_lsa_detail_adv_router (vty, type, &adv_router);

  return CMD_SUCCESS;
}
#ifdef HAVE_NSSA
ALIAS (show_ip_ospf_database_type_adv_router,
       show_ip_ospf_database_type_self_cmd,
       "show ip ospf database (nssa-external|asbr-summary|external|network|router|summary) (self-originate|)",
       SHOW_STR
       IP_STR
       "OSPF information\n"
       "Database summary\n"
       "ASBR summary link states\n"
       "External link states\n"
       "Network link states\n"
       "Router link states\n"
       "Network summary link states\n"
       "Self-originated link states\n")
#else /* ! HAVE_NSSA */
ALIAS (show_ip_ospf_database_type_adv_router,
       show_ip_ospf_database_type_self_cmd,
       "show ip ospf database (asbr-summary|external|network|router|summary) (self-originate|)",
       SHOW_STR
       IP_STR
       "OSPF information\n"
       "Database summary\n"
       "ASBR summary link states\n"
       "External link states\n"
       "Network link states\n"
       "Router link states\n"
       "Network summary link states\n"
       "Self-originated link states\n")
#endif /* HAVE_NSSA */

/* Install LSA related commands. */
void
ospf_lsa_init ()
{
  install_element (VIEW_NODE, &show_ip_ospf_database_type_cmd);
  install_element (VIEW_NODE, &show_ip_ospf_database_type_id_cmd);
  install_element (VIEW_NODE, &show_ip_ospf_database_type_id_adv_router_cmd);
  install_element (VIEW_NODE, &show_ip_ospf_database_type_adv_router_cmd);
  install_element (VIEW_NODE, &show_ip_ospf_database_type_id_self_cmd);
  install_element (VIEW_NODE, &show_ip_ospf_database_type_self_cmd);
  install_element (VIEW_NODE, &show_ip_ospf_database_cmd);
  install_element (ENABLE_NODE, &show_ip_ospf_database_type_cmd);
  install_element (ENABLE_NODE, &show_ip_ospf_database_type_id_cmd);
  install_element (ENABLE_NODE, &show_ip_ospf_database_type_id_adv_router_cmd);
  install_element (ENABLE_NODE, &show_ip_ospf_database_type_adv_router_cmd);
  install_element (ENABLE_NODE, &show_ip_ospf_database_type_id_self_cmd);
  install_element (ENABLE_NODE, &show_ip_ospf_database_type_self_cmd);
  install_element (ENABLE_NODE, &show_ip_ospf_database_cmd);
}
