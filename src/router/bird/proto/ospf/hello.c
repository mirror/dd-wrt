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
  unsigned int size, i, twoway, eligible, peers;
  u32 tmp;
  u32 *pnrid;

  size = ntohs(ps_i->length);
  if (size < sizeof(struct ospf_hello_packet))
  {
    log(L_ERR "%s%I - too short (%u B)", beg, faddr, size);
    return;
  }

  struct ospf_hello_packet *ps = (void *) ps_i;

  OSPF_TRACE(D_PACKETS, "HELLO packet received from %I via %s%s", faddr,
      (ifa->type == OSPF_IT_VLINK ? "vlink-" : ""), ifa->iface->name);

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
  if (tmp != ifa->dead)
  {
    log(L_ERR "%s%I - dead interval mismatch (%d)", beg, faddr, tmp);
    return;
  }

  tmp = !(ps->options & OPT_E);
  if (tmp != !!ifa->oa->stub)
  {
    log(L_ERR "%s%I - stub area flag mismatch (%d)", beg, faddr, tmp);
    return;
  }

  if (!n)
  {
    if ((ifa->type == OSPF_IT_NBMA))
    {
      struct nbma_node *nn;
      int found = 0;

      WALK_LIST(nn, ifa->nbma_list)
      {
	if (ipa_equal(faddr, nn->ip))
	{
	  found = 1;
	  break;
	}
      }
      if ((found == 0) && (ifa->strictnbma))
      {
	log(L_WARN "Ignoring new neighbor: %I on %s", faddr,
	    ifa->iface->name);
	return;
      }
      if (found)
      {
	eligible = nn->eligible;
	if (((ps->priority == 0) && eligible)
	    || ((ps->priority > 0) && (eligible == 0)))
	{
	  log(L_ERR "Eligibility mismatch for neighbor: %I on %s",
	      faddr, ifa->iface->name);
	  return;
	}
      }
    }
    OSPF_TRACE(D_EVENTS, "New neighbor found: %I on %s", faddr,
	       ifa->iface->name);

    n = ospf_neighbor_new(ifa);

    n->rid = ntohl(((struct ospf_packet *) ps)->routerid);
    n->ip = faddr;
    n->dr = ntohl(ps->dr);
    n->bdr = ntohl(ps->bdr);
    n->priority = ps->priority;
#ifdef OSPFv3
    n->iface_id = ntohl(ps->iface_id);
#endif
  }
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
      ospf_hello_send(NULL, 0, n);
  }
  ospf_neigh_sm(n, INM_HELLOREC);
}

void
ospf_hello_send(timer *timer, int poll, struct ospf_neighbor *dirn)
{
  struct ospf_iface *ifa;
  struct ospf_hello_packet *pkt;
  struct ospf_packet *op;
  struct proto *p;
  struct ospf_neighbor *neigh, *n1;
  u16 length;
  u32 *pp;
  int i, send;
  struct nbma_node *nb;

  if (timer == NULL)
    ifa = dirn->ifa;
  else
    ifa = (struct ospf_iface *) timer->data;

  if (ifa->state == OSPF_IS_DOWN)
    return;

  if (ifa->stub)
    return;			/* Don't send any packet on stub iface */

  p = (struct proto *) (ifa->oa->po);
  DBG("%s: Hello/Poll timer fired on interface %s with IP %I\n",
      p->name, ifa->iface->name, ifa->addr->ip);

  /* Now we should send a hello packet */
  pkt = ospf_tx_buffer(ifa);
  op = &pkt->ospf_packet;

  /* Now fill ospf_hello header */
  ospf_pkt_fill_hdr(ifa, pkt, HELLO_P);

#ifdef OSPFv2
  pkt->netmask = ipa_mkmask(ifa->addr->pxlen);
  ipa_hton(pkt->netmask);
  if ((ifa->type == OSPF_IT_VLINK) || (ifa->type == OSPF_IT_PTP))
    pkt->netmask = IPA_NONE;
#endif

  pkt->helloint = ntohs(ifa->helloint);
  pkt->priority = ifa->priority;

#ifdef OSPFv3
  pkt->iface_id = htonl(ifa->iface->index);

  pkt->options3 = ifa->oa->options >> 16;
  pkt->options2 = ifa->oa->options >> 8;
#endif
  pkt->options = ifa->oa->options;

#ifdef OSPFv2
  pkt->deadint = htonl(ifa->dead);
  pkt->dr = htonl(ipa_to_u32(ifa->drip));
  pkt->bdr = htonl(ipa_to_u32(ifa->bdrip));
#else /* OSPFv3 */
  pkt->deadint = htons(ifa->dead);
  pkt->dr = htonl(ifa->drid);
  pkt->bdr = htonl(ifa->bdrid);
#endif

  /* Fill all neighbors */
  i = 0;
  pp = (u32 *) (((u8 *) pkt) + sizeof(struct ospf_hello_packet));
  WALK_LIST(neigh, ifa->neigh_list)
  {
    if ((i+1) * sizeof(u32) + sizeof(struct ospf_hello_packet) > ospf_pkt_maxsize(ifa))
    {
      OSPF_TRACE(D_PACKETS, "Too many neighbors on the interface!");
      break;
    }
    *(pp + i) = htonl(neigh->rid);
    i++;
  }

  length = sizeof(struct ospf_hello_packet) + i * sizeof(u32);
  op->length = htons(length);

  switch(ifa->type)
  {
    case OSPF_IT_NBMA:
      if (timer == NULL)		/* Response to received hello */
      {
        ospf_send_to(ifa, dirn->ip);
      }
      else
      {
        int toall = 0;
        int meeli = 0;
        if (ifa->state > OSPF_IS_DROTHER)
          toall = 1;
        if (ifa->priority > 0)
          meeli = 1;
 
        WALK_LIST(nb, ifa->nbma_list)
        {
          send = 1;
          WALK_LIST(n1, ifa->neigh_list)
          {
            if (ipa_equal(nb->ip, n1->ip))
            {
              send = 0;
              break;
            }
          }
          if ((poll == 1) && (send))
          {
            if (toall || (meeli && nb->eligible))
              ospf_send_to(ifa, nb->ip);
          }
        }
        if (poll == 0)
        {
          WALK_LIST(n1, ifa->neigh_list)
          {
            if (toall || (n1->rid == ifa->drid) || (n1->rid == ifa->bdrid) ||
                (meeli && (n1->priority > 0)))
              ospf_send_to(ifa, n1->ip);
          }
        }
      }
      break;
    case OSPF_IT_VLINK:
      ospf_send_to(ifa, ifa->vip);
      break;
    default:
      ospf_send_to(ifa, AllSPFRouters);
  }

  OSPF_TRACE(D_PACKETS, "HELLO packet sent via %s%s",
	     (ifa->type == OSPF_IT_VLINK ? "vlink-" : ""), ifa->iface->name);
}
