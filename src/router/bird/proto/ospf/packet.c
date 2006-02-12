/*
 *	BIRD -- OSPF
 *
 *	(c) 1999--2004 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "ospf.h"
#include "nest/password.h"
#include "lib/md5.h"

void
ospf_pkt_fill_hdr(struct ospf_iface *ifa, void *buf, u8 h_type)
{
  struct ospf_packet *pkt;
  struct proto *p = (struct proto *) (ifa->oa->po);

  pkt = (struct ospf_packet *) buf;

  pkt->version = OSPF_VERSION;

  pkt->type = h_type;

  pkt->routerid = htonl(p->cf->global->router_id);
  pkt->areaid = htonl(ifa->oa->areaid);
  pkt->autype = htons(ifa->autype);
  pkt->checksum = 0;
}

unsigned
ospf_pkt_maxsize(struct ospf_iface *ifa)
{
  return ifa->iface->mtu - SIZE_OF_IP_HEADER -
    ((ifa->autype == OSPF_AUTH_CRYPT) ? OSPF_AUTH_CRYPT_SIZE : 0);
}

void
ospf_pkt_finalize(struct ospf_iface *ifa, struct ospf_packet *pkt)
{
  struct password_item *passwd = password_find (ifa->passwords);
  void *tail;
  struct MD5Context ctxt;
  char password[OSPF_AUTH_CRYPT_SIZE];

  pkt->autype = htons(ifa->autype);

  switch(ifa->autype)
  {
    case OSPF_AUTH_SIMPLE:
      bzero(&pkt->u, sizeof(union ospf_auth));
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
      if (!passwd)
      {
        log( L_ERR "No suitable password found for authentication" );
        return;
      }

      pkt->checksum = 0;

      if (!ifa->csn)
        ifa->csn = (u32) time(NULL);

      pkt->u.md5.keyid = passwd->id;
      pkt->u.md5.len = OSPF_AUTH_CRYPT_SIZE;
      pkt->u.md5.zero = 0;
      pkt->u.md5.csn = htonl(ifa->csn++);
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
      pass = password_find (ifa->passwords);
      if(!pass)
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
        OSPF_TRACE(D_PACKETS, "OSPF_auth: size mismatch (%d vs %s)",
	  ntohs(pkt->length) + OSPF_AUTH_CRYPT_SIZE, size);
        return 0;
      }

      if (pkt->u.md5.zero)
      {
        OSPF_TRACE(D_PACKETS, "OSPF_auth: \"zero\" area is non-zero");
        return 0;
      }

      tail = ((void *)pkt) + ntohs(pkt->length);

      WALK_LIST(ptmp, *(ifa->passwords))
      {
        if (pkt->u.md5.keyid != ptmp->id) continue;
        if ((ptmp->accfrom > now) || (ptmp->accto < now)) continue;
        pass = ptmp;
        break;
      }

      if(!pass)
      {
        OSPF_TRACE(D_PACKETS, "OSPF_auth: no suitable md5 password found");
        return 0;
      }

      if(n)
      {
        if(ntohs(pkt->u.md5.csn) < n->csn)
        {
          OSPF_TRACE(D_PACKETS, "OSPF_auth: lower sequence number");
          return 0;
        }
        n->csn = ntohs(pkt->u.md5.csn);
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

  ps = (struct ospf_packet *) ipv4_skip_header(sk->rbuf, &size);

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

  osize = ntohs(ps->length);
  if (ps == NULL)
  {
    log(L_ERR "%s%I - bad IP header", mesg, sk->faddr);
    return 1;
  }

  if ((unsigned) size < sizeof(struct ospf_packet))
  {
    log(L_ERR "%s%I - too short (%u bytes)", mesg, sk->faddr, size);
    return 1;
  }

  if ((osize > size) || (osize != (4 * (osize / 4))))
  {
    log(L_ERR "%s%I - size field does not match (%d/%d)", mesg, sk->faddr, ntohs(ps->length), size );
    return 1;
  }

  if (ps->version != OSPF_VERSION)
  {
    log(L_ERR "%s%I - version %u", mesg, sk->faddr, ps->version);
    return 1;
  }

  if ((ps->autype != htons(OSPF_AUTH_CRYPT)) &&
      (!ipsum_verify(ps, 16, (void *) ps + sizeof(struct ospf_packet),
		    ntohs(ps->length) - sizeof(struct ospf_packet), NULL)))
  {
    log(L_ERR "%s%I - bad checksum", mesg, sk->faddr);
    return 1;
  }

  if (ntohl(ps->areaid) != ifa->oa->areaid)
  {
    log(L_ERR "%s%I - different area %ld", mesg, sk->faddr, ntohl(ps->areaid));
    return 1;
  }

  if (ntohl(ps->routerid) == p->cf->global->router_id)
  {
    log(L_ERR "%s%I - received my own router ID!", mesg, sk->faddr);
    return 1;
  }

  if (ntohl(ps->routerid) == 0)
  {
    log(L_ERR "%s%I - router id = 0.0.0.0", mesg, sk->faddr);
    return 1;
  }

  if ((unsigned) size > ifa->iface->mtu)
  {
    log(L_ERR "%s%I - received larger packet than MTU", mesg, sk->faddr);
    return 1;
  }

  n = find_neigh(ifa, ntohl(((struct ospf_packet *) ps)->routerid));

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
    ospf_hello_receive((struct ospf_hello_packet *) ps, ifa, n, sk->faddr);
    break;
  case DBDES_P:
    DBG("%s: Database description received.\n", p->name);
    ospf_dbdes_receive((struct ospf_dbdes_packet *) ps, ifa, n);
    break;
  case LSREQ_P:
    DBG("%s: Link state request received.\n", p->name);
    ospf_lsreq_receive((struct ospf_lsreq_packet *) ps, ifa, n);
    break;
  case LSUPD_P:
    DBG("%s: Link state update received.\n", p->name);
    ospf_lsupd_receive((struct ospf_lsupd_packet *) ps, ifa, n);
    break;
  case LSACK_P:
    DBG("%s: Link state ack received.\n", p->name);
    ospf_lsack_receive((struct ospf_lsack_packet *) ps, ifa, n);
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
ospf_send_to_agt(sock * sk, struct ospf_iface *ifa, u8 state)
{
  struct ospf_neighbor *n;

  WALK_LIST(n, ifa->neigh_list) if (n->state >= state)
    ospf_send_to(sk, n->ip, ifa);
}

void
ospf_send_to_bdr(sock * sk, struct ospf_iface *ifa)
{
  if (!ipa_equal(ifa->drip, IPA_NONE))
    ospf_send_to(sk, ifa->drip, ifa);
  if (!ipa_equal(ifa->bdrip, IPA_NONE))
    ospf_send_to(sk, ifa->bdrip, ifa);
}

void
ospf_send_to(sock *sk, ip_addr ip, struct ospf_iface *ifa)
{
  struct ospf_packet *pkt = (struct ospf_packet *) sk->tbuf;
  int len = ntohs(pkt->length) + ((ifa->autype == OSPF_AUTH_CRYPT) ? OSPF_AUTH_CRYPT_SIZE : 0);
  ospf_pkt_finalize(ifa, pkt);

  if (ipa_equal(ip, IPA_NONE))
    sk_send(sk, len);
  else
    sk_send_to(sk, len, ip, OSPF_PROTO);
}

