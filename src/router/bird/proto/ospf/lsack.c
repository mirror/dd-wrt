/*
 *	BIRD -- OSPF
 *
 *	(c) 2000-2004 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "ospf.h"

char *s_queue[] = { "direct", "delayed" };


static void ospf_dump_lsack(struct proto *p, struct ospf_lsack_packet *pkt)
{
  struct ospf_packet *op = &pkt->ospf_packet;

  ASSERT(op->type == LSACK_P);
  ospf_dump_common(p, op);

  struct ospf_lsa_header *plsa = (void *) (pkt + 1);
  int i, j;

  j = (ntohs(op->length) - sizeof(struct ospf_lsack_packet)) /
    sizeof(struct ospf_lsa_header);

  for (i = 0; i < j; i++)
    ospf_dump_lsahdr(p, plsa + i);
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
  sock *sk;
  u16 len, i = 0;
  struct ospf_lsa_header *h;
  struct lsah_n *no;
  struct ospf_iface *ifa = n->ifa;
  struct proto *p = &n->ifa->oa->po->proto;

  if (EMPTY_LIST(n->ackl[queue]))
    return;

  if (ifa->type == OSPF_IT_BCAST)
    sk = ifa->hello_sk;
  else
    sk = ifa->ip_sk;

  pk = (struct ospf_lsack_packet *) sk->tbuf;
  op = (struct ospf_packet *) sk->tbuf;

  ospf_pkt_fill_hdr(n->ifa, pk, LSACK_P);
  h = (struct ospf_lsa_header *) (pk + 1);

  while (!EMPTY_LIST(n->ackl[queue]))
  {
    no = (struct lsah_n *) HEAD(n->ackl[queue]);
    memcpy(h + i, &no->lsa, sizeof(struct ospf_lsa_header));
    i++;
    DBG("Iter %u ID: %R, RT: %R, Type: %u\n", i, ntohl((h + i)->id),
	ntohl((h + i)->rt), (h + i)->type);
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

	OSPF_PACKET(ospf_dump_lsack, (struct ospf_lsack_packet *) sk->tbuf,
		    "LSACK packet sent via %s", ifa->iface->name);

	if (ifa->type == OSPF_IT_BCAST)
	{
	  if ((ifa->state == OSPF_IS_DR) || (ifa->state == OSPF_IS_BACKUP))
	    ospf_send_to(sk, AllSPFRouters, ifa);
	  else
	    ospf_send_to(sk, AllDRouters, ifa);
	}
	else
	{
	  if ((ifa->state == OSPF_IS_DR) || (ifa->state == OSPF_IS_BACKUP))
	    ospf_send_to_agt(sk, ifa, NEIGHBOR_EXCHANGE);
	  else
	    ospf_send_to_bdr(sk, ifa);
	}

	ospf_pkt_fill_hdr(n->ifa, pk, LSACK_P);
	i = 0;
      }
    }
  }

  len = sizeof(struct ospf_lsack_packet) + i * sizeof(struct ospf_lsa_header);
  op->length = htons(len);
  DBG("Sending! Len=%u\n", len);

  OSPF_PACKET(ospf_dump_lsack, (struct ospf_lsack_packet *) sk->tbuf,
	      "LSACK packet sent via %s", ifa->iface->name);

  if (ifa->type == OSPF_IT_BCAST)
  {
    if ((ifa->state == OSPF_IS_DR) || (ifa->state == OSPF_IS_BACKUP))
    {
      ospf_send_to(sk, AllSPFRouters, ifa);
    }
    else
    {
      ospf_send_to(sk, AllDRouters, ifa);
    }
  }
  else
  {
    ospf_send_to_agt(sk, ifa, NEIGHBOR_EXCHANGE);
  }
}

void
ospf_lsack_receive(struct ospf_lsack_packet *ps,
		   struct ospf_iface *ifa, struct ospf_neighbor *n)
{
  struct ospf_lsa_header lsa, *plsa;
  u16 nolsa;
  struct top_hash_entry *en;
  struct proto *p = &ifa->oa->po->proto;
  unsigned int size = ntohs(ps->ospf_packet.length), i;

  OSPF_PACKET(ospf_dump_lsack, ps, "LSACK packet received from %I via %s", n->ip, ifa->iface->name);

  ospf_neigh_sm(n, INM_HELLOREC);

  if (n->state < NEIGHBOR_EXCHANGE)
    return;

  nolsa = (size - sizeof(struct ospf_lsack_packet)) /
    sizeof(struct ospf_lsa_header);

  if ((nolsa < 1) || ((size - sizeof(struct ospf_lsack_packet)) !=
		      (nolsa * sizeof(struct ospf_lsa_header))))
  {
    log(L_ERR "Received corrupted LS ack from %I", n->ip);
    return;
  }

  plsa = (struct ospf_lsa_header *) (ps + 1);

  for (i = 0; i < nolsa; i++)
  {
    ntohlsah(plsa + i, &lsa);
    if ((en = ospf_hash_find_header(n->lsrth, n->ifa->oa->areaid, &lsa)) == NULL)
      continue;			/* pg 155 */

    if (lsa_comp(&lsa, &en->lsa) != CMP_SAME)	/* pg 156 */
    {
      if ((lsa.sn == LSA_MAXSEQNO) && (lsa.age == LSA_MAXAGE))
	continue;

      OSPF_TRACE(D_PACKETS, "Strange LS acknoledgement from %I", n->ip);
      OSPF_TRACE(D_PACKETS, "Id: %R, Rt: %R, Type: %u",
		 lsa.id, lsa.rt, lsa.type);
      OSPF_TRACE(D_PACKETS, "I have: Age: %4u, Seqno: 0x%08x, Sum: %u",
		 en->lsa.age, en->lsa.sn, en->lsa.checksum);
      OSPF_TRACE(D_PACKETS, "He has: Age: %4u, Seqno: 0x%08x, Sum: %u",
		 lsa.age, lsa.sn, lsa.checksum);
      continue;
    }

    DBG("Deleting LS Id: %R RT: %R Type: %u from LS Retl for neighbor %R\n",
	lsa.id, lsa.rt, lsa.type, n->rid);
    s_rem_node(SNODE en);
    if (en->lsa_body != NULL)
      mb_free(en->lsa_body);
    en->lsa_body = NULL;
    ospf_hash_delete(n->lsrth, en);
  }
}
