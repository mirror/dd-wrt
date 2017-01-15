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
struct ospf_lsack_packet
{
  struct ospf_packet hdr;
  // union ospf_auth auth;

  struct ospf_lsa_header lsas[];
};
*/

struct lsa_node
{
  node n;
  struct ospf_lsa_header lsa;
};


static inline void
ospf_lsack_body(struct ospf_proto *p, struct ospf_packet *pkt,
		struct ospf_lsa_header **body, uint *count)
{
  uint plen = ntohs(pkt->length);
  uint hlen = ospf_pkt_hdrlen(p);

  *body = ((void *) pkt) + hlen;
  *count = (plen - hlen) / sizeof(struct ospf_lsa_header);
}

static void
ospf_dump_lsack(struct ospf_proto *p, struct ospf_packet *pkt)
{
  struct ospf_lsa_header *lsas;
  uint i, lsa_count;

  ASSERT(pkt->type == LSACK_P);
  ospf_dump_common(p, pkt);

  ospf_lsack_body(p, pkt, &lsas, &lsa_count);
  for (i = 0; i < lsa_count; i++)
    ospf_dump_lsahdr(p, lsas + i);
}


void
ospf_enqueue_lsack(struct ospf_neighbor *n, struct ospf_lsa_header *h_n, int queue)
{
  /* Note that h_n is in network endianity */
  struct lsa_node *no = mb_alloc(n->pool, sizeof(struct lsa_node));
  memcpy(&no->lsa, h_n, sizeof(struct ospf_lsa_header));
  add_tail(&n->ackl[queue], NODE no);
  DBG("Adding %s ack for %R, ID: %R, RT: %R, Type: %u\n",
      (queue == ACKL_DIRECT) ? "direct" : "delayed",
      n->rid, ntohl(h_n->id), ntohl(h_n->rt), h_n->type);
}

void
ospf_reset_lsack_queue(struct ospf_neighbor *n)
{
  struct lsa_node *no;

  WALK_LIST_FIRST(no, n->ackl[ACKL_DELAY])
  {
    rem_node(NODE no);
    mb_free(no);
  }
}

static inline void
ospf_send_lsack_(struct ospf_proto *p, struct ospf_neighbor *n, int queue)
{
  struct ospf_iface *ifa = n->ifa;
  struct ospf_lsa_header *lsas;
  struct ospf_packet *pkt;
  struct lsa_node *no;
  uint i, lsa_max, length;

  /* RFC 2328 13.5 */

  pkt = ospf_tx_buffer(ifa);
  ospf_pkt_fill_hdr(ifa, pkt, LSACK_P);
  ospf_lsack_body(p, pkt, &lsas, &lsa_max);

  for (i = 0; i < lsa_max && !EMPTY_LIST(n->ackl[queue]); i++)
  {
    no = (struct lsa_node *) HEAD(n->ackl[queue]);
    memcpy(&lsas[i], &no->lsa, sizeof(struct ospf_lsa_header));
    DBG("Iter %u ID: %R, RT: %R, Type: %04x\n",
	i, ntohl(lsas[i].id), ntohl(lsas[i].rt), lsas[i].type);
    rem_node(NODE no);
    mb_free(no);
  }

  length = ospf_pkt_hdrlen(p) + i * sizeof(struct ospf_lsa_header);
  pkt->length = htons(length);

  OSPF_PACKET(ospf_dump_lsack, pkt, "LSACK packet sent via %s", ifa->ifname);

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

void
ospf_send_lsack(struct ospf_proto *p, struct ospf_neighbor *n, int queue)
{
  while (!EMPTY_LIST(n->ackl[queue]))
    ospf_send_lsack_(p, n, queue);
}

void
ospf_receive_lsack(struct ospf_packet *pkt, struct ospf_iface *ifa,
		   struct ospf_neighbor *n)
{
  struct ospf_proto *p = ifa->oa->po;
  struct ospf_lsa_header lsa, *lsas;
  struct top_hash_entry *ret, *en;
  uint i, lsa_count;
  u32 lsa_type, lsa_domain;

  /* RFC 2328 13.7 */

  /* No need to check length, lsack has only basic header */

  OSPF_PACKET(ospf_dump_lsack, pkt, "LSACK packet received from nbr %R on %s", n->rid, ifa->ifname);

  if (n->state < NEIGHBOR_EXCHANGE)
  {
    OSPF_TRACE(D_PACKETS, "LSACK packet ignored - lesser state than Exchange");
    return;
  }

  ospf_neigh_sm(n, INM_HELLOREC);	/* Not in RFC */

  ospf_lsack_body(p, pkt, &lsas, &lsa_count);
  for (i = 0; i < lsa_count; i++)
  {
    lsa_ntoh_hdr(&lsas[i], &lsa);
    lsa_get_type_domain(&lsa, n->ifa, &lsa_type, &lsa_domain);

    ret = ospf_hash_find(n->lsrth, lsa_domain, lsa.id, lsa.rt, lsa_type);
    if (!ret)
      continue;

    if (lsa_comp(&lsa, &ret->lsa) != CMP_SAME)
    {
      OSPF_TRACE(D_PACKETS, "Strange LSACK from nbr %R on %s", n->rid, ifa->ifname);
      OSPF_TRACE(D_PACKETS, "    Type: %04x, Id: %R, Rt: %R",
		 lsa_type, lsa.id, lsa.rt);
      OSPF_TRACE(D_PACKETS, "    I have: Seq: %08x, Age: %4u, Sum: %04x",
		 ret->lsa.sn, ret->lsa.age, ret->lsa.checksum);
      OSPF_TRACE(D_PACKETS, "    It has: Seq: %08x, Age: %4u, Sum: %04x",
		 lsa.sn, lsa.age, lsa.checksum);
      continue;
    }

    DBG("Deleting LSA (Type: %04x Id: %R Rt: %R) from lsrtl for neighbor %R\n",
	lsa_type, lsa.id, lsa.rt, n->rid);

    en = ospf_hash_find_entry(p->gr, ret);
    ospf_lsa_lsrt_down_(en, n, ret);
  }
}
