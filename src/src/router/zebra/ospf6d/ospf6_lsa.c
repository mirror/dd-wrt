/*
 * LSA function
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

/* Include other stuffs */
#include "version.h"
#include "log.h"
#include "getopt.h"
#include "linklist.h"
#include "thread.h"
#include "command.h"
#include "memory.h"
#include "sockunion.h"
#include "if.h"
#include "prefix.h"
#include "stream.h"
#include "thread.h"
#include "filter.h"
#include "zclient.h"
#include "table.h"
#include "plist.h"

#include "ospf6_proto.h"
#include "ospf6_prefix.h"
#include "ospf6_lsa.h"
#include "ospf6_lsdb.h"
#include "ospf6_message.h"
#include "ospf6_dump.h"

#include "ospf6_top.h"
#include "ospf6_area.h"
#include "ospf6_interface.h"
#include "ospf6_neighbor.h"
#include "ospf6_redistribute.h"
#include "ospf6_ism.h"
#include "ospf6_nsm.h"
#include "ospf6_dbex.h"

#define HEADER_DEPENDENCY
#include "ospf6d.h"
#undef HEADER_DEPENDENCY

char *ospf6_lsa_type_strings[] =
{
  "None", "Router", "Network", "InterPrefix", "InterRouter",
  "ASExternal", "GroupMembership", "Type7", "Link", "IntraPrefix",
  NULL
};

char *ospf6_router_lsd_type_strings[] =
{ "None", "PointToPoint", "Transit", "Stub", "Virtual", NULL };

char *
ospf6_lsa_print_id (struct ospf6_lsa_header *lsa_header, char *buf, int size)
{
  char adv_router[64];
  inet_ntop (AF_INET, &lsa_header->advrtr, adv_router, sizeof (adv_router));
  snprintf (buf, size, "%s[id:%lu %s]",
            ospf6_lsa_type_string (lsa_header->type),
            (unsigned long) ntohl (lsa_header->ls_id), adv_router);
  return buf;
}

/* test LSAs identity */
int
ospf6_lsa_issame (struct ospf6_lsa_header *lsh1,
                  struct ospf6_lsa_header *lsh2)
{
  assert (lsh1 && lsh2);

  if (lsh1->advrtr != lsh2->advrtr)
    return 0;

  if (lsh1->ls_id != lsh2->ls_id)
    return 0;

  if (lsh1->type != lsh2->type)
    return 0;

  return 1;
}

/* RFC2328: Section 13.2 */
int
ospf6_lsa_differ (struct ospf6_lsa *lsa1,
                  struct ospf6_lsa *lsa2)
{
  int diff;
  struct ospf6_lsa_header *lsh1, *lsh2;

  ospf6_lsa_age_current (lsa1);
  ospf6_lsa_age_current (lsa2);
  lsh1 = (struct ospf6_lsa_header *) lsa1->lsa_hdr;
  lsh2 = (struct ospf6_lsa_header *) lsa2->lsa_hdr;

  if (! ospf6_lsa_issame (lsh1, lsh2))
    return 1;

  /* check Options field */
  /* xxx */

  if (ntohs (lsh1->age) == MAXAGE && ntohs (lsh2->age) != MAXAGE)
    return 1;
  if (ntohs (lsh1->age) != MAXAGE && ntohs (lsh2->age) == MAXAGE)
    return 1;

  /* compare body */
  if (ntohs (lsh1->length) != ntohs (lsh2->length))
    return 1;

  diff = memcmp (lsh1 + 1, lsh2 + 1,
                 ntohs (lsh1->length) - sizeof (struct ospf6_lsa_header));

  return diff;
}

int
ospf6_lsa_match (u_int16_t type, u_int32_t ls_id, u_int32_t advrtr,
                 struct ospf6_lsa_header *lsh)
{
  if (lsh->advrtr != advrtr)
    return 0;

  if (lsh->ls_id != ls_id)
    return 0;

  if (lsh->type != type)
    return 0;

  return 1;
}

/* ospf6 age functions */
/* calculate birth and set expire timer */
static void
ospf6_lsa_age_set (struct ospf6_lsa *lsa)
{
  struct timeval now;

  assert (lsa && lsa->lsa_hdr);

  if (gettimeofday (&now, (struct timezone *)NULL) < 0)
    zlog_warn ("Lsa: gettimeofday failed, may fail LSA AGEs: %s",
               strerror (errno));

  lsa->birth = now.tv_sec - ntohs (lsa->lsa_hdr->lsh_age);
  lsa->expire = thread_add_timer (master, ospf6_lsa_expire, lsa,
                                  lsa->birth + MAXAGE - now.tv_sec);
  return;
}

/* this function calculates current age from its birth,
   then update age field of LSA header. return value is current age */
u_int16_t
ospf6_lsa_age_current (struct ospf6_lsa *lsa)
{
  struct timeval now;
  u_int32_t ulage;
  u_int16_t age;
  struct ospf6_lsa_header *lsa_header;

  assert (lsa);
  lsa_header = (struct ospf6_lsa_header *) lsa->lsa_hdr;
  assert (lsa_header);

  /* current time */
  if (gettimeofday (&now, (struct timezone *)NULL) < 0)
    zlog_warn ("Lsa: gettimeofday failed, may fail ages: %s",
               strerror (errno));

  /* calculate age */
  ulage = now.tv_sec - lsa->birth;

  /* if over MAXAGE, set to it */
  if (ulage > MAXAGE)
    age = MAXAGE;
  else
    age = ulage;

  lsa_header->age = htons (age);
  return age;
}

/* returns 1 if LSA's age is MAXAGE. else returns 0 */
int
ospf6_lsa_is_maxage (struct ospf6_lsa *lsa)
{
  if (ospf6_lsa_age_current (lsa) == MAXAGE)
    return 1;
  return 0;
}

/* update age field of LSA header with adding InfTransDelay */
void
ospf6_lsa_age_update_to_send (struct ospf6_lsa *lsa, u_int32_t transdelay)
{
  unsigned short age;
  struct ospf6_lsa_header *lsa_header;

  lsa_header = (struct ospf6_lsa_header *) lsa->lsa_hdr;
  age = ospf6_lsa_age_current (lsa) + transdelay;
  if (age > MAXAGE)
    age = MAXAGE;
  lsa_header->age = htons (age);
  return;
}

void
ospf6_lsa_premature_aging (struct ospf6_lsa *lsa)
{
  /* log */
  if (IS_OSPF6_DUMP_LSA)
    zlog_info ("Lsa: Premature aging: %s", lsa->str);

  if (lsa->expire)
    thread_cancel (lsa->expire);
  lsa->expire = (struct thread *) NULL;
  if (lsa->refresh)
    thread_cancel (lsa->refresh);
  lsa->refresh = (struct thread *) NULL;

  lsa->birth = 0;
  thread_execute (master, ospf6_lsa_expire, lsa, 0);
}

/* check which is more recent. if a is more recent, return -1;
   if the same, return 0; otherwise(b is more recent), return 1 */
int
ospf6_lsa_check_recent (struct ospf6_lsa *a, struct ospf6_lsa *b)
{
  signed long seqnuma, seqnumb;
  u_int16_t cksuma, cksumb;
  u_int16_t agea, ageb;

  assert (a && a->lsa_hdr);
  assert (b && b->lsa_hdr);
  assert (ospf6_lsa_issame ((struct ospf6_lsa_header *) a->lsa_hdr,
                            (struct ospf6_lsa_header *) b->lsa_hdr));

  seqnuma = ((signed long) ntohl (a->lsa_hdr->lsh_seqnum))
             - (signed long)INITIAL_SEQUENCE_NUMBER;
  seqnumb = ((signed long) ntohl (b->lsa_hdr->lsh_seqnum))
             - (signed long)INITIAL_SEQUENCE_NUMBER;

  /* compare by sequence number */
    /* xxx, care about LS sequence number wrapping */
  recent_reason = "seqnum";
  if (IS_OSPF6_DUMP_LSA && seqnuma != seqnumb)
    zlog_info ("LSA: recent: %s:[seqnum %d] %s:[seqnum %d]",
               a->str, ntohl(a->header->seqnum),
               b->str, ntohl(b->header->seqnum));
  if (seqnuma > seqnumb)
    return -1;
  else if (seqnuma < seqnumb)
    return 1;

  /* Checksum */
  cksuma = ntohs (a->header->checksum);
  cksumb = ntohs (b->header->checksum);
  if (cksuma > cksumb)
    return -1;
  if (cksuma < cksumb)
    return 0;

  /* Age check */
  agea = ospf6_lsa_age_current (a);
  ageb = ospf6_lsa_age_current (b);

    /* MaxAge check */
  recent_reason = "max age";
  if (IS_OSPF6_DUMP_LSA &&
      ((agea == OSPF6_LSA_MAXAGE && ageb != OSPF6_LSA_MAXAGE) ||
       (agea != OSPF6_LSA_MAXAGE && ageb == OSPF6_LSA_MAXAGE)))
    zlog_info ("LSA: recent: %s:[age %hu] %s:[age %hu]", a->str, agea, b->str, ageb);
  if (agea == OSPF6_LSA_MAXAGE && ageb != OSPF6_LSA_MAXAGE)
    return -1;
  else if (agea != OSPF6_LSA_MAXAGE && ageb == OSPF6_LSA_MAXAGE)
    return 1;

  recent_reason = "age differ";
  if (IS_OSPF6_DUMP_LSA &&
      ((agea > ageb && agea - ageb >= OSPF6_LSA_MAXAGEDIFF) ||
       (agea < ageb && ageb - agea >= OSPF6_LSA_MAXAGEDIFF)))
    zlog_info ("LSA: recent: %s:[age %hu] %s:[age %hu]", a->str, agea, b->str, ageb);
  if (agea > ageb && agea - ageb >= OSPF6_LSA_MAXAGEDIFF)
    return 1;
  else if (agea < ageb && ageb - agea >= OSPF6_LSA_MAXAGEDIFF)
    return -1;

  /* neither recent */
  recent_reason = "the same instance";
  if (IS_OSPF6_DUMP_LSA)
    zlog_info ("LSA: recent: %s == %s", a->str, b->str);
  return 0;
}

int
ospf6_lsa_lsd_num (struct ospf6_lsa_header *lsa_header)
{
  int ldnum = 0;
  u_int16_t len;

  len = ntohs (lsa_header->length);
  len -= sizeof (struct ospf6_lsa_header);
  if (lsa_header->type == htons (OSPF6_LSA_TYPE_ROUTER))
    {
      len -= sizeof (struct ospf6_router_lsa);
      ldnum = len / sizeof (struct ospf6_router_lsd);
    }
  else /* (lsa_header->type == htons (OSPF6_LSA_TYPE_NETWORK)) */
    {
      len -= sizeof (struct ospf6_network_lsa);
      ldnum = len / sizeof (u_int32_t);
    }

  return ldnum;
}

void *
ospf6_lsa_lsd_get (int index, struct ospf6_lsa_header *lsa_header)
{
  void *p;
  struct ospf6_router_lsa *router_lsa;
  struct ospf6_router_lsd *router_lsd;
  struct ospf6_network_lsa *network_lsa;
  struct ospf6_network_lsd *network_lsd;

  if (lsa_header->type == htons (OSPF6_LSA_TYPE_ROUTER))
    {
      router_lsa = (struct ospf6_router_lsa *) (lsa_header + 1);
      router_lsd = (struct ospf6_router_lsd *) (router_lsa + 1);
      router_lsd += index;
      p = (void *) router_lsd;
    }
  else if (lsa_header->type == htons (OSPF6_LSA_TYPE_NETWORK))
    {
      network_lsa = (struct ospf6_network_lsa *) (lsa_header + 1);
      network_lsd = (struct ospf6_network_lsd *) (network_lsa + 1);
      network_lsd += index;
      p = (void *) network_lsd;
    }
  else
    {
      p = (void *) NULL;
    }

  return p;
}

/* network_lsd <-> router_lsd */
static int
ospf6_lsa_lsd_network_reference_match (struct ospf6_network_lsd *network_lsd1,
                                       struct ospf6_lsa_header *lsa_header1,
                                       struct ospf6_router_lsd *router_lsd2,
                                       struct ospf6_lsa_header *lsa_header2)
{
  if (network_lsd1->adv_router != lsa_header2->advrtr)
    return 0;
  if (router_lsd2->type != OSPF6_ROUTER_LSD_TYPE_TRANSIT_NETWORK)
    return 0;
  if (router_lsd2->neighbor_router_id != lsa_header1->advrtr)
    return 0;
  if (router_lsd2->neighbor_interface_id != lsa_header1->ls_id)
    return 0;
  return 1;
}

/* router_lsd <-> router_lsd */
static int
ospf6_lsa_lsd_router_reference_match (struct ospf6_router_lsd *router_lsd1,
                                      struct ospf6_lsa_header *lsa_header1,
                                      struct ospf6_router_lsd *router_lsd2,
                                      struct ospf6_lsa_header *lsa_header2)
{
  if (router_lsd1->type != OSPF6_ROUTER_LSD_TYPE_POINTTOPOINT)
    return 0;
  if (router_lsd2->type != OSPF6_ROUTER_LSD_TYPE_POINTTOPOINT)
    return 0;
  if (router_lsd1->neighbor_router_id != lsa_header2->advrtr)
    return 0;
  if (router_lsd2->neighbor_router_id != lsa_header1->advrtr)
    return 0;
  if (router_lsd1->neighbor_interface_id != router_lsd2->interface_id)
    return 0;
  if (router_lsd2->neighbor_interface_id != router_lsd1->interface_id)
    return 0;
  return 1;
}

int
ospf6_lsa_lsd_is_refer_ok (int index1, struct ospf6_lsa_header *lsa_header1,
                           int index2, struct ospf6_lsa_header *lsa_header2)
{
  struct ospf6_router_lsd *r1, *r2;
  struct ospf6_network_lsd *n;

  r1 = (struct ospf6_router_lsd *) NULL;
  r2 = (struct ospf6_router_lsd *) NULL;
  n = (struct ospf6_network_lsd *) NULL;
  if (lsa_header1->type == htons (OSPF6_LSA_TYPE_ROUTER))
    r1 = (struct ospf6_router_lsd *) ospf6_lsa_lsd_get (index1, lsa_header1);
  else
    n = (struct ospf6_network_lsd *) ospf6_lsa_lsd_get (index1, lsa_header1);

  if (lsa_header2->type == htons (OSPF6_LSA_TYPE_ROUTER))
    r2 = (struct ospf6_router_lsd *) ospf6_lsa_lsd_get (index2, lsa_header2);
  else
    n = (struct ospf6_network_lsd *) ospf6_lsa_lsd_get (index2, lsa_header2);

  if (r1 && r2)
    return ospf6_lsa_lsd_router_reference_match (r1, lsa_header1,
                                                 r2, lsa_header2);
  else if (r1 && n)
    return ospf6_lsa_lsd_network_reference_match (n, lsa_header2,
                                                  r1, lsa_header1);
  else if (n && r2)
    return ospf6_lsa_lsd_network_reference_match (n, lsa_header1,
                                                 r2, lsa_header2);
  return 0;
}

/* vty function for LSAs */
void
ospf6_lsa_show_body_unknown (struct vty *vty,
                            struct ospf6_lsa_header *lsa_header)
{
  vty_out (vty, "    Unknown Type%s", VTY_NEWLINE);
}

void
ospf6_lsa_show_body_router (struct vty *vty,
                           struct ospf6_lsa_header *lsa_header)
{
  int lsdnum;
  char buf[32];
  struct ospf6_router_lsa *router_lsa;
  struct ospf6_router_lsd *router_lsd;

  assert (lsa_header);
  router_lsa = (struct ospf6_router_lsa *)(lsa_header + 1);
  router_lsd = (struct ospf6_router_lsd *)(router_lsa + 1);

  lsdnum = ospf6_lsa_lsd_num (lsa_header);
  assert (lsdnum >= 0);

  while (lsdnum --)
    {
      vty_out (vty, "    Type: %s Metric: %d%s",
               ospf6_router_lsd_type_strings[router_lsd->type],
               ntohs(router_lsd->metric), VTY_NEWLINE);
      vty_out (vty, "    Interface ID: %d%s",
               ntohl (router_lsd->interface_id), VTY_NEWLINE);
      vty_out (vty, "    Neighbor Interface ID: %d%s",
               ntohl (router_lsd->neighbor_interface_id), VTY_NEWLINE);
      vty_out (vty, "    Neighbor Router ID: %s%s",
               inet_ntop (AF_INET, &router_lsd->neighbor_router_id,
                          buf, sizeof (buf)), VTY_NEWLINE);
      router_lsd++;
    }
}

void
ospf6_lsa_show_body_network (struct vty *vty,
                            struct ospf6_lsa_header *lsa_header)
{
  int lsdnum;
  struct ospf6_network_lsa *lsa;
  u_int32_t *router_id;
  char buf[128];

  assert (lsa_header);
  lsa = (struct ospf6_network_lsa *) (lsa_header + 1);
  router_id = (u_int32_t *)(lsa + 1);

  ospf6_opt_capability_string (lsa->options, buf, sizeof (buf));
  vty_out (vty, "     Options: %s%s", buf, VTY_NEWLINE);

  lsdnum = ospf6_lsa_lsd_num (lsa_header);
  assert (lsdnum >= 0);

  while (lsdnum--)
    {
      inet_ntop (AF_INET, router_id, buf, sizeof (buf));
      vty_out (vty, "     Attached Router: %s%s", buf, VTY_NEWLINE);
      router_id ++;
    }
}

void
ospf6_lsa_show_body_inter_prefix (struct vty *vty,
                                 struct ospf6_lsa_header *lsa_header)
{
  struct ospf6_lsa_inter_area_prefix_lsa *inter_prefix_lsa;
  struct ospf6_prefix *prefix;
  struct in6_addr in6;
  char buf[128];

  assert (lsa_header);
  inter_prefix_lsa = (struct ospf6_lsa_inter_area_prefix_lsa *) (lsa_header + 1);

  /* Metric */
  vty_out (vty, "     Metric: %d%s", ntohl (inter_prefix_lsa->metric),
           VTY_NEWLINE);

  prefix = (struct ospf6_prefix *)(inter_prefix_lsa + 1);

  /* Prefix Options */
  ospf6_prefix_options_str (prefix, buf, sizeof (buf));
  vty_out (vty, "     Prefix Options: %s%s", buf, VTY_NEWLINE);

  /* Address Prefix */
  ospf6_prefix_in6_addr (prefix, &in6);
  inet_ntop (AF_INET6, &in6, buf, sizeof (buf));
  vty_out (vty, "     Prefix: %s/%d%s",
           buf, prefix->prefix_length, VTY_NEWLINE);
}

void
ospf6_lsa_show_body_inter_router (struct vty *vty,
                                 struct ospf6_lsa_header *lsa_header)
{
  struct ospf6_lsa_inter_area_router_lsa *inter_router_lsa;
  char buf[128];

  assert (lsa_header);
  inter_router_lsa = (struct ospf6_lsa_inter_area_router_lsa *) (lsa_header + 1);

  /* Options */
  ospf6_opt_capability_string (inter_router_lsa->options, buf, sizeof (buf));
  vty_out (vty, "     Options: %s%s", buf, VTY_NEWLINE);

  /* Metric */
  vty_out (vty, "     Metric: %d%s", ntohl (inter_router_lsa->metric),
           VTY_NEWLINE);

  /* Destination Router ID */
  inet_ntop (AF_INET, &inter_router_lsa->router_id, buf, sizeof (buf));
  vty_out (vty, "     Destination Router ID: %s", buf);
}

void
ospf6_lsa_show_body_as_external (struct vty *vty,
                                struct ospf6_lsa_header *lsa_header)
{
  struct ospf6_as_external_lsa *elsa;
  char buf[128], *ptr;
  struct in6_addr in6;

  assert (lsa_header);
  elsa = (struct ospf6_as_external_lsa *)(lsa_header + 1);

  /* bits */
  snprintf (buf, sizeof (buf), "%s%s%s",
            (ASE_LSA_ISSET (elsa, ASE_LSA_BIT_E) ? "E" : "-"),
            (ASE_LSA_ISSET (elsa, ASE_LSA_BIT_F) ? "F" : "-"),
            (ASE_LSA_ISSET (elsa, ASE_LSA_BIT_T) ? "T" : "-"));

  vty_out (vty, "     Bits: %s%s", buf, VTY_NEWLINE);
  vty_out (vty, "     Metric: %5hu%s", ntohs (elsa->ase_metric), VTY_NEWLINE);

  ospf6_prefix_options_str (&elsa->ospf6_prefix, buf, sizeof (buf));
  vty_out (vty, "     Prefix Options: %s%s", buf, VTY_NEWLINE);

  vty_out (vty, "     Referenced LSType: %d%s",
           ntohs (elsa->ospf6_prefix.prefix_refer_lstype), VTY_NEWLINE);

  ospf6_prefix_in6_addr (&elsa->ospf6_prefix, &in6);
  inet_ntop (AF_INET6, &in6, buf, sizeof (buf));
  vty_out (vty, "     Prefix: %s/%d%s",
           buf, elsa->ospf6_prefix.prefix_length, VTY_NEWLINE);

  /* Forwarding-Address */
  if (ASE_LSA_ISSET (elsa, ASE_LSA_BIT_F))
    {
      ptr = ((char *)(elsa + 1))
            + OSPF6_PREFIX_SPACE (elsa->ospf6_prefix.prefix_length);
      inet_ntop (AF_INET6, (struct in6_addr *) ptr, buf, sizeof (buf));
      vty_out (vty, "     Forwarding-Address: %s%s", buf, VTY_NEWLINE);
    }
}

void
ospf6_lsa_show_body_group_membership (struct vty *vty,
                                     struct ospf6_lsa_header *lsa_header)
{
}

void
ospf6_lsa_show_body_type_7 (struct vty *vty,
                           struct ospf6_lsa_header *lsa_header)
{
}

void
ospf6_lsa_show_body_link (struct vty *vty,
                         struct ospf6_lsa_header *lsa_header)
{
  struct ospf6_link_lsa *llsap;
  int prefixnum;
  struct ospf6_prefix *prefix;
  char buf[128];
  struct in6_addr in6;

  assert (lsa_header);

  llsap = (struct ospf6_link_lsa *) (lsa_header + 1);
  prefixnum = ntohl (llsap->llsa_prefix_num);

  inet_ntop (AF_INET6, (void *)&llsap->llsa_linklocal, buf, sizeof (buf));
  vty_out (vty, "     LinkLocal Address: %s%s", buf, VTY_NEWLINE);
  vty_out (vty, "     Number of Prefix: %d%s", prefixnum, VTY_NEWLINE);

  prefix = (struct ospf6_prefix *)(llsap + 1);

  while (prefixnum--)
    {
      ospf6_prefix_options_str (prefix, buf, sizeof (buf));
      vty_out (vty, "     Prefix Options: %s%s", buf, VTY_NEWLINE);

      ospf6_prefix_in6_addr (prefix, &in6);
      inet_ntop (AF_INET6, &in6, buf, sizeof (buf));
      vty_out (vty, "     Prefix: %s/%d%s",
               buf, prefix->prefix_length, VTY_NEWLINE);

      prefix = OSPF6_NEXT_PREFIX (prefix);
    }
}

void
ospf6_lsa_show_body_intra_prefix (struct vty *vty,
                                 struct ospf6_lsa_header *lsa_header)
{
  struct ospf6_intra_area_prefix_lsa *iap_lsa;
  struct ospf6_prefix *prefix;
  unsigned short prefixnum;
  char buf[128];
  struct in6_addr in6;

  assert (lsa_header);
  iap_lsa = (struct ospf6_intra_area_prefix_lsa *) (lsa_header + 1);
  prefixnum = ntohs (iap_lsa->prefix_number);

  vty_out (vty, "     Number of Prefix: %d%s", prefixnum, VTY_NEWLINE);
  vty_out (vty, "     Referenced LS Type: %s%s",
           ospf6_lsa_type_string (iap_lsa->refer_lstype),
           VTY_NEWLINE);
  vty_out (vty, "     Referenced LS ID: %d%s",
           ntohl (iap_lsa->refer_lsid), VTY_NEWLINE);
  inet_ntop (AF_INET, &iap_lsa->refer_advrtr, buf, sizeof (buf));
  vty_out (vty, "     Referenced Advertising Router: %s%s",
           buf, VTY_NEWLINE);

  prefix = (struct ospf6_prefix *)(iap_lsa + 1);
  while (prefixnum--)
    {
      ospf6_prefix_options_str (prefix, buf, sizeof (buf));
      vty_out (vty, "     Prefix Options: %s%s", buf, VTY_NEWLINE);

      ospf6_prefix_in6_addr (prefix, &in6);
      inet_ntop (AF_INET6, &in6, buf, sizeof (buf));
      vty_out (vty, "     Prefix: %s/%d%s",
               buf, prefix->prefix_length, VTY_NEWLINE);

      prefix = OSPF6_NEXT_PREFIX (prefix);
    }
}

void
(*ospf6_lsa_show_body[OSPF6_LSA_TYPE_MAX]) (struct vty *,
                                           struct ospf6_lsa_header *) =
{
  ospf6_lsa_show_body_unknown,
  ospf6_lsa_show_body_router,
  ospf6_lsa_show_body_network,
  ospf6_lsa_show_body_inter_prefix,
  ospf6_lsa_show_body_inter_router,
  ospf6_lsa_show_body_as_external,
  ospf6_lsa_show_body_group_membership,
  ospf6_lsa_show_body_type_7,
  ospf6_lsa_show_body_link,
  ospf6_lsa_show_body_intra_prefix,
};

void
ospf6_lsa_show (struct vty *vty, struct ospf6_lsa *lsa)
{
  struct ospf6_lsa_header *lsa_header;
  char advrtr[64];

  assert (lsa);
  lsa_header = (struct ospf6_lsa_header *) lsa->lsa_hdr;
  assert (lsa_header);

  inet_ntop (AF_INET, &lsa_header->advrtr, advrtr, sizeof (advrtr));

  vty_out (vty, "Age: %4hu Type: %s%s", ospf6_lsa_age_current (lsa),
           ospf6_lsa_type_string (lsa_header->type), VTY_NEWLINE);
  vty_out (vty, "Link State ID: %d%s", ntohl (lsa_header->ls_id),
           VTY_NEWLINE);
  vty_out (vty, "Advertising Router: %s%s", advrtr, VTY_NEWLINE);
  vty_out (vty, "LS Sequence Number: %#x%s", ntohl (lsa_header->seqnum),
           VTY_NEWLINE);
  vty_out (vty, "CheckSum: %#hx Length: %hu%s", ntohs (lsa_header->checksum),
           ntohs (lsa_header->length), VTY_NEWLINE);

  if (OSPF6_LSA_TYPESW_ISKNOWN (lsa_header->type))
    (*ospf6_lsa_show_body [OSPF6_LSA_TYPESW (lsa_header->type)])
       (vty, lsa_header);
  else
    ospf6_lsa_show_body_unknown (vty, lsa_header);

  vty_out (vty, "%s", VTY_NEWLINE);
}

/* OSPFv3 LSA creation/deletion function */

/* calculate LS sequence number for my new LSA.
   return value is network byte order */
static signed long
ospf6_seqnum_new (u_int16_t type, u_int32_t id, u_int32_t adv_router)
{
  struct ospf6_lsa *lsa;
  signed long seqnum;

  /* get current database copy */
  lsa = ospf6_lsdb_lookup (type, id, adv_router);

  /* if current database copy not found, return InitialSequenceNumber */
  if (!lsa)
    seqnum = INITIAL_SEQUENCE_NUMBER;
  else
    seqnum = (signed long) ntohl (lsa->header->seqnum) + 1;

  return (htonl (seqnum));
}

static void
ospf6_lsa_header_set (u_int16_t type, u_int32_t ls_id, u_int32_t advrtr,
                      struct ospf6_lsa_header *lsa_header, int bodysize)
{
  /* fill LSA header */
  lsa_header->age = 0;
  lsa_header->type = type;
  lsa_header->ls_id = ls_id;
  lsa_header->advrtr = advrtr;
  lsa_header->seqnum =
    ospf6_seqnum_new (lsa_header->type, lsa_header->ls_id,
                      lsa_header->advrtr);
  lsa_header->length = htons (sizeof (struct ospf6_lsa_header) + bodysize);

  /* LSA checksum */
  ospf6_lsa_checksum (lsa_header);
}

struct ospf6_lsa *
ospf6_lsa_create (struct ospf6_lsa_header *source)
{
  struct ospf6_lsa *lsa = NULL;
  struct ospf6_lsa_header *lsa_header = NULL;
  u_int16_t lsa_size = 0;
  char buf[128];

  /* whole length of this LSA */
  lsa_size = ntohs (source->length);

  /* allocate memory for this LSA */
  lsa_header = (struct ospf6_lsa_header *)
    XMALLOC (MTYPE_OSPF6_LSA, lsa_size);
  if (! lsa_header)
    {
      zlog_err ("Lsa: Can't allocate memory for LSA Header");
      return (struct ospf6_lsa *) NULL;
    }
  memset (lsa_header, 0, lsa_size);

  /* copy LSA from source */
  memcpy (lsa_header, source, lsa_size);

  /* LSA information structure */
  /* allocate memory */
  lsa = (struct ospf6_lsa *)
          XMALLOC (MTYPE_OSPF6_LSA, sizeof (struct ospf6_lsa));
  memset (lsa, 0, sizeof (struct ospf6_lsa));

  /* dump string */
  snprintf (lsa->str, sizeof (lsa->str), "%s(%s[%lu])",
            ospf6_lsa_type_string (lsa_header->type),
            inet_ntop (AF_INET, &lsa_header->advrtr, buf, sizeof (buf)),
            (unsigned long) ntohl (lsa_header->ls_id));

  lsa->lsa_hdr = (struct ospf6_lsa_hdr *) lsa_header;
  lsa->header = (struct ospf6_lsa_header__ *) lsa_header;

  lsa->summary = 0; /* this is not LSA summary */

  /* calculate birth, expire and refresh of this lsa */
  ospf6_lsa_age_set (lsa);

#ifdef DEBUG
  if (IS_OSPF6_DUMP_LSA)
    zlog_info ("Lsa: created %s(%#x)", lsa->str, lsa);
#endif /* DEBUG */

  return lsa;
}

struct ospf6_lsa *
ospf6_lsa_summary_create (struct ospf6_lsa_header__ *source)
{
  struct ospf6_lsa *lsa = NULL;
  struct ospf6_lsa_header *lsa_header = NULL;
  u_int16_t lsa_size = 0;
  char buf[128];

  /* LSA summary contains LSA Header only */
  lsa_size = sizeof (struct ospf6_lsa_header);

  /* allocate memory for this LSA */
  lsa_header = (struct ospf6_lsa_header *)
    XMALLOC (MTYPE_OSPF6_LSA_SUMMARY, lsa_size);
  memset (lsa_header, 0, lsa_size);

  /* copy LSA from source */
  memcpy (lsa_header, source, lsa_size);

  /* LSA information structure */
  /* allocate memory */
  lsa = (struct ospf6_lsa *)
          XMALLOC (MTYPE_OSPF6_LSA_SUMMARY, sizeof (struct ospf6_lsa));
  memset (lsa, 0, sizeof (struct ospf6_lsa));

  /* dump string */
  snprintf (lsa->str, sizeof (lsa->str), "%s(%s[%lu])[S]",
            ospf6_lsa_type_string (lsa_header->type),
            inet_ntop (AF_INET, &lsa_header->advrtr, buf, sizeof (buf)),
            (unsigned long) ntohl (lsa_header->ls_id));

  lsa->lsa_hdr = (struct ospf6_lsa_hdr *) lsa_header;
  lsa->header = (struct ospf6_lsa_header__ *) lsa_header;

  lsa->summary = 1; /* this is LSA summary */

  /* calculate birth, expire and refresh of this lsa */
  ospf6_lsa_age_set (lsa);

#ifdef DEBUG
  if (IS_OSPF6_DUMP_LSA)
    zlog_info ("Lsa: created summary %s(%#x)", lsa->str, lsa);
#endif /* DEBUG */

  return lsa;
}

void
ospf6_lsa_delete (struct ospf6_lsa *lsa)
{
  /* just to make sure */
  if (lsa->lock != 0)
    {
      zlog_err ("Can't delete %s: lock: %ld", lsa->str, lsa->lock);
      return;
    }

  /* cancel threads */
  if (lsa->expire)
    thread_cancel (lsa->expire);
  lsa->expire = (struct thread *) NULL;
  if (lsa->refresh)
    thread_cancel (lsa->refresh);
  lsa->refresh = (struct thread *) NULL;

  if (IS_OSPF6_DUMP_LSA)
    {
      if (lsa->summary)
        zlog_info ("LSA: delete summary %s(%p)", lsa->str, lsa);
      else
        zlog_info ("LSA: delete %s(%p)", lsa->str, lsa);
    }

  /* do free */
  if (lsa->summary)
    XFREE (MTYPE_OSPF6_LSA_SUMMARY, lsa->header);
  else
    XFREE (MTYPE_OSPF6_LSA, lsa->header);
  lsa->header = NULL;

  if (lsa->summary)
    XFREE (MTYPE_OSPF6_LSA_SUMMARY, lsa);
  else
    XFREE (MTYPE_OSPF6_LSA, lsa);
}

/* increment reference counter of  struct ospf6_lsa */
void
ospf6_lsa_lock (struct ospf6_lsa *lsa)
{
  lsa->lock++;
  return;
}

/* decrement reference counter of  struct ospf6_lsa */
void
ospf6_lsa_unlock (struct ospf6_lsa *lsa)
{
  /* decrement reference counter */
  if (lsa->lock > 0)
    lsa->lock--;
  else
    zlog_warn ("Can't unlock %s: already no lock", lsa->str);
}

/* check necessity to update LSA:
   returns 1 if it's necessary to reoriginate */
static int
ospf6_lsa_is_really_reoriginate (struct ospf6_lsa *new)
{
  struct ospf6_lsa *old;
  int diff;

  /* find previous LSA */
  old = ospf6_lsdb_lookup (new->header->type, new->header->id,
                           new->header->adv_router);
  if (! old)
    return 1;

  /* Check if this is refresh */
  if (old->refresh == (struct thread *) NULL)
    {
      zlog_warn ("LSA: reoriginate: %s: cause it's refresh", new->str);
      return 1;
    }

  /* Are these contents different ? */
  diff = ospf6_lsa_differ (new, old);

  if (diff)
    return 1;

  if (IS_OSPF6_DUMP_LSA)
    zlog_info ("LSA: Suppress updating %s", new->str);
  return 0;
}

/* RFC2740 3.4.3.1 Router-LSA */
void
ospf6_lsa_update_router (u_int32_t area_id)
{
  char buffer [MAXLSASIZE];
  u_int16_t size;
  struct ospf6_lsa *lsa, *prev_lsa;
  struct ospf6_area *o6a;
  int count;

  struct ospf6_router_lsa *router_lsa;
  struct ospf6_router_lsd *router_lsd;
  listnode i;
  struct ospf6_interface *o6i;
  struct ospf6_neighbor *o6n = NULL;

  o6a = ospf6_area_lookup (area_id, ospf6);
  if (! o6a)
    {
      inet_ntop (AF_INET, &area_id, buffer, sizeof (buffer));
      zlog_warn ("LSA: Update Router-LSA: No such area: %s", buffer);
      return;
    }

  if (IS_OSPF6_DUMP_LSA)
    zlog_info ("Lsa: Update Router-LSA for %s", o6a->str);

  /* find previous LSA */
  prev_lsa = ospf6_lsdb_lookup (htons (OSPF6_LSA_TYPE_ROUTER),
                                htonl (0), o6a->ospf6->router_id);

  size = sizeof (struct ospf6_router_lsa);
  memset (buffer, 0, sizeof (buffer));
  router_lsa = (struct ospf6_router_lsa *)
    (buffer + sizeof (struct ospf6_lsa_header));

  OSPF6_OPT_CLEAR_ALL (router_lsa->options);
  OSPF6_OPT_SET (router_lsa->options, OSPF6_OPT_V6);
  OSPF6_OPT_SET (router_lsa->options, OSPF6_OPT_E);
  OSPF6_OPT_CLEAR (router_lsa->options, OSPF6_OPT_MC);
  OSPF6_OPT_CLEAR (router_lsa->options, OSPF6_OPT_N);
  OSPF6_OPT_SET (router_lsa->options, OSPF6_OPT_R);
  OSPF6_OPT_CLEAR (router_lsa->options, OSPF6_OPT_DC);

  OSPF6_ROUTER_LSA_CLEAR_ALL_BITS (router_lsa);
  OSPF6_ROUTER_LSA_CLEAR (router_lsa, OSPF6_ROUTER_LSA_BIT_B);

  if (ospf6_is_asbr (o6a->ospf6))
    OSPF6_ROUTER_LSA_SET (router_lsa, OSPF6_ROUTER_LSA_BIT_E);
  else
    OSPF6_ROUTER_LSA_CLEAR (router_lsa, OSPF6_ROUTER_LSA_BIT_E);

  OSPF6_ROUTER_LSA_CLEAR (router_lsa, OSPF6_ROUTER_LSA_BIT_V);
  OSPF6_ROUTER_LSA_CLEAR (router_lsa, OSPF6_ROUTER_LSA_BIT_W);

  /* describe links for each interfaces */
  router_lsd = (struct ospf6_router_lsd *) (router_lsa + 1);
  for (i = listhead (o6a->if_list); i; nextnode (i))
    {
      o6i = (struct ospf6_interface *) getdata (i);
      assert (o6i);

      /* Interfaces in state Down or Loopback are not described */
      if (o6i->state == IFS_DOWN || o6i->state == IFS_LOOPBACK)
        continue;

      /* Nor are interfaces without any full adjacencies described */
      count = 0;
      o6i->foreach_nei (o6i, &count, NBS_FULL, ospf6_count_state);
      if (count == 0)
        continue;

      /* Point-to-Point interfaces */
      if (if_is_pointopoint (o6i->interface))
        {
          if (listcount (o6i->neighbor_list) == 0)
            continue;

          if (listcount (o6i->neighbor_list) != 1)
            zlog_warn ("LSA: Multiple neighbors on PoinToPoint: %s",
                       o6i->interface->name);

          o6n = (struct ospf6_neighbor *)
                   getdata (listhead (o6i->neighbor_list));
          assert (o6n);

          router_lsd->type = OSPF6_ROUTER_LSD_TYPE_POINTTOPOINT;
          router_lsd->metric = htons (o6i->cost);
          router_lsd->interface_id = htonl (o6i->if_id);
          router_lsd->neighbor_interface_id = htonl (o6n->ifid);
          router_lsd->neighbor_router_id = o6n->router_id;

          size += sizeof (struct ospf6_router_lsd);
          router_lsd ++;

          continue;
        }

      /* Broadcast and NBMA interfaces */
      if (if_is_broadcast (o6i->interface))
        {
          /* If this router is not DR,
             and If this router not fully adjacent with DR,
             this interface is not transit yet: ignore. */
          if (o6i->state != IFS_DR)
            {
              o6n = ospf6_neighbor_lookup (o6i->dr, o6i); /* find DR */
              assert (o6n);
              if (o6n->state != NBS_FULL)
                continue;
            }
          else
            {
              count = 0;
              o6i->foreach_nei (o6i, &count, NBS_FULL, ospf6_count_state);
              if (count == 0)
                continue;
            }

          router_lsd->type = OSPF6_ROUTER_LSD_TYPE_TRANSIT_NETWORK;
          router_lsd->metric = htons (o6i->cost);
          router_lsd->interface_id = htonl (o6i->if_id);
          if (o6i->state != IFS_DR)
            {
              router_lsd->neighbor_interface_id = htonl (o6n->ifid);
              router_lsd->neighbor_router_id = o6n->router_id;
            }
          else
            {
              router_lsd->neighbor_interface_id = htonl (o6i->if_id);
              router_lsd->neighbor_router_id = o6i->area->ospf6->router_id;
            }

          size += sizeof (struct ospf6_router_lsd);
          router_lsd ++;

          continue;
        }

      /* Virtual links */
        /* xxx */
      /* Point-to-Multipoint interfaces */
        /* xxx */
    }

  /* Fill LSA Header */
  ospf6_lsa_header_set (htons (OSPF6_LSA_TYPE_ROUTER), htonl (0),
                        o6a->ospf6->router_id,
                        (struct ospf6_lsa_header *)buffer, size);

  /* create LSA */
  lsa = ospf6_lsa_create ((struct ospf6_lsa_header *) buffer);
  lsa->refresh = thread_add_timer (master, ospf6_lsa_refresh, lsa,
                                   OSPF6_LS_REFRESH_TIME);
  lsa->scope = (void *) o6a;

  if (ospf6_lsa_is_really_reoriginate (lsa))
    {
      ospf6_dbex_remove_from_all_retrans_list (lsa);
      ospf6_dbex_flood (lsa, NULL);
      ospf6_lsdb_install (lsa);
    }
  else
    ospf6_lsa_delete (lsa);
}

/* RFC2740 3.4.3.2 Network-LSA */
void
ospf6_lsa_update_network (char *ifname)
{
  char buffer [MAXLSASIZE];
  u_int16_t size;
  struct ospf6_lsa *lsa, *prev_lsa;
  struct interface *ifp;
  struct ospf6_interface *o6i;
  int count;

  struct ospf6_network_lsa *nlsa;
  listnode i;
  struct ospf6_neighbor *o6n;
  u_int32_t *router_id;

  ifp = if_lookup_by_name (ifname);
  if (! ifp)
    {
      zlog_warn ("LSA: Update Network: No such Interface: %s", ifname);
      return;
    }
  o6i = (struct ospf6_interface *) ifp->info;
  if (! o6i)
    {
      zlog_warn ("LSA: Update Network: Interface not enabled: %s", ifname);
      return;
    }

  /* find previous LSA */
  prev_lsa = ospf6_lsdb_lookup (htons (OSPF6_LSA_TYPE_NETWORK),
                                htonl (o6i->if_id),
                                o6i->area->ospf6->router_id);

  /* Don't originate Network-LSA if not DR */
  if (o6i->state != IFS_DR)
    {
      if (IS_OSPF6_DUMP_LSA)
        zlog_info ("LSA Update Network: Interface %s is not DR",
                   o6i->interface->name);
      if (prev_lsa)
        ospf6_lsa_premature_aging (prev_lsa);
      return;
    }

  /* If none of neighbor is adjacent to us */
  count = 0;
  o6i->foreach_nei (o6i, &count, NBS_FULL, ospf6_count_state);
  if (count == 0)
    {
      if (IS_OSPF6_DUMP_LSA)
        zlog_info ("LSA Update Network: Interface %s is Stub",
                   o6i->interface->name);
      if (prev_lsa)
        ospf6_lsa_premature_aging (prev_lsa);
      return;
    }

  if (IS_OSPF6_DUMP_LSA)
    zlog_info ("LSA Update Network: Interface %s", o6i->interface->name);

  /* prepare buffer */
  size = sizeof (struct ospf6_network_lsa);
  memset (buffer, 0, sizeof (buffer));
  nlsa = (struct ospf6_network_lsa *)
    (buffer + sizeof (struct ospf6_lsa_header));
  router_id = (u_int32_t *)(nlsa + 1);

  /* set fields of myself */
  *router_id++ = o6i->area->ospf6->router_id;
  size += sizeof (u_int32_t);
  nlsa->options[0] |= o6i->area->options[0];
  nlsa->options[1] |= o6i->area->options[1];
  nlsa->options[2] |= o6i->area->options[2];

  /* Walk through neighbors */
  for (i = listhead (o6i->neighbor_list); i; nextnode (i))
    {
      o6n = (struct ospf6_neighbor *) getdata (i);

      if (o6n->state != NBS_FULL)
        continue;

      /* set this neighbor's Router-ID to LSA */
      *router_id++ = o6n->router_id;
      size += sizeof (u_int32_t);

      /* options field is logical OR */
      nlsa->options[0] |= o6n->options[0];
      nlsa->options[1] |= o6n->options[1];
      nlsa->options[2] |= o6n->options[2];
    }

  /* Fill LSA Header */
  ospf6_lsa_header_set (htons (OSPF6_LSA_TYPE_NETWORK), htonl (o6i->if_id),
                        o6i->area->ospf6->router_id,
                        (struct ospf6_lsa_header *) buffer, size);

  /* create LSA */
  lsa = ospf6_lsa_create ((struct ospf6_lsa_header *) buffer);
  lsa->refresh = thread_add_timer (master, ospf6_lsa_refresh, lsa,
                                   OSPF6_LS_REFRESH_TIME);
  lsa->scope = (void *) o6i->area;

  if (ospf6_lsa_is_really_reoriginate (lsa))
    {
      ospf6_dbex_remove_from_all_retrans_list (lsa);
      ospf6_dbex_flood (lsa, NULL);
      ospf6_lsdb_install (lsa);
    }
  else
    ospf6_lsa_delete (lsa);
}

/* RFC2740 3.4.3.5 AS-External-LSA */
void
ospf6_lsa_update_as_external (u_int32_t lsid)
{
  char buffer [MAXLSASIZE];
  u_int16_t size;
  struct ospf6_lsa *lsa, *prev_lsa;

  struct ospf6_as_external_lsa *elsa;
  struct route_node *rn, *target;
  struct ospf6_redistribute_info *info = NULL;
  char *p;

  /* find previous LSA */
  prev_lsa = ospf6_lsdb_lookup (htons (OSPF6_LSA_TYPE_AS_EXTERNAL),
                                ntohl (lsid), ospf6->router_id);

  /* find corresponding route node */
  target = NULL;
  for (rn = route_top (ospf6->external_table); rn; rn = route_next (rn))
    {
      info = (struct ospf6_redistribute_info *) rn->info;
      if (info && info->id == lsid)
        target = rn;
    }

  if (! target)
    {
      if (IS_OSPF6_DUMP_LSA)
        zlog_info ("LSA: Don't create AS-External-LSA for ID %d: No entry",
                   lsid);

      if (prev_lsa)
        {
          zlog_info ("LSA: Delete old Self-originated AS-External-LSA");
          ospf6_lsa_premature_aging (prev_lsa);
        }
      return;
    }

  if (IS_OSPF6_DUMP_LSA)
    zlog_info ("LSA Update AS-External: ID:%d", lsid);

  /* prepare buffer */
  size = sizeof (struct ospf6_as_external_lsa);
  memset (buffer, 0, sizeof (buffer));
  elsa = (struct ospf6_as_external_lsa *)
    (buffer + sizeof (struct ospf6_lsa_header));
  p = (char *) (elsa + 1);

  info = (struct ospf6_redistribute_info *) target->info;
  if (info->metric_type == 2)
    ASE_LSA_SET (elsa, ASE_LSA_BIT_E);   /* type2 */
  else
    ASE_LSA_CLEAR (elsa, ASE_LSA_BIT_E); /* type1 */

  if (! IN6_IS_ADDR_UNSPECIFIED (&info->forward))
    ASE_LSA_SET (elsa, ASE_LSA_BIT_F);   /* forwarding address */
  else
    ASE_LSA_CLEAR (elsa, ASE_LSA_BIT_F); /* forwarding address */

  ASE_LSA_CLEAR (elsa, ASE_LSA_BIT_T);   /* external route tag */

  /* don't know how to use */
  elsa->ase_pre_metric = 0;

  /* set metric. note: related to E bit */
  elsa->ase_metric = htons (info->metric);

  /* prefixlen */
  elsa->ospf6_prefix.prefix_length = target->p.prefixlen;

  /* PrefixOptions */
  elsa->ospf6_prefix.prefix_options = info->prefix_options;

  /* don't use refer LS-type */
  elsa->ospf6_prefix.prefix_refer_lstype = htons (0);

  /* set Prefix */
  memcpy (p, &target->p.u.prefix6, OSPF6_PREFIX_SPACE (target->p.prefixlen));
  ospf6_prefix_apply_mask (&elsa->ospf6_prefix);
  size += OSPF6_PREFIX_SPACE (target->p.prefixlen);
  p += OSPF6_PREFIX_SPACE (target->p.prefixlen);

  /* Forwarding address */
  if (ASE_LSA_ISSET (elsa, ASE_LSA_BIT_F))
    {
      memcpy (p, &info->forward, sizeof (struct in6_addr));
      size += sizeof (struct in6_addr);
      p += sizeof (struct in6_addr);
    }

  /* External Route Tag */
  if (ASE_LSA_ISSET (elsa, ASE_LSA_BIT_T))
    {
      /* xxx */
    }

  /* Fill LSA Header */
  ospf6_lsa_header_set (htons (OSPF6_LSA_TYPE_AS_EXTERNAL), htonl (lsid),
                        ospf6->router_id,
                        (struct ospf6_lsa_header *) buffer, size);

  /* create LSA */
  lsa = ospf6_lsa_create ((struct ospf6_lsa_header *) buffer);
  lsa->refresh = thread_add_timer (master, ospf6_lsa_refresh, lsa,
                                   OSPF6_LS_REFRESH_TIME);
  lsa->scope = (void *) ospf6;

  if (ospf6_lsa_is_really_reoriginate (lsa))
    {
      ospf6_dbex_remove_from_all_retrans_list (lsa);
      ospf6_dbex_flood (lsa, NULL);
      ospf6_lsdb_install (lsa);
    }
  else
    ospf6_lsa_delete (lsa);
}

/* RFC2740 3.4.3.7 Intra-Area-Prefix-LSA */
void
ospf6_lsa_update_intra_prefix_transit (char *ifname)
{
  char buffer [MAXLSASIZE];
  u_int16_t size;
  struct ospf6_lsa *lsa, *prev_lsa;
  struct interface *ifp;
  struct ospf6_interface *o6i;
  int count;

  struct ospf6_intra_area_prefix_lsa *intra_prefix_lsa;
  struct ospf6_lsdb_node *node;
  struct ospf6_neighbor *o6n;
  struct ospf6_prefix *o6_pstart, *o6_pcurrent, *o6_p1, *o6_p2;
  struct ospf6_link_lsa *link_body;
  u_int16_t prefix_number, link_prefix_number;
  struct in6_addr in6;
  char buf[128];
  int duplicate;

  ifp = if_lookup_by_name (ifname);
  if (! ifp)
    {
      zlog_warn ("LSA: Update Intra-Area-Prefix-LSA(Transit): "
                 "No such Interface: %s", ifname);
      return;
    }
  o6i = (struct ospf6_interface *) ifp->info;
  if (! o6i)
    {
      zlog_warn ("LSA: Update Intra-Area-Prefix-LSA(Transit): "
                 "Interface not enabled: %s", ifname);
      return;
    }

  /* find previous LSA */
  prev_lsa = ospf6_lsdb_lookup (htons (OSPF6_LSA_TYPE_INTRA_PREFIX),
                                htonl (o6i->if_id),
                                o6i->area->ospf6->router_id);

  /* Don't originate Network-LSA if not DR */
  if (o6i->state != IFS_DR)
    {
      if (IS_OSPF6_DUMP_LSA)
        zlog_info ("LSA Update Intra-Area-Prefix(Transit): "
                   "Interface %s is not DR", o6i->interface->name);
      if (prev_lsa)
        ospf6_lsa_premature_aging (prev_lsa);
      return;
    }

  /* If none of neighbor is adjacent to us */
  count = 0;
  o6i->foreach_nei (o6i, &count, NBS_FULL, ospf6_count_state);
  if (count == 0)
    {
      if (IS_OSPF6_DUMP_LSA)
        zlog_info ("LSA Update Intra-Area-Prefix(Transit): "
                   "Interface %s is Stub", o6i->interface->name);
      if (prev_lsa)
        ospf6_lsa_premature_aging (prev_lsa);
      return;
    }

  if (IS_OSPF6_DUMP_LSA)
    zlog_info ("LSA Update Intra-Area-Prefix(Transit): Interface %s",
               o6i->interface->name);

  /* prepare buffer */
  size = sizeof (struct ospf6_intra_area_prefix_lsa);
  memset (buffer, 0, sizeof (buffer));
  intra_prefix_lsa = (struct ospf6_intra_area_prefix_lsa *)
    (buffer + sizeof (struct ospf6_lsa_header));
  o6_pstart = (struct ospf6_prefix *)(intra_prefix_lsa + 1);
  o6_pcurrent = o6_pstart;
  prefix_number = 0;

  /* Set Referenced LSA field */
  intra_prefix_lsa->refer_lstype = htons (OSPF6_LSA_TYPE_NETWORK);
  intra_prefix_lsa->refer_lsid = htonl (o6i->if_id);
  intra_prefix_lsa->refer_advrtr = o6i->area->ospf6->router_id;

  /* foreach Link-LSA associated with this Link */
  for (node = ospf6_lsdb_head (o6i->lsdb); node; node = ospf6_lsdb_next (node))
    {
      if (node->lsa->header->type != htons (OSPF6_LSA_TYPE_LINK))
        continue;

      if (ospf6_lsa_is_maxage (node->lsa))
        continue;

      if (IS_OSPF6_DUMP_LSA)
        zlog_info ("LSA Update Intra-Area-Prefix(Transit): "
                   "Checking %s", node->lsa->str);

      if (node->lsa->header->adv_router != o6i->area->ospf6->router_id)
        {
          o6n = ospf6_neighbor_lookup (node->lsa->header->adv_router, o6i);
          if (! o6n || o6n->state != NBS_FULL)
            {
              if (IS_OSPF6_DUMP_LSA)
                zlog_info ("LSA Update Intra-Area-Prefix(Transit): "
                           "neighbor not found or not FULL");
              continue;
            }
        }

      /* For each Prefix in this Link-LSA */
      link_body = (struct ospf6_link_lsa *) (node->lsa->header + 1);
      link_prefix_number = ntohl (link_body->llsa_prefix_num);

      if (IS_OSPF6_DUMP_LSA)
        zlog_info ("LSA Update Intra-Area-Prefix(Transit): "
                   "Prefix #%d", link_prefix_number);

      for (o6_p1 = (struct ospf6_prefix *) (link_body + 1);
           link_prefix_number; link_prefix_number --)
        {
          ospf6_prefix_in6_addr (o6_p1, &in6);
          inet_ntop (AF_INET6, &in6, buf, sizeof (buf));

          if (IS_OSPF6_DUMP_LSA)
            zlog_info ("LSA Update Intra-Area-Prefix(Transit): "
                       "Prefix %s", buf);

          /* filter linklocal prefix */
          if (IN6_IS_ADDR_LINKLOCAL (&in6))
            {
              if (IS_OSPF6_DUMP_LSA)
                zlog_info ("LSA Update Intra-Area-Prefix(Transit): "
                           "%s is Linklocal", buf);
              o6_p1 = OSPF6_NEXT_PREFIX (o6_p1);
              continue;
            }

          /* filter unspecified(default) prefix */
          if (IN6_IS_ADDR_UNSPECIFIED (&in6))
            {
              if (IS_OSPF6_DUMP_LSA)
                zlog_info ("LSA Update Intra-Area-Prefix(Transit): "
                           "%s is Unspecified", buf);
              o6_p1 = OSPF6_NEXT_PREFIX (o6_p1);
              continue;
            }

          /* filter loopback prefix */
          if (IN6_IS_ADDR_LOOPBACK (&in6))
            {
              if (IS_OSPF6_DUMP_LSA)
                zlog_info ("LSA Update Intra-Area-Prefix(Transit): "
                           "%s is Loopback", buf);
              o6_p1 = OSPF6_NEXT_PREFIX (o6_p1);
              continue;
            }

          /* filter IPv4 compatible prefix */
          if (IN6_IS_ADDR_V4COMPAT (&in6))
            {
              if (IS_OSPF6_DUMP_LSA)
                zlog_info ("LSA Update Intra-Area-Prefix(Transit): "
                           "%s is v4-Compatible", buf);
              o6_p1 = OSPF6_NEXT_PREFIX (o6_p1);
              continue;
            }

          /* filter IPv4 Mapped prefix */
          if (IN6_IS_ADDR_V4MAPPED (&in6))
            {
              if (IS_OSPF6_DUMP_LSA)
                zlog_info ("LSA Update Intra-Area-Prefix(Transit): "
                           "%s is v4-Mapped", buf);
              o6_p1 = OSPF6_NEXT_PREFIX (o6_p1);
              continue;
            }

          /* Check duplicate prefix */
          duplicate = 0;
          for (o6_p2 = o6_pstart; o6_p2 < o6_pcurrent;
               o6_p2 = OSPF6_NEXT_PREFIX (o6_p2))
            {
              if (! ospf6_prefix_issame (o6_p1, o6_p2))
                continue;

              duplicate = 1;
              o6_p2->prefix_options |= o6_p1->prefix_options;
            }
          if (duplicate)
            {
              if (IS_OSPF6_DUMP_LSA)
                zlog_info ("LSA Update Intra-Area-Prefix(Transit): "
                           "Duplicate %s", buf);
              o6_p1 = OSPF6_NEXT_PREFIX (o6_p1);
              continue;
            }

          /* copy prefix to new LSA */
          if (IS_OSPF6_DUMP_LSA)
            zlog_info ("LSA Update Intra-Area-Prefix(Transit): "
                       "including %s", buf);
          o6_pcurrent->prefix_length = o6_p1->prefix_length;
          o6_pcurrent->prefix_options = o6_p1->prefix_options;
          memcpy (o6_pcurrent + 1, o6_p1 + 1,
                  OSPF6_PREFIX_SPACE (o6_p1->prefix_length));

          size += OSPF6_PREFIX_SIZE (o6_pcurrent);
          o6_pcurrent = OSPF6_NEXT_PREFIX (o6_pcurrent);
          o6_p1 = OSPF6_NEXT_PREFIX (o6_p1);
          prefix_number ++;
        }
    }

  /* if no prefix to advertise, return */
  if (prefix_number == 0)
    {
      if (IS_OSPF6_DUMP_LSA)
        zlog_info ("LSA Update Intra-Area-Prefix(Transit): "
                   "No Prefix to advertise");
      return;
    }

  intra_prefix_lsa->prefix_number = htons (prefix_number);

  /* Fill LSA Header */
  ospf6_lsa_header_set (htons (OSPF6_LSA_TYPE_INTRA_PREFIX),
                        htonl (o6i->if_id),
                        o6i->area->ospf6->router_id,
                        (struct ospf6_lsa_header *) buffer, size);

  /* create LSA */
  lsa = ospf6_lsa_create ((struct ospf6_lsa_header *) buffer);
  lsa->refresh = thread_add_timer (master, ospf6_lsa_refresh, lsa,
                                   OSPF6_LS_REFRESH_TIME);
  lsa->scope = (void *) o6i->area;

  if (ospf6_lsa_is_really_reoriginate (lsa))
    {
      ospf6_dbex_remove_from_all_retrans_list (lsa);
      ospf6_dbex_flood (lsa, NULL);
      ospf6_lsdb_install (lsa);
    }
  else
    ospf6_lsa_delete (lsa);
}

/* RFC2740 3.4.3.7 Intra-Area-Prefix-LSA */
void
ospf6_lsa_update_intra_prefix_stub (u_int32_t area_id)
{
  char buffer [MAXLSASIZE];
  u_int16_t size;
  struct ospf6_lsa *lsa, *prev_lsa;
  struct ospf6_area *o6a;
  int count;

  struct ospf6_intra_area_prefix_lsa *intra_prefix_lsa;
  listnode i,j;
  struct ospf6_interface *o6i = NULL;
  struct ospf6_prefix *prefix_dst;
  struct connected *c;
  struct prefix_ipv6 prefix_ipv6;
  char buf[128];
  u_int16_t prefix_number;

  o6a = ospf6_area_lookup (area_id, ospf6);
  if (! o6a)
    {
      inet_ntop (AF_INET, &area_id, buffer, sizeof (buffer));
      zlog_warn ("LSA: Update Intra-Area-Prefix-LSA(Stub): "
                 "No such area: %s", buffer);
      return;
    }

  /* find previous LSA */
  prev_lsa = ospf6_lsdb_lookup (htons (OSPF6_LSA_TYPE_INTRA_PREFIX),
                                htonl (0),     /* xxx */
                                o6a->ospf6->router_id);

  /* prepare buffer */
  size = sizeof (struct ospf6_intra_area_prefix_lsa);
  memset (buffer, 0, sizeof (buffer));
  intra_prefix_lsa = (struct ospf6_intra_area_prefix_lsa *)
    (buffer + sizeof (struct ospf6_lsa_header));
  prefix_dst = (struct ospf6_prefix *)(intra_prefix_lsa + 1);
  prefix_number = 0;

  /* Examin for each interface */
  for (i = listhead (o6a->if_list); i; nextnode (i))
    {
      o6i = (struct ospf6_interface *) getdata (i);

      if (IS_OSPF6_DUMP_LSA)
        zlog_info ("LSA Update Intra-Area-Prefix(Stub): "
                   "Checking Interface %s", o6i->interface->name);

      if (o6i->state == IFS_DOWN)
        {
          if (IS_OSPF6_DUMP_LSA)
            zlog_info ("LSA Update Intra-Area-Prefix(Stub): "
                       "Interface %s down", o6i->interface->name);
          continue;
        }

      count = 0;
      o6i->foreach_nei (o6i, &count, NBS_FULL, ospf6_count_state);
      if (o6i->state != IFS_LOOPBACK && o6i->state != IFS_PTOP &&
          count != 0)
        {
          /* This interface's prefix will be included in DR's */
          if (IS_OSPF6_DUMP_LSA)
            zlog_info ("LSA Update Intra-Area-Prefix(Stub): "
                       "Interface %s is not stub", o6i->interface->name);
          continue;
        }

      /* copy foreach site-local or global prefix */
      for (j = listhead (o6i->interface->connected); j; nextnode (j))
        {
          c = (struct connected *) getdata (j);

          /* filter prefix not IPv6 */
          if (c->address->family != AF_INET6)
            continue;

          inet_ntop (AF_INET6, &c->address->u.prefix6, buf, sizeof (buf));

          if (IS_OSPF6_DUMP_LSA)
            zlog_info ("LSA Update Intra-Area-Prefix(Stub): "
                       "Checking %s", buf);

          /* filter linklocal prefix */
          if (IN6_IS_ADDR_LINKLOCAL (&c->address->u.prefix6))
            {
              if (IS_OSPF6_DUMP_LSA)
                zlog_info ("LSA Update Intra-Area-Prefix(Stub): "
                           "%s is Linklocal", buf);
              continue;
            }

          /* filter unspecified(default) prefix */
          if (IN6_IS_ADDR_UNSPECIFIED (&c->address->u.prefix6))
            {
              if (IS_OSPF6_DUMP_LSA)
                zlog_info ("LSA Update Intra-Area-Prefix(Stub): "
                           "%s is Unspecified", buf);
              continue;
            }

          /* filter loopback prefix */
          if (IN6_IS_ADDR_LOOPBACK (&c->address->u.prefix6))
            {
              if (IS_OSPF6_DUMP_LSA)
                zlog_info ("LSA Update Intra-Area-Prefix(Stub): "
                           "%s is Loopback", buf);
              continue;
            }

          /* filter IPv4 compatible prefix */
          if (IN6_IS_ADDR_V4COMPAT (&c->address->u.prefix6))
            {
              if (IS_OSPF6_DUMP_LSA)
                zlog_info ("LSA Update Intra-Area-Prefix(Stub): "
                           "%s is v4-Compatible", buf);
              continue;
            }

          /* filter IPv4 Mapped prefix */
          if (IN6_IS_ADDR_V4MAPPED (&c->address->u.prefix6))
            {
              if (IS_OSPF6_DUMP_LSA)
                zlog_info ("LSA Update Intra-Area-Prefix(Stub): "
                           "%s is v4-Mapped", buf);
              continue;
            }

          /* set ospf6 prefix according to its state */
          /* xxx, virtual links */
          prefix_copy ((struct prefix *) &prefix_ipv6, c->address);
          if (o6i->state == IFS_LOOPBACK || o6i->state == IFS_PTOP
              /* xxx, PoinToMultiPoint I/F type */ )
            {
              prefix_dst->prefix_length = 128;
              prefix_dst->prefix_options = OSPF6_PREFIX_OPTION_LA;
              prefix_dst->prefix_metric = htons (0);
              memcpy (prefix_dst + 1, &prefix_ipv6.prefix,
                      OSPF6_PREFIX_SPACE (prefix_dst->prefix_length));
            }
          else
            {
              /* apply mask */
              apply_mask_ipv6 (&prefix_ipv6);
              inet_ntop (AF_INET6, &c->address->u.prefix6, buf, sizeof (buf));

              prefix_dst->prefix_length = prefix_ipv6.prefixlen;
              prefix_dst->prefix_options = 0;  /* xxx, no options yet */
              prefix_dst->prefix_metric = htons (o6i->cost);
              memcpy (prefix_dst + 1, &prefix_ipv6.prefix,
                      OSPF6_PREFIX_SPACE (prefix_dst->prefix_length));
            }

          if (IS_OSPF6_DUMP_LSA)
            zlog_info ("LSA Update Intra-Area-Prefix(Stub): "
                       "include %s", buf);

          /* forward current buffer pointer */
          prefix_number ++;
          size += OSPF6_PREFIX_SIZE (prefix_dst);
          prefix_dst = OSPF6_NEXT_PREFIX (prefix_dst);
        }
    }

  /* If no prefix to advertise */
  if (prefix_number == 0)
    {
      if (IS_OSPF6_DUMP_LSA)
        zlog_info ("LSA Update Intra-Area-Prefix(Stub): "
                   "No prefix to advertise for area %s", o6a->str);
      if (prev_lsa)
        ospf6_lsa_premature_aging (prev_lsa);
      return;
    }

  if (IS_OSPF6_DUMP_LSA)
    zlog_info ("LSA Update Intra-Area-Prefix(Stub): Area %s", o6a->str);

  /* Set Referenced LSA field */
  intra_prefix_lsa->refer_lstype = htons (OSPF6_LSA_TYPE_ROUTER);
  intra_prefix_lsa->refer_lsid = htonl (0);
  intra_prefix_lsa->refer_advrtr = o6a->ospf6->router_id;

  intra_prefix_lsa->prefix_number = htons (prefix_number);

  /* Fill LSA Header */
  ospf6_lsa_header_set (htons (OSPF6_LSA_TYPE_INTRA_PREFIX),
                        htonl (0), /* xxx */
                        o6i->area->ospf6->router_id,
                        (struct ospf6_lsa_header *) buffer, size);

  /* create LSA */
  lsa = ospf6_lsa_create ((struct ospf6_lsa_header *) buffer);
  lsa->refresh = thread_add_timer (master, ospf6_lsa_refresh, lsa,
                                   OSPF6_LS_REFRESH_TIME);
  lsa->scope = (void *) o6a;

  if (ospf6_lsa_is_really_reoriginate (lsa))
    {
      ospf6_dbex_remove_from_all_retrans_list (lsa);
      ospf6_dbex_flood (lsa, NULL);
      ospf6_lsdb_install (lsa);
    }
  else
    ospf6_lsa_delete (lsa);
}

void
ospf6_lsa_update_link (char *ifname)
{
  char *cp, buffer [MAXLSASIZE];
  u_int16_t size;
  struct ospf6_lsa *lsa, *prev_lsa;
  struct interface *ifp;
  struct ospf6_interface *o6i;

  struct ospf6_link_lsa *llsa;
  struct ospf6_prefix *p;
  list prefix_connected;
  listnode n;
  struct connected *c;

  ifp = if_lookup_by_name (ifname);
  if (! ifp)
    {
      if (IS_OSPF6_DUMP_LSA)
        zlog_info ("LSA: Update Link-LSA: No such Interface: %s", ifname);
      return;
    }
  o6i = (struct ospf6_interface *) ifp->info;
  if (! o6i)
    {
      if (IS_OSPF6_DUMP_LSA)
        zlog_info ("LSA: Update Link-LSA: Interface not enabled: %s", ifname);
      return;
    }

  /* find previous LSA */
  prev_lsa = ospf6_lsdb_lookup (htons (OSPF6_LSA_TYPE_LINK),
                                htonl (o6i->if_id),
                                ospf6->router_id);

  /* can't make Link-LSA if linklocal address not set */
  if (! o6i->lladdr)
    {
      zlog_err ("LSA Update Link: No Linklocal Address: %s",
                o6i->interface->name);
      if (prev_lsa)
        ospf6_lsa_premature_aging (prev_lsa);
      return;
    }

  if (IS_OSPF6_DUMP_LSA)
    zlog_info ("LSA Update Link: Interface %s", o6i->interface->name);

  /* check connected prefix */
  prefix_connected = list_new ();
  for (n = listhead (o6i->interface->connected); n; nextnode (n))
    {
      c = (struct connected *) getdata (n);

      /* filter prefix not IPv6 */
      if (c->address->family != AF_INET6)
        continue;

      /* filter linklocal prefix */
      if (IN6_IS_ADDR_LINKLOCAL (&c->address->u.prefix6))
        continue;

      /* filter unspecified(default) prefix */
      if (IN6_IS_ADDR_UNSPECIFIED (&c->address->u.prefix6))
        continue;

      /* filter loopback prefix */
      if (IN6_IS_ADDR_LOOPBACK (&c->address->u.prefix6))
        continue;

      /* filter IPv4 compatible prefix */
      if (IN6_IS_ADDR_V4COMPAT (&c->address->u.prefix6))
        continue;

      /* filter IPv4 Mapped prefix */
      if (IN6_IS_ADDR_V4MAPPED (&c->address->u.prefix6))
        continue;

      /* hold prefix in list. duplicate is filtered in ospf6_prefix_add() */
      p = ospf6_prefix_make (0, 0, (struct prefix_ipv6 *) c->address);
      ospf6_prefix_add (prefix_connected, p);
    }

  /* Note: if no prefix configured, still we have to create Link-LSA
     for next-hop resolution */

  /* fill Link LSA and calculate size */
  size = sizeof (struct ospf6_link_lsa);
  memset (buffer, 0, sizeof (buffer));
  llsa = (struct ospf6_link_lsa *)
    (buffer + sizeof (struct ospf6_lsa_header));
  llsa->llsa_rtr_pri = o6i->priority;
  llsa->llsa_options[0] = o6i->area->options[0];
  llsa->llsa_options[1] = o6i->area->options[1];
  llsa->llsa_options[2] = o6i->area->options[2];

  /* linklocal address */
  memcpy (&llsa->llsa_linklocal, o6i->lladdr, sizeof (struct in6_addr));
#ifdef KAME /* clear ifindex */
  if (llsa->llsa_linklocal.s6_addr[3] & 0x0f)
    llsa->llsa_linklocal.s6_addr[3] &= ~((char)0x0f);
#endif /* KAME */

  llsa->llsa_prefix_num = htonl (listcount (prefix_connected));
  cp = (char *)(llsa + 1);
  for (n = listhead (prefix_connected); n; nextnode (n))
    {
      p = (struct ospf6_prefix *) getdata (n);
      size += OSPF6_PREFIX_SIZE (p);
      memcpy (cp, p, OSPF6_PREFIX_SIZE (p));
      cp += OSPF6_PREFIX_SIZE (p);
    }

  for (n = listhead (prefix_connected); n; nextnode (n))
    {
      p = (struct ospf6_prefix *) getdata (n);
      ospf6_prefix_free (p);
    }
  list_delete (prefix_connected);

  /* Fill LSA Header */
  ospf6_lsa_header_set (htons (OSPF6_LSA_TYPE_LINK), htonl (o6i->if_id),
                        o6i->area->ospf6->router_id,
                        (struct ospf6_lsa_header *) buffer, size);

  /* create LSA */
  lsa = ospf6_lsa_create ((struct ospf6_lsa_header *) buffer);
  lsa->refresh = thread_add_timer (master, ospf6_lsa_refresh, lsa,
                                   OSPF6_LS_REFRESH_TIME);
  lsa->scope = (void *) o6i;

  if (ospf6_lsa_is_really_reoriginate (lsa))
    {
      ospf6_dbex_remove_from_all_retrans_list (lsa);
      ospf6_dbex_flood (lsa, NULL);
      ospf6_lsdb_install (lsa);
    }
  else
    ospf6_lsa_delete (lsa);
}

#if 0
void
ospf6_lsa_update_inter_prefix (struct ospf6_area *o6a)
{
  char *cp, buffer [MAXLSASIZE];
  u_int16_t size;
  struct ospf6_lsa *lsa, *prev_lsa;

  struct ospf6_lsa_inter_prefix *inter_prefix;
  struct ospf6_prefix *o6p;

  /* find previous LSA */
  prev_lsa = ospf6_lsdb_lookup (htons (OSPF6_LSA_TYPE_INTER_PREFIX),
                                htonl (0), /* xxx */
                                ospf6->router_id);

  if (o6a->area_range.prefixlen == 0)
    {
      zlog_err ("LSA Update Inter-Area-Prefix: No Area range Configured for %s",
                o6a->str);
      if (prev_lsa)
        ospf6_lsa_premature_aging (prev_lsa);
      return;
    }

  if (IS_OSPF6_DUMP_LSA)
    zlog_info ("LSA Update Inter-Area-Prefix: Area %s", o6a->str);

  size = sizeof (struct ospf6_lsa_inter_prefix_lsa);
  memset (buffer, 0, sizeof (buffer));
  inter_prefix = (struct ospf6_lsa_inter_prefix_lsa *)
    (buffer + sizeof (struct ospf6_lsa_header));

  /* linklocal address */
  memcpy (&llsa->llsa_linklocal, o6i->lladdr, sizeof (struct in6_addr));
#ifdef KAME /* clear ifindex */
  if (llsa->llsa_linklocal.s6_addr[3] & 0x0f)
    llsa->llsa_linklocal.s6_addr[3] &= ~((char)0x0f);
#endif /* KAME */

  llsa->llsa_prefix_num = htonl (listcount (prefix_connected));
  cp = (char *)(llsa + 1);
  for (n = listhead (prefix_connected); n; nextnode (n))
    {
      p = (struct ospf6_prefix *) getdata (n);
      size += OSPF6_PREFIX_SIZE (p);
      memcpy (cp, p, OSPF6_PREFIX_SIZE (p));
      cp += OSPF6_PREFIX_SIZE (p);
    }

  /* Fill LSA Header */
  ospf6_lsa_header_set (htons (OSPF6_LSA_TYPE_LINK), htonl (o6i->if_id),
                        o6i->area->ospf6->router_id,
                        (struct ospf6_lsa_header *) buffer, size);

  /* create LSA */
  lsa = ospf6_lsa_create ((struct ospf6_lsa_header *) buffer);
  lsa->refresh = thread_add_timer (master, ospf6_lsa_refresh, lsa,
                                   OSPF6_LS_REFRESH_TIME);
  lsa->scope = (void *) o6i;

  if (ospf6_lsa_is_really_reoriginate (lsa))
    {
      ospf6_dbex_remove_from_all_retrans_list (lsa);
      ospf6_dbex_flood (lsa, NULL);
      ospf6_lsdb_install (lsa);
    }
  else
    ospf6_lsa_delete (lsa);
}

#endif

void
ospf6_lsa_reoriginate (struct ospf6_lsa *lsa)
{
  struct ospf6_lsa_header *lsa_header;

  struct ospf6 *o6;
  struct ospf6_area *o6a;
  struct ospf6_interface *o6i;

  lsa_header = (struct ospf6_lsa_header *) lsa->lsa_hdr;

  if (IS_OSPF6_DUMP_LSA)
    zlog_info ("Re-originate %s", lsa->str);

  switch (ntohs (lsa_header->type))
    {
      case OSPF6_LSA_TYPE_ROUTER:
        o6a = (struct ospf6_area *) lsa->scope;
        assert (o6a);
        ospf6_lsa_update_router (o6a->area_id);
        break;

      case OSPF6_LSA_TYPE_NETWORK:
        o6a = (struct ospf6_area *) lsa->scope;
        assert (o6a);
        o6i = ospf6_interface_lookup_by_index (ntohl (lsa_header->ls_id));
        if (o6i)
          ospf6_lsa_update_network (o6i->interface->name);
        else
          ospf6_lsa_premature_aging (lsa);
        break;

      case OSPF6_LSA_TYPE_INTRA_PREFIX:
        /* xxx, assume LS-ID has addressing semantics */
        o6a = (struct ospf6_area *) lsa->scope;
        o6i = NULL;
        assert (o6a);
        if (ntohl (lsa_header->ls_id) != 0)
          o6i = ospf6_interface_lookup_by_index (ntohl (lsa_header->ls_id));

        if (o6i)
          ospf6_lsa_update_intra_prefix_transit (o6i->interface->name);
        else if (ntohl (lsa_header->ls_id) == 0)
          ospf6_lsa_update_intra_prefix_stub (o6a->area_id);
        else
          ospf6_lsa_premature_aging (lsa);
        break;

      case OSPF6_LSA_TYPE_LINK:
        o6i = (struct ospf6_interface *) lsa->scope;
        assert (o6i);
        ospf6_lsa_update_link (o6i->interface->name);
        break;

      case OSPF6_LSA_TYPE_AS_EXTERNAL:
        o6 = (struct ospf6 *) lsa->scope;
        assert (o6);
        ospf6_lsa_update_as_external (ntohl (lsa_header->ls_id));
        break;

      default:
        break;
    }
}


/* ospf6_lsa expired */
int
ospf6_lsa_expire (struct thread *thread)
{
  struct ospf6_lsa *lsa;

  lsa = (struct ospf6_lsa *) THREAD_ARG (thread);
  assert (lsa && lsa->lsa_hdr);

  /* assertion */
  assert (ospf6_lsa_is_maxage (lsa));
  assert (!lsa->refresh);

  lsa->expire = (struct thread *) NULL;

  /* log */
  if (IS_OSPF6_DUMP_LSA)
    zlog_info ("LSA: Expire: %s", lsa->str);

  if (!lsa->summary)
    {
      /* reflood lsa */
      ospf6_dbex_flood (lsa, NULL);

      /* do not free LSA, and do nothing about lslists.
         wait event (ospf6_lsdb_check_maxage) */
    }

  return 0;
}

int
ospf6_lsa_refresh (struct thread *thread)
{
  struct ospf6_lsa *lsa;

  assert (thread);
  lsa = (struct ospf6_lsa *) THREAD_ARG  (thread);
  assert (lsa && lsa->lsa_hdr);

  /* this will be used later as flag to decide really originate */
  lsa->refresh = (struct thread *)NULL;

  /* log */
  if (IS_OSPF6_DUMP_LSA)
    {
      zlog_info ("LSA Refresh: %s", lsa->str);
    }

  ospf6_lsa_reoriginate (lsa);

  return 0;
}



/* enhanced Fletcher checksum algorithm, RFC1008 7.2 */
#define MODX                4102
#define LSA_CHECKSUM_OFFSET   15

unsigned short
ospf6_lsa_checksum (struct ospf6_lsa_header *lsa_header)
{
  u_char *sp, *ep, *p, *q;
  int c0 = 0, c1 = 0;
  int x, y;
  u_int16_t length;

  lsa_header->checksum = 0;
  length = ntohs (lsa_header->length) - 2;
  sp = (char *) &lsa_header->type;

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

  lsa_header->checksum = htons ((x << 8) + y);

  return (lsa_header->checksum);
}

u_int16_t
ospf6_lsa_get_scope_type (u_int16_t type)
{
  return (ntohs (type) & OSPF6_LSA_SCOPE_MASK);
}

int
ospf6_lsa_is_known_type (struct ospf6_lsa_header *lsa_header)
{
  if (lsa_header->type == htons (OSPF6_LSA_TYPE_ROUTER))
    return 1;
  if (lsa_header->type == htons (OSPF6_LSA_TYPE_NETWORK))
    return 1;
#if 0
  if (lsa_header->type == htons (OSPF6_LSA_TYPE_INTER_PREFIX))
    return 1;
  if (lsa_header->type == htons (OSPF6_LSA_TYPE_INTER_ROUTER))
    return 1;
#endif
  if (lsa_header->type == htons (OSPF6_LSA_TYPE_AS_EXTERNAL))
    return 1;
  if (lsa_header->type == htons (OSPF6_LSA_TYPE_LINK))
    return 1;
  if (lsa_header->type == htons (OSPF6_LSA_TYPE_INTRA_PREFIX))
    return 1;
  return 0;
}

