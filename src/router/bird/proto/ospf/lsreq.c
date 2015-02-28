/*
 *	BIRD -- OSPF
 *
 *	(c) 2000--2004 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "ospf.h"


struct ospf_lsreq_packet
{
  struct ospf_packet ospf_packet;
  struct ospf_lsreq_header lsh[];
};


static void ospf_dump_lsreq(struct proto *p, struct ospf_lsreq_packet *pkt)
{
  struct ospf_packet *op = &pkt->ospf_packet;

  ASSERT(op->type == LSREQ_P);
  ospf_dump_common(p, op);

  unsigned int i, j;
  j = (ntohs(op->length) - sizeof(struct ospf_lsreq_packet)) /
    sizeof(struct ospf_lsreq_header);

  for (i = 0; i < j; i++)
    log(L_TRACE "%s:     LSR      Type: %04x, Id: %R, Rt: %R", p->name,
	htonl(pkt->lsh[i].type), htonl(pkt->lsh[i].id), htonl(pkt->lsh[i].rt));
}

void
ospf_lsreq_send(struct ospf_neighbor *n)
{
  snode *sn;
  struct top_hash_entry *en;
  struct ospf_lsreq_packet *pk;
  struct ospf_packet *op;
  struct ospf_lsreq_header *lsh;
  u16 length;
  int i, j;
  struct proto *p = &n->ifa->oa->po->proto;

  pk = ospf_tx_buffer(n->ifa);
  op = &pk->ospf_packet;

  ospf_pkt_fill_hdr(n->ifa, pk, LSREQ_P);

  sn = SHEAD(n->lsrql);
  if (EMPTY_SLIST(n->lsrql))
  {
    if (n->state == NEIGHBOR_LOADING)
      ospf_neigh_sm(n, INM_LOADDONE);
    return;
  }

  i = j = (ospf_pkt_maxsize(n->ifa) - sizeof(struct ospf_lsreq_packet)) /
    sizeof(struct ospf_lsreq_header);
  lsh = pk->lsh;

  for (; i > 0; i--)
  {
    en = (struct top_hash_entry *) sn;
    lsh->type = htonl(en->lsa.type);
    lsh->rt = htonl(en->lsa.rt);
    lsh->id = htonl(en->lsa.id);
    DBG("Requesting %uth LSA: Type: %u, ID: %R, RT: %R, SN: 0x%x, Age %u\n",
	i, en->lsa.type, en->lsa.id, en->lsa.rt, en->lsa.sn, en->lsa.age);
    lsh++;
    if (sn == STAIL(n->lsrql))
      break;
    sn = sn->next;
  }
  if (i != 0)
    i--;

  length =
    sizeof(struct ospf_lsreq_packet) + (j -
					i) * sizeof(struct ospf_lsreq_header);
  op->length = htons(length);

  OSPF_PACKET(ospf_dump_lsreq, pk, "LSREQ packet sent to %I via %s", n->ip, n->ifa->ifname);
  ospf_send_to(n->ifa, n->ip);
}

void
ospf_lsreq_receive(struct ospf_packet *ps_i, struct ospf_iface *ifa,
		   struct ospf_neighbor *n)
{
  struct ospf_area *oa = ifa->oa;
  struct proto_ospf *po = oa->po;
  struct proto *p = &po->proto;
  struct ospf_lsreq_header *lsh;
  struct l_lsr_head *llsh;
  list uplist;
  slab *upslab;
  int i, lsano;

  unsigned int size = ntohs(ps_i->length);
  if (size < sizeof(struct ospf_lsreq_packet))
  {
    log(L_ERR "Bad OSPF LSREQ packet from %I -  too short (%u B)", n->ip, size);
    return;
  }

  struct ospf_lsreq_packet *ps = (void *) ps_i;
  OSPF_PACKET(ospf_dump_lsreq, ps, "LSREQ packet received from %I via %s", n->ip, ifa->ifname);

  if (n->state < NEIGHBOR_EXCHANGE)
    return;

  ospf_neigh_sm(n, INM_HELLOREC);

  lsh = ps->lsh;
  init_list(&uplist);
  upslab = sl_new(n->pool, sizeof(struct l_lsr_head));

  lsano = (size - sizeof(struct ospf_lsreq_packet)) /
    sizeof(struct ospf_lsreq_header);
  for (i = 0; i < lsano; lsh++, i++)
  {
    u32 hid = ntohl(lsh->id);
    u32 hrt = ntohl(lsh->rt);
    u32 htype = ntohl(lsh->type);
    u32 dom = ospf_lsa_domain(htype, ifa);
    DBG("Processing requested LSA: Type: %u, ID: %R, RT: %R\n", lsh->type, hid, hrt);
    llsh = sl_alloc(upslab);
    llsh->lsh.id = hid;
    llsh->lsh.rt = hrt;
    llsh->lsh.type = htype;
    add_tail(&uplist, NODE llsh);
    if (ospf_hash_find(po->gr, dom, hid, hrt, htype) == NULL)
    {
      log(L_WARN "Received bad LSREQ from %I: Type: %04x, Id: %R, Rt: %R",
	  n->ip, htype, hid, hrt);
      ospf_neigh_sm(n, INM_BADLSREQ);
      rfree(upslab);
      return;
    }
  }
  ospf_lsupd_send_list(n, &uplist);
  rfree(upslab);
}
