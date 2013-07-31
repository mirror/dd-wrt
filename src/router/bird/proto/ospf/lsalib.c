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
	     "Going to remove LSA Type: %04x, Id: %R, Rt: %R, Age: %u, Seqno: 0x%x",
	     en->lsa.type, en->lsa.id, en->lsa.rt, en->lsa.age, en->lsa.sn);
  s_rem_node(SNODE en);
  if (en->lsa_body != NULL)
    mb_free(en->lsa_body);
  en->lsa_body = NULL;
  ospf_hash_delete(po->gr, en);
}

void
ospf_flush_area(struct proto_ospf *po, u32 areaid)
{
  struct top_hash_entry *en, *nxt;

  WALK_SLIST_DELSAFE(en, nxt, po->lsal)
  {
    if ((LSA_SCOPE(&en->lsa) == LSA_SCOPE_AREA) && (en->domain == areaid))
      flush_lsa(en, po);
  }
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

  WALK_SLIST_DELSAFE(en, nxt, po->lsal)
  {
    if (en->lsa.age == LSA_MAXAGE)
    {
      if (flush)
	flush_lsa(en, po);
      continue;
    }
    if ((en->lsa.rt == po->router_id) && (en->lsa.age >= LSREFRESHTIME))
    {
      OSPF_TRACE(D_EVENTS, "Refreshing my LSA: Type: %u, Id: %R, Rt: %R",
		 en->lsa.type, en->lsa.id, en->lsa.rt);
      en->lsa.sn++;
      en->lsa.age = 0;
      en->inst_t = now;
      en->ini_age = 0;
      lsasum_calculate(&en->lsa, en->lsa_body);
      ospf_lsupd_flood(po, NULL, NULL, &en->lsa, en->domain, 1);
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
}

#ifndef CPU_BIG_ENDIAN
void
htonlsah(struct ospf_lsa_header *h, struct ospf_lsa_header *n)
{
  n->age = htons(h->age);
#ifdef OSPFv2
  n->options = h->options;
#endif
  n->type = htont(h->type);
  n->id = htonl(h->id);
  n->rt = htonl(h->rt);
  n->sn = htonl(h->sn);
  n->checksum = htons(h->checksum);
  n->length = htons(h->length);
}

void
ntohlsah(struct ospf_lsa_header *n, struct ospf_lsa_header *h)
{
  h->age = ntohs(n->age);
#ifdef OSPFv2
  h->options = n->options;
#endif
  h->type = ntoht(n->type);
  h->id = ntohl(n->id);
  h->rt = ntohl(n->rt);
  h->sn = ntohl(n->sn);
  h->checksum = ntohs(n->checksum);
  h->length = ntohs(n->length);
}

void
htonlsab(void *h, void *n, u16 len)
{
  u32 *hid = h;
  u32 *nid = n;
  unsigned i;

  for (i = 0; i < (len / sizeof(u32)); i++)
    nid[i] = htonl(hid[i]);
}

void
ntohlsab(void *n, void *h, u16 len)
{
  u32 *nid = n;
  u32 *hid = h;
  unsigned i;

  for (i = 0; i < (len / sizeof(u32)); i++)
    hid[i] = ntohl(nid[i]);
}
#endif /* little endian */

/*
void
buf_dump(const char *hdr, const byte *buf, int blen)
{
  char b2[1024];
  char *bp;
  int first = 1;
  int i;

  const char *lhdr = hdr;

  bp = b2;
  for(i = 0; i < blen; i++)
    {
      if ((i > 0) && ((i % 16) == 0))
	{
	      *bp = 0;
	      log(L_WARN "%s\t%s", lhdr, b2);
	      lhdr = "";
	      bp = b2;
	}

      bp += snprintf(bp, 1022, "%02x ", buf[i]);

    }

  *bp = 0;
  log(L_WARN "%s\t%s", lhdr, b2);
}
*/

#define MODX 4102		/* larges signed value without overflow */

/* Fletcher Checksum -- Refer to RFC1008. */
#define MODX                 4102
#define LSA_CHECKSUM_OFFSET    15

/* FIXME This is VERY uneficient, I have huge endianity problems */
void
lsasum_calculate(struct ospf_lsa_header *h, void *body)
{
  u16 length = h->length;

  //  log(L_WARN "Checksum %R %R %d start (len %d)", h->id, h->rt, h->type, length);
  htonlsah(h, h);
  htonlsab1(body, length - sizeof(struct ospf_lsa_header));

  /*
  char buf[1024];
  memcpy(buf, h, sizeof(struct ospf_lsa_header));
  memcpy(buf + sizeof(struct ospf_lsa_header), body, length - sizeof(struct ospf_lsa_header));
  buf_dump("CALC", buf, length);
  */

  (void) lsasum_check(h, body);

  //  log(L_WARN "Checksum result %4x", h->checksum);

  ntohlsah(h, h);
  ntohlsab1(body, length - sizeof(struct ospf_lsa_header));
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
  sp = (char *) h;
  sp += 2; /* Skip Age field */
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
	c0 += *(b + (p - (u8 *) (h + 1)));
      }

      c1 += c0;
    }
    c0 %= 255;
    c1 %= 255;
  }

  x = (int)((length - LSA_CHECKSUM_OFFSET) * c0 - c1) % 255;
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

#define HDRLEN sizeof(struct ospf_lsa_header)

static int
lsa_validate_rt(struct ospf_lsa_header *lsa, struct ospf_lsa_rt *body)
{
  unsigned int i, max;

  if (lsa->length < (HDRLEN + sizeof(struct ospf_lsa_rt)))
    return 0;

  struct ospf_lsa_rt_link *rtl = (struct ospf_lsa_rt_link *) (body + 1);
  max = lsa_rt_count(lsa);

#ifdef OSPFv2
  if (body->links != max)
    return 0;
#endif  

  for (i = 0; i < max; i++)
  {
    u8 type = rtl[i].type;
    if (!((type == LSART_PTP) ||
	  (type == LSART_NET) ||
#ifdef OSPFv2
	  (type == LSART_STUB) ||
#endif
	  (type == LSART_VLNK)))
      return 0;
  }
  return 1;
}

static int
lsa_validate_net(struct ospf_lsa_header *lsa, struct ospf_lsa_net *body UNUSED)
{
  if (lsa->length < (HDRLEN + sizeof(struct ospf_lsa_net)))
    return 0;

  return 1;
}

#ifdef OSPFv2

static int
lsa_validate_sum(struct ospf_lsa_header *lsa, struct ospf_lsa_sum *body)
{
  if (lsa->length < (HDRLEN + sizeof(struct ospf_lsa_sum)))
    return 0;

  /* First field should have TOS = 0, we ignore other TOS fields */
  if ((body->metric & LSA_SUM_TOS) != 0)
    return 0;

  return 1;
}
#define lsa_validate_sum_net(A,B) lsa_validate_sum(A,B)
#define lsa_validate_sum_rt(A,B)  lsa_validate_sum(A,B)

static int
lsa_validate_ext(struct ospf_lsa_header *lsa, struct ospf_lsa_ext *body)
{
  if (lsa->length < (HDRLEN + sizeof(struct ospf_lsa_ext)))
    return 0;

  /* First field should have TOS = 0, we ignore other TOS fields */
  if ((body->metric & LSA_EXT_TOS) != 0)
    return 0;

  return 1;
}

#else /* OSPFv3 */

static inline int
pxlen(u32 *buf)
{
  return *buf >> 24;
}

static int
lsa_validate_sum_net(struct ospf_lsa_header *lsa, struct ospf_lsa_sum_net *body)
{
  if (lsa->length < (HDRLEN + sizeof(struct ospf_lsa_sum_net) + 4))
    return 0;

  u8 pxl = pxlen(body->prefix);
  if (pxl > MAX_PREFIX_LENGTH)
    return 0;

  if (lsa->length != (HDRLEN + sizeof(struct ospf_lsa_sum_net) + 
		      IPV6_PREFIX_SPACE(pxl)))
    return 0;

  return 1;
}


static int
lsa_validate_sum_rt(struct ospf_lsa_header *lsa, struct ospf_lsa_sum_rt *body)
{
  if (lsa->length != (HDRLEN + sizeof(struct ospf_lsa_sum_rt)))
    return 0;

  return 1;
}

static int
lsa_validate_ext(struct ospf_lsa_header *lsa, struct ospf_lsa_ext *body)
{
  if (lsa->length < (HDRLEN + sizeof(struct ospf_lsa_ext) + 4))
    return 0;

  u8 pxl = pxlen(body->rest);
  if (pxl > MAX_PREFIX_LENGTH)
    return 0;

  int len = IPV6_PREFIX_SPACE(pxl);
  if (body->metric & LSA_EXT_FBIT) // forwardinf address
    len += 16;
  if (body->metric & LSA_EXT_TBIT) // route tag
    len += 4;
  if (*body->rest & 0xFFFF) // referenced LS type field
    len += 4;

  if (lsa->length != (HDRLEN + sizeof(struct ospf_lsa_ext) + len))
    return 0;

  return 1;
}

static int
lsa_validate_pxlist(struct ospf_lsa_header *lsa, u32 pxcount, unsigned int offset, u8 *pbuf)
{
  unsigned int bound = lsa->length - HDRLEN - 4;
  u32 i;

  for (i = 0; i < pxcount; i++)
    {
      if (offset > bound)
	return 0;

      u8 pxl = pxlen((u32 *) (pbuf + offset));
      if (pxl > MAX_PREFIX_LENGTH)
	return 0;
  
      offset += IPV6_PREFIX_SPACE(pxl);
    }

  if (lsa->length != (HDRLEN + offset))
    return 0;

  return 1;
}

static int
lsa_validate_link(struct ospf_lsa_header *lsa, struct ospf_lsa_link *body)
{
  if (lsa->length < (HDRLEN + sizeof(struct ospf_lsa_link)))
    return 0;

  return lsa_validate_pxlist(lsa, body->pxcount, sizeof(struct ospf_lsa_link), (u8 *) body);
}

static int
lsa_validate_prefix(struct ospf_lsa_header *lsa, struct ospf_lsa_prefix *body)
{
  if (lsa->length < (HDRLEN + sizeof(struct ospf_lsa_prefix)))
    return 0;

  return lsa_validate_pxlist(lsa, body->pxcount, sizeof(struct ospf_lsa_prefix), (u8 *) body);
}

#endif


/**
 * lsa_validate - check whether given LSA is valid
 * @lsa: LSA header
 * @body: pointer to LSA body
 *
 * Checks internal structure of given LSA body (minimal length,
 * consistency). Returns true if valid.
 */

int
lsa_validate(struct ospf_lsa_header *lsa, void *body)
{
  switch (lsa->type)
    {
    case LSA_T_RT:
      return lsa_validate_rt(lsa, body);
    case LSA_T_NET:
      return lsa_validate_net(lsa, body);
    case LSA_T_SUM_NET:
      return lsa_validate_sum_net(lsa, body);
    case LSA_T_SUM_RT:
      return lsa_validate_sum_rt(lsa, body);
    case LSA_T_EXT:
    case LSA_T_NSSA:
      return lsa_validate_ext(lsa, body);
#ifdef OSPFv3
    case LSA_T_LINK:
      return lsa_validate_link(lsa, body);
    case LSA_T_PREFIX:
      return lsa_validate_prefix(lsa, body);
#endif
    default:
      /* In OSPFv3, unknown LSAs are OK,
	 In OSPFv2, unknown LSAs are already rejected
      */
      return 1;
    }
}

/**
 * lsa_install_new - install new LSA into database
 * @po: OSPF protocol
 * @lsa: LSA header
 * @domain: domain of LSA
 * @body: pointer to LSA body
 *
 * This function ensures installing new LSA into LSA database. Old instance is
 * replaced. Several actions are taken to detect if new routing table
 * calculation is necessary. This is described in 13.2 of RFC 2328.
 */
struct top_hash_entry *
lsa_install_new(struct proto_ospf *po, struct ospf_lsa_header *lsa, u32 domain, void *body)
{
  /* LSA can be temporarrily, but body must be mb_allocated. */
  int change = 0;
  struct top_hash_entry *en;

  if ((en = ospf_hash_find_header(po->gr, domain, lsa)) == NULL)
  {
    en = ospf_hash_get_header(po->gr, domain, lsa);
    change = 1;
  }
  else
  {
    if ((en->lsa.length != lsa->length)
#ifdef OSPFv2       
	|| (en->lsa.options != lsa->options)
#endif
	|| (en->lsa.age == LSA_MAXAGE)
	|| (lsa->age == LSA_MAXAGE)
	|| memcmp(en->lsa_body, body, lsa->length - sizeof(struct ospf_lsa_header)))
      change = 1;

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
    schedule_rtcalc(po);

  return en;
}
