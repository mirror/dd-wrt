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
  struct proto *p = &po->proto;
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
  /* For virtual links use mtu=576, can be mtu < 576? */
  unsigned mtu = (ifa->type == OSPF_IT_VLINK) ? OSPF_VLINK_MTU : ifa->iface->mtu;
  unsigned add = 0;

#ifdef OSPFv2
  add = ((ifa->autype == OSPF_AUTH_CRYPT) ? OSPF_AUTH_CRYPT_SIZE : 0);
#endif

  return ((mtu <=  ifa->iface->mtu) ? mtu : ifa->iface->mtu) -
  SIZE_OF_IP_HEADER - add;
}


#ifdef OSPFv2

static void
ospf_pkt_finalize(struct ospf_iface *ifa, struct ospf_packet *pkt)
{
  struct password_item *passwd = NULL;
  void *tail;
  struct MD5Context ctxt;
  char password[OSPF_AUTH_CRYPT_SIZE];

  pkt->autype = htons(ifa->autype);

  switch(ifa->autype)
  {
    case OSPF_AUTH_SIMPLE:
      bzero(&pkt->u, sizeof(union ospf_auth));
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

      pkt->checksum = 0;

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

  if (n && (ifa != n->ifa))
  {
    OSPF_TRACE(D_PACKETS, "OSPF_auth: received packet from strange interface (%s/%s)",
      ifa->iface->name, n->ifa->iface->name);
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

      if (ntohs(pkt->length) + OSPF_AUTH_CRYPT_SIZE != size)
      {
        OSPF_TRACE(D_PACKETS, "OSPF_auth: size mismatch (%d vs %d)",
	  ntohs(pkt->length) + OSPF_AUTH_CRYPT_SIZE, size);
        return 0;
      }

      if (pkt->u.md5.zero)
      {
        OSPF_TRACE(D_PACKETS, "OSPF_auth: \"zero\" area is non-zero");
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
 * @sk: socket we received the packet. Its ignored.
 * @size: size of the packet
 *
 * This is the entry point for messages from neighbors. Many checks (like
 * authentication, checksums, size) are done before the packet is passed to
 * non generic functions.
 */
int
ospf_rx_hook(sock * sk, int size)
{
  struct ospf_packet *ps;
  struct ospf_iface *ifa = (struct ospf_iface *) (sk->data);
  struct proto_ospf *po = ifa->oa->po;
  struct proto *p = &po->proto;
  struct ospf_neighbor *n;
  int osize;
  char *mesg = "Bad OSPF packet from ";
  struct ospf_iface *iff;

  if (ifa->stub)
    return (1);

  ps = (struct ospf_packet *) ip_skip_header(sk->rbuf, &size);

  if (ps == NULL)
  {
    log(L_ERR "%s%I - bad IP header", mesg, sk->faddr);
    return 1;
  }

  /* We receive packets related to vlinks even on non-vlink sockets */
  if ((ifa->oa->areaid != 0) && (ntohl(ps->areaid) == 0))
  {
    WALK_LIST(iff, po->iface_list)
    {
      if ((iff->type == OSPF_IT_VLINK) && (iff->iface == ifa->iface) &&
          (iff->voa = ifa->oa) && ipa_equal(sk->faddr, iff->vip))
      {
        return 1;       /* Packet is for VLINK */
      }
    }
  }

  DBG("%s: RX_Hook called on interface %s.\n", p->name, sk->iface->name);

  if ((unsigned) size < sizeof(struct ospf_packet))
  {
    log(L_ERR "%s%I - too short (%u bytes)", mesg, sk->faddr, size);
    return 1;
  }

  osize = ntohs(ps->length);
  if ((osize > size) || ((osize % 4) != 0))
  {
    log(L_ERR "%s%I - size field does not match (%d/%d)", mesg, sk->faddr, osize, size);
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

  if (ntohl(ps->areaid) != ifa->oa->areaid)
  {
    log(L_ERR "%s%I - different area (%u)", mesg, sk->faddr, ntohl(ps->areaid));
    return 1;
  }

  /* FIXME - handling of instance id should be better */
#ifdef OSPFv3
  if (ps->instance_id != ifa->instance_id)
  {
    log(L_ERR "%s%I - different instance (%u)", mesg, sk->faddr, ps->instance_id);
    return 1;
  }
#endif

  if (ntohl(ps->routerid) == po->router_id)
  {
    log(L_ERR "%s%I - received my own router ID!", mesg, sk->faddr);
    return 1;
  }

  if (ntohl(ps->routerid) == 0)
  {
    log(L_ERR "%s%I - router id = 0.0.0.0", mesg, sk->faddr);
    return 1;
  }

  if ((unsigned) size > sk->rbsize)
  {
    log(L_ERR "%s%I - packet is too large (%d vs %d)",
	mesg, sk->faddr, size, sk->rbsize);
    return 1;
  }

  /* This is deviation from RFC 2328 - neighbours should be identified by
   * IP address on broadcast and NBMA networks.
   */
  n = find_neigh(ifa, ntohl(ps->routerid));

  if(!n && (ps->type != HELLO_P))
  {
    OSPF_TRACE(D_PACKETS, "Received non-hello packet from uknown neighbor (%I)", sk->faddr);
    return 1;
  }

  if (!ospf_pkt_checkauth(n, ifa, ps, size))
  {
    log(L_ERR "%s%I - authentification failed", mesg, sk->faddr);
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
  struct proto *p = (struct proto *) (ifa->oa->po);
  log(L_ERR "%s: TX_Hook called on interface %s\n", p->name, sk->iface->name);
}

void
ospf_err_hook(sock * sk, int err)
{
  struct ospf_iface *ifa= (struct ospf_iface *) (sk->data);
  struct proto *p = (struct proto *) (ifa->oa->po);
  log(L_ERR "%s: Err_Hook called on interface %s with err=%d\n",
    p->name, sk->iface->name, err);
}

void
ospf_send_to_agt(struct ospf_iface *ifa, u8 state)
{
  struct ospf_neighbor *n;

  WALK_LIST(n, ifa->neigh_list) if (n->state >= state)
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
ospf_send_to(struct ospf_iface *ifa, ip_addr ip)
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
    log(L_ERR "Aiee, old packet was overwritted in TX buffer");

  if (ipa_equal(ip, IPA_NONE))
    sk_send(sk, len);
  else
    sk_send_to(sk, len, ip, OSPF_PROTO);
}

