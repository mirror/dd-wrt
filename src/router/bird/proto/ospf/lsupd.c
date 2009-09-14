/*
 *	BIRD -- OSPF
 *
 *	(c) 2000--2004 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "ospf.h"


void ospf_dump_lsahdr(struct proto *p, struct ospf_lsa_header *lsa_n)
{
  struct ospf_lsa_header lsa;
  ntohlsah(lsa_n, &lsa);

  log(L_TRACE "%s:     LSA      Id: %R, Rt: %R, Type: %u, Age: %u, Seqno: 0x%08x, Sum: %u",
      p->name, lsa.id, lsa.rt, lsa.type, lsa.age, lsa.sn, lsa.checksum);
}

void ospf_dump_common(struct proto *p, struct ospf_packet *op)
{
  log(L_TRACE "%s:     length   %d", p->name, ntohs(op->length));
  log(L_TRACE "%s:     router   %R", p->name, ntohl(op->routerid));
}

static void ospf_dump_lsupd(struct proto *p, struct ospf_lsupd_packet *pkt)
{
  struct ospf_packet *op = &pkt->ospf_packet;

  ASSERT(op->type == LSUPD_P);
  ospf_dump_common(p, op);

  u8 *pbuf= (u8 *) pkt;
  int offset = sizeof(struct ospf_lsupd_packet);
  int bound = ntohs(op->length) - sizeof(struct ospf_lsa_header);
  int i, j;

  j = ntohl(pkt->lsano);
  for (i = 0; i < j; i++)
    {
      if (offset > bound)
	{
	  log(L_TRACE "%s:     LSA      invalid", p->name);
	  return;
	}

      struct ospf_lsa_header *lsa = (void *) (pbuf + offset);
      ospf_dump_lsahdr(p, lsa);
      offset += ntohs(lsa->length);
    }
}

/**
 * ospf_lsupd_flood - send received or generated lsa to the neighbors
 * @n: neighbor than sent this lsa (or NULL if generated)
 * @hn: LSA header followed by lsa body in network endianity (may be NULL) 
 * @hh: LSA header in host endianity (must be filled)
 * @iff: interface which received this LSA (or NULL if LSA is generated)
 * @oa: ospf_area which is the LSA generated for
 * @rtl: add this LSA into retransmission list
 *
 * return value - was the LSA flooded back?
 */

int
ospf_lsupd_flood(struct ospf_neighbor *n, struct ospf_lsa_header *hn,
		 struct ospf_lsa_header *hh, struct ospf_iface *iff,
		 struct ospf_area *oa, int rtl)
{
  struct ospf_iface *ifa;
  struct ospf_neighbor *nn;
  struct top_hash_entry *en;
  struct proto_ospf *po = oa->po;
  struct proto *p = &po->proto;
  int ret, retval = 0;

  /* pg 148 */
  WALK_LIST(ifa, po->iface_list)
  {
    if (ifa->stub)
      continue;

    if (hh->type == LSA_T_EXT)
    {
      if (ifa->type == OSPF_IT_VLINK)
	continue;
      if (ifa->oa->stub)
	continue;
    }
    else
    {
      if (ifa->oa != oa)
        continue;
    }

    DBG("Wanted to flood LSA: Type: %u, ID: %R, RT: %R, SN: 0x%x, Age %u\n",
	hh->type, hh->id, hh->rt, hh->sn, hh->age);

    ret = 0;
    WALK_LIST(nn, ifa->neigh_list)
    {
      if (nn->state < NEIGHBOR_EXCHANGE)
	continue;
      if (nn->state < NEIGHBOR_FULL)
      {
	if ((en = ospf_hash_find_header(nn->lsrqh, nn->ifa->oa->areaid, hh)) != NULL)
	{
	  DBG("That LSA found in lsreq list for neigh %R\n", nn->rid);

	  switch (lsa_comp(hh, &en->lsa))
	  {
	  case CMP_OLDER:
	    continue;
	    break;
	  case CMP_SAME:
	    s_rem_node(SNODE en);
	    if (en->lsa_body != NULL)
	      mb_free(en->lsa_body);
	    en->lsa_body = NULL;
	    DBG("Removing from lsreq list for neigh %R\n", nn->rid);
	    ospf_hash_delete(nn->lsrqh, en);
	    if (EMPTY_SLIST(nn->lsrql))
	      ospf_neigh_sm(nn, INM_LOADDONE);
	    continue;
	    break;
	  case CMP_NEWER:
	    s_rem_node(SNODE en);
	    if (en->lsa_body != NULL)
	      mb_free(en->lsa_body);
	    en->lsa_body = NULL;
	    DBG("Removing from lsreq list for neigh %R\n", nn->rid);
	    ospf_hash_delete(nn->lsrqh, en);
	    if (EMPTY_SLIST(nn->lsrql))
	      ospf_neigh_sm(nn, INM_LOADDONE);
	    break;
	  default:
	    bug("Bug in lsa_comp?");
	  }
	}
      }

      if (nn == n)
	continue;

      if (rtl)
      {
	if ((en = ospf_hash_find_header(nn->lsrth, nn->ifa->oa->areaid, hh)) == NULL)
	{
	  en = ospf_hash_get_header(nn->lsrth, nn->ifa->oa, hh);
	}
	else
	{
	  s_rem_node(SNODE en);
	}
	s_add_tail(&nn->lsrtl, SNODE en);
	memcpy(&en->lsa, hh, sizeof(struct ospf_lsa_header));
	DBG("Adding that LSA for flood to %I\n", nn->ip);
      }
      else
      {
	if ((en = ospf_hash_find_header(nn->lsrth, nn->ifa->oa->areaid, hh)) != NULL)
	{
	  s_rem_node(SNODE en);
	  if (en->lsa_body != NULL)
	    mb_free(en->lsa_body);
	  en->lsa_body = NULL;
	  ospf_hash_delete(nn->lsrth, en);
	}
      }

      ret = 1;
    }

    if (ret == 0)
      continue;			/* pg 150 (2) */

    if (ifa == iff)
    {
      if ((n->rid == iff->drid) || n->rid == iff->bdrid)
	continue;		/* pg 150 (3) */
      if (iff->state == OSPF_IS_BACKUP)
	continue;		/* pg 150 (4) */
      retval = 1;
    }

    {
      sock *sk;
      u16 len, age;
      struct ospf_lsupd_packet *pk;
      struct ospf_packet *op;
      struct ospf_lsa_header *lh;

      if ((ifa->type == OSPF_IT_NBMA) || (ifa->type == OSPF_IT_VLINK))
	sk = ifa->ip_sk;
      else
	sk = ifa->hello_sk;

      pk = (struct ospf_lsupd_packet *) sk->tbuf;
      op = (struct ospf_packet *) sk->tbuf;

      ospf_pkt_fill_hdr(ifa, pk, LSUPD_P);
      pk->lsano = htonl(1);

      lh = (struct ospf_lsa_header *) (pk + 1);

      /* Copy LSA into the packet */
      if (hn)
      {
	memcpy(lh, hn, ntohs(hn->length));
      }
      else
      {
	u8 *help;
	struct top_hash_entry *en;

	htonlsah(hh, lh);
	help = (u8 *) (lh + 1);
	en = ospf_hash_find_header(po->gr, oa->areaid, hh);
	htonlsab(en->lsa_body, help, hh->type, hh->length
		 - sizeof(struct ospf_lsa_header));
      }

      len = sizeof(struct ospf_lsupd_packet) + ntohs(lh->length);

      age = ntohs(lh->age);
      age += ifa->inftransdelay;
      if (age > LSA_MAXAGE)
	age = LSA_MAXAGE;
      lh->age = htons(age);

      op->length = htons(len);

      OSPF_PACKET(ospf_dump_lsupd,  (struct ospf_lsupd_packet *) sk->tbuf,
		  "LSUPD packet flooded via %s", ifa->iface->name);

      switch (ifa->type)
      {
      case OSPF_IT_NBMA:
	if ((ifa->state == OSPF_IS_BACKUP) || (ifa->state == OSPF_IS_DR))
	  ospf_send_to_agt(sk, ifa, NEIGHBOR_EXCHANGE);
	else
	  ospf_send_to_bdr(sk, ifa);
	break;

      case OSPF_IT_VLINK:
	ospf_send_to(sk, ifa->vip, ifa);
	break;

      default:
	if ((ifa->state == OSPF_IS_BACKUP) || (ifa->state == OSPF_IS_DR) ||
	    (ifa->type == OSPF_IT_PTP))
	  ospf_send_to(sk, AllSPFRouters, ifa);
	else
	  ospf_send_to(sk, AllDRouters, ifa);
      }
    }
  }
  return retval;
}

void				/* I send all I received in LSREQ */
ospf_lsupd_send_list(struct ospf_neighbor *n, list * l)
{
  struct l_lsr_head *llsh;
  u16 len;
  u32 lsano;
  struct top_hash_entry *en;
  struct ospf_lsupd_packet *pk;
  struct ospf_packet *op;
  struct ospf_area *oa = n->ifa->oa;
  struct proto_ospf *po = oa->po;
  struct proto *p = &po->proto;
  void *pktpos;

  if (EMPTY_LIST(*l))
    return;

  pk = (struct ospf_lsupd_packet *) n->ifa->ip_sk->tbuf;
  op = (struct ospf_packet *) n->ifa->ip_sk->tbuf;

  DBG("LSupd: 1st packet\n");

  ospf_pkt_fill_hdr(n->ifa, pk, LSUPD_P);
  len = sizeof(struct ospf_lsupd_packet);
  lsano = 0;
  pktpos = (pk + 1);

  WALK_LIST(llsh, *l)
  {
    if ((en = ospf_hash_find(po->gr, oa->areaid, llsh->lsh.id, llsh->lsh.rt,
			     llsh->lsh.type)) == NULL)
      continue;			/* Probably flushed LSA */
    /* FIXME This is a bug! I cannot flush LSA that is in lsrt */

    DBG("Sending LSA: Type=%u, ID=%R, RT=%R, SN: 0x%x, Age: %u\n",
	llsh->lsh.type, llsh->lsh.id, llsh->lsh.rt, en->lsa.sn, en->lsa.age);
    if (((u32) (len + en->lsa.length)) > ospf_pkt_maxsize(n->ifa))
    {
      pk->lsano = htonl(lsano);
      op->length = htons(len);

      OSPF_PACKET(ospf_dump_lsupd,  (struct ospf_lsupd_packet *) n->ifa->ip_sk->tbuf,
		  "LSUPD packet sent to %I via %s", n->ip, n->ifa->iface->name);
      ospf_send_to(n->ifa->ip_sk, n->ip, n->ifa);

      DBG("LSupd: next packet\n");
      ospf_pkt_fill_hdr(n->ifa, pk, LSUPD_P);
      len = sizeof(struct ospf_lsupd_packet);
      lsano = 0;
      pktpos = (pk + 1);
    }
    htonlsah(&(en->lsa), pktpos);
    pktpos = pktpos + sizeof(struct ospf_lsa_header);
    htonlsab(en->lsa_body, pktpos, en->lsa.type, en->lsa.length
	     - sizeof(struct ospf_lsa_header));
    pktpos = pktpos + en->lsa.length - sizeof(struct ospf_lsa_header);
    len += en->lsa.length;
    lsano++;
  }
  if (lsano > 0)
  {
    pk->lsano = htonl(lsano);
    op->length = htons(len);

    OSPF_PACKET(ospf_dump_lsupd,  (struct ospf_lsupd_packet *) n->ifa->ip_sk->tbuf,
		"LSUPD packet sent to %I via %s", n->ip, n->ifa->iface->name);
    ospf_send_to(n->ifa->ip_sk, n->ip, n->ifa);
  }
}

void
ospf_lsupd_receive(struct ospf_lsupd_packet *ps,
		   struct ospf_iface *ifa, struct ospf_neighbor *n)
{
  u32 area;
  struct ospf_neighbor *ntmp;
  struct ospf_lsa_header *lsa;
  struct ospf_area *oa;
  struct proto_ospf *po = ifa->oa->po;
  struct proto *p = &po->proto;
  unsigned int i, sendreq = 1, size = ntohs(ps->ospf_packet.length);

  OSPF_PACKET(ospf_dump_lsupd, ps, "LSUPD packet received from %I via %s", n->ip, ifa->iface->name);

  if (n->state < NEIGHBOR_EXCHANGE)
  {
    OSPF_TRACE(D_PACKETS, "Received lsupd in lesser state than EXCHANGE from (%I)", n->ip);
    return;
  }

  if (size <=
      (sizeof(struct ospf_lsupd_packet) + sizeof(struct ospf_lsa_header)))
  {
    log(L_WARN "Received lsupd from %I is too short!", n->ip);
    return;
  }

  ospf_neigh_sm(n, INM_HELLOREC);	/* Questionable */

  lsa = (struct ospf_lsa_header *) (ps + 1);
  area = htonl(ps->ospf_packet.areaid);
  oa = ospf_find_area((struct proto_ospf *) p, area);

  for (i = 0; i < ntohl(ps->lsano); i++,
       lsa = (struct ospf_lsa_header *) (((u8 *) lsa) + ntohs(lsa->length)))
  {
    struct ospf_lsa_header lsatmp;
    struct top_hash_entry *lsadb;
    unsigned diff = ((u8 *) lsa) - ((u8 *) ps), lenn = ntohs(lsa->length);
    u16 chsum;

    if (((diff + sizeof(struct ospf_lsa_header)) >= size)
	|| ((lenn + diff) > size))
    {
      log(L_WARN "Received lsupd from %I is too short!", n->ip);
      ospf_neigh_sm(n, INM_BADLSREQ);
      break;
    }

    if ((lenn <= sizeof(struct ospf_lsa_header))
	|| (lenn != (4 * (lenn / 4))))
    {
      log(L_WARN "Received LSA from %I with bad length", n->ip);
      ospf_neigh_sm(n, INM_BADLSREQ);
      break;
    }

    /* pg 143 (1) */
    chsum = lsa->checksum;
    if (chsum != lsasum_check(lsa, NULL))
    {
      log(L_WARN "Received bad lsa checksum from %I", n->ip);
      continue;
    }

    /* pg 143 (2) */
    if ((lsa->type < LSA_T_RT) || (lsa->type > LSA_T_EXT))
    {
      log(L_WARN "Unknown LSA type from %I", n->ip);
      continue;
    }

    /* pg 143 (3) */
    if ((lsa->type == LSA_T_EXT) && oa->stub)
    {
      log(L_WARN "Received External LSA in stub area from %I", n->ip);
      continue;
    }

    ntohlsah(lsa, &lsatmp);

    DBG("Update Type: %u ID: %R RT: %R, Sn: 0x%08x Age: %u, Sum: %u\n",
	lsatmp.type, lsatmp.id, lsatmp.rt, lsatmp.sn, lsatmp.age, lsatmp.checksum);

    lsadb = ospf_hash_find_header(po->gr, oa->areaid, &lsatmp);

#ifdef LOCAL_DEBUG
    if (lsadb)
      DBG("I have Type: %u ID: %R RT: %R, Sn: 0x%08x Age: %u, Sum: %u\n",
	  lsadb->lsa.type, lsadb->lsa.id, lsadb->lsa.rt,
	  lsadb->lsa.sn, lsadb->lsa.age, lsadb->lsa.checksum);
#endif

    /* pg 143 (4) */
    if ((lsatmp.age == LSA_MAXAGE) && (lsadb == NULL) && can_flush_lsa(po))
    {
      ospf_lsack_enqueue(n, lsa, ACKL_DIRECT);
      continue;
    }

    /* pg 144 (5) */
    if ((lsadb == NULL) || (lsa_comp(&lsatmp, &lsadb->lsa) == CMP_NEWER))
    {
      struct ospf_iface *ift = NULL;
      void *body;
      struct ospf_iface *nifa;
      int self = (lsatmp.rt == p->cf->global->router_id);

      DBG("PG143(5): Received LSA is newer\n");

      /* pg 145 (5f) - premature aging of self originated lsa */
      if ((!self) && (lsatmp.type == LSA_T_NET))
      {
	WALK_LIST(nifa, po->iface_list)
	{
	  if (!nifa->iface)
	    continue;
	  if (ipa_equal(nifa->iface->addr->ip, ipa_from_u32(lsatmp.id)))
	  {
	    self = 1;
	    break;
	  }
	}
      }

      if (self)
      {
	struct top_hash_entry *en;

	if ((lsatmp.age == LSA_MAXAGE) && (lsatmp.sn == LSA_MAXSEQNO))
	{
	  ospf_lsack_enqueue(n, lsa, ACKL_DIRECT);
	  continue;
	}

	lsatmp.age = LSA_MAXAGE;
	lsatmp.sn = LSA_MAXSEQNO;
	lsa->age = htons(LSA_MAXAGE);
	lsa->sn = htonl(LSA_MAXSEQNO);
	OSPF_TRACE(D_EVENTS, "Premature aging self originated lsa.");
	OSPF_TRACE(D_EVENTS, "Type: %d, Id: %R, Rt: %R",
		   lsatmp.type, lsatmp.id, lsatmp.rt);
	lsasum_check(lsa, (lsa + 1));	/* It also calculates chsum! */
	lsatmp.checksum = ntohs(lsa->checksum);
	ospf_lsupd_flood(NULL, lsa, &lsatmp, NULL, oa, 0);
	if (en = ospf_hash_find_header(po->gr, oa->areaid, &lsatmp))
	{
	  ospf_lsupd_flood(NULL, NULL, &en->lsa, NULL, oa, 1);
	}
	continue;
      }

      /* pg 144 (5a) */
      if (lsadb && ((now - lsadb->inst_t) <= MINLSARRIVAL))	/* FIXME: test for flooding? */
      {
	DBG("I got it in less that MINLSARRIVAL\n");
	sendreq = 0;
	continue;
      }

      if (ospf_lsupd_flood(n, lsa, &lsatmp, ifa, ifa->oa, 1) == 0)
      {
	DBG("Wasn't flooded back\n");	/* ps 144(5e), pg 153 */
	if (ifa->state == OSPF_IS_BACKUP)
	{
	  if (ifa->drid == n->rid)
	    ospf_lsack_enqueue(n, lsa, ACKL_DELAY);
	}
	else
	  ospf_lsack_enqueue(n, lsa, ACKL_DELAY);
      }

      /* Remove old from all ret lists */
      /* pg 144 (5c) */
      if (lsadb)
	WALK_LIST(ift, po->iface_list)
	  WALK_LIST(ntmp, ift->neigh_list)
      {
	struct top_hash_entry *en;
	if (ntmp->state > NEIGHBOR_EXSTART)
	  if ((en = ospf_hash_find_header(ntmp->lsrth, ntmp->ifa->oa->areaid, &lsadb->lsa)) != NULL)
	  {
	    s_rem_node(SNODE en);
	    if (en->lsa_body != NULL)
	      mb_free(en->lsa_body);
	    en->lsa_body = NULL;
	    ospf_hash_delete(ntmp->lsrth, en);
	  }
      }

      if ((lsatmp.age == LSA_MAXAGE) && (lsatmp.sn == LSA_MAXSEQNO)
	  && lsadb && can_flush_lsa(po))
      {
	flush_lsa(lsadb, po);
	schedule_rtcalc(po);
	continue;
      }				/* FIXME lsack? */

      /* pg 144 (5d) */
      body =
	mb_alloc(p->pool, lsatmp.length - sizeof(struct ospf_lsa_header));
      ntohlsab(lsa + 1, body, lsatmp.type,
	       lsatmp.length - sizeof(struct ospf_lsa_header));
      lsadb = lsa_install_new(&lsatmp, body, oa);
      DBG("New LSA installed in DB\n");

      continue;
    }

    /* FIXME pg145 (6) */

    /* pg145 (7) */
    if (lsa_comp(&lsatmp, &lsadb->lsa) == CMP_SAME)
    {
      struct top_hash_entry *en;
      DBG("PG145(7) Got the same LSA\n");
      if ((en = ospf_hash_find_header(n->lsrth, n->ifa->oa->areaid, &lsadb->lsa)) != NULL)
      {
	/* pg145 (7a) */
	s_rem_node(SNODE en);
	if (en->lsa_body != NULL)
	  mb_free(en->lsa_body);
	en->lsa_body = NULL;
	ospf_hash_delete(n->lsrth, en);
	if (ifa->state == OSPF_IS_BACKUP)
	{
	  if (n->rid == ifa->drid)
	    ospf_lsack_enqueue(n, lsa, ACKL_DELAY);
	}
      }
      else
      {
	/* pg145 (7b) */
	ospf_lsack_enqueue(n, lsa, ACKL_DIRECT);
      }
      sendreq = 0;
      continue;
    }

    /* pg145 (8) */
    if ((lsadb->lsa.age == LSA_MAXAGE) && (lsadb->lsa.sn == LSA_MAXSEQNO))
    {
      continue;
    }

    {
      list l;
      struct l_lsr_head ll;
      init_list(&l);
      ll.lsh.id = lsadb->lsa.id;
      ll.lsh.rt = lsadb->lsa.rt;
      ll.lsh.type = lsadb->lsa.type;
      add_tail(&l, NODE & ll);
      ospf_lsupd_send_list(n, &l);
    }
  }

  /* Send direct LSAs */
  ospf_lsack_send(n, ACKL_DIRECT);

  if (sendreq && (n->state == NEIGHBOR_LOADING))
  {
    ospf_lsreq_send(n);		/* Ask for another part of neighbor's database */
  }
}

void
ospf_lsupd_flush_nlsa(struct top_hash_entry *en, struct ospf_area *oa)
{
  struct ospf_lsa_header *lsa = &en->lsa;
  struct proto_ospf *po = oa->po;
  struct proto *p = &po->proto;

  lsa->age = LSA_MAXAGE;
  lsa->sn = LSA_MAXSEQNO;
  lsasum_calculate(lsa, en->lsa_body);
  OSPF_TRACE(D_EVENTS, "Premature aging self originated lsa!");
  OSPF_TRACE(D_EVENTS, "Type: %d, Id: %R, Rt: %R", lsa->type, lsa->id, lsa->rt);
  ospf_lsupd_flood(NULL, NULL, lsa, NULL, oa, 0);
}
