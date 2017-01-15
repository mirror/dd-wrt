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
struct ospf_lsreq_packet
{
  struct ospf_packet hdr;
  // union ospf_auth auth;

  struct ospf_lsreq_header lsrs[];
};
*/


static inline void
ospf_lsreq_body(struct ospf_proto *p, struct ospf_packet *pkt,
		struct ospf_lsreq_header **body, uint *count)
{
  uint plen = ntohs(pkt->length);
  uint hlen = ospf_pkt_hdrlen(p);

  *body = ((void *) pkt) + hlen;
  *count = (plen - hlen) / sizeof(struct ospf_lsreq_header);
}

static void
ospf_dump_lsreq(struct ospf_proto *p, struct ospf_packet *pkt)
{
  struct ospf_lsreq_header *lsrs;
  uint i, lsr_count;

  ASSERT(pkt->type == LSREQ_P);
  ospf_dump_common(p, pkt);

  ospf_lsreq_body(p, pkt, &lsrs, &lsr_count);
  for (i = 0; i < lsr_count; i++)
    log(L_TRACE "%s:     LSR      Type: %04x, Id: %R, Rt: %R", p->p.name,
	ntohl(lsrs[i].type), ntohl(lsrs[i].id), ntohl(lsrs[i].rt));
}


void
ospf_send_lsreq(struct ospf_proto *p, struct ospf_neighbor *n)
{
  struct ospf_iface *ifa = n->ifa;
  struct ospf_lsreq_header *lsrs;
  struct top_hash_entry *req;
  struct ospf_packet *pkt;
  uint i, lsr_max, length;

  /* RFC 2328 10.9 */

  /* ASSERT((n->state >= NEIGHBOR_EXCHANGE) && !EMPTY_SLIST(n->lsrql)); */

  pkt = ospf_tx_buffer(ifa);
  ospf_pkt_fill_hdr(ifa, pkt, LSREQ_P);
  ospf_lsreq_body(p, pkt, &lsrs, &lsr_max);

  i = 0;
  WALK_SLIST(req, n->lsrql)
  {
    if (i == lsr_max)
      break;

    DBG("Requesting %uth LSA: Type: %04u, ID: %R, RT: %R, SN: 0x%x, Age %u\n",
	i, req->lsa_type, req->lsa.id, req->lsa.rt, req->lsa.sn, req->lsa.age);

    u32 etype = lsa_get_etype(&req->lsa, p);
    lsrs[i].type = htonl(etype);
    lsrs[i].rt = htonl(req->lsa.rt);
    lsrs[i].id = htonl(req->lsa.id);
    i++;
  }

  /* We store the position to see whether requested LSAs have been received */
  n->lsrqi = req;

  length = ospf_pkt_hdrlen(p) + i * sizeof(struct ospf_lsreq_header);
  pkt->length = htons(length);

  OSPF_PACKET(ospf_dump_lsreq, pkt, "LSREQ packet sent to nbr %R on %s", n->rid, ifa->ifname);
  ospf_send_to(ifa, n->ip);
}


void
ospf_receive_lsreq(struct ospf_packet *pkt, struct ospf_iface *ifa,
		   struct ospf_neighbor *n)
{
  struct ospf_proto *p = ifa->oa->po;
  struct ospf_lsreq_header *lsrs;
  uint i, lsr_count;

  /* RFC 2328 10.7 */

  /* No need to check length, lsreq has only basic header */

  OSPF_PACKET(ospf_dump_lsreq, pkt, "LSREQ packet received from nbr %R on %s", n->rid, ifa->ifname);

  if (n->state < NEIGHBOR_EXCHANGE)
  {
    OSPF_TRACE(D_PACKETS, "LSREQ packet ignored - lesser state than Exchange");
    return;
  }

  ospf_neigh_sm(n, INM_HELLOREC);	/* Not in RFC */

  ospf_lsreq_body(p, pkt, &lsrs, &lsr_count);

  struct top_hash_entry *en, *entries[lsr_count];

  for (i = 0; i < lsr_count; i++)
  {
    u32 id, rt, type, domain;

    id = ntohl(lsrs[i].id);
    rt = ntohl(lsrs[i].rt);
    lsa_get_type_domain_(ntohl(lsrs[i].type), ifa, &type, &domain);

    DBG("Processing requested LSA: Type: %04x, Id: %R, Rt: %R\n", type, id, rt);

    en = ospf_hash_find(p->gr, domain, id, rt, type);
    if (!en)
    {
      LOG_LSA1("Bad LSR (Type: %04x, Id: %R, Rt: %R) in LSREQ", type, id, rt);
      LOG_LSA2("  received from nbr %R on %s - LSA is missing", n->rid, ifa->ifname);

      ospf_neigh_sm(n, INM_BADLSREQ);
      return;
    }

    entries[i] = en;
  }

  ospf_send_lsupd(p, entries, lsr_count, n);
}
