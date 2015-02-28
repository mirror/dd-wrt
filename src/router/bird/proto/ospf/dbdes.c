/*
 *	BIRD -- OSPF
 *
 *	(c) 1999--2004 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "ospf.h"


#ifdef OSPFv2
struct ospf_dbdes_packet
{
  struct ospf_packet ospf_packet;
  u16 iface_mtu;
  u8 options;
  union imms imms;		/* I, M, MS bits */
  u32 ddseq;
};

#define hton_opt(X) X
#define ntoh_opt(X) X
#endif


#ifdef OSPFv3
struct ospf_dbdes_packet
{
  struct ospf_packet ospf_packet;
  u32 options;
  u16 iface_mtu;
  u8 padding;
  union imms imms;		/* I, M, MS bits */
  u32 ddseq;
};

#define hton_opt(X) htonl(X)
#define ntoh_opt(X) ntohl(X)
#endif

  
static void ospf_dump_dbdes(struct proto *p, struct ospf_dbdes_packet *pkt)
{
  struct ospf_packet *op = &pkt->ospf_packet;

  ASSERT(op->type == DBDES_P);
  ospf_dump_common(p, op);
  log(L_TRACE "%s:     imms     %s%s%s",
      p->name, pkt->imms.bit.ms ? "MS " : "",
      pkt->imms.bit.m ? "M " : "",
      pkt->imms.bit.i ? "I " : "" );
  log(L_TRACE "%s:     ddseq    %u", p->name, ntohl(pkt->ddseq));

  struct ospf_lsa_header *plsa = (void *) (pkt + 1);
  unsigned int i, j;

  j = (ntohs(op->length) - sizeof(struct ospf_dbdes_packet)) /
    sizeof(struct ospf_lsa_header);

  for (i = 0; i < j; i++)
    ospf_dump_lsahdr(p, plsa + i);
}


/**
 * ospf_dbdes_send - transmit database description packet
 * @n: neighbor
 * @next: whether to send a next packet in a sequence (1) or to retransmit the old one (0)
 *
 * Sending of a database description packet is described in 10.8 of RFC 2328.
 * Reception of each packet is acknowledged in the sequence number of another.
 * When I send a packet to a neighbor I keep a copy in a buffer. If the neighbor
 * does not reply, I don't create a new packet but just send the content
 * of the buffer.
 */
void
ospf_dbdes_send(struct ospf_neighbor *n, int next)
{
  struct ospf_dbdes_packet *pkt;
  struct ospf_packet *op;
  struct ospf_iface *ifa = n->ifa;
  struct ospf_area *oa = ifa->oa;
  struct proto_ospf *po = oa->po;
  struct proto *p = &po->proto;
  u16 length, i, j;

  /* FIXME ??? */
  if ((oa->rt == NULL) || (EMPTY_LIST(po->lsal)))
    update_rt_lsa(oa);

  switch (n->state)
  {
  case NEIGHBOR_EXSTART:	/* Send empty packets */
    n->myimms.bit.i = 1;
    pkt = ospf_tx_buffer(ifa);
    op = &pkt->ospf_packet;
    ospf_pkt_fill_hdr(ifa, pkt, DBDES_P);
    pkt->iface_mtu = (ifa->type == OSPF_IT_VLINK) ? 0 : htons(ifa->iface->mtu);
    pkt->options = hton_opt(oa->options);
    pkt->imms = n->myimms;
    pkt->ddseq = htonl(n->dds);
    length = sizeof(struct ospf_dbdes_packet);
    op->length = htons(length);

    OSPF_PACKET(ospf_dump_dbdes, pkt, "DBDES packet sent to %I via %s", n->ip, ifa->ifname);
    ospf_send_to(ifa, n->ip);
    break;

  case NEIGHBOR_EXCHANGE:
    n->myimms.bit.i = 0;

    if (next)
    {
      snode *sn;
      struct ospf_lsa_header *lsa;

      if (n->ldd_bsize != ifa->tx_length)
      {
	mb_free(n->ldd_buffer);
	n->ldd_buffer = mb_allocz(n->pool, ifa->tx_length);
	n->ldd_bsize = ifa->tx_length;
      }

      pkt = n->ldd_buffer;
      op = (struct ospf_packet *) pkt;

      ospf_pkt_fill_hdr(ifa, pkt, DBDES_P);
      pkt->iface_mtu = (ifa->type == OSPF_IT_VLINK) ? 0 : htons(ifa->iface->mtu);
      pkt->ddseq = htonl(n->dds);
      pkt->options = hton_opt(oa->options);

      j = i = (ospf_pkt_maxsize(ifa) - sizeof(struct ospf_dbdes_packet)) / sizeof(struct ospf_lsa_header);	/* Number of possible lsaheaders to send */
      lsa = (n->ldd_buffer + sizeof(struct ospf_dbdes_packet));

      if (n->myimms.bit.m)
      {
	sn = s_get(&(n->dbsi));

	DBG("Number of LSA: %d\n", j);
	for (; i > 0; i--)
	{
	  struct top_hash_entry *en= (struct top_hash_entry *) sn;

          if (ospf_lsa_flooding_allowed(&en->lsa, en->domain, ifa))
          {
	    htonlsah(&(en->lsa), lsa);
	    DBG("Working on: %d\n", i);
	    DBG("\tX%01x %-1R %-1R %p\n", en->lsa.type, en->lsa.id, en->lsa.rt, en->lsa_body);

	    lsa++;
          }
          else i++;	/* No lsa added */

	  if (sn == STAIL(po->lsal))
          {
            i--;
	    break;
          }

	  sn = sn->next;
	}

	if (sn == STAIL(po->lsal))
	{
	  DBG("Number of LSA NOT sent: %d\n", i);
	  DBG("M bit unset.\n");
	  n->myimms.bit.m = 0;	/* Unset more bit */
	}

	s_put(&(n->dbsi), sn);
      }

      pkt->imms.byte = n->myimms.byte;

      length = (j - i) * sizeof(struct ospf_lsa_header) +
	sizeof(struct ospf_dbdes_packet);
      op->length = htons(length);

      DBG("%s: DB_DES (M) prepared for %I.\n", p->name, n->ip);
    }

  case NEIGHBOR_LOADING:
  case NEIGHBOR_FULL:
    length = n->ldd_buffer ? ntohs(((struct ospf_packet *) n->ldd_buffer)->length) : 0;

    if (!length)
    {
      OSPF_TRACE(D_PACKETS, "No packet in my buffer for repeating");
      ospf_neigh_sm(n, INM_KILLNBR);
      return;
    }

    /* Send last packet from ldd buffer */

    OSPF_PACKET(ospf_dump_dbdes, n->ldd_buffer, "DBDES packet sent to %I via %s", n->ip, ifa->ifname);

    sk_set_tbuf(ifa->sk, n->ldd_buffer);
    ospf_send_to(ifa, n->ip);
    sk_set_tbuf(ifa->sk, NULL);

    if(n->myimms.bit.ms) tm_start(n->rxmt_timer, n->ifa->rxmtint);		/* Restart timer */

    if (!n->myimms.bit.ms)
    {
      if ((n->myimms.bit.m == 0) && (n->imms.bit.m == 0) &&
	  (n->state == NEIGHBOR_EXCHANGE))
      {
	ospf_neigh_sm(n, INM_EXDONE);
      }
    }
    break;

  default:			/* Ignore it */
    break;
  }
}

static void
ospf_dbdes_reqladd(struct ospf_dbdes_packet *ps, struct ospf_neighbor *n)
{
  struct ospf_lsa_header *plsa, lsa;
  struct top_hash_entry *he, *sn;
  struct ospf_area *oa = n->ifa->oa;
  struct top_graph *gr = oa->po->gr;
  struct ospf_packet *op;
  int i, j;

  op = (struct ospf_packet *) ps;

  plsa = (void *) (ps + 1);

  j = (ntohs(op->length) - sizeof(struct ospf_dbdes_packet)) /
    sizeof(struct ospf_lsa_header);

  for (i = 0; i < j; i++)
  {
    ntohlsah(plsa + i, &lsa);
    u32 dom = ospf_lsa_domain(lsa.type, n->ifa);
    if (((he = ospf_hash_find_header(gr, dom, &lsa)) == NULL) ||
	(lsa_comp(&lsa, &(he->lsa)) == 1))
    {
      /* Is this condition necessary? */
      if (ospf_hash_find_header(n->lsrqh, dom, &lsa) == NULL)
      {
	sn = ospf_hash_get_header(n->lsrqh, dom, &lsa);
	ntohlsah(plsa + i, &(sn->lsa));
	s_add_tail(&(n->lsrql), SNODE sn);
      }
    }
  }
}

void
ospf_dbdes_receive(struct ospf_packet *ps_i, struct ospf_iface *ifa,
		   struct ospf_neighbor *n)
{
  struct proto_ospf *po = ifa->oa->po;
  struct proto *p = &po->proto;

  unsigned int size = ntohs(ps_i->length);
  if (size < sizeof(struct ospf_dbdes_packet))
  {
    log(L_ERR "Bad OSPF DBDES packet from %I -  too short (%u B)", n->ip, size);
    return;
  }

  struct ospf_dbdes_packet *ps = (void *) ps_i;
  u32 ps_ddseq = ntohl(ps->ddseq);
  u32 ps_options = ntoh_opt(ps->options);
  u16 ps_iface_mtu = ntohs(ps->iface_mtu);
  
  OSPF_PACKET(ospf_dump_dbdes, ps, "DBDES packet received from %I via %s", n->ip, ifa->ifname);

  ospf_neigh_sm(n, INM_HELLOREC);

  switch (n->state)
  {
  case NEIGHBOR_DOWN:
  case NEIGHBOR_ATTEMPT:
  case NEIGHBOR_2WAY:
    return;
    break;
  case NEIGHBOR_INIT:
    ospf_neigh_sm(n, INM_2WAYREC);
    if (n->state != NEIGHBOR_EXSTART)
      return;
  case NEIGHBOR_EXSTART:

    if ((ifa->type != OSPF_IT_VLINK) && (ps_iface_mtu != ifa->iface->mtu)
	&& (ps_iface_mtu != 0) && (ifa->iface->mtu != 0))
      log(L_WARN "OSPF: MTU mismatch with neighbour %I on interface %s (remote %d, local %d)",
	  n->ip, ifa->ifname, ps_iface_mtu, ifa->iface->mtu);

    if ((ps->imms.bit.m && ps->imms.bit.ms && ps->imms.bit.i)
	&& (n->rid > po->router_id) && (size == sizeof(struct ospf_dbdes_packet)))
    {
      /* I'm slave! */
      n->dds = ps_ddseq;
      n->ddr = ps_ddseq;
      n->options = ps_options;
      n->myimms.bit.ms = 0;
      n->imms.byte = ps->imms.byte;
      OSPF_TRACE(D_PACKETS, "I'm slave to %I.", n->ip);
      ospf_neigh_sm(n, INM_NEGDONE);
      ospf_dbdes_send(n, 1);
      break;
    }

    if (((ps->imms.bit.i == 0) && (ps->imms.bit.ms == 0)) &&
        (n->rid < po->router_id) && (n->dds == ps_ddseq))
    {
      /* I'm master! */
      n->options = ps_options;
      n->ddr = ps_ddseq - 1;	/* It will be set corectly a few lines down */
      n->imms.byte = ps->imms.byte;
      OSPF_TRACE(D_PACKETS, "I'm master to %I.", n->ip);
      ospf_neigh_sm(n, INM_NEGDONE);
    }
    else
    {
      DBG("%s: Nothing happend to %I (imms=%u)\n", p->name, n->ip,
          ps->imms.byte);
      break;
    }
  case NEIGHBOR_EXCHANGE:
    if ((ps->imms.byte == n->imms.byte) && (ps_options == n->options) &&
	(ps_ddseq == n->ddr))
    {
      /* Duplicate packet */
      OSPF_TRACE(D_PACKETS, "Received duplicate dbdes from %I.", n->ip);
      if (n->myimms.bit.ms == 0)
      {
	/* Slave should retransmit dbdes packet */
	ospf_dbdes_send(n, 0);
      }
      return;
    }

    n->ddr = ps_ddseq;

    if (ps->imms.bit.ms != n->imms.bit.ms)	/* M/S bit differs */
    {
      OSPF_TRACE(D_PACKETS, "dbdes - sequence mismatch neighbor %I (bit MS)",
		 n->ip);
      ospf_neigh_sm(n, INM_SEQMIS);
      break;
    }

    if (ps->imms.bit.i)		/* I bit is set */
    {
      OSPF_TRACE(D_PACKETS, "dbdes - sequence mismatch neighbor %I (bit I)",
		 n->ip);
      ospf_neigh_sm(n, INM_SEQMIS);
      break;
    }

    n->imms.byte = ps->imms.byte;

    if (ps_options != n->options)	/* Options differs */
    {
      OSPF_TRACE(D_PACKETS, "dbdes - sequence mismatch neighbor %I (options)",
		 n->ip);
      ospf_neigh_sm(n, INM_SEQMIS);
      break;
    }

    if (n->myimms.bit.ms)
    {
      if (ps_ddseq != n->dds)	/* MASTER */
      {
	OSPF_TRACE(D_PACKETS, "dbdes - sequence mismatch neighbor %I (master)",
		   n->ip);
	ospf_neigh_sm(n, INM_SEQMIS);
	break;
      }
      n->dds++;
      DBG("Incrementing dds\n");
      ospf_dbdes_reqladd(ps, n);
      if ((n->myimms.bit.m == 0) && (ps->imms.bit.m == 0))
      {
	ospf_neigh_sm(n, INM_EXDONE);
      }
      else
      {
	ospf_dbdes_send(n, 1);
      }

    }
    else
    {
      if (ps_ddseq != (n->dds + 1))	/* SLAVE */
      {
	OSPF_TRACE(D_PACKETS, "dbdes - sequence mismatch neighbor %I (slave)", n->ip);
	ospf_neigh_sm(n, INM_SEQMIS);
	break;
      }
      n->ddr = ps_ddseq;
      n->dds = ps_ddseq;
      ospf_dbdes_reqladd(ps, n);
      ospf_dbdes_send(n, 1);
    }

    break;
  case NEIGHBOR_LOADING:
  case NEIGHBOR_FULL:
    if ((ps->imms.byte == n->imms.byte) && (ps_options == n->options)
	&& (ps_ddseq == n->ddr))
      /* Only duplicate are accepted */
    {
      OSPF_TRACE(D_PACKETS, "Received duplicate dbdes from %I.", n->ip);
      if (n->myimms.bit.ms == 0)
      {
	/* Slave should retransmit dbdes packet */
	ospf_dbdes_send(n, 0);
      }
      return;
    }
    else
    {
      OSPF_TRACE(D_PACKETS, "dbdes - sequence mismatch neighbor %I (full)",
		 n->ip);
      DBG("PS=%u, DDR=%u, DDS=%u\n", ps_ddseq, n->ddr, n->dds);
      ospf_neigh_sm(n, INM_SEQMIS);
    }
    break;
  default:
    bug("Received dbdes from %I in undefined state.", n->ip);
  }
}
