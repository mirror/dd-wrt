/*
 *	BIRD -- The Border Gateway Protocol
 *
 *	(c) 2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Border Gateway Protocol
 *
 * The BGP protocol is implemented in three parts: |bgp.c| which takes care of the
 * connection and most of the interface with BIRD core, |packets.c| handling
 * both incoming and outgoing BGP packets and |attrs.c| containing functions for
 * manipulation with BGP attribute lists.
 *
 * As opposed to the other existing routing daemons, BIRD has a sophisticated core
 * architecture which is able to keep all the information needed by BGP in the
 * primary routing table, therefore no complex data structures like a central
 * BGP table are needed. This increases memory footprint of a BGP router with
 * many connections, but not too much and, which is more important, it makes
 * BGP much easier to implement.
 *
 * Each instance of BGP (corresponding to a single BGP peer) is described by a &bgp_proto
 * structure to which are attached individual connections represented by &bgp_connection
 * (usually, there exists only one connection, but during BGP session setup, there
 * can be more of them). The connections are handled according to the BGP state machine
 * defined in the RFC with all the timers and all the parameters configurable.
 *
 * In incoming direction, we listen on the connection's socket and each time we receive
 * some input, we pass it to bgp_rx(). It decodes packet headers and the markers and
 * passes complete packets to bgp_rx_packet() which distributes the packet according
 * to its type.
 *
 * In outgoing direction, we gather all the routing updates and sort them to buckets
 * (&bgp_bucket) according to their attributes (we keep a hash table for fast comparison
 * of &rta's and a &fib which helps us to find if we already have another route for
 * the same destination queued for sending, so that we can replace it with the new one
 * immediately instead of sending both updates). There also exists a special bucket holding
 * all the route withdrawals which cannot be queued anywhere else as they don't have any
 * attributes. If we have any packet to send (due to either new routes or the connection
 * tracking code wanting to send a Open, Keepalive or Notification message), we call
 * bgp_schedule_packet() which sets the corresponding bit in a @packet_to_send
 * bit field in &bgp_conn and as soon as the transmit socket buffer becomes empty,
 * we call bgp_fire_tx(). It inspects state of all the packet type bits and calls
 * the corresponding bgp_create_xx() functions, eventually rescheduling the same packet
 * type if we have more data of the same type to send.
 *
 * The processing of attributes consists of two functions: bgp_decode_attrs() for checking
 * of the attribute blocks and translating them to the language of BIRD's extended attributes
 * and bgp_encode_attrs() which does the converse. Both functions are built around a
 * @bgp_attr_table array describing all important characteristics of all known attributes.
 * Unknown transitive attributes are attached to the route as %EAF_TYPE_OPAQUE byte streams.
 */

#undef LOCAL_DEBUG

#include "nest/bird.h"
#include "nest/iface.h"
#include "nest/protocol.h"
#include "nest/route.h"
#include "nest/locks.h"
#include "conf/conf.h"
#include "lib/socket.h"
#include "lib/resource.h"
#include "lib/string.h"

#include "bgp.h"

struct linpool *bgp_linpool;		/* Global temporary pool */
static sock *bgp_listen_sk;		/* Global listening socket */
static int bgp_counter;			/* Number of protocol instances using the listening socket */
static char *bgp_state_names[] = { "Idle", "Connect", "Active", "OpenSent", "OpenConfirm", "Established" };

static void bgp_connect(struct bgp_proto *p);
static void bgp_initiate(struct bgp_proto *p);
static void bgp_setup_listen_sk(void);

static void
bgp_close(struct bgp_proto *p UNUSED)
{
  ASSERT(bgp_counter);
  bgp_counter--;
  if (!bgp_counter)
    {
      rfree(bgp_listen_sk);
      bgp_listen_sk = NULL;
      rfree(bgp_linpool);
      bgp_linpool = NULL;
    }
}

/**
 * bgp_start_timer - start a BGP timer
 * @t: timer
 * @value: time to fire (0 to disable the timer)
 *
 * This functions calls tm_start() on @t with time @value and the
 * amount of randomization suggested by the BGP standard. Please use
 * it for all BGP timers.
 */
void
bgp_start_timer(timer *t, int value)
{
  if (value)
    {
      /* The randomization procedure is specified in RFC 1771: 9.2.3.3 */
      t->randomize = value / 4;
      tm_start(t, value - t->randomize);
    }
  else
    tm_stop(t);
}

/**
 * bgp_close_conn - close a BGP connection
 * @conn: connection to close
 *
 * This function takes a connection described by the &bgp_conn structure,
 * closes its socket and frees all resources associated with it.
 *
 * If the connection is being closed due to a protocol error, adjust
 * the connection restart timer as well according to the error recovery
 * policy set in the configuration.
 *
 * If the connection was marked as primary, it shuts down the protocol as well.
 */
void
bgp_close_conn(struct bgp_conn *conn)
{
  struct bgp_proto *p = conn->bgp;
  struct bgp_config *cf = p->cf;

  DBG("BGP: Closing connection\n");
  conn->packets_to_send = 0;
  rfree(conn->connect_retry_timer);
  conn->connect_retry_timer = NULL;
  rfree(conn->keepalive_timer);
  conn->keepalive_timer = NULL;
  rfree(conn->hold_timer);
  conn->hold_timer = NULL;
  rfree(conn->sk);
  conn->sk = NULL;
  conn->state = BS_IDLE;
  if (conn->error_flag > 1)
    {
      if (cf->disable_after_error)
	p->p.disabled = 1;
      if (p->last_connect && (bird_clock_t)(p->last_connect + cf->error_amnesia_time) < now)
	p->startup_delay = 0;
      if (!p->startup_delay)
	p->startup_delay = cf->error_delay_time_min;
      else
	{
	  p->startup_delay *= 2;
	  if (p->startup_delay > cf->error_delay_time_max)
	    p->startup_delay = cf->error_delay_time_max;
	}
    }
  if (conn->primary)
    {
      bgp_close(p);
      p->conn = NULL;
      proto_notify_state(&p->p, PS_DOWN);
    }
  else if (conn->error_flag > 1)
    bgp_initiate(p);
}

static int
bgp_graceful_close_conn(struct bgp_conn *c)
{
  switch (c->state)
    {
    case BS_IDLE:
      return 0;
    case BS_CONNECT:
    case BS_ACTIVE:
      bgp_close_conn(c);
      return 1;
    case BS_OPENSENT:
    case BS_OPENCONFIRM:
    case BS_ESTABLISHED:
      bgp_error(c, 6, 0, NULL, 0);
      return 1;
    default:
      bug("bgp_graceful_close_conn: Unknown state %d", c->state);
    }
}

static void
bgp_send_open(struct bgp_conn *conn)
{
  DBG("BGP: Sending open\n");
  conn->sk->rx_hook = bgp_rx;
  conn->sk->tx_hook = bgp_tx;
  tm_stop(conn->connect_retry_timer);
  bgp_schedule_packet(conn, PKT_OPEN);
  conn->state = BS_OPENSENT;
  bgp_start_timer(conn->hold_timer, conn->bgp->cf->initial_hold_time);
}

static void
bgp_connected(sock *sk)
{
  struct bgp_conn *conn = sk->data;
  struct bgp_proto *p = conn->bgp;

  BGP_TRACE(D_EVENTS, "Connected");
  bgp_send_open(conn);
}

static void
bgp_connect_timeout(timer *t)
{
  struct bgp_conn *conn = t->data;
  struct bgp_proto *p = conn->bgp;

  DBG("BGP: connect_timeout\n");
  bgp_close_conn(conn);
  bgp_connect(p);
}

static void
bgp_sock_err(sock *sk, int err)
{
  struct bgp_conn *conn = sk->data;
  struct bgp_proto *p = conn->bgp;

  if (err)
    BGP_TRACE(D_EVENTS, "Connection lost (%M)", err);
  else
    BGP_TRACE(D_EVENTS, "Connection closed");
  switch (conn->state)
    {
    case BS_CONNECT:
    case BS_OPENSENT:
      rfree(conn->sk);
      conn->sk = NULL;
      conn->state = BS_ACTIVE;
      bgp_start_timer(conn->connect_retry_timer, p->cf->connect_retry_time);
      break;
    case BS_OPENCONFIRM:
    case BS_ESTABLISHED:
      bgp_close_conn(conn);
      break;
    default:
      bug("bgp_sock_err called in invalid state %d", conn->state);
    }
}

static void
bgp_hold_timeout(timer *t)
{
  struct bgp_conn *conn = t->data;

  DBG("BGP: Hold timeout, closing connection\n");
  bgp_error(conn, 4, 0, NULL, 0);
}

static void
bgp_keepalive_timeout(timer *t)
{
  struct bgp_conn *conn = t->data;

  DBG("BGP: Keepalive timer\n");
  bgp_schedule_packet(conn, PKT_KEEPALIVE);
}

static void
bgp_setup_conn(struct bgp_proto *p, struct bgp_conn *conn)
{
  timer *t;

  conn->sk = NULL;
  conn->bgp = p;
  conn->packets_to_send = 0;
  conn->error_flag = 0;
  conn->primary = 0;

  t = conn->connect_retry_timer = tm_new(p->p.pool);
  t->hook = bgp_connect_timeout;
  t->data = conn;
  t = conn->hold_timer = tm_new(p->p.pool);
  t->hook = bgp_hold_timeout;
  t->data = conn;
  t = conn->keepalive_timer = tm_new(p->p.pool);
  t->hook = bgp_keepalive_timeout;
  t->data = conn;
}

static void
bgp_setup_sk(struct bgp_proto *p, struct bgp_conn *conn, sock *s)
{
  s->data = conn;
  s->ttl = p->cf->multihop ? : 1;
  s->rbsize = BGP_RX_BUFFER_SIZE;
  s->tbsize = BGP_TX_BUFFER_SIZE;
  s->err_hook = bgp_sock_err;
  s->tos = IP_PREC_INTERNET_CONTROL;
  conn->sk = s;
}

/**
 * bgp_connect - initiate an outgoing connection
 * @p: BGP instance
 *
 * The bgp_connect() function creates a new &bgp_conn and initiates
 * a TCP connection to the peer. The rest of connection setup is governed
 * by the BGP state machine as described in the standard.
 */
static void
bgp_connect(struct bgp_proto *p)	/* Enter Connect state and start establishing connection */
{
  sock *s;
  struct bgp_conn *conn = &p->outgoing_conn;

  DBG("BGP: Connecting\n");
  p->last_connect = now;
  s = sk_new(p->p.pool);
  s->type = SK_TCP_ACTIVE;
  if (ipa_nonzero(p->cf->source_addr))
    s->saddr = p->cf->source_addr;
  else
    s->saddr = p->local_addr;
  s->daddr = p->cf->remote_ip;
  s->dport = BGP_PORT;
  BGP_TRACE(D_EVENTS, "Connecting to %I from local address %I", s->daddr, s->saddr);
  bgp_setup_conn(p, conn);
  bgp_setup_sk(p, conn, s);
  s->tx_hook = bgp_connected;
  conn->state = BS_CONNECT;
  if (sk_open(s))
    {
      bgp_sock_err(s, 0);
      return;
    }
  DBG("BGP: Waiting for connect success\n");
  bgp_start_timer(conn->connect_retry_timer, p->cf->connect_retry_time);
}

static void
bgp_initiate(struct bgp_proto *p)
{
  unsigned delay;

  delay = p->cf->start_delay_time;
  if (p->startup_delay > delay)
    delay = p->startup_delay;
  if (delay)
    {
      BGP_TRACE(D_EVENTS, "Connect delayed by %d seconds", delay);
      bgp_setup_conn(p, &p->outgoing_conn);
      bgp_start_timer(p->outgoing_conn.connect_retry_timer, delay);
    }
  else
    bgp_connect(p);
}

/**
 * bgp_incoming_connection - handle an incoming connection
 * @sk: TCP socket
 * @dummy: unused
 *
 * This function serves as a socket hook for accepting of new BGP
 * connections. It searches a BGP instance corresponding to the peer
 * which has connected and if such an instance exists, it creates a
 * &bgp_conn structure, attaches it to the instance and either sends
 * an Open message or (if there already is an active connection) it
 * closes the new connection by sending a Notification message.
 */
static int
bgp_incoming_connection(sock *sk, int dummy UNUSED)
{
  struct proto_config *pc;
  int match = 0;

  DBG("BGP: Incoming connection from %I port %d\n", sk->daddr, sk->dport);
  WALK_LIST(pc, config->protos)
    if (pc->protocol == &proto_bgp && pc->proto)
      {
	struct bgp_proto *p = (struct bgp_proto *) pc->proto;
	if (ipa_equal(p->cf->remote_ip, sk->daddr))
	  {
	    match = 1;
	    if ((p->p.proto_state == PS_START || p->p.proto_state == PS_UP) && p->neigh && p->neigh->iface)
	      {
		BGP_TRACE(D_EVENTS, "Incoming connection from %I port %d", sk->daddr, sk->dport);
		if (p->incoming_conn.sk)
		  {
		    DBG("BGP: But one incoming connection already exists, how is that possible?\n");
		    break;
		  }
		bgp_setup_conn(p, &p->incoming_conn);
		bgp_setup_sk(p, &p->incoming_conn, sk);
		bgp_send_open(&p->incoming_conn);
		return 0;
	      }
	  }
      }
  if (!match)
    log(L_AUTH "BGP: Unauthorized connect from %I port %d", sk->daddr, sk->dport);
  rfree(sk);
  return 0;
}

static void
bgp_setup_listen_sk(void)
{
  if (!bgp_listen_sk)
    {
      sock *s = sk_new(&root_pool);
      DBG("BGP: Creating incoming socket\n");
      s->type = SK_TCP_PASSIVE;
      s->sport = BGP_PORT;
      s->tos = IP_PREC_INTERNET_CONTROL;
      s->ttl = 1;
      s->rbsize = BGP_RX_BUFFER_SIZE;
      s->tbsize = BGP_TX_BUFFER_SIZE;
      s->rx_hook = bgp_incoming_connection;
      if (sk_open(s))
	{
	  log(L_ERR "Unable to open incoming BGP socket");
	  rfree(s);
	}
      else
	bgp_listen_sk = s;
    }
}

static void
bgp_start_neighbor(struct bgp_proto *p)
{
  p->local_addr = p->neigh->iface->addr->ip;
  DBG("BGP: local=%I remote=%I\n", p->local_addr, p->next_hop);
#ifdef IPV6
  {
    struct ifa *a;
    p->local_link = ipa_or(ipa_build(0xfe80,0,0,0), ipa_and(p->local_addr, ipa_build(0,0,~0,~0)));
    WALK_LIST(a, p->neigh->iface->addrs)
      if (a->scope == SCOPE_LINK)
        {
	  p->local_link = a->ip;
	  break;
	}
    DBG("BGP: Selected link-level address %I\n", p->local_link);
  }
#endif
  bgp_initiate(p);
}

static void
bgp_neigh_notify(neighbor *n)
{
  struct bgp_proto *p = (struct bgp_proto *) n->proto;

  if (n->iface)
    {
      BGP_TRACE(D_EVENTS, "Neighbor found");
      bgp_start_neighbor(p);
    }
  else
    {
      BGP_TRACE(D_EVENTS, "Neighbor lost");
      /* Send cease packets, but don't wait for them to be delivered */
      bgp_graceful_close_conn(&p->outgoing_conn);
      bgp_graceful_close_conn(&p->incoming_conn);
      proto_notify_state(&p->p, PS_DOWN);
    }
}

static void
bgp_start_locked(struct object_lock *lock)
{
  struct bgp_proto *p = lock->data;
  struct bgp_config *cf = p->cf;

  DBG("BGP: Got lock\n");
  p->local_id = cf->c.global->router_id;
  p->next_hop = cf->multihop ? cf->multihop_via : cf->remote_ip;
  p->neigh = neigh_find(&p->p, &p->next_hop, NEF_STICKY);
  if (!p->neigh)
    {
      log(L_ERR "%s: Invalid next hop %I", p->p.name, p->next_hop);
      p->p.disabled = 1;
      proto_notify_state(&p->p, PS_DOWN);
    }
  else if (p->neigh->iface)
    bgp_start_neighbor(p);
  else
    BGP_TRACE(D_EVENTS, "Waiting for %I to become my neighbor", p->next_hop);
}

static int
bgp_start(struct proto *P)
{
  struct bgp_proto *p = (struct bgp_proto *) P;
  struct object_lock *lock;

  DBG("BGP: Startup.\n");
  p->outgoing_conn.state = BS_IDLE;
  p->incoming_conn.state = BS_IDLE;
  p->startup_delay = 0;
  p->neigh = NULL;

  bgp_counter++;
  bgp_setup_listen_sk();
  if (!bgp_linpool)
    bgp_linpool = lp_new(&root_pool, 4080);

  /*
   *  Before attempting to create the connection, we need to lock the
   *  port, so that are sure we're the only instance attempting to talk
   *  with that neighbor.
   */

  lock = p->lock = olock_new(P->pool);
  lock->addr = p->cf->remote_ip;
  lock->type = OBJLOCK_TCP;
  lock->port = BGP_PORT;
  lock->iface = NULL;
  lock->hook = bgp_start_locked;
  lock->data = p;
  olock_acquire(lock);
  return PS_START;
}

static int
bgp_shutdown(struct proto *P)
{
  struct bgp_proto *p = (struct bgp_proto *) P;

  BGP_TRACE(D_EVENTS, "Shutdown requested");

  /*
   *  We want to send the Cease notification message to all connections
   *  we have open, but we don't want to wait for all of them to complete.
   *  We are willing to handle the primary connection carefully, but for
   *  the others we just try to send the packet and if there is no buffer
   *  space free, we'll gracefully finish.
   */

  proto_notify_state(&p->p, PS_STOP);
  if (!p->conn)
    {
      if (p->outgoing_conn.state != BS_IDLE)
	p->outgoing_conn.primary = 1;	/* Shuts protocol down after connection close */
      else if (p->incoming_conn.state != BS_IDLE)
	p->incoming_conn.primary = 1;
    }
  if (bgp_graceful_close_conn(&p->outgoing_conn) || bgp_graceful_close_conn(&p->incoming_conn))
    return p->p.proto_state;
  else
    {
      /* No connections open, shutdown automatically */
      bgp_close(p);
      return PS_DOWN;
    }
}

static struct proto *
bgp_init(struct proto_config *C)
{
  struct bgp_config *c = (struct bgp_config *) C;
  struct proto *P = proto_new(C, sizeof(struct bgp_proto));
  struct bgp_proto *p = (struct bgp_proto *) P;

  P->rt_notify = bgp_rt_notify;
  P->rte_better = bgp_rte_better;
  P->import_control = bgp_import_control;
  P->neigh_notify = bgp_neigh_notify;
  p->cf = c;
  p->local_as = c->local_as;
  p->remote_as = c->remote_as;
  p->is_internal = (c->local_as == c->remote_as);
  return P;
}

/**
 * bgp_error - report a protocol error
 * @c: connection
 * @code: error code (according to the RFC)
 * @subcode: error sub-code
 * @data: data to be passed in the Notification message
 * @len: length of the data
 *
 * bgp_error() sends a notification packet to tell the other side that a protocol
 * error has occurred (including the data considered erroneous if possible) and
 * closes the connection.
 */
void
bgp_error(struct bgp_conn *c, unsigned code, unsigned subcode, byte *data, int len)
{
  if (c->error_flag)
    return;
  bgp_log_error(c->bgp, "Error", code, subcode, data, (len > 0) ? len : -len);
  c->error_flag = 1 + (code != 6);
  c->notify_code = code;
  c->notify_subcode = subcode;
  c->notify_data = data;
  c->notify_size = (len > 0) ? len : 0;
  if (c->primary)
    proto_notify_state(&c->bgp->p, PS_STOP);
  bgp_schedule_packet(c, PKT_NOTIFICATION);
}

void
bgp_check(struct bgp_config *c)
{
  if (!c->local_as)
    cf_error("Local AS number must be set");
  if (!c->remote_as)
    cf_error("Neighbor must be configured");
}

static void
bgp_get_status(struct proto *P, byte *buf)
{
  struct bgp_proto *p = (struct bgp_proto *) P;

  if (P->proto_state == PS_DOWN)
    buf[0] = 0;
  else
    strcpy(buf, bgp_state_names[MAX(p->incoming_conn.state, p->outgoing_conn.state)]);
}

static int
bgp_reconfigure(struct proto *P, struct proto_config *C)
{
  struct bgp_config *new = (struct bgp_config *) C;
  struct bgp_proto *p = (struct bgp_proto *) P;
  struct bgp_config *old = p->cf;

  return !memcmp(((byte *) old) + sizeof(struct proto_config),
		 ((byte *) new) + sizeof(struct proto_config),
		 sizeof(struct bgp_config) - sizeof(struct proto_config));
}

struct protocol proto_bgp = {
  name:			"BGP",
  template:		"bgp%d",
  attr_class:		EAP_BGP,
  init:			bgp_init,
  start:		bgp_start,
  shutdown:		bgp_shutdown,
  get_status:		bgp_get_status,
  get_attr:		bgp_get_attr,
  reconfigure:		bgp_reconfigure,
  get_route_info:	bgp_get_route_info,
};
