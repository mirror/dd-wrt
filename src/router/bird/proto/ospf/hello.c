/*
 *	BIRD -- OSPF
 *
 *	(c) 1999--2004 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "ospf.h"

void
ospf_hello_receive(struct ospf_hello_packet *ps,
		   struct ospf_iface *ifa, struct ospf_neighbor *n, ip_addr faddr)
{
  u32 *pnrid;
  ip_addr olddr, oldbdr;
  ip_addr mask;
  char *beg = "Bad OSPF hello packet from ", *rec = " received: ";
  struct proto *p = (struct proto *) ifa->oa->po;
  unsigned int size = ntohs(ps->ospf_packet.length), i, twoway, oldpriority, eligible = 0, peers;

  OSPF_TRACE(D_PACKETS, "HELLO packet received from %I via %s%s", faddr,
      (ifa->type == OSPF_IT_VLINK ? "vlink-" : ""), ifa->iface->name);
  mask = ps->netmask;
  ipa_ntoh(mask);

  if (ifa->type != OSPF_IT_VLINK)
    {
      char *msg = L_WARN "Received HELLO packet %s (%I) is inconsistent "
	"with the primary address of interface %s.";

      if ((ifa->type != OSPF_IT_PTP) &&
	  !ipa_equal(mask, ipa_mkmask(ifa->iface->addr->pxlen)))
	{
	  if (!n) log(msg, "netmask", mask, ifa->iface->name);
	  return;
	}

      /* This check is not specified in RFC 2328, but it is needed
       * to handle the case when there is more IP networks on one
       * physical network (which is not handled in RFC 2328).
       * We allow OSPF on primary IP address only and ignore HELLO packets
       * with secondary addresses (which are sent for example by Quagga.
       */
      if ((ifa->iface->addr->flags & IA_UNNUMBERED) ?
	  !ipa_equal(faddr, ifa->iface->addr->opposite) :
	  !ipa_equal(ipa_and(faddr,mask), ifa->iface->addr->prefix))
	{
	  if (!n) log(msg, "address", faddr, ifa->iface->name);
	  return;
	}
    }

  if (ntohs(ps->helloint) != ifa->helloint)
  {
    log(L_ERR "%s%I%shello interval mismatch (%d).", beg, faddr, rec,
	ntohs(ps->helloint));
    return;
  }

  if (ntohl(ps->deadint) != ifa->dead)
  {
    log(L_ERR "%s%I%sdead interval mismatch (%d).", beg, faddr, rec,
	ntohl(ps->deadint));
    return;
  }

  if (ps->options != ifa->oa->opt.byte)
  {
    log(L_ERR "%s%I%soptions mismatch (0x%x).", beg, faddr, rec, ps->options);
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
	log(L_WARN "Ignoring new neighbor: %I on %s.", faddr,
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
    OSPF_TRACE(D_EVENTS, "New neighbor found: %I on %s.", faddr,
	       ifa->iface->name);

    n = ospf_neighbor_new(ifa);

    n->rid = ntohl(((struct ospf_packet *) ps)->routerid);
    n->ip = faddr;
    n->dr = ps->dr;
    ipa_ntoh(n->dr);
    n->bdr = ps->bdr;
    ipa_ntoh(n->bdr);
    n->priority = ps->priority;
    n->options = ps->options;
  }
  ospf_neigh_sm(n, INM_HELLOREC);

  pnrid = (u32 *) ((struct ospf_hello_packet *) (ps + 1));

  peers = (size - sizeof(struct ospf_hello_packet))/ sizeof(u32);

  twoway = 0;
  for (i = 0; i < peers; i++)
  {
    if (ntohl(*(pnrid + i)) == p->cf->global->router_id)
    {
      DBG("%s: Twoway received from %I\n", p->name, faddr);
      ospf_neigh_sm(n, INM_2WAYREC);
      twoway = 1;
      break;
    }
  }

  if (!twoway)
    ospf_neigh_sm(n, INM_1WAYREC);

  olddr = n->dr;
  n->dr = ipa_ntoh(ps->dr);
  oldbdr = n->bdr;
  n->bdr = ipa_ntoh(ps->bdr);
  oldpriority = n->priority;
  n->priority = ps->priority;

  /* Check priority change */
  if (n->state >= NEIGHBOR_2WAY)
  {
    if (n->priority != oldpriority)
      ospf_iface_sm(ifa, ISM_NEICH);

    /* Router is declaring itself ad DR and there is no BDR */
    if (ipa_equal(n->ip, n->dr) && (ipa_to_u32(n->bdr) == 0)
	&& (n->state != NEIGHBOR_FULL))
      ospf_iface_sm(ifa, ISM_BACKS);

    /* Neighbor is declaring itself as BDR */
    if (ipa_equal(n->ip, n->bdr) && (n->state != NEIGHBOR_FULL))
      ospf_iface_sm(ifa, ISM_BACKS);

    /* Neighbor is newly declaring itself as DR or BDR */
    if ((ipa_equal(n->ip, n->dr) && (!ipa_equal(n->dr, olddr)))
	|| (ipa_equal(n->ip, n->bdr) && (!ipa_equal(n->bdr, oldbdr))))
      ospf_iface_sm(ifa, ISM_NEICH);

    /* Neighbor is no more declaring itself as DR or BDR */
    if ((ipa_equal(n->ip, olddr) && (!ipa_equal(n->dr, olddr)))
	|| (ipa_equal(n->ip, oldbdr) && (!ipa_equal(n->bdr, oldbdr))))
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
ospf_hello_send(timer * timer, int poll, struct ospf_neighbor *dirn)
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
  DBG("%s: Hello/Poll timer fired on interface %s.\n",
      p->name, ifa->iface->name);
  /* Now we should send a hello packet */
  /* First a common packet header */
  if ((ifa->type == OSPF_IT_NBMA) || (ifa->type == OSPF_IT_VLINK))
  {
    pkt = (struct ospf_hello_packet *) (ifa->ip_sk->tbuf);
  }
  else
  {
    pkt = (struct ospf_hello_packet *) (ifa->hello_sk->tbuf);
  }

  /* Now fill ospf_hello header */
  op = (struct ospf_packet *) pkt;

  ospf_pkt_fill_hdr(ifa, pkt, HELLO_P);

  pkt->netmask = ipa_mkmask(ifa->iface->addr->pxlen);
  ipa_hton(pkt->netmask);
  if ((ifa->type == OSPF_IT_VLINK) || (ifa->type == OSPF_IT_PTP))
    pkt->netmask = IPA_NONE;
  pkt->helloint = ntohs(ifa->helloint);
  pkt->options = ifa->oa->opt.byte;
  pkt->priority = ifa->priority;
  pkt->deadint = htonl(ifa->dead);
  pkt->dr = ifa->drip;
  ipa_hton(pkt->dr);
  pkt->bdr = ifa->bdrip;
  ipa_hton(pkt->bdr);

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
        ospf_send_to(ifa->ip_sk, dirn->ip, ifa);
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
              ospf_send_to(ifa->ip_sk, nb->ip, ifa);
          }
        }
        if (poll == 0)
        {
          WALK_LIST(n1, ifa->neigh_list)
          {
            if (toall || (n1->rid == ifa->drid) || (n1->rid == ifa->bdrid) ||
                (meeli && (n1->priority > 0)))
              ospf_send_to(ifa->ip_sk, n1->ip, ifa);
          }
        }
      }
      break;
    case OSPF_IT_VLINK:
      ospf_send_to(ifa->ip_sk, ifa->vip, ifa);
      break;
    default:
      ospf_send_to(ifa->hello_sk, IPA_NONE, ifa);
  }

  OSPF_TRACE(D_PACKETS, "HELLO packet sent via %s%s",
	     (ifa->type == OSPF_IT_VLINK ? "vlink-" : ""), ifa->iface->name);
}
