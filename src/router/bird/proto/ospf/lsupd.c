/*
 *	BIRD -- OSPF
 *
 *	(c) 2000--2004 Ondrej Filip <feela@network.cz>
 *	(c) 2009--2014 Ondrej Zajicek <santiago@crfreenet.org>
 *	(c) 2009--2014 CZ.NIC z.s.p.o.
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "ospf.h"


/*
struct ospf_lsupd_packet
{
  struct ospf_packet hdr;
  // union ospf_auth auth;

  u32 lsa_count;
  void lsas[];
};
*/


void
ospf_dump_lsahdr(struct ospf_proto *p, struct ospf_lsa_header *lsa_n)
{
  struct ospf_lsa_header lsa;
  u32 lsa_etype;

  lsa_ntoh_hdr(lsa_n, &lsa);
  lsa_etype = lsa_get_etype(&lsa, p);

  log(L_TRACE "%s:     LSA      Type: %04x, Id: %R, Rt: %R, Seq: %08x, Age: %u, Sum: %04x",
      p->p.name, lsa_etype, lsa.id, lsa.rt, lsa.sn, lsa.age, lsa.checksum);
}

void
ospf_dump_common(struct ospf_proto *p, struct ospf_packet *pkt)
{
  log(L_TRACE "%s:     length   %d", p->p.name, ntohs(pkt->length));
  log(L_TRACE "%s:     router   %R", p->p.name, ntohl(pkt->routerid));
}

static inline uint
ospf_lsupd_hdrlen(struct ospf_proto *p)
{
  return ospf_pkt_hdrlen(p) + 4; /* + u32 lsa count field */
}

static inline u32
ospf_lsupd_get_lsa_count(struct ospf_packet *pkt, uint hdrlen)
{
  u32 *c = ((void *) pkt) + hdrlen - 4;
  return ntohl(*c);
}

static inline void
ospf_lsupd_set_lsa_count(struct ospf_packet *pkt, uint hdrlen, u32 val)
{
  u32 *c = ((void *) pkt) + hdrlen - 4;
  *c = htonl(val);
}

static inline void
ospf_lsupd_body(struct ospf_proto *p, struct ospf_packet *pkt,
		uint *offset, uint *lsa_count)
{
  uint hlen = ospf_lsupd_hdrlen(p);
  *offset = hlen;
  *lsa_count = ospf_lsupd_get_lsa_count(pkt, hlen);
}

static void
ospf_dump_lsupd(struct ospf_proto *p, struct ospf_packet *pkt)
{
  uint offset, plen, i, lsa_count, lsa_len;

  ASSERT(pkt->type == LSUPD_P);
  ospf_dump_common(p, pkt);

  plen = ntohs(pkt->length);
  ospf_lsupd_body(p, pkt, &offset, &lsa_count);
  for (i = 0; i < lsa_count; i++)
  {
    if ((offset + sizeof(struct ospf_lsa_header)) > plen)
      goto invalid;

    struct ospf_lsa_header *lsa = ((void *) pkt) + offset;
    lsa_len = ntohs(lsa->length);

    if (((lsa_len % 4) != 0) || (lsa_len <= sizeof(struct ospf_lsa_header)))
      goto invalid;

    ospf_dump_lsahdr(p, lsa);
    offset += lsa_len;
  }
  return;

invalid:
  log(L_TRACE "%s:     LSA      invalid", p->p.name);
  return;
}


static inline void
ospf_lsa_lsrq_down(struct top_hash_entry *req, struct ospf_neighbor *n)
{
  if (req == n->lsrqi)
    n->lsrqi = SNODE_NEXT(req);

  s_rem_node(SNODE req);
  ospf_hash_delete(n->lsrqh, req);

  if (EMPTY_SLIST(n->lsrql))
  {
    tm_stop(n->lsrq_timer);

    if (n->state == NEIGHBOR_LOADING)
      ospf_neigh_sm(n, INM_LOADDONE);
  }
}

static inline void
ospf_lsa_lsrt_up(struct top_hash_entry *en, struct ospf_neighbor *n)
{
  struct top_hash_entry *ret = ospf_hash_get_entry(n->lsrth, en);

  if (!SNODE_VALID(ret))
  {
    en->ret_count++;
    s_add_tail(&n->lsrtl, SNODE ret);
  }

  ret->lsa = en->lsa;
  ret->lsa_body = LSA_BODY_DUMMY;

  if (!tm_active(n->lsrt_timer))
    tm_start(n->lsrt_timer, n->ifa->rxmtint);
}

void
ospf_lsa_lsrt_down_(struct top_hash_entry *en, struct ospf_neighbor *n, struct top_hash_entry *ret)
{
  if (en)
    en->ret_count--;

  s_rem_node(SNODE ret);
  ospf_hash_delete(n->lsrth, ret);

  if (EMPTY_SLIST(n->lsrtl))
    tm_stop(n->lsrt_timer);
}

static inline int
ospf_lsa_lsrt_down(struct top_hash_entry *en, struct ospf_neighbor *n)
{
  struct top_hash_entry *ret = ospf_hash_find_entry(n->lsrth, en);

  if (ret)
    ospf_lsa_lsrt_down_(en, n, ret);

  return ret != NULL;
}

void
ospf_add_flushed_to_lsrt(struct ospf_proto *p, struct ospf_neighbor *n)
{
  struct top_hash_entry *en;

  WALK_SLIST(en, p->lsal)
    if ((en->lsa.age == LSA_MAXAGE) && (en->lsa_body != NULL) &&
	lsa_flooding_allowed(en->lsa_type, en->domain, n->ifa))
      ospf_lsa_lsrt_up(en, n);

  /* If we found any flushed LSA, we send them ASAP */
  if (tm_active(n->lsrt_timer))
    tm_start(n->lsrt_timer, 0);
}

static int ospf_flood_lsupd(struct ospf_proto *p, struct top_hash_entry **lsa_list, uint lsa_count, uint lsa_min_count, struct ospf_iface *ifa);

static void
ospf_enqueue_lsa(struct ospf_proto *p, struct top_hash_entry *en, struct ospf_iface *ifa)
{
  if (ifa->flood_queue_used == ifa->flood_queue_size)
  {
    /* If we already have full queue, we send some packets */
    uint sent = ospf_flood_lsupd(p, ifa->flood_queue, ifa->flood_queue_used, ifa->flood_queue_used / 2, ifa);
    uint i;

    for (i = 0; i < sent; i++)
      ifa->flood_queue[i]->ret_count--;

    ifa->flood_queue_used -= sent;
    memmove(ifa->flood_queue, ifa->flood_queue + sent, ifa->flood_queue_used * sizeof(void *));
    bzero(ifa->flood_queue + ifa->flood_queue_used, sent * sizeof(void *));
  }

  en->ret_count++;
  ifa->flood_queue[ifa->flood_queue_used] = en;
  ifa->flood_queue_used++;

  if (!ev_active(p->flood_event))
    ev_schedule(p->flood_event);
}

void
ospf_flood_event(void *ptr)
{
  struct ospf_proto *p = ptr;
  struct ospf_iface *ifa;
  int i, count;

  WALK_LIST(ifa, p->iface_list)
  {
    if (ifa->flood_queue_used == 0)
      continue;

    count = ifa->flood_queue_used;
    ospf_flood_lsupd(p, ifa->flood_queue, count, count, ifa);

    for (i = 0; i < count; i++)
      ifa->flood_queue[i]->ret_count--;

    ifa->flood_queue_used = 0;
    bzero(ifa->flood_queue, count * sizeof(void *));
  }
}


/**
 * ospf_flood_lsa - send LSA to the neighbors
 * @p: OSPF protocol instance
 * @en: LSA entry
 * @from: neighbor than sent this LSA (or NULL if LSA is local)
 *
 * return value - was the LSA flooded back?
 */
int
ospf_flood_lsa(struct ospf_proto *p, struct top_hash_entry *en, struct ospf_neighbor *from)
{
  struct ospf_iface *ifa;
  struct ospf_neighbor *n;

  /* RFC 2328 13.3 */

  int back = 0;
  WALK_LIST(ifa, p->iface_list)
  {
    if (ifa->stub)
      continue;

    if (! lsa_flooding_allowed(en->lsa_type, en->domain, ifa))
      continue;

    DBG("Wanted to flood LSA: Type: %u, ID: %R, RT: %R, SN: 0x%x, Age %u\n",
	hh->type, hh->id, hh->rt, hh->sn, hh->age);

    int used = 0;
    WALK_LIST(n, ifa->neigh_list)
    {
      /* 13.3 (1a) */
      if (n->state < NEIGHBOR_EXCHANGE)
	continue;

      /* 13.3 (1b) */
      if (n->state < NEIGHBOR_FULL)
      {
	struct top_hash_entry *req = ospf_hash_find_entry(n->lsrqh, en);
	if (req != NULL)
	{
	  int cmp = lsa_comp(&en->lsa, &req->lsa);

	  /* If same or newer, remove LSA from the link state request list */
	  if (cmp > CMP_OLDER)
	    ospf_lsa_lsrq_down(req, n);

	  /* If older or same, skip processing of this neighbor */
	  if (cmp < CMP_NEWER)
	    continue;
	}
      }

      /* 13.3 (1c) */
      if (n == from)
	continue;

      /* In OSPFv3, there should be check whether receiving router understand
	 that type of LSA (for LSA types with U-bit == 0). But as we do not support
	 any optional LSA types, this is not needed yet */

      /* 13.3 (1d) - add LSA to the link state retransmission list */
      ospf_lsa_lsrt_up(en, n);

      used = 1;
    }

    /* 13.3 (2) */
    if (!used)
      continue;

    if (from && (from->ifa == ifa))
    {
      /* 13.3 (3) */
      if ((from->rid == ifa->drid) || (from->rid == ifa->bdrid))
	continue;

      /* 13.3 (4) */
      if (ifa->state == OSPF_IS_BACKUP)
	continue;

      back = 1;
    }

    /* 13.3 (5) - finally flood the packet */
    ospf_enqueue_lsa(p, en, ifa);
  }

  return back;
}

static uint
ospf_prepare_lsupd(struct ospf_proto *p, struct ospf_iface *ifa,
		   struct top_hash_entry **lsa_list, uint lsa_count)
{
  struct ospf_packet *pkt;
  uint hlen, pos, i, maxsize;

  pkt = ospf_tx_buffer(ifa);
  hlen = ospf_lsupd_hdrlen(p);
  maxsize = ospf_pkt_maxsize(ifa);

  ospf_pkt_fill_hdr(ifa, pkt, LSUPD_P);
  pos = hlen;

  for (i = 0; i < lsa_count; i++)
  {
    struct top_hash_entry *en = lsa_list[i];
    uint len = en->lsa.length;

    if ((pos + len) > maxsize)
    {
      /* The packet if full, stop adding LSAs and sent it */
      if (i > 0)
	break;

      /* LSA is larger than MTU, check buffer size */
      if (ospf_iface_assure_bufsize(ifa, pos + len) < 0)
      {
	/* Cannot fit in a tx buffer, skip that */
	log(L_ERR "%s: LSA too large to send on %s (Type: %04x, Id: %R, Rt: %R)",
	    p->p.name, ifa->ifname, en->lsa_type, en->lsa.id, en->lsa.rt);
	break;
      }

      /* TX buffer could be reallocated */
      pkt = ospf_tx_buffer(ifa);
    }

    struct ospf_lsa_header *buf = ((void *) pkt) + pos;
    lsa_hton_hdr(&en->lsa, buf);
    lsa_hton_body(en->lsa_body, ((void *) buf) + sizeof(struct ospf_lsa_header),
		  len - sizeof(struct ospf_lsa_header));
    buf->age = htons(MIN(en->lsa.age + ifa->inftransdelay, LSA_MAXAGE));

    pos += len;
  }

  ospf_lsupd_set_lsa_count(pkt, hlen, i);
  pkt->length = htons(pos);

  return i;
}


static int
ospf_flood_lsupd(struct ospf_proto *p, struct top_hash_entry **lsa_list, uint lsa_count, uint lsa_min_count, struct ospf_iface *ifa)
{
  uint i, c;

  for (i = 0; i < lsa_min_count; i += c)
  {
    c = ospf_prepare_lsupd(p, ifa, lsa_list + i, lsa_count - i);

    if (!c)	/* Too large LSA */
      { i++; continue; }

    OSPF_PACKET(ospf_dump_lsupd, ospf_tx_buffer(ifa),
		"LSUPD packet flooded via %s", ifa->ifname);

    if (ifa->type == OSPF_IT_BCAST)
    {
      if ((ifa->state == OSPF_IS_DR) || (ifa->state == OSPF_IS_BACKUP))
	ospf_send_to_all(ifa);
      else
	ospf_send_to_des(ifa);
    }
    else
      ospf_send_to_agt(ifa, NEIGHBOR_EXCHANGE);
  }

  return i;
}

int
ospf_send_lsupd(struct ospf_proto *p, struct top_hash_entry **lsa_list, uint lsa_count, struct ospf_neighbor *n)
{
  struct ospf_iface *ifa = n->ifa;
  uint i, c;

  for (i = 0; i < lsa_count; i += c)
  {
    c = ospf_prepare_lsupd(p, ifa, lsa_list + i, lsa_count - i);

    if (!c)	/* Too large LSA */
      { i++; continue; }

    OSPF_PACKET(ospf_dump_lsupd, ospf_tx_buffer(ifa),
		"LSUPD packet sent to nbr %R on %s", n->rid, ifa->ifname);

    ospf_send_to(ifa, n->ip);
  }

  return i;
}

void
ospf_rxmt_lsupd(struct ospf_proto *p, struct ospf_neighbor *n)
{
  uint max = 2 * n->ifa->flood_queue_size;
  struct top_hash_entry *entries[max];
  struct top_hash_entry *ret, *nxt, *en;
  uint i = 0;

  /* ASSERT((n->state >= NEIGHBOR_EXCHANGE) && !EMPTY_SLIST(n->lsrtl)); */

  WALK_SLIST_DELSAFE(ret, nxt, n->lsrtl)
  {
    if (i == max)
      break;

    en = ospf_hash_find_entry(p->gr, ret);
    if (!en)
    {
      /* Probably flushed LSA, this should not happen */
      log(L_WARN "%s: LSA disappeared (Type: %04x, Id: %R, Rt: %R)",
	  p->p.name, ret->lsa_type, ret->lsa.id, ret->lsa.rt);

      s_rem_node(SNODE ret);
      ospf_hash_delete(n->lsrth, ret);

      continue;
    }

    entries[i] = en;
    i++;
  }

  ospf_send_lsupd(p, entries, i, n);
}


static inline int
ospf_addr_is_local(struct ospf_proto *p, struct ospf_area *oa, ip_addr ip)
{
  struct ospf_iface *ifa;
  WALK_LIST(ifa, p->iface_list)
    if ((ifa->oa == oa) && ifa->addr && ipa_equal(ifa->addr->ip, ip))
      return 1;

  return 0;
}

void
ospf_receive_lsupd(struct ospf_packet *pkt, struct ospf_iface *ifa,
		   struct ospf_neighbor *n)
{
  struct ospf_proto *p = ifa->oa->po;
  const char *err_dsc = NULL;
  uint plen, err_val = 0;

  /* RFC 2328 13. */

  plen = ntohs(pkt->length);
  if (plen < ospf_lsupd_hdrlen(p))
  {
    LOG_PKT("Bad LSUPD packet from nbr %R on %s - %s (%u)", n->rid, ifa->ifname, "too short", plen);
    return;
  }

  OSPF_PACKET(ospf_dump_lsupd, pkt, "LSUPD packet received from nbr %R on %s", n->rid, ifa->ifname);

  if (n->state < NEIGHBOR_EXCHANGE)
  {
    OSPF_TRACE(D_PACKETS, "LSUPD packet ignored - lesser state than Exchange");
    return;
  }

  ospf_neigh_sm(n, INM_HELLOREC);	/* Questionable */

  uint offset, i, lsa_count;
  ospf_lsupd_body(p, pkt, &offset, &lsa_count);

  for (i = 0; i < lsa_count; i++)
  {
    struct ospf_lsa_header lsa, *lsa_n;
    struct top_hash_entry *en;
    u32 lsa_len, lsa_type, lsa_domain;

    if ((offset + sizeof(struct ospf_lsa_header)) > plen)
      DROP("too short", plen);

    /* LSA header in network order */
    lsa_n = ((void *) pkt) + offset;
    lsa_len = ntohs(lsa_n->length);
    offset += lsa_len;

    if (offset > plen)
      DROP("too short", plen);

    if (((lsa_len % 4) != 0) || (lsa_len <= sizeof(struct ospf_lsa_header)))
      DROP("invalid LSA length", lsa_len);

    /* LSA header in host order */
    lsa_ntoh_hdr(lsa_n, &lsa);
    lsa_get_type_domain(&lsa, ifa, &lsa_type, &lsa_domain);

    DBG("Update Type: %04x, Id: %R, Rt: %R, Sn: 0x%08x, Age: %u, Sum: %u\n",
	lsa_type, lsa.id, lsa.rt, lsa.sn, lsa.age, lsa.checksum);

    /* RFC 2328 13. (1) - verify LSA checksum */
    if ((lsa_n->checksum == 0) || !lsa_verify_checksum(lsa_n, lsa_len))
      SKIP("invalid checksum");

    /* RFC 2328 13. (2) */
    if (!lsa_type)
      SKIP("unknown type");

    /* RFC 5340 4.5.1 (2) and RFC 2328 13. (3) */
    if (!oa_is_ext(ifa->oa) && (LSA_SCOPE(lsa_type) == LSA_SCOPE_AS))
      SKIP("AS scope in stub area");

    /* Errata 3746 to RFC 2328 - rt-summary-LSAs forbidden in stub areas */
    if (!oa_is_ext(ifa->oa) && (lsa_type == LSA_T_SUM_RT))
      SKIP("rt-summary-LSA in stub area");

    /* RFC 5340 4.5.1 (3) */
    if (LSA_SCOPE(lsa_type) == LSA_SCOPE_RES)
      SKIP("invalid scope");

    /* Find local copy of LSA in link state database */
    en = ospf_hash_find(p->gr, lsa_domain, lsa.id, lsa.rt, lsa_type);

#ifdef LOCAL_DEBUG
    if (en)
      DBG("I have Type: %04x, Id: %R, Rt: %R, Sn: 0x%08x, Age: %u, Sum: %u\n",
	  en->lsa_type, en->lsa.id, en->lsa.rt, en->lsa.sn, en->lsa.age, en->lsa.checksum);
#endif

    /* 13. (4) - ignore maxage LSA if i have no local copy */
    if ((lsa.age == LSA_MAXAGE) && !en && (p->padj == 0))
    {
      /* 13.5. - schedule ACKs (tbl 19, case 5) */
      ospf_enqueue_lsack(n, lsa_n, ACKL_DIRECT);
      continue;
    }

    /* 13. (5) - received LSA is newer (or no local copy) */
    if (!en || (lsa_comp(&lsa, &en->lsa) == CMP_NEWER))
    {
      /* 13. (5a) - enforce minimum time between updates for received LSAs */
      /* We also use this to ratelimit reactions to received self-originated LSAs */
      if (en && ((now - en->inst_time) < MINLSARRIVAL))
      {
	OSPF_TRACE(D_EVENTS, "Skipping LSA received in less that MinLSArrival");
	continue;
      }

      /* Copy and validate LSA body */
      int blen = lsa.length - sizeof(struct ospf_lsa_header);
      void *body = mb_alloc(p->p.pool, blen);
      lsa_ntoh_body(lsa_n + 1, body, blen);

      if (lsa_validate(&lsa, lsa_type, ospf_is_v2(p), body) == 0)
      {
	mb_free(body);
	SKIP("invalid body");
      }

      /* 13. (5f) - handle self-originated LSAs, see also 13.4. */
      if ((lsa.rt == p->router_id) ||
	  (ospf_is_v2(p) && (lsa_type == LSA_T_NET) && ospf_addr_is_local(p, ifa->oa, ipa_from_u32(lsa.id))))
      {
	OSPF_TRACE(D_EVENTS, "Received unexpected self-originated LSA");
	ospf_advance_lsa(p, en, &lsa, lsa_type, lsa_domain, body);
	continue;
      }

      /* 13. (5c) - remove old LSA from all retransmission lists
       *
       * We only need to remove it from the retransmission list of the neighbor
       * that send us the new LSA. The old LSA is automatically replaced in
       * retransmission lists by the new LSA.
       */
      if (en)
	ospf_lsa_lsrt_down(en, n);

#if 0
      /*
       * Old code for removing LSA from all retransmission lists. Must be done
       * before (5b), otherwise it also removes the new entries from (5b).
       */
      struct ospf_iface *ifi;
      struct ospf_neighbor *ni;

      WALK_LIST(ifi, p->iface_list)
	WALK_LIST(ni, ifi->neigh_list)
	  if (ni->state > NEIGHBOR_EXSTART)
	    ospf_lsa_lsrt_down(en, ni);
#endif

      /* 13. (5d) - install new LSA into database */
      en = ospf_install_lsa(p, &lsa, lsa_type, lsa_domain, body);

      /* RFC 5340 4.4.3 Events 6+7 - new Link LSA received */
      if (lsa_type == LSA_T_LINK)
	ospf_notify_net_lsa(ifa);

      /* 13. (5b) - flood new LSA */
      int flood_back = ospf_flood_lsa(p, en, n);

      /* 13.5. - schedule ACKs (tbl 19, cases 1+2) */
      if (! flood_back)
	if ((ifa->state != OSPF_IS_BACKUP) || (n->rid == ifa->drid))
	  ospf_enqueue_lsack(n, lsa_n, ACKL_DELAY);

      /* FIXME: remove LSA entry if it is LSA_MAXAGE and it is possible? */

      continue;
    }

    /* 13. (6) - received LSA is in Link state request list (but not newer) */
    if (ospf_hash_find_entry(n->lsrqh, en) != NULL)
      DROP1("error in LSA database exchange");

    /* 13. (7) - received LSA is same */
    if (lsa_comp(&lsa, &en->lsa) == CMP_SAME)
    {
      /* Duplicate LSA, treat as implicit ACK */
      int implicit_ack = ospf_lsa_lsrt_down(en, n);

      /* 13.5. - schedule ACKs (tbl 19, cases 3+4) */
      if (implicit_ack)
      {
	if ((ifa->state == OSPF_IS_BACKUP) && (n->rid == ifa->drid))
	  ospf_enqueue_lsack(n, lsa_n, ACKL_DELAY);
      }
      else
	ospf_enqueue_lsack(n, lsa_n, ACKL_DIRECT);

      continue;
    }

    /* 13. (8) - received LSA is older */
    {
      /* Seqnum is wrapping, wait until it is flushed */
      if ((en->lsa.age == LSA_MAXAGE) && (en->lsa.sn == LSA_MAXSEQNO))
	continue;

      /* Send newer local copy back to neighbor */
      /* FIXME - check for MinLSArrival ? */
      ospf_send_lsupd(p, &en, 1, n);

      continue;
    }

  skip:
    LOG_LSA1("Bad LSA (Type: %04x, Id: %R, Rt: %R) in LSUPD", lsa_type, lsa.id, lsa.rt);
    LOG_LSA2("  received from nbr %R on %s - %s", n->rid, ifa->ifname, err_dsc);
  }

  /* Send direct LSACKs */
  ospf_send_lsack(p, n, ACKL_DIRECT);

  /* Send enqueued LSAs immediately, do not wait for flood_event */
  if (ev_active(p->flood_event))
  {
    ev_postpone(p->flood_event);
    ospf_flood_event(p);
  }

  /*
   * During loading, we should ask for another batch of LSAs. This is only
   * vaguely mentioned in RFC 2328. We send a new LSREQ if all requests sent in
   * the last packet were already answered and/or removed from the LS request
   * list and therefore lsrqi is pointing to the first node of the list.
   */
  if (!EMPTY_SLIST(n->lsrql) && (n->lsrqi == SHEAD(n->lsrql)))
  {
    ospf_send_lsreq(p, n);
    tm_start(n->lsrq_timer, n->ifa->rxmtint);
  }

  return;

drop:
  LOG_PKT("Bad LSUPD packet from nbr %R on %s - %s (%u)",
	  n->rid, ifa->ifname, err_dsc, err_val);

  /* Malformed LSUPD - there is no defined error event, we abuse BadLSReq */
  ospf_neigh_sm(n, INM_BADLSREQ);
  return;
}
