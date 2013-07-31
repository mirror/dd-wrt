/*
 *	BIRD -- OSPF
 *
 *	(c) 1999--2005 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "ospf.h"
#include "nest/password.h"
#include "lib/md5.h"

void
ospf_pkt_fill_hdr(struct ospf_iface *ifa, void *buf, u8 h_type)
{
  struct proto_ospf *po = ifa->oa->po;
  struct ospf_packet *pkt;

  pkt = (struct ospf_packet *) buf;

  pkt->version = OSPF_VERSION;

  pkt->type = h_type;

  pkt->routerid = htonl(po->router_id);
  pkt->areaid = htonl(ifa->oa->areaid);

#ifdef OSPFv3
  pkt->instance_id = ifa->instance_id;
#endif

#ifdef OSPFv2
  pkt->autype = htons(ifa->autype);
#endif

  pkt->checksum = 0;
}

unsigned
ospf_pkt_maxsize(struct ospf_iface *ifa)
{
  unsigned mtu = (ifa->type == OSPF_IT_VLINK) ? OSPF_VLINK_MTU : ifa->iface->mtu;
  unsigned headers = SIZE_OF_IP_HEADER;

#ifdef OSPFv2
  if (ifa->autype == OSPF_AUTH_CRYPT)
    headers += OSPF_AUTH_CRYPT_SIZE;
#endif

  return mtu - headers;
}

#ifdef OSPFv2

static void
ospf_pkt_finalize(struct ospf_iface *ifa, struct ospf_packet *pkt)
{
  struct password_item *passwd = NULL;
  void *tail;
  struct MD5Context ctxt;
  char password[OSPF_AUTH_CRYPT_SIZE];

  pkt->checksum = 0;
  pkt->autype = htons(ifa->autype);
  bzero(&pkt->u, sizeof(union ospf_auth));

  /* Compatibility note: pkt->u may contain anything if autype is
     none, but nonzero values do not work with Mikrotik OSPF */

  switch(ifa->autype)
  {
    case OSPF_AUTH_SIMPLE:
      passwd = password_find(ifa->passwords, 1);
      if (!passwd)
      {
        log( L_ERR "No suitable password found for authentication" );
        return;
      }
      password_cpy(pkt->u.password, passwd->password, sizeof(union ospf_auth));
    case OSPF_AUTH_NONE:
      pkt->checksum = ipsum_calculate(pkt, sizeof(struct ospf_packet) -
                                  sizeof(union ospf_auth), (pkt + 1),
				  ntohs(pkt->length) -
				  sizeof(struct ospf_packet), NULL);
      break;
    case OSPF_AUTH_CRYPT:
      passwd = password_find(ifa->passwords, 0);
      if (!passwd)
      {
        log( L_ERR "No suitable password found for authentication" );
        return;
      }

      /* Perhaps use random value to prevent replay attacks after
	 reboot when system does not have independent RTC? */
      if (!ifa->csn)
	{
	  ifa->csn = (u32) now;
	  ifa->csn_use = now;
	}

      /* We must have sufficient delay between sending a packet and increasing 
	 CSN to prevent reordering of packets (in a network) with different CSNs */
      if ((now - ifa->csn_use) > 1)
	ifa->csn++;

      ifa->csn_use = now;

      pkt->u.md5.keyid = passwd->id;
      pkt->u.md5.len = OSPF_AUTH_CRYPT_SIZE;
      pkt->u.md5.zero = 0;
      pkt->u.md5.csn = htonl(ifa->csn);
      tail = ((void *)pkt) + ntohs(pkt->length);
      MD5Init(&ctxt);
      MD5Update(&ctxt, (char *) pkt, ntohs(pkt->length));
      password_cpy(password, passwd->password, OSPF_AUTH_CRYPT_SIZE);
      MD5Update(&ctxt, password, OSPF_AUTH_CRYPT_SIZE);
      MD5Final(tail, &ctxt);
      break;
    default:
      bug("Unknown authentication type");
  }
}

static int
ospf_pkt_checkauth(struct ospf_neighbor *n, struct ospf_iface *ifa, struct ospf_packet *pkt, int size)
{
  struct proto_ospf *po = ifa->oa->po;
  struct proto *p = &po->proto;
  struct password_item *pass = NULL, *ptmp;
  void *tail;
  char md5sum[OSPF_AUTH_CRYPT_SIZE];
  char password[OSPF_AUTH_CRYPT_SIZE];
  struct MD5Context ctxt;


  if (pkt->autype != htons(ifa->autype))
  {
    OSPF_TRACE(D_PACKETS, "OSPF_auth: Method differs (%d)", ntohs(pkt->autype));
    return 0;
  }

  switch(ifa->autype)
  {
    case OSPF_AUTH_NONE:
      return 1;
      break;
    case OSPF_AUTH_SIMPLE:
      pass = password_find(ifa->passwords, 1);
      if (!pass)
      {
        OSPF_TRACE(D_PACKETS, "OSPF_auth: no password found");
	return 0;
      }
      password_cpy(password, pass->password, sizeof(union ospf_auth));

      if (memcmp(pkt->u.password, password, sizeof(union ospf_auth)))
      {
        char ppass[sizeof(union ospf_auth) + 1];
        bzero(ppass, (sizeof(union ospf_auth) + 1));
        memcpy(ppass, pkt->u.password, sizeof(union ospf_auth));
        OSPF_TRACE(D_PACKETS, "OSPF_auth: different passwords (%s)", ppass);
	return 0;
      }
      return 1;
      break;
    case OSPF_AUTH_CRYPT:
      if (pkt->u.md5.len != OSPF_AUTH_CRYPT_SIZE)
      {
        OSPF_TRACE(D_PACKETS, "OSPF_auth: wrong size of md5 digest");
        return 0;
      }

      if (ntohs(pkt->length) + OSPF_AUTH_CRYPT_SIZE > size)
      {
        OSPF_TRACE(D_PACKETS, "OSPF_auth: size mismatch (%d vs %d)",
	  ntohs(pkt->length) + OSPF_AUTH_CRYPT_SIZE, size);
        return 0;
      }

      tail = ((void *)pkt) + ntohs(pkt->length);

      if (ifa->passwords)
      {
	WALK_LIST(ptmp, *(ifa->passwords))
	{
	  if (pkt->u.md5.keyid != ptmp->id) continue;
	  if ((ptmp->accfrom > now_real) || (ptmp->accto < now_real)) continue;
	  pass = ptmp;
	  break;
	}
      }

      if (!pass)
      {
        OSPF_TRACE(D_PACKETS, "OSPF_auth: no suitable md5 password found");
        return 0;
      }

      if (n)
      {
	u32 rcv_csn = ntohl(pkt->u.md5.csn);
	if(rcv_csn < n->csn)
	{
	  OSPF_TRACE(D_PACKETS, "OSPF_auth: lower sequence number (rcv %d, old %d)", rcv_csn, n->csn);
	  return 0;
	}

	n->csn = rcv_csn;
      }

      MD5Init(&ctxt);
      MD5Update(&ctxt, (char *) pkt, ntohs(pkt->length));
      password_cpy(password, pass->password, OSPF_AUTH_CRYPT_SIZE);
      MD5Update(&ctxt, password, OSPF_AUTH_CRYPT_SIZE);
      MD5Final(md5sum, &ctxt);
      if (memcmp(md5sum, tail, OSPF_AUTH_CRYPT_SIZE))
      {
        OSPF_TRACE(D_PACKETS, "OSPF_auth: wrong md5 digest");
        return 0;
      }
      return 1;
      break;
    default:
      OSPF_TRACE(D_PACKETS, "OSPF_auth: unknown auth type");
      return 0;
  }
}

#else

/* OSPFv3 authentication not yet supported */

static inline void
ospf_pkt_finalize(struct ospf_iface *ifa, struct ospf_packet *pkt)
{ }

static int
ospf_pkt_checkauth(struct ospf_neighbor *n, struct ospf_iface *ifa, struct ospf_packet *pkt, int size)
{ return 1; }
 
#endif


/**
 * ospf_rx_hook
 * @sk: socket we received the packet.
 * @size: size of the packet
 *
 * This is the entry point for messages from neighbors. Many checks (like
 * authentication, checksums, size) are done before the packet is passed to
 * non generic functions.
 */
int
ospf_rx_hook(sock *sk, int size)
{
  char *mesg = "OSPF: Bad packet from ";

  /* We want just packets from sk->iface. Unfortunately, on BSD we
     cannot filter out other packets at kernel level and we receive
     all packets on all sockets */
  if (sk->lifindex != sk->iface->index)
    return 1;

  DBG("OSPF: RX hook called (iface %s, src %I, dst %I)\n",
      sk->iface->name, sk->faddr, sk->laddr);

  /* Initially, the packet is associated with the 'master' iface */
  struct ospf_iface *ifa = sk->data;
  struct proto_ospf *po = ifa->oa->po;
  // struct proto *p = &po->proto;

  int src_local, dst_local UNUSED, dst_mcast; 
  src_local = ipa_in_net(sk->faddr, ifa->addr->prefix, ifa->addr->pxlen);
  dst_local = ipa_equal(sk->laddr, ifa->addr->ip);
  dst_mcast = ipa_equal(sk->laddr, ifa->all_routers) || ipa_equal(sk->laddr, AllDRouters);

#ifdef OSPFv2
  /* First, we eliminate packets with strange address combinations.
   * In OSPFv2, they might be for other ospf_ifaces (with different IP
   * prefix) on the same real iface, so we don't log it. We enforce
   * that (src_local || dst_local), therefore we are eliminating all
   * such cases. 
   */
  if (dst_mcast && !src_local)
    return 1;
  if (!dst_mcast && !dst_local)
    return 1;

  /* Ignore my own broadcast packets */
  if (ifa->cf->real_bcast && ipa_equal(sk->faddr, ifa->addr->ip))
    return 1;
#else /* OSPFv3 */

  /* In OSPFv3, src_local and dst_local mean link-local. 
   * RFC 5340 says that local (non-vlink) packets use
   * link-local src address, but does not enforce it. Strange.
   */
  if (dst_mcast && !src_local)
    log(L_WARN "OSPF: Received multicast packet from %I (not link-local)", sk->faddr);
#endif

  /* Second, we check packet size, checksum, and the protocol version */
  struct ospf_packet *ps = (struct ospf_packet *) ip_skip_header(sk->rbuf, &size);

  if (ps == NULL)
  {
    log(L_ERR "%s%I - bad IP header", mesg, sk->faddr);
    return 1;
  }

  if (ifa->check_ttl && (sk->ttl < 255))
  {
    log(L_ERR "%s%I - TTL %d (< 255)", mesg, sk->faddr, sk->ttl);
    return 1;
  }

  if ((unsigned) size < sizeof(struct ospf_packet))
  {
    log(L_ERR "%s%I - too short (%u bytes)", mesg, sk->faddr, size);
    return 1;
  }

  int osize = ntohs(ps->length);
  if ((unsigned) osize < sizeof(struct ospf_packet))
  {
    log(L_ERR "%s%I - too low value in size field (%u bytes)", mesg, sk->faddr, osize);
    return 1;
  }

  if ((osize > size) || ((osize % 4) != 0))
  {
    log(L_ERR "%s%I - size field does not match (%d/%d)", mesg, sk->faddr, osize, size);
    return 1;
  }

  if ((unsigned) size > sk->rbsize)
  {
    log(L_ERR "%s%I - too large (%d vs %d)", mesg, sk->faddr, size, sk->rbsize);
    return 1;
  }

  if (ps->version != OSPF_VERSION)
  {
    log(L_ERR "%s%I - version %u", mesg, sk->faddr, ps->version);
    return 1;
  }

#ifdef OSPFv2
  if ((ps->autype != htons(OSPF_AUTH_CRYPT)) &&
      (!ipsum_verify(ps, 16, (void *) ps + sizeof(struct ospf_packet),
		     osize - sizeof(struct ospf_packet), NULL)))
  {
    log(L_ERR "%s%I - bad checksum", mesg, sk->faddr);
    return 1;
  }
#endif


  /* Third, we resolve associated iface and handle vlinks. */

  u32 areaid = ntohl(ps->areaid);
  u32 rid = ntohl(ps->routerid);

  if ((areaid == ifa->oa->areaid)
#ifdef OSPFv3
      && (ps->instance_id == ifa->instance_id)
#endif
      )
  {
    /* It is real iface, source should be local (in OSPFv2) */
#ifdef OSPFv2
    if (!src_local)
      return 1;
#endif
  }
  else if (dst_mcast || (areaid != 0))
  {
    /* Obvious mismatch */

#ifdef OSPFv2
    /* We ignore mismatch in OSPFv3, because there might be
       other instance with different instance ID */
    log(L_ERR "%s%I - area does not match (%R vs %R)",
	mesg, sk->faddr, areaid, ifa->oa->areaid);
#endif
    return 1;
  }
  else
  {
    /* Some vlink? */
    struct ospf_iface *iff = NULL;

    WALK_LIST(iff, po->iface_list)
    {
      if ((iff->type == OSPF_IT_VLINK) && 
	  (iff->voa == ifa->oa) &&
#ifdef OSPFv3
	  (iff->instance_id == ps->instance_id) &&
#endif
	  (iff->vid == rid))
	{
	  /* Vlink should be UP */
	  if (iff->state != OSPF_IS_PTP)
	    return 1;
	  
	  ifa = iff;
	  goto found;
	}
    }

#ifdef OSPFv2
    log(L_WARN "OSPF: Received packet for unknown vlink (ID %R, IP %I)", rid, sk->faddr);
#endif
    return 1;
  }

 found:
  if (ifa->stub)	    /* This shouldn't happen */
    return 1;

  if (ipa_equal(sk->laddr, AllDRouters) && (ifa->sk_dr == 0))
    return 1;

  if (rid == po->router_id)
  {
    log(L_ERR "%s%I - received my own router ID!", mesg, sk->faddr);
    return 1;
  }

  if (rid == 0)
  {
    log(L_ERR "%s%I - router id = 0.0.0.0", mesg, sk->faddr);
    return 1;
  }

#ifdef OSPFv2
  /* In OSPFv2, neighbors are identified by either IP or Router ID, base on network type */
  struct ospf_neighbor *n;
  if ((ifa->type == OSPF_IT_BCAST) || (ifa->type == OSPF_IT_NBMA) || (ifa->type == OSPF_IT_PTMP))
    n = find_neigh_by_ip(ifa, sk->faddr);
  else
    n = find_neigh(ifa, rid);
#else
  struct ospf_neighbor *n = find_neigh(ifa, rid);
#endif

  if(!n && (ps->type != HELLO_P))
  {
    log(L_WARN "OSPF: Received non-hello packet from unknown neighbor (src %I, iface %s)",
	sk->faddr, ifa->iface->name);
    return 1;
  }

  if (!ospf_pkt_checkauth(n, ifa, ps, size))
  {
    log(L_ERR "%s%I - authentication failed", mesg, sk->faddr);
    return 1;
  }

  /* Dump packet 
     pu8=(u8 *)(sk->rbuf+5*4);
     for(i=0;i<ntohs(ps->length);i+=4)
     DBG("%s: received %u,%u,%u,%u\n",p->name, pu8[i+0], pu8[i+1], pu8[i+2],
     pu8[i+3]);
     DBG("%s: received size: %u\n",p->name,size);
   */

  switch (ps->type)
  {
  case HELLO_P:
    DBG("%s: Hello received.\n", p->name);
    ospf_hello_receive(ps, ifa, n, sk->faddr);
    break;
  case DBDES_P:
    DBG("%s: Database description received.\n", p->name);
    ospf_dbdes_receive(ps, ifa, n);
    break;
  case LSREQ_P:
    DBG("%s: Link state request received.\n", p->name);
    ospf_lsreq_receive(ps, ifa, n);
    break;
  case LSUPD_P:
    DBG("%s: Link state update received.\n", p->name);
    ospf_lsupd_receive(ps, ifa, n);
    break;
  case LSACK_P:
    DBG("%s: Link state ack received.\n", p->name);
    ospf_lsack_receive(ps, ifa, n);
    break;
  default:
    log(L_ERR "%s%I - wrong type %u", mesg, sk->faddr, ps->type);
    return 1;
  };
  return 1;
}

void
ospf_tx_hook(sock * sk)
{
  struct ospf_iface *ifa= (struct ospf_iface *) (sk->data);
//  struct proto *p = (struct proto *) (ifa->oa->po);
  log(L_ERR "OSPF: TX hook called on %s", ifa->iface->name);
}

void
ospf_err_hook(sock * sk, int err)
{
  struct ospf_iface *ifa= (struct ospf_iface *) (sk->data);
//  struct proto *p = (struct proto *) (ifa->oa->po);
  log(L_ERR "OSPF: Socket error on %s: %M", ifa->iface->name, err);
}

void
ospf_send_to_agt(struct ospf_iface *ifa, u8 state)
{
  struct ospf_neighbor *n;

  WALK_LIST(n, ifa->neigh_list)
    if (n->state >= state)
      ospf_send_to(ifa, n->ip);
}

void
ospf_send_to_bdr(struct ospf_iface *ifa)
{
  if (!ipa_equal(ifa->drip, IPA_NONE))
    ospf_send_to(ifa, ifa->drip);
  if (!ipa_equal(ifa->bdrip, IPA_NONE))
    ospf_send_to(ifa, ifa->bdrip);
}

void
ospf_send_to(struct ospf_iface *ifa, ip_addr dst)
{
  sock *sk = ifa->sk;
  struct ospf_packet *pkt = (struct ospf_packet *) sk->tbuf;
  int len = ntohs(pkt->length);

#ifdef OSPFv2
  if (ifa->autype == OSPF_AUTH_CRYPT)
    len += OSPF_AUTH_CRYPT_SIZE;
#endif

  ospf_pkt_finalize(ifa, pkt);
  if (sk->tbuf != sk->tpos)
    log(L_ERR "Aiee, old packet was overwritten in TX buffer");

  sk_send_to(sk, len, dst, 0);
}

