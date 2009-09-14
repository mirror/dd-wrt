/*
 *	BIRD -- OSPF
 *
 *	(c) 1999--2004 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "ospf.h"

void
flush_lsa(struct top_hash_entry *en, struct proto_ospf *po)
{
  struct proto *p = &po->proto;

  OSPF_TRACE(D_EVENTS,
	     "Going to remove node Type: %u, Id: %R, Rt: %R, Age: %u, SN: 0x%x",
	     en->lsa.type, en->lsa.id, en->lsa.rt, en->lsa.age, en->lsa.sn);
  s_rem_node(SNODE en);
  if (en->lsa_body != NULL)
    mb_free(en->lsa_body);
  en->lsa_body = NULL;
  ospf_hash_delete(po->gr, en);
}

/**
 * ospf_age
 * @po: ospf protocol
 *
 * This function is periodicaly invoked from ospf_disp(). It computes the new
 * age of all LSAs and old (@age is higher than %LSA_MAXAGE) LSAs are flushed
 * whenever possible. If an LSA originated by the router itself is older
 * than %LSREFRESHTIME a new instance is originated.
 *
 * The RFC says that a router should check the checksum of every LSA to detect
 * hardware problems. BIRD does not do this to minimalize CPU utilization.
 *
 * If routing table calculation is scheduled, it also invalidates the old routing
 * table calculation results.
 */
void
ospf_age(struct proto_ospf *po)
{
  struct proto *p = &po->proto;
  struct top_hash_entry *en, *nxt;
  int flush = can_flush_lsa(po);

  if (po->cleanup) OSPF_TRACE(D_EVENTS, "Running ospf_age cleanup");

  WALK_SLIST_DELSAFE(en, nxt, po->lsal)
  {
    if (po->cleanup)
    {
      en->color = OUTSPF;
      en->dist = LSINFINITY;
      en->nhi = NULL;
      en->nh = IPA_NONE;
      en->lb = IPA_NONE;
      DBG("Infinitying Type: %u, Id: %R, Rt: %R\n", en->lsa.type,
	  en->lsa.id, en->lsa.rt);
    }
    if (en->lsa.age == LSA_MAXAGE)
    {
      if (flush)
	flush_lsa(en, po);
      continue;
    }
    if ((en->lsa.rt == p->cf->global->router_id) &&(en->lsa.age >=
						    LSREFRESHTIME))
    {
      OSPF_TRACE(D_EVENTS, "Refreshing my LSA: Type: %u, Id: %R, Rt: %R",
		 en->lsa.type, en->lsa.id, en->lsa.rt);
      en->lsa.sn++;
      en->lsa.age = 0;
      en->inst_t = now;
      en->ini_age = 0;
      lsasum_calculate(&en->lsa, en->lsa_body);
      ospf_lsupd_flood(NULL, NULL, &en->lsa, NULL, en->oa, 1);
      continue;
    }
    if ((en->lsa.age = (en->ini_age + (now - en->inst_t))) >= LSA_MAXAGE)
    {
      if (flush)
      {
	flush_lsa(en, po);
	schedule_rtcalc(po);
      }
      else
	en->lsa.age = LSA_MAXAGE;
    }
  }
  po->cleanup = 0;
}

void
htonlsah(struct ospf_lsa_header *h, struct ospf_lsa_header *n)
{
  n->age = htons(h->age);
  n->options = h->options;
  n->type = h->type;
  n->id = htonl(h->id);
  n->rt = htonl(h->rt);
  n->sn = htonl(h->sn);
  n->checksum = htons(h->checksum);
  n->length = htons(h->length);
};

void
ntohlsah(struct ospf_lsa_header *n, struct ospf_lsa_header *h)
{
  h->age = ntohs(n->age);
  h->options = n->options;
  h->type = n->type;
  h->id = ntohl(n->id);
  h->rt = ntohl(n->rt);
  h->sn = ntohl(n->sn);
  h->checksum = ntohs(n->checksum);
  h->length = ntohs(n->length);
};

void
htonlsab(void *h, void *n, u8 type, u16 len)
{
  unsigned int i;
  switch (type)
  {
  case LSA_T_RT:
    {
      struct ospf_lsa_rt *hrt, *nrt;
      struct ospf_lsa_rt_link *hrtl, *nrtl;
      u16 links;

      nrt = n;
      hrt = h;
      links = hrt->links;

      nrt->veb.byte = hrt->veb.byte;
      nrt->padding = 0;
      nrt->links = htons(hrt->links);
      nrtl = (struct ospf_lsa_rt_link *) (nrt + 1);
      hrtl = (struct ospf_lsa_rt_link *) (hrt + 1);
      for (i = 0; i < links; i++)
      {
	(nrtl + i)->id = htonl((hrtl + i)->id);
	(nrtl + i)->data = htonl((hrtl + i)->data);
	(nrtl + i)->type = (hrtl + i)->type;
	(nrtl + i)->notos = (hrtl + i)->notos;
	(nrtl + i)->metric = htons((hrtl + i)->metric);
      }
      break;
    }
  case LSA_T_NET:
    {
      u32 *hid, *nid;

      nid = n;
      hid = h;

      for (i = 0; i < (len / sizeof(u32)); i++)
      {
	*(nid + i) = htonl(*(hid + i));
      }
      break;
    }
  case LSA_T_SUM_NET:
  case LSA_T_SUM_RT:
    {
      struct ospf_lsa_sum *hs, *ns;
      union ospf_lsa_sum_tm *hn, *nn;

      hs = h;
      ns = n;

      ns->netmask = hs->netmask;
      ipa_hton(ns->netmask);

      hn = (union ospf_lsa_sum_tm *) (hs + 1);
      nn = (union ospf_lsa_sum_tm *) (ns + 1);

      for (i = 0; i < ((len - sizeof(struct ospf_lsa_sum)) /
		       sizeof(union ospf_lsa_sum_tm)); i++)
      {
	(nn + i)->metric = htonl((hn + i)->metric);
      }
      break;
    }
  case LSA_T_EXT:
    {
      struct ospf_lsa_ext *he, *ne;
      struct ospf_lsa_ext_tos *ht, *nt;

      he = h;
      ne = n;

      ne->netmask = he->netmask;
      ipa_hton(ne->netmask);

      ht = (struct ospf_lsa_ext_tos *) (he + 1);
      nt = (struct ospf_lsa_ext_tos *) (ne + 1);

      for (i = 0; i < ((len - sizeof(struct ospf_lsa_ext)) /
		       sizeof(struct ospf_lsa_ext_tos)); i++)
      {
	(nt + i)->etm.metric = htonl((ht + i)->etm.metric);
	(nt + i)->fwaddr = (ht + i)->fwaddr;
	ipa_hton((nt + i)->fwaddr);
	(nt + i)->tag = htonl((ht + i)->tag);
      }
      break;
    }
  default:
    bug("(hton): Unknown LSA");
  }
};

void
ntohlsab(void *n, void *h, u8 type, u16 len)
{
  unsigned int i;
  switch (type)
  {
  case LSA_T_RT:
    {
      struct ospf_lsa_rt *hrt, *nrt;
      struct ospf_lsa_rt_link *hrtl, *nrtl;
      u16 links;

      nrt = n;
      hrt = h;

      hrt->veb.byte = nrt->veb.byte;
      hrt->padding = 0;
      links = hrt->links = ntohs(nrt->links);
      nrtl = (struct ospf_lsa_rt_link *) (nrt + 1);
      hrtl = (struct ospf_lsa_rt_link *) (hrt + 1);
      for (i = 0; i < links; i++)
      {
	(hrtl + i)->id = ntohl((nrtl + i)->id);
	(hrtl + i)->data = ntohl((nrtl + i)->data);
	(hrtl + i)->type = (nrtl + i)->type;
	(hrtl + i)->notos = (nrtl + i)->notos;
	(hrtl + i)->metric = ntohs((nrtl + i)->metric);
      }
      break;
    }
  case LSA_T_NET:
    {
      u32 *hid, *nid;

      hid = h;
      nid = n;

      for (i = 0; i < (len / sizeof(u32)); i++)
      {
	*(hid + i) = ntohl(*(nid + i));
      }
      break;
    }
  case LSA_T_SUM_NET:
  case LSA_T_SUM_RT:
    {
      struct ospf_lsa_sum *hs, *ns;
      union ospf_lsa_sum_tm *hn, *nn;

      hs = h;
      ns = n;

      hs->netmask = ns->netmask;
      ipa_ntoh(hs->netmask);

      hn = (union ospf_lsa_sum_tm *) (hs + 1);
      nn = (union ospf_lsa_sum_tm *) (ns + 1);

      for (i = 0; i < ((len - sizeof(struct ospf_lsa_sum)) /
		       sizeof(union ospf_lsa_sum_tm)); i++)
      {
	(hn + i)->metric = ntohl((nn + i)->metric);
      }
      break;
    }
  case LSA_T_EXT:
    {
      struct ospf_lsa_ext *he, *ne;
      struct ospf_lsa_ext_tos *ht, *nt;

      he = h;
      ne = n;

      he->netmask = ne->netmask;
      ipa_ntoh(he->netmask);

      ht = (struct ospf_lsa_ext_tos *) (he + 1);
      nt = (struct ospf_lsa_ext_tos *) (ne + 1);

      for (i = 0; i < ((len - sizeof(struct ospf_lsa_ext)) /
		       sizeof(struct ospf_lsa_ext_tos)); i++)
      {
	(ht + i)->etm.metric = ntohl((nt + i)->etm.metric);
	(ht + i)->fwaddr = (nt + i)->fwaddr;
	ipa_ntoh((ht + i)->fwaddr);
	(ht + i)->tag = ntohl((nt + i)->tag);
      }
      break;
    }
  default:
    bug("(ntoh): Unknown LSA");
  }
};

#define MODX 4102		/* larges signed value without overflow */

/* Fletcher Checksum -- Refer to RFC1008. */
#define MODX                 4102
#define LSA_CHECKSUM_OFFSET    15

/* FIXME This is VERY uneficient, I have huge endianity problems */
void
lsasum_calculate(struct ospf_lsa_header *h, void *body)
{
  u16 length;

  length = h->length;

  htonlsah(h, h);
  htonlsab(body, body, h->type, length - sizeof(struct ospf_lsa_header));

  (void) lsasum_check(h, body);

  ntohlsah(h, h);
  ntohlsab(body, body, h->type, length - sizeof(struct ospf_lsa_header));
}

/*
 * Note, that this function expects that LSA is in big endianity
 * It also returns value in big endian
 */
u16
lsasum_check(struct ospf_lsa_header *h, void *body)
{
  u8 *sp, *ep, *p, *q, *b;
  int c0 = 0, c1 = 0;
  int x, y;
  u16 length;

  b = body;
  sp = (char *) &h->options;
  length = ntohs(h->length) - 2;
  h->checksum = 0;

  for (ep = sp + length; sp < ep; sp = q)
  {				/* Actually MODX is very large, do we need the for-cyclus? */
    q = sp + MODX;
    if (q > ep)
      q = ep;
    for (p = sp; p < q; p++)
    {
      /* 
       * I count with bytes from header and than from body
       * but if there is no body, it's appended to header
       * (probably checksum in update receiving) and I go on
       * after header
       */
      if ((b == NULL) || (p < (u8 *) (h + 1)))
      {
	c0 += *p;
      }
      else
      {
	c0 += *(b + (p - sp) - sizeof(struct ospf_lsa_header) + 2);
      }

      c1 += c0;
    }
    c0 %= 255;
    c1 %= 255;
  }

  x = ((length - LSA_CHECKSUM_OFFSET) * c0 - c1) % 255;
  if (x <= 0)
    x += 255;
  y = 510 - c0 - x;
  if (y > 255)
    y -= 255;

  ((u8 *) & h->checksum)[0] = x;
  ((u8 *) & h->checksum)[1] = y;
  return h->checksum;
}

int
lsa_comp(struct ospf_lsa_header *l1, struct ospf_lsa_header *l2)
			/* Return codes from point of view of l1 */
{
  u32 sn1, sn2;

  sn1 = l1->sn - LSA_INITSEQNO + 1;
  sn2 = l2->sn - LSA_INITSEQNO + 1;

  if (sn1 > sn2)
    return CMP_NEWER;
  if (sn1 < sn2)
    return CMP_OLDER;

  if (l1->checksum != l2->checksum)
    return l1->checksum < l2->checksum ? CMP_OLDER : CMP_NEWER;

  if ((l1->age == LSA_MAXAGE) && (l2->age != LSA_MAXAGE))
    return CMP_NEWER;
  if ((l2->age == LSA_MAXAGE) && (l1->age != LSA_MAXAGE))
    return CMP_OLDER;

  if (ABS(l1->age - l2->age) > LSA_MAXAGEDIFF)
    return l1->age < l2->age ? CMP_NEWER : CMP_OLDER;

  return CMP_SAME;
}

/**
 * lsa_install_new - install new LSA into database
 * @lsa: LSA header
 * @body: pointer to LSA body
 * @oa: current ospf_area
 *
 * This function ensures installing new LSA into LSA database. Old instance is
 * replaced. Several actions are taken to detect if new routing table
 * calculation is necessary. This is described in 13.2 of RFC 2328.
 */
struct top_hash_entry *
lsa_install_new(struct ospf_lsa_header *lsa, void *body, struct ospf_area *oa)
{
  /* LSA can be temporarrily, but body must be mb_allocated. */
  int change = 0;
  unsigned i;
  struct top_hash_entry *en;
  struct proto_ospf *po = oa->po;

  if ((en = ospf_hash_find_header(po->gr, oa->areaid, lsa)) == NULL)
  {
    en = ospf_hash_get_header(po->gr, oa, lsa);
    change = 1;
  }
  else
  {
    if ((en->lsa.length != lsa->length) || (en->lsa.options != lsa->options)
	|| ((en->lsa.age == LSA_MAXAGE) || (lsa->age == LSA_MAXAGE)))
      change = 1;
    else
    {
      u8 *k = en->lsa_body, *l = body;
      for (i = 0; i < (lsa->length - sizeof(struct ospf_lsa_header)); i++)
      {
	if (*(k + i) != *(l + i))
	{
	  change = 1;
	  break;
	}
      }
    }
    s_rem_node(SNODE en);
  }

  DBG("Inst lsa: Id: %R, Rt: %R, Type: %u, Age: %u, Sum: %u, Sn: 0x%x\n",
      lsa->id, lsa->rt, lsa->type, lsa->age, lsa->checksum, lsa->sn);

  s_add_tail(&po->lsal, SNODE en);
  en->inst_t = now;
  if (en->lsa_body != NULL)
    mb_free(en->lsa_body);
  en->lsa_body = body;
  memcpy(&en->lsa, lsa, sizeof(struct ospf_lsa_header));
  en->ini_age = en->lsa.age;

  if (change)
  {
    schedule_rtcalc(po);
  }

  return en;
}
