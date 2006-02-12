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
#include "conf/conf.h"
#include "lib/unaligned.h"
#include "lib/socket.h"

#include "bgp.h"

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

static byte *
bgp_create_open(struct bgp_conn *conn, byte *buf)
{
  struct bgp_proto *p = conn->bgp;

  BGP_TRACE(D_PACKETS, "Sending OPEN(ver=%d,as=%d,hold=%d,id=%08x)",
	    BGP_VERSION, p->local_as, p->cf->hold_time, p->local_id);
  buf[0] = BGP_VERSION;
  put_u16(buf+1, p->local_as);
  put_u16(buf+3, p->cf->hold_time);
  put_u32(buf+5, p->local_id);
#ifndef IPV6
  buf[9] = 0;				/* No optional parameters */
  return buf+10;
#else
  buf += 9;
  *buf++ = 8;		/* Optional params len */
  *buf++ = 2;		/* Option: Capability list */
  *buf++ = 6;		/* Option length */
  *buf++ = 1;		/* Capability 1: Multiprotocol extensions */
  *buf++ = 4;		/* Capability data length */
  *buf++ = 0;		/* We support AF IPv6 */
  *buf++ = BGP_AF_IPV6;
  *buf++ = 0;		/* RFU */
  *buf++ = 1;		/* and SAFI 1 */
  return buf;
#endif
}

static unsigned int
bgp_encode_prefixes(struct bgp_proto *p, byte *w, struct bgp_bucket *buck, unsigned int remains)
{
  byte *start = w;
  ip_addr a;
  int bytes;

  while (!EMPTY_LIST(buck->prefixes) && remains >= 5)
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

  if (remains >= 2048)
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
	  a_size = bgp_encode_attrs(w+2, buck->eattrs, 1024);
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
      BGP_TRACE(D_PACKETS, "Sending UPDATE");
      return w;
    }
  else
    return NULL;
}

#else		/* IPv6 version */

static byte *
bgp_create_update(struct bgp_conn *conn, byte *buf)
{
  struct bgp_proto *p = conn->bgp;
  struct bgp_bucket *buck;
  int size, is_ll;
  int remains = BGP_MAX_PACKET_LENGTH - BGP_HEADER_LENGTH - 4;
  byte *w, *tmp, *tstart;
  ip_addr ip, ip_ll;
  ea_list *ea;
  eattr *nh;
  neighbor *n;

  put_u16(buf, 0);
  w = buf+4;

  if ((buck = p->withdraw_bucket) && !EMPTY_LIST(buck->prefixes))
    {
      DBG("Withdrawn routes:\n");
      tmp = bgp_attach_attr(&ea, bgp_linpool, BA_MP_UNREACH_NLRI, remains-8);
      *tmp++ = 0;
      *tmp++ = BGP_AF_IPV6;
      *tmp++ = 1;
      ea->attrs[0].u.ptr->length = bgp_encode_prefixes(p, tmp, buck, remains-11);
      size = bgp_encode_attrs(w, ea, remains);
      w += size;
      remains -= size;
    }

  if (remains >= 2048)
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
	  size = bgp_encode_attrs(w, buck->eattrs, 1024);
	  w += size;
	  remains -= size;
	  tstart = tmp = bgp_attach_attr(&ea, bgp_linpool, BA_MP_REACH_NLRI, remains-8);
	  *tmp++ = 0;
	  *tmp++ = BGP_AF_IPV6;
	  *tmp++ = 1;
	  nh = ea_find(buck->eattrs, EA_CODE(EAP_BGP, BA_NEXT_HOP));
	  ASSERT(nh);
	  ip = *(ip_addr *) nh->u.ptr->data;
	  is_ll = 0;
	  if (ipa_equal(ip, p->local_addr))
	    {
	      is_ll = 1;
	      ip_ll = p->local_link;
	    }
	  else
	    {
	      n = neigh_find(&p->p, &ip, 0);
	      if (n && n->iface == p->neigh->iface)
		{
		  /* FIXME: We are assuming the global scope addresses use the lower 64 bits
		   * as an interface identifier which hasn't necessarily to be true.
		   */
		  is_ll = 1;
	          ip_ll = ipa_or(ipa_build(0xfe800000,0,0,0), ipa_and(ip, ipa_build(0,0,~0,~0)));
		}
	    }
	  if (is_ll)
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
	  w += bgp_encode_attrs(w, ea, remains);
	  break;
	}
    }

  size = w - (buf+4);
  put_u16(buf+2, size);
  lp_flush(bgp_linpool);
  if (size)
    {
      BGP_TRACE(D_PACKETS, "Sending UPDATE");
      return w;
    }
  else
    return NULL;
}

#endif

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
      bgp_close_conn(conn);
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
    while (bgp_fire_tx(conn))
      ;
}

void
bgp_tx(sock *sk)
{
  struct bgp_conn *conn = sk->data;

  DBG("BGP: TX hook\n");
  while (bgp_fire_tx(conn))
    ;
}

static int
bgp_parse_options(struct bgp_conn *conn, byte *opt, int len)
{
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
      switch (opt[0])
	{
	case 2:
	  /* Capatibility negotiation as per RFC 2842 */
	  /* We can safely ignore all capabilities announced */
	  break;
	default:
	  /*
	   *  BGP specs don't tell us to send which option
	   *  we didn't recognize, but it's common practice
	   *  to do so. Also, capability negotiation with
	   *  Cisco routers doesn't work without that.
	   */
	  bgp_error(conn, 2, 4, opt, opt[1]);
	  return 0;
	}
      len -= 2 + opt[1];
      opt += 2 + opt[1];
    }
  return 0;
}

static void
bgp_rx_open(struct bgp_conn *conn, byte *pkt, int len)
{
  struct bgp_conn *other;
  struct bgp_proto *p = conn->bgp;
  struct bgp_config *cf = p->cf;
  unsigned as, hold;
  u32 id;

  /* Check state */
  if (conn->state != BS_OPENSENT)
    { bgp_error(conn, 5, 0, NULL, 0); }

  /* Check message contents */
  if (len < 29 || len != 29 + pkt[28])
    { bgp_error(conn, 1, 2, pkt+16, 2); return; }
  if (pkt[19] != BGP_VERSION)
    { bgp_error(conn, 2, 1, pkt+19, 1); return; } /* RFC 1771 says 16 bits, draft-09 tells to use 8 */
  as = get_u16(pkt+20);
  hold = get_u16(pkt+22);
  id = get_u32(pkt+24);
  BGP_TRACE(D_PACKETS, "Got OPEN(as=%d,hold=%d,id=%08x)", as, hold, id);
  if (cf->remote_as && as != p->remote_as)
    { bgp_error(conn, 2, 2, pkt+20, -2); return; }
  if (hold > 0 && hold < 3)
    { bgp_error(conn, 2, 6, pkt+22, 2); return; }
  p->remote_id = id;
  if (bgp_parse_options(conn, pkt+29, pkt[28]))
    return;
  if (!id || id == 0xffffffff || id == p->local_id)
    { bgp_error(conn, 2, 3, pkt+24, -4); return; }

  /* Check the other connection */
  other = (conn == &p->outgoing_conn) ? &p->incoming_conn : &p->outgoing_conn;
  switch (other->state)
    {
    case BS_IDLE:
      break;
    case BS_CONNECT:
    case BS_ACTIVE:
    case BS_OPENSENT:
      BGP_TRACE(D_EVENTS, "Connection collision, giving up the other connection");
      bgp_close_conn(other);
      break;
    case BS_OPENCONFIRM:
      if ((p->local_id < id) == (conn == &p->incoming_conn))
	{
	  /* Should close the other connection */
	  BGP_TRACE(D_EVENTS, "Connection collision, giving up the other connection");
	  bgp_error(other, 6, 0, NULL, 0);
	  break;
	}
      /* Fall thru */
    case BS_ESTABLISHED:
      /* Should close this connection */
      BGP_TRACE(D_EVENTS, "Connection collision, giving up this connection");
      bgp_error(conn, 6, 0, NULL, 0);
      return;
    default:
      bug("bgp_rx_open: Unknown state");
    }

  /* Make this connection primary */
  conn->primary = 1;
  p->conn = conn;

  /* Update our local variables */
  if (hold < p->cf->hold_time)
    conn->hold_time = hold;
  else
    conn->hold_time = p->cf->hold_time;
  conn->keepalive_time = p->cf->keepalive_time ? : conn->hold_time / 3;
  p->remote_as = as;
  p->remote_id = id;
  DBG("BGP: Hold timer set to %d, keepalive to %d, AS to %d, ID to %x\n", conn->hold_time, conn->keepalive_time, p->remote_as, p->remote_id);

  bgp_schedule_packet(conn, PKT_KEEPALIVE);
  bgp_start_timer(conn->hold_timer, conn->hold_time);
  conn->state = BS_OPENCONFIRM;
}

#define DECODE_PREFIX(pp, ll) do {		\
  int b = *pp++;				\
  int q;					\
  ll--;						\
  if (b > BITS_PER_IP_ADDRESS) { err=10; goto bad; } \
  q = (b+7) / 8;				\
  if (ll < q) { err=1; goto bad; }		\
  memcpy(&prefix, pp, q);			\
  pp += q;					\
  ll -= q;					\
  ipa_ntoh(prefix);				\
  prefix = ipa_and(prefix, ipa_mkmask(b));	\
  pxlen = b;					\
} while (0)

static inline int
bgp_get_nexthop(struct bgp_proto *bgp, rta *a)
{
  neighbor *neigh;
  ip_addr nexthop;
  struct eattr *nh = ea_find(a->eattrs, EA_CODE(EAP_BGP, BA_NEXT_HOP));
  ASSERT(nh);
  nexthop = *(ip_addr *) nh->u.ptr->data;
  neigh = neigh_find(&bgp->p, &nexthop, 0);
  if (neigh)
    {
      if (neigh->scope == SCOPE_HOST)
	{
	  DBG("BGP: Loop!\n");
	  return 0;
	}
    }
  else
    neigh = bgp->neigh;
  a->gw = neigh->addr;
  a->iface = neigh->iface;
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
  rta *a0;
  rta *a = NULL;
  ip_addr prefix;
  net *n;
  int err = 0, pxlen;

  /* Withdraw routes */
  while (withdrawn_len)
    {
      DECODE_PREFIX(withdrawn, withdrawn_len);
      DBG("Withdraw %I/%d\n", prefix, pxlen);
      if (n = net_find(p->p.table, prefix, pxlen))
	rte_update(p->p.table, n, &p->p, NULL);
    }

  if (!attr_len && !nlri_len)		/* shortcut */
    return;

  a0 = bgp_decode_attrs(conn, attrs, attr_len, bgp_linpool, nlri_len);
  if (a0 && nlri_len && bgp_get_nexthop(p, a0))
    {
      a = rta_lookup(a0);
      while (nlri_len)
	{
	  rte *e;
	  DECODE_PREFIX(nlri, nlri_len);
	  DBG("Add %I/%d\n", prefix, pxlen);
	  e = rte_get_temp(rta_clone(a));
	  n = net_get(p->p.table, prefix, pxlen);
	  e->net = n;
	  e->pflags = 0;
	  rte_update(p->p.table, n, &p->p, e);
	}
    }
bad:
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
      if (len < 3) goto bad;				\
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
bgp_do_rx_update(struct bgp_conn *conn,
		 byte *withdrawn, int withdrawn_len,
		 byte *nlri, int nlri_len,
		 byte *attrs, int attr_len)
{
  struct bgp_proto *p = conn->bgp;
  byte *start, *x;
  int len, len0;
  unsigned af, sub;
  rta *a0;
  rta *a = NULL;
  ip_addr prefix;
  net *n;
  rte e;
  int err = 0, pxlen;

  p->mp_reach_len = 0;
  p->mp_unreach_len = 0;
  a0 = bgp_decode_attrs(conn, attrs, attr_len, bgp_linpool, 0);
  if (!a0)
    return;

  DO_NLRI(mp_unreach)
    {
      while (len)
	{
	  DECODE_PREFIX(x, len);
	  DBG("Withdraw %I/%d\n", prefix, pxlen);
	  if (n = net_find(p->p.table, prefix, pxlen))
	    rte_update(p->p.table, n, &p->p, NULL);
	}
    }

  DO_NLRI(mp_reach)
    {
      int i;

      /* Create fake NEXT_HOP attribute */
      if (len < 1 || (*x != 16 && *x != 32) || len < *x + 2)
	goto bad;
      memcpy(bgp_attach_attr(&a0->eattrs, bgp_linpool, BA_NEXT_HOP, 16), x+1, 16);
      len -= *x + 2;
      x += *x + 1;

      /* Ignore SNPA info */
      i = *x++;
      while (i--)
	{
	  if (len < 1 || len < 1 + *x)
	    goto bad;
	  len -= *x + 1;
	  x += *x + 1;
	}

      if (bgp_get_nexthop(p, a0))
	{
	  a = rta_lookup(a0);
	  while (len)
	    {
	      rte *e;
	      DECODE_PREFIX(x, len);
	      DBG("Add %I/%d\n", prefix, pxlen);
	      e = rte_get_temp(rta_clone(a));
	      n = net_get(p->p.table, prefix, pxlen);
	      e->net = n;
	      e->pflags = 0;
	      rte_update(p->p.table, n, &p->p, e);
	    }
	  rta_free(a);
	}
    }

  return;

bad:
  bgp_error(conn, 3, 9, start, len0);
  if (a)
    rta_free(a);
  return;
}

#endif

static void
bgp_rx_update(struct bgp_conn *conn, byte *pkt, int len)
{
  struct bgp_proto *p = conn->bgp;
  byte *withdrawn, *attrs, *nlri;
  int withdrawn_len, attr_len, nlri_len;

  BGP_TRACE(D_PACKETS, "Got UPDATE");
  if (conn->state != BS_ESTABLISHED)
    { bgp_error(conn, 5, 0, NULL, 0); return; }
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
  { 2, 7, "Required capability missing" }, /* capability negotiation draft */
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
  { 5, 0, "Finite state machine error" },
  { 6, 0, "Cease" }
};

void
bgp_log_error(struct bgp_proto *p, char *msg, unsigned code, unsigned subcode, byte *data, unsigned len)
{
  byte *name, namebuf[16];
  byte *t, argbuf[36];
  unsigned i;

  if (code == 6 && !subcode)		/* Don't report Cease messages */
    return;

  bsprintf(namebuf, "%d.%d", code, subcode);
  name = namebuf;
  for (i=0; i < ARRAY_SIZE(bgp_msg_table); i++)
    if (bgp_msg_table[i].major == code && bgp_msg_table[i].minor == subcode)
      {
	name = bgp_msg_table[i].msg;
	break;
      }
  t = argbuf;
  if (len)
    {
      *t++ = ':';
      *t++ = ' ';
      if (len > 16)
	len = 16;
      for (i=0; i<len; i++)
	t += bsprintf(t, "%02x", data[i]);
    }
  *t = 0;
  log(L_REMOTE "%s: %s: %s%s", p->p.name, msg, name, argbuf);
}

static void
bgp_rx_notification(struct bgp_conn *conn, byte *pkt, int len)
{
  if (len < 21)
    {
      bgp_error(conn, 1, 2, pkt+16, 2);
      return;
    }
  bgp_log_error(conn->bgp, "Received error notification", pkt[19], pkt[20], pkt+21, len-21);
  conn->error_flag = 1;
  if (conn->primary)
    proto_notify_state(&conn->bgp->p, PS_STOP);
  bgp_schedule_packet(conn, PKT_SCHEDULE_CLOSE);
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
      DBG("BGP: UP!!!\n");
      conn->state = BS_ESTABLISHED;
      bgp_attr_init(conn->bgp);
      proto_notify_state(&conn->bgp->p, PS_UP);
      break;
    case BS_ESTABLISHED:
      break;
    default:
      bgp_error(conn, 5, 0, NULL, 0);
    }
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
  DBG("BGP: Got packet %02x (%d bytes)\n", pkt[18], len);
  switch (pkt[18])
    {
    case PKT_OPEN:		return bgp_rx_open(conn, pkt, len);
    case PKT_UPDATE:		return bgp_rx_update(conn, pkt, len);
    case PKT_NOTIFICATION:      return bgp_rx_notification(conn, pkt, len);
    case PKT_KEEPALIVE:		return bgp_rx_keepalive(conn);
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
      if (conn->error_flag)
	{
	  /*
	   *  We still need to remember the erroneous packet, so that
	   *  we can generate error notifications properly.  To avoid
	   *  subsequent reads rewriting the buffer, we just reset the
	   *  rx_hook.
	   */
	  DBG("BGP: Error, dropping input\n");
	  sk->rx_hook = NULL;
	  return 0;
	}
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
