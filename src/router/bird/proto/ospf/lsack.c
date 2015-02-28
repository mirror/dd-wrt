/*
 *	BIRD -- OSPF
 *
 *	(c) 2000-2004 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "ospf.h"


struct ospf_lsack_packet
{
  struct ospf_packet ospf_packet;
  struct ospf_lsa_header lsh[];
};


char *s_queue[] = { "direct", "delayed" };


static void ospf_dump_lsack(struct proto *p, struct ospf_lsack_packet *pkt)
{
  struct ospf_packet *op = &pkt->ospf_packet;

  ASSERT(op->type == LSACK_P);
  ospf_dump_common(p, op);

  unsigned int i, j;
  j = (ntohs(op->length) - sizeof(struct ospf_lsack_packet)) /
    sizeof(struct ospf_lsa_header);

  for (i = 0; i < j; i++)
    ospf_dump_lsahdr(p, pkt->lsh + i);
}


/*
 * =====================================
 * Note, that h is in network endianity!
 * =====================================
 */

void
ospf_lsack_enqueue(struct ospf_neighbor *n, struct ospf_lsa_header *h,
		   int queue)
{
  struct lsah_n *no = mb_alloc(n->pool, sizeof(struct lsah_n));
  memcpy(&no->lsa, h, sizeof(struct ospf_lsa_header));
  add_tail(&n->ackl[queue], NODE no);
  DBG("Adding (%s) ack for %R, ID: %R, RT: %R, Type: %u\n", s_queue[queue],
      n->rid, ntohl(h->id), ntohl(h->rt), h->type);
}

void
ospf_lsack_send(struct ospf_neighbor *n, int queue)
{
  struct ospf_packet *op;
  struct ospf_lsack_packet *pk;
  u16 len, i = 0;
  struct ospf_lsa_header *h;
  struct lsah_n *no;
  struct ospf_iface *ifa = n->ifa;
  struct proto *p = &n->ifa->oa->po->proto;

  if (EMPTY_LIST(n->ackl[queue]))
    return;

  pk = ospf_tx_buffer(ifa);
  op = &pk->ospf_packet;

  ospf_pkt_fill_hdr(n->ifa, pk, LSACK_P);
  h = pk->lsh;

  while (!EMPTY_LIST(n->ackl[queue]))
  {
    no = (struct lsah_n *) HEAD(n->ackl[queue]);
    memcpy(h + i, &no->lsa, sizeof(struct ospf_lsa_header));
    DBG("Iter %u ID: %R, RT: %R, Type: %04x\n", i, ntohl((h + i)->id),
	ntohl((h + i)->rt), (h + i)->type);
    i++;
    rem_node(NODE no);
    mb_free(no);
    if ((i * sizeof(struct ospf_lsa_header) +
	 sizeof(struct ospf_lsack_packet)) > ospf_pkt_maxsize(n->ifa))
    {
      if (!EMPTY_LIST(n->ackl[queue]))
      {
	len =
	  sizeof(struct ospf_lsack_packet) +
	  i * sizeof(struct ospf_lsa_header);
	op->length = htons(len);
	DBG("Sending and continuing! Len=%u\n", len);

	OSPF_PACKET(ospf_dump_lsack, pk, "LSACK packet sent via %s", ifa->ifname);

	if (ifa->type == OSPF_IT_BCAST)
	{
	  if ((ifa->state == OSPF_IS_DR) || (ifa->state == OSPF_IS_BACKUP))
	    ospf_send_to_all(ifa);
	  else if (ifa->cf->real_bcast)
	    ospf_send_to_bdr(ifa);
	  else
	    ospf_send_to(ifa, AllDRouters);
	}
	else
	{
	  if ((ifa->state == OSPF_IS_DR) || (ifa->state == OSPF_IS_BACKUP))
	    ospf_send_to_agt(ifa, NEIGHBOR_EXCHANGE);
	  else
	    ospf_send_to_bdr(ifa);
	}

	ospf_pkt_fill_hdr(n->ifa, pk, LSACK_P);
	i = 0;
      }
    }
  }

  len = sizeof(struct ospf_lsack_packet) + i * sizeof(struct ospf_lsa_header);
  op->length = htons(len);
  DBG("Sending! Len=%u\n", len);

  OSPF_PACKET(ospf_dump_lsack, pk, "LSACK packet sent via %s", ifa->ifname);

  if (ifa->type == OSPF_IT_BCAST)
  {
    if ((ifa->state == OSPF_IS_DR) || (ifa->state == OSPF_IS_BACKUP))
      ospf_send_to_all(ifa);
    else if (ifa->cf->real_bcast)
      ospf_send_to_bdr(ifa);
    else
      ospf_send_to(ifa, AllDRouters);
  }
  else
    ospf_send_to_agt(ifa, NEIGHBOR_EXCHANGE);
}

void
ospf_lsack_receive(struct ospf_packet *ps_i, struct ospf_iface *ifa,
		   struct ospf_neighbor *n)
{
  struct proto *p = &ifa->oa->po->proto;
  struct ospf_lsa_header lsa;
  struct top_hash_entry *en;
  unsigned int i, lsano;

  unsigned int size = ntohs(ps_i->length);
  if (size < sizeof(struct ospf_lsack_packet))
  {
    log(L_ERR "Bad OSPF LSACK packet from %I -  too short (%u B)", n->ip, size);
    return;
  }

  struct ospf_lsack_packet *ps = (void *) ps_i;
  OSPF_PACKET(ospf_dump_lsack, ps, "LSACK packet received from %I via %s", n->ip, ifa->ifname);

  ospf_neigh_sm(n, INM_HELLOREC);

  if (n->state < NEIGHBOR_EXCHANGE)
    return;

  lsano = (size - sizeof(struct ospf_lsack_packet)) /
    sizeof(struct ospf_lsa_header);
  for (i = 0; i < lsano; i++)
  {
    ntohlsah(ps->lsh + i, &lsa);
    u32 dom = ospf_lsa_domain(lsa.type, n->ifa);
    if ((en = ospf_hash_find_header(n->lsrth, dom, &lsa)) == NULL)
      continue;			/* pg 155 */

    if (lsa_comp(&lsa, &en->lsa) != CMP_SAME)	/* pg 156 */
    {
      if ((lsa.sn == LSA_MAXSEQNO) && (lsa.age == LSA_MAXAGE))
	continue;

      OSPF_TRACE(D_PACKETS, "Strange LSACK from %I", n->ip);
      OSPF_TRACE(D_PACKETS, "Type: %04x, Id: %R, Rt: %R",
		 lsa.type, lsa.id, lsa.rt);
      OSPF_TRACE(D_PACKETS, "I have: Age: %4u, Seq: %08x, Sum: %04x",
		 en->lsa.age, en->lsa.sn, en->lsa.checksum);
      OSPF_TRACE(D_PACKETS, "He has: Age: %4u, Seq: %08x, Sum: %04x",
		 lsa.age, lsa.sn, lsa.checksum);
      continue;
    }

    DBG("Deleting LS Id: %R RT: %R Type: %u from LS Retl for neighbor %R\n",
	lsa.id, lsa.rt, lsa.type, n->rid);
    s_rem_node(SNODE en);
    ospf_hash_delete(n->lsrth, en);
  }
}
