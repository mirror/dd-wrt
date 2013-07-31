/*
 *	Rest in pieces - RIP protocol
 *
 *	Copyright (c) 1998, 1999 Pavel Machek <pavel@ucw.cz>
 *	              2004       Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 *
 	FIXME: IPv6 support: packet size
	FIXME: (nonurgent) IPv6 support: receive "route using" blocks
	FIXME: (nonurgent) IPv6 support: generate "nexthop" blocks
		next hops are only advisory, and they are pretty ugly in IPv6.
		I suggest just forgetting about them.

	FIXME: (nonurgent): fold rip_connection into rip_interface?

	FIXME: propagation of metric=infinity into main routing table may or may not be good idea.
 */

/**
 * DOC: Routing Information Protocol
 *
 * RIP is a pretty simple protocol, so about a half of its code is interface
 * with the core.
 *
 * We maintain our own linked list of &rip_entry structures -- it serves
 * as our small routing table. RIP never adds to this linked list upon
 * packet reception; instead, it lets the core know about data from the packet
 * and waits for the core to call rip_rt_notify().
 *
 * Within rip_tx(), the list is
 * walked and a packet is generated using rip_tx_prepare(). This gets
 * tricky because we may need to send more than one packet to one
 * destination. Struct &rip_connection is used to hold context information such as how
 * many of &rip_entry's we have already sent and it's also used to protect
 * against two concurrent sends to one destination. Each &rip_interface has
 * at most one &rip_connection.
 *
 * We are not going to honor requests for sending part of
 * routing table. That would need to turn split horizon off etc.  
 *
 * About triggered updates, RFC says: when a triggered update was sent,
 * don't send a new one for something between 1 and 5 seconds (and send one
 * after that). We do something else: each 5 seconds,
 * we look for any changed routes and broadcast them.
 */

#undef LOCAL_DEBUG
#define LOCAL_DEBUG 1

#include "nest/bird.h"
#include "nest/iface.h"
#include "nest/protocol.h"
#include "nest/route.h"
#include "lib/socket.h"
#include "lib/resource.h"
#include "lib/lists.h"
#include "lib/timer.h"
#include "lib/string.h"

#include "rip.h"

#define P ((struct rip_proto *) p)
#define P_CF ((struct rip_proto_config *)p->cf)

#define TRACE(level, msg, args...) do { if (p->debug & level) { log(L_TRACE "%s: " msg, p->name , ## args); } } while(0)

static struct rip_interface *new_iface(struct proto *p, struct iface *new, unsigned long flags, struct iface_patt *patt);

/*
 * Output processing
 *
 * This part is responsible for getting packets out to the network.
 */

static void
rip_tx_err( sock *s, int err )
{
  struct rip_connection *c = ((struct rip_interface *)(s->data))->busy;
  struct proto *p = c->proto;
  log( L_ERR "%s: Unexpected error at rip transmit: %M", p->name, err );
}

/*
 * rip_tx_prepare:
 * @e: rip entry that needs to be translated to form suitable for network
 * @b: block to be filled
 *
 * Fill one rip block with info that needs to go to the network. Handle
 * nexthop and split horizont correctly. (Next hop is ignored for IPv6,
 * that could be fixed but it is not real problem).
 */
static int
rip_tx_prepare(struct proto *p, struct rip_block *b, struct rip_entry *e, struct rip_interface *rif, int pos )
{
  int metric;
  DBG( "." );
  b->tag     = htons( e->tag );
  b->network = e->n.prefix;
  metric = e->metric;
  if (neigh_connected_to(p, &e->whotoldme, rif->iface)) {
    DBG( "(split horizon)" );
    metric = P_CF->infinity;
  }
#ifndef IPV6
  b->family  = htons( 2 ); /* AF_INET */
  b->netmask = ipa_mkmask( e->n.pxlen );
  ipa_hton( b->netmask );

  if (neigh_connected_to(p, &e->nexthop, rif->iface))
    b->nexthop = e->nexthop;
  else
    b->nexthop = IPA_NONE;
  ipa_hton( b->nexthop );  
  b->metric  = htonl( metric );
#else
  b->pxlen = e->n.pxlen;
  b->metric  = metric; /* it is u8 */
#endif

  ipa_hton( b->network );

  return pos+1;
}

/*
 * rip_tx - send one rip packet to the network
 */
static void
rip_tx( sock *s )
{
  struct rip_interface *rif = s->data;
  struct rip_connection *c = rif->busy;
  struct proto *p = c->proto;
  struct rip_packet *packet = (void *) s->tbuf;
  int i, packetlen;
  int maxi, nullupdate = 1;

  DBG( "Sending to %I\n", s->daddr );
  do {

    if (c->done)
      goto done;

    DBG( "Preparing packet to send: " );

    packet->heading.command = RIPCMD_RESPONSE;
#ifndef IPV6
    packet->heading.version = RIP_V2;
#else
    packet->heading.version = RIP_NG;
#endif
    packet->heading.unused  = 0;

    i = !!P_CF->authtype;
#ifndef IPV6
    maxi = ((P_CF->authtype == AT_MD5) ? PACKET_MD5_MAX : PACKET_MAX);
#else
    maxi = 5; /* We need to have at least reserve of one at end of packet */
#endif
    
    FIB_ITERATE_START(&P->rtable, &c->iter, z) {
      struct rip_entry *e = (struct rip_entry *) z;

      if (!rif->triggered || (!(e->updated < now-2))) {		/* FIXME: Should be probably 1 or some different algorithm */
	nullupdate = 0;
	i = rip_tx_prepare( p, packet->block + i, e, rif, i );
	if (i >= maxi) {
	  FIB_ITERATE_PUT(&c->iter, z);
	  goto break_loop;
	}
      }
    } FIB_ITERATE_END(z);
    c->done = 1;

  break_loop:

    packetlen = rip_outgoing_authentication(p, (void *) &packet->block[0], packet, i);

    DBG( ", sending %d blocks, ", i );
    if (nullupdate) {
      DBG( "not sending NULL update\n" );
      c->done = 1;
      goto done;
    }
    if (ipa_nonzero(c->daddr))
      i = sk_send_to( s, packetlen, c->daddr, c->dport );
    else
      i = sk_send( s, packetlen );

    DBG( "it wants more\n" );
  
  } while (i>0);
  
  if (i<0) rip_tx_err( s, i );
  DBG( "blocked\n" );
  return;

done:
  DBG( "Looks like I'm" );
  c->rif->busy = NULL;
  rem_node(NODE c);
  mb_free(c);
  DBG( " done\n" );
  return;
}

/* 
 * rip_sendto - send whole routing table to selected destination
 * @rif: interface to use. Notice that we lock interface so that at
 * most one send to one interface is done.
 */
static void
rip_sendto( struct proto *p, ip_addr daddr, int dport, struct rip_interface *rif )
{
  struct iface *iface = rif->iface;
  struct rip_connection *c;
  static int num = 0;

  if (rif->busy) {
    log (L_WARN "%s: Interface %s is much too slow, dropping request", p->name, iface->name);
    return;
  }
  c = mb_alloc( p->pool, sizeof( struct rip_connection ));
  rif->busy = c;
  
  c->addr = daddr;
  c->proto = p;
  c->num = num++;
  c->rif = rif;

  c->dport = dport;
  c->daddr = daddr;
  if (c->rif->sock->data != rif)
    bug("not enough send magic");

  c->done = 0;
  FIB_ITERATE_INIT( &c->iter, &P->rtable );
  add_head( &P->connections, NODE c );
  if (ipa_nonzero(daddr))
    TRACE(D_PACKETS, "Sending my routing table to %I:%d on %s", daddr, dport, rif->iface->name );
  else
    TRACE(D_PACKETS, "Broadcasting routing table to %s", rif->iface->name );

  rip_tx(c->rif->sock);
}

static struct rip_interface*
find_interface(struct proto *p, struct iface *what)
{
  struct rip_interface *i;

  WALK_LIST (i, P->interfaces)
    if (i->iface == what)
      return i;
  return NULL;
}

/*
 * Input processing
 *
 * This part is responsible for any updates that come from network 
 */

static void
rip_rte_update_if_better(rtable *tab, net *net, struct proto *p, rte *new)
{
  rte *old;

  old = rte_find(net, p);
  if (!old || p->rte_better(new, old) ||
      (ipa_equal(old->attrs->from, new->attrs->from) &&
      (old->u.rip.metric != new->u.rip.metric)) )
    rte_update(tab, net, p, p, new);
  else
    rte_free(new);
}

/*
 * advertise_entry - let main routing table know about our new entry
 * @b: entry in network format
 *
 * This basically translates @b to format used by bird core and feeds
 * bird core with this route.
 */
static void
advertise_entry( struct proto *p, struct rip_block *b, ip_addr whotoldme, struct iface *iface )
{
  rta *a, A;
  rte *r;
  net *n;
  neighbor *neighbor;
  struct rip_interface *rif;
  int pxlen;

  bzero(&A, sizeof(A));
  A.proto = p;
  A.source = RTS_RIP;
  A.scope = SCOPE_UNIVERSE;
  A.cast = RTC_UNICAST;
  A.dest = RTD_ROUTER;
  A.flags = 0;
#ifndef IPV6
  A.gw = ipa_nonzero(b->nexthop) ? b->nexthop : whotoldme;
  pxlen = ipa_mklen(b->netmask);
#else
  /* FIXME: next hop is in other packet for v6 */
  A.gw = whotoldme; 
  pxlen = b->pxlen;
#endif
  A.from = whotoldme;

  /* No need to look if destination looks valid - ie not net 0 or 127 -- core will do for us. */

  neighbor = neigh_find2( p, &A.gw, iface, 0 );
  if (!neighbor) {
    log( L_REMOTE "%s: %I asked me to route %I/%d using not-neighbor %I.", p->name, A.from, b->network, pxlen, A.gw );
    return;
  }
  if (neighbor->scope == SCOPE_HOST) {
    DBG("Self-destined route, ignoring.\n");
    return;
  }

  A.iface = neighbor->iface;
  if (!(rif = neighbor->data)) {
    rif = neighbor->data = find_interface(p, A.iface);
  }
  if (!rif)
    bug("Route packet using unknown interface? No.");
    
  /* set to: interface of nexthop */
  a = rta_lookup(&A);
  if (pxlen==-1)  {
    log( L_REMOTE "%s: %I gave me invalid pxlen/netmask for %I.", p->name, A.from, b->network );
    return;
  }
  n = net_get( p->table, b->network, pxlen );
  r = rte_get_temp(a);
#ifndef IPV6
  r->u.rip.metric = ntohl(b->metric) + rif->metric;
#else  
  r->u.rip.metric = b->metric + rif->metric;
#endif

  r->u.rip.entry = NULL;
  if (r->u.rip.metric > P_CF->infinity) r->u.rip.metric = P_CF->infinity;
  r->u.rip.tag = ntohl(b->tag);
  r->net = n;
  r->pflags = 0; /* Here go my flags */
  rip_rte_update_if_better( p->table, n, p, r );
  DBG( "done\n" );
}

/*
 * process_block - do some basic check and pass block to advertise_entry
 */
static void
process_block( struct proto *p, struct rip_block *block, ip_addr whotoldme, struct iface *iface )
{
  int metric, pxlen;

#ifndef IPV6
  metric = ntohl( block->metric );
  pxlen = ipa_mklen(block->netmask);
#else
  metric = block->metric;
  pxlen = block->pxlen;
#endif
  ip_addr network = block->network;

  CHK_MAGIC;

  TRACE(D_ROUTES, "block: %I tells me: %I/%d available, metric %d... ",
      whotoldme, network, pxlen, metric );

  if ((!metric) || (metric > P_CF->infinity)) {
#ifdef IPV6 /* Someone is sending us nexthop and we are ignoring it */
    if (metric == 0xff)
      { DBG( "IPv6 nexthop ignored" ); return; }
#endif
    log( L_WARN "%s: Got metric %d from %I", p->name, metric, whotoldme );
    return;
  }

  advertise_entry( p, block, whotoldme, iface );
}

#define BAD( x ) { log( L_REMOTE "%s: " x, p->name ); return 1; }

/*
 * rip_process_packet - this is main routine for incoming packets.
 */
static int
rip_process_packet( struct proto *p, struct rip_packet *packet, int num, ip_addr whotoldme, int port, struct iface *iface )
{
  int i;
  int authenticated = 0;
  neighbor *neighbor;

  switch( packet->heading.version ) {
  case RIP_V1: DBG( "Rip1: " ); break;
  case RIP_V2: DBG( "Rip2: " ); break;
  default: BAD( "Unknown version" );
  }

  switch( packet->heading.command ) {
  case RIPCMD_REQUEST: DBG( "Asked to send my routing table\n" ); 
	  if (P_CF->honor == HO_NEVER)
	    BAD( "They asked me to send routing table, but I was told not to do it" );

	  if ((P_CF->honor == HO_NEIGHBOR) && (!neigh_find2( p, &whotoldme, iface, 0 )))
	    BAD( "They asked me to send routing table, but he is not my neighbor" );
    	  rip_sendto( p, whotoldme, port, HEAD(P->interfaces) ); /* no broadcast */
          break;
  case RIPCMD_RESPONSE: DBG( "*** Rtable from %I\n", whotoldme ); 
          if (port != P_CF->port) {
	    log( L_REMOTE "%s: %I send me routing info from port %d", p->name, whotoldme, port );
	    return 1;
	  }

	  if (!(neighbor = neigh_find2( p, &whotoldme, iface, 0 )) || neighbor->scope == SCOPE_HOST) {
	    log( L_REMOTE "%s: %I send me routing info but he is not my neighbor", p->name, whotoldme );
	    return 0;
	  }

          for (i=0; i<num; i++) {
	    struct rip_block *block = &packet->block[i];
#ifndef IPV6
	    /* Authentication is not defined for v6 */
	    if (block->family == 0xffff) {
	      if (i)
		continue;	/* md5 tail has this family */
	      if (rip_incoming_authentication(p, (void *) block, packet, num, whotoldme))
		BAD( "Authentication failed" );
	      authenticated = 1;
	      continue;
	    }
#endif
	    if ((!authenticated) && (P_CF->authtype != AT_NONE))
	      BAD( "Packet is not authenticated and it should be" );
	    ipa_ntoh( block->network );
#ifndef IPV6
	    ipa_ntoh( block->netmask );
	    ipa_ntoh( block->nexthop );
	    if (packet->heading.version == RIP_V1)	/* FIXME (nonurgent): switch to disable this? */
	      block->netmask = ipa_class_mask(block->network);
#endif
	    process_block( p, block, whotoldme, iface );
	  }
          break;
  case RIPCMD_TRACEON:
  case RIPCMD_TRACEOFF: BAD( "I was asked for traceon/traceoff" );
  case 5: BAD( "Some Sun extension around here" );
  default: BAD( "Unknown command" );
  }

  return 0;
}

/*
 * rip_rx - Receive hook: do basic checks and pass packet to rip_process_packet
 */
static int
rip_rx(sock *s, int size)
{
  struct rip_interface *i = s->data;
  struct proto *p = i->proto;
  struct iface *iface = NULL;
  int num;

  /* In non-listening mode, just ignore packet */
  if (i->mode & IM_NOLISTEN)
    return 1;

#ifdef IPV6
  if (! i->iface || s->lifindex != i->iface->index)
    return 1;

  iface = i->iface;
#endif

  if (i->check_ttl && (s->ttl < 255))
  {
    log( L_REMOTE "%s: Discarding packet with TTL %d (< 255) from %I on %s",
	 p->name, s->ttl, s->faddr, i->iface->name);
    return 1;
  }


  CHK_MAGIC;
  DBG( "RIP: message came: %d bytes from %I via %s\n", size, s->faddr, i->iface ? i->iface->name : "(dummy)" );
  size -= sizeof( struct rip_packet_heading );
  if (size < 0) BAD( "Too small packet" );
  if (size % sizeof( struct rip_block )) BAD( "Odd sized packet" );
  num = size / sizeof( struct rip_block );
  if (num>PACKET_MAX) BAD( "Too many blocks" );

  if (ipa_equal(i->iface->addr->ip, s->faddr)) {
    DBG("My own packet\n");
    return 1;
  }

  rip_process_packet( p, (struct rip_packet *) s->rbuf, num, s->faddr, s->fport, iface );
  return 1;
}

/*
 * Interface to BIRD core
 */

static void
rip_dump_entry( struct rip_entry *e )
{
  debug( "%I told me %d/%d ago: to %I/%d go via %I, metric %d ", 
  e->whotoldme, e->updated-now, e->changed-now, e->n.prefix, e->n.pxlen, e->nexthop, e->metric );
  debug( "\n" );
}

/**
 * rip_timer
 * @t: timer
 *
 * Broadcast routing tables periodically (using rip_tx) and kill
 * routes that are too old. RIP keeps a list of its own entries present
 * in the core table by a linked list (functions rip_rte_insert() and
 * rip_rte_delete() are responsible for that), it walks this list in the timer
 * and in case an entry is too old, it is discarded.
 */

static void
rip_timer(timer *t)
{
  struct proto *p = t->data;
  struct fib_node *e, *et;

  CHK_MAGIC;
  DBG( "RIP: tick tock\n" );
  
  WALK_LIST_DELSAFE( e, et, P->garbage ) {
    rte *rte;
    rte = SKIP_BACK( struct rte, u.rip.garbage, e );

    CHK_MAGIC;

    DBG( "Garbage: (%p)", rte ); rte_dump( rte );

    if (now - rte->lastmod > P_CF->timeout_time) {
      TRACE(D_EVENTS, "entry is too old: %I", rte->net->n.prefix );
      if (rte->u.rip.entry) {
	rte->u.rip.entry->metric = P_CF->infinity;
	rte->u.rip.metric = P_CF->infinity;
      }
    }

    if (now - rte->lastmod > P_CF->garbage_time) {
      TRACE(D_EVENTS, "entry is much too old: %I", rte->net->n.prefix );
      rte_discard(p->table, rte);
    }
  }

  DBG( "RIP: Broadcasting routing tables\n" );
  {
    struct rip_interface *rif;

    if ( P_CF->period > 2 ) {		/* Bring some randomness into sending times */
      if (! (P->tx_count % P_CF->period)) P->rnd_count = random_u32() % 2;
    } else P->rnd_count = P->tx_count % P_CF->period;

    WALK_LIST( rif, P->interfaces ) {
      struct iface *iface = rif->iface;

      if (!iface) continue;
      if (rif->mode & IM_QUIET) continue;
      if (!(iface->flags & IF_UP)) continue;
      rif->triggered = P->rnd_count;

      rip_sendto( p, IPA_NONE, 0, rif );
    }
    P->tx_count++;
    P->rnd_count--;
  }

  DBG( "RIP: tick tock done\n" );
}

/*
 * rip_start - initialize instance of rip
 */
static int
rip_start(struct proto *p)
{
  struct rip_interface *rif;
  DBG( "RIP: starting instance...\n" );

  ASSERT(sizeof(struct rip_packet_heading) == 4);
  ASSERT(sizeof(struct rip_block) == 20);
  ASSERT(sizeof(struct rip_block_auth) == 20);

#ifdef LOCAL_DEBUG
  P->magic = RIP_MAGIC;
#endif
  fib_init( &P->rtable, p->pool, sizeof( struct rip_entry ), 0, NULL );
  init_list( &P->connections );
  init_list( &P->garbage );
  init_list( &P->interfaces );
  P->timer = tm_new( p->pool );
  P->timer->data = p;
  P->timer->recurrent = 1;
  P->timer->hook = rip_timer;
  tm_start( P->timer, 2 );
  rif = new_iface(p, NULL, 0, NULL);	/* Initialize dummy interface */
  add_head( &P->interfaces, NODE rif );
  CHK_MAGIC;

  rip_init_instance(p);

  DBG( "RIP: ...done\n");
  return PS_UP;
}

static struct proto *
rip_init(struct proto_config *cfg)
{
  struct proto *p = proto_new(cfg, sizeof(struct rip_proto));

  return p;
}

static void
rip_dump(struct proto *p)
{
  int i;
  node *w;
  struct rip_interface *rif;

  CHK_MAGIC;
  WALK_LIST( w, P->connections ) {
    struct rip_connection *n = (void *) w;
    debug( "RIP: connection #%d: %I\n", n->num, n->addr );
  }
  i = 0;
  FIB_WALK( &P->rtable, e ) {
    debug( "RIP: entry #%d: ", i++ );
    rip_dump_entry( (struct rip_entry *)e );
  } FIB_WALK_END;
  i = 0;
  WALK_LIST( rif, P->interfaces ) {
    debug( "RIP: interface #%d: %s, %I, busy = %x\n", i++, rif->iface?rif->iface->name:"(dummy)", rif->sock->daddr, rif->busy );
  }
}

static void
rip_get_route_info(rte *rte, byte *buf, ea_list *attrs)
{
  eattr *metric = ea_find(attrs, EA_RIP_METRIC);
  eattr *tag = ea_find(attrs, EA_RIP_TAG);

  buf += bsprintf(buf, " (%d/%d)", rte->pref, metric ? metric->u.data : 0);
  if (tag && tag->u.data)
    bsprintf(buf, " t%04x", tag->u.data);
}

static void
kill_iface(struct rip_interface *i)
{
  DBG( "RIP: Interface %s disappeared\n", i->iface->name);
  rfree(i->sock);
  mb_free(i);
}

/**
 * new_iface
 * @p: myself
 * @new: interface to be created or %NULL if we are creating a magic
 * socket. The magic socket is used for listening and also for
 * sending requested responses.
 * @flags: interface flags
 * @patt: pattern this interface matched, used for access to config options
 *
 * Create an interface structure and start listening on the interface.
 */
static struct rip_interface *
new_iface(struct proto *p, struct iface *new, unsigned long flags, struct iface_patt *patt )
{
  struct rip_interface *rif;
  struct rip_patt *PATT = (struct rip_patt *) patt;

  rif = mb_allocz(p->pool, sizeof( struct rip_interface ));
  rif->iface = new;
  rif->proto = p;
  rif->busy = NULL;
  if (PATT) {
    rif->mode = PATT->mode;
    rif->metric = PATT->metric;
    rif->multicast = (!(PATT->mode & IM_BROADCAST)) && (flags & IF_MULTICAST);
    rif->check_ttl = (PATT->ttl_security == 1);
  }
  /* lookup multicasts over unnumbered links - no: rip is not defined over unnumbered links */

  if (rif->multicast)
    DBG( "Doing multicasts!\n" );

  rif->sock = sk_new( p->pool );
  rif->sock->type = SK_UDP;
  rif->sock->sport = P_CF->port;
  rif->sock->rx_hook = rip_rx;
  rif->sock->data = rif;
  rif->sock->rbsize = 10240;
  rif->sock->iface = new;		/* Automagically works for dummy interface */
  rif->sock->tbuf = mb_alloc( p->pool, sizeof( struct rip_packet ));
  rif->sock->tx_hook = rip_tx;
  rif->sock->err_hook = rip_tx_err;
  rif->sock->daddr = IPA_NONE;
  rif->sock->dport = P_CF->port;
  if (new)
    {
      rif->sock->tos = PATT->tx_tos;
      rif->sock->priority = PATT->tx_priority;
      rif->sock->ttl = PATT->ttl_security ? 255 : 1;
      rif->sock->flags = SKF_LADDR_RX | (rif->check_ttl ? SKF_TTL_RX : 0);
    }

  if (new) {
    if (new->addr->flags & IA_PEER)
      log( L_WARN "%s: rip is not defined over unnumbered links", p->name );
    rif->sock->saddr = IPA_NONE;
    if (rif->multicast) {
#ifndef IPV6
      rif->sock->daddr = ipa_from_u32(0xe0000009);
#else
      rif->sock->daddr = ipa_build(0xff020000, 0, 0, 9);
#endif
    } else {
      rif->sock->daddr = new->addr->brd;
    }
  }

  if (!ipa_nonzero(rif->sock->daddr)) {
    if (rif->iface)
      log( L_WARN "%s: interface %s is too strange for me", p->name, rif->iface->name );
  } else {

    if (sk_open(rif->sock)<0)
      goto err;

    if (rif->multicast)
      {
	if (sk_setup_multicast(rif->sock) < 0)
	  goto err;
	if (sk_join_group(rif->sock, rif->sock->daddr) < 0)
	  goto err;
      }
    else
      {
	if (sk_set_broadcast(rif->sock, 1) < 0)
	  goto err;
      }
  }

  TRACE(D_EVENTS, "Listening on %s, port %d, mode %s (%I)", rif->iface ? rif->iface->name : "(dummy)", P_CF->port, rif->multicast ? "multicast" : "broadcast", rif->sock->daddr );
  
  return rif;

 err:
  log( L_ERR "%s: could not create socket for %s", p->name, rif->iface ? rif->iface->name : "(dummy)" );
  if (rif->iface) {
    rfree(rif->sock);
    mb_free(rif);
    return NULL;
  }
  /* On dummy, we just return non-working socket, so that user gets error every time anyone requests table */
  return rif;
}

static void
rip_real_if_add(struct object_lock *lock)
{
  struct iface *iface = lock->iface;
  struct proto *p = lock->data;
  struct rip_interface *rif;
  struct iface_patt *k = iface_patt_find(&P_CF->iface_list, iface, iface->addr);

  if (!k)
    bug("This can not happen! It existed few seconds ago!" );
  DBG("adding interface %s\n", iface->name );
  rif = new_iface(p, iface, iface->flags, k);
  if (rif) {
    add_head( &P->interfaces, NODE rif );
    DBG("Adding object lock of %p for %p\n", lock, rif);
    rif->lock = lock;
  } else { rfree(lock); }
}

static void
rip_if_notify(struct proto *p, unsigned c, struct iface *iface)
{
  DBG( "RIP: if notify\n" );
  if (iface->flags & IF_IGNORE)
    return;
  if (c & IF_CHANGE_DOWN) {
    struct rip_interface *i;
    i = find_interface(p, iface);
    if (i) {
      rem_node(NODE i);
      rfree(i->lock);
      kill_iface(i);
    }
  }
  if (c & IF_CHANGE_UP) {
    struct iface_patt *k = iface_patt_find(&P_CF->iface_list, iface, iface->addr);
    struct object_lock *lock;
    struct rip_patt *PATT = (struct rip_patt *) k;

    if (!k) return; /* We are not interested in this interface */

    lock = olock_new( p->pool );
    if (!(PATT->mode & IM_BROADCAST) && (iface->flags & IF_MULTICAST))
#ifndef IPV6
      lock->addr = ipa_from_u32(0xe0000009);
#else
      ip_pton("FF02::9", &lock->addr);
#endif
    else
      lock->addr = iface->addr->brd;
    lock->port = P_CF->port;
    lock->iface = iface;
    lock->hook = rip_real_if_add;
    lock->data = p;
    lock->type = OBJLOCK_UDP;
    olock_acquire(lock);
  }
}

static struct ea_list *
rip_gen_attrs(struct linpool *pool, int metric, u16 tag)
{
  struct ea_list *l = lp_alloc(pool, sizeof(struct ea_list) + 2*sizeof(eattr));

  l->next = NULL;
  l->flags = EALF_SORTED;
  l->count = 2;
  l->attrs[0].id = EA_RIP_TAG;
  l->attrs[0].flags = 0;
  l->attrs[0].type = EAF_TYPE_INT | EAF_TEMP;
  l->attrs[0].u.data = tag;
  l->attrs[1].id = EA_RIP_METRIC;
  l->attrs[1].flags = 0;
  l->attrs[1].type = EAF_TYPE_INT | EAF_TEMP;
  l->attrs[1].u.data = metric;
  return l;
}

static int
rip_import_control(struct proto *p, struct rte **rt, struct ea_list **attrs, struct linpool *pool)
{
  if ((*rt)->attrs->proto == p)	/* My own must not be touched */
    return 1;

  if ((*rt)->attrs->source != RTS_RIP) {
    struct ea_list *new = rip_gen_attrs(pool, 1, 0);
    new->next = *attrs;
    *attrs = new;
  }
  return 0;
}

static struct ea_list *
rip_make_tmp_attrs(struct rte *rt, struct linpool *pool)
{
  return rip_gen_attrs(pool, rt->u.rip.metric, rt->u.rip.tag);
}

static void 
rip_store_tmp_attrs(struct rte *rt, struct ea_list *attrs)
{
  rt->u.rip.tag = ea_get_int(attrs, EA_RIP_TAG, 0);
  rt->u.rip.metric = ea_get_int(attrs, EA_RIP_METRIC, 1);
}

/*
 * rip_rt_notify - core tells us about new route (possibly our
 * own), so store it into our data structures. 
 */
static void
rip_rt_notify(struct proto *p, struct rtable *table UNUSED, struct network *net,
	      struct rte *new, struct rte *old UNUSED, struct ea_list *attrs)
{
  CHK_MAGIC;
  struct rip_entry *e;

  e = fib_find( &P->rtable, &net->n.prefix, net->n.pxlen );
  if (e)
    fib_delete( &P->rtable, e );

  if (new) {
    e = fib_get( &P->rtable, &net->n.prefix, net->n.pxlen );

    e->nexthop = new->attrs->gw;
    e->metric = 0;
    e->whotoldme = IPA_NONE;
    new->u.rip.entry = e;

    e->tag = ea_get_int(attrs, EA_RIP_TAG, 0);
    e->metric = ea_get_int(attrs, EA_RIP_METRIC, 1);
    if (e->metric > P_CF->infinity)
      e->metric = P_CF->infinity;

    if (new->attrs->proto == p)
      e->whotoldme = new->attrs->from;

    if (!e->metric)	/* That's okay: this way user can set his own value for external
			   routes in rip. */
      e->metric = 5;
    e->updated = e->changed = now;
    e->flags = 0;
  }
}

static int
rip_rte_same(struct rte *new, struct rte *old)
{
  /* new->attrs == old->attrs always */
  return new->u.rip.metric == old->u.rip.metric;
}


static int
rip_rte_better(struct rte *new, struct rte *old)
{
  struct proto *p = new->attrs->proto;

  if (ipa_equal(old->attrs->from, new->attrs->from))
    return 1;

  if (old->u.rip.metric < new->u.rip.metric)
    return 0;

  if (old->u.rip.metric > new->u.rip.metric)
    return 1;

  if (old->attrs->proto == new->attrs->proto)		/* This does not make much sense for different protocols */
    if ((old->u.rip.metric == new->u.rip.metric) &&
	((now - old->lastmod) > (P_CF->timeout_time / 2)))
      return 1;

  return 0;
}

/*
 * rip_rte_insert - we maintain linked list of "our" entries in main
 * routing table, so that we can timeout them correctly. rip_timer()
 * walks the list.
 */
static void
rip_rte_insert(net *net UNUSED, rte *rte)
{
  struct proto *p = rte->attrs->proto;
  CHK_MAGIC;
  DBG( "rip_rte_insert: %p\n", rte );
  add_head( &P->garbage, &rte->u.rip.garbage );
}

/*
 * rip_rte_remove - link list maintenance
 */
static void
rip_rte_remove(net *net UNUSED, rte *rte)
{
#ifdef LOCAL_DEBUG
  struct proto *p = rte->attrs->proto;
  CHK_MAGIC;
  DBG( "rip_rte_remove: %p\n", rte );
#endif
  rem_node( &rte->u.rip.garbage );
}

void
rip_init_instance(struct proto *p)
{
  p->accept_ra_types = RA_OPTIMAL;
  p->if_notify = rip_if_notify;
  p->rt_notify = rip_rt_notify;
  p->import_control = rip_import_control;
  p->make_tmp_attrs = rip_make_tmp_attrs;
  p->store_tmp_attrs = rip_store_tmp_attrs;
  p->rte_better = rip_rte_better;
  p->rte_same = rip_rte_same;
  p->rte_insert = rip_rte_insert;
  p->rte_remove = rip_rte_remove;
}

void
rip_init_config(struct rip_proto_config *c)
{
  init_list(&c->iface_list);
  c->infinity	= 16;
  c->port	= RIP_PORT;
  c->period	= 30;
  c->garbage_time = 120+180;
  c->timeout_time = 120;
  c->passwords	= NULL;
  c->authtype	= AT_NONE;
}

static int
rip_get_attr(eattr *a, byte *buf, int buflen UNUSED)
{
  switch (a->id) {
  case EA_RIP_METRIC: bsprintf( buf, "metric: %d", a->u.data ); return GA_FULL;
  case EA_RIP_TAG:    bsprintf( buf, "tag: %d", a->u.data );    return GA_FULL;
  default: return GA_UNKNOWN;
  }
}

static int
rip_pat_compare(struct rip_patt *a, struct rip_patt *b)
{
  return ((a->metric == b->metric) &&
	  (a->mode == b->mode) &&
	  (a->tx_tos == b->tx_tos) &&
	  (a->tx_priority == b->tx_priority));
}

static int
rip_reconfigure(struct proto *p, struct proto_config *c)
{
  struct rip_proto_config *new = (struct rip_proto_config *) c;
  int generic = sizeof(struct proto_config) + sizeof(list) /* + sizeof(struct password_item *) */;

  if (!iface_patts_equal(&P_CF->iface_list, &new->iface_list, (void *) rip_pat_compare))
    return 0;
  return !memcmp(((byte *) P_CF) + generic,
                 ((byte *) new) + generic,
                 sizeof(struct rip_proto_config) - generic);
}

static void
rip_copy_config(struct proto_config *dest, struct proto_config *src)
{
  /* Shallow copy of everything */
  proto_copy_rest(dest, src, sizeof(struct rip_proto_config));

  /* We clean up iface_list, ifaces are non-sharable */
  init_list(&((struct rip_proto_config *) dest)->iface_list);

  /* Copy of passwords is OK, it just will be replaced in dest when used */
}


struct protocol proto_rip = {
  name: "RIP",
  template: "rip%d",
  attr_class: EAP_RIP,
  preference: DEF_PREF_RIP,
  get_route_info: rip_get_route_info,
  get_attr: rip_get_attr,

  init: rip_init,
  dump: rip_dump,
  start: rip_start,
  reconfigure: rip_reconfigure,
  copy_config: rip_copy_config
};
