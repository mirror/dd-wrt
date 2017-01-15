/*
 *	BIRD -- OSPF
 *
 *	(c) 1999--2005 Ondrej Filip <feela@network.cz>
 *	(c) 2009--2014 Ondrej Zajicek <santiago@crfreenet.org>
 *	(c) 2009--2014 CZ.NIC z.s.p.o.
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "ospf.h"
#include "nest/password.h"
#include "lib/md5.h"
#include "lib/mac.h"
#include "lib/socket.h"

void
ospf_pkt_fill_hdr(struct ospf_iface *ifa, void *buf, u8 h_type)
{
  struct ospf_proto *p = ifa->oa->po;
  struct ospf_packet *pkt;

  pkt = (struct ospf_packet *) buf;

  pkt->version = ospf_get_version(p);
  pkt->type = h_type;
  pkt->length = htons(ospf_pkt_maxsize(ifa));
  pkt->routerid = htonl(p->router_id);
  pkt->areaid = htonl(ifa->oa->areaid);
  pkt->checksum = 0;
  pkt->instance_id = ifa->instance_id;
  pkt->autype = ifa->autype;
}

/* We assume OSPFv2 in ospf_pkt_finalize() */
static void
ospf_pkt_finalize(struct ospf_iface *ifa, struct ospf_packet *pkt, uint *plen)
{
  struct password_item *pass = NULL;
  union ospf_auth *auth = (void *) (pkt + 1);

  pkt->checksum = 0;
  pkt->autype = ifa->autype;
  bzero(auth, sizeof(union ospf_auth));

  /* Compatibility note: auth may contain anything if autype is
     none, but nonzero values do not work with Mikrotik OSPF */

  switch (ifa->autype)
  {
  case OSPF_AUTH_SIMPLE:
    pass = password_find(ifa->passwords, 1);
    if (!pass)
    {
      log(L_ERR "No suitable password found for authentication");
      return;
    }
    strncpy(auth->password, pass->password, sizeof(auth->password));

  case OSPF_AUTH_NONE:
    {
      void *body = (void *) (auth + 1);
      uint blen = *plen - sizeof(struct ospf_packet) - sizeof(union ospf_auth);
      pkt->checksum = ipsum_calculate(pkt, sizeof(struct ospf_packet), body, blen, NULL);
    }
    break;

  case OSPF_AUTH_CRYPT:
    pass = password_find(ifa->passwords, 0);
    if (!pass)
    {
      log(L_ERR "No suitable password found for authentication");
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

    uint auth_len = mac_type_length(pass->alg);
    byte *auth_tail = ((byte *) pkt + *plen);
    *plen += auth_len;

    ASSERT(*plen < ifa->sk->tbsize);

    auth->c32.zero = 0;
    auth->c32.keyid = pass->id;
    auth->c32.len = auth_len;
    auth->c32.csn = htonl(ifa->csn);

    /* Append key for keyed hash, append padding for HMAC (RFC 5709 3.3) */
    if (pass->alg < ALG_HMAC)
      strncpy(auth_tail, pass->password, auth_len);
    else
      memset32(auth_tail, HMAC_MAGIC, auth_len / 4);

    mac_fill(pass->alg, pass->password, pass->length,
	     (byte *) pkt, *plen, auth_tail);
    break;

  default:
    bug("Unknown authentication type");
  }
}


/* We assume OSPFv2 in ospf_pkt_checkauth() */
static int
ospf_pkt_checkauth(struct ospf_neighbor *n, struct ospf_iface *ifa, struct ospf_packet *pkt, uint len)
{
  struct ospf_proto *p = ifa->oa->po;
  union ospf_auth *auth = (void *) (pkt + 1);
  struct password_item *pass = NULL;
  const char *err_dsc = NULL;
  uint err_val = 0;

  uint plen = ntohs(pkt->length);
  u8 autype = pkt->autype;

  if (autype != ifa->autype)
    DROP("authentication method mismatch", autype);

  switch (autype)
  {
  case OSPF_AUTH_NONE:
    return 1;

  case OSPF_AUTH_SIMPLE:
    pass = password_find(ifa->passwords, 1);
    if (!pass)
      DROP1("no password found");

    if (!password_verify(pass, auth->password, sizeof(auth->password)))
      DROP("wrong password", pass->id);

    return 1;

  case OSPF_AUTH_CRYPT:
    pass = password_find_by_id(ifa->passwords, auth->c32.keyid);
    if (!pass)
      DROP("no suitable password found", auth->c32.keyid);

    uint auth_len = mac_type_length(pass->alg);

    if (plen + auth->c32.len > len)
      DROP("packet length mismatch", len);

    if (auth->c32.len != auth_len)
      DROP("wrong authentication length", auth->c32.len);

    u32 rcv_csn = ntohl(auth->c32.csn);
    if (n && (rcv_csn < n->csn))
      // DROP("lower sequence number", rcv_csn);
    {
      /* We want to report both new and old CSN */
      LOG_PKT_AUTH("Authentication failed for nbr %R on %s - "
		   "lower sequence number (rcv %u, old %u)",
		   n->rid, ifa->ifname, rcv_csn, n->csn);
      return 0;
    }

    byte *auth_tail = ((byte *) pkt) + plen;
    byte *auth_data = alloca(auth_len);
    memcpy(auth_data, auth_tail, auth_len);

    /* Append key for keyed hash, append padding for HMAC (RFC 5709 3.3) */
    if (pass->alg < ALG_HMAC)
      strncpy(auth_tail, pass->password, auth_len);
    else
      memset32(auth_tail, HMAC_MAGIC, auth_len / 4);

    if (!mac_verify(pass->alg, pass->password, pass->length,
		    (byte *) pkt, plen + auth_len, auth_data))
      DROP("wrong authentication code", pass->id);

    if (n)
      n->csn = rcv_csn;

    return 1;

  default:
    bug("Unknown authentication type");
  }

drop:
  LOG_PKT_AUTH("Authentication failed for nbr %R on %s - %s (%u)",
	       (n ? n->rid : ntohl(pkt->routerid)), ifa->ifname, err_dsc, err_val);

  return 0;
}

/**
 * ospf_rx_hook
 * @sk: socket we received the packet.
 * @len: size of the packet
 *
 * This is the entry point for messages from neighbors. Many checks (like
 * authentication, checksums, size) are done before the packet is passed to
 * non generic functions.
 */
int
ospf_rx_hook(sock *sk, uint len)
{
  /* We want just packets from sk->iface. Unfortunately, on BSD we cannot filter
     out other packets at kernel level and we receive all packets on all sockets */
  if (sk->lifindex != sk->iface->index)
    return 1;

  DBG("OSPF: RX hook called (iface %s, src %I, dst %I)\n",
      sk->iface->name, sk->faddr, sk->laddr);

  /* Initially, the packet is associated with the 'master' iface */
  struct ospf_iface *ifa = sk->data;
  struct ospf_proto *p = ifa->oa->po;
  const char *err_dsc = NULL;
  uint err_val = 0;

  /* Should not happen */
  if (ifa->state <= OSPF_IS_LOOP)
    return 1;

  int src_local, dst_local, dst_mcast;
  src_local = ipa_in_net(sk->faddr, ifa->addr->prefix, ifa->addr->pxlen);
  dst_local = ipa_equal(sk->laddr, ifa->addr->ip);
  dst_mcast = ipa_equal(sk->laddr, ifa->all_routers) || ipa_equal(sk->laddr, ifa->des_routers);

  if (ospf_is_v2(p))
  {
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
  }
  else
  {
    /* In OSPFv3, src_local and dst_local mean link-local.
     * RFC 5340 says that local (non-vlink) packets use
     * link-local src address, but does not enforce it. Strange.
     */
    if (dst_mcast && !src_local)
      LOG_PKT_WARN("Multicast packet received from non-link-local %I via %s",
		   sk->faddr, ifa->ifname);
  }

  /* Second, we check packet length, checksum, and the protocol version */
  struct ospf_packet *pkt = (void *) sk_rx_buffer(sk, &len);


  if (pkt == NULL)
    DROP("bad IP header", len);

  if (ifa->check_ttl && (sk->rcv_ttl < 255))
    DROP("wrong TTL", sk->rcv_ttl);

  if (len < sizeof(struct ospf_packet))
    DROP("too short", len);

  if (pkt->version != ospf_get_version(p))
    DROP("version mismatch", pkt->version);

  uint plen = ntohs(pkt->length);
  if ((plen < sizeof(struct ospf_packet)) || ((plen % 4) != 0))
    DROP("invalid length", plen);

  if (sk->flags & SKF_TRUNCATED)
  {
    /* If we have dynamic buffers and received truncated message, we expand RX buffer */

    uint bs = plen + 256;
    bs = BIRD_ALIGN(bs, 1024);

    if (!ifa->cf->rx_buffer && (bs > sk->rbsize))
      sk_set_rbsize(sk, bs);

    DROP("truncated", plen);
  }

  if (plen > len)
    DROP("length mismatch", plen);

  if (ospf_is_v2(p) && (pkt->autype != OSPF_AUTH_CRYPT))
  {
    uint hlen = sizeof(struct ospf_packet) + sizeof(union ospf_auth);
    uint blen = plen - hlen;
    void *body = ((void *) pkt) + hlen;

    if (!ipsum_verify(pkt, sizeof(struct ospf_packet), body, blen, NULL))
      DROP1("invalid checksum");
  }

  /* Third, we resolve associated iface and handle vlinks. */

  u32 areaid = ntohl(pkt->areaid);
  u32 rid = ntohl(pkt->routerid);
  u8 instance_id = pkt->instance_id;

  if (areaid == ifa->oa->areaid)
  {
    /* Matching area ID */

    if (instance_id != ifa->instance_id)
      return 1;

    /* It is real iface, source should be local (in OSPFv2) */
    if (ospf_is_v2(p) && !src_local)
      DROP1("strange source address");

    goto found;
  }
  else if ((areaid == 0) && !dst_mcast)
  {
    /* Backbone area ID and possible vlink packet */

    if ((p->areano == 1) || !oa_is_ext(ifa->oa))
      return 1;

    struct ospf_iface *iff = NULL;
    WALK_LIST(iff, p->iface_list)
    {
      if ((iff->type == OSPF_IT_VLINK) &&
	  (iff->voa == ifa->oa) &&
	  (iff->instance_id == instance_id) &&
	  (iff->vid == rid))
      {
	/* Vlink should be UP */
	if (iff->state != OSPF_IS_PTP)
	  return 1;

	ifa = iff;
	goto found;
      }
    }

    /*
     * Cannot find matching vlink. It is either misconfigured vlink; NBMA or
     * PtMP with misconfigured area ID, or packet for some other instance (that
     * is possible even if instance_id == ifa->instance_id, because it may be
     * also vlink packet in the other instance, which is different namespace).
     */

    return 1;
  }
  else
  {
    /* Non-matching area ID but cannot be vlink packet */

    if (instance_id != ifa->instance_id)
      return 1;

    DROP("area mismatch", areaid);
  }


found:
  if (ifa->stub)	    /* This shouldn't happen */
    return 1;

  if (ipa_equal(sk->laddr, ifa->des_routers) && (ifa->sk_dr == 0))
    return 1;

  if (rid == p->router_id)
    DROP1("my own router ID");

  if (rid == 0)
    DROP1("zero router ID");

  /* In OSPFv2, neighbors are identified by either IP or Router ID, based on network type */
  uint t = ifa->type;
  struct ospf_neighbor *n;
  if (ospf_is_v2(p) && ((t == OSPF_IT_BCAST) || (t == OSPF_IT_NBMA) || (t == OSPF_IT_PTMP)))
    n = find_neigh_by_ip(ifa, sk->faddr);
  else
    n = find_neigh(ifa, rid);

  if (!n && (pkt->type != HELLO_P))
  {
    OSPF_TRACE(D_PACKETS, "Non-HELLO packet received from unknown nbr %R on %s, src %I",
	       rid, ifa->ifname, sk->faddr);
    return 1;
  }

  /* ospf_pkt_checkauth() has its own error logging */
  if (ospf_is_v2(p) && !ospf_pkt_checkauth(n, ifa, pkt, len))
    return 1;

  switch (pkt->type)
  {
  case HELLO_P:
    ospf_receive_hello(pkt, ifa, n, sk->faddr);
    break;

  case DBDES_P:
    ospf_receive_dbdes(pkt, ifa, n);
    break;

  case LSREQ_P:
    ospf_receive_lsreq(pkt, ifa, n);
    break;

  case LSUPD_P:
    ospf_receive_lsupd(pkt, ifa, n);
    break;

  case LSACK_P:
    ospf_receive_lsack(pkt, ifa, n);
    break;

  default:
    DROP("invalid packet type", pkt->type);
  };
  return 1;

drop:
  LOG_PKT("Bad packet from %I via %s - %s (%u)",
	  sk->faddr, ifa->ifname, err_dsc, err_val);

  return 1;
}

/*
void
ospf_tx_hook(sock * sk)
{
  struct ospf_iface *ifa= (struct ospf_iface *) (sk->data);
//  struct proto *p = (struct proto *) (ifa->oa->p);
  log(L_ERR "OSPF: TX hook called on %s", ifa->ifname);
}
*/

void
ospf_err_hook(sock * sk, int err)
{
  struct ospf_iface *ifa= (struct ospf_iface *) (sk->data);
  struct ospf_proto *p = ifa->oa->po;
  log(L_ERR "%s: Socket error on %s: %M", p->p.name, ifa->ifname, err);
}

void
ospf_verr_hook(sock *sk, int err)
{
  struct ospf_proto *p = (struct ospf_proto *) (sk->data);
  log(L_ERR "%s: Vlink socket error: %M", p->p.name, err);
}

void
ospf_send_to(struct ospf_iface *ifa, ip_addr dst)
{
  sock *sk = ifa->sk;
  struct ospf_packet *pkt = (struct ospf_packet *) sk->tbuf;
  uint plen = ntohs(pkt->length);

  if (ospf_is_v2(ifa->oa->po))
    ospf_pkt_finalize(ifa, pkt, &plen);

  int done = sk_send_to(sk, plen, dst, 0);
  if (!done)
    log(L_WARN "OSPF: TX queue full on %s", ifa->ifname);
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
  if (ipa_nonzero(ifa->drip))
    ospf_send_to(ifa, ifa->drip);
  if (ipa_nonzero(ifa->bdrip))
    ospf_send_to(ifa, ifa->bdrip);
}
