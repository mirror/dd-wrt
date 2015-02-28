/*
 *	BIRD -- OSPF
 *
 *	(c) 1999--2004 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "ospf.h"


#ifdef OSPFv2
struct ospf_hello_packet
{
  struct ospf_packet ospf_packet;
  ip_addr netmask;
  u16 helloint;
  u8 options;
  u8 priority;
  u32 deadint;
  u32 dr;
  u32 bdr;
};
#endif


#ifdef OSPFv3
struct ospf_hello_packet
{
  struct ospf_packet ospf_packet;
  u32 iface_id;
  u8 priority;
  u8 options3;
  u8 options2;
  u8 options;
  u16 helloint;
  u16 deadint;
  u32 dr;
  u32 bdr;
};
#endif


void
ospf_hello_receive(struct ospf_packet *ps_i, struct ospf_iface *ifa,
		   struct ospf_neighbor *n, ip_addr faddr)
{
  struct proto_ospf *po = ifa->oa->po;
  struct proto *p = &po->proto;
  char *beg = "OSPF: Bad HELLO packet from ";
  unsigned int size, i, twoway, peers;
  u32 tmp;
  u32 *pnrid;

  size = ntohs(ps_i->length);
  if (size < sizeof(struct ospf_hello_packet))
  {
    log(L_ERR "%s%I - too short (%u B)", beg, faddr, size);
    return;
  }

  struct ospf_hello_packet *ps = (void *) ps_i;

  OSPF_TRACE(D_PACKETS, "HELLO packet received from %I via %s", faddr, ifa->ifname);

#ifdef OSPFv2
  ip_addr mask = ps->netmask;
  ipa_ntoh(mask);
  if ((ifa->type != OSPF_IT_VLINK) &&
      (ifa->type != OSPF_IT_PTP) &&
      !ipa_equal(mask, ipa_mkmask(ifa->addr->pxlen)))
  {
    log(L_ERR "%s%I - netmask mismatch (%I)", beg, faddr, mask);
    return;
  }
#endif

  tmp = ntohs(ps->helloint);
  if (tmp != ifa->helloint)
  {
    log(L_ERR "%s%I - hello interval mismatch (%d)", beg, faddr, tmp);
    return;
  }

#ifdef OSPFv2
  tmp = ntohl(ps->deadint);
#else /* OSPFv3 */
  tmp = ntohs(ps->deadint);
#endif
  if (tmp != ifa->deadint)
  {
    log(L_ERR "%s%I - dead interval mismatch (%d)", beg, faddr, tmp);
    return;
  }

  /* Check whether bits E, N match */
  if ((ps->options ^ ifa->oa->options) & (OPT_E | OPT_N))
  {
    log(L_ERR "%s%I - area type mismatch (%x)", beg, faddr, ps->options);
    return;
  }

#ifdef OSPFv2
  if (n && (n->rid != ntohl(ps_i->routerid)))
  {
    OSPF_TRACE(D_EVENTS,
	"Neighbor %I has changed router id from %R to %R.",
	     n->ip, n->rid, ntohl(ps_i->routerid));
    ospf_neigh_remove(n);
    n = NULL;
  }
#endif

  if (!n)
  {
    if ((ifa->type == OSPF_IT_NBMA) || (ifa->type == OSPF_IT_PTMP))
    {
      struct nbma_node *nn = find_nbma_node(ifa, faddr);

      if (!nn && ifa->strictnbma)
      {
	log(L_WARN "Ignoring new neighbor: %I on %s", faddr, ifa->ifname);
	return;
      }

      if (nn && (ifa->type == OSPF_IT_NBMA) &&
	  (((ps->priority == 0) && nn->eligible) ||
	   ((ps->priority > 0) && !nn->eligible)))
      {
	log(L_ERR "Eligibility mismatch for neighbor: %I on %s", faddr, ifa->ifname);
	return;
      }

      if (nn)
	nn->found = 1;
    }

    OSPF_TRACE(D_EVENTS, "New neighbor found: %I on %s", faddr, ifa->ifname);

    n = ospf_neighbor_new(ifa);

    n->rid = ntohl(ps_i->routerid);
    n->ip = faddr;
    n->dr = ntohl(ps->dr);
    n->bdr = ntohl(ps->bdr);
    n->priority = ps->priority;
#ifdef OSPFv3
    n->iface_id = ntohl(ps->iface_id);
#endif

    if (n->ifa->cf->bfd)
      ospf_neigh_update_bfd(n, n->ifa->bfd);
  }
#ifdef OSPFv3	/* NOTE: this could also be relevant for OSPFv2 on PtP ifaces */
  else if (!ipa_equal(faddr, n->ip))
  {
    OSPF_TRACE(D_EVENTS, "Neighbor address changed from %I to %I", n->ip, faddr);
    n->ip = faddr;
  }
#endif

  ospf_neigh_sm(n, INM_HELLOREC);

  pnrid = (u32 *) ((struct ospf_hello_packet *) (ps + 1));

  peers = (size - sizeof(struct ospf_hello_packet))/ sizeof(u32);

  twoway = 0;
  for (i = 0; i < peers; i++)
  {
    if (ntohl(pnrid[i]) == po->router_id)
    {
      DBG("%s: Twoway received from %I\n", p->name, faddr);
      ospf_neigh_sm(n, INM_2WAYREC);
      twoway = 1;
      break;
    }
  }

  if (!twoway)
    ospf_neigh_sm(n, INM_1WAYREC);

  u32 olddr = n->dr;
  u32 oldbdr = n->bdr;
  u32 oldpriority = n->priority;
#ifdef OSPFv3
  u32 oldiface_id = n->iface_id;
#endif

  n->dr = ntohl(ps->dr);
  n->bdr = ntohl(ps->bdr);
  n->priority = ps->priority;
#ifdef OSPFv3
  n->iface_id = ntohl(ps->iface_id);
#endif


  /* Check priority change */
  if (n->state >= NEIGHBOR_2WAY)
  {
#ifdef OSPFv2
    u32 neigh = ipa_to_u32(n->ip);
#else /* OSPFv3 */
    u32 neigh = n->rid;
#endif

    if (n->priority != oldpriority)
      ospf_iface_sm(ifa, ISM_NEICH);

#ifdef OSPFv3
    if (n->iface_id != oldiface_id)
      ospf_iface_sm(ifa, ISM_NEICH);
#endif

    /* Neighbor is declaring itself ad DR and there is no BDR */
    if ((n->dr == neigh) && (n->bdr == 0)
	&& (n->state != NEIGHBOR_FULL))
      ospf_iface_sm(ifa, ISM_BACKS);

    /* Neighbor is declaring itself as BDR */
    if ((n->bdr == neigh) && (n->state != NEIGHBOR_FULL))
      ospf_iface_sm(ifa, ISM_BACKS);

    /* Neighbor is newly declaring itself as DR or BDR */
    if (((n->dr == neigh) && (n->dr != olddr))
	|| ((n->bdr == neigh) && (n->bdr != oldbdr)))
      ospf_iface_sm(ifa, ISM_NEICH);

    /* Neighbor is no more declaring itself as DR or BDR */
    if (((olddr == neigh) && (n->dr != olddr))
	|| ((oldbdr == neigh) && (n->bdr != oldbdr)))
      ospf_iface_sm(ifa, ISM_NEICH);
  }

  if (ifa->type == OSPF_IT_NBMA)
  {
    if ((ifa->priority == 0) && (n->priority > 0))
      ospf_hello_send(n->ifa, OHS_HELLO, n);
  }
  ospf_neigh_sm(n, INM_HELLOREC);
}

void
ospf_hello_send(struct ospf_iface *ifa, int kind, struct ospf_neighbor *dirn)
{
  struct ospf_hello_packet *pkt;
  struct ospf_packet *op;
  struct proto *p;
  struct ospf_neighbor *neigh, *n1;
  u16 length;
  int i;
  struct nbma_node *nb;

  if (ifa->state <= OSPF_IS_LOOP)
    return;

  if (ifa->stub)
    return;			/* Don't send any packet on stub iface */

  p = (struct proto *) (ifa->oa->po);
  DBG("%s: Hello/Poll timer fired on interface %s with IP %I\n",
      p->name, ifa->ifname, ifa->addr->ip);

  /* Now we should send a hello packet */
  pkt = ospf_tx_buffer(ifa);
  op = &pkt->ospf_packet;

  /* Now fill ospf_hello header */
  ospf_pkt_fill_hdr(ifa, pkt, HELLO_P);

#ifdef OSPFv2
  pkt->netmask = ipa_mkmask(ifa->addr->pxlen);
  ipa_hton(pkt->netmask);
  if ((ifa->type == OSPF_IT_VLINK) ||
      ((ifa->type == OSPF_IT_PTP) && !ifa->ptp_netmask))
    pkt->netmask = IPA_NONE;
#endif

  pkt->helloint = ntohs(ifa->helloint);
  pkt->priority = ifa->priority;

#ifdef OSPFv3
  pkt->iface_id = htonl(ifa->iface_id);

  pkt->options3 = ifa->oa->options >> 16;
  pkt->options2 = ifa->oa->options >> 8;
#endif
  pkt->options = ifa->oa->options;

#ifdef OSPFv2
  pkt->deadint = htonl(ifa->deadint);
  pkt->dr = htonl(ipa_to_u32(ifa->drip));
  pkt->bdr = htonl(ipa_to_u32(ifa->bdrip));
#else /* OSPFv3 */
  pkt->deadint = htons(ifa->deadint);
  pkt->dr = htonl(ifa->drid);
  pkt->bdr = htonl(ifa->bdrid);
#endif

  /* Fill all neighbors */
  i = 0;

  if (kind != OHS_SHUTDOWN)
  {
    u32 *pp = (u32 *) (((u8 *) pkt) + sizeof(struct ospf_hello_packet));
    WALK_LIST(neigh, ifa->neigh_list)
    {
      if ((i+1) * sizeof(u32) + sizeof(struct ospf_hello_packet) > ospf_pkt_maxsize(ifa))
      {
	log(L_WARN "%s: Too many neighbors on interface %s", p->name, ifa->ifname);
	break;
      }
      *(pp + i) = htonl(neigh->rid);
      i++;
    }
  }

  length = sizeof(struct ospf_hello_packet) + i * sizeof(u32);
  op->length = htons(length);

  switch(ifa->type)
  {
  case OSPF_IT_BCAST:
  case OSPF_IT_PTP:
    ospf_send_to_all(ifa);
    break;

  case OSPF_IT_NBMA:
    if (dirn)		/* Response to received hello */
    {
      ospf_send_to(ifa, dirn->ip);
      break;
    }

    int to_all = ifa->state > OSPF_IS_DROTHER;
    int me_elig = ifa->priority > 0;
 
    if (kind == OHS_POLL)	/* Poll timer */
    {
      WALK_LIST(nb, ifa->nbma_list)
	if (!nb->found && (to_all || (me_elig && nb->eligible)))
	  ospf_send_to(ifa, nb->ip);
    }
    else			/* Hello timer */
    {
      WALK_LIST(n1, ifa->neigh_list)
	if (to_all || (me_elig && (n1->priority > 0)) ||
	    (n1->rid == ifa->drid) || (n1->rid == ifa->bdrid))
	  ospf_send_to(ifa, n1->ip);
    }
    break;

  case OSPF_IT_PTMP:
    WALK_LIST(n1, ifa->neigh_list)
      ospf_send_to(ifa, n1->ip);

    WALK_LIST(nb, ifa->nbma_list)
      if (!nb->found)
	ospf_send_to(ifa, nb->ip);

    /* If there is no other target, we also send HELLO packet to the other end */
    if (ipa_nonzero(ifa->addr->opposite) && !ifa->strictnbma &&
	EMPTY_LIST(ifa->neigh_list) && EMPTY_LIST(ifa->nbma_list))
      ospf_send_to(ifa, ifa->addr->opposite);
    break;

  case OSPF_IT_VLINK:
    ospf_send_to(ifa, ifa->vip);
    break;

  default:
    bug("Bug in ospf_hello_send()");
  }

  OSPF_TRACE(D_PACKETS, "HELLO packet sent via %s", ifa->ifname);
}
