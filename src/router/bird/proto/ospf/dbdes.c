/*
 *	BIRD -- OSPF
 *
 *	(c) 1999--2004 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "ospf.h"

  
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
  int i, j;

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
 * Sending of a database description packet is described in 10.6 of RFC 2328.
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

  if ((oa->rt == NULL) || (EMPTY_LIST(po->lsal)))
    originate_rt_lsa(oa);

  switch (n->state)
  {
  case NEIGHBOR_EXSTART:	/* Send empty packets */
    n->myimms.bit.i = 1;
    pkt = (struct ospf_dbdes_packet *) (ifa->ip_sk->tbuf);
    op = (struct ospf_packet *) pkt;
    ospf_pkt_fill_hdr(ifa, pkt, DBDES_P);
    pkt->iface_mtu = htons(ifa->iface->mtu);
    pkt->options = oa->opt.byte;
    pkt->imms = n->myimms;
    pkt->ddseq = htonl(n->dds);
    length = sizeof(struct ospf_dbdes_packet);
    op->length = htons(length);

    OSPF_PACKET(ospf_dump_dbdes, pkt, "DBDES packet sent to %I via %s", n->ip, ifa->iface->name);
    ospf_send_to(ifa->ip_sk, n->ip, ifa);
    break;

  case NEIGHBOR_EXCHANGE:
    n->myimms.bit.i = 0;

    if (next)
    {
      snode *sn;
      struct ospf_lsa_header *lsa;

      pkt = n->ldbdes;
      op = (struct ospf_packet *) pkt;

      ospf_pkt_fill_hdr(ifa, pkt, DBDES_P);
      pkt->iface_mtu = htons(ifa->iface->mtu);
      pkt->options = oa->opt.byte;
      pkt->ddseq = htonl(n->dds);

      j = i = (ospf_pkt_maxsize(ifa) - sizeof(struct ospf_dbdes_packet)) / sizeof(struct ospf_lsa_header);	/* Number of possible lsaheaders to send */
      lsa = (n->ldbdes + sizeof(struct ospf_dbdes_packet));

      if (n->myimms.bit.m)
      {
	sn = s_get(&(n->dbsi));

	DBG("Number of LSA: %d\n", j);
	for (; i > 0; i--)
	{
	  struct top_hash_entry *en= (struct top_hash_entry *) sn;
          int send = 1;

          /* Don't send ext LSA into stub areas */
          if (oa->stub && (en->lsa.type == LSA_T_EXT)) send = 0;
          /* Don't send ext LSAs through VLINK  */
          if ((ifa->type == OSPF_IT_VLINK) && (en->lsa.type == LSA_T_EXT)) send = 0;;
          /* Don't send LSA of other areas */
          if ((en->lsa.type != LSA_T_EXT) && (en->oa != oa)) send = 0;

          if (send)
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
    length = ntohs(((struct ospf_packet *) n->ldbdes)->length);

    if (!length)
    {
      OSPF_TRACE(D_PACKETS, "No packet in my buffer for repeating");
      ospf_neigh_sm(n, INM_KILLNBR);
      return;
    }

    /* Copy last sent packet again */
    memcpy(ifa->ip_sk->tbuf, n->ldbdes, length);

    OSPF_PACKET(ospf_dump_dbdes, (struct ospf_dbdes_packet *) ifa->ip_sk->tbuf,
		"DBDES packet sent to %I via %s", n->ip, ifa->iface->name);
    ospf_send_to(ifa->ip_sk, n->ip, n->ifa);

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
    if (((he = ospf_hash_find(gr, oa->areaid, lsa.id, lsa.rt, lsa.type)) == NULL) ||
	(lsa_comp(&lsa, &(he->lsa)) == 1))
    {
      /* Is this condition necessary? */
      if (ospf_hash_find(n->lsrqh, oa->areaid, lsa.id, lsa.rt, lsa.type) == NULL)
      {
	sn = ospf_hash_get(n->lsrqh, oa, lsa.id, lsa.rt, lsa.type);
	ntohlsah(plsa + i, &(sn->lsa));
	s_add_tail(&(n->lsrql), SNODE sn);
      }
    }
  }
}

void
ospf_dbdes_receive(struct ospf_dbdes_packet *ps,
		   struct ospf_iface *ifa, struct ospf_neighbor *n)
{
  struct proto *p = &ifa->oa->po->proto;
  u32 myrid = p->cf->global->router_id;
  unsigned int size = ntohs(ps->ospf_packet.length);

  OSPF_PACKET(ospf_dump_dbdes, ps, "DBDES packet received from %I via %s", n->ip, ifa->iface->name);

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
    if ((ps->imms.bit.m && ps->imms.bit.ms && ps->imms.bit.i)
	&& (n->rid > myrid) && (size == sizeof(struct ospf_dbdes_packet)))
    {
      /* I'm slave! */
      n->dds = ntohl(ps->ddseq);
      n->ddr = ntohl(ps->ddseq);
      n->options = ps->options;
      n->myimms.bit.ms = 0;
      n->imms.byte = ps->imms.byte;
      OSPF_TRACE(D_PACKETS, "I'm slave to %I.", n->ip);
      ospf_neigh_sm(n, INM_NEGDONE);
      ospf_dbdes_send(n, 1);
      break;
    }

    if (((ps->imms.bit.i == 0) && (ps->imms.bit.ms == 0)) &&
        (n->rid < myrid) && (n->dds == ntohl(ps->ddseq)))
    {
      /* I'm master! */
      n->options = ps->options;
      n->ddr = ntohl(ps->ddseq) - 1;	/* It will be set corectly a few lines down */
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
    if ((ps->imms.byte == n->imms.byte) && (ps->options == n->options) &&
	(ntohl(ps->ddseq) == n->ddr))
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

    n->ddr = ntohl(ps->ddseq);

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

    if (ps->options != n->options)	/* Options differs */
    {
      OSPF_TRACE(D_PACKETS, "dbdes - sequence mismatch neighbor %I (options)",
		 n->ip);
      ospf_neigh_sm(n, INM_SEQMIS);
      break;
    }

    if (n->myimms.bit.ms)
    {
      if (ntohl(ps->ddseq) != n->dds)	/* MASTER */
      {
	OSPF_TRACE(D_PACKETS,
		   "dbdes - sequence mismatch neighbor %I (master)", n->ip);
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
      if (ntohl(ps->ddseq) != (n->dds + 1))	/* SLAVE */
      {
	OSPF_TRACE(D_PACKETS, "dbdes - sequence mismatch neighbor %I (slave)",
		   n->ip);
	ospf_neigh_sm(n, INM_SEQMIS);
	break;
      }
      n->ddr = ntohl(ps->ddseq);
      n->dds = ntohl(ps->ddseq);
      ospf_dbdes_reqladd(ps, n);
      ospf_dbdes_send(n, 1);
    }

    break;
  case NEIGHBOR_LOADING:
  case NEIGHBOR_FULL:
    if ((ps->imms.byte == n->imms.byte) && (ps->options == n->options)
	&& (ntohl(ps->ddseq) == n->ddr))
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
      DBG("PS=%u, DDR=%u, DDS=%u\n", ntohl(ps->ddseq), n->ddr, n->dds);
      ospf_neigh_sm(n, INM_SEQMIS);
    }
    break;
  default:
    bug("Received dbdes from %I in undefined state.", n->ip);
  }
}
