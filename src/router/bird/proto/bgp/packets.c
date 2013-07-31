/*
 *	BIRD -- BGP Packet Processing
 *
 *	(c) 2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#undef LOCAL_DEBUG

#include "nest/bird.h"
#include "nest/iface.h"
#include "nest/protocol.h"
#include "nest/route.h"
#include "nest/attrs.h"
#include "nest/mrtdump.h"
#include "conf/conf.h"
#include "lib/unaligned.h"
#include "lib/socket.h"

#include "nest/cli.h"

#include "bgp.h"

static struct rate_limit rl_rcv_update,  rl_snd_update;

/* Table for state -> RFC 6608 FSM error subcodes */
static byte fsm_err_subcode[BS_MAX] = {
  [BS_OPENSENT] = 1,
  [BS_OPENCONFIRM] = 2,
  [BS_ESTABLISHED] = 3
};

/*
 * MRT Dump format is not semantically specified.
 * We will use these values in appropriate fields:
 *
 * Local AS, Remote AS - configured AS numbers for given BGP instance.
 * Local IP, Remote IP - IP addresses of the TCP connection (0 if no connection)
 *
 * We dump two kinds of MRT messages: STATE_CHANGE (for BGP state
 * changes) and MESSAGE (for received BGP messages).
 *
 * STATE_CHANGE uses always AS4 variant, but MESSAGE uses AS4 variant
 * only when AS4 session is established and even in that case MESSAGE
 * does not use AS4 variant for initial OPEN message. This strange
 * behavior is here for compatibility with Quagga and Bgpdump,
 */

static byte *
mrt_put_bgp4_hdr(byte *buf, struct bgp_conn *conn, int as4)
{
  struct bgp_proto *p = conn->bgp;

  if (as4)
    {
      put_u32(buf+0, p->remote_as);
      put_u32(buf+4, p->local_as);
      buf+=8;
    }
  else
    {
      put_u16(buf+0, (p->remote_as <= 0xFFFF) ? p->remote_as : AS_TRANS);
      put_u16(buf+2, (p->local_as <= 0xFFFF)  ? p->local_as  : AS_TRANS);
      buf+=4;
    }

  put_u16(buf+0, (p->neigh && p->neigh->iface) ? p->neigh->iface->index : 0);
  put_u16(buf+2, BGP_AF);
  buf+=4;
  buf = ipa_put_addr(buf, conn->sk ? conn->sk->daddr : IPA_NONE);
  buf = ipa_put_addr(buf, conn->sk ? conn->sk->saddr : IPA_NONE);

  return buf;
}

static void
mrt_dump_bgp_packet(struct bgp_conn *conn, byte *pkt, unsigned len)
{
  byte buf[BGP_MAX_PACKET_LENGTH + 128];
  byte *bp = buf + MRTDUMP_HDR_LENGTH;
  int as4 = conn->bgp->as4_session;

  bp = mrt_put_bgp4_hdr(bp, conn, as4);
  memcpy(bp, pkt, len);
  bp += len;
  mrt_dump_message(&conn->bgp->p, BGP4MP, as4 ? BGP4MP_MESSAGE_AS4 : BGP4MP_MESSAGE,
		   buf, bp-buf);
}

static inline u16
convert_state(unsigned state)
{
  /* Convert state from our BS_* values to values used in MRTDump */
  return (state == BS_CLOSE) ? 1 : state + 1;
}

void
mrt_dump_bgp_state_change(struct bgp_conn *conn, unsigned old, unsigned new)
{
  byte buf[128];
  byte *bp = buf + MRTDUMP_HDR_LENGTH;

  bp = mrt_put_bgp4_hdr(bp, conn, 1);
  put_u16(bp+0, convert_state(old));
  put_u16(bp+2, convert_state(new));
  bp += 4;
  mrt_dump_message(&conn->bgp->p, BGP4MP, BGP4MP_STATE_CHANGE_AS4, buf, bp-buf);
}

static byte *
bgp_create_notification(struct bgp_conn *conn, byte *buf)
{
  struct bgp_proto *p = conn->bgp;

  BGP_TRACE(D_PACKETS, "Sending NOTIFICATION(code=%d.%d)", conn->notify_code, conn->notify_subcode);
  buf[0] = conn->notify_code;
  buf[1] = conn->notify_subcode;
  memcpy(buf+2, conn->notify_data, conn->notify_size);
  return buf + 2 + conn->notify_size;
}

#ifdef IPV6
static byte *
bgp_put_cap_ipv6(struct bgp_conn *conn UNUSED, byte *buf)
{
  *buf++ = 1;		/* Capability 1: Multiprotocol extensions */
  *buf++ = 4;		/* Capability data length */
  *buf++ = 0;		/* We support AF IPv6 */
  *buf++ = BGP_AF_IPV6;
  *buf++ = 0;		/* RFU */
  *buf++ = 1;		/* and SAFI 1 */
  return buf;
}

#else

static byte *
bgp_put_cap_ipv4(struct bgp_conn *conn UNUSED, byte *buf)
{
  *buf++ = 1;		/* Capability 1: Multiprotocol extensions */
  *buf++ = 4;		/* Capability data length */
  *buf++ = 0;		/* We support AF IPv4 */
  *buf++ = BGP_AF_IPV4;
  *buf++ = 0;		/* RFU */
  *buf++ = 1;		/* and SAFI 1 */
  return buf;
}
#endif

static byte *
bgp_put_cap_rr(struct bgp_conn *conn UNUSED, byte *buf)
{
  *buf++ = 2;		/* Capability 2: Support for route refresh */
  *buf++ = 0;		/* Capability data length */
  return buf;
}

static byte *
bgp_put_cap_as4(struct bgp_conn *conn, byte *buf)
{
  *buf++ = 65;		/* Capability 65: Support for 4-octet AS number */
  *buf++ = 4;		/* Capability data length */
  put_u32(buf, conn->bgp->local_as);
  return buf + 4;
}

static byte *
bgp_create_open(struct bgp_conn *conn, byte *buf)
{
  struct bgp_proto *p = conn->bgp;
  byte *cap;
  int cap_len;

  BGP_TRACE(D_PACKETS, "Sending OPEN(ver=%d,as=%d,hold=%d,id=%08x)",
	    BGP_VERSION, p->local_as, p->cf->hold_time, p->local_id);
  buf[0] = BGP_VERSION;
  put_u16(buf+1, (p->local_as < 0xFFFF) ? p->local_as : AS_TRANS);
  put_u16(buf+3, p->cf->hold_time);
  put_u32(buf+5, p->local_id);

  if (conn->start_state == BSS_CONNECT_NOCAP)
    {
      BGP_TRACE(D_PACKETS, "Skipping capabilities");
      buf[9] = 0;
      return buf + 10;
    }

  /* Skipped 3 B for length field and Capabilities parameter header */
  cap = buf + 12;

#ifndef IPV6
  if (p->cf->advertise_ipv4)
    cap = bgp_put_cap_ipv4(conn, cap);
#endif

#ifdef IPV6
  cap = bgp_put_cap_ipv6(conn, cap);
#endif

  if (p->cf->enable_refresh)
    cap = bgp_put_cap_rr(conn, cap);

  if (conn->want_as4_support)
    cap = bgp_put_cap_as4(conn, cap);

  cap_len = cap - buf - 12;
  if (cap_len > 0)
    {
      buf[9]  = cap_len + 2;	/* Optional params len */
      buf[10] = 2;		/* Option: Capability list */
      buf[11] = cap_len;	/* Option length */
      return cap;
    }
  else
    {
      buf[9] = 0;		/* No optional parameters */
      return buf + 10;
    }
}

static unsigned int
bgp_encode_prefixes(struct bgp_proto *p, byte *w, struct bgp_bucket *buck, unsigned int remains)
{
  byte *start = w;
  ip_addr a;
  int bytes;

  while (!EMPTY_LIST(buck->prefixes) && remains >= (1+sizeof(ip_addr)))
    {
      struct bgp_prefix *px = SKIP_BACK(struct bgp_prefix, bucket_node, HEAD(buck->prefixes));
      DBG("\tDequeued route %I/%d\n", px->n.prefix, px->n.pxlen);
      *w++ = px->n.pxlen;
      bytes = (px->n.pxlen + 7) / 8;
      a = px->n.prefix;
      ipa_hton(a);
      memcpy(w, &a, bytes);
      w += bytes;
      remains -= bytes + 1;
      rem_node(&px->bucket_node);
      fib_delete(&p->prefix_fib, px);
    }
  return w - start;
}

static void
bgp_flush_prefixes(struct bgp_proto *p, struct bgp_bucket *buck)
{
  while (!EMPTY_LIST(buck->prefixes))
    {
      struct bgp_prefix *px = SKIP_BACK(struct bgp_prefix, bucket_node, HEAD(buck->prefixes));
      log(L_ERR "%s: - route %I/%d skipped", p->p.name, px->n.prefix, px->n.pxlen);
      rem_node(&px->bucket_node);
      fib_delete(&p->prefix_fib, px);
    }
}

#ifndef IPV6		/* IPv4 version */

static byte *
bgp_create_update(struct bgp_conn *conn, byte *buf)
{
  struct bgp_proto *p = conn->bgp;
  struct bgp_bucket *buck;
  int remains = BGP_MAX_PACKET_LENGTH - BGP_HEADER_LENGTH - 4;
  byte *w;
  int wd_size = 0;
  int r_size = 0;
  int a_size = 0;

  w = buf+2;
  if ((buck = p->withdraw_bucket) && !EMPTY_LIST(buck->prefixes))
    {
      DBG("Withdrawn routes:\n");
      wd_size = bgp_encode_prefixes(p, w, buck, remains);
      w += wd_size;
      remains -= wd_size;
    }
  put_u16(buf, wd_size);

  if (remains >= 3072)
    {
      while ((buck = (struct bgp_bucket *) HEAD(p->bucket_queue))->send_node.next)
	{
	  if (EMPTY_LIST(buck->prefixes))
	    {
	      DBG("Deleting empty bucket %p\n", buck);
	      rem_node(&buck->send_node);
	      bgp_free_bucket(p, buck);
	      continue;
	    }

	  DBG("Processing bucket %p\n", buck);
	  a_size = bgp_encode_attrs(p, w+2, buck->eattrs, 2048);

	  if (a_size < 0)
	    {
	      log(L_ERR "%s: Attribute list too long, skipping corresponding routes", p->p.name);
	      bgp_flush_prefixes(p, buck);
	      rem_node(&buck->send_node);
	      bgp_free_bucket(p, buck);
	      continue;
	    }

	  put_u16(w, a_size);
	  w += a_size + 2;
	  r_size = bgp_encode_prefixes(p, w, buck, remains - a_size);
	  w += r_size;
	  break;
	}
    }
  if (!a_size)				/* Attributes not already encoded */
    {
      put_u16(w, 0);
      w += 2;
    }
  if (wd_size || r_size)
    {
      BGP_TRACE_RL(&rl_snd_update, D_PACKETS, "Sending UPDATE");
      return w;
    }
  else
    return NULL;
}

#else		/* IPv6 version */

static inline int
same_iface(struct bgp_proto *p, ip_addr *ip)
{
  neighbor *n = neigh_find(&p->p, ip, 0);
  return n && p->neigh && n->iface == p->neigh->iface;
}

static byte *
bgp_create_update(struct bgp_conn *conn, byte *buf)
{
  struct bgp_proto *p = conn->bgp;
  struct bgp_bucket *buck;
  int size, second, rem_stored;
  int remains = BGP_MAX_PACKET_LENGTH - BGP_HEADER_LENGTH - 4;
  byte *w, *w_stored, *tmp, *tstart;
  ip_addr *ipp, ip, ip_ll;
  ea_list *ea;
  eattr *nh;

  put_u16(buf, 0);
  w = buf+4;

  if ((buck = p->withdraw_bucket) && !EMPTY_LIST(buck->prefixes))
    {
      DBG("Withdrawn routes:\n");
      tmp = bgp_attach_attr_wa(&ea, bgp_linpool, BA_MP_UNREACH_NLRI, remains-8);
      *tmp++ = 0;
      *tmp++ = BGP_AF_IPV6;
      *tmp++ = 1;
      ea->attrs[0].u.ptr->length = 3 + bgp_encode_prefixes(p, tmp, buck, remains-11);
      size = bgp_encode_attrs(p, w, ea, remains);
      ASSERT(size >= 0);
      w += size;
      remains -= size;
    }

  if (remains >= 3072)
    {
      while ((buck = (struct bgp_bucket *) HEAD(p->bucket_queue))->send_node.next)
	{
	  if (EMPTY_LIST(buck->prefixes))
	    {
	      DBG("Deleting empty bucket %p\n", buck);
	      rem_node(&buck->send_node);
	      bgp_free_bucket(p, buck);
	      continue;
	    }

	  DBG("Processing bucket %p\n", buck);
	  rem_stored = remains;
	  w_stored = w;

	  size = bgp_encode_attrs(p, w, buck->eattrs, 2048);
	  if (size < 0)
	    {
	      log(L_ERR "%s: Attribute list too long, skipping corresponding routes", p->p.name);
	      bgp_flush_prefixes(p, buck);
	      rem_node(&buck->send_node);
	      bgp_free_bucket(p, buck);
	      continue;
	    }
	  w += size;
	  remains -= size;

	  /* We have two addresses here in NEXT_HOP eattr. Really.
	     Unless NEXT_HOP was modified by filter */
	  nh = ea_find(buck->eattrs, EA_CODE(EAP_BGP, BA_NEXT_HOP));
	  ASSERT(nh);
	  second = (nh->u.ptr->length == NEXT_HOP_LENGTH);
	  ipp = (ip_addr *) nh->u.ptr->data;
	  ip = ipp[0];
	  ip_ll = IPA_NONE;

	  if (ipa_equal(ip, p->source_addr))
	    ip_ll = p->local_link;
	  else
	    {
	      /* If we send a route with 'third party' next hop destinated 
	       * in the same interface, we should also send a link local 
	       * next hop address. We use the received one (stored in the 
	       * other part of BA_NEXT_HOP eattr). If we didn't received
	       * it (for example it is a static route), we can't use
	       * 'third party' next hop and we have to use local IP address
	       * as next hop. Sending original next hop address without
	       * link local address seems to be a natural way to solve that
	       * problem, but it is contrary to RFC 2545 and Quagga does not
	       * accept such routes.
	       *
	       * There are two cases, either we have global IP, or
	       * IPA_NONE if the neighbor is link-local. For IPA_NONE,
	       * we suppose it is on the same iface, see bgp_update_attrs().
	       */

	      if (ipa_zero(ip) || same_iface(p, &ip))
		{
		  if (second && ipa_nonzero(ipp[1]))
		    ip_ll = ipp[1];
		  else
		    {
		      switch (p->cf->missing_lladdr)
			{
			case MLL_SELF:
			  ip = p->source_addr;
			  ip_ll = p->local_link;
			  break;
			case MLL_DROP:
			  log(L_ERR "%s: Missing link-local next hop address, skipping corresponding routes", p->p.name);
			  w = w_stored;
			  remains = rem_stored;
			  bgp_flush_prefixes(p, buck);
			  rem_node(&buck->send_node);
			  bgp_free_bucket(p, buck);
			  continue;
			case MLL_IGNORE:
			  break;
			}
		    }
		}
	    }

	  tstart = tmp = bgp_attach_attr_wa(&ea, bgp_linpool, BA_MP_REACH_NLRI, remains-8);
	  *tmp++ = 0;
	  *tmp++ = BGP_AF_IPV6;
	  *tmp++ = 1;

	  if (ipa_has_link_scope(ip))
	    ip = IPA_NONE;

	  if (ipa_nonzero(ip_ll))
	    {
	      *tmp++ = 32;
	      ipa_hton(ip);
	      memcpy(tmp, &ip, 16);
	      ipa_hton(ip_ll);
	      memcpy(tmp+16, &ip_ll, 16);
	      tmp += 32;
	    }
	  else
	    {
	      *tmp++ = 16;
	      ipa_hton(ip);
	      memcpy(tmp, &ip, 16);
	      tmp += 16;
	    }

	  *tmp++ = 0;			/* No SNPA information */
	  tmp += bgp_encode_prefixes(p, tmp, buck, remains - (8+3+32+1));
	  ea->attrs[0].u.ptr->length = tmp - tstart;
	  size = bgp_encode_attrs(p, w, ea, remains);
	  ASSERT(size >= 0);
	  w += size;
	  break;
	}
    }

  size = w - (buf+4);
  put_u16(buf+2, size);
  lp_flush(bgp_linpool);
  if (size)
    {
      BGP_TRACE_RL(&rl_snd_update, D_PACKETS, "Sending UPDATE");
      return w;
    }
  else
    return NULL;
}

#endif

static byte *
bgp_create_route_refresh(struct bgp_conn *conn, byte *buf)
{
  struct bgp_proto *p = conn->bgp;
  BGP_TRACE(D_PACKETS, "Sending ROUTE-REFRESH");

  *buf++ = 0;
  *buf++ = BGP_AF;
  *buf++ = 0;		/* RFU */
  *buf++ = 1;		/* and SAFI 1 */
  return buf;
}

static void
bgp_create_header(byte *buf, unsigned int len, unsigned int type)
{
  memset(buf, 0xff, 16);		/* Marker */
  put_u16(buf+16, len);
  buf[18] = type;
}

/**
 * bgp_fire_tx - transmit packets
 * @conn: connection
 *
 * Whenever the transmit buffers of the underlying TCP connection
 * are free and we have any packets queued for sending, the socket functions
 * call bgp_fire_tx() which takes care of selecting the highest priority packet
 * queued (Notification > Keepalive > Open > Update), assembling its header
 * and body and sending it to the connection.
 */
static int
bgp_fire_tx(struct bgp_conn *conn)
{
  struct bgp_proto *p = conn->bgp;
  unsigned int s = conn->packets_to_send;
  sock *sk = conn->sk;
  byte *buf, *pkt, *end;
  int type;

  if (!sk)
    {
      conn->packets_to_send = 0;
      return 0;
    }
  buf = sk->tbuf;
  pkt = buf + BGP_HEADER_LENGTH;

  if (s & (1 << PKT_SCHEDULE_CLOSE))
    {
      /* We can finally close connection and enter idle state */
      bgp_conn_enter_idle_state(conn);
      return 0;
    }
  if (s & (1 << PKT_NOTIFICATION))
    {
      s = 1 << PKT_SCHEDULE_CLOSE;
      type = PKT_NOTIFICATION;
      end = bgp_create_notification(conn, pkt);
    }
  else if (s & (1 << PKT_KEEPALIVE))
    {
      s &= ~(1 << PKT_KEEPALIVE);
      type = PKT_KEEPALIVE;
      end = pkt;			/* Keepalives carry no data */
      BGP_TRACE(D_PACKETS, "Sending KEEPALIVE");
      bgp_start_timer(conn->keepalive_timer, conn->keepalive_time);
    }
  else if (s & (1 << PKT_OPEN))
    {
      s &= ~(1 << PKT_OPEN);
      type = PKT_OPEN;
      end = bgp_create_open(conn, pkt);
    }
  else if (s & (1 << PKT_ROUTE_REFRESH))
    {
      s &= ~(1 << PKT_ROUTE_REFRESH);
      type = PKT_ROUTE_REFRESH;
      end = bgp_create_route_refresh(conn, pkt);
    }
  else if (s & (1 << PKT_UPDATE))
    {
      end = bgp_create_update(conn, pkt);
      type = PKT_UPDATE;
      if (!end)
	{
	  conn->packets_to_send = 0;
	  return 0;
	}
    }
  else
    return 0;
  conn->packets_to_send = s;
  bgp_create_header(buf, end - buf, type);
  return sk_send(sk, end - buf);
}

/**
 * bgp_schedule_packet - schedule a packet for transmission
 * @conn: connection
 * @type: packet type
 *
 * Schedule a packet of type @type to be sent as soon as possible.
 */
void
bgp_schedule_packet(struct bgp_conn *conn, int type)
{
  DBG("BGP: Scheduling packet type %d\n", type);
  conn->packets_to_send |= 1 << type;
  if (conn->sk && conn->sk->tpos == conn->sk->tbuf)
    ev_schedule(conn->tx_ev);
}

void
bgp_kick_tx(void *vconn)
{
  struct bgp_conn *conn = vconn;

  DBG("BGP: kicking TX\n");
  while (bgp_fire_tx(conn) > 0)
    ;
}

void
bgp_tx(sock *sk)
{
  struct bgp_conn *conn = sk->data;

  DBG("BGP: TX hook\n");
  while (bgp_fire_tx(conn) > 0)
    ;
}

/* Capatibility negotiation as per RFC 2842 */

void
bgp_parse_capabilities(struct bgp_conn *conn, byte *opt, int len)
{
  // struct bgp_proto *p = conn->bgp;
  int cl;

  while (len > 0)
    {
      if (len < 2 || len < 2 + opt[1])
	goto err;
      
      cl = opt[1];

      switch (opt[0])
	{
	case 2:	/* Route refresh capability, RFC 2918 */
	  if (cl != 0)
	    goto err;
	  conn->peer_refresh_support = 1;
	  break;

	case 65: /* AS4 capability, RFC 4893 */ 
	  if (cl != 4)
	    goto err;
	  conn->peer_as4_support = 1;
	  if (conn->want_as4_support)
	    conn->advertised_as = get_u32(opt + 2);
	  break;

	  /* We can safely ignore all other capabilities */
	}
      len -= 2 + cl;
      opt += 2 + cl;
    }
  return;

    err:
  bgp_error(conn, 2, 0, NULL, 0);
  return;
}

static int
bgp_parse_options(struct bgp_conn *conn, byte *opt, int len)
{
  struct bgp_proto *p = conn->bgp;
  int ol;

  while (len > 0)
    {
      if (len < 2 || len < 2 + opt[1])
	{ bgp_error(conn, 2, 0, NULL, 0); return 0; }
#ifdef LOCAL_DEBUG
      {
	int i;
	DBG("\tOption %02x:", opt[0]);
	for(i=0; i<opt[1]; i++)
	  DBG(" %02x", opt[2+i]);
	DBG("\n");
      }
#endif

      ol = opt[1];
      switch (opt[0])
	{
	case 2:
	  if (conn->start_state == BSS_CONNECT_NOCAP)
	    BGP_TRACE(D_PACKETS, "Ignoring received capabilities");
	  else
	    bgp_parse_capabilities(conn, opt + 2, ol);
	  break;

	default:
	  /*
	   *  BGP specs don't tell us to send which option
	   *  we didn't recognize, but it's common practice
	   *  to do so. Also, capability negotiation with
	   *  Cisco routers doesn't work without that.
	   */
	  bgp_error(conn, 2, 4, opt, ol);
	  return 0;
	}
      len -= 2 + ol;
      opt += 2 + ol;
    }
  return 0;
}

static void
bgp_rx_open(struct bgp_conn *conn, byte *pkt, int len)
{
  struct bgp_conn *other;
  struct bgp_proto *p = conn->bgp;
  unsigned hold;
  u16 base_as;
  u32 id;

  /* Check state */
  if (conn->state != BS_OPENSENT)
    { bgp_error(conn, 5, fsm_err_subcode[conn->state], NULL, 0); return; }

  /* Check message contents */
  if (len < 29 || len != 29 + pkt[28])
    { bgp_error(conn, 1, 2, pkt+16, 2); return; }
  if (pkt[19] != BGP_VERSION)
    { bgp_error(conn, 2, 1, pkt+19, 1); return; } /* RFC 1771 says 16 bits, draft-09 tells to use 8 */
  conn->advertised_as = base_as = get_u16(pkt+20);
  hold = get_u16(pkt+22);
  id = get_u32(pkt+24);
  BGP_TRACE(D_PACKETS, "Got OPEN(as=%d,hold=%d,id=%08x)", conn->advertised_as, hold, id);

  if (bgp_parse_options(conn, pkt+29, pkt[28]))
    return;

  if (hold > 0 && hold < 3)
    { bgp_error(conn, 2, 6, pkt+22, 2); return; }

  if (!id || id == 0xffffffff || id == p->local_id)
    { bgp_error(conn, 2, 3, pkt+24, -4); return; }

  if ((conn->advertised_as != base_as) && (base_as != AS_TRANS))
    log(L_WARN "%s: Peer advertised inconsistent AS numbers", p->p.name);

  if (conn->advertised_as != p->remote_as)
    {
      if (conn->peer_as4_support)
	{
	  u32 val = htonl(conn->advertised_as);
	  bgp_error(conn, 2, 2, (byte *) &val, 4);
	}
      else
	bgp_error(conn, 2, 2, pkt+20, 2);

      return;
    }

  /* Check the other connection */
  other = (conn == &p->outgoing_conn) ? &p->incoming_conn : &p->outgoing_conn;
  switch (other->state)
    {
    case BS_IDLE:
    case BS_CONNECT:
    case BS_ACTIVE:
    case BS_OPENSENT:
    case BS_CLOSE:
      break;
    case BS_OPENCONFIRM:
      if ((p->local_id < id) == (conn == &p->incoming_conn))
	{
	  /* Should close the other connection */
	  BGP_TRACE(D_EVENTS, "Connection collision, giving up the other connection");
	  bgp_error(other, 6, 7, NULL, 0);
	  break;
	}
      /* Fall thru */
    case BS_ESTABLISHED:
      /* Should close this connection */
      BGP_TRACE(D_EVENTS, "Connection collision, giving up this connection");
      bgp_error(conn, 6, 7, NULL, 0);
      return;
    default:
      bug("bgp_rx_open: Unknown state");
    }

  /* Update our local variables */
  conn->hold_time = MIN(hold, p->cf->hold_time);
  conn->keepalive_time = p->cf->keepalive_time ? : conn->hold_time / 3;
  p->remote_id = id;
  p->as4_session = conn->want_as4_support && conn->peer_as4_support;

  DBG("BGP: Hold timer set to %d, keepalive to %d, AS to %d, ID to %x, AS4 session to %d\n", conn->hold_time, conn->keepalive_time, p->remote_as, p->remote_id, p->as4_session);

  bgp_schedule_packet(conn, PKT_KEEPALIVE);
  bgp_start_timer(conn->hold_timer, conn->hold_time);
  bgp_conn_enter_openconfirm_state(conn);
}

#define DECODE_PREFIX(pp, ll) do {		\
  int b = *pp++;				\
  int q;					\
  ll--;						\
  if (b > BITS_PER_IP_ADDRESS) { err=10; goto done; } \
  q = (b+7) / 8;				\
  if (ll < q) { err=1; goto done; }		\
  memcpy(&prefix, pp, q);			\
  pp += q;					\
  ll -= q;					\
  ipa_ntoh(prefix);				\
  prefix = ipa_and(prefix, ipa_mkmask(b));	\
  pxlen = b;					\
} while (0)

static inline int
bgp_set_next_hop(struct bgp_proto *p, rta *a)
{
  struct eattr *nh = ea_find(a->eattrs, EA_CODE(EAP_BGP, BA_NEXT_HOP));
  ip_addr *nexthop = (ip_addr *) nh->u.ptr->data;

#ifdef IPV6
  int second = (nh->u.ptr->length == NEXT_HOP_LENGTH) && ipa_nonzero(nexthop[1]);

  /* First address should not be link-local, but may be zero in direct mode */
  if (ipa_has_link_scope(*nexthop))
    *nexthop = IPA_NONE;
#else
  int second = 0;
#endif

  if (p->cf->gw_mode == GW_DIRECT)
    {
      neighbor *ng = NULL;

      if (ipa_nonzero(*nexthop))
	ng = neigh_find(&p->p, nexthop, 0);
      else if (second)	/* GW_DIRECT -> single_hop -> p->neigh != NULL */
	ng = neigh_find2(&p->p, nexthop + 1, p->neigh->iface, 0);

      /* Fallback */
      if (!ng)
	ng = p->neigh;

      if (ng->scope == SCOPE_HOST)
	return 0;

      a->dest = RTD_ROUTER;
      a->gw = ng->addr;
      a->iface = ng->iface;
      a->hostentry = NULL;
      a->igp_metric = 0;
    }
  else /* GW_RECURSIVE */
    {
      if (ipa_zero(*nexthop))
	  return 0;

      rta_set_recursive_next_hop(p->p.table, a, p->igp_table, nexthop, nexthop + second);
    }

  return 1;
}

#ifndef IPV6		/* IPv4 version */

static void
bgp_do_rx_update(struct bgp_conn *conn,
		 byte *withdrawn, int withdrawn_len,
		 byte *nlri, int nlri_len,
		 byte *attrs, int attr_len)
{
  struct bgp_proto *p = conn->bgp;
  net *n;
  rta *a0, *a = NULL;
  ip_addr prefix;
  int pxlen, err = 0;

  /* Withdraw routes */
  while (withdrawn_len)
    {
      DECODE_PREFIX(withdrawn, withdrawn_len);
      DBG("Withdraw %I/%d\n", prefix, pxlen);
      if (n = net_find(p->p.table, prefix, pxlen))
	rte_update(p->p.table, n, &p->p, &p->p, NULL);
    }

  if (!attr_len && !nlri_len)		/* shortcut */
    return;

  a0 = bgp_decode_attrs(conn, attrs, attr_len, bgp_linpool, nlri_len);

  if (conn->state != BS_ESTABLISHED)	/* fatal error during decoding */
    return;

  if (a0 && nlri_len && bgp_set_next_hop(p, a0))
    a = rta_lookup(a0);

  while (nlri_len)
    {
      DECODE_PREFIX(nlri, nlri_len);
      DBG("Add %I/%d\n", prefix, pxlen);

      if (a)
	{
	  rte *e = rte_get_temp(rta_clone(a));
	  e->net = net_get(p->p.table, prefix, pxlen);
	  e->pflags = 0;
	  e->u.bgp.suppressed = 0;
	  rte_update(p->p.table, e->net, &p->p, &p->p, e);
	}
      else
	{
	  /* Forced withdraw as a result of soft error */
	  if (n = net_find(p->p.table, prefix, pxlen))
	    rte_update(p->p.table, n, &p->p, &p->p, NULL);
	}
    }

 done:
  if (a)
    rta_free(a);

  if (err)
    bgp_error(conn, 3, err, NULL, 0);

  return;
}

#else			/* IPv6 version */

#define DO_NLRI(name)					\
  start = x = p->name##_start;				\
  len = len0 = p->name##_len;				\
  if (len)						\
    {							\
      if (len < 3) { err=9; goto done; }		\
      af = get_u16(x);					\
      sub = x[2];					\
      x += 3;						\
      len -= 3;						\
      DBG("\tNLRI AF=%d sub=%d len=%d\n", af, sub, len);\
    }							\
  else							\
    af = 0;						\
  if (af == BGP_AF_IPV6)

static void
bgp_attach_next_hop(rta *a0, byte *x)
{
  ip_addr *nh = (ip_addr *) bgp_attach_attr_wa(&a0->eattrs, bgp_linpool, BA_NEXT_HOP, NEXT_HOP_LENGTH);
  memcpy(nh, x+1, 16);
  ipa_ntoh(nh[0]);

  /* We store received link local address in the other part of BA_NEXT_HOP eattr. */
  if (*x == 32)
    {
      memcpy(nh+1, x+17, 16);
      ipa_ntoh(nh[1]);
    }
  else
    nh[1] = IPA_NONE;
}


static void
bgp_do_rx_update(struct bgp_conn *conn,
		 byte *withdrawn, int withdrawn_len,
		 byte *nlri, int nlri_len,
		 byte *attrs, int attr_len)
{
  struct bgp_proto *p = conn->bgp;
  byte *start, *x;
  int len, len0;
  unsigned af, sub;
  net *n;
  rta *a0, *a = NULL;
  ip_addr prefix;
  int pxlen, err = 0;

  p->mp_reach_len = 0;
  p->mp_unreach_len = 0;
  a0 = bgp_decode_attrs(conn, attrs, attr_len, bgp_linpool, 0);

  if (conn->state != BS_ESTABLISHED)	/* fatal error during decoding */
    return;

  DO_NLRI(mp_unreach)
    {
      while (len)
	{
	  DECODE_PREFIX(x, len);
	  DBG("Withdraw %I/%d\n", prefix, pxlen);
	  if (n = net_find(p->p.table, prefix, pxlen))
	    rte_update(p->p.table, n, &p->p, &p->p, NULL);
	}
    }

  DO_NLRI(mp_reach)
    {
      /* Create fake NEXT_HOP attribute */
      if (len < 1 || (*x != 16 && *x != 32) || len < *x + 2)
	{ err = 9; goto done; }

      if (a0)
	bgp_attach_next_hop(a0, x);

      /* Also ignore one reserved byte */
      len -= *x + 2;
      x += *x + 2;

      if (a0 && bgp_set_next_hop(p, a0))
	a = rta_lookup(a0);

      while (len)
	{
	  DECODE_PREFIX(x, len);
	  DBG("Add %I/%d\n", prefix, pxlen);

	  if (a)
	    {
	      rte *e = rte_get_temp(rta_clone(a));
	      e->net = net_get(p->p.table, prefix, pxlen);
	      e->pflags = 0;
	      e->u.bgp.suppressed = 0;
	      rte_update(p->p.table, e->net, &p->p, &p->p, e);
	    }
	  else
	    {
	      /* Forced withdraw as a result of soft error */
	      if (n = net_find(p->p.table, prefix, pxlen))
		rte_update(p->p.table, n, &p->p, &p->p, NULL);
	    }
	}
    }

 done:
  if (a)
    rta_free(a);

  if (err) /* Use subcode 9, not err */
    bgp_error(conn, 3, 9, NULL, 0);

  return;
}

#endif

static void
bgp_rx_update(struct bgp_conn *conn, byte *pkt, int len)
{
  struct bgp_proto *p = conn->bgp;
  byte *withdrawn, *attrs, *nlri;
  int withdrawn_len, attr_len, nlri_len;

  BGP_TRACE_RL(&rl_rcv_update, D_PACKETS, "Got UPDATE");

  /* Workaround for some BGP implementations that skip initial KEEPALIVE */
  if (conn->state == BS_OPENCONFIRM)
    bgp_conn_enter_established_state(conn);

  if (conn->state != BS_ESTABLISHED)
    { bgp_error(conn, 5, fsm_err_subcode[conn->state], NULL, 0); return; }
  bgp_start_timer(conn->hold_timer, conn->hold_time);

  /* Find parts of the packet and check sizes */
  if (len < 23)
    {
      bgp_error(conn, 1, 2, pkt+16, 2);
      return;
    }
  withdrawn = pkt + 21;
  withdrawn_len = get_u16(pkt + 19);
  if (withdrawn_len + 23 > len)
    goto malformed;
  attrs = withdrawn + withdrawn_len + 2;
  attr_len = get_u16(attrs - 2);
  if (withdrawn_len + attr_len + 23 > len)
    goto malformed;
  nlri = attrs + attr_len;
  nlri_len = len - withdrawn_len - attr_len - 23;
  if (!attr_len && nlri_len)
    goto malformed;
  DBG("Sizes: withdrawn=%d, attrs=%d, NLRI=%d\n", withdrawn_len, attr_len, nlri_len);

  lp_flush(bgp_linpool);

  bgp_do_rx_update(conn, withdrawn, withdrawn_len, nlri, nlri_len, attrs, attr_len);
  return;

malformed:
  bgp_error(conn, 3, 1, NULL, 0);
}

static struct {
  byte major, minor;
  byte *msg;
} bgp_msg_table[] = {
  { 1, 0, "Invalid message header" },
  { 1, 1, "Connection not synchronized" },
  { 1, 2, "Bad message length" },
  { 1, 3, "Bad message type" },
  { 2, 0, "Invalid OPEN message" },
  { 2, 1, "Unsupported version number" },
  { 2, 2, "Bad peer AS" },
  { 2, 3, "Bad BGP identifier" },
  { 2, 4, "Unsupported optional parameter" },
  { 2, 5, "Authentication failure" },
  { 2, 6, "Unacceptable hold time" },
  { 2, 7, "Required capability missing" }, /* [RFC3392] */
  { 2, 8, "No supported AFI/SAFI" }, /* This error msg is nonstandard */
  { 3, 0, "Invalid UPDATE message" },
  { 3, 1, "Malformed attribute list" },
  { 3, 2, "Unrecognized well-known attribute" },
  { 3, 3, "Missing mandatory attribute" },
  { 3, 4, "Invalid attribute flags" },
  { 3, 5, "Invalid attribute length" },
  { 3, 6, "Invalid ORIGIN attribute" },
  { 3, 7, "AS routing loop" },		/* Deprecated */
  { 3, 8, "Invalid NEXT_HOP attribute" },
  { 3, 9, "Optional attribute error" },
  { 3, 10, "Invalid network field" },
  { 3, 11, "Malformed AS_PATH" },
  { 4, 0, "Hold timer expired" },
  { 5, 0, "Finite state machine error" }, /* Subcodes are according to [RFC6608] */
  { 5, 1, "Unexpected message in OpenSent state" },
  { 5, 2, "Unexpected message in OpenConfirm state" },
  { 5, 3, "Unexpected message in Established state" },
  { 6, 0, "Cease" }, /* Subcodes are according to [RFC4486] */
  { 6, 1, "Maximum number of prefixes reached" },
  { 6, 2, "Administrative shutdown" },
  { 6, 3, "Peer de-configured" },
  { 6, 4, "Administrative reset" },
  { 6, 5, "Connection rejected" },
  { 6, 6, "Other configuration change" },
  { 6, 7, "Connection collision resolution" },
  { 6, 8, "Out of Resources" }
};

/**
 * bgp_error_dsc - return BGP error description
 * @code: BGP error code
 * @subcode: BGP error subcode
 *
 * bgp_error_dsc() returns error description for BGP errors
 * which might be static string or given temporary buffer.
 */
const char *
bgp_error_dsc(unsigned code, unsigned subcode)
{
  static char buff[32];
  unsigned i;
  for (i=0; i < ARRAY_SIZE(bgp_msg_table); i++)
    if (bgp_msg_table[i].major == code && bgp_msg_table[i].minor == subcode)
      {
	return bgp_msg_table[i].msg;
      }

  bsprintf(buff, "Unknown error %d.%d", code, subcode);
  return buff;
}

void
bgp_log_error(struct bgp_proto *p, u8 class, char *msg, unsigned code, unsigned subcode, byte *data, unsigned len)
{
  const byte *name;
  byte *t, argbuf[36];
  unsigned i;

  /* Don't report Cease messages generated by myself */
  if (code == 6 && class == BE_BGP_TX)
    return;

  name = bgp_error_dsc(code, subcode);
  t = argbuf;
  if (len)
    {
      *t++ = ':';
      *t++ = ' ';

      if ((code == 2) && (subcode == 2) && ((len == 2) || (len == 4)))
	{
	  /* Bad peer AS - we would like to print the AS */
	  t += bsprintf(t, "%d", (len == 2) ? get_u16(data) : get_u32(data));
	  goto done;
	}
      if (len > 16)
	len = 16;
      for (i=0; i<len; i++)
	t += bsprintf(t, "%02x", data[i]);
    }
 done:
  *t = 0;
  log(L_REMOTE "%s: %s: %s%s", p->p.name, msg, name, argbuf);
}

static void
bgp_rx_notification(struct bgp_conn *conn, byte *pkt, int len)
{
  struct bgp_proto *p = conn->bgp;
  if (len < 21)
    {
      bgp_error(conn, 1, 2, pkt+16, 2);
      return;
    }

  unsigned code = pkt[19];
  unsigned subcode = pkt[20];
  int err = (code != 6);

  bgp_log_error(p, BE_BGP_RX, "Received", code, subcode, pkt+21, len-21);
  bgp_store_error(p, conn, BE_BGP_RX, (code << 16) | subcode);

#ifndef IPV6
  if ((code == 2) && ((subcode == 4) || (subcode == 7))
      /* Error related to capability:
       * 4 - Peer does not support capabilities at all.
       * 7 - Peer request some capability. Strange unless it is IPv6 only peer.
       */
      && (p->cf->capabilities == 2)
      /* Capabilities are not explicitly enabled or disabled, therefore heuristic is used */
      && (conn->start_state == BSS_CONNECT)
      /* Failed connection attempt have used capabilities */
      && (p->cf->remote_as <= 0xFFFF))
      /* Not possible with disabled capabilities */
    {
      /* We try connect without capabilities */
      log(L_WARN "%s: Capability related error received, retry with capabilities disabled", p->p.name);
      p->start_state = BSS_CONNECT_NOCAP;
      err = 0;
    }
#endif

  bgp_conn_enter_close_state(conn);
  bgp_schedule_packet(conn, PKT_SCHEDULE_CLOSE);

  if (err) 
    {
      bgp_update_startup_delay(p);
      bgp_stop(p, 0);
    }
}

static void
bgp_rx_keepalive(struct bgp_conn *conn)
{
  struct bgp_proto *p = conn->bgp;

  BGP_TRACE(D_PACKETS, "Got KEEPALIVE");
  bgp_start_timer(conn->hold_timer, conn->hold_time);
  switch (conn->state)
    {
    case BS_OPENCONFIRM:
      bgp_conn_enter_established_state(conn);
      break;
    case BS_ESTABLISHED:
      break;
    default:
      bgp_error(conn, 5, fsm_err_subcode[conn->state], NULL, 0);
    }
}

static void
bgp_rx_route_refresh(struct bgp_conn *conn, byte *pkt, int len)
{
  struct bgp_proto *p = conn->bgp;

  BGP_TRACE(D_PACKETS, "Got ROUTE-REFRESH");

  if (conn->state != BS_ESTABLISHED)
    { bgp_error(conn, 5, fsm_err_subcode[conn->state], NULL, 0); return; }

  if (!p->cf->enable_refresh)
    { bgp_error(conn, 1, 3, pkt+18, 1); return; }

  if (len != (BGP_HEADER_LENGTH + 4))
    { bgp_error(conn, 1, 2, pkt+16, 2); return; }

  /* FIXME - we ignore AFI/SAFI values, as we support
     just one value and even an error code for an invalid
     request is not defined */

  proto_request_feeding(&p->p);
}


/**
 * bgp_rx_packet - handle a received packet
 * @conn: BGP connection
 * @pkt: start of the packet
 * @len: packet size
 *
 * bgp_rx_packet() takes a newly received packet and calls the corresponding
 * packet handler according to the packet type.
 */
static void
bgp_rx_packet(struct bgp_conn *conn, byte *pkt, unsigned len)
{
  byte type = pkt[18];

  DBG("BGP: Got packet %02x (%d bytes)\n", type, len);

  if (conn->bgp->p.mrtdump & MD_MESSAGES)
    mrt_dump_bgp_packet(conn, pkt, len);

  switch (type)
    {
    case PKT_OPEN:		return bgp_rx_open(conn, pkt, len);
    case PKT_UPDATE:		return bgp_rx_update(conn, pkt, len);
    case PKT_NOTIFICATION:      return bgp_rx_notification(conn, pkt, len);
    case PKT_KEEPALIVE:		return bgp_rx_keepalive(conn);
    case PKT_ROUTE_REFRESH:	return bgp_rx_route_refresh(conn, pkt, len);
    default:			bgp_error(conn, 1, 3, pkt+18, 1);
    }
}

/**
 * bgp_rx - handle received data
 * @sk: socket
 * @size: amount of data received
 *
 * bgp_rx() is called by the socket layer whenever new data arrive from
 * the underlying TCP connection. It assembles the data fragments to packets,
 * checks their headers and framing and passes complete packets to
 * bgp_rx_packet().
 */
int
bgp_rx(sock *sk, int size)
{
  struct bgp_conn *conn = sk->data;
  byte *pkt_start = sk->rbuf;
  byte *end = pkt_start + size;
  unsigned i, len;

  DBG("BGP: RX hook: Got %d bytes\n", size);
  while (end >= pkt_start + BGP_HEADER_LENGTH)
    {
      if ((conn->state == BS_CLOSE) || (conn->sk != sk))
	return 0;
      for(i=0; i<16; i++)
	if (pkt_start[i] != 0xff)
	  {
	    bgp_error(conn, 1, 1, NULL, 0);
	    break;
	  }
      len = get_u16(pkt_start+16);
      if (len < BGP_HEADER_LENGTH || len > BGP_MAX_PACKET_LENGTH)
	{
	  bgp_error(conn, 1, 2, pkt_start+16, 2);
	  break;
	}
      if (end < pkt_start + len)
	break;
      bgp_rx_packet(conn, pkt_start, len);
      pkt_start += len;
    }
  if (pkt_start != sk->rbuf)
    {
      memmove(sk->rbuf, pkt_start, end - pkt_start);
      sk->rpos = sk->rbuf + (end - pkt_start);
    }
  return 0;
}
